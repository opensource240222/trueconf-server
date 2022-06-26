#include "VS_FrameReceiver.h"
#include "streams/Relay/VS_TransmitFrameInterface.h"
#include "TransceiverLib/VS_ControlRelayMessage.h"
#include "TransceiverLib/VS_MainRelayMessage.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/cpplib/scope_exit.h"
#include "streams/Protocol.h"

#include <boost/make_shared.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER
//#define TRACE_RECEIVED_PACKETS

VS_FrameReceiver::VS_FrameReceiver(const std::string &addrs, std::string m_circuit_name, unsigned char *secretData, const unsigned long sz, boost::asio::io_service &ios)
	: VS_NetChannelsRouter(ios, std::move(m_circuit_name))
	, m_serverAddrList(addrs)
	, m_secretData(secretData,secretData+sz)
	, m_byteReceived(0)
	, m_timeLastCalc()
{
}
VS_FrameReceiver::~VS_FrameReceiver()
{
}

bool VS_FrameReceiver::ConnectToTransmitFrame(const char *conf_name, const std::shared_ptr<VS_TransmitFrameInterface> &trans_fr_cb) {
	bool res(false);
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		res = ConnectToTransmitFrameImpl(conf_name, trans_fr_cb);
	});
	done.wait();
	return res;
}

bool VS_FrameReceiver::ConnectToTransmitFrameImpl(const std::string &conf_name, const std::shared_ptr<VS_TransmitFrameInterface> &trans_fr_cb)
{
	/**
	    0. Send command to work thread
		1. find/create signal in map
		2. wrap trans_fr_cb to weak_ptr
		3. connect TransmitFrame to signal
	*/

	if(!trans_fr_cb)
		return false;

	auto info = std::make_tuple(conf_name, trans_fr_cb.get());
	if(m_circuitsCollection.find(info) != m_circuitsCollection.end())
		return false;

	std::weak_ptr<VS_TransmitFrameInterface>	w_circuit(trans_fr_cb);
	stream::FrameReceivedSignalType::slot_type slot = [this, w_circuit](const char* conf_name, const char* part, const stream::FrameHeader* frame_head, const void* frame_data)
	{
		TransmitFrame(w_circuit,conf_name,part,frame_head,frame_data);
	};
	auto iter = m_transmitFrameSignalsByConf.find(conf_name);
	if(iter == m_transmitFrameSignalsByConf.end())
	{
		iter = m_transmitFrameSignalsByConf.emplace(conf_name, boost::make_shared<stream::FrameReceivedSignalType>()).first;
	}
	m_circuitsCollection[info] = iter->second->connect(slot);
	return true;
}

bool VS_FrameReceiver::DisconnectFromTransmitFrame(const char* conf_name, const std::shared_ptr<VS_TransmitFrameInterface> &trans_fr_cb) {
	if (!conf_name) return false;

	bool res(false);
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		res = DisconnectFromTransmitFrameImpl(conf_name, trans_fr_cb);
	});
	done.wait();
	return res;
}

bool VS_FrameReceiver::DisconnectFromTransmitFrameImpl(const std::string &conf_name, const std::shared_ptr<VS_TransmitFrameInterface> &trans_fr_cb)
{
	if(!trans_fr_cb)
		return false;
	/**
		1. find connection;
		2. disconnect;
		3. remove frome map;
	*/
	auto info = std::make_tuple(conf_name, trans_fr_cb.get());
	auto i = m_circuitsCollection.find(info);
	if(i==m_circuitsCollection.end())
		return false;
	i->second.disconnect();
	m_circuitsCollection.erase(i);
	auto i_sig = m_transmitFrameSignalsByConf.find(conf_name);
	if(i_sig!=m_transmitFrameSignalsByConf.end() && i_sig->second && i_sig->second->empty())
		m_transmitFrameSignalsByConf.erase(i_sig);
	return true;
}

void VS_FrameReceiver::StartConference(const char *conf_name)
{
	ConnectToServer(m_serverAddrList,&m_secretData[0],m_secretData.size(),conf_name);
}
void VS_FrameReceiver::StartConference(const char *conf_name, const char *, VS_Conference_Type, VS_GroupConf_SubType)
{
	StartConference(conf_name);
}

void VS_FrameReceiver::StopConference(const char *conf_name) {
	if (!conf_name) return;

	m_strand.dispatch([this, w_this = this->weak_from_this(), conf_name = std::string(conf_name)]() {
		if (auto self = w_this.lock())
			StopConferenceImpl(conf_name);
	});
}

void VS_FrameReceiver::StopConferenceImpl(const std::string &conf_name)
{
	dstream4 << "VS_FrameReceiver::StopConference " << conf_name;
	CloseChannel(conf_name);
	/**
		disconnect all circuits
	*/
	m_transmitFrameSignalsByConf.erase(conf_name);
	auto iter = m_circuitsCollection.begin();
	while(iter!=m_circuitsCollection.end())
	{
		if(!iter->second.connected())
			iter = m_circuitsCollection.erase(iter);
		else
			iter++;
	}
}

void VS_FrameReceiver::TransmitFrame(const std::weak_ptr<VS_TransmitFrameInterface> &circuit, const char *conf_name, const char *part, const stream::FrameHeader *frame_head, const void *frame_data)
{
//	return;
	auto circ = circuit.lock();
	if(circ)
		circ->TransmitFrame(conf_name,part,frame_head,frame_data);
}

void VS_FrameReceiver::ParticipantConnect(const char *conf_name, const char *part_name)
{
	/**
		TODO: nothing
	*/
}
void VS_FrameReceiver::ParticipantDisconnect(const char *conf_name, const char *part_name)
{
	/**
		TODO: nothing
	*/
}
void VS_FrameReceiver::SetParticipantCaps(const char *conf_name, const char *part_name,const void *caps_buf,const unsigned long buf_sz)
{
}

void VS_FrameReceiver::ProcessingRcvMessage(const boost::shared_ptr<VS_MainRelayMessage> &mess)
{
	if (mess->GetMessageType() == VS_MainRelayMessage::e_transmit_frame)
	{
		boost::shared_ptr<VS_ControlRelayMessage> temp_mess(new VS_ControlRelayMessage);
		temp_mess->SetMessage(mess->GetMess());
#ifdef TRACE_RECEIVED_PACKETS	// remove it after porting completed
		dstream4 << "VS_FrameReceiver::ProcessingRcvMessage received frame\n";
		if (const stream::FrameHeader *head = temp_mess->GetFrameHead()) {
			dstream4 << "Frame: len=" << head->length << " chksum=" << static_cast<uint32_t>(head->cksum) << " tick=" << head->tick_count << " track=" << static_cast<uint32_t>(head->track);
		}
#endif // TRACE_RECEIVED_PACKETS

		auto iter = m_transmitFrameSignalsByConf.find(temp_mess->GetConferenceName());
		if(iter!=m_transmitFrameSignalsByConf.end())
			(*(iter->second))(temp_mess->GetConferenceName(),temp_mess->GetParticipantName(),temp_mess->GetFrameHead(),temp_mess->GetFrameData());

		const auto now = std::chrono::steady_clock::now();
		if (now - m_timeLastCalc > std::chrono::seconds(5))
		{
			if(m_byteReceived!=0)
			{
				int64_t bitrate = m_byteReceived / std::chrono::duration_cast<std::chrono::seconds>(now - m_timeLastCalc).count();
				//dprint4("bitrate = %d\n",bitrate);
				m_byteReceived = 0;
			}
			m_timeLastCalc = now;
		}
		unsigned long sz(0);
		mess->GetMess(sz);
		m_byteReceived += sz;
	}
}