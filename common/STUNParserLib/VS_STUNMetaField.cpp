#include "VS_STUNMetaField.h"
#include "../SIPParserBase/VS_Const.h"

#include <memory>

VS_STUNMetaField::VS_STUNMetaField():
	iTransactionID(0)
{
	this->iType = STUN_MESSAGE_TYPE_INVALID;
	this->iLength = 0;
}

VS_STUNMetaField::~VS_STUNMetaField()
{
	for (unsigned int i=0; i < iContainer.size(); i++)
		if ( iContainer[i] )
 			delete iContainer[i];

	iContainer.clear();

	if ( iTransactionID )
		delete iTransactionID;
}

TSIPErrorCodes VS_STUNMetaField::Decode(VS_SIPBuffer &aBuffer)
{
	iFactory = VS_STUNObjectFactory::Instance();

	if ( !iFactory )
		return TSIPErrorCodes::e_noMemory;

	if ( !iFactory->IsValid() )
	{
		return iFactory->GetLastClassError();
	}

	const unsigned long theDataSize = 20;
	auto theData = std::make_unique<char[]>(theDataSize);

// Message Type
	TSIPErrorCodes err = aBuffer.GetData(theData.get(), 2);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}

	iType = (*reinterpret_cast<short int*>(theData.get()) >> 8) + ((0xFF & *reinterpret_cast<short int*>(theData.get())) << 8);
	if ( !iType )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

// Message Length
	err = aBuffer.GetData(theData.get(), 2);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}

	iLength = (*reinterpret_cast<short int*>(theData.get()) >> 8) + ((0xFF & *reinterpret_cast<short int*>(theData.get())) << 8);
	if ( !iLength )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	memset(theData.get(), 0, theDataSize);

// Transaction ID
	err = aBuffer.GetData(theData.get(), 16);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetError(err);
		return err;
	}

	iTransactionID = theData.release();

	VS_BaseField* theField = 0;
	unsigned int theSize = 0;

	VS_ObjectFactory::CreateFieldResult create_field_res(nullptr, TSIPErrorCodes::e_null);

// *** <Decode>
    while (true)
	{
		theField = 0;

		create_field_res = iFactory->CreateField(aBuffer);
		switch (create_field_res.error_code)
		{
		case TSIPErrorCodes::e_ok:
				// Nothing do here!
			break;

		//case e_ObjectFactory:
		//		aBuffer.SkipHeader();
		//	break;

		case TSIPErrorCodes::e_EndOfBuffer:
				if ( theField )
					delete theField;

				theSize = (unsigned int) iContainer.size();
//				iContainer->GetSize(theSize);

				if ( theSize > 0 )
				{
					//int err = e_null;

					//err = CreateStaticLinks();
					//if ( e_ok != err )
					//{
					//	SetValid(false);
					//	SetError(err);
					//	return err;
					//}

					SetValid(true);
					SetError(TSIPErrorCodes::e_ok);
					return TSIPErrorCodes::e_ok;
				}
				else
				{
					SetValid(false);
					SetError(TSIPErrorCodes::e_EndOfBuffer);
					return TSIPErrorCodes::e_EndOfBuffer;
				}
			break;

		default:
			if ( theField )
				delete theField;

			return err;
		}

		if ( !theField )
			continue;

		err = theField->Decode(aBuffer);
		if ( err != TSIPErrorCodes::e_ok )
		{
			if ( theField )
				delete theField;

			return err;
		}

		iContainer.push_back(theField);
//		iContainer->AddField(theField);
	}
// *** </Decode>

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
    return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_STUNMetaField::Encode(VS_SIPBuffer &aBuffer) const
{
// Encode STUN Message Header
	if ( !IsValid() )
		return GetLastClassError();

	// Host to Network byte order
	short int theType = ((iType & 0xFF) << 8) + ((iType & 0xFF00) >> 8);
	short int theLength = ((iLength & 0xFF) << 8) + ((iLength & 0xFF00) >> 8);

	aBuffer.AddData((char*) &theType, 2);
	aBuffer.AddData((char*) &theLength, 2);

	aBuffer.AddData(iTransactionID, 16);

	unsigned int theSize = 0;
	theSize = (unsigned int) iContainer.size();

	TSIPErrorCodes err = TSIPErrorCodes::e_null;

	for (unsigned int i=0; i < theSize; i++)
	{
		err = ((VS_BaseField*) iContainer[i])->Encode(aBuffer);
		if (TSIPErrorCodes::e_ok != err)
		{
			return err;
		}
	}

    return TSIPErrorCodes::e_ok;
}