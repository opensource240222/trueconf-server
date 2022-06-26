#ifdef _WIN32 // win only code
#include "TransceiversPool.h"
#include "VS_ProtocolConst.h"
#include "std/cpplib/VS_WorkThreadIOCP.h"
#include "VS_TransceiverAuthenticator.h"
#include "VS_TransceiverNetChannel.h"
#include "VS_RemoteCircuitFrameTransmit.h"
#include "acs/connection/VS_ConnectionTCP.h"

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

bool ts::ProxiesPool::InitProxyComponents(ts::Components &components, bool createTransceiverProcess)
{
	return components.Init(m_acs, m_workThread, onTransceiverReady, m_ios, createTransceiverProcess);
}

ts::ProxiesPool::ProxiesPool(boost::asio::io_service& io, unsigned minFreeProxyes, unsigned maxAllowedConfByTransceiver, std::chrono::minutes maxTransceiverUnusedDuration)
	: ProxiesPoolBase(io, minFreeProxyes, maxAllowedConfByTransceiver, maxTransceiverUnusedDuration)
	, m_workThread(boost::make_shared<VS_WorkThreadIOCP>())
{
	assert(m_workThread != nullptr);
	m_workThread->Start("Relay");
}

void ts::ProxiesPool::Init(VS_AccessConnectionSystem* acs) {
	assert(acs != nullptr);

	m_acs = acs;

	for (unsigned i = 0; i < m_minFreeProxyes; ++i) {
		if(!InitNewTransceiverProxy(true))
			break;
		dstream2 << "ProxiesPool::Init push transceiver proxy to pool. Current pool size = '" << m_proxies.proxies.size() << "'\n";
	}

	ScheduleTimer();
}

bool ts::ProxiesPool::SetTCPConnection(VS_ConnectionTCP * conn, const void * in_buf, const unsigned long in_len)
{
	const auto& hs = *static_cast<const net::HandshakeHeader*>(in_buf);
	if (hs.version < VS_CIRCUIT_MIN_VERSION)
		return false;
	std::shared_ptr<VS_SetConnectionInterface> set_conn;
	if (strncmp(hs.primary_field, VS_Circuit_PrimaryField, sizeof(hs.primary_field)) == 0) {
		auto login = auth::Transceiver::GetLoginFromHandshake(string_view((const char*)in_buf, in_len));
		auto ch = GetFreeNetChannel(login);
		if (!ch)
			return false;

		ch->ConnectToOnConnectionDie([login = std::string(login), w_this=weak_from_this()]() {
			if (auto self = w_this.lock())
				self->OnNetChannelDie(login);
		});
		set_conn = std::static_pointer_cast<VS_SetConnectionInterface>(ch);
	}
	else if (strncmp(hs.primary_field, VS_FrameTransmit_PrimaryField, sizeof(hs.primary_field)) == 0) {
		const char *conf_name(nullptr);
		VS_StartFrameTransmitterMess mess;
		if (!mess.SetMessage(static_cast<const unsigned char*>(in_buf), in_len) || !(conf_name = mess.GetConferenceName())) return false;
		set_conn = std::static_pointer_cast<VS_SetConnectionInterface>(GetFrameTransmit(conf_name));
	}
	if (!set_conn || !set_conn->SetTCPConnection(conn, in_buf, in_len)) {
		delete conn;
		return false;
	}

	return true;
}

#undef DEBUG_CURRENT_MODULE
#endif