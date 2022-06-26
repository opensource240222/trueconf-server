#include "VS_STUNField_Error.h"
#include "../SIPParserBase/VS_Const.h"

#include <memory>

VS_STUNField_Error::VS_STUNField_Error():
	iClass(0), iNumber(0), iPhrase(0)
{

}

VS_STUNField_Error::~VS_STUNField_Error()
{
	if ( iPhrase )
		delete iPhrase;
}

TSIPErrorCodes VS_STUNField_Error::Decode(VS_SIPBuffer &aBuffer)
{
	const unsigned long theDataSize = 20;
	auto theData = std::make_unique<char[]>(theDataSize);

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
	memset(theData.get(), 0, theDataSize);
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


// Check Type
	if ( theType != STUN_FIELD_ERROR_CODE )
	{
		iType = STUN_FIELD_INVALID;
		SetError(TSIPErrorCodes::e_UNKNOWN);
		return TSIPErrorCodes::e_UNKNOWN;
	}

// Check Length
    if (theLength < 4)
	{
		SetError(TSIPErrorCodes::e_UNKNOWN);
		return TSIPErrorCodes::e_UNKNOWN;
	}

	iType = theType;
	iLength = theLength;

// Skip 2 bytes
	err = aBuffer.Skip(2);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}

	memset(theData.get(), 0, theDataSize);

// Class
	err = aBuffer.GetData(theData.get(), 1);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}

	short int theClass = *reinterpret_cast<short int*>(theData.get()) & 0x7;
	if ( (theClass < 1) || (theClass > 6) )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	iClass = theClass;

// Number
	err = aBuffer.GetData(theData.get(), 1);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}

	short int theNumber = *reinterpret_cast<short int*>(theData.get());
	if ( (theNumber < 0) || (theNumber > 99) )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	iNumber = (unsigned char) theNumber;

	theData.reset(new char[theLength + 1]);	// add '\0' by aBuffer

// Phrase
	err = aBuffer.GetData(theData.get(), theLength);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}

	iPhrase = theData.release();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
    return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_STUNField_Error::Encode(VS_SIPBuffer &aBuffer) const
{
	return TSIPErrorCodes::e_null;
}

std::unique_ptr<VS_BaseField> VS_STUNField_Error_Instance()
{
	return std::make_unique<VS_STUNField_Error>();
}