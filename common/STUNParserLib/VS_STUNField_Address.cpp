#include "VS_STUNField_Address.h"
#include "../SIPParserBase/VS_Const.h"

#include <memory>
#include <string>

VS_STUNField_Address::VS_STUNField_Address():
	iAddress(0), iFamily(0), iPort(0)
{

}

VS_STUNField_Address::~VS_STUNField_Address()
{

}

TSIPErrorCodes VS_STUNField_Address::Decode(VS_SIPBuffer &aBuffer)
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

// Check Type
	if ( (theType != STUN_FIELD_ADDRESS_MAPPED) &&
		(theType != STUN_FIELD_ADDRESS_RESPONSE) &&
		(theType != STUN_FIELD_ADDRESS_SOURCE) &&
		(theType != STUN_FIELD_ADDRESS_CHANGED) &&
		(theType != STUN_FIELD_ADDRESS_REFLECTED_FROM)
		)
	{
		iType = STUN_FIELD_INVALID;
		SetError(TSIPErrorCodes::e_UNKNOWN);
		return TSIPErrorCodes::e_UNKNOWN;
	}

// Check Length
    if (theLength != 8)
	{
		SetError(TSIPErrorCodes::e_UNKNOWN);
		return TSIPErrorCodes::e_UNKNOWN;
	}

	iType = theType;
	iLength = 8;

// Skip 1st byte
	err = aBuffer.Skip(1);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}
	memset(theData.get(), 0, theDataSize);
// Family
	err = aBuffer.GetData(theData.get(), 1);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}

	if ( theData[0] !=  0x01 )
	{
		SetError(TSIPErrorCodes::e_UNKNOWN);
		return TSIPErrorCodes::e_UNKNOWN;
	}

	iFamily = theData[0];

	memset(theData.get(), 0, theDataSize);
// Port
	err = aBuffer.GetData(theData.get(), 2);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}

	//short int thePort = (short int) ( ( (*((int*) theData)) >> 8 ) + ( (0xFF & (*((int*) theData))) << 8) );
	short int thePort = ((*reinterpret_cast<short int*>(theData.get()) >> 8) & 0xFF) + ((0xFF & *reinterpret_cast<short int*>(theData.get())) << 8);

	if ( !thePort )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	iPort = thePort;

	memset(theData.get(), 0, theDataSize);

	err = aBuffer.GetData(theData.get(), 4);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}

// IP-address
	iAddress = *reinterpret_cast<const uint32_t*>(theData.get());

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
    return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_STUNField_Address::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	// Host to Network byte order
	short int theType = ((iType & 0xFF) << 8) + ((iType & 0xFF00) >> 8);
	short int theLength = ((iLength & 0xFF) << 8) + ((iLength & 0xFF00) >> 8);

	aBuffer.AddData((char*) &theType, 2);
	aBuffer.AddData((char*) &theLength, 2);

	unsigned int theNULL = 0x00;
	aBuffer.AddData((char*) &theNULL, 1);
    aBuffer.AddData((char*) &iFamily, 1);

	// Host to Network byte order
	short int thePort = ((iPort & 0xFF) << 8) + ((iPort & 0xFF00) >> 8);
	aBuffer.AddData((char*) &thePort, 2);

// IP-address
	aBuffer.AddData((char*) &iAddress, 4);

    return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_BaseField> VS_STUNField_Address_Instance()
{
	return std::make_unique<VS_STUNField_Address>();
}