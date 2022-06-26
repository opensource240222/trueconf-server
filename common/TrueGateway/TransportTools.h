#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>

class VS_ClientControlInterface;
namespace ts { struct IPool; }

/* gateway */
namespace gw {
	bool InitWithRtpControl(const std::weak_ptr<ts::IPool> &tsPool, const boost::shared_ptr<VS_ClientControlInterface> &trans, const std::string& confID, std::string& OUT_resrvationToken);
	bool ConnectReservedProxyToConf(const std::weak_ptr<ts::IPool> &tsPool, const std::string& reservationToken, const std::string& confID);
}