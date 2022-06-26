#include "std/debuglog/VS_Debug.h"
#include "VS_H281Frame.h"
#include "VS_H281Handler.h"

#define DEBUG_CURRENT_MODULE VS_DM_OTHER


VS_H281Frame::VS_H281Frame()
: VS_H224Frame(3)
{
	SetHighPriority(true);

	unsigned char *data = GetClientDataPtr();

	// Setting RequestType to StartAction
	SetRequestType(eRequestType::StartAction);

	// Setting Pan / Tilt / Zoom and Focus Off
	// Setting timeout to zero
	data[1] = 0x00;
	data[2] = 0x00;
}

VS_H281Frame::~VS_H281Frame()
{
}

void VS_H281Frame::SetRequestType(eRequestType requestType)
{
	unsigned char *data = GetClientDataPtr();

	data[0] = (unsigned char)requestType;

	switch(requestType) {

		case eRequestType::StartAction:
			SetClientDataSize(3);
			break;
		default:
			SetClientDataSize(2);
			break;
	}
}

VS_H281Frame::ePanDirection VS_H281Frame::GetPanDirection() const
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::StartAction &&
		requestType != eRequestType::ContinueAction &&
		requestType != eRequestType::StopAction)
	{
		// not valid
		return ePanDirection::IllegalPan;
	}

	unsigned char *data = GetClientDataPtr();

	return (ePanDirection)(data[1] & 0xc0);
}

void VS_H281Frame::SetPanDirection(ePanDirection direction)
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::StartAction &&
		requestType != eRequestType::ContinueAction &&
		requestType != eRequestType::StopAction)
	{
		// not valid
		return;
	}

	unsigned char *data = GetClientDataPtr();

	data[1] &= 0x3f;
	data[1] |= ((int)direction & 0xc0);
}

VS_H281Frame::eTiltDirection VS_H281Frame::GetTiltDirection() const
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::StartAction &&
		requestType != eRequestType::ContinueAction &&
		requestType != eRequestType::StopAction)
	{
		// not valid
		return eTiltDirection::IllegalTilt;
	}

	unsigned char *data = GetClientDataPtr();

	return (eTiltDirection)(data[1] & 0x30);
}

void VS_H281Frame::SetTiltDirection(eTiltDirection direction)
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::StartAction &&
		requestType != eRequestType::ContinueAction &&
		requestType != eRequestType::StopAction)
	{
		// not valid
		return;
	}

	unsigned char *data = GetClientDataPtr();

	data[1] &= 0xcf;
	data[1] |= ((int)direction & 0x30);
}

VS_H281Frame::eZoomDirection VS_H281Frame::GetZoomDirection() const
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::StartAction &&
		requestType != eRequestType::ContinueAction &&
		requestType != eRequestType::StopAction)
	{
		// not valid
		return eZoomDirection::IllegalZoom;
	}

	unsigned char *data = GetClientDataPtr();

	return (eZoomDirection)(data[1] & 0x0c);
}

void VS_H281Frame::SetZoomDirection(eZoomDirection direction)
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::StartAction &&
		requestType != eRequestType::ContinueAction &&
		requestType != eRequestType::StopAction)
	{
		// not valid
		return;
	}

	unsigned char *data = GetClientDataPtr();

	data[1] &= 0xf3;
	data[1] |= ((int)direction & 0x0c);
}

VS_H281Frame::eFocusDirection VS_H281Frame::GetFocusDirection() const
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::StartAction &&
		requestType != eRequestType::ContinueAction &&
		requestType != eRequestType::StopAction)
	{
		// not valid
		return eFocusDirection::IllegalFocus;
	}

	unsigned char *data = GetClientDataPtr();

	return (eFocusDirection)(data[1] & 0x03);
}

void VS_H281Frame::SetFocusDirection(eFocusDirection direction)
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::StartAction &&
		requestType != eRequestType::ContinueAction &&
		requestType != eRequestType::StopAction)
	{
		// not valid
		return;
	}

	unsigned char *data = GetClientDataPtr();

	data[1] &= 0xfc;
	data[1] |= ((int)direction & 0x03);
}

unsigned char VS_H281Frame::GetTimeout() const
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::StartAction) {
		return 0x00;
	}

	unsigned char *data = GetClientDataPtr();

	return (data[2] & 0x0f);
}

void VS_H281Frame::SetTimeout(unsigned char timeout)
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::StartAction) {
		return;
	}

	unsigned char *data = GetClientDataPtr();

	data[2] = (timeout & 0x0f);
}

unsigned char VS_H281Frame::GetVideoSourceNumber() const
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::SelectVideoSource &&
		requestType != eRequestType::VideoSourceSwitched)
	{
		return 0x00;
	}

	unsigned char *data = GetClientDataPtr();

	return (data[1] >> 4) & 0x0f;
}

void VS_H281Frame::SetVideoSourceNumber(unsigned char videoSourceNumber)
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::SelectVideoSource &&
		requestType != eRequestType::VideoSourceSwitched)
	{
		return;
	}

	unsigned char *data = GetClientDataPtr();

	data[1] &= 0x0f;
	data[1] |= (videoSourceNumber << 4) & 0xf0;
}

VS_H281Frame::eVideoMode VS_H281Frame::GetVideoMode() const
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::SelectVideoSource &&
		requestType != eRequestType::VideoSourceSwitched)
	{
		return eVideoMode::IllegalVideoMode;
	}

	unsigned char *data = GetClientDataPtr();

	return (eVideoMode)(data[1] & 0x03);
}

void VS_H281Frame::SetVideoMode(eVideoMode mode)
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::SelectVideoSource &&
		requestType != eRequestType::VideoSourceSwitched)
	{
		return;
	}

	unsigned char *data = GetClientDataPtr();

	data[1] &= 0xfc;
	data[1] |= ((int)mode & 0x03);
}

unsigned char VS_H281Frame::GetPresetNumber() const
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::StoreAsPreset &&
		requestType != eRequestType::ActivatePreset)
	{
		return 0x00;
	}

	unsigned char *data = GetClientDataPtr();

	return (data[1] >> 4) & 0x0f;
}

void VS_H281Frame::SetPresetNumber(unsigned char presetNumber)
{
	eRequestType requestType = GetRequestType();

	if (requestType != eRequestType::StoreAsPreset &&
		requestType != eRequestType::ActivatePreset)
	{
		return;
	}

	unsigned char *data = GetClientDataPtr();

	data[1] &= 0x0f;
	data[1] |= (presetNumber << 4) & 0xf0;
}
