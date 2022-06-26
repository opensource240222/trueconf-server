#include "VS_STUNField_Change.h"
#include "../SIPParserBase/VS_Const.h"

#include <memory>
#include <string>

VS_STUNField_Change::VS_STUNField_Change()
{
	iChangeIP = false;
	iChangePort = false;
}

VS_STUNField_Change::~VS_STUNField_Change()
{

}

TSIPErrorCodes VS_STUNField_Change::Decode(VS_SIPBuffer &aBuffer)
{
	const unsigned long theDataSize = 20;
	auto theData = std::make_unique<char[]>(theDataSize); // add '\0' by aBuffer

// Type
	TSIPErrorCodes err = aBuffer.GetData(theData.get(), 2);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}

	short int theType = (*reinterpret_cast<short int*>(theData.get()) >> 8) + ((0xFF & *reinterpret_cast<short int*>(theData.get())) << 8);
	if ( !theType )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

// Length
	err = aBuffer.GetData(theData.get(), 2);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}

	short int theLength = (*reinterpret_cast<short int*>(theData.get()) >> 8) + ((0xFF & *reinterpret_cast<short int*>(theData.get())) << 8);
	if ( !theLength )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	memset(theData.get(), 0, theDataSize);

// Check Type
	if ( theType != STUN_FIELD_CHANGE_REQUEST )
	{
		iType = STUN_FIELD_INVALID;
		SetError(TSIPErrorCodes::e_UNKNOWN);
		return TSIPErrorCodes::e_UNKNOWN;
	}

// Check Length
    if (theLength != 4)
	{
		SetError(TSIPErrorCodes::e_UNKNOWN);
		return TSIPErrorCodes::e_UNKNOWN;
	}

	iType = theType;
	iLength = theLength;



// Skip 3 bytes
	err = aBuffer.Skip(3);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}
	memset(theData.get(), 0, theDataSize);
// Value
	err = aBuffer.GetData(theData.get(), 1);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}

	iChangeIP = (theData[0] & STUN_MASK_CHANGE_IP) == STUN_MASK_CHANGE_IP;
	iChangePort = (theData[0] & STUN_MASK_CHANGE_PORT) == STUN_MASK_CHANGE_PORT;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
    return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_STUNField_Change::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	// Host to Network byte order
	short int theType = ((iType & 0xFF) << 8) + ((iType & 0xFF00) >> 8);
	short int theLength = ((iLength & 0xFF) << 8) + ((iLength & 0xFF00) >> 8);

	aBuffer.AddData((char*) &theType, 2);
	aBuffer.AddData((char*) &theLength, 2);

	unsigned int theChange = 0x0000;
	aBuffer.AddData((char*) &theChange, 2);

	if (iChangeIP)
		theChange = theChange | STUN_MASK_CHANGE_IP;

	if (iChangePort)
		theChange = theChange | STUN_MASK_CHANGE_PORT;

	// Host to Network byte order
	theChange = ((theChange & 0xFF) << 8) + ((theChange & 0xFF00) >> 8);

	aBuffer.AddData((char*) &theChange, 2);

    return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_BaseField> VS_STUNField_Change_Instance()
{
	return std::make_unique<VS_STUNField_Change>();
}