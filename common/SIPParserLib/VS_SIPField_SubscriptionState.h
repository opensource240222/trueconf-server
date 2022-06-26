#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <memory>

class VS_SIPField_SubscriptionState: public VS_BaseField
{
public:
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	VS_SIPField_SubscriptionState();
	~VS_SIPField_SubscriptionState();

private:
//	unsigned int		m_value;

	enum STATE
	{
		STATE_INVALID = 0,
		STATE_ACTIVE,
		STATE_PENDING,
		STATE_TERMINATED
	};
	STATE				m_state;

//	bool FindParam_refresher(const char* aInput);
};

std::unique_ptr<VS_BaseField> VS_SIPField_SubscriptionState_Instance();