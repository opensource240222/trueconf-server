#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

enum eSIP_Events;

class VS_SIPField_Event: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;

	eSIP_Events Event() const;
	void Event(const eSIP_Events ev);

	VS_SIPField_Event();
	~VS_SIPField_Event();

private:
	eSIP_Events				m_event;
	bool					m_compact;
};

std::unique_ptr<VS_BaseField> VS_SIPField_Event_Instance();