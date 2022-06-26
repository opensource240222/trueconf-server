#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

enum eSIP_ExtensionPack;

class VS_SIPField_Require : public VS_BaseField
{
public:
	const static boost::regex e1;
	const static boost::regex e2;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	void AddExtension(const eSIP_ExtensionPack ext);

	VS_SIPField_Require();
	VS_SIPField_Require(const VS_SIPField_Require &) = delete;
	~VS_SIPField_Require();

private:
	std::uint32_t			m_extensions;

	bool FindParam_timer(string_view aInput);
};

std::unique_ptr<VS_BaseField> VS_SIPField_Require_Instance();