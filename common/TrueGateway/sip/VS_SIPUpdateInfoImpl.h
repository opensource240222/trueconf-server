#pragma once
#include "../../SIPParserLib/VS_SIPUpdateInfoInterface.h"
#include "VS_SIPParserInfo.h"

class VS_SIPUpdateInfoImpl : public VS_SipUpdateInfoInterface
{
public:
	explicit VS_SIPUpdateInfoImpl(VS_SIPParserInfo& sipParser);
	void ResetIndexSipVia() override;
	void ClearSipVia() override;
	void SetDisplayNameMy(std::string name) override;
	void FillUriSetForEstablishedDialog(const VS_SIPURI* contact, const std::vector<VS_SIPField_RecordRoute*>& routeSet,
		const bool isClientUa) override;
	void SetSipDialogId(std::string id) override;
	bool SetSipVia(const VS_SIPField_Via* via) override;
	void SetRefresher(const REFRESHER refresh) override;
	void SetLastUpdate(std::chrono::steady_clock::time_point time) override;
	void SetIsUpdating(bool isUpdating) override;
	void SetRefreshPeriod(std::chrono::steady_clock::duration period) override;
	void SetMessageType(const eStartLineType messageType) override;
	void SetSipSequenceNumber(const int seq) override;
	void SetAliasRemote(std::string param) override;
	void SetAliasMy(std::string param) override;
	bool SetTagMy(std::string tag) override;
	void SetEpidMy(std::string id) override;
	void SetTagSip(std::string tag) override;
	void SetEpidSip(std::string id) override;
	void SetUserAgent(std::string name) override;
	void SetDisplayNameSip(std::string name) override;
	void LimitH264Level(const int level) override;
	VS_SDPField_MediaStream* MediaStream(const std::size_t index, const bool create) override;
	void SetRemoteBandwidth(const unsigned int bandwidth) override;
	void IsRequest(const bool isReq) override;
	void AlterMyBranch() override;
	void SetContentType(const eContentType type) override;
	void AuthInInvite(const bool isAuth) override;
	void ResetSipRouteIndex() override;
	int IncreaseMySequenceNumber() override;
	void SetMyBranch(std::string branch) override;
	void SetResponseCode(const int code) override;
	void SetResponseStr(std::string str) override;
	void IncreaseSdpSessionVersion() override;
	void EnableSessionTimer() override;
	void SetExpires(std::chrono::seconds expires) override;
	void ClearSipContact() override;
	bool SetSipContact(const VS_SIPField_Contact* contact) override;
	void EnableKeepAlive() override;
	void MsgAliveTick(const std::chrono::steady_clock::time_point t) override;
	void SetSipInstance(std::string) override;
private:
	VS_SIPParserInfo& sipParser_;
};
