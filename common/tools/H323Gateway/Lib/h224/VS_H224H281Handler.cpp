#include "VS_H224H281Handler.h"

#include "OpalH224Handler.h"

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_OTHER

VS_H224H281Handler::VS_H224H281Handler()
	: VS_H224Handler("H281")
{
	Initialise();
}

VS_H224H281Handler::VS_H224H281Handler(const std::shared_ptr<OpalH224Handler> &theH224Handler)
	: VS_H224Handler("H281")
{
	AttachH224Handler(theH224Handler);
	Initialise();
}

void VS_H224H281Handler::Initialise()
{
	remoteHasH281 = false;
	localNumberOfPresets = 0;
	remoteNumberOfPresets = 0;

	// set correct video source numbers
	for (unsigned char srcnum = 0; srcnum < 6; srcnum++) {
		localVideoSources[srcnum].SetVideoSourceNumber(srcnum);
		remoteVideoSources[srcnum].SetVideoSourceNumber(srcnum);
	}

	// initiate the local cameras so that the main camera is enabled
	// and provides motion video (nothing more)
	localVideoSources[MainCamera].SetEnabled(true);
	localVideoSources[MainCamera].SetCanMotionVideo(true);
	//localVideoSources[MainCamera].SetCanPan(true);
	//localVideoSources[MainCamera].SetCanTilt(true);
	//localVideoSources[MainCamera].SetCanZoom(true);

	transmitFrame.SetRequestType(VS_H281Frame::eRequestType::IllegalRequest);
	transmitFrame.SetBS(true);
	transmitFrame.SetES(true);

	requestedPanDirection = VS_H281Frame::ePanDirection::NoPan;
	requestedTiltDirection = VS_H281Frame::eTiltDirection::NoTilt;
	requestedZoomDirection = VS_H281Frame::eZoomDirection::NoZoom;
	requestedFocusDirection = VS_H281Frame::eFocusDirection::NoFocus;
}

VS_H224H281Handler::~VS_H224H281Handler()
{

}

VS_H281VideoSource & VS_H224H281Handler::GetLocalVideoSource(VideoSource source)
{
	return localVideoSources[source];
}

VS_H281VideoSource & VS_H224H281Handler::GetRemoteVideoSource(VideoSource source)
{
	return remoteVideoSources[source];
}

std::shared_ptr<RTPPacket> VS_H224H281Handler::MakeStartActionRTP(VS_H281Frame::ePanDirection panDirection,
	VS_H281Frame::eTiltDirection tiltDirection,
	VS_H281Frame::eZoomDirection zoomDirection,
	VS_H281Frame::eFocusDirection focusDirection)
{
	auto action = VS_H281Frame::eRequestType::StartAction;

	if (transmitFrame.GetRequestType() != VS_H281Frame::eRequestType::IllegalRequest) {

		if (transmitFrame.GetPanDirection() == panDirection &&
			transmitFrame.GetTiltDirection() == tiltDirection &&
			transmitFrame.GetZoomDirection() == zoomDirection &&
			transmitFrame.GetFocusDirection() == focusDirection)
		{
			// change start_action to continue_action if receive frame with same direction
			// and timeout is not exceeded
			auto current_timestamp = std::chrono::high_resolution_clock::now();
			if (current_timestamp - m_last_timestamp < std::chrono::milliseconds(H281_TIMEOUT_MILLIS)) {
				action = VS_H281Frame::eRequestType::ContinueAction;
			}
			m_last_timestamp = current_timestamp;
		}
	}

	transmitFrame.SetRequestType(action);
	transmitFrame.SetPanDirection(panDirection);
	transmitFrame.SetTiltDirection(tiltDirection);
	transmitFrame.SetZoomDirection(zoomDirection);
	transmitFrame.SetFocusDirection(focusDirection);
	transmitFrame.SetTimeout(0); //800msec

	m_last_timestamp = std::chrono::high_resolution_clock::now();

	auto handler = m_h224Handler.lock();
	if (!handler)
		return std::shared_ptr<RTPPacket>(nullptr);
	transmitFrame.SetClientID(VS_H281_CLIENT_ID);
	return handler->MakeRTP(transmitFrame);
}

std::shared_ptr<RTPPacket> VS_H224H281Handler::MakeStopActionRTP()
{
	transmitFrame.SetRequestType(VS_H281Frame::eRequestType::StopAction);

	auto handler = m_h224Handler.lock();
	if (!handler)
		return {};

	transmitFrame.SetClientID(VS_H281_CLIENT_ID);
	auto packet = handler->MakeRTP(transmitFrame);

	transmitFrame.SetRequestType(VS_H281Frame::eRequestType::IllegalRequest);
	return packet;
}

std::shared_ptr<RTPPacket> VS_H224H281Handler::MakeSelectVideoSourceRTP(unsigned char videoSourceNumber, VS_H281Frame::eVideoMode videoMode)
{
	transmitFrame.SetRequestType(VS_H281Frame::eRequestType::SelectVideoSource);
	transmitFrame.SetVideoSourceNumber(videoSourceNumber);
	transmitFrame.SetVideoMode(videoMode);

	auto handler = m_h224Handler.lock();
	if (!handler)
		return {};

	transmitFrame.SetClientID(VS_H281_CLIENT_ID);
	auto packet = handler->MakeRTP(transmitFrame);

	transmitFrame.SetRequestType(VS_H281Frame::eRequestType::IllegalRequest);
	return packet;
}

std::shared_ptr<RTPPacket> VS_H224H281Handler::MakeStoreAsPreset(unsigned char presetNumber)
{
	return MakePresetImpl(presetNumber, false);
}

std::shared_ptr<RTPPacket> VS_H224H281Handler::MakeActivatePreset(unsigned char presetNumber)
{
	return MakePresetImpl(presetNumber, true);
}

std::shared_ptr<RTPPacket> VS_H224H281Handler::MakePresetImpl(
	unsigned char presetNumber,
	bool is_activate)
{
	transmitFrame.SetRequestType(is_activate ? VS_H281Frame::eRequestType::ActivatePreset : VS_H281Frame::eRequestType::StoreAsPreset);
	transmitFrame.SetPresetNumber(presetNumber);

	auto handler = m_h224Handler.lock();
	if (!handler)
		return {};

	transmitFrame.SetClientID(VS_H281_CLIENT_ID);
	auto packet = handler->MakeRTP(transmitFrame);

	transmitFrame.SetRequestType(VS_H281Frame::eRequestType::IllegalRequest);
	return packet;
}

std::shared_ptr<RTPPacket> VS_H224H281Handler::MakeExtraCapabilitiesRTP() const
{
	unsigned char capabilities[11];

	// The default implementation has no presets
	capabilities[0] = 0x00;

	unsigned long size = 1;

	for (unsigned long i = 1; i < 6; i++) {

		if (localVideoSources[i].IsEnabled()) {
			localVideoSources[i].Encode(capabilities + size);
			size += 2;
		}
	}

	auto handler = m_h224Handler.lock();
	if (!handler)
		return {};

	return handler->MakeExtraCapabilitiesMessageRTP(VS_H281_CLIENT_ID, capabilities, size);
}

void VS_H224H281Handler::SetRemoteSupport()
{
	remoteHasH281 = true;
}

void VS_H224H281Handler::SetLocalSupport()
{
	remoteHasH281 = true;
}

bool VS_H224H281Handler::HasRemoteSupport()
{
	return remoteHasH281;
}

void VS_H224H281Handler::OnReceivedExtraCapabilities(const unsigned char *capabilities, unsigned long size)
{
	remoteHasH281 = true;

	remoteNumberOfPresets = (capabilities[0] & 0x0f);

	{
		dstream4 << "FECC \tReceived Extra Capabilities [number of presets: " << (int)remoteNumberOfPresets << "]\n";
	}

	unsigned long i = 1;

	while (i < size) {

		unsigned char videoSource = (capabilities[i] >> 4) & 0x0f;

		if (videoSource <= 5) {
			auto ds = dstream4;
			ds << ">>> { [videoSource: " << (int) videoSource << "], ";
			remoteVideoSources[videoSource].SetEnabled(true);
			remoteVideoSources[videoSource].Decode(capabilities + i);
			i += 2;
			ds << "[D: " << remoteVideoSources[videoSource].CanDoubleResolutionStillImage() << "], ";
			ds << "[N: " << remoteVideoSources[videoSource].CanNormalResolutionStillImage() << "], ";
			ds << "[M: " << remoteVideoSources[videoSource].CanMotionVideo() << "], ";
			ds << "[P: " << remoteVideoSources[videoSource].CanPan() << "], ";
			ds << "[T: " << remoteVideoSources[videoSource].CanTilt() << "], ";
			ds << "[Z: " << remoteVideoSources[videoSource].CanZoom() << "], ";
			ds << "[F: " << remoteVideoSources[videoSource].CanFocus() << "] }\n";
		}
		else {
			// video sources from 6 to 15 are not supported but still need to be parsed
			auto ds = dstream4;
			ds << ">>> { [videoSource: " << (int) videoSource << "] (not supported video sources from 6 to 15) }\n";
			do {
				i++;
			} while (capabilities[i] != 0);

			// scan past the pan/tilt/zoom/focus field
			i++;
		}
	}

	OnRemoteCapabilitiesUpdated();
}

void VS_H224H281Handler::OnReceivedMessage(const VS_H224Frame & msg)
{
	const VS_H281Frame & message = (const VS_H281Frame &)msg;
	VS_H281Frame::eRequestType requestType = message.GetRequestType();

	if (requestType == VS_H281Frame::eRequestType::StartAction) {

		if (requestedPanDirection != VS_H281Frame::ePanDirection::NoPan ||
			requestedTiltDirection != VS_H281Frame::eTiltDirection::NoTilt ||
			requestedZoomDirection != VS_H281Frame::eZoomDirection::NoZoom ||
			requestedFocusDirection != VS_H281Frame::eFocusDirection::NoFocus)
		{
			// an action is already running and thus is stopped
			OnStopAction();
		}

		requestedPanDirection = message.GetPanDirection();
		requestedTiltDirection = message.GetTiltDirection();
		requestedZoomDirection = message.GetZoomDirection();
		requestedFocusDirection = message.GetFocusDirection();

		dprint4("FECC \tReceived Start Action [P: %d T: %d Z: %d]\n", (int)requestedPanDirection, (int)requestedTiltDirection, (int)requestedZoomDirection);

		OnStartAction(requestedPanDirection,
			requestedTiltDirection,
			requestedZoomDirection,
			requestedFocusDirection);
	}
	else if (requestType == VS_H281Frame::eRequestType::ContinueAction) {

		VS_H281Frame::ePanDirection panDirection = message.GetPanDirection();
		VS_H281Frame::eTiltDirection tiltDirection = message.GetTiltDirection();
		VS_H281Frame::eZoomDirection zoomDirection = message.GetZoomDirection();
		VS_H281Frame::eFocusDirection focusDirection = message.GetFocusDirection();

		if (panDirection == requestedPanDirection &&
			tiltDirection == requestedTiltDirection &&
			zoomDirection == requestedZoomDirection &&
			focusDirection == requestedFocusDirection &&
			(panDirection != VS_H281Frame::ePanDirection::NoPan ||
			tiltDirection != VS_H281Frame::eTiltDirection::NoTilt ||
			zoomDirection != VS_H281Frame::eZoomDirection::NoZoom ||
			focusDirection != VS_H281Frame::eFocusDirection::NoFocus))
		{
			// received valid continue message, but it is not supported =)
		}

	}
	else if (requestType == VS_H281Frame::eRequestType::StopAction){
		VS_H281Frame::ePanDirection panDirection = message.GetPanDirection();
		VS_H281Frame::eTiltDirection tiltDirection = message.GetTiltDirection();
		VS_H281Frame::eZoomDirection zoomDirection = message.GetZoomDirection();
		VS_H281Frame::eFocusDirection focusDirection = message.GetFocusDirection();

		// if request is valid, stop the action. Otherwise ignore
		if (panDirection == requestedPanDirection &&
			tiltDirection == requestedTiltDirection &&
			zoomDirection == requestedZoomDirection &&
			focusDirection == requestedFocusDirection &&
			(panDirection != VS_H281Frame::ePanDirection::NoPan ||
			tiltDirection != VS_H281Frame::eTiltDirection::NoTilt ||
			zoomDirection != VS_H281Frame::eZoomDirection::NoZoom ||
			focusDirection != VS_H281Frame::eFocusDirection::NoFocus))
		{
			requestedPanDirection = VS_H281Frame::ePanDirection::NoPan;
			requestedTiltDirection = VS_H281Frame::eTiltDirection::NoTilt;
			requestedZoomDirection = VS_H281Frame::eZoomDirection::NoZoom;
			requestedFocusDirection = VS_H281Frame::eFocusDirection::NoFocus;

			dprint4("FECC \tReceived Stop Action [P: %d T: %d Z: %d]\n", (int)requestedPanDirection, (int)requestedTiltDirection, (int)requestedZoomDirection);

			OnStopAction();
		}

	}
	else if (requestType == VS_H281Frame::eRequestType::SelectVideoSource) {
		OnSelectVideoSource(message.GetVideoSourceNumber(),
			message.GetVideoMode());

	}
	else if (requestType == VS_H281Frame::eRequestType::StoreAsPreset) {
		OnStoreAsPreset(message.GetPresetNumber());

	}
	else if (requestType == VS_H281Frame::eRequestType::ActivatePreset) {
		OnActivatePreset(message.GetPresetNumber());

	}
	else if (requestType == VS_H281Frame::eRequestType::VideoSourceSwitched) {
		OnVideoSourceSwitched(message.GetVideoSourceNumber(), message.GetVideoMode());

	}
	else {
		dprint3("FECC \tUnknown Request: %d\n", (int)requestType);
	}
}

void VS_H224H281Handler::OnRemoteCapabilitiesUpdated()
{
}

void VS_H224H281Handler::OnStartAction(VS_H281Frame::ePanDirection /*panDirection*/,
	VS_H281Frame::eTiltDirection /*tiltDirection*/,
	VS_H281Frame::eZoomDirection /*zoomDirection*/,
	VS_H281Frame::eFocusDirection /*focusDirection*/)
{
	// not handled
}

void VS_H224H281Handler::OnStopAction()
{
	// not handled
}

void VS_H224H281Handler::OnSelectVideoSource(unsigned char /*videoSourceNumber*/, VS_H281Frame::eVideoMode /*videoMode*/)
{
	// not handled
}

void VS_H224H281Handler::OnStoreAsPreset(unsigned char /*presetNumber*/)
{
	// not handled
}

void VS_H224H281Handler::OnActivatePreset(unsigned char /*presetNumber*/)
{
	// not handled
}

void VS_H224H281Handler::OnVideoSourceSwitched(unsigned char videoSourceNumber, VS_H281Frame::eVideoMode videoMode)
{
	auto ds = dstream4;
	ds << "FECC\tReceived VideoSourceSwitched { [videoSourceNumber: " << (int) videoSourceNumber << "], [videoMode: " << (int)videoMode << "] }\n";
}

