#pragma once

#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include <functional>

class OpalH224Handler;
enum class eFeccRequestType: int32_t;
class RTPPacket;

namespace ffl
{
class SinkFECCChannel: public AbstractSingleSourceSink
{
public:
	typedef std::function<void(eFeccRequestType type, int32_t extra_param)> CallbackToVCS;
	typedef std::function<void(std::shared_ptr<RTPPacket>&&)> CallbackToTerminal;
	static std::shared_ptr<SinkFECCChannel> Create(
		CallbackToVCS&& onSendToVCS,
		CallbackToTerminal&& onSendToTerminal,
		const std::shared_ptr<OpalH224Handler>& handler);

	SinkFECCChannel(
		CallbackToVCS&& onSendToVCS,
		CallbackToTerminal&& onSendToTerminal,
		const std::shared_ptr<OpalH224Handler>& handler);
	bool IsCompatibleWith(const AbstractSink* sink) override;
	void PutFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer buffer, FrameMetadata md) override;
	void NotifyNewFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;
	void Stop();
private:
	std::shared_ptr<OpalH224Handler> m_handler;
	CallbackToVCS m_onSendToVCS;
	CallbackToTerminal m_onSendToTerminal;
};
}
