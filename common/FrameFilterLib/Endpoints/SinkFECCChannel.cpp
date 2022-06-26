#include "SinkFECCChannel.h"

#include "FrameFilterLib/Base/TraceLog.h"
#include "tools/H323Gateway/Lib/h224/OpalH224Handler.h"
#include "std/cpplib/VS_Protocol.h"
#include "Transcoder/RTPPacket.h"

namespace ffl
{

std::shared_ptr<SinkFECCChannel> SinkFECCChannel::Create(
	CallbackToVCS&& onSendToVCS,
	CallbackToTerminal&& onSendToTerminal,
	const std::shared_ptr<OpalH224Handler>& handler)
{
	return std::make_shared<SinkFECCChannel>(
		std::move(onSendToVCS),
		std::move(onSendToTerminal),
		handler);
}

SinkFECCChannel::SinkFECCChannel(
	CallbackToVCS&& onSendToVCS,
	CallbackToTerminal&& onSendToTerminal,
	const std::shared_ptr<OpalH224Handler>& handler)
	: m_handler(handler)
	, m_onSendToVCS(onSendToVCS)
	, m_onSendToTerminal(onSendToTerminal)
{
	SetName("FECC sink");
}

bool SinkFECCChannel::IsCompatibleWith(const AbstractSink* sink)
{
	auto p = dynamic_cast<const SinkFECCChannel*>(sink);
	if (!p)
		return false;
	return true;
}

void SinkFECCChannel::PutFrame(const std::shared_ptr<AbstractSource>& /*src*/,
	vs::SharedBuffer buffer,
	FrameMetadata md)
{
	if (auto log = m_log.lock())
		log->TraceBuffer(this, buffer, md);
	RTPPacket packet(buffer.data<const void>(), buffer.size());
	if (!packet.IsValid()) {
		if (auto log = m_log.lock())
			log->TraceMessage(this, "error: invalid RTP-packet received!");
		return;
	}
	VS_H224Frame h224Frame = VS_H224Frame();

	if (!h224Frame.Decode(packet.Data(), packet.DataSize(), m_handler->GetQ922CodecType())) {
		return;
	}

	if (auto rtpPacket = m_handler->OnReceivedFrame(h224Frame)) {
		m_onSendToTerminal(std::move(rtpPacket));
		return;
	}

	if (h224Frame.GetClientID() == VS_H281_CLIENT_ID) // FECC over H224
	{
		eFeccRequestType type;
		long extra_param = -1;
		VS_H281Frame h281Frame;
		if (!h281Frame.Decode(packet.Data(), packet.DataSize(), m_handler->GetQ922CodecType()))
		{
			if (auto log = m_log.lock())
				log->TraceMessage(this, "error: failed to decode an H281 frame!");
			return;
		}


		if (h281Frame.GetRequestType() == VS_H281Frame::eRequestType::StartAction ||
			h281Frame.GetRequestType() == VS_H281Frame::eRequestType::ContinueAction)
		{
			if (h281Frame.GetPanDirection() == VS_H281Frame::ePanDirection::PanRight)
				type = eFeccRequestType::RIGHT;
			else if (h281Frame.GetPanDirection() == VS_H281Frame::ePanDirection::PanLeft)
				type = eFeccRequestType::LEFT;
			else if (h281Frame.GetTiltDirection() == VS_H281Frame::eTiltDirection::TiltUp)
				type = eFeccRequestType::UP;
			else if (h281Frame.GetTiltDirection() == VS_H281Frame::eTiltDirection::TiltDown)
				type = eFeccRequestType::DOWN;
			else if (h281Frame.GetZoomDirection() == VS_H281Frame::eZoomDirection::ZoomIn)
				type = eFeccRequestType::ZOOM_IN;
			else if (h281Frame.GetZoomDirection() == VS_H281Frame::eZoomDirection::ZoomOut)
				type = eFeccRequestType::ZOOM_OUT;
			else if (h281Frame.GetFocusDirection() == VS_H281Frame::eFocusDirection::FocusIn)
				type = eFeccRequestType::FOCUS_IN;
			else if (h281Frame.GetFocusDirection() == VS_H281Frame::eFocusDirection::FocusOut)
				type = eFeccRequestType::FOCUS_OUT;
			else {
				if (auto log = m_log.lock())
					log->TraceMessage(this, "warning: illegal StartAction request received!");
				return;
			}
		} else if (h281Frame.GetRequestType() == VS_H281Frame::eRequestType::StoreAsPreset) {
			type = eFeccRequestType::SAVE_PRESET;
			extra_param = h281Frame.GetPresetNumber();
		} else if (h281Frame.GetRequestType() == VS_H281Frame::eRequestType::ActivatePreset) {
			type = eFeccRequestType::USE_PRESET;
			extra_param = h281Frame.GetPresetNumber();
		} else if (h281Frame.GetRequestType() == VS_H281Frame::eRequestType::StopAction) {
			type = eFeccRequestType::STOP;
		} else {
			if (auto log = m_log.lock())
				log->TraceMessage(this, "warning: unsupported request type!");
			return;
		}
		if (m_onSendToVCS)
			m_onSendToVCS(type, extra_param);
	} else if (auto log = m_log.lock())
		log->TraceMessage(this, "error: bad client ID from camera!");
}

void SinkFECCChannel::NotifyNewFormat(
	const std::shared_ptr<AbstractSource>& /*src*/,
	const FilterFormat& format)
{
	if (auto log = m_log.lock())
		log->TraceFormat(this, format);
}

void SinkFECCChannel::Stop()
{
	Detach();
}

}// namespace ffl
