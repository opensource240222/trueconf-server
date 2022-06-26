#include "VS_H281VideoSource.h"

VS_H281VideoSource::VS_H281VideoSource()
{
	// disabled camera with no options
	isEnabled = false;
	firstOctet = 0x00;
	secondOctet = 0x00;
}

VS_H281VideoSource::~VS_H281VideoSource()
{
}

void VS_H281VideoSource::SetVideoSourceNumber(unsigned char number)
{
	// only accepting the default camera types
	if (number > 5)	{
		return;
	}

	firstOctet &= 0x0f;
	firstOctet |= (number << 4) & 0xf0;
}

void VS_H281VideoSource::SetCanMotionVideo(bool flag)
{
	if (flag) {
		firstOctet |= 0x04;
	}
	else {
		firstOctet &= 0xfb;
	}
}

void VS_H281VideoSource::SetCanNormalResolutionStillImage(bool flag)
{
	if (flag) {
		firstOctet |= 0x02;
	}
	else {
		firstOctet &= 0xfd;
	}
}

void VS_H281VideoSource::SetCanDoubleResolutionStillImage(bool flag)
{
	if (flag) {
		firstOctet |= 0x01;
	}
	else {
		firstOctet &= 0xfe;
	}
}

void VS_H281VideoSource::SetCanPan(bool flag)
{
	if (flag) {
		secondOctet |= 0x80;
	}
	else {
		secondOctet &= 0x7f;
	}
}

void VS_H281VideoSource::SetCanTilt(bool flag)
{
	if (flag) {
		secondOctet |= 0x40;
	}
	else {
		secondOctet &= 0xbf;
	}
}

void VS_H281VideoSource::SetCanZoom(bool flag)
{
	if (flag) {
		secondOctet |= 0x20;
	}
	else {
		secondOctet &= 0xdf;
	}
}

void VS_H281VideoSource::SetCanFocus(bool flag)
{
	if (flag) {
		secondOctet |= 0x10;
	}
	else {
		secondOctet &= 0xef;
	}
}

void VS_H281VideoSource::Encode(unsigned char *data) const
{
	data[0] = firstOctet;
	data[1] = secondOctet;
}

bool VS_H281VideoSource::Decode(const unsigned char *data)
{
	// only accepting the standard video sources
	unsigned char videoSourceNumber = (data[0] >> 4) & 0x0f;
	if (videoSourceNumber > 5)	{
		return false;
	}

	firstOctet = data[0];
	secondOctet = data[1];

	return true;
}
