#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

enum eSIP_ExtensionPack;

class VS_SIPField_Unsupported: public VS_BaseField
{
public:
	const static boost::regex e1;
	const static boost::regex e2;
	const static boost::regex e3;
	const static boost::regex e4;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	VS_SIPField_Unsupported & operator=(const VS_SIPField_Unsupported &src) = delete;
	void operator=(const VS_SIPField_Unsupported* src) = delete;

	void AddExtension(const eSIP_ExtensionPack ext);

	VS_SIPField_Unsupported();
	VS_SIPField_Unsupported(const VS_SIPField_Unsupported &) = delete;
	~VS_SIPField_Unsupported();

private:
	unsigned int			m_extensions;

	bool FindParam_timer(string_view aInput);
	bool FindParam_100rel(string_view aInput);
	bool FindParam_replaces(string_view aInput);
};

std::unique_ptr<VS_BaseField> VS_SIPField_Unsupported_Instance();