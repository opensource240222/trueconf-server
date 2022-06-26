#include "FrameTransmitter.h"
#include "TransceiverLib/VS_NetworkRelayMessage.h"
#include "TransceiverLib/VS_AuthConnectionInterface.h"
#include "TransceiverLib/VS_MainRelayMessage.h"
#include "TransceiverLib/VS_ControlRelayMessage.h"

ts::FrameTransmitter::FrameTransmitter(const std::shared_ptr<VS_AuthConnectionInterface>& auth): m_auth_conn(auth)
{
}

ts::FrameTransmitter::~FrameTransmitter()
{
}

bool EqualEndpoints(const boost::asio::ip::tcp::socket & s1, const boost::asio::ip::tcp::socket & s2) {
	try{
		return s1.local_endpoint() == s2.local_endpoint() && s1.remote_endpoint() == s2.remote_endpoint();
	}
	catch (const boost::system::system_error& /*err*/){
		return false;
	}

	return false;
}

bool ts::FrameTransmitter::SetTCPConnection(boost::asio::ip::tcp::socket && socket, acs::Handler::stream_buffer && buffer)
{
	auto auth = m_auth_conn.lock();
	if (!auth) return false;

	const char *conf_name(nullptr);
	const unsigned char *auth_data(nullptr);
	unsigned long auth_data_ln(0);

	VS_StartFrameTransmitterMess mess;
	if (!mess.SetMessage(buffer.data(), buffer.size()) || !(conf_name = mess.GetConferenceName()) || !(auth_data = mess.GetAuthData(auth_data_ln)))
		return false;
	if (!auth->AuthConnection(auth_data, auth_data_ln)) return false;

	{auto pLockedConns = m_conns_info.lock();
	const std::string confName(conf_name);

	auto it = pLockedConns->find(confName);
	if (it != pLockedConns->end()) {
		if (EqualEndpoints(socket, it->second.socket)) return true;
		pLockedConns->erase(it);
	}

	auto el = pLockedConns->emplace(confName, ConnectionInfo(std::move(socket), confName)).first;

	el->second.socket.async_receive(boost::asio::buffer(&el->second.rcv_byte, sizeof(el->second.rcv_byte)),
		[this, self = shared_from_this(), confName](const boost::system::error_code & error, std::size_t bytes_received) {
		if (error) {
			handle_error(confName, error);
			return;
		}

		handle_read(confName,error, bytes_received);
	});
	}// end of lock

	return true;
}

void ts::FrameTransmitter::TransmitFrame(const char * conf_name, const char * part, const stream::FrameHeader * frame_head, const void * frame_data)
{
	if (!conf_name) return;

	VS_ControlRelayMessage temp_mess;
	if (!temp_mess.MakeTransmitFrame(conf_name, part, frame_head, frame_data))
		return;

	auto mess = std::make_shared<VS_MainRelayMessage>();
	mess->SetMessage(temp_mess.GetMess());

	{auto pLockedConfsInfo = m_conns_info.lock();
	const std::string confName(conf_name);
	auto it = pLockedConfsInfo->find(confName);
	if (it == pLockedConfsInfo->end()) return;

	bool write_in_progress = !it->second.writeMessQueue.empty();
	it->second.writeMessQueue.push(mess);

	if (!write_in_progress) SendMsg(it->second, mess);
	} // end of lock
}


void ts::FrameTransmitter::handle_read(const std::string& confName, const boost::system::error_code & error, std::size_t /*bytes_received*/)
{
	// for now we are not expect read operations
	handle_error(confName, error);
}

void ts::FrameTransmitter::handle_error(const std::string & confName, const boost::system::error_code & /*error*/)
{
	{auto pLockedConns = m_conns_info.lock();
	auto it = pLockedConns->find(confName);
	if (it == pLockedConns->end()) return;
	pLockedConns->erase(it);
	} // end of lock
}

void ts::FrameTransmitter::handle_write(const std::string & confName, const boost::system::error_code & /*error*/)
{
	{auto pLockedConns = m_conns_info.lock();
	auto it = pLockedConns->find(confName);
	if (it == pLockedConns->end()) return;

	it->second.writeMessQueue.pop();
	if (!it->second.writeMessQueue.empty()) {
		SendMsg(it->second, it->second.writeMessQueue.front());
	}
	} // end of lock
}

void ts::FrameTransmitter::SendMsg(ConnectionInfo & i, const std::shared_ptr<VS_NetworkRelayMessageBase>& m)
{
	const auto &confName = i.conf_name;
	const auto& buf = m->GetMess();

	i.socket.async_send(boost::asio::buffer(buf->data(), buf->size()),
		[this, self = shared_from_this(), confName](const boost::system::error_code & error, std::size_t /*bytes_transferred*/) {
		if (error) {
			handle_error(confName, error);
			return;
		}

		handle_write(confName, error);
	});
}
