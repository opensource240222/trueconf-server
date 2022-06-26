#pragma once

#include <memory>
#include <string>

class VS_TransceiverProxy;
namespace ts {
struct IPool{
	virtual ~IPool() { /*stub*/ }
	virtual std::shared_ptr<VS_TransceiverProxy> GetTransceiverProxy(const std::string& confId, bool createNewProxy = false) = 0;
	virtual std::shared_ptr<VS_TransceiverProxy> ReserveProxy(std::string& OUT_reservationToken) = 0;
	virtual bool ConnectReservedProxyToConference(const std::string& reservationToken, const std::string& confId) = 0;
	virtual void UnreserveProxy(const std::string& reservationToken) = 0;
};
}