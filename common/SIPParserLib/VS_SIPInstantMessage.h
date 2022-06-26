#pragma once

#include "../SIPParserBase/VS_BaseField.h"

class VS_SIPInstantMessage: public VS_BaseField
{
public:
	VS_SIPInstantMessage();
	~VS_SIPInstantMessage();
	const std::string &GetMessageText() const;

	TSIPErrorCodes Decode(VS_SIPBuffer &inBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &outBuffer) const override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	virtual TSIPErrorCodes Init(const VS_SIPGetInfoInterface& info, string_view message);
	virtual void Clear();

private:
	std::string m_message;
};