#ifndef VS_H224FRAME_H
#define VS_H224FRAME_H

#include "VS_Q922Frame.h"
#include <string>

#define VS_H224_HEADER_SIZE 6

#define VS_H224_BROADCAST 0x0000

class VS_H224Frame : public VS_Q922Frame
{
public:

	explicit VS_H224Frame(unsigned long clientDataSize = 254);
	virtual ~VS_H224Frame();

	bool IsHighPriority() const { return (GetLowOrderAddressOctet() == 0x71); }
	void SetHighPriority(bool flag);

	unsigned short GetDestinationTerminalAddress() const;
	void SetDestinationTerminalAddress(unsigned short destination);

	unsigned short GetSourceTerminalAddress() const;
	void SetSourceTerminalAddress(unsigned short source);

	// Only standard client IDs are supported at the moment
	unsigned char GetClientID() const;
	void SetClientID(unsigned char clientID);

	bool GetBS() const;
	void SetBS(bool bs);

	bool GetES() const;
	void SetES(bool es);

	bool GetC1() const;
	void SetC1(bool c1);

	bool GetC0() const;
	void SetC0(bool c0);

	unsigned char GetSegmentNumber() const;
	void SetSegmentNumber(unsigned char segmentNumber);

	unsigned char *GetClientDataPtr() const { return (GetInformationFieldPtr() + VS_H224_HEADER_SIZE); }

	unsigned long GetClientDataSize() const { return (GetInformationFieldSize() - VS_H224_HEADER_SIZE); }
	void SetClientDataSize(unsigned long size) { SetInformationFieldSize(size + VS_H224_HEADER_SIZE); }

	bool Decode(const unsigned char *data, unsigned long size, CodecType type = ExtendedCodec);
};

#endif // H224_H