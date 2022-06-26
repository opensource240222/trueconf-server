#include "QoSSettings.h"

#include "std-generic/clib/strcasecmp.h"

#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"

#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

namespace net {
static std::mutex g_qos_settings_lock;
static std::unique_ptr<QoSSettings> g_qos_settings;

// !!! QoSSettings starts here

QoSSettings &QoSSettings::GetInstance(void)
{
	std::lock_guard<std::mutex> lock(g_qos_settings_lock);

	if (g_qos_settings == nullptr)
	{
		g_qos_settings.reset(new QoSSettings());

		if (!IsQoSAvailable())
		{
			g_qos_settings->DisableQoS();
			dprint0("QoS subsystem has been disabled because the QoS facility is not available or permitted.");
		}
	}

	return *(g_qos_settings.get());
}

QoSSettings::QoSSettings(void)
	: m_disabled(false)
{
	LoadDataFromRegistry();
}

QoSSettings::~QoSSettings(void)
{
}

QoSFlowSharedPtr QoSSettings::GetRTPQoSFlow(void)
{
	if (m_disabled)
	{
		return nullptr;
	}

	return CreateFlow(m_rtp.traffic_type, m_rtp.dscp_value);
}

QoSFlowSharedPtr QoSSettings::GetRTSPQoSFlow(bool ipv6)
{
	if (m_disabled)
	{
		return nullptr;
	}

	if (!ipv6)
	{
		return m_rtsp_flow;
	}

	return m_rtsp_flow_v6;
}

QoSFlowSharedPtr QoSSettings::GetSIPQoSFlow(bool udp, bool ipv6)
{
	if (m_disabled)
	{
		return nullptr;
	}

	if (!udp && !ipv6)
	{
		return m_sip_tcp_flow;
	}
	else if (!udp && ipv6)
	{
		return m_sip_tcp_flow_v6;
	}

	return CreateFlow(m_sip.traffic_type, m_sip.dscp_value);
}

QoSFlowSharedPtr QoSSettings::GetH323QoSFlow(bool udp, bool ipv6)
{
	if (m_disabled)
	{
		return nullptr;
	}

	if (!udp && !ipv6)
	{
		return m_h323_tcp_flow;
	}
	else if (!udp && ipv6)
	{
		return m_h323_tcp_flow_v6;
	}

	return CreateFlow(m_h323.traffic_type, m_h323.dscp_value);
}

QoSFlowSharedPtr QoSSettings::GetTCTransportQoSFlow(bool ipv6)
{
	if (m_disabled)
	{
		return nullptr;
	}

	if (!ipv6)
	{
		return m_tc_transport_flow;
	}

	return m_tc_transport_flow_v6;
}

QoSFlowSharedPtr QoSSettings::GetTCStreamQoSFlow(bool ipv6)
{
	if (m_disabled)
	{
		return nullptr;
	}

	if (!ipv6)
	{
		return m_tc_stream_flow;
	}

	return m_tc_stream_flow_v6;
}

QoSFlowSharedPtr QoSSettings::CreateFlow(const QoSTrafficType traffic_type, const uint8_t dscp_value)
{
	if (dscp_value != 0)
		return QoSFlow::Make(dscp_value);
	else if (traffic_type != QoSTrafficType::Illegal && traffic_type != QoSTrafficType::BestEffort) // best effort means no QoS marking
		return QoSFlow::Make(traffic_type);

	return nullptr; // default
}

struct QoSConstant {
	const char *name;
	const QoSTrafficType traffic_type;
};

static const QoSConstant qos_constants[] = {
	{ "best_effort", QoSTrafficType::BestEffort },
	{ "background",  QoSTrafficType::Background },
	{ "excellent_effort",  QoSTrafficType::ExcellentEffort },
	{ "audio_video", QoSTrafficType::AudioVideo },
	{ "voice",       QoSTrafficType::Voice },
	{ "control",     QoSTrafficType::Control }
};

bool QoSSettings::LoadValueFromRegistry(const char *param_name, QoSTrafficType &traffic_type, uint8_t &dscp_value)
{
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	std::unique_ptr<char, free_deleter> data;
	if (!key.IsValid())
		return false;

	if (key.GetValue(data, VS_REG_STRING_VT, param_name) != 0 && data != nullptr)
	{
		const char* str = data.get();
		QoSTrafficType type = QoSTrafficType::Illegal;
		bool found_const = false;
		bool ret = false;

		if (isdigit(str[0]) == 0)
		{
			for (const auto &v : qos_constants)
			{
				if (strcasecmp(str, v.name) == 0)
				{
					type = v.traffic_type;
					found_const = true;
					break;
				}
			}
		}

		if (found_const)
		{
			dscp_value = 0;
			traffic_type = type;
			ret = true;
		}
		else if (isdigit(str[0]))
		{
			dscp_value = static_cast<uint8_t>(atoi(str));
			traffic_type = QoSTrafficType::Illegal;
			ret = true;
		}

		return ret;
	}

	return false;
}

void QoSSettings::LoadDataFromRegistry(void)
{
	{
		uint32_t val;
		VS_RegistryKey key(false, CONFIGURATION_KEY);

		if (!key.IsValid())
			return;

		if (key.GetValue(&val, sizeof(val), VS_REG_INTEGER_VT, "QoS Disabled") > 0)
		{
			m_disabled = (val != 0);
		}
	}

	if (m_disabled)
		return;

	// RTP
	{
		m_rtp.traffic_type = QoSTrafficType::AudioVideo;
		m_rtp.dscp_value = 0;

		LoadValueFromRegistry("QoS RTP Value", m_rtp.traffic_type, m_rtp.dscp_value);
	}

	// RTSP
	{
		m_rtsp.traffic_type = QoSTrafficType::Control;
		m_rtsp.dscp_value = 0;

		LoadValueFromRegistry("QoS RTSP Value", m_rtsp.traffic_type, m_rtsp.dscp_value);
		m_rtsp_flow = CreateFlow(m_rtsp.traffic_type, m_rtsp.dscp_value);
		m_rtsp_flow_v6 = CreateFlow(m_rtsp.traffic_type, m_rtsp.dscp_value);
	}

	// SIP
	{
		m_sip.traffic_type = QoSTrafficType::Control;
		m_sip.dscp_value = 0;

		LoadValueFromRegistry("QoS SIP Value", m_sip.traffic_type, m_sip.dscp_value);
		m_sip_tcp_flow = CreateFlow(m_sip.traffic_type, m_sip.dscp_value);
		m_sip_tcp_flow_v6 = CreateFlow(m_sip.traffic_type, m_sip.dscp_value);
	}

	// H323
	{
		m_h323.traffic_type = QoSTrafficType::Control;
		m_h323.dscp_value = 0;

		LoadValueFromRegistry("QoS H323 Value", m_h323.traffic_type, m_h323.dscp_value);
		m_h323_tcp_flow = CreateFlow(m_h323.traffic_type, m_h323.dscp_value);
		m_h323_tcp_flow_v6 = CreateFlow(m_h323.traffic_type, m_h323.dscp_value);
	}

	// TrueConf Transport
	{
		m_tc_transport.traffic_type = QoSTrafficType::Control;
		m_tc_transport.dscp_value = 0;

		LoadValueFromRegistry("QoS TC Transport Value", m_tc_transport.traffic_type, m_tc_transport.dscp_value);
		m_tc_transport_flow = CreateFlow(m_tc_transport.traffic_type, m_tc_transport.dscp_value);
		m_tc_transport_flow_v6 = CreateFlow(m_tc_transport.traffic_type, m_tc_transport.dscp_value);
	}

	// TrueConf Stream
	{
		m_tc_stream.traffic_type = QoSTrafficType::AudioVideo;
		m_tc_stream.dscp_value = 0;

		LoadValueFromRegistry("QoS TC Stream Value", m_tc_stream.traffic_type, m_tc_stream.dscp_value);
		m_tc_stream_flow = CreateFlow(m_tc_stream.traffic_type, m_tc_stream.dscp_value);
		m_tc_stream_flow_v6 = CreateFlow(m_tc_stream.traffic_type, m_tc_stream.dscp_value);
	}
}


void QoSSettings::DisableQoS(bool disable)
{
	m_disabled = disable;
}

}

