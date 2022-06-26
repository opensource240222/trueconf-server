#include "VS_CommandReceiver.h"
#include "TransceiverLib/VS_ControlRelayMessage.h"
#include "TransceiverLib/VS_MainRelayMessage.h"
#include "TransceiverLib/VS_NetworkRelayMessage.h"
#include "TransceiverLib/VS_RelayModule.h"
#include "TransceiverCircuit/ConnectToServer.h"
#include "streams/Relay/VS_ConfControlInterface.h"
#include "std/debuglog/VS_Debug.h"
#include "std/VS_AuthUtils.h"
#include "std/cpplib/VS_MediaFormat.h"
#include "std/cpplib/event.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/MakeShared.h"
#include "std-generic/cpplib/scope_exit.h"
#include "net/Lib.h"
#include "net/tls/socket.h"

#include <boost/make_shared.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

namespace
{
	enum {
		e_NONE,
		e_START_IO,
		e_STOP
	};
}

// do not call constructor directly
VS_CommandReceiver::VS_CommandReceiver(
	const std::string &addrs,
	std::string circuit_name,
	const unsigned char *secretData,
	const unsigned long sz,
	boost::asio::io_service& ios)

	: m_serverAddrList(addrs)
	, m_circuit_name(std::move(circuit_name))
	, m_secretData(secretData,secretData+sz)
	, m_ios(ios)
{
	m_fireConferenceStart.connect([this](const char *conf_name, const char *owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type){StartConference(conf_name, owner_name, type, sub_type);}, boost::signals2::at_back);
	m_fireConferenceStop.connect([this](const char *conf_name) {StopConference(conf_name);}, boost::signals2::at_front);
	m_fireParticipantConnect.connect([this](const char *conf_name,const char *part_name){ParticipantConnect(conf_name,part_name);});
	m_fireParticipantDisconnect.connect([this](const char *conf_name, const char *part_name){ParticipantDisconnect(conf_name,part_name);});
	m_fireSetPartCaps.connect([this](const char *conf_name, const char *part_name, const void *caps_buf, const unsigned long caps_len){SetPartCaps(conf_name,part_name,caps_buf,caps_len);});
	m_fireRestrictBitrateSVC.connect([this](const char *conferenceName, const char *participantName, long v_bitrate, long bitrate, long old_bitrate){RestrictBitrateSVC(conferenceName,participantName,v_bitrate,bitrate,old_bitrate);});
	m_fireRequestKeyFrame.connect([this](const char *conf_name, const char*part_name) {RequestKeyFrame(conf_name, part_name);});
}

std::shared_ptr<ts::NetChannelInterface>& VS_CommandReceiver::GetNetChannel()
{
	assert(m_net_channel != nullptr);
	return m_net_channel;
}

bool VS_CommandReceiver::SendMsg(const boost::shared_ptr<VS_NetworkRelayMessageBase>& mess)
{
	assert(m_net_channel != nullptr);
	return m_net_channel->SendMsg(mess);
}

net::address VS_CommandReceiver::GetRemoteAddress() const
{
	assert(m_net_channel != nullptr);
	return m_net_channel->GetRemoteAddress();
}

VS_CommandReceiver::~VS_CommandReceiver()
{
}

bool VS_CommandReceiver::ConnectToServer(std::function<void()>&& onConnDie)
{
	int32_t value = 0;
	const bool tlsEnabled = VS_RegistryKey(false, CONFIGURATION_KEY).GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "TransceiverRelayTLS") > 0 && value != 0;

	if (tlsEnabled)
		return Connect<net::tls>(std::move(onConnDie));
	else
		return Connect<boost::asio::ip::tcp>(std::move(onConnDie));
}

