#pragma once

#include "QoS.h"

namespace net {
class QoSSettings {
public:
	static QoSSettings &GetInstance(void);

	~QoSSettings(void);

	QoSFlowSharedPtr GetRTPQoSFlow(void);
	QoSFlowSharedPtr GetRTSPQoSFlow(bool ipv6 = false);
	QoSFlowSharedPtr GetSIPQoSFlow(bool udp, bool ipv6 = false);
	QoSFlowSharedPtr GetH323QoSFlow(bool udp, bool ipv6 = false);
	QoSFlowSharedPtr GetTCTransportQoSFlow(bool ipv6 = false);
	QoSFlowSharedPtr GetTCStreamQoSFlow(bool ipv6 = false);
private:
	struct QoSValue {
		QoSTrafficType traffic_type;
		uint8_t dscp_value;
	};
private:
	QoSSettings(void);

	bool LoadValueFromRegistry(const char *param_name, QoSTrafficType &traffic_type, uint8_t &dscp_value);
	void LoadDataFromRegistry(void);

	void DisableQoS(bool disable = true);

	static QoSFlowSharedPtr CreateFlow(const QoSTrafficType traffic_type, const uint8_t dscp_value);
private:
	bool m_disabled;

	QoSValue m_rtp;
	QoSValue m_sip;
	QoSFlowSharedPtr m_sip_tcp_flow;
	QoSFlowSharedPtr m_sip_tcp_flow_v6;

	QoSValue m_h323;
	QoSFlowSharedPtr m_h323_tcp_flow;
	QoSFlowSharedPtr m_h323_tcp_flow_v6;

	QoSValue m_rtsp;
	QoSFlowSharedPtr m_rtsp_flow;
	QoSFlowSharedPtr m_rtsp_flow_v6;

	QoSValue m_tc_transport;
	QoSFlowSharedPtr m_tc_transport_flow;
	QoSFlowSharedPtr m_tc_transport_flow_v6;
	QoSValue m_tc_stream;
	QoSFlowSharedPtr m_tc_stream_flow;
	QoSFlowSharedPtr m_tc_stream_flow_v6;
};
}

