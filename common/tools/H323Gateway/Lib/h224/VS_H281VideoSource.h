#pragma once

/** This class implements a storage for which cameras are
available at both the local or remote side
*/
class VS_H281VideoSource
{
public:

	VS_H281VideoSource();
	~VS_H281VideoSource();

	bool IsEnabled() const { return isEnabled; }
	void SetEnabled(bool flag) { isEnabled = flag; }

	unsigned char GetVideoSourceNumber() const { return (firstOctet >> 4) & 0x0f; }
	void SetVideoSourceNumber(unsigned char number);

	bool CanMotionVideo() const { return (firstOctet >> 2) & 0x01; }
	void SetCanMotionVideo(bool flag);

	bool CanNormalResolutionStillImage() const { return (firstOctet >> 1) & 0x01; }
	void SetCanNormalResolutionStillImage(bool flag);

	bool CanDoubleResolutionStillImage() const { return (firstOctet & 0x01); }
	void SetCanDoubleResolutionStillImage(bool flag);

	bool CanPan() const { return (secondOctet >> 7) & 0x01; }
	void SetCanPan(bool flag);

	bool CanTilt() const { return (secondOctet >> 6) & 0x01; }
	void SetCanTilt(bool flag);

	bool CanZoom() const { return (secondOctet >> 5) & 0x01; }
	void SetCanZoom(bool flag);

	bool CanFocus() const { return (secondOctet >> 4) & 0x01; }
	void SetCanFocus(bool flag);

	void Encode(unsigned char *data) const;
	bool Decode(const unsigned char *data);

protected:

	bool isEnabled;
	unsigned char firstOctet;
	unsigned char secondOctet;

};