template <class Protocol>
bool VS_CommandReceiver::Connect(std::function<void()>&& onConnDie)
{
	bool result = false;
	vs::event done(true);
	ts::ConnectToServer<Protocol>(m_ios, m_serverAddrList, std::chrono::seconds(30), [&](const boost::system::error_code& ec, typename Protocol::socket&& socket)
	{
		VS_SCOPE_EXIT { done.set(); };
		if (ec)
		{
			dstream0 << "Couldn't create connection for command channel: " << ec.message();
			return;
		}

		auto net_channel = vs::MakeShared<ts::NetChannel<typename Protocol::socket>>(socket.get_io_service());
		net_channel->SetOnConnDie(std::move(onConnDie));
		auto w_instance = this->weak_from_this(); // MSVC gets confused when w_instance is used directly in the capture list and complains that there is no weak_from_this() in the class generated for parent lambda.
		net_channel->SetRecvMessageCallBack(
			[w_instance = std::move(w_instance)](boost::shared_ptr<VS_MainRelayMessage>& recvMess) {
				if (auto instance = w_instance.lock())
					instance->ProcessingRcvMessage(recvMess);
			}
		);
		if (!net_channel->SetChannelConnection(std::move(socket)))
			return;

		const auto temp_password = auth::MakeTempPassword(string_view((const char*)m_secretData.data(), m_secretData.size()), m_circuit_name);
		result = net_channel->SendMsg(boost::make_shared<VS_StartControlMess>((const uint8_t*)temp_password.c_str(), temp_password.length()));
		net_channel->RequestRead();
		m_net_channel = std::move(net_channel);
	});
	done.wait();
	return result;
}

void VS_CommandReceiver::ProcessingRcvMessage(boost::shared_ptr<VS_MainRelayMessage>& recvMess)
{
	boost::shared_ptr<ConfConditionsSignals> sigs;
	boost::shared_ptr<VS_ControlRelayMessage> ContMess(new VS_ControlRelayMessage);
	ContMess->SetMessage(recvMess->GetMess());
	switch(recvMess->GetMessageType())
	{
	case VS_MainRelayMessage::e_start_conference:
		{
			dprint3("Start Conference %s\n", ContMess->GetConferenceName());
			VS_Conference_Type type;
			VS_GroupConf_SubType sub_type;
			if(!ContMess->GetConferenceTypes(type,sub_type))
				break;
			m_fireConferenceStart(ContMess->GetConferenceName(),ContMess->GetOwnerName(),type,sub_type);
		}
		break;
	case VS_MainRelayMessage::e_stop_conference:
		dprint3("Stop Conference %s\n", ContMess->GetConferenceName());
		m_fireConferenceStop(ContMess->GetConferenceName());
		break;
	case VS_MainRelayMessage::e_part_connect:
		dprint3("Participant Connect c=%s p=%s\n", ContMess->GetConferenceName(), ContMess->GetParticipantName());
		m_fireParticipantConnect(ContMess->GetConferenceName(),ContMess->GetParticipantName());
		break;
	case VS_MainRelayMessage::e_part_disconenct:
		dprint3("Participant Disconnect c=%s p=%s\n", ContMess->GetConferenceName(), ContMess->GetParticipantName());
		m_fireParticipantDisconnect(ContMess->GetConferenceName(),ContMess->GetParticipantName());
		break;
	case VS_MainRelayMessage::e_set_caps:
		{
			size_t caps_len = 0;
			const void *caps_buf = ContMess->GetCaps(caps_len);
			dprint3("SetCaps c=%s p=%s\n", ContMess->GetConferenceName(), ContMess->GetParticipantName());
			m_fireSetPartCaps(ContMess->GetConferenceName(),ContMess->GetParticipantName(),caps_buf,caps_len);

		}
		break;
	case VS_MainRelayMessage::e_restrict_btr_svc:
		{
			int32_t v_btr = 0;
			int32_t btr = 0;
			int32_t o_btr = 0;
			if(!ContMess->GetSVCBitrate(v_btr,btr,o_btr))
				break;
			dstream3 << "Restrict bitrate SVC c=" << ContMess->GetConferenceName() << " p=" << ContMess->GetParticipantName() << " v_b=" << v_btr << " b=" << btr << " ob=" << o_btr;
			m_fireRestrictBitrateSVC(ContMess->GetConferenceName(),ContMess->GetParticipantName(),v_btr,btr,o_btr);
			break;
		}
		break;
	case VS_MainRelayMessage::e_request_key_frame:
		{
			dprint3("RequestKeyFrame c=%s p=%s\n", ContMess->GetConferenceName(), ContMess->GetParticipantName());
			m_fireRequestKeyFrame(ContMess->GetConferenceName(),ContMess->GetParticipantName());
		}
		break;
	case VS_MainRelayMessage::e_is_not_complete:
		break;
	case VS_MainRelayMessage::e_bad_message:
		dprint1("Bad message was received\n");
		recvMess->Clear();
		break;
	case VS_MainRelayMessage::e_envelope:
		const char* m = recvMess->GetModuleName();
		dprint3("e_envelope received: module = %s\n", m);
		std::shared_ptr<VS_RelayModule>	module;
		{
			std::lock_guard<std::mutex> _(m_lock);
			module = GetModule(recvMess->GetModuleName());
		}
		if(module)
		{
			module->ProcessingMessage(recvMess);
			recvMess = boost::make_shared<VS_MainRelayMessage>();
		}
		else
			recvMess->Clear();
		break;
	}
}

bool VS_CommandReceiver::RegisterModule(const std::shared_ptr<VS_RelayModule> &module)
{
	if(!module)
		return false;
	std::lock_guard<std::mutex> _(m_lock);
	bool res = VS_RelayModulesMgr::RegisterModule(module);
	if(res)
		module->SetMessageSender(m_sender_interface.lock());
	return res;
}

void VS_CommandReceiver::UnregisterModule(const std::shared_ptr<VS_RelayModule> &module)
{
	if(!module)
		return;
	std::lock_guard<std::mutex> _(m_lock);
	VS_RelayModulesMgr::UnregisterModule(module);
}

void VS_CommandReceiver::ConnectToConfControl(const char *conf_name, const std::shared_ptr<VS_ConfControlInterface> &conf_ctrl_cb)
{
	if(!conf_ctrl_cb)
		return;
	auto info = std::make_tuple(conf_name, conf_ctrl_cb.get());
	ConfCtrlConnections			conns;
	std::weak_ptr<VS_ConfControlInterface> weak_cntrl(conf_ctrl_cb);
	std::lock_guard<std::mutex> _(m_lock);
	if(m_connsCollection.find(info) != m_connsCollection.end())
		return;
	auto cc_slot= [this,weak_cntrl](const char *conf_name, const char *owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type)
	{
		auto lock = weak_cntrl.lock();
		if(lock)
			lock->StartConference(conf_name, owner_name, type, sub_type);
	};
	auto sc_slot	= [this, weak_cntrl](const char *conf_name)
	{
		auto lock = weak_cntrl.lock();
		if(lock)
			lock->StopConference(conf_name);
	};
	auto pc_slot	= [this, weak_cntrl](const char *conf_name,const char *part_name)
	{
		auto lock = weak_cntrl.lock();
		if(lock)
			lock->ParticipantConnect(conf_name,part_name);
	};
	auto pd_slot = [this, weak_cntrl](const char *conf_name, const char *part_name)
	{
		auto lock = weak_cntrl.lock();
		if(lock)
			lock->ParticipantDisconnect(conf_name,part_name);
	};
	auto set_part_caps_slot = [this, weak_cntrl](const char *conf_name, const char *part_name, const void *caps_buf, const unsigned long caps_len)
	{
		auto lock = weak_cntrl.lock();
		if(lock)
			lock->SetParticipantCaps(conf_name,part_name,caps_buf,caps_len);
	};
	auto rstrct_bitrate_svc_slot = [this, weak_cntrl](const char *conferenceName, const char *participantName, long v_bitrate, long bitrate, long old_bitrate)
	{
		auto lock = weak_cntrl.lock();
		if(lock)
			lock->RestrictBitrateSVC(conferenceName, participantName, v_bitrate, bitrate, old_bitrate);
	};
	auto req_key_frame_slot = [this, weak_cntrl](const char *conferenceName, const char *participantName)
	{
		auto lock = weak_cntrl.lock();
		if (lock)
			lock->RequestKeyFrame(conferenceName, participantName);
	};
	boost::shared_ptr<ConfConditionsSignals> conf_cond_sigs_ptr;
	std::map<std::string,boost::shared_ptr<ConfConditionsSignals>>::iterator iter = m_confConditionsSignals.find(conf_name);
	if(iter==m_confConditionsSignals.end())
	{
		conf_cond_sigs_ptr.reset(new ConfConditionsSignals);
		m_confConditionsSignals[conf_name] = conf_cond_sigs_ptr;
	}
	else
		conf_cond_sigs_ptr = iter->second;
	conns.startConfConn = conf_cond_sigs_ptr->fireStartConf.connect(cc_slot);
	conns.stoptConfConn = conf_cond_sigs_ptr->fireStopConf.connect(sc_slot);
	conns.partConnectConn = conf_cond_sigs_ptr->firePartConnect.connect(pc_slot);
	conns.partDisconnectConn = conf_cond_sigs_ptr->firePartDisconnect.connect(pd_slot);
	conns.setPartCapsConn = conf_cond_sigs_ptr->fireSetPartCaps.connect(set_part_caps_slot);
	conns.restrictBitrateSVCConn = conf_cond_sigs_ptr->fireRestrictBitrateSVC.connect(rstrct_bitrate_svc_slot);
	conns.requestKeyFrameConn = conf_cond_sigs_ptr->fireRequestKeyFrame.connect(req_key_frame_slot);

	m_connsCollection[info] = conns;
}

void VS_CommandReceiver::ConnectToAllConfControl(const std::shared_ptr<VS_ConfControlInterface> &conf_ctrl_cb)
{
	if(!conf_ctrl_cb)
		return;
	auto info = std::make_tuple("", conf_ctrl_cb.get());
	std::weak_ptr<VS_ConfControlInterface> weak_cb(conf_ctrl_cb);
	ConfCtrlConnections conns;
	conns.startConfConn	= m_fireConferenceStart.connect([this,weak_cb](const char *conf_name, const char *owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type)
	{
		auto lock = weak_cb.lock();
		if(lock)
			lock->StartConference(conf_name, owner_name, type, sub_type);
	},boost::signals2::at_front);
	conns.stoptConfConn	= m_fireConferenceStop.connect([this, weak_cb](const char *conf_name)
	{
		auto lock = weak_cb.lock();
		if(lock)
			lock->StopConference(conf_name);
	},boost::signals2::at_back);
	conns.partConnectConn = m_fireParticipantConnect.connect([this, weak_cb](const char *conf_name,const char *part_name)
	{
		auto lock = weak_cb.lock();
		if(lock)
			lock->ParticipantConnect(conf_name,part_name);
	});
	conns.partDisconnectConn	= m_fireParticipantDisconnect.connect([this, weak_cb](const char *conf_name, const char *part_name)
	{
		auto lock = weak_cb.lock();
		if(lock)
			lock->ParticipantDisconnect(conf_name,part_name);
	});
	conns.setPartCapsConn = m_fireSetPartCaps.connect([this, weak_cb](const char *conf_name, const char *part_name, const void *caps_buf, const unsigned long caps_len)
	{
		auto lock = weak_cb.lock();
		if(lock)
			lock->SetParticipantCaps(conf_name,part_name,caps_buf,caps_len);
	});
	conns.restrictBitrateSVCConn = m_fireRestrictBitrateSVC.connect([this, weak_cb](const char *conferenceName, const char *participantName, long v_bitrate, long bitrate, long old_bitrate)
	{
		auto lock = weak_cb.lock();
		if(lock)
			lock->RestrictBitrateSVC(conferenceName, participantName, v_bitrate, bitrate, old_bitrate);
	});
	conns.requestKeyFrameConn = m_fireRequestKeyFrame.connect([this, weak_cb](const char *conferenceName, const char *participantName)
	{
		auto lock = weak_cb.lock();
		if (lock)
			lock->RequestKeyFrame(conferenceName, participantName);
	});
	std::lock_guard<std::mutex> _(m_lock);
	auto iter = m_connsCollection.find(info);
	if(iter!=m_connsCollection.end())
		iter->second.DisconnectAll();
	m_connsCollection[info] = conns;
}

void VS_CommandReceiver::DisconnectFromConfControl(const char *conf_name, const std::shared_ptr<VS_ConfControlInterface> &conf_ctrl_cb)
{
	if(!conf_ctrl_cb)
		return;
	auto info = std::make_tuple(conf_name, conf_ctrl_cb.get());
	std::lock_guard<std::mutex> _(m_lock);
	auto iter = m_connsCollection.find(info);
	if(iter != m_connsCollection.end())
	{
		iter->second.DisconnectAll();
		m_connsCollection.erase(iter);
	}
	std::map<std::string,boost::shared_ptr<ConfConditionsSignals>>::iterator i = m_confConditionsSignals.find(conf_name);
	if(i!=m_confConditionsSignals.end() && i->second->fireStartConf.empty())
		m_confConditionsSignals.erase(i);
}

void VS_CommandReceiver::DisconnectFromAllConfControl(const std::shared_ptr<VS_ConfControlInterface> &conf_ctrl_cb)
{
	if(!conf_ctrl_cb)
		return;
	std::lock_guard<std::mutex> _(m_lock);
	auto info = std::make_tuple("", conf_ctrl_cb.get());
	auto iter = m_connsCollection.find(info);
	if(iter!=m_connsCollection.end())
	{
		iter->second.DisconnectAll();
		m_connsCollection.erase(iter);
	}
}

void VS_CommandReceiver::StartConference(const char *conf_name, const char *owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type)
{
	boost::shared_ptr<ConfConditionsSignals> sigs = GetConfConditionsSignals(conf_name);
	if(sigs)
		sigs->fireStartConf(conf_name, owner_name, type, sub_type);
}

void VS_CommandReceiver::StopConference(const char *conf_name)
{
	boost::shared_ptr<ConfConditionsSignals> sigs = GetConfConditionsSignals(conf_name);
	if(sigs)
		sigs->fireStopConf(conf_name);
}

void VS_CommandReceiver::ParticipantConnect(const char *conf_name,const char *part_name)
{
	boost::shared_ptr<ConfConditionsSignals> sigs = GetConfConditionsSignals(conf_name);
	if(sigs)
		sigs->firePartConnect(conf_name,part_name);
}

void VS_CommandReceiver::ParticipantDisconnect(const char *conf_name, const char *part_name)
{
	boost::shared_ptr<ConfConditionsSignals> sigs = GetConfConditionsSignals(conf_name);
	if(sigs)
		sigs->firePartDisconnect(conf_name,part_name);
}

boost::shared_ptr<VS_CommandReceiver::ConfConditionsSignals> VS_CommandReceiver::GetConfConditionsSignals(const char *conf_name)
{
	boost::shared_ptr<ConfConditionsSignals> nul_res;
	std::lock_guard<std::mutex> _(m_lock);
	std::map<std::string,boost::shared_ptr<ConfConditionsSignals>>::iterator iter = m_confConditionsSignals.find(conf_name);
	if(iter!=m_confConditionsSignals.end())
		return iter->second;
	return nul_res;
}

void VS_CommandReceiver::SetPartCaps(const char *conf_name, const char *part_name, const void *caps_buf, const unsigned long caps_len)
{
	boost::shared_ptr<ConfConditionsSignals> sigs = GetConfConditionsSignals(conf_name);
	if(sigs)
		sigs->fireSetPartCaps(conf_name,part_name,caps_buf,caps_len);
}

void VS_CommandReceiver::RestrictBitrateSVC(const char *conferenceName, const char *participantName, long v_bitrate, long bitrate, long old_bitrate)
{
	boost::shared_ptr<ConfConditionsSignals> sigs = GetConfConditionsSignals(conferenceName);
	if(sigs)
		sigs->fireRestrictBitrateSVC(conferenceName,participantName,v_bitrate, bitrate, old_bitrate);
}

void VS_CommandReceiver::RequestKeyFrame(const char *conferenceName, const char *participantName)
{
	auto sigs = GetConfConditionsSignals(conferenceName);
	if (sigs)
		sigs->fireRequestKeyFrame(conferenceName, participantName);
}

#undef DEBUG_CURRENT_MODULE
