#include "VS_STUNObjectFactory.h"
#include "../SIPParserBase/VS_Const.h"

/*********************************************
 * Fields
 *********************************************/
#include "VS_STUNField_Common.h"
#include "VS_STUNField_Address.h"
#include "VS_STUNField_Change.h"
#include "VS_STUNField_Error.h"

void * VS_STUNObjectFactory::iThis = 0;

VS_STUNObjectFactory::VS_STUNObjectFactory()
{

}

VS_STUNObjectFactory::~VS_STUNObjectFactory()
{
	if (iThis) iThis = 0;
}

VS_STUNObjectFactory* VS_STUNObjectFactory::Instance()
{
	if ( !iThis )
	{
		iThis = new VS_STUNObjectFactory;

		TSIPErrorCodes res = ((VS_STUNObjectFactory*) iThis)->Init();

		if ( res != TSIPErrorCodes::e_ok )
		{
			((VS_STUNObjectFactory*) iThis)->SetError(res);
			((VS_STUNObjectFactory*) iThis)->SetValid(false);
		}
	}

	return (VS_STUNObjectFactory*) iThis;
}

TSIPErrorCodes VS_STUNObjectFactory::Init()
{
// Common
	TSIPErrorCodes err = AddClass("COMMON", &VS_STUNField_Common_Instance);
	if ( err != TSIPErrorCodes::e_ok )
		return err;

// Address
	err = AddClass("ADDRESS", &VS_STUNField_Address_Instance);
	if ( err != TSIPErrorCodes::e_ok )
		return err;

// Change
	err = AddClass("CHANGE", &VS_STUNField_Change_Instance);
	if ( err != TSIPErrorCodes::e_ok )
		return err;

// Error
	err = AddClass("ERROR", &VS_STUNField_Error_Instance);
	if ( err != TSIPErrorCodes::e_ok )
		return err;

	//TODO: Добавить все филды VS_STUN...Field

	return err;
}

VS_ObjectFactory::CreateFieldResult VS_STUNObjectFactory::CreateField(VS_SIPBuffer &aBuffer) const
{
	char theData[2 + 1]; // add '\0'

// Type
	TSIPErrorCodes err = aBuffer.GetDataConst(theData, 2);
	if (err != TSIPErrorCodes::e_ok)
	{
		return CreateFieldResult(nullptr, err);
	}

	short int theType = (*reinterpret_cast<short int*>(theData) >> 8) + ((0xFF & *reinterpret_cast<short int*>(theData)) << 8);
	if ( !theType )
	{
		return CreateFieldResult(nullptr, TSIPErrorCodes::e_InputParam);
	}

	const char* Type = nullptr;
	switch (theType)
	{
	// Address
	case STUN_FIELD_ADDRESS_MAPPED:
	case STUN_FIELD_ADDRESS_RESPONSE:
	case STUN_FIELD_ADDRESS_SOURCE:
	case STUN_FIELD_ADDRESS_CHANGED:
			Type = "ADDRESS";
		break;

	// Change
	case STUN_FIELD_CHANGE_REQUEST:
			Type = "CHANGE";
		break;

	// Common
	case STUN_FIELD_USERNAME:
	case STUN_FIELD_PASSWORD:
	case STUN_FIELD_MESSAGE_INTEGRITY:
			Type = "COMMON";
		break;

	// Error
	case STUN_FIELD_ERROR_CODE:
			Type = "ERROR";
		break;

	default:
		return CreateFieldResult(nullptr, TSIPErrorCodes::e_UNKNOWN);
	}

	GetInstanceResult get_inst_res = GetInstance(Type);
	if (get_inst_res.error_code != TSIPErrorCodes::e_ok )
	{
		return CreateFieldResult(nullptr, get_inst_res.error_code);
	}

	return CreateFieldResult((*(get_inst_res.instance))(), TSIPErrorCodes::e_ok);
}