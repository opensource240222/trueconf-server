#pragma once

#include "../SIPParserBase/VS_BaseField.h"
#include "VS_SIPObjectFactory.h"

#include <vector>

class VS_SIPMetaContainer;
class VS_SIPField_To;
class VS_SIPField_From;
class VS_SIPField_Via;
class VS_SIPField_RecordRoute;
class VS_SIPField_Contact;
class VS_SIPField_CSeq;
class VS_SIPField_CallID;
class VS_SIPField_StartLine;
class VS_SIPField_ContentType;
class VS_SIPField_ContentLength;
class VS_SIPField_Auth;
class VS_SIPField_Expires;
class VS_SIPField_Event;
class VS_SIPField_UserAgent;
class VS_SIPField_Require;
class VS_SIPField_SessionExpires;
class VS_SIPField_MinExpires;
class VS_SIPField_Supported;
class VS_SIPField_RetryAfter;
class VS_SIPField_Server;

class VS_SIPMetaField: public VS_BaseField
{
public:

	// "static" links
	VS_SIPField_To* iTo;
	VS_SIPField_From* iFrom;
	std::vector<VS_SIPField_Via*> iVia;
	std::vector<VS_SIPField_RecordRoute*> iRouteSet;
	VS_SIPField_Contact* iContact;
	VS_SIPField_CSeq* iCSeq;
	VS_SIPField_CallID* iCallID;
	VS_SIPField_StartLine* iStartLine;
	VS_SIPField_ContentType* iContentType;
	VS_SIPField_ContentLength* iContentLength;
	//VS_SIPField_Auth* iAuthHeader;
	std::vector<VS_SIPField_Auth*> iAuthHeader;
	VS_SIPField_Expires* iExpires;
	VS_SIPField_Event* iEvent;
	VS_SIPField_UserAgent* iUserAgent;
	VS_SIPField_SessionExpires* iSessionExpires;
	VS_SIPField_MinExpires *iMinExpires;
	VS_SIPField_Require *iRequire;
	VS_SIPField_Supported *iSupported;
	VS_SIPField_RetryAfter *iRetryAfterField;
	VS_SIPField_Server *iServer;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void AddField(std::unique_ptr<VS_BaseField>&& aBaseField);
	void EraseFields(const VS_SIPObjectFactory::SIPHeader header);
	bool HasField(const VS_SIPObjectFactory::SIPHeader header);
	void Clear();
	bool GetType(int &aType) const;

	VS_SIPMetaField();
	~VS_SIPMetaField();

protected:
	std::unique_ptr<VS_SIPMetaContainer>					iContainer;

	TSIPErrorCodes CreateStaticLinks();
};
