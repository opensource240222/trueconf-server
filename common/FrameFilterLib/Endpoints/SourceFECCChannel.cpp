#include "FrameFilterLib/Endpoints/SourceFECCChannel.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "std/cpplib/VS_Protocol.h"
#include "tools/H323Gateway/Lib/h224/OpalH224Handler.h"
#include "tools/H323Gateway/Lib/h224/VS_H224H281Handler.h"
#include "Transcoder/RTPPacket.h"

#include <memory>

namespace ffl
{

std::shared_ptr<SourceFECCChannel> SourceFECCChannel::Create(const std::shared_ptr<OpalH224Handler>& handler)
{
	return std::make_shared<SourceFECCChannel>(handler);
}

SourceFECCChannel::SourceFECCChannel(const std::shared_ptr<OpalH224Handler>& handler)
	:m_handler(handler)
{
	SetName("FECC source");
}

void SourceFECCChannel::ProcessMessage(eFeccRequestType type, int32_t extra_param)
{
	std::shared_ptr<RTPPacket> packet;
	switch (type) {
	case eFeccRequestType::UP: {
		packet = m_handler->MakeStartAction(
			VS_H281Frame::ePanDirection::NoPan,
			VS_H281Frame::eTiltDirection::TiltUp,
			VS_H281Frame::eZoomDirection::NoZoom,
			VS_H281Frame::eFocusDirection::NoFocus);
	} break;
	case eFeccRequestType::DOWN: {
		packet = m_handler->MakeStartAction(
			VS_H281Frame::ePanDirection::NoPan,
			VS_H281Frame::eTiltDirection::TiltDown,
			VS_H281Frame::eZoomDirection::NoZoom,
			VS_H281Frame::eFocusDirection::NoFocus);
	} break;
	case eFeccRequestType::LEFT: {
		packet = m_handler->MakeStartAction(
			VS_H281Frame::ePanDirection::PanLeft,
			VS_H281Frame::eTiltDirection:: NoTilt,
			VS_H281Frame::eZoomDirection::NoZoom,
			VS_H281Frame::eFocusDirection::NoFocus);
	} break;
	case eFeccRequestType::RIGHT: {
		packet = m_handler->MakeStartAction(
			VS_H281Frame::ePanDirection::PanRight,
			VS_H281Frame::eTiltDirection::NoTilt,
			VS_H281Frame::eZoomDirection::NoZoom,
			VS_H281Frame::eFocusDirection::NoFocus);
	} break;
	case eFeccRequestType::ZOOM_IN: {
		packet = m_handler->MakeStartAction(
			VS_H281Frame::ePanDirection::NoPan,
			VS_H281Frame::eTiltDirection::NoTilt,
			VS_H281Frame::eZoomDirection::ZoomIn,
			VS_H281Frame::eFocusDirection::NoFocus);
	} break;
	case eFeccRequestType::ZOOM_OUT: {
		packet = m_handler->MakeStartAction(
			VS_H281Frame::ePanDirection::NoPan,
			VS_H281Frame::eTiltDirection::NoTilt,
			VS_H281Frame::eZoomDirection::ZoomOut,
			VS_H281Frame::eFocusDirection::NoFocus);
	} break;
	case eFeccRequestType::FOCUS_IN: {
		packet = m_handler->MakeStartAction(
			VS_H281Frame::ePanDirection::NoPan,
			VS_H281Frame::eTiltDirection::NoTilt,
			VS_H281Frame::eZoomDirection::NoZoom,
			VS_H281Frame::eFocusDirection::FocusIn);
	} break;
	case eFeccRequestType::FOCUS_OUT: {
		packet = m_handler->MakeStartAction(
			VS_H281Frame::ePanDirection::NoPan,
			VS_H281Frame::eTiltDirection::NoTilt,
			VS_H281Frame::eZoomDirection::NoZoom,
			VS_H281Frame::eFocusDirection::FocusOut);
	} break;
	case eFeccRequestType::STOP: {
		packet = m_handler->MakeStopAction();
	} VS_FALLTHROUGH;

		// ignore next two
	case eFeccRequestType::HOME: VS_FALLTHROUGH;
	case eFeccRequestType::POSITION:
		break;

	case eFeccRequestType::USE_PRESET: {
		unsigned char preset_number = static_cast<unsigned char>(extra_param);
		if (auto h224h281handler = m_handler->GetH224H281Handler())
			packet = h224h281handler->MakeActivatePreset(preset_number);
	} break;
	case eFeccRequestType::SAVE_PRESET: {
		unsigned char preset_number = static_cast<unsigned char>(extra_param);
		if (auto h224h281handler = m_handler->GetH224H281Handler())
			packet = h224h281handler->MakeStoreAsPreset(preset_number);
	} break;
	default:
		if (auto log = m_log.lock())
			log->TraceMessage(this, "error: can't handle FECC request!");
		return;
	}
	if (!packet)
	{
		if (auto log = m_log.lock())
			log->TraceMessage(this, "error: couldn't form an RTP packet from correct data!");
			return;
	}
	vs::SharedBuffer output(packet->GetPacket(nullptr));
	packet->GetPacket(output.data());
	SendFrameToSubscribers(std::move(output), FrameMetadata::MakeCommand());
}

void SourceFECCChannel::ForwardMessage(std::shared_ptr<RTPPacket>&& packet)
{
	vs::SharedBuffer output(packet->GetPacket(nullptr));
	packet->GetPacket(output.data());
	SendFrameToSubscribers(std::move(output), FrameMetadata::MakeCommand());
}

}//namespace ffl
