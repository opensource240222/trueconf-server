#pragma once

#include "VS_SIPMessage.h"
#include "VS_SIPObjectFactory.h"

class VS_SIPParserInfo;

class VS_SIPRequest: public VS_SIPMessage
{
public:
	bool MakeRefreshINVITE(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeINVITE(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeACK(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo, const bool newTransaction); // ACK for 2xx responses must be send in the new transaction
	bool MakeBYE(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeCANCEL(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeINFO_FastUpdatePicture(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeINFO_DTMF(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo, const char digit);
	bool MakeREGISTER(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo, const VS_SIPObjectFactory::SIPHeader authHeader = VS_SIPObjectFactory::SIPHeader::SIPHeader_Invalid);
	bool MakeNOTIFY(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeMESSAGE(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo, string_view message, eContentType ct = CONTENTTYPE_TEXT_PLAIN);
	bool MakeOPTIONS(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo, const bool outsideOfDialog);

	void FillInfoByRequest(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo) const;

public:
	bool GenerateMyInfo(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
};