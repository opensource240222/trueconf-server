#pragma once
#include "../SIPParserBase/VS_ObjectFactory.h"
#include "../SIPParserBase/VS_SIPBuffer.h"

/*********************************************
 * Singleton
 *********************************************/

class VS_STUNObjectFactory: public VS_ObjectFactory
{
public:
	static VS_STUNObjectFactory* Instance();

	CreateFieldResult CreateField(VS_SIPBuffer &aBuffer) const override;

	virtual ~VS_STUNObjectFactory();

protected:
	VS_STUNObjectFactory();

private:
	TSIPErrorCodes Init();

	static void * iThis;
};