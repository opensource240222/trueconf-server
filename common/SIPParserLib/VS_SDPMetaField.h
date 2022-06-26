#pragma once

#include "../SIPParserBase/VS_BaseField.h"
#include "VS_SDPObjectFactory.h"

#include <vector>

class VS_SIPMetaContainer;
class VS_SDPField_Version;
class VS_SDPField_Connection;
class VS_SDPField_Origin;
class VS_SDPField_Bandwidth;
class VS_SDPField_MediaStream;

enum eSDP_MediaChannelDirection;

class VS_SDPMetaField: public VS_BaseField
{
public:

	// "static" links
	VS_SDPField_Version* iVersion;
	VS_SDPField_Connection* iConnection;
	VS_SDPField_Origin* iOrigin;
	VS_SDPField_Bandwidth* iBandwidth;
	std::vector<VS_SDPField_MediaStream*> iMediaStreams;
	eSDP_MediaChannelDirection iDirection;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void AddField(std::unique_ptr<VS_BaseField>&& aBaseField);
	void EraseFields(const VS_SDPObjectFactory::SDPHeader header);
	bool HasField(const VS_SDPObjectFactory::SDPHeader header);
	void Clear();

	VS_SDPMetaField();
	~VS_SDPMetaField();

protected:
	std::unique_ptr<VS_SIPMetaContainer> iContainer;

	TSIPErrorCodes CreateStaticLinks();
};