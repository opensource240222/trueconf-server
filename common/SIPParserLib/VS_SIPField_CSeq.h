#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

class VS_SIPGetInfoInterface;
enum eStartLineType : int;

class VS_SIPField_CSeq: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	eStartLineType GetType() const;
	void SetType(const eStartLineType type);

	void Value(const unsigned int aValue);
	unsigned int Value() const;

	VS_SIPField_CSeq();
	~VS_SIPField_CSeq();

	int order() const override
	{
		return 70;
	}

private:
	eStartLineType iType;
	unsigned int iValue;
};

std::unique_ptr<VS_BaseField> VS_SIPField_CSeq_Instance();