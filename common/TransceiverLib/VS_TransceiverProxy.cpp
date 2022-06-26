#include "VS_TransceiverProxy.h"
#include "VS_RemoteCircuitController.h"
#include "VS_RTPModuleControl.h"
#include "VS_ConfControlModule.h"
#include "std/debuglog/VS_Debug.h"
#include "std/cpplib/VS_MediaFormat.h"
#include "std/cpplib/VS_ClientCaps.h"
#include "VS_ConfRecorderModuleCtrl.h"
#include "VS_RTSPBroadcastModuleCtrl.h"
#include "std/CallLog/VS_ConferenceDescription.h"
#include "std/VS_TransceiverInfo.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

VS_TransceiverProxy::VS_TransceiverProxy(const std::shared_ptr<VS_RemoteCircuitController>& circuitControl,
										const std::shared_ptr<VS_RelayMessageSenderInterface>& msgSender,
										const std::shared_ptr<VS_TransmitFrameInterface>& frameTrans, const std::shared_ptr<VS_ConfControlModule>& conf_control,
										const std::shared_ptr<VS_RTPModuleControl>& rtp_control)
	: m_circuitControl(circuitControl)
	, m_msgSender(msgSender)
	, m_FrameTrans(frameTrans)
	, m_conf_control(conf_control)
	, m_rtp_control(rtp_control)
{
	if (!circuitControl)
		return;

	if (!conf_control || !circuitControl->RegisterModule(conf_control, true))
		return;

	if (!rtp_control || !circuitControl->RegisterModule(rtp_control, true))
		return;

	if (msgSender)
	{
		conf_control->SetMessageSender(msgSender);
		rtp_control->SetMessageSender(msgSender);
	}
}
VS_TransceiverProxy::~VS_TransceiverProxy()
{
	if (auto circuitControl = m_circuitControl.lock())
	{
		circuitControl->UnregisterModule(m_rtp_control.lock());
		circuitControl->UnregisterModule(m_conf_control.lock());
	}
}

void VS_TransceiverProxy::TransmitFrame(const char *conf_name, const char *part, const stream::FrameHeader *frame_head, const void *frame_data)
{
	if(auto frameTrans = m_FrameTrans.lock())
		frameTrans->TransmitFrame(conf_name,part,frame_head,frame_data);
}
void VS_TransceiverProxy::StartConference(const char *conf_name)
{
	dprint4("TRANSPX: StartConference conf=%s\n",conf_name);
	/**
		skip. Use method with conference type
	*/
	/*if(m_circuitCntrl)
		m_circuitCntrl->StartConference(conf_name);*/
	/**
		open connection for conference
	*/
}
void VS_TransceiverProxy::StartConference(const char *conf_name, const char *owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type)
{
	dprint4("TRANSPX: StartConference conf=%s owner=%s type=%d, sub_type=%d\n",conf_name,owner_name,type,sub_type);
	if (auto circuitControl = m_circuitControl.lock())
		circuitControl->StartConference(conf_name, owner_name, type, sub_type);

}
void VS_TransceiverProxy::StopConference(const char *conf_name)
{
	assert(conf_name != nullptr);
	dprint4("TRANSPX: StopConference conf=%s\n",conf_name);

	std::lock_guard<vs::fast_mutex> _(m_confControlMtx);	// for now using only for stop conference
	auto circuitControl = m_circuitControl.lock();
	if (circuitControl)
	{
		circuitControl->StopConference(conf_name);
		circuitControl->UnregisterAllTemporaryModules();
	}
	m_fireRemoveMeFromSenders();
	m_fireRemoveMeFromSenders.disconnect_all_slots();
	m_fireReturnToPool(conf_name);
	/**
		close conference connection
	*/
}

void VS_TransceiverProxy::StartAndAnounceBroadcast(const std::shared_ptr<VS_RTSPBroadcastModuleCtrl> &broadcaster, const VS_ConferenceDescription& conf) const{
	if (!broadcaster) return;

	std::string url("c/");
	url += conf.m_call_id.m_str;
	broadcaster->StartBroadcast(conf.m_name.m_str, url, conf.m_topic, conf.m_rtspEnabledCodecs, conf.m_rtspHelperProgram);
	for (const auto& announce : conf.m_rtspAnnounces)
		broadcaster->AnnounceBroadcast(
		conf.m_name.m_str,
		announce.first,
		announce.second.url,
		announce.second.username,
		announce.second.password,
		announce.second.rtp_over_tcp,
		announce.second.enabled_codecs,
		announce.second.keepalive_timeout,
		announce.second.retries,
		announce.second.retry_delay
		);
}

void VS_TransceiverProxy::RestoreConferences(const std::vector<VS_ConferenceDescription> &to_restore, const std::unordered_map<std::string, std::vector<part_start_info>> &confs_users){
	dprint4("TRANSPX: RestoreConferences %zu item(s)\n", to_restore.size());
	for (const auto &conf : to_restore){
		StartConference(conf.m_name.m_str, conf.m_owner.m_str, (VS_Conference_Type)conf.m_type, (VS_GroupConf_SubType)conf.m_SubType);
		if (m_conf_recorder_module && conf.m_need_record) m_conf_recorder_module->StartRecordConference(conf.m_name.m_str);

		if (conf.m_isBroadcastEnabled){
			auto rtspBroadcast = m_rtsp_broadcast_module.lock();
			StartAndAnounceBroadcast(rtspBroadcast, conf);
		}

		/* confs_users: conf_id1 -> user1,user2,...,userN */
		/*              conf_id2 -> user1,user2,...,userN */
		const auto it = confs_users.find(conf.m_name.m_str);
		if (it != confs_users.end()){
			const auto &users_to_connect = it->second;
			for (const auto& user_info : users_to_connect){
				const auto& part_name = std::get<0>(user_info);
				const auto& part_caps = std::get<1>(user_info);
				const auto& part_lfltr = std::get<2>(user_info);
				const auto& display_name = std::get<3>(user_info);
				const auto& part_role = std::get<4>(user_info);

				ParticipantConnect(conf.m_name.m_str, part_name.c_str());	// connect to conf
				SetParticipantCaps(conf.m_name.m_str, part_name.c_str(), part_caps.Buffer(), part_caps.Size());

				if (auto confControl = m_conf_control.lock()){
					if (!display_name.empty())	confControl->SetDisplayName(conf.m_name.m_str, part_name.c_str(), display_name.c_str());
					if (part_lfltr != 0L)		confControl->UpdateFilter(conf.m_name.m_str, part_name.c_str(), part_lfltr, part_role);
				}
			}
		}
	}
}

void VS_TransceiverProxy::ParticipantConnect(const char *conf_name,const char *part_name)
{
	dprint4("TRANSPX: ParticipantConnect conf=%s part=%s\n",conf_name,part_name);
	if (auto circuitControl = m_circuitControl.lock())
		circuitControl->ParticipantConnect(conf_name, part_name);
	/**
		Signal abour partconnect;
	*/
}
void VS_TransceiverProxy::ParticipantDisconnect(const char *conf_name, const char *part_name)
{
	dprint4("TRANSPX: ParticipantDisconnect conf=%s part=%s\n",conf_name,part_name);
	if (auto circuitControl = m_circuitControl.lock())
		circuitControl->ParticipantDisconnect(conf_name, part_name);
	/**
		Signal about part disconnect
	**/
}
void VS_TransceiverProxy::SetParticipantCaps(const char *conf_name, const char *part_name, const void *caps_buf, const unsigned long buf_sz)
{
	dprint4("TRANSPX: SetParticipantCaps conf=%s part=%s\n",conf_name,part_name);
	if (auto circuitControl = m_circuitControl.lock())
		circuitControl->SetParticipantCaps(conf_name, part_name, caps_buf, buf_sz);
}
void VS_TransceiverProxy::RestrictBitrateSVC(const char *conferenceName, const char *participantName, long v_bitrate, long bitrate, long old_bitrate)
{
	dprint4("TRANSPX: RestrictBitrateSVC conf=%s part=%s v_bitrate = %ld bitrate = %ld old_bitrate = %ld\n", conferenceName, participantName, v_bitrate, bitrate, old_bitrate);
	if (auto circuitControl = m_circuitControl.lock())
		circuitControl->RestrictBitrateSVC(conferenceName, participantName, v_bitrate, bitrate, old_bitrate);
}
void VS_TransceiverProxy::RequestKeyFrame(const char *conferenceName, const char *participantName)
{
	dprint4("TRANSPX: RequestKeyFrame conf=%s part=%s\n", conferenceName, participantName);
	if (auto circuitControl = m_circuitControl.lock())
		circuitControl->RequestKeyFrame(conferenceName, participantName);
}
bool VS_TransceiverProxy::SendMsg(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess)
{
	auto msgSender = m_msgSender.lock();
	return (msgSender)? msgSender->SendMsg(mess): false;
}
net::address VS_TransceiverProxy::GetRemoteAddress() const
{
	auto msgSender = m_msgSender.lock();
	return msgSender ? msgSender->GetRemoteAddress() : net::address();
}
bool VS_TransceiverProxy::RegisterModule(const std::shared_ptr<VS_RelayModule> &module, bool permanent)
{
	auto circuitControl = m_circuitControl.lock();
	return circuitControl ? circuitControl->RegisterModule(module, permanent) : false;
}
void VS_TransceiverProxy::UnregisterModule(const std::shared_ptr<VS_RelayModule> &module)
{
	if (auto circuitControl = m_circuitControl.lock())
		circuitControl->UnregisterModule(module);
}
std::shared_ptr<VS_ConfControlModule> VS_TransceiverProxy::ConfControl()
{
	return m_conf_control.lock();
}

std::shared_ptr<VS_RTPModuleControlInterface> VS_TransceiverProxy::GetRTPModule()
{
	return m_rtp_control.lock();
}

void VS_TransceiverProxy::SetConfRecorderModule(const std::shared_ptr<VS_ConfRecorderModuleCtrl> &module){
	m_conf_recorder_module = module;
}

void VS_TransceiverProxy::SetRTSPBroadcastModule(const std::shared_ptr<VS_RTSPBroadcastModuleCtrl> &module){
	m_rtsp_broadcast_module = module;
}

const std::string & VS_TransceiverProxy::GetTransceiverName() const
{
	return m_connectedTransceiverName;
}

void VS_TransceiverProxy::SetTransceiverName(const std::string & name)
{
	dstream2 << "VS_TransceiverProxy::SetTransceiverName name='" << name << "'\n";
	m_connectedTransceiverName = name;
}

net::port VS_TransceiverProxy::GetLive555Port() const
{
	if (auto circuitControl = m_circuitControl.lock())
		return circuitControl->GetLive555Port();
	else
		return 0;
}

std::string VS_TransceiverProxy::GetLive555Secret() const
{
	if (auto circuitControl = m_circuitControl.lock())
		return circuitControl->GetLive555Secret();
	else
		return {};
}

#undef DEBUG_CURRENT_MODULE
