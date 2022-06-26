#include "VS_SIPGetInfoImpl.h"


VS_SIPGetInfoImpl::VS_SIPGetInfoImpl(VS_SIPParserInfo& sip_parser)
	: sipParser_(sip_parser)
{
}

std::shared_ptr<VS_SIPAuthScheme> VS_SIPGetInfoImpl::GetAuthScheme() const
{
	return sipParser_.GetAuthScheme();
}

bool VS_SIPGetInfoImpl::IsRequest() const
{
	return sipParser_.IsRequest();
}

net::port VS_SIPGetInfoImpl::GetListenPort() const
{
	return sipParser_.GetListenPort();
}

const std::string &VS_SIPGetInfoImpl::GetViaHost() const
{
	return sipParser_.GetViaHost();
}

std::string VS_SIPGetInfoImpl::GetMyBranch() const
{
	return sipParser_.MyBranch();
}

eStartLineType VS_SIPGetInfoImpl::GetMessageType() const
{
	return sipParser_.GetMessageType();
}

bool VS_SIPGetInfoImpl::IsDirectionToSip() const
{
	return sipParser_.IsDirection_ToSIP();
}

bool VS_SIPGetInfoImpl::IsCompactHeaderAllowed() const
{
	return sipParser_.IsCompactHeaderAllowed();
}

bool VS_SIPGetInfoImpl::UseSingleBestCodec() const
{
	return sipParser_.UseSingleBestCodec();
}

const VS_SIPField_Via* VS_SIPGetInfoImpl::GetSipViaCurrent() const
{
	return sipParser_.GetSIPViaCurrent();
}

bool VS_SIPGetInfoImpl::IsKeepAliveEnabled() const
{
	return sipParser_.IsKeepAliveEnabled();
}

const net::address &VS_SIPGetInfoImpl::GetMyCsAddress() const
{
	return static_cast<const VS_SIPParserInfo&>(sipParser_).GetMyCsAddress().addr;
}

net::protocol VS_SIPGetInfoImpl::GetMyCsType() const
{
	return static_cast<const VS_SIPParserInfo&>(sipParser_).GetMyCsAddress().protocol;
}

net::port VS_SIPGetInfoImpl::GetMyCsPort() const
{
	return static_cast<const VS_SIPParserInfo&>(sipParser_).GetMyCsAddress().port;
}

const net::address & VS_SIPGetInfoImpl::GetMyMediaAddress() const
{
	return sipParser_.GetMyMediaAddress();
}

TimerExtention& VS_SIPGetInfoImpl::GetTimerExtention() const
{
	return sipParser_.GetTimerExtention();
}

std::vector<std::shared_ptr<const VS_SDPCodec>> VS_SIPGetInfoImpl::GetLocalAudioCodecs() const
{
	return sipParser_.GetLocalAudioCodecs();
}

std::vector<std::shared_ptr<const VS_SDPCodec>> VS_SIPGetInfoImpl::GetLocalVideoCodecs() const
{
	return sipParser_.GetLocalVideoCodecs();
}

std::vector<std::shared_ptr<const VS_SDPCodec>> VS_SIPGetInfoImpl::GetLocalDataCodecs() const
{
	return sipParser_.GetLocalDataCodecs();
}

bool VS_SIPGetInfoImpl::NoRtpmapForAudioStaticPayload() const
{
	return sipParser_.NoRtpmapForAudioStaticPayload();
}

bool VS_SIPGetInfoImpl::NoRtpmapForVideoStaticPayload() const
{
	return sipParser_.NoRtpmapForVideoStaticPayload();
}

bool VS_SIPGetInfoImpl::IceEnabled() const
{
	return sipParser_.ICEEnabled();
}

bool VS_SIPGetInfoImpl::SrtpEnabled() const
{
	return sipParser_.SRTPEnabled();
}

std::pair<std::uint32_t, std::uint32_t> VS_SIPGetInfoImpl::GetSsrcRangeAudio() const
{
	return sipParser_.GetSsrcRangeAudio();
}

std::pair<std::uint32_t, std::uint32_t> VS_SIPGetInfoImpl::GetSsrcRangeVideo() const
{
	return sipParser_.GetSsrcRangeVideo();
}

const std::string &VS_SIPGetInfoImpl::GetIceUfrag() const
{
	return sipParser_.IceUfrag();
}

const std::string &VS_SIPGetInfoImpl::GetIcePwd() const
{
	return sipParser_.IcePwd();
}


unsigned int VS_SIPGetInfoImpl::GetLocalBandwidth() const
{
	return sipParser_.GetLocalBandwidth();
}

unsigned int VS_SIPGetInfoImpl::GetRemoteBandwidth() const
{
	return sipParser_.GetRemoteBandwidth();
}

bool VS_SIPGetInfoImpl::IsBfcpEnabled() const
{
	return sipParser_.IsBFCPEnabled();
}

net::port VS_SIPGetInfoImpl::GetBfcpLocalPort() const
{
	return sipParser_.GetBFCPLocalPort();
}

unsigned int VS_SIPGetInfoImpl::GetBfcpSupportedRoles() const
{
	return sipParser_.GetBFCPSupportedRoles();
}

bool VS_SIPGetInfoImpl::IsH224Enabled() const
{
	return sipParser_.IsH224Enabled();
}

const std::vector<VS_SDPField_MediaStream*>& VS_SIPGetInfoImpl::MediaStreams() const
{
	return sipParser_.MediaStreams();
}

const std::string &VS_SIPGetInfoImpl::GetSrtpKey() const
{
	return sipParser_.GetSRTPKey();
}

bool VS_SIPGetInfoImpl::HaveAuthenticatedTLSConnection() const {
	return sipParser_.HaveAuthenticatedTLSConnection();
}

const std::string &VS_SIPGetInfoImpl::GetAliasMy() const
{
	return sipParser_.GetAliasMy();
}

bool VS_SIPGetInfoImpl::IsSessionTimerEnabled() const
{
	return sipParser_.IsSessionTimerEnabled();
}

bool VS_SIPGetInfoImpl::IsSessionTimerUsed() const
{
	return sipParser_.IsSessionTimerUsed();
}
bool VS_SIPGetInfoImpl::IsSessionTimerInRequire() const
{
	return sipParser_.IsSessionTimerInRequire();
}

bool VS_SIPGetInfoImpl::IsAuthInInvite() const
{
	return sipParser_.IsAuthInInvite();
}

const std::string &VS_SIPGetInfoImpl::GetTagMy() const
{
	return sipParser_.GetTagMy();
}

const std::string &VS_SIPGetInfoImpl::GetTagSip() const
{
	return sipParser_.GetTagSip();
}

bool VS_SIPGetInfoImpl::IsAnswered() const
{
	return sipParser_.IsAnswered();
}

std::size_t VS_SIPGetInfoImpl::GetSipViaSize() const
{
	return sipParser_.GetSIPViaSize();
}

VS_SIPField_Contact* VS_SIPGetInfoImpl::GetSipContact() const
{
	return sipParser_.GetSIPContact();
}

std::size_t VS_SIPGetInfoImpl::GetSipRouteSetSize() const
{
	return sipParser_.GetSIPRouteSetSize();
}

const VS_SIPURI* VS_SIPGetInfoImpl::GetNextSipRouteFromSet() const
{
	return sipParser_.GetNextSIPRouteFromSet();
}


std::chrono::steady_clock::duration VS_SIPGetInfoImpl::GetRetryAfterValue() const
{
	return sipParser_.GetRetryAfterValue();
}

const std::string &VS_SIPGetInfoImpl::GetMyExternalCsAddress() const
{
	return sipParser_.GetMyExternalCsAddress();
}

uint64_t VS_SIPGetInfoImpl::GetSdpSessionId() const
{
	return sipParser_.GetSDPSessionId();
}

unsigned VS_SIPGetInfoImpl::GetSdpSessionVersion() const
{
	return sipParser_.GetSDPSessionVersion();
}

const std::string &VS_SIPGetInfoImpl::GetUser() const
{
	return sipParser_.GetUser();
}

const std::string &VS_SIPGetInfoImpl::GetPassword() const
{
	return sipParser_.GetPassword();
}

const std::string &VS_SIPGetInfoImpl::GetSIPDialogId() const
{
	return sipParser_.SIPDialogID();
}

std::chrono::seconds VS_SIPGetInfoImpl::GetExpires() const
{
	return sipParser_.GetExpires();
}

const std::string& VS_SIPGetInfoImpl::GetContactHost() const
{
	return sipParser_.GetContactHost();
}

const std::string& VS_SIPGetInfoImpl::GetContactGruu() const
{
	return sipParser_.GetContactGruu();
}

const std::string &VS_SIPGetInfoImpl::GetSipInstance() const
{
	return sipParser_.SipInstance();
}

eContentType VS_SIPGetInfoImpl::GetContentType() const
{
	return sipParser_.GetContentType();
}

int VS_SIPGetInfoImpl::GetMySequenceNumber() const
{
	return sipParser_.GetMySequenceNumber();
}

int VS_SIPGetInfoImpl::GetSipSequenceNumber() const
{
	return sipParser_.GetSIPSequenceNumber();
}

const std::string &VS_SIPGetInfoImpl::GetEpidMy() const
{
	return sipParser_.GetEpidMy();
}

const std::string &VS_SIPGetInfoImpl::GetDisplayNameMy() const
{
	return sipParser_.GetDisplayNameMy();
}

const std::string &VS_SIPGetInfoImpl::GetAliasRemote() const
{
	return sipParser_.GetAliasRemote();
}

const std::string &VS_SIPGetInfoImpl::GetDisplayNameSip() const
{
	return sipParser_.GetDisplayNameSip();
}

const std::string &VS_SIPGetInfoImpl::GetEpidSip() const
{
	return sipParser_.GetEpidSip();
}

int VS_SIPGetInfoImpl::GetSIPProtocol() const
{
	return sipParser_.GetSIPProtocol();
}

const std::string &VS_SIPGetInfoImpl::GetSipRemoteTarget() const
{
	return  sipParser_.GetSIPRemoteTarget();
}

int VS_SIPGetInfoImpl::GetResponseCode() const
{
	return sipParser_.GetResponseCode();
}

const std::string &VS_SIPGetInfoImpl::GetResponseStr() const
{
	return sipParser_.ResponseStr();
}

const std::string& VS_SIPGetInfoImpl::GetServerUserAgent() const
{
	return sipParser_.GetServerUserAgent();
}

bool VS_SIPGetInfoImpl::IsGroupConf() const
{
	return sipParser_.IsGroupConf();
}
