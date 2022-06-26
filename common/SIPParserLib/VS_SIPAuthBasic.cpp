#include "VS_SIPAuthBasic.h"
#include "VS_RTSP_ParserInfo.h"
#include "std-generic/cpplib/scope_exit.h"
#include "VS_SIPGetInfoInterface.h"
#include "std/cpplib/base64.h"
#include "std-generic/compat/memory.h"

VS_SIPAuthBasic::VS_SIPAuthBasic(void)
{
}

VS_SIPAuthBasic::~VS_SIPAuthBasic(void)
{
}

TSIPErrorCodes VS_SIPAuthBasic::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
	if ( (TSIPErrorCodes::e_ok != err) || !ptr || !ptr_sz )
	{
		SetValid(false);
		SetError(err);
		return err;
	}
	char *s = strstr(ptr.get(),"realm=");

	if (s)
	{
		realm(s + 6);
	}
	else
	{
		// base64 decode
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPAuthBasic::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( login().empty() && !realm().empty() ) // server requrst
	{
		TSIPErrorCodes err;
		if ((err = aBuffer.AddData("realm=")) == TSIPErrorCodes::e_ok)
		{
			return aBuffer.AddData(realm());
		}
		return err;
	}

	// user responce

	if (login().empty()) return TSIPErrorCodes::e_UnknownContent;


	std::string x = this->login() + ":" + this->password();
	std::size_t output_len = 0;
	base64_encode(x.c_str(), x.length(), nullptr, output_len);

	auto output = vs::make_unique_default_init<char[]>(output_len);
	const bool encode_success = base64_encode(x.c_str(), x.length(), output.get(), output_len);
	assert(encode_success);

	return aBuffer.AddData(output.get(), output_len);
}

TSIPErrorCodes VS_SIPAuthBasic::Init(const VS_SIPGetInfoInterface& call)
{
	if (call.GetUser().empty() || call.GetPassword().empty())
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_header);
		return TSIPErrorCodes::e_header;
	}

	login(call.GetUser());
	password(call.GetPassword());

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPAuthBasic::Init(VS_RTSP_ParserInfo* call)
{
	if (call->GetUser().empty() || call->GetPassword().empty())
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_header);
		return TSIPErrorCodes::e_header;
	}

	login(call->GetUser());
	password(call->GetPassword());

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

eSIP_AUTH_SCHEME VS_SIPAuthBasic::scheme() const {
	return SIP_AUTHSCHEME_BASIC;
}
