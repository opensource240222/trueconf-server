#include "VS_TorrentService.h"

#include <string>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include "libtorrent/aux_/escape_string.hpp"
#include "libtorrent/sha1_hash.hpp"
#include "libtorrent/bencode.hpp"

#include "net/DNSUtils/VS_DNS.h"
#include "net/EndpointRegistry.h"

#include "std/cpplib/md5.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/hton.h"
#include "std-generic/cpplib/ignore.h"
#include "std-generic/cpplib/VS_Container_io.h"
#include "std-generic/compat/iterator.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/MakeShared.h"

#include "tools/Server/VS_ServerComponentsInterface.h"

#include "VS_TRStorageInterface.h"
#include "FileTransfer/utils.h"
#include "FakeClient/VS_FakeClientManager.h"

#include "statuslib/VS_ResolveServerFinder.h"

#include "BaseServer/Services/storage/VS_DBStorageInterface.h"
#include "BaseServer/Services/VS_BasePresenceService.h"

#define DEBUG_CURRENT_MODULE VS_DM_FILETRANSFER

namespace {
    const auto _2_GB = (2LL * 1024 * 1024 * 1024);

	constexpr std::chrono::minutes DEFAULT_CHECK_USELESS_TIMEOUT(30); //30 min
	constexpr std::chrono::minutes TRY_DELETE_TIMEOUT(1); //1 min

	constexpr std::chrono::minutes TRACKER_ENDPOINT_TTL(10); // 10 minutes
	constexpr std::chrono::seconds TIMEOUT_INTERVAL(10); //10 sec

	constexpr std::chrono::seconds STOPPED_INTERVAL(3600);
	constexpr std::chrono::seconds STOPPED_MIN_INTERVAL(3600);

	const net::port DEFAULT_PORT = 4307;

	constexpr char SIGNATURE[] = "\x13" "BitTorrent protocol";	// start of handshake message <pstrlen><pstr>...

	constexpr std::size_t MAX_BUFFER_PROXY_SIZE = 16384;		// 16KiB
	constexpr std::chrono::seconds TIMEOUT_PROXY(15);
	const net::address TORRENT_PROXY_ADDR(net::address_v4::loopback());

	//http://dandylife.net/docs/BitTorrent-Protocol.pdf
	enum tracker_errors
	{
		//not complete list
		missing_info_hash	= 101,
		missing_peer_id		= 102,
		missing_port		= 103,
		invalid_infohash	= 150,
		invalid_peerid		= 151,
		no_error			= 200,
		generic_error       = 900
	};

	inline const char *tracker_strerror(tracker_errors err) noexcept
	{
		switch(err)
		{
		case tracker_errors::missing_info_hash	: return "Missing info_hash";
		case tracker_errors::missing_peer_id	: return "Missing peer_id";
		case tracker_errors::missing_port		: return "Missing port";
		case tracker_errors::invalid_infohash	: return "Invalid infohash";
		case tracker_errors::invalid_peerid		: return "Invalid peerid";
		case tracker_errors::no_error			: return "OK";
		case tracker_errors::generic_error		: return "Generic error";
		}
		assert(NULL);
		return NULL;
	}

	inline void parse_tracker_request(string_view in, tracker_errors &errorCode,
		std::string& infoHash, net::port& port, std::string& peerId, std::string& evt, bool& noPeerId) noexcept
	{
		string_view sv(in);
		auto line_end = sv.find(string_view("\r\n"));
		if (line_end == string_view::npos)
		{
			errorCode = tracker_errors::generic_error;
			return;
		}
		sv.remove_suffix(sv.length() - line_end);

		// parse url and get key-value array
		std::vector<std::pair<string_view, string_view>> kv;
		kv = VS_ParseUrlParams(sv);

		// info_hash
		libtorrent::error_code ec;
		auto it = std::find_if(kv.begin(), kv.end(), [](const std::pair<string_view, string_view>& p) {
			return p.first == string_view("info_hash");
		});
		if (it == kv.end() || it->second.empty())
		{
			errorCode = tracker_errors::missing_info_hash;
			return;
		}
		std::string _h = libtorrent::unescape_string(std::string(it->second), ec);
		if (ec || _h.size() != 20)
		{
			errorCode = tracker_errors::invalid_infohash;
			return;
		}
		infoHash = libtorrent::hash_to_string(libtorrent::sha1_hash(_h));

		// port
		it = std::find_if(kv.begin(), kv.end(), [](const std::pair<string_view, string_view>& p) {
			return p.first == string_view("port");
		});
		if (it == kv.end() || it->second.empty())
		{
			errorCode = tracker_errors::missing_port;
			return;
		}
		const int tmp_p = ::atoi(std::string(it->second).c_str());
		if (tmp_p <= 0 || tmp_p > std::numeric_limits<net::port>::max())
		{
			errorCode = tracker_errors::missing_port;
			return;
		}
		port = static_cast<net::port>(tmp_p);

		// peer_id
		it = std::find_if(kv.begin(), kv.end(), [](const std::pair<string_view, string_view>& p) {
			return p.first == string_view("peer_id");
		});
		if (it == kv.end() || it->second.empty())
		{
			errorCode = tracker_errors::missing_peer_id;
			return;
		}
		peerId = libtorrent::unescape_string(std::string(it->second), ec);
		if (ec || peerId.size() != 20)
		{
			errorCode = tracker_errors::invalid_peerid;
			return;
		}

		// event
		it = std::find_if(kv.begin(), kv.end(), [](const std::pair<string_view, string_view>& p) {
			return p.first == string_view("event");
		});
		if (it != kv.end() && !it->second.empty())
			evt = std::string(it->second);

		// TODO: compact
		//it = std::find_if(kv.begin(), kv.end(), [](const std::pair<string_view, string_view>& p) {
		//	return p.first == string_view("compact");
		//});
		//
		//if (it != kv.end() && !it->second.empty())
		//	compact = (::atoi(std::string(it->second).c_str()) != 0);

		// no_peer_id
		it = std::find_if(kv.begin(), kv.end(), [](const std::pair<string_view, string_view>& p) {
			return p.first == string_view("no_peer_id");
		});

		if (it != kv.end() && !it->second.empty())
			noPeerId = (::atoi(std::string(it->second).c_str()) != 0);

		errorCode = tracker_errors::no_error;
	}

	inline std::uintmax_t available_storage_space(const std::string &path) noexcept
	{
		boost::system::error_code ec;
		const boost::filesystem::path p{ path };

		if (!boost::filesystem::exists(p, ec))
			return 0;

		return boost::filesystem::space(p, ec).available;
	}

	inline std::pair<unsigned int, int> libtorrent_settings() noexcept
	{
		std::pair<unsigned int, int> info;
		info.second = VS_FileTransfer::NAT_PMP | VS_FileTransfer::UPNP | VS_FileTransfer::LockFiles;
		int flag = 0;
		VS_RegistryKey reg(false, CONFIGURATION_KEY);
		if (reg.GetValue(&flag, sizeof(flag), VS_REG_INTEGER_VT, TR_DHT_KEY) > 0) {
			info.second = flag ? (info.second | VS_FileTransfer::DHT) : (info.second & ~VS_FileTransfer::DHT);
		}
		if (reg.GetValue(&flag, sizeof(flag), VS_REG_INTEGER_VT, TR_LSD_KEY) > 0) {
			info.second = flag ? (info.second | VS_FileTransfer::LSD) : (info.second & ~VS_FileTransfer::LSD);
		}
		if (reg.GetValue(&flag, sizeof(flag), VS_REG_INTEGER_VT, TR_NAT_PMP_KEY) > 0) {
			info.second = flag ? (info.second | VS_FileTransfer::NAT_PMP) : (info.second & ~VS_FileTransfer::NAT_PMP);
		}
		if (reg.GetValue(&flag, sizeof(flag), VS_REG_INTEGER_VT, TR_UPNP_KEY) > 0) {
			info.second = flag ? (info.second | VS_FileTransfer::UPNP) : (info.second & ~VS_FileTransfer::UPNP);
		}
		if (reg.GetValue(&flag, sizeof(flag), VS_REG_INTEGER_VT, TR_LOCK_FILES_KEY) > 0) {
			info.second = flag ? (info.second | VS_FileTransfer::LockFiles) : (info.second & ~VS_FileTransfer::LockFiles);
		}
		if(reg.GetValue(&info.first, sizeof(info.first), VS_REG_INTEGER_VT, TR_FILE_POLL_SIZE) <= 0) {
			info.first = ~0u;
		}
		return info;
	}

	inline std::chrono::seconds load_check_useless_timeout() noexcept
	{
		VS_RegistryKey reg(false, CONFIGURATION_KEY);
		unsigned data;
		return (reg.GetValue(&data, sizeof(data), VS_REG_INTEGER_VT, TR_CHECK_USELESS_TIMEOUT) > 0 && data > 0 ? std::chrono::seconds(data) : DEFAULT_CHECK_USELESS_TIMEOUT);
	}

	inline  std::string get_preview_file_name(const std::string &name) noexcept
	{
		MD5 md5;
		md5.Update(name);
		md5.Final();
		char md5_str[33];
		md5.GetString(md5_str, false);

		return std::string("preview-") + md5_str + boost::filesystem::path(name).extension().string();
	}

	template<class T, class L>
	inline std::string get_trackers(T &listenAddrs, string_view ourEndpoint, L &&trackersSeq) noexcept
	{
		VS_RegistryKey	reg(false, "AppProperties", false, true);
		if (reg.IsValid()) {
			std::string trackers;
			if (reg.GetString(trackers, TR_TRACKERS_KEY) && !trackers.empty()) {
				// Server already has custom trackers property set
				return {};
			}
		}

		// Add addresses from Server\Endpoints\<serverName>\ConnectTCP<#>
		const unsigned n_ctcp = net::endpoint::GetCountConnectTCP(ourEndpoint, true);
		for (unsigned i = 1; i <= n_ctcp; ++i) {
			if (auto tcp = net::endpoint::ReadConnectTCP(i, ourEndpoint, true)) {
				trackersSeq.emplace(std::move(tcp->host), tcp->port);
			}
		}

		// Add addresses from Server\Endpoints\<serverName>\AcceptTCP<#>
		const unsigned n_atcp = net::endpoint::GetCountAcceptTCP(ourEndpoint, true);
		for (unsigned i = 1; i <= n_atcp; ++i) {
			if (auto tcp = net::endpoint::ReadAcceptTCP(i, ourEndpoint, true)) {
				trackersSeq.emplace(std::move(tcp->host), tcp->port);
			}
		}

		std::string trackers_str;

		char port_buffer[std::numeric_limits<net::port>::digits10 + 1 + 1 /*0-terminator*/];

		const char http_buff[] = "http://";
		const char announce_buff[] = "/announce";

		for (const auto &t : trackersSeq)
		{
			const auto &host = t.first;
			net::port port = t.second;

			if (host.empty())
				continue;

			if (!trackers_str.empty())
				trackers_str.push_back(';');

			trackers_str.append(http_buff);

			if(net::is_ipv6(host))
			{
				trackers_str.push_back('[');
				trackers_str.append(host);
				trackers_str.push_back(']');
			}
			else
			{
				trackers_str.append(host);
			}

			if(port == 0)
			{
				port = DEFAULT_PORT;
			}

			trackers_str.push_back(':');
			::sprintf(port_buffer, "%u", port);
			trackers_str.append(port_buffer).append(announce_buff);
		}

		listenAddrs = std::forward<L>(trackersSeq);

		return trackers_str;
	}

	namespace proxy {
		namespace tcp {

			typedef boost::asio::ip::tcp::socket socket_t;

			class Session : public vs::enable_shared_from_this<Session>
			{
			public:
				typedef std::shared_ptr<Session>					self_t;
				typedef std::vector<uint8_t>						stream_buffer_t;
				typedef std::chrono::steady_clock					clock_t;
				typedef clock_wrapper<clock_t>						clock_wrapper_t;
				typedef boost::asio::basic_waitable_timer<clock_t>	timer_t;

				struct Info final
				{
					Info(boost::asio::io_service &ios, std::size_t buffMaxSize)
						: sock(ios)
						, buffMaxSize(buffMaxSize)
					{
					}

					socket_t sock;
					stream_buffer_t buffer;
					const std::size_t buffMaxSize;
				};

				void Start(socket_t &&clientSock, stream_buffer_t &&clientBuffer, const socket_t::endpoint_type &serverEnp)
				{
					assert(clientSock.is_open());

					m_client.sock = std::move(clientSock);
					m_client.buffer = std::move(clientBuffer);

					Start(serverEnp);
				}

				void Stop() noexcept
				{
					if (!m_stopped.exchange(true, std::memory_order_acq_rel))
					{
						m_timeoutTimer.cancel(vs::ignore<boost::system::error_code>());

						//For portable behaviour with respect to graceful closure of a connected socket, call shutdown() before closing the socket.
						//https://www.boost.org/doc/libs/1_43_0/doc/html/boost_asio/reference/basic_stream_socket/close/overload2.html

						m_client.sock.shutdown(boost::asio::socket_base::shutdown_both, vs::ignore<boost::system::error_code>());
						m_server.sock.shutdown(boost::asio::socket_base::shutdown_both, vs::ignore<boost::system::error_code>());

						m_client.sock.close(vs::ignore<boost::system::error_code>());
						m_server.sock.close(vs::ignore<boost::system::error_code>());
					}
				}

				bool IsStopped() const noexcept
				{
					return m_stopped.load(std::memory_order_acquire);
				}

			protected:
				Session(boost::asio::io_service &ios, std::size_t clientBufSize, std::size_t serverBufSize, clock_t::duration timeout)
					: m_server(ios, serverBufSize == 0 || serverBufSize > 0x10000 ? 1024 : serverBufSize)
					, m_client(ios, clientBufSize == 0 || clientBufSize > 0x10000 ? 1024 : clientBufSize)
					, m_timeoutTimer(ios)
					, m_timeout(timeout)
					, m_lastActive(), m_stopped(true)
				{
					assert(m_client.buffMaxSize != 0 && m_client.buffMaxSize <= 0x10000);
					assert(m_server.buffMaxSize != 0 && m_server.buffMaxSize <= 0x10000);
				}

				void Start(const socket_t::endpoint_type &serverEp)
				{
					m_server.sock.set_option(boost::asio::ip::tcp::socket::reuse_address(true), vs::ignore<boost::system::error_code>{});

					m_stopped.store(false, std::memory_order_release);

					m_server.sock.async_connect(serverEp,
						[self = shared_from_this()](const boost::system::error_code &ec)
					{
						if (!ec) // no error
						{
							self->SetLastActive();

							self->Init(self->m_client, self->m_server);
							self->Init(self->m_server, self->m_client);
						}
						else
						{
							self->Stop();
						}
					});

					SetTimeout();
				}

				clock_wrapper_t &clock() const noexcept { return m_clock; }

			private:
				inline void PrepareBuffer(Info &info) const
				{
					if (info.buffer.size() < info.buffMaxSize)
					{
						auto available = info.sock.available(vs::ignore<boost::system::error_code>{});
						available = available != 0 ? available : 1024;

						if (available > info.buffer.size())
						{
							info.buffer.resize(std::min(info.buffMaxSize, std::max(available, info.buffer.size() * 3 / 2)));
						}
					}
				}

				inline void Init(Info &infoFrom, Info &infoTo)
				{
					assert(m_lastActive.load(std::memory_order_acquire) != clock_t::duration{});

					const auto byte_trans = infoFrom.buffer.size();

					if (byte_trans == 0)
					{
						HandleWrite(boost::system::error_code{}, infoFrom, infoTo);
					}
					else
					{
						HandleRead(boost::system::error_code{}, byte_trans, infoFrom, infoTo);
					}
				}

				inline void SetLastActive() noexcept
				{
					m_lastActive.store(clock().now().time_since_epoch(), std::memory_order_release);
				}

				void HandleRead(const boost::system::error_code &ec, const size_t bytesTransferred, Info &infoFrom, Info &infoTo)
				{
					assert(m_lastActive.load(std::memory_order_acquire) != clock_t::duration{});

					if (!ec)
					{
						boost::asio::async_write(infoTo.sock,
							boost::asio::buffer(infoTo.buffer, bytesTransferred),
							[self = shared_from_this(), &infoFrom, &infoTo](const boost::system::error_code &ec, std::size_t /*bytesTransferred*/)
						{
							self->SetLastActive();
							self->HandleWrite(ec, infoFrom, infoTo);
						});
					}
					else
					{
						Stop();
					}
				}

				void HandleWrite(const boost::system::error_code &ec, Info &infoFrom, Info &infoTo)
				{
					assert(m_lastActive.load(std::memory_order_acquire) != clock_t::duration{});

					if (!ec)
					{
						PrepareBuffer(infoFrom);

						infoFrom.sock.async_read_some(
							boost::asio::buffer(vs::data(infoFrom.buffer), infoFrom.buffer.size()),
							[self = shared_from_this(), &infoFrom, &infoTo](const boost::system::error_code &ec, std::size_t bytesTransferred)
							{
								self->SetLastActive();
								self->HandleRead(ec, bytesTransferred, infoFrom, infoTo);
							}
						);
					}
					else
					{
						Stop();
					}
				}

				void SetTimeout()
				{
					if (m_timeout != decltype(m_timeout)())
					{
						 m_timeoutTimer.expires_from_now(m_timeout); //TODO:throw ?!
						 m_timeoutTimer.async_wait(
						 	[self = shared_from_this()](const boost::system::error_code &ec)
						 {
							 if (ec)
							 {
								 return;
							 }

							if (self->clock().now().time_since_epoch() >= self->m_lastActive.load(std::memory_order_acquire))
							{
								self->Stop();
								return;
							}

							self->SetTimeout();

						 });
					}
				}

			private:
				Info m_server;
				Info m_client;
				timer_t m_timeoutTimer;
				mutable clock_wrapper_t m_clock;
				const clock_t::duration m_timeout;
				std::atomic<clock_t::duration> m_lastActive;
				std::atomic_bool m_stopped;
			};

		} // namespace tcp_proxy
	}//namespace proxy


}

VS_TorrentService::VS_TorrentService(boost::asio::io_service &ios, const std::string &ourEndpoint,
									 const std::string &fileStorageDir,
									 const std::shared_ptr<VS_TRStorageInterface> &dbStorage,
									 const std::function<std::string(const char *)> &getServerId)
	: m_init(false)
	, m_ios(ios)
	, m_dbStorage(dbStorage)
	, m_ourEndpoint(ourEndpoint)
	, m_fileStorageDir(fileStorageDir)
	, m_getServerId(getServerId)
	, m_lastCheckTime(0)
	, m_lastCleanTime(0)
	, m_lastDeleteTime(0)
	, m_checkUselessTimeout(load_check_useless_timeout())
	, m_cacheListenPort(0)
{
	m_TimeInterval = TIMEOUT_INTERVAL;
}

void VS_TorrentService::PostConstruct(std::shared_ptr<VS_TorrentService>& ptr)
{
	auto info = libtorrent_settings();
	ptr->m_torrent = VS_FileTransfer::MakeFileTransfer(ptr->m_fileStorageDir.c_str(), info.first, info.second, ptr);
}

boost::optional<std::string> VS_TorrentService::ProcessTrackerRequest(string_view in, const net::address &fromAddr)
{
	std::string info_hash, peer_id, evt;
	tracker_errors error_code;
	net::port port(0);
	bool no_peer_id(false);

	parse_tracker_request(in, error_code, info_hash, port, peer_id, evt, no_peer_id);

	if(error_code != tracker_errors::no_error) //parse error
	{
		return { "HTTP/1.1 " + std::to_string(error_code) + " " + tracker_strerror(error_code) + "\r\n\r\n" };
	}

	//VS_IPPortAddress from_copy(from);
	// todo(ktrushnikov): get IP from handler
	//from_copy.port(port);
	return  { MakeTrackerResponse(info_hash, peer_id, evt, fromAddr, port, no_peer_id) };
}

bool VS_TorrentService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess )
{
	if (recvMess == nullptr)
		return true;

	VS_Container cnt;

	if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize()))
	{
		const char* method = cnt.GetStrValueRef(METHOD_PARAM);
		m_recvMess = recvMess.get();
		dstream3 << "Method: " << method << "; from_srv: " <<
			recvMess->SrcServer() << "; fromuser: " <<
			recvMess->SrcUser();
		if (method && strcasecmp(method, SHARE_METHOD) == 0) {
			Share_Method(cnt);
		}
	}
	return true;
}

void VS_TorrentService::Share_Method(VS_Container &cnt) {
	const char *uri = cnt.GetStrValueRef(MAGNET_LINK_PARAM);
	const char *from = m_recvMess->SrcUser(); // only authorized from user

	std::string info_hash = VS_FileTransfer::hash_str_from_uri(uri);

	if (!from) {
		return;
	}

	if (!uri) {
		// Cannot share file without magnet, so send error message
		SendError(from, TR_MESSAGE_WITHOUT_MAGNET);
		return;
	}
	dstream4 << "Share_Method: from = " <<
		from << "; recv cnt: " <<
		cnt;
	// ----------------------Setup HTTP-links----------------------

	std::vector<DownloadEntry> entries;
	std::string id_param;
	std::size_t num_ids = 0;
	cnt.Reset();
	while (cnt.Next()) {
		if (!strcmp(cnt.GetName(), FILENAME_PARAM)) {
			entries.emplace_back(cnt.GetStrValueRef());
		} else if (!strcmp(cnt.GetName(), ID_PARAM)) {
			id_param = cnt.GetStrValueRef();
			num_ids++;
		}
	}

	if (entries.empty() || num_ids != entries.size()) {
		SendError(from, TR_INVALID_FILELIST);
		return;
	}

	std::string http_link;
	VS_RegistryKey reg(false, CONFIGURATION_KEY);
	if (reg.GetString(http_link, TR_HTTP_DOWNLOAD_PREFIX_KEY) && !http_link.empty()) {
		if (http_link.back() != '/') {
			http_link += '/';
		}
		http_link += info_hash;
	}

	{
		std::lock_guard<decltype(m_stateLock)> lock{ m_stateLock };
		auto it = m_state.find(info_hash);
		std::shared_ptr<TorrentState> t;
		if (it == m_state.end()) {
			t = std::make_shared<TorrentState>(info_hash);
			m_state[info_hash] = t;
		} else {
			t = it->second;
		}
		t->magnet = uri;
		t->lastInitiator = VS_RealUserLogin(from);
		t->entries = entries;
		t->httpLink = http_link;
		t->idParam = id_param;
		t->isAccepted = true;
		t->lastChanged = clock().now();
	}

	// ----------------------Save preview image----------------------

	size_t bufsize;
	const void *buf = cnt.GetBinValueRef(PREVIEW_BODY_PARAM, bufsize);
	const char *type = cnt.GetStrValueRef(PREVIEW_TYPE_PARAM);
	if (buf && type && bufsize != 0) {
		std::string storage_path = FileStorageDir();
		storage_path += info_hash + char(boost::filesystem::path::preferred_separator);

		boost::filesystem::path p = { storage_path };
		boost::system::error_code ec;
		boost::filesystem::create_directories(p, ec);
		if (ec) {
			dstream0 << "Share_Method: Error creating '" << storage_path << "': " << ec.message();
			SendError(from, TR_UNKNOWN_ERROR);
			return;
		}

		storage_path += get_preview_file_name(entries[0]);

		p = { storage_path };
		if (!boost::filesystem::exists(p, ec)) {
			std::ofstream out_file(storage_path, std::ios::binary);
			out_file.write(static_cast<const char *>(buf), bufsize);
		}
	}

	// ----------------------Add torrent to downloads----------------------
	dstream4 << "Share_Method: StartDownload(" << uri << ", " << from << ")";
	m_torrent->StartDownload(std::string(uri), std::string(from));
}

void VS_TorrentService::SendError(const char *to, VS_TorrentResult res) {
	VS_Container answer_cnt;
	dstream3 << "SendError: to = " << to << ", res = " << res;
	answer_cnt.AddValue(METHOD_PARAM, SHARE_METHOD);
	answer_cnt.AddValueI32(RESULT_PARAM, res);

	PostRequest(to, answer_cnt);
}

void VS_TorrentService::onReadyToSend( const std::string &magnet, const std::string &to )
{
	// nothing to do
	dstream4 << "VS_TorrentService::onReadyToSend: magnet = " <<
		magnet << "; to = " <<
		to;
}

void VS_TorrentService::onPeerConnected( const std::string &infoHash, const VS_FileTransfer::Endpoint &endpoint )
{
	dstream4 << "VS_TorrentService::onPeerConnected: info_hash = " <<
		infoHash << "; endpoint = " <<
		endpoint.ip << ":" <<
		endpoint.port;
	// nothing to do
}

bool VS_TorrentService::onAskUser( const std::string &infoHash, const std::string &filename, const std::string &to, uint64_t size)
{
	using namespace boost::filesystem;
	dstream4 << "onAskUser: info_hash = " <<
		infoHash << ", filename = " <<
		filename << ", to = " <<
		to;

	VS_TorrentResult result = TR_NO_ERROR;

	auto free_space = available_storage_space(FileStorageDir());

    if (!size) {
		dstream0 << "Error in the file size";
        result = TR_UNKNOWN_ERROR;
    }

	if (result == TR_NO_ERROR && (free_space - size) < _2_GB) {
		dstream0 << "Not enough space to download file.";
		result = TR_P2P_ONLY;
	}

	VS_Container answer_cnt;
	answer_cnt.AddValue(METHOD_PARAM, SHARE_METHOD);

	VS_RealUserLogin initiator(to);
	std::string magnet;

	{
		std::lock_guard<decltype(m_stateLock)> lock{ m_stateLock };
		auto it = m_state.find(infoHash);
		if (it != m_state.cend()) {
			auto t = it->second;
			magnet = t->magnet;

			answer_cnt.AddValue(ID_PARAM, t->idParam.c_str());
			if (!t->httpLink.empty()) {
				answer_cnt.AddValue(URL_PARAM, t->httpLink.c_str());
			}
		} else {
			result = TR_UNKNOWN_ERROR;
		}
	}
	answer_cnt.AddValueI32(RESULT_PARAM, result);
	answer_cnt.AddValue(MAGNET_LINK_PARAM, magnet.c_str());

	// ----------------------Send response----------------------

	PostRequest(initiator.GetID(), answer_cnt);

	if (result != TR_NO_ERROR)
	{
		std::string preview_file_dir = FileStorageDir() + infoHash + char(path::preferred_separator);
		std::string preview_file_path = preview_file_dir + get_preview_file_name(VS_FileTransfer::name_from_uri(magnet));
		boost::system::error_code ec;
		boost::filesystem::remove(preview_file_path, ec);
		if (boost::filesystem::is_empty(preview_file_dir, ec)) {
			boost::filesystem::remove(preview_file_dir, ec);
		}

		return false;
	}

	return true;
}

void VS_TorrentService::onMetadataSignal(const std::string &infoHash, const std::string &to)
{
	VS_FileTransfer::Info stats{};
	m_torrent->GetControl(infoHash).GetStatistics(stats);
	if (!stats.totalWanted) {
		dstream0 << "Metainfo was not received";
		return;
	}

	std::string owner = VS_RealUserLogin{ to }.GetUser();
	std::string magnet;

	{
		std::lock_guard<decltype(m_stateLock)> lock{ m_stateLock };
		auto it = m_state.find(infoHash);
		if (it != m_state.cend()) {
			auto t = it->second;
			magnet = t->magnet;
			if (owner.empty())
			{
				owner = t->lastInitiator;
			}
		}
	}

	// ----------------------Write to db----------------------

	m_dbStorage->CreateUploadRecord(owner, infoHash, magnet, stats.totalWanted);
	for (auto& f : stats.files) {
		m_dbStorage->CreateFileRecord(infoHash, f.name, f.size);
	}
}

void VS_TorrentService::onError(const std::string &infoHash, VS_FileTransfer::eErrorCode error) {
	dstream4 << "VS_TorrentService::onError: info_hash = " <<
		infoHash << "; error = " <<
		error;
	int file_errors;
	{
		std::lock_guard<decltype(m_stateLock)> lock{ m_stateLock };
		auto it = m_state.find(infoHash);
		if (it != m_state.cend()) {
			file_errors = it->second->fileErrors;
		} else {
			dstream3 << "Unknown torrent!";
			return;
		}
	}

	if (error == VS_FileTransfer::FileError) {
		dstream3 << "File read/write error on torrent " << infoHash << "!";
		if (file_errors > 100) {
			m_dbStorage->MarkDeleteUpload(infoHash);
		}
	}
}

bool VS_TorrentService::Timer(std::chrono::milliseconds tickcount)
{
	if(!m_init)
	{
		// Resume downloads from database
		ResumeDownloads();
		// Mark old downloads
		MarkUselessEntities();
		// Remove downloads with deleteFiles flag set to 1
		CheckUselessEntities();
		RemoveUselessEntities();

		m_init = true;
	}

	const auto is_out_of_time = [this, start = clock().now()]() noexcept -> bool {
		const auto diff = std::chrono::duration_cast<std::chrono::seconds>(this->clock().now() - start);
		const bool res = diff >= std::chrono::seconds(10);
		if (res)
			dstream4 << "Timer() took long time: " << diff.count() << " seconds";
		return res;
	};

	if (tickcount - m_lastCheckTime >= std::chrono::seconds(10)) // 10 sec.
	{
		VS_SCOPE_EXIT{ m_lastCheckTime = tickcount; };
		std::vector<VS_FileTransfer::Info> infos;
		m_torrent->GetStatistics( infos );
		if (is_out_of_time())
			return true;
		CheckTorrents(infos);

		for (auto &i : infos) {
            if (i.unchanged) { continue; }

			for (auto &f : i.files) {
				if (is_out_of_time())
					return true;
				m_dbStorage->UpdateFileRecord(i.infoHash, f.name, f.done, f.downloadRate, f.uploadRate, f.numPeers);
			}

			if (is_out_of_time())
				return true;
			m_dbStorage->UpdateUploadRecord(i.infoHash, i.totalWantedDone, i.downloadRate, i.uploadRate, i.numPeers);
		}

		if (tickcount - m_lastCleanTime > m_checkUselessTimeout) { // default 30 minutes
			VS_SCOPE_EXIT{ m_lastCleanTime = tickcount; };
			MarkUselessEntities();
			if (is_out_of_time())
				return true;
			CheckUselessEntities();
		}
	}
	if (tickcount - m_lastDeleteTime > TRY_DELETE_TIMEOUT) { // 1 minute
		VS_SCOPE_EXIT{ m_lastDeleteTime = tickcount; };
		if (is_out_of_time())
			return true;
		RemoveUselessEntities();
	}
	return true;
}

void VS_TorrentService::CheckTorrents(const std::vector<VS_FileTransfer::Info>  &info) {
	std::lock_guard<decltype(m_stateLock)> lock{ m_stateLock };
	for (const auto &i : info) {
		auto it = m_state.find(i.infoHash);
		if (it != m_state.end() && it->second) {
			TorrentState &s = *it->second;

			if (i.downloadRate != 0 || i.uploadRate != 0) {
				s.lastActive = clock().now();
			}

			s.isFinished = i.isFinished;
			s.completionTime = i.completedTime;
		}
	}
}

void VS_TorrentService::MarkUselessEntities() {
	int files_lifetime_days = 1;
	{	VS_RegistryKey reg(false, CONFIGURATION_KEY);
		reg.GetValue(&files_lifetime_days, 4, VS_REG_INTEGER_VT, TR_FILES_LIFETIME);
	}

	m_dbStorage->MarkDeleteOldUploads(files_lifetime_days);

	int files_retrytime_minutes = 30;
	{	VS_RegistryKey reg(false, CONFIGURATION_KEY);
		reg.GetValue(&files_retrytime_minutes, 4, VS_REG_INTEGER_VT, TR_FILES_RETRYTIME);
	}

	std::lock_guard<decltype(m_stateLock)> lock{ m_stateLock };
	for (const auto &s : m_state) {
		const auto &t = s.second;
		if (!t->isFinished && t->completionTime &&
			clock().now() - t->lastActive > std::chrono::minutes(files_retrytime_minutes)) {
			m_dbStorage->MarkDeleteUpload(t->infoHash);
		}
	}
}

void VS_TorrentService::CheckUselessEntities() {
	auto to_delete = m_dbStorage->QueryUploadsToDelete();
	for (const auto &u : to_delete) {
		boost::system::error_code ec;
		{
			// delete torrent from tracker if exists
			std::lock_guard<decltype(m_stateLock)> lock{ m_stateLock };
			m_state.erase(u.info_hash);
		}
		// Check if folder exists
		std::string folder_path = FileStorageDir() + u.info_hash + char(boost::filesystem::path::preferred_separator);
		if (boost::filesystem::exists(boost::filesystem::path(folder_path), ec)) {
			std::string preview_file_name = get_preview_file_name(VS_FileTransfer::name_from_uri(u.magnet_link));
			boost::filesystem::remove(folder_path + preview_file_name, ec);

			// Find out size of files in folder
			boost::uintmax_t folder_size = 0;
			for (boost::filesystem::recursive_directory_iterator it(folder_path, ec);
				it != boost::filesystem::recursive_directory_iterator();
				++it) {
				if (!is_directory(*it)) {
					folder_size += file_size(*it, ec);
				}
			}
			// Check if size of folder is the same as torrent size
			// because files inside folder might have been edited/added/deleted
			if (!u.size || folder_size == u.size || folder_size == 0) {
				// Add torrent folder to deletion list
				m_notDeletedYet.push_back(u.info_hash);
			} else {
				// Folder files was changed, it is not safe to delete folder
				dstream0 << "Error deleting folder: " << folder_path << " (files were changed, delete it manually)";
			}
		} else {
			// Folder already deleted, delete from database
			m_notDeletedYet.push_back(u.info_hash);
		}
		// Remove torrent from current downloads if exists
		m_torrent->GetControl(u.info_hash).Remove(true);
	}

	/*for (auto t : m_state) {
		if (!t.second->is_accepted &&
			t.second->last_changed + std::chrono::minutes(10) < std::chrono::steady_clock::now()) {
			// Torrent was added by tracker, but wasn't accepted in share method
			if (std::find(m_notDeletedYet.begin(), m_notDeletedYet.end(), t.second->info_hash) == m_notDeletedYet.end()) {
				m_notDeletedYet.push_back(t.second->info_hash);
			}
		}
	}*/
}

void VS_TorrentService::RemoveUselessEntities() {
	for (auto it = m_notDeletedYet.begin(); it != m_notDeletedYet.end();) {
		boost::system::error_code ec;
		boost::filesystem::path folder(FileStorageDir() + (*it) + char(boost::filesystem::path::preferred_separator));
		boost::filesystem::path resume_data(FileStorageDir() + (*it) + ".resume_data");
		if (boost::filesystem::exists(folder, ec)) {
			boost::filesystem::remove_all(folder, ec);
		}
		if (boost::filesystem::exists(resume_data, ec)) {
			boost::filesystem::remove(resume_data, ec);
		}
		if (!boost::filesystem::exists(folder, ec) && !boost::filesystem::exists(resume_data, ec)) {
			// deletion was successfull,
			// delete from database now
			if (m_dbStorage->DeleteUploadRecord(*it)) {
				it = m_notDeletedYet.erase(it);
			} else {
				++it;
			}
		} else {
			++it;
		}
	}
}

void VS_TorrentService::ResumeDownloads() {
	auto &&active_list = m_dbStorage->QueryUploadsActive();
	for (auto &u : active_list) {
		std::transform(u.info_hash.begin(), u.info_hash.end(), u.info_hash.begin(), static_cast<int(*)(int)>(std::tolower));
		if (u.info_hash != VS_FileTransfer::hash_str_from_uri(u.magnet_link)) {
			dstream0 << "Invalid database record " << u.info_hash;;
			continue;
		}

		{
			std::lock_guard<decltype(m_stateLock)> lock{ m_stateLock };
			if (m_state.find(u.info_hash) == m_state.end()) {
				std::shared_ptr<TorrentState> t = std::make_shared<TorrentState>(u.info_hash);
				t->lastInitiator = VS_RealUserLogin(u.owner);
				t->lastActive = t->lastChanged = clock().now();
				t->magnet = u.magnet_link;
				t->isAccepted = true;
				t->completionTime = u.completion_time;
				m_state[u.info_hash] = t;
			}
		}
		m_torrent->StartDownload(u.magnet_link, {});
	}
}

std::string VS_TorrentService::MakeTrackerResponse(const std::string &infoHash, const std::string &peerId, const std::string &evt, const net::address &fromAddr, net::port port, bool noPeerId)
{
	std::lock_guard<decltype(m_stateLock)> lock{ m_stateLock };
	// todo(ktrushnikov): imp that check
	//if (from.ip.empty()) RETURN_ERROR( 900 );

	//from.port(port);
	bool stopped = false;
	if (evt == "stopped") {
		auto t_it = m_state.find(infoHash);
		if (t_it != m_state.end()) {
			TorrentState &t = *t_it->second;
			for (auto e = t.endpoints.begin(); e != t.endpoints.end(); ) {
				if ((e->endpoint.addr == fromAddr && e->endpoint.port == port) || (!peerId.empty() && e->peerId == peerId)) {
					e = t.endpoints.erase(e);
				} else {
					++e;
				}
			}
		}
		stopped = true;
	}

	libtorrent::entry ans{ libtorrent::entry::data_type::dictionary_t };
	ans.dict().emplace("peers", libtorrent::entry::data_type::list_t);

	auto add_endpoint = [&](const std::string & r_addr, net::port r_port, const std::string & r_peer_id)
	{
		if (!stopped && ((net::address::from_string(r_addr, vs::ignore<boost::system::error_code>{}) == fromAddr && port == r_port) || (!peerId.empty() && r_peer_id == peerId)))
		{
			return;
		}

		assert(!r_addr.empty());
		assert(r_port != 0);

		libtorrent::entry data;

		data["ip"] = r_addr;
		data["port"] = r_port;
		if (!noPeerId)
			data["peer_id"] = r_peer_id;

		ans["peers"].list().emplace_back(std::move(data));
	};

	if (!stopped)
	{
		std::shared_ptr<TorrentState> t;

		{
			auto it = m_state.find(infoHash);
			if (it == m_state.end()) {
				t = std::make_shared<TorrentState>(infoHash);
				t->lastChanged = clock().now();
				m_state[infoHash] = t;
			}
			else {
				t = it->second;
			}
		}

		TrackerRecord* rec = nullptr;

		bool found = false;
		for (std::size_t i = 0; !found && i < t->endpoints.size(); i++) {
			const bool eq_endpoint = t->endpoints[i].endpoint.addr == fromAddr && t->endpoints[i].endpoint.port == port;
			const bool eq_peer_id = !peerId.empty() && t->endpoints[i].peerId == peerId;
			if (eq_endpoint || eq_peer_id)
			{
				if (eq_endpoint)
				{
					rec = &t->endpoints[i];
					found = true;
				}

				t->endpoints[i].expire = TRACKER_ENDPOINT_TTL;
				if (evt == "completed") {
					t->endpoints[i].complete = true;
					std::swap(t->endpoints[i], t->endpoints.back());
				}
			}
		}

		if (!found)
		{
			TrackerRecord r;
			r.endpoint = { fromAddr, port };
			r.expire = TRACKER_ENDPOINT_TTL;
			r.numAnnounces = 0;
			r.peerId = peerId;
			r.complete = false;
			t->endpoints.push_back(std::move(r));

			rec = &t->endpoints.back();
		}

		int num_complete(0);
		int num_total(0);

		std::string self_peer_id = m_torrent->PeerId();
		bool self_completed = false;

		for (const auto& r : t->endpoints)
		{
			if ((r.endpoint.addr == fromAddr && r.endpoint.port == port) || (!peerId.empty() && r.peerId == peerId))
				continue;

			if (r.peerId == self_peer_id)
				self_completed = true;

			if (r.complete)
				num_complete++;

			num_total++;
			add_endpoint(r.endpoint.addr.to_string(), r.endpoint.port, r.peerId);
		}

		if (peerId != self_peer_id)
		{
			for (const auto& addr : m_listenAddrs)
			{
				if (self_completed)
					num_complete++;

				num_total++;
				add_endpoint(addr.first, addr.second, self_peer_id);
			}
		}

		//dprint0("=====/PEERS (%i %i)====\n", num_complete, num_incomplete);

		//ans["interval"] = (t.endpoints.size() == 1 || num_incomplete) ? 10 : (TRACKER_ENDPOINT_TTL / 2);
		rec->numAnnounces++;
		ans["interval"] = std::min(rec->numAnnounces * 20, 120);
		ans["min interval"] = 5;

		ans["complete"] = num_complete;
		ans["incomplete"] = (num_total - 1) - num_complete;
	}
	else
	{
		boost::system::error_code ec;
		add_endpoint(fromAddr.to_string(ec), port, peerId);
		assert(!ec);
		assert(ans["peers"].list().size() == 1);
		ans["interval"] = STOPPED_INTERVAL.count();
		ans["min interval"] = STOPPED_MIN_INTERVAL.count();
	}

	std::vector<char> res;
	libtorrent::bencode(std::back_inserter(res), ans);

	std::stringstream ss;
	ss <<
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: " << res.size() << "\r\n\r\n";
	ss.write(res.data(), res.size());

	return ss.str();
}

void VS_TorrentService::InitTrackersProperty(vs::set<std::pair<std::string, net::port>> trackersList)
{
	std::string trackers_str = get_trackers(m_listenAddrs, m_ourEndpoint, std::move(trackersList));
	if (!trackers_str.empty()) {
		auto dbStorage = g_dbStorage;
		if (dbStorage && !trackers_str.empty()) {
			if (!dbStorage->SetAppProperty(TRACKERS_PROP, trackers_str.c_str())) {
				dstream0 << "Cannot set trackers property";
			}
		} else {
			dstream0 << "Cannot set trackers property";
		}
	}
}

void VS_TorrentService::ClearTrackersProperty() {
    auto dbStorage = g_dbStorage;
    if (dbStorage) {
		if (!dbStorage->SetAppProperty(TRACKERS_PROP, "")) {
			dstream0 << "Cannot set trackers property";
        }
    } else {
		dstream0 << "Cannot set trackers property";
    }
}

void VS_TorrentService::PostRequest(const char *toUser, const VS_Container& cnt) {
	if (toUser) {
		if (m_getServerId) {
			auto server_id = m_getServerId(toUser);
			VS_TransportRouterServiceHelper::PostRequest(server_id.c_str(), toUser, cnt);
		} else {
			VS_TransportRouterServiceHelper::PostRequest(nullptr, toUser, cnt);
		}
	}
}

inline const std::string& VS_TorrentService::FileStorageDir() const
{
	return m_fileStorageDir;
}

boost::optional<std::string> VS_TorrentService::HandleRequest(string_view in, const net::address& fromAddr, const net::port /*fromPort*/)
{
	return ProcessTrackerRequest(in, fromAddr);
}

acs::Response VS_TorrentService::Protocol(const stream_buffer& buffer, unsigned /*channelToken*/)
{
	std::size_t len = buffer.size();
	if (len > sizeof(SIGNATURE) - 1)
		len = sizeof(SIGNATURE) - 1;

	if (::memcmp(SIGNATURE, &buffer[0], len) != 0)
		return acs::Response::not_my_connection;

	if (len == sizeof(SIGNATURE) - 1)
		return acs::Response::accept_connection;

	return acs::Response::next_step;
}

void VS_TorrentService::Accept(boost::asio::ip::tcp::socket&& socket, stream_buffer&& buffer)
{
	if (m_cacheListenPort == 0 && (m_cacheListenPort = this->ListenPort()) == 0)
		return;

	auto proxy_session = vs::MakeShared<proxy::tcp::Session>(
		m_ios, //ios
		MAX_BUFFER_PROXY_SIZE, //max size client buffer
		MAX_BUFFER_PROXY_SIZE, // max size server buffer
		TIMEOUT_PROXY // timeout
	);

	proxy_session->Start(std::move(socket), std::move(buffer), { TORRENT_PROXY_ADDR, m_cacheListenPort });
}

#undef DEBUG_CURRENT_MODULE
