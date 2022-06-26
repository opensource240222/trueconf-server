#pragma once

#include "std-generic/compat/memory.h"
#include <boost/system/error_code.hpp>

class VS_SIPMessage;

namespace sip {
class Channel;

class ChannelEventListener {
public:
	virtual ~ChannelEventListener() = default;

	virtual void OnConnectionDie(unsigned channelID) = 0;
	virtual void OnProcessMsg(const std::shared_ptr<sip::Channel>& ch, const std::shared_ptr<VS_SIPMessage>& msg) = 0;
	virtual void OnWriteEnd(unsigned channelID, const boost::system::error_code& ec) = 0;
};
}