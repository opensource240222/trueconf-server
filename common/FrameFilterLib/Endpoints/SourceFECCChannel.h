#pragma once

#include "FrameFilterLib/Base/AbstractSource.h"

enum class eFeccRequestType : int32_t;

class OpalH224Handler;
class RTPPacket;

namespace ffl
{
class SourceFECCChannel: public AbstractSource
{
public:
	static std::shared_ptr<SourceFECCChannel> Create(const std::shared_ptr<OpalH224Handler>& handler);

	SourceFECCChannel(const std::shared_ptr<OpalH224Handler>& handler);
	~SourceFECCChannel() = default;

	void ProcessMessage(eFeccRequestType type, int32_t extra_param);
	void ForwardMessage(std::shared_ptr<RTPPacket>&& packet);
private:
	std::shared_ptr<OpalH224Handler> m_handler;
};
}
