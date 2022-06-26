#pragma once

#include "VS_BaseField.h"
#include "VS_SIPError.h"
#include "std-generic/cpplib/StrCompare.h"
#include  <vector>
#include "std-generic/compat/functional.h"
#include  <memory>
#include "../acs/Lib/VS_AcsLog.h"
#include "std-generic/compat/map.h"

typedef std::unique_ptr<VS_BaseField>(*TInstance)();

class VS_ObjectFactory : public VS_SIPError
{
public:

	struct CreateFieldResult
	{
		std::unique_ptr<VS_BaseField> p_field = nullptr;
		TSIPErrorCodes error_code = TSIPErrorCodes::e_null;

		CreateFieldResult(std::unique_ptr<VS_BaseField>&& arg_p_field, TSIPErrorCodes arg_error_code)
			:p_field(std::move(arg_p_field))
			, error_code(arg_error_code)
		{}
	};

	struct GetInstanceResult
	{
		TInstance instance;
		TSIPErrorCodes error_code;
	};

	VS_ObjectFactory();

	GetInstanceResult GetInstance(const char* aHeader) const;

	CreateFieldResult CreateField(const char* header) const;

	virtual CreateFieldResult CreateField(VS_SIPBuffer &aBuffer) const = 0;

	virtual ~VS_ObjectFactory();

protected:
	TSIPErrorCodes AddClass(const char* aHeader, TInstance aInstance);
	TSIPErrorCodes RemoveClass(const char* aHeader);
	void RemoveAllClasses();

	struct VS_ObjectFactoryContainer
	{
		TInstance iInstancer;
		char* iHeader;
	};

private:
	VS_AcsLog*				m_error_log;
	vs::map<std::string, TInstance, vs::str_less> iContainer;
};
