#include "VS_TransceiverParticipant.h"

#include "std/cpplib/VS_ConferenceID.h"
#include "streams/Command.h"
#include "streams/Handshake.h"
#include "streams/Protocol.h"
#include "streams/VS_StreamBufferedConnection.h"
#include "net/Address.h"
#include "net/Port.h"
#include "net/tls/socket.h"
#include "SIPParserBase/VS_Const.h"
#include "std/debuglog/VS_Debug.h"
#include "net/DNSUtils/VS_DNS.h"
#include "net/DNSUtils/VS_DNSUtils.h"
#include "net/Lib.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

#define TC_VERBOSE_LOGS 0

VS_TransceiverParticipant::VS_TransceiverParticipant(
	std::string ServerAddr, std::string ServerEP,
	std::string confName, std::string partName,
	boost::asio::io_service& ios)
	: m_serverEP(std::move(ServerEP))
	, m_conf_name(std::move(confName))
	, m_part_name(std::move(partName))
	, m_isStopped(true)
	, m_ios(ios)
{
	std::string addrs(ServerAddr);
	addrs.push_back(','); // Finalize string with address separator to simplify parsing
	auto addr_start = addrs.begin();
	while (true)
	{
		auto addr_sep = std::find(addr_start, addrs.end(), ',');
		if (addr_sep == addrs.end())
			break;
		*addr_sep = '\0';

		unsigned short port = 4307;
		// Try to find address and port separator, that is the last ':' unless it's a part of IPv6 address (located inside '[' ']')
		auto port_sep = addr_sep;
		for (; port_sep != addr_start; --port_sep)
			if (*port_sep == ':' || *port_sep == ']')
				break;
		if (*port_sep == ':')
		{
			*port_sep = '\0';
			auto port_start = port_sep + 1;
			if (port_start != addrs.end())
			{
				port = atoi(&*port_start);
				if (port == 0 && *port_start != '0') // conversion failed, restore default
					port = 4307;
			}
		}

		boost::system::error_code ec;
		const char* host = &*addr_start;
		auto addr = boost::asio::ip::address::from_string(host,ec);
		if (addr.is_unspecified())
			addr = net::MakeA_lookup(host, m_ios);
		if (addr.is_unspecified())
			addr = net::MakeA_lookup(host, m_ios, true);
		if(!addr.is_unspecified())
			m_serverAddrs.emplace_back(std::move(addr),port);

		addr_start = addr_sep + 1;
	}

	VS_RegistryKey rKey(false, CONFIGURATION_KEY);
	int32_t value = 0;
	if (rKey.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "TransceiverParticipantTLS") > 0 && value == 0)
		m_tlsEnabled = false;
	else
		m_tlsEnabled = true;
}

VS_TransceiverParticipant::~VS_TransceiverParticipant()
{
	Stop();
}

const char* VS_TransceiverParticipant::GetPartName() const
{
	return m_part_name.c_str();
}

const char* VS_TransceiverParticipant::GetConfName() const
{
	return m_conf_name.c_str();
}

bool VS_TransceiverParticipant::ConnectStream()
{
	if (!m_isStopped)
		return false;
	auto connect_lambda = [this]()
	{
		m_isStopped = false;
		// Prepare status for the first attempt
		m_connsStatus.withLock([&](ConnectionsStatus& status) {
			status.try_id = 0;
			status.inputConnectionActive = false;
			status.outputConnectionActive = false;
		});
		return TryConnect(0);
	};
	const auto conf_ep = VS_GetConfEndpoint(m_conf_name.c_str());
	if (!conf_ep || conf_ep == m_serverEP)
		return connect_lambda();

	m_serverAddrs.clear();

	auto name = net::dns::get_srv_query_by_server_id(conf_ep);

	std::vector<std::pair<std::string, net::port>> hosts;
	{
		auto default_con = net::endpoint::GetConnectTCPFromID(conf_ep);
		if (default_con.res)
			hosts.emplace_back(std::move(default_con.connect_tcp.host), default_con.connect_tcp.port);
	}

	net::dns::async_make_srv_lookup(std::move(name), m_ios.wrap([self = shared_from_this(), hosts = std::move(hosts), cb = std::move(connect_lambda)](net::dns::srv_reply_list reply, boost::system::error_code ec) mutable
	{
		if (!ec) //no error
		{
			for (auto &record : reply)
				hosts.emplace_back(std::move(record.host), record.port);
		}

		if (hosts.empty())
			return;


		struct Resolver final
		{
			typedef decltype(cb) callback_t;
			typedef decltype(self) tr_part_ptr_t;

			Resolver(tr_part_ptr_t &&trPartObj, callback_t &&connCb) 
				: trPart(std::move(trPartObj))
				, strand(trPart->m_ios)
				, m_cb(std::move(connCb)) 
				{}
			~Resolver() { (void)m_cb(); }

			tr_part_ptr_t trPart;
			boost::asio::io_service::strand strand;
		private:
			callback_t m_cb;
		};

		auto res = std::make_shared<Resolver>(std::move(self), std::move(cb));

		for (auto &host : hosts)
		{
			net::dns::async_make_a_aaaa_lookup(std::move(host.first), res->strand.wrap([res, port = host.second](net::dns::a_hostent_reply a, net::dns::aaaa_hostent_reply aaaa)
			{
				for (auto &item : { &a, &aaaa })
				{
					if (!item->ec)
					{
						for (const auto &addr : item->host.addrs)
							res->trPart->m_serverAddrs.emplace_back(addr, port);
					}
				}
			}));

		}}));

	return true;
}

void VS_TransceiverParticipant::Stop()
{
	if (m_isStopped)
		return;
	m_isStopped = true;

	// Stop asynchronous connect handlers from proceeding
	m_connsStatus.withLock([&](ConnectionsStatus& status) { status.try_id = static_cast<unsigned>(-1); });

#if TC_VERBOSE_LOGS
	dstream4 << "Participant(" << m_part_name << "): stop, input " << (m_in ? "present" : "missing") << ", output " << (m_out ? "present" : "missing");
#endif

	if (m_in)
	{
		m_in->m_fireNewBuffer.disconnect_all_slots();
		m_in->Shutdown();
		m_in = nullptr;
	}
	if (m_out)
	{
		m_out->m_fireDisconnect.disconnect_all_slots();
		m_out->Shutdown();
		m_out = nullptr;
	}

	m_fireConnectionDie();
}

void VS_TransceiverParticipant::WaitForStop()
{
	//FIXME: implement
}

void VS_TransceiverParticipant::SendFrame(const unsigned char *buf, const unsigned long size, stream::Track track)
{
	if (m_isStopped)
		return;
	if (size > 0xffff)
		return;

	vs::SharedBuffer frame(sizeof(stream::FrameHeader) + size);
	std::memcpy(frame.data<char>() + sizeof(stream::FrameHeader), buf, size);
	auto& sendFrame = *frame.data<stream::FrameHeader>();
	sendFrame.length = (unsigned short)size;
	sendFrame.track = track;
	sendFrame.cksum = stream::GetFrameBodyChecksum(buf, size);

	SendOrSaveBuffer(std::move(frame));
}

void VS_TransceiverParticipant::SendFrame(vs::SharedBuffer&& buffer)
{
	if (m_isStopped)
		return;
	SendOrSaveBuffer(std::move(buffer));
}

void VS_TransceiverParticipant::SendOrSaveBuffer(vs::SharedBuffer&& frame)
{
	const bool handshake_completed = m_in && m_out && m_in->IsHandshakeDone() && m_out->IsHandshakeDone();

	if (handshake_completed == false || !m_out_buffer.empty())
	{
		auto *frame_data = reinterpret_cast<uint8_t *>(frame.data());
		auto frame_size = frame.size();

		if (maxWriteBuffSize < m_out_buffer.size() + frame_size) {
			dstream4 << "VS_TransceiverParticipant Error: write buffer overflow! Dropping frames!\n";
		}
		else {
			m_out_buffer.resize(m_out_buffer.size() + frame_size);
			std::memcpy(&m_out_buffer[m_out_buffer.size() - frame_size], &frame_data[0], frame_size);
		}

	}

	if (handshake_completed)
	{
		if (m_out_buffer.empty())
		{
			m_out->Send(std::move(frame));
		}
		else
		{
			vs::SharedBuffer data(m_out_buffer.size());
			std::memcpy(data.data<uint8_t>(), &m_out_buffer[0], m_out_buffer.size());
			m_out_buffer.clear();
			m_out->Send(std::move(data));
		}
	}
}

void VS_TransceiverParticipant::OnNewBuffer(const vs::SharedBuffer& buffer)
{
	const auto& f = *buffer.data<const stream::FrameHeader>();
	const void* payload = buffer.data<const char>() + sizeof(stream::FrameHeader);
	const size_t payload_size = buffer.size() - sizeof(stream::FrameHeader);

	switch (f.track)
	{
	case stream::Track::command:
	{
		stream::Command cmd(payload, payload_size);
		if (cmd.Size() > payload_size)
			dstream4 << "Participant(" << m_part_name << "): truncated command: real size: " << payload_size << ", expected size: " << cmd.Size();

		if     (cmd.type == stream::Command::Type::ChangeRcvMFormat)
		{
			auto& mf = *reinterpret_cast<VS_MediaFormat*>(cmd.data);
			if      (cmd.sub_type == stream::Command::Request)
				m_fireChangeRcvFormatRequest(mf, true);
			else if (cmd.sub_type == stream::Command::Info)
				m_fireChangeRcvFormatRequest(mf, false);
			else if (cmd.sub_type == stream::Command::Reply /*&& cmd.result == stream::Command::OK*/) // apparently client always returns error status
				m_fireChangeRcvFormatReply(mf);
		}
		else if (cmd.type == stream::Command::Type::ChangeSndMFormat)
		{
			auto& mf = *reinterpret_cast<VS_MediaFormat*>(cmd.data);
			if (cmd.sub_type == stream::Command::Request)
				m_fireChangeSndFormatRequest(mf);
		}
		else if (cmd.type == stream::Command::Type::RestrictBitrate)
		{
			unsigned int bitrate = *reinterpret_cast<uint32_t*>(cmd.data);
			if (cmd.sub_type == stream::Command::Request)
				m_fireSetBitrateRequest(bitrate);
		}
		else if (cmd.type == stream::Command::Type::RequestKeyFrame)
			m_fireKeyFrameRequest();
		else if (static_cast<int>(cmd.type) > stream::Command::last_type)
			dstream4 << "Participant(" << m_part_name << "): unknown command: type=" << static_cast<int>(cmd.type) << ", sub_type=" << static_cast<int>(cmd.sub_type) << ", result=" << static_cast<int>(cmd.result) << ", data_size=" << static_cast<int>(cmd.data_size);
		break;
	}
	case stream::Track::old_command:
		// Type of the command depends on size:
		//   0  - ping
		//   1  - key frame request
		//   >1 - broker statistics (stream::StreamStatistics)
		if (payload_size == 1)
			m_fireKeyFrameRequest();
		break;
	default:
		m_fireNewFrame(buffer);
		break;
	}
}

bool VS_TransceiverParticipant::TryConnect(unsigned try_id)
{
#if !defined(NDEBUG)
	m_connsStatus.withLock([&](ConnectionsStatus& status) { assert(try_id == status.try_id); });
#endif
	assert(!m_isStopped);

	if (try_id == 0)
	{
		// On the first attempt we try to connect to the first address.
		m_tryingTls = m_tlsEnabled;
	}
	else if (m_tryingTls)
	{
		// If last connect attempt was via TLS, try the same address without TLS.
		assert(m_tlsEnabled);
		m_tryingTls = false;
	}
	else
	{
		// Otherwise try the next address.
		// If there are no more arrdesses we will exit soon, just have to not crash before that.
		if (!m_serverAddrs.empty())
			m_serverAddrs.erase(m_serverAddrs.begin());
		// If TLS is enabled try it first.
		m_tryingTls = m_tlsEnabled;
	}

	if (m_serverAddrs.empty())
	{
		dstream2 << "Participant(" << m_part_name << "): could not connect to any address, tried " << try_id << " addresses";
		Stop();
		return false;
	}

#if TC_VERBOSE_LOGS
	dstream4 << "Participant(" << m_part_name << "): connect #" << try_id << ": starting, address=" << (m_tryingTls ? "TLS" : "TCP") << ':' << m_serverAddrs[0];
#endif

	// Close connections from previous attempt, otherwise they would remain open until the process exit.
	if (m_in)
	{
		m_in->Shutdown();
		m_in = nullptr;
	}
	if (m_out)
	{
		m_out->Shutdown();
		m_out = nullptr;
	}

	auto on_in_connect = [try_id, self_weak = weak_from_this()](const boost::system::error_code& /*ec*/, std::shared_ptr<VS_StreamBufferedConnection> connection)
	{
		if (auto self = self_weak.lock())
			self->OnConnect(try_id, std::move(connection), true);
	};
	auto on_out_connect = [try_id, self_weak = weak_from_this()](const boost::system::error_code& /*ec*/, std::shared_ptr<VS_StreamBufferedConnection> connection)
	{
		if (auto self = self_weak.lock())
			self->OnConnect(try_id, std::move(connection), false);
	};

	if (m_tryingTls)
	{
		VS_StreamBufferedConnection::Create<net::tls>(m_ios, m_serverAddrs[0], maxWriteBuffSize, maxReadBuffSize, std::move(on_in_connect));
		VS_StreamBufferedConnection::Create<net::tls>(m_ios, m_serverAddrs[0], maxWriteBuffSize, maxReadBuffSize, std::move(on_out_connect));
	}
	else
	{
		VS_StreamBufferedConnection::Create<boost::asio::ip::tcp>(m_ios, m_serverAddrs[0], maxWriteBuffSize, maxReadBuffSize, std::move(on_in_connect));
		VS_StreamBufferedConnection::Create<boost::asio::ip::tcp>(m_ios, m_serverAddrs[0], maxWriteBuffSize, maxReadBuffSize, std::move(on_out_connect));
	}
	return true;
}

void VS_TransceiverParticipant::OnConnect(unsigned try_id, std::shared_ptr<VS_StreamBufferedConnection>&& connection, bool is_input)
{
	VS_SCOPE_EXIT {
		// Close connection in case we didn't use it, otherwise it would remain open until the process exit.
		if (connection)
			connection->Shutdown();
	};
	const bool success = connection != nullptr;
	bool reject = false;
	bool completed = false;
	m_connsStatus.withLock([&](ConnectionsStatus& status) {
		if (try_id != status.try_id)
			reject = true; // Other connection initiated a new connection attempt
		else if (success)
		{
			if (is_input)
			{
				assert(!m_in);
				m_in = std::move(connection);
				status.inputConnectionActive = true;
			}
			else
			{
				assert(!m_out);
				m_out = std::move(connection);
				status.outputConnectionActive = true;
			}
			completed = status.inputConnectionActive && status.outputConnectionActive;
		}
		else
		{
			// Prepare status for the next attempt
			status.try_id += 1;
			status.inputConnectionActive = false;
			status.outputConnectionActive = false;
		}
	});
#if TC_VERBOSE_LOGS
	dstream4 << "Participant(" << m_part_name << "): connect #" << try_id << ": " << (is_input ? "input" : "output") << std::boolalpha << ": success=" << success << ", reject=" << reject << ", completed=" << completed;
#endif
	if (reject)
		return;
	if (completed)
	{
		assert(m_in);
		assert(m_out);

		m_in->m_fireNewBuffer.connect([this](const vs::SharedBuffer& buffer) { OnNewBuffer(buffer); });
		m_out->m_fireDisconnect.connect([this]() { Stop(); });

		m_in->StartHandshake(stream::ClientType::receiver, m_serverEP, m_conf_name, m_part_name);
		m_out->StartHandshake(stream::ClientType::sender, m_serverEP, m_conf_name, m_part_name);
	}
	else if (!success)
		TryConnect(try_id + 1); // We are the first connection from the pair to fail, so we initiate the next attempt.
}