#include "VS_STUNField_Common.h"
#include "../SIPParserBase/VS_Const.h"

#include <memory>

VS_STUNField_Common::VS_STUNField_Common():
	iValue(0)
{

}

VS_STUNField_Common::~VS_STUNField_Common()
{
	if ( iValue )
		delete iValue;
}

TSIPErrorCodes VS_STUNField_Common::Decode(VS_SIPBuffer &aBuffer)
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
	if ( (theType != STUN_FIELD_USERNAME) &&
		(theType != STUN_FIELD_PASSWORD) &&
		(theType != STUN_FIELD_MESSAGE_INTEGRITY)
		)
	{
		iType = STUN_FIELD_INVALID;
		SetError(TSIPErrorCodes::e_UNKNOWN);
		return TSIPErrorCodes::e_UNKNOWN;
	}

	iType = theType;
	iLength = theLength;

	theData.reset(new char[theLength+1]);
// Value
	err = aBuffer.GetData(theData.get(), theLength);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}

	///ÍÅ ÓÄÀËßÅÌ theData
	iValue = theData.release();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
    return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_STUNField_Common::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	// Host to Network byte order
	short int theType = ((iType & 0xFF) << 8) + ((iType & 0xFF00) >> 8);
	short int theLength = ((iLength & 0xFF) << 8) + ((iLength & 0xFF00) >> 8);

	aBuffer.AddData((char*) &theType, 2);
	aBuffer.AddData((char*) &theLength, 2);

	aBuffer.AddData(iValue, iLength);

    return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_BaseField> VS_STUNField_Common_Instance()
{
	return std::make_unique<VS_STUNField_Common>();
}