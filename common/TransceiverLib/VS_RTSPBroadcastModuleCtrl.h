#pragma once

#include "VS_RelayModule.h"
#include "std-generic/cpplib/string_view.h"

#include <boost/signals2/signal.hpp>
#include <boost/signals2/connection.hpp>

class VS_RTSPBroadcastModuleCtrl : public VS_RelayModule
{
public:
	VS_RTSPBroadcastModuleCtrl();
	virtual ~VS_RTSPBroadcastModuleCtrl(){};

	bool StartBroadcast(string_view conf_name, string_view url, string_view description_utf8, string_view enabled_codecs = {}, string_view helper_program = {});
	bool StopBroadcast(string_view conf_name);
	bool AnnounceBroadcast(string_view conf_name, string_view announce_id, string_view url, string_view username, string_view password, bool rtp_over_tcp, string_view enabled_codecs, unsigned keepalive_timeout, unsigned retries, unsigned retry_delay);

	template <class Slot>
	boost::signals2::connection ConnectToAnnounceStatusReport(Slot slot)
	{
		return m_announce_status_report_signal.connect(slot);
	}

private:
	virtual bool ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase>& msg);

private:
	boost::signals2::signal<void(string_view conf_name, string_view announce_id, bool is_active, string_view reason)> m_announce_status_report_signal;
};
