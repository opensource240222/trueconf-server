#ifndef VS_Q922FRAME_H
#define VS_Q922FRAME_H

#include <vector>

#define VS_Q922_HEADER_SIZE 3

class VS_Q922Frame
{
public:
	enum CodecType {
		SimpleCodec = 1,
		ExtendedCodec
	};
	explicit VS_Q922Frame(unsigned long informationFieldSize = 260);
	virtual ~VS_Q922Frame();

	unsigned char GetHighOrderAddressOctet() const { return theArray[0]; }
	unsigned char GetLowOrderAddressOctet() const { return theArray[1]; }
	void SetHighOrderAddressOctet(unsigned char octet) { theArray[0] = octet; }
	void SetLowOrderAddressOctet(unsigned char octet) { theArray[1] = octet; }

	unsigned char GetControlFieldOctet() const { return theArray[2]; }
	void SetControlFieldOctet(unsigned char octet) { theArray[2] = octet; }

	unsigned char *GetInformationFieldPtr() const { return (unsigned char *)(&theArray[0] + VS_Q922_HEADER_SIZE); }

	unsigned long GetInformationFieldSize() const { return informationFieldSize; }
	void SetInformationFieldSize(unsigned long size);

	/** Decodes a Q.922 frame from a given buffer, returns the success of this operation
		*/
	bool Decode(const unsigned char *data, unsigned long size, CodecType type = ExtendedCodec);

	/** Returns an estimate of the encoded size.
		The receiver will use at most the size when encoding. Returns zero if encoding will fail.
	 */
	unsigned long GetEncodedSize() const;

	/** Encodes this Q.922 frame into the given buffer.
		On return, size contains the number of octets occupied in the buffer.
		*/
	bool Encode(unsigned char *buffer, unsigned long & size, CodecType type = ExtendedCodec) const;

protected:

	unsigned long informationFieldSize;
	std::vector<unsigned char> theArray;

private:

	/** Encodes this Q.922 frame into the given buffer.
		On return, size contains the number of octets occupied in the buffer.
		Use bitPosition to determine at which bit the Q.922 FLAG sequence should begin.
		On return, bitPosition contains the bit at which the encoded stream ends.
		bitPosition shall be in the range 0-7, whereas 7 means that the FLAG sequence
		is encoded at unsigned char boundaries
		*/
	bool EncodeExtended(unsigned char *buffer, unsigned long & size, unsigned char & bitPosition) const;
	bool EncodeSimple(unsigned char *buffer, unsigned long & size) const;

	bool DecodeExtended(const unsigned char *data, unsigned long size);
	bool DecodeSimple(const unsigned char *data, unsigned long size);

	inline bool FindFlagEnd(const unsigned char *buffer, unsigned long bufferSize, unsigned long & octetIndex, unsigned char & bitIndex);
	inline unsigned char DecodeByte(const unsigned char *buffer, unsigned char *destination, unsigned long & octetIndex, unsigned char & bitIndex, unsigned char & onesCounter);
	inline unsigned char DecodeBit(const unsigned char *buffer, unsigned long & octetIndex, unsigned char & bitIndex);

	inline void EncodeOctet(unsigned char octet, unsigned char *buffer, unsigned long & octetIndex, unsigned char & bitIndex, unsigned char & onesCounter) const;
	inline void EncodeOctetNoEscape(unsigned char octet, unsigned char *buffer, unsigned long & octetIndex, unsigned char & bitIndex) const;
	inline void EncodeBit(unsigned char bit, unsigned char *buffer, unsigned long & octetIndex, unsigned char & bitIndex) const;

	inline unsigned short CalculateFCS(const unsigned char*data, unsigned long length) const;
};

#endif // Q922_H
