#include "TransportTools.h"
#include "TrueGateway/clientcontrols/VS_ClientControlInterface.h"
#include "TransceiverLib/TransceiversPool.h"

bool gw::InitWithRtpControl(const std::weak_ptr<ts::IPool> &tsPool, const boost::shared_ptr<VS_ClientControlInterface>& trans, const std::string& confID, std::string& OUT_resrvationToken)
{
	if (!trans) return false;

	auto pool = tsPool.lock();
	if (!pool) return false;

	std::shared_ptr<VS_TransceiverProxy> proxy(nullptr);
	std::string reservationToken;

	if(!confID.empty()) proxy = pool->GetTransceiverProxy(confID, true);
	else                proxy = pool->ReserveProxy(reservationToken);
	if (!proxy) return false;

	std::shared_ptr<VS_RTPModuleControlInterface> rtpModule = proxy->GetRTPModule();
	if (!rtpModule) return false;
	trans->SetRTPModuleInterface(rtpModule);
	OUT_resrvationToken = reservationToken;

	return true;
}

bool gw::ConnectReservedProxyToConf(const std::weak_ptr<ts::IPool> &tsPool, const std::string & reservationToken, const std::string & confID)
{
	auto pool = tsPool.lock();
	if (!pool) return false;

	return pool->ConnectReservedProxyToConference(reservationToken, confID);
}
