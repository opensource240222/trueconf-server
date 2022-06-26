#pragma once

#include "VS_SIPMessage.h"

class VS_SIPParserInfo;

class VS_SIPResponse: public VS_SIPMessage
{
public:
	VS_SIPResponse();
	~VS_SIPResponse();

	bool MakeOnUpdateResponseOK(const VS_SIPRequest * req, const VS_SIPGetInfoInterface & getInfo, VS_SipUpdateInfoInterface & updateInfo);
	bool MakeOnInviteResponseOK(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnByeResponseOK(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnByeResponseOK(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnInfoResponseOK(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnInfoResponseUnsupported(const VS_SIPRequest *req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeResponseUnsupported(const VS_SIPRequest *req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);

	bool MakeOnInviteResponseBusyHere(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnInviteResponseBusyHere(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnInviteResponseTrying(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnInviteResponseRinging(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnInviteResponseNotFound(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnInviteResponseTemporarilyUnavailable(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnInviteResponseUnauthorized(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo, const std::string& nonce);
	bool MakeOnInviteResponseServiceUnavailable(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeResponseUseProxy(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo, const std::string& nonce);
	bool MakeMovedPermanently(const VS_SIPGetInfoInterface &get_info, VS_SipUpdateInfoInterface &update_info);
	bool MakeRequestTimeout(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnRegisterResponseOK(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnRegisterResponseUnauthorized(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo, const std::string& nonce);
	bool MakeOnRegisterResponseForbidden(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnSubscribeResponseOK(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnOptionsResponseOK(const VS_SIPRequest *req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);

	bool MakeOnCancelResponseOK(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnMessageResponseOK(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnMessageResponseAccepted(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);
	bool MakeOnMessageResponseNotFound(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo);

	void FillUriSetForEstablishedDialog(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo) const;

private:
	bool MakeSimpleResponse(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo,
		const bool responseToInvite = false);
};
