#pragma once

#include <chrono>

#include "VS_H224Handler.h"
#include "VS_H281VideoSource.h"
#include "VS_H281Frame.h"

/** This class implements a default H.281 handler
*/
class VS_H224H281Handler : public VS_H224Handler
{
	const unsigned H281_TIMEOUT_MILLIS = 1000;
public:

	VS_H224H281Handler();
	explicit VS_H224H281Handler(const std::shared_ptr<OpalH224Handler> &h224Handler);
	VS_H224H281Handler(const VS_H224H281Handler &rhs) = delete;
	virtual ~VS_H224H281Handler();

	enum VideoSource {
		CurrentVideoSource = 0x00,
		MainCamera = 0x01,
		AuxiliaryCamera = 0x02,
		DocumentCamera = 0x03,
		AuxiliaryDocumentCamera = 0x04,
		VideoPlaybackSource = 0x05
	};

	void Initialise();

	virtual unsigned char GetClientID() const override
	{
		return VS_H281_CLIENT_ID;
	}

	bool GetRemoteHasH281() const { return remoteHasH281; }
	void SetRemoteHasH281(bool flag) { remoteHasH281 = flag; }

	unsigned char GetLocalNumberOfPresets() const { return localNumberOfPresets; }
	void SetLocalNumberOfPresets(unsigned char presets) { localNumberOfPresets = presets; }

	unsigned char GetRemoteNumberOfPresets() const { return remoteNumberOfPresets; }

	VS_H281VideoSource & GetLocalVideoSource(VideoSource source);
	VS_H281VideoSource & GetRemoteVideoSource(VideoSource source);

	/** Causes the H.281 handler to start the desired action
	The action will continue until StopAction() is called.
	*/
//	Causes handler to make RTP packet with StartAction request. Action will continue until StopAction() is called or timeout is reached
	std::shared_ptr<RTPPacket> MakeStartActionRTP(VS_H281Frame::ePanDirection panDirection,
		VS_H281Frame::eTiltDirection tiltDirection,
		VS_H281Frame::eZoomDirection zoomDirection,
		VS_H281Frame::eFocusDirection focusDirection);

	/** Stops any action currently ongoing
	*/
	std::shared_ptr<RTPPacket> MakeStopActionRTP();

	/** Tells the remote side to select the desired video source using the
	mode specified. Does nothing if either video source or mode aren't
	available
	*/
	std::shared_ptr<RTPPacket> MakeSelectVideoSourceRTP(unsigned char videoSourceNumber, VS_H281Frame::eVideoMode videoMode);

	/** Tells the remote side to store the current camera settings as a preset
	with the preset number given
	*/
	std::shared_ptr<RTPPacket> MakeStoreAsPreset(unsigned char presetNumber);

	/** Tells the remote side to activate the given preset
	*/
	std::shared_ptr<RTPPacket> MakeActivatePreset(unsigned char presetNumber);

	/** Set Remote (receive) Support
	*/
	virtual void SetRemoteSupport() override;

	/** Set Local (transmit) Support
	*/
	virtual void SetLocalSupport() override;

	/** Has Remote Support
	*/
	virtual bool HasRemoteSupport();

	virtual void OnRemoteSupportDetected() override {};

	/** Causes the H.281 handler to send its capabilities.
	Capabilities include the number of available cameras, (default one)
	the camera abilities (default none) and the number of presets that
	can be stored (default zero)
	*/
	virtual std::shared_ptr<RTPPacket> MakeExtraCapabilitiesRTP() const override;

	/** Processing incoming frames
	*/
	virtual void OnReceivedExtraCapabilities(const unsigned char *capabilities, unsigned long size) override;
	virtual void OnReceivedMessage(const VS_H224Frame & message) override;

	/*
	* methods that subclasses can override.
	* The default handler does not implement FECC on the local side.
	* Thus, the default behaviour is to do nothing.
	*/

	/** Called each time a remote endpoint sends its capability list
	*/
	virtual void OnRemoteCapabilitiesUpdated();

	/** Indicates to start the action specified
	*/
	virtual void OnStartAction(VS_H281Frame::ePanDirection panDirection,
		VS_H281Frame::eTiltDirection tiltDirection,
		VS_H281Frame::eZoomDirection zoomDirection,
		VS_H281Frame::eFocusDirection focusDirection);

	/** Indicates to stop the action stared with OnStartAction()
	*/
	virtual void OnStopAction();

	/** Indicates to select the desired video source
	*/
	virtual void OnSelectVideoSource(unsigned char videoSourceNumber, VS_H281Frame::eVideoMode videoMode);

	/** Indicates to store the current camera settings as a preset
	*/
	virtual void OnStoreAsPreset(unsigned char presetNumber);

	/** Indicates to activate the given preset number
	*/
	virtual void OnActivatePreset(unsigned char presetNumber);

	/** Indicates to swtich to the given video source
	*/
	virtual void OnVideoSourceSwitched(unsigned char videoSourceNumber, VS_H281Frame::eVideoMode videoMode);

protected:

	void PresetImpl(unsigned char presetNumber, bool is_activate);
	std::shared_ptr<RTPPacket> MakePresetImpl(unsigned char presetNumber, bool is_activate);

	bool remoteHasH281;
	unsigned char localNumberOfPresets;
	unsigned char remoteNumberOfPresets;
	VS_H281VideoSource localVideoSources[6];
	VS_H281VideoSource remoteVideoSources[6];

	VS_H281Frame transmitFrame;

	VS_H281Frame::ePanDirection requestedPanDirection;
	VS_H281Frame::eTiltDirection requestedTiltDirection;
	VS_H281Frame::eZoomDirection requestedZoomDirection;
	VS_H281Frame::eFocusDirection requestedFocusDirection;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_last_timestamp;
};
