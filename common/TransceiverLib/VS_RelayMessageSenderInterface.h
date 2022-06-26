#pragma once

#include "net/Address.h"
#include <boost/shared_ptr.hpp>

class VS_NetworkRelayMessageBase;

class VS_RelayMessageSenderInterface
{
public:
	virtual ~VS_RelayMessageSenderInterface(){}
	virtual bool SendMsg(const boost::shared_ptr<VS_NetworkRelayMessageBase>& mess) = 0;
	virtual net::address GetRemoteAddress() const = 0;
};