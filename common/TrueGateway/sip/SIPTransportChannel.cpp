#include "SIPTransportChannel.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/event.h"
#include "SIPParserLib/VS_SIPMessage.h"

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY

namespace sip{

const unsigned int c_maxMsgHops = 50;

std::shared_ptr<VS_SIPMessage> Channel::GetMsg() {
	assert(m_strand.running_in_this_thread());

	uint32_t size(0);

	std::unique_ptr<unsigned char[]> msg_buf = m_queueIn.GetChannelMessage(size, e_SIP_CS);
	if (!msg_buf)
		return nullptr;
	// check for possible keep-alive ping
	if (size == 4) {
		if (msg_buf[0] == '\r' && msg_buf[1] == '\n' && msg_buf[2] == '\r' && msg_buf[3] == '\n') {
			return std::make_shared<VS_SIPMessage>();
		}
	}

	auto msg = std::make_shared<VS_SIPMessage>();
	if (TSIPErrorCodes::e_ok != msg->Decode(reinterpret_cast<char*>(msg_buf.get()), size)) {
		dstream3 << "SIP: Channel decode failed\n";
		return nullptr;
	}
	return msg;
}

void Channel::ProcessInputMsgs()
{
	assert(m_strand.running_in_this_thread());
	auto listener = m_eventListener.lock();
	if (!listener)
		return;

	std::shared_ptr<VS_SIPMessage> msg(nullptr);
	unsigned int msgHops = c_maxMsgHops;
	while (msgHops-- && (msg = GetMsg())) {
		listener->OnProcessMsg(shared_from_this(), msg);
	}
}

unsigned int sip::Channel::GetID() const
{
	return m_id;
}

std::string sip::Channel::GetLogID()
{
	vs::event done(true);
	std::string res;
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		res = LogID();
	});
	done.wait();
	return res;
}
}

#undef DEBUG_CURRENT_MODULE