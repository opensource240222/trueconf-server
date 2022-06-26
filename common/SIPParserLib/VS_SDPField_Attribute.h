#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

enum eSDP_MediaChannelDirection;

class VS_SDPField_Attribute : public VS_BaseField
{
public:
	enum eType { None, Direction, XMediaBW, XDeviceCaps };
	explicit VS_SDPField_Attribute(eType type = None);

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	void Clean() noexcept override;

	eSDP_MediaChannelDirection GetDirection() const {return direction_;}

	const static boost::regex e1;

	int order() const override
	{
		return 70;
	}
private:
	eType type_;
	eSDP_MediaChannelDirection direction_;
	std::string directionStr_;
	std::string media_bw_str_, device_caps_str_;
};

std::unique_ptr<VS_BaseField> VS_SDPField_Attribute_Instance();
