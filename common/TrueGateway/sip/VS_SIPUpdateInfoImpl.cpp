#include "VS_SIPUpdateInfoImpl.h"


VS_SIPUpdateInfoImpl::VS_SIPUpdateInfoImpl(VS_SIPParserInfo& sipParser)
	: sipParser_(sipParser)
{
}

void VS_SIPUpdateInfoImpl::ResetIndexSipVia()
{
	sipParser_.ResetIndexSIPVia();
}

void VS_SIPUpdateInfoImpl::ClearSipVia()
{
	sipParser_.ClearSIPVia();
}

void VS_SIPUpdateInfoImpl::SetDisplayNameMy(std::string name)
{
	sipParser_.SetDisplayNameMy(std::move(name));
}

void VS_SIPUpdateInfoImpl::FillUriSetForEstablishedDialog(const VS_SIPURI* contact,
	const std::vector<VS_SIPField_RecordRoute*>& routeSet,
	const bool isClientUa)
{
	sipParser_.FillUriSetForEstablishedDialog(contact, routeSet, isClientUa);
}

void VS_SIPUpdateInfoImpl::SetSipDialogId(std::string id)
{
	sipParser_.SIPDialogID(std::move(id));
}

bool VS_SIPUpdateInfoImpl::SetSipVia(const VS_SIPField_Via* via)
{
	return sipParser_.SetSIPVia(via);
}

void VS_SIPUpdateInfoImpl::SetRefresher(const REFRESHER refresh)
{
	sipParser_.GetTimerExtention().refresher = refresh;
}

void VS_SIPUpdateInfoImpl::SetLastUpdate(std::chrono::steady_clock::time_point time)
{
	sipParser_.GetTimerExtention().lastUpdate = time;
}

void VS_SIPUpdateInfoImpl::SetRefreshPeriod(std::chrono::steady_clock::duration period)
{
	sipParser_.GetTimerExtention().refreshPeriod = period;
}

void VS_SIPUpdateInfoImpl::SetMessageType(const eStartLineType messageType)
{
	sipParser_.SetMessageType(messageType);
}

void VS_SIPUpdateInfoImpl::SetSipSequenceNumber(const int seq)
{
	sipParser_.SetSIPSequenceNumber(seq);
}

void VS_SIPUpdateInfoImpl::SetIsUpdating(bool isUpdating)
{
	sipParser_.GetTimerExtention().IsUpdating = isUpdating;
}

void VS_SIPUpdateInfoImpl::SetAliasRemote(std::string param)
{
	sipParser_.SetAliasRemote(std::move(param));
}

void VS_SIPUpdateInfoImpl::SetAliasMy(std::string param)
{
	return sipParser_.SetAliasMy(std::move(param));
}

bool VS_SIPUpdateInfoImpl::SetTagMy(std::string tag)
{
	return sipParser_.SetTagMy(std::move(tag));
}

void VS_SIPUpdateInfoImpl::SetEpidMy(std::string id)
{
	sipParser_.SetEpidMy(std::move(id));
}

void VS_SIPUpdateInfoImpl::SetTagSip(std::string tag)
{
	sipParser_.SetTagSip(std::move(tag));
}

void VS_SIPUpdateInfoImpl::SetEpidSip(std::string id)
{
	sipParser_.SetEpidSip(std::move(id));
}


void VS_SIPUpdateInfoImpl::SetUserAgent(std::string name)
{
	return sipParser_.SetUserAgent(std::move(name));
}

void VS_SIPUpdateInfoImpl::SetDisplayNameSip(std::string name)
{
	sipParser_.SetDisplayNameSip(std::move(name));
}

void VS_SIPUpdateInfoImpl::LimitH264Level(const int level)
{
	sipParser_.LimitH264Level(level);
}

VS_SDPField_MediaStream* VS_SIPUpdateInfoImpl::MediaStream(const std::size_t index, const bool create)
{
	return sipParser_.MediaStream(index, create);
}

void VS_SIPUpdateInfoImpl::SetRemoteBandwidth(const unsigned int bandwidth)
{
	sipParser_.SetRemoteBandwidth(bandwidth);
}

void VS_SIPUpdateInfoImpl::IsRequest(const bool isReq)
{
	sipParser_.IsRequest(isReq);
}

void VS_SIPUpdateInfoImpl::AlterMyBranch()
{
	sipParser_.AlterMyBranch();
}

void VS_SIPUpdateInfoImpl::SetContentType(const eContentType type)
{
	sipParser_.SetContentType(type);
}

void VS_SIPUpdateInfoImpl::AuthInInvite(const bool isAuth)
{
	sipParser_.AuthInInvite(isAuth);
}

void VS_SIPUpdateInfoImpl::ResetSipRouteIndex()
{
	sipParser_.ResetSIPRouteIndex();
}

std::int32_t VS_SIPUpdateInfoImpl::IncreaseMySequenceNumber()
{
	return sipParser_.IncreaseMySequenceNumber();
}

void VS_SIPUpdateInfoImpl::SetMyBranch(std::string branch)
{
	return sipParser_.MyBranch(std::move(branch));
}

void VS_SIPUpdateInfoImpl::SetResponseCode(const int code)
{
	sipParser_.SetResponseCode(code);
}

void VS_SIPUpdateInfoImpl::SetResponseStr(std::string str)
{
	sipParser_.ResponseStr(std::move(str));
}

void VS_SIPUpdateInfoImpl::IncreaseSdpSessionVersion()
{
	sipParser_.IncreaseSDPSessionVersion();
}

void VS_SIPUpdateInfoImpl::EnableSessionTimer()
{
	sipParser_.EnableSessionTimer();
}

void VS_SIPUpdateInfoImpl::SetExpires(const std::chrono::seconds expires)
{
	sipParser_.SetExpires(expires);
}

void VS_SIPUpdateInfoImpl::ClearSipContact()
{
	sipParser_.ClearSIPContact();
}

bool VS_SIPUpdateInfoImpl::SetSipContact(const VS_SIPField_Contact* contact)
{
	return sipParser_.SetSIPContact(contact);
}

void VS_SIPUpdateInfoImpl::EnableKeepAlive()
{
	sipParser_.EnableKeepAlive();
}

void VS_SIPUpdateInfoImpl::MsgAliveTick(const std::chrono::steady_clock::time_point t)
{
	sipParser_.MsgAliveTick(t);
}

void VS_SIPUpdateInfoImpl::SetSipInstance(std::string str)
{
	sipParser_.SipInstance(std::move(str));
}
