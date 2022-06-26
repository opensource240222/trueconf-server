#include "VS_RTSP_Public.h"
#include "VS_RTSPObjectFactory.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/compat/memory.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/find_iterator.hpp>

std::unique_ptr<VS_BaseField> VS_RTSP_Public_Instance()
{
	return vs::make_unique<VS_RTSP_Public>();
}
TSIPErrorCodes VS_RTSP_Public::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> cpInput=0;
	std::size_t size=0;
	TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(cpInput, size);
	if(TSIPErrorCodes::e_ok != err)
	{
		SetValid(false);
		SetError(err);
		return err;

	}

	if(cpInput == nullptr)
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	VS_RTSPObjectFactory* factory = VS_RTSPObjectFactory::Instance();
	assert(factory);

	std::string header(cpInput.get());
	boost::to_upper(header);
	if (!boost::starts_with(header, string_view("PUBLIC:")))
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}
	header.erase(0, 7);

	for (auto name_it = boost::make_find_iterator(header, boost::algorithm::token_finder([](char x) { return x != ' ' && x != ','; }, boost::algorithm::token_compress_on)); !name_it.eof(); ++name_it)
	{
		auto method = factory->GetMethod(std::string(name_it->begin(), name_it->end()));
		if (method != REQUEST_invalid)
			m_Server_supported_command.insert(method);
	}

	SetValid(true);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_RTSP_Public::Encode(VS_SIPBuffer &aBuffer) const
{
	if(!IsValid())
		return GetLastClassError();

	VS_RTSPObjectFactory* factory = VS_RTSPObjectFactory::Instance();
	assert(factory);

	aBuffer.AddData("PUBLIC: ");
	bool first = true;
	for (const auto& cmd: m_Server_supported_command)
	{
		const auto& method = factory->GetMethod(cmd);
		if (method.empty())
			continue;
		if (!first)
			aBuffer.AddData(", ");
		aBuffer.AddData(method);
		first = false;
	}
	aBuffer.AddData("\r\n");
	return TSIPErrorCodes::e_ok;
}

const CommandsSet &VS_RTSP_Public::GetValue() const
{
	return m_Server_supported_command;
}

void VS_RTSP_Public::SetValue(const CommandsSet& cmd_set)
{
	m_Server_supported_command = cmd_set;
}
