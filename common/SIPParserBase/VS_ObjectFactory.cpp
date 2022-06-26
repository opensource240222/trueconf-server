#include "VS_ObjectFactory.h"

#include <algorithm>
#include <cctype>

VS_ObjectFactory::VS_ObjectFactory() : m_error_log(0)
{

}

VS_ObjectFactory::~VS_ObjectFactory()
{
	RemoveAllClasses();
}

TSIPErrorCodes VS_ObjectFactory::AddClass(const char* aHeader, TInstance aInstance)
{
	if (!aHeader)
		return TSIPErrorCodes::e_InputParam;

	std::string str_header(aHeader);
	transform(str_header.begin(), str_header.end(), str_header.begin(), toupper);

	iContainer[str_header] = aInstance;

	this->SetError(TSIPErrorCodes::e_ok);
	this->SetValid(true);

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_ObjectFactory::RemoveClass(const char* aHeader)
{
	if (!aHeader)
		return TSIPErrorCodes::e_InputParam;
	std::string str_header(aHeader);
	transform(str_header.begin(), str_header.end(), str_header.begin(), toupper);
	auto iter = iContainer.find(str_header);

	if (iter != iContainer.end())
	{
		iContainer.erase(iter);
		return TSIPErrorCodes::e_ok;
	}
	return TSIPErrorCodes::e_ObjectFactory;
}

void VS_ObjectFactory::RemoveAllClasses()
{
	iContainer.clear();
}

VS_ObjectFactory::GetInstanceResult VS_ObjectFactory::GetInstance(const char* aHeader) const
{
	if (!aHeader)
		return GetInstanceResult{nullptr, TSIPErrorCodes::e_InputParam};

	std::string str_header(aHeader);
	transform(str_header.begin(), str_header.end(), str_header.begin(), toupper);

	auto iter = iContainer.find(str_header);

	if (iter != iContainer.end())
	{
		return GetInstanceResult{iter->second, TSIPErrorCodes::e_ok};
	}

	return GetInstanceResult{nullptr, TSIPErrorCodes::e_ObjectFactory};
}

VS_ObjectFactory::CreateFieldResult VS_ObjectFactory::CreateField(const char* header) const
{
	if (!header)
		return CreateFieldResult(nullptr, TSIPErrorCodes::e_InputParam);

	const auto res =  GetInstance(header);
	if (TSIPErrorCodes::e_ok != res.error_code)
	{
		return CreateFieldResult(nullptr, res.error_code);
	}

	return CreateFieldResult((*(res.instance))(), TSIPErrorCodes::e_ok);
}

