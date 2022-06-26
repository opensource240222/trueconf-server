#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

class VS_SIPURI;

class VS_SIPField_RecordRoute : public VS_BaseField
{
public:

	VS_SIPField_RecordRoute();
	~VS_SIPField_RecordRoute();
	VS_SIPField_RecordRoute(const VS_SIPField_RecordRoute &rhs);
	VS_SIPField_RecordRoute & operator=(const VS_SIPField_RecordRoute &rhs);

	// from VS_BaseField
	TSIPErrorCodes Decode(VS_SIPBuffer &buffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &buffer)const override;
	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;

	// from VS_SIPError
	void Clean() noexcept override;

	const std::vector<std::shared_ptr<VS_SIPURI>> &GetURIs() const;

	int order() const override
	{
		return 30;
	}

	const static boost::regex e;
	const static boost::regex e2;
private:

	std::vector<std::shared_ptr<VS_SIPURI>>	m_uri;
	bool operator!=(const VS_SIPField_RecordRoute &rhs)const;
};

std::unique_ptr<VS_BaseField> VS_SIPField_RecordRoute_Instance();
