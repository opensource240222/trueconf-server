#pragma once

#include <vector>
#include <chrono>
#include <string>

class VS_SIPField_RecordRoute;
class VS_SIPURI;
class VS_SIPField_Via;
class VS_SIPParserInfo;
class VS_SDPField_MediaStream;
class VS_SIPField_Contact;

struct TimerExtention;

enum class REFRESHER : unsigned int;
enum eStartLineType : int;
enum eContentType : int;


class VS_SipUpdateInfoInterface
{
public:
	VS_SipUpdateInfoInterface() = default;
	VS_SipUpdateInfoInterface(const VS_SipUpdateInfoInterface& other) = delete;
	virtual ~VS_SipUpdateInfoInterface() {}

	virtual void ResetIndexSipVia() = 0;
	virtual void ClearSipVia() = 0;
	virtual void SetDisplayNameMy(std::string name) = 0;
	virtual void FillUriSetForEstablishedDialog(const VS_SIPURI *contact,
		const std::vector<VS_SIPField_RecordRoute*> &routeSet, const bool isClientUa) = 0;
	virtual void SetSipDialogId(std::string id) = 0;
	virtual bool SetSipVia(const VS_SIPField_Via* via) = 0;
	virtual void SetRefresher(const REFRESHER refresh) = 0;
	virtual void SetLastUpdate(std::chrono::steady_clock::time_point time) = 0;
	virtual void SetIsUpdating(bool isUpdating) = 0;
	virtual void SetRefreshPeriod(std::chrono::steady_clock::duration period) = 0;
	virtual void SetMessageType(const eStartLineType messageType) = 0;
	virtual void SetSipSequenceNumber(const int seq) = 0;
	virtual void SetAliasRemote(std::string param) = 0;
	virtual void SetAliasMy(std::string param) = 0;
	virtual bool SetTagMy(std::string tag) = 0;
	virtual void SetEpidMy(std::string id) = 0;
	virtual void SetTagSip(std::string tag) = 0;
	virtual void SetEpidSip(std::string id) = 0;
	virtual void SetUserAgent(std::string name) = 0;
	virtual void SetDisplayNameSip(std::string name) = 0;
	virtual void LimitH264Level(const int level) = 0;
	virtual VS_SDPField_MediaStream* MediaStream(const std::size_t index, const bool create = false) = 0;
	virtual void SetRemoteBandwidth(const unsigned int bandwidth) = 0;
	virtual void IsRequest(const bool isReq) = 0;
	virtual void AlterMyBranch() = 0;
	virtual void SetContentType(const eContentType type) = 0;
	virtual void AuthInInvite(const bool isAuth) = 0;
	virtual void ResetSipRouteIndex() = 0;
	virtual int  IncreaseMySequenceNumber() = 0;
	virtual void SetMyBranch(std::string branch) = 0;
	virtual void SetResponseCode(const int code) = 0;
	virtual void SetResponseStr(std::string str) = 0;
	virtual void IncreaseSdpSessionVersion() = 0;
	virtual void EnableSessionTimer() = 0;
	virtual void SetExpires(std::chrono::seconds expires) = 0;
	virtual void ClearSipContact() = 0;
	virtual bool SetSipContact(const VS_SIPField_Contact* contact) = 0;
	virtual void EnableKeepAlive() = 0;
	virtual void MsgAliveTick(const std::chrono::steady_clock::time_point t) = 0;
	virtual void SetSipInstance(std::string) = 0;
};