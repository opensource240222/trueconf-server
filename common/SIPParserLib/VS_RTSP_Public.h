#pragma once

#include "../SIPParserBase/VS_BaseField.h"
#include "VS_RTSP_Const.h"

#include <memory>

class VS_RTSP_Public : public VS_BaseField
{
public:
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	const CommandsSet &GetValue() const;
	void SetValue(const CommandsSet& cmd_set);

private:
	CommandsSet m_Server_supported_command;
};
std::unique_ptr<VS_BaseField> VS_RTSP_Public_Instance();