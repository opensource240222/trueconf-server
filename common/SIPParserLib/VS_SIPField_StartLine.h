#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

class VS_SIPURI;

enum eStartLineType : int;

class VS_SIPField_StartLine: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	void Clean() noexcept override;

	int GetMessageType() const;
	void SetMessageType(int type);
	int GetSIPProto() const;
	void SetSIPProto(int sip_proto);

	/* Request */
	int GetRequestType() const;
	void SetRequestType(eStartLineType type);
	const VS_SIPURI* GetRequestURI() const;
	bool SetRequestURI(std::unique_ptr<VS_SIPURI>&& uri);

	/* Response */
	int GetResponseCode() const;
	int GetResponseCodeClass() const;
	void SetResponseCode(int code);
	const std::string &GetResponseStr() const;
	void SetResponseStr(std::string str);

	VS_SIPField_StartLine();
	~VS_SIPField_StartLine();

	int order() const override
	{
		return 10;
	}

private:
	int iMessageType;
	int iSIPProto;

	/* Request */
	eStartLineType iRequestType;
	std::unique_ptr<VS_SIPURI> iRequestURI;

	/* Response */
	int iResponseCode;
	std::string iResponseStr;
};

std::unique_ptr<VS_BaseField> VS_SIPField_StartLine_Instance();