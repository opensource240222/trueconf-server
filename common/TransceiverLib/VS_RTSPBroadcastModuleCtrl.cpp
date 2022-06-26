#include "VS_RTSPBroadcastModuleCtrl.h"
#include "VS_RTSPBroadcastRelayMessage.h"

#include <boost/make_shared.hpp>

#include <memory>

VS_RTSPBroadcastModuleCtrl::VS_RTSPBroadcastModuleCtrl()
	: VS_RelayModule(VS_RTSPBroadcastRelayMessage::module_name)
{
}

bool VS_RTSPBroadcastModuleCtrl::StartBroadcast(string_view conf_name, string_view url, string_view description_utf8, string_view enabled_codecs, string_view helper_program)
{
	boost::shared_ptr<VS_RTSPBroadcastRelayMessage> msg(boost::make_shared<VS_RTSPBroadcastRelayMessage>());
	msg->MakeStartRTSPBroadcast(conf_name, url, description_utf8, enabled_codecs, helper_program);
	return SendMsg(msg, conf_name);
}

bool VS_RTSPBroadcastModuleCtrl::StopBroadcast(string_view conf_name)
{
	boost::shared_ptr<VS_RTSPBroadcastRelayMessage> msg(boost::make_shared<VS_RTSPBroadcastRelayMessage>());
	msg->MakeStopRTSPBroadcast(conf_name);
	return SendMsg(msg, conf_name);
}

bool VS_RTSPBroadcastModuleCtrl::AnnounceBroadcast(string_view conf_name, string_view announce_id, string_view url, string_view username, string_view password, bool rtp_over_tcp, string_view enabled_codecs, unsigned keepalive_timeout, unsigned retries, unsigned retry_delay)
{
	boost::shared_ptr<VS_RTSPBroadcastRelayMessage> msg(boost::make_shared<VS_RTSPBroadcastRelayMessage>());
	msg->MakeAnnounceRTSPBroadcast(conf_name, announce_id, url, username, password, rtp_over_tcp, enabled_codecs, keepalive_timeout, retries, retry_delay);
	return SendMsg(msg, conf_name);
}

bool VS_RTSPBroadcastModuleCtrl::ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase>& msg_in)
{
	std::unique_ptr<VS_RTSPBroadcastRelayMessage> msg(new VS_RTSPBroadcastRelayMessage);
	if (msg->SetMessage(msg_in->GetMess()))
	{
		switch (msg->GetMessageType())
		{
		case VS_RTSPBroadcastRelayMessage::e_AnnounceStatusReport:
			m_announce_status_report_signal(msg->GetConferenceName(), msg->GetAnnounceID(), msg->GetIsActive(), msg->GetReason());
			break;
		}
	}
	return true;
}
