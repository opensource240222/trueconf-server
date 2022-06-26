#pragma once

#include "VS_SIPParserInfo.h"
#include "../../SIPParserLib/VS_SIPGetInfoInterface.h"

class VS_SIPGetInfoImpl : public VS_SIPGetInfoInterface
{
public:
	explicit VS_SIPGetInfoImpl(VS_SIPParserInfo& sipParser);
	~VS_SIPGetInfoImpl() = default;
	std::shared_ptr<VS_SIPAuthScheme> GetAuthScheme() const override;
	bool IsRequest() const override;
	net::port GetListenPort() const override;
	const std::string &GetViaHost() const override;
	std::string GetMyBranch() const override;
	eStartLineType GetMessageType() const override;
	bool IsDirectionToSip() const override;
	bool IsCompactHeaderAllowed() const override;
	bool UseSingleBestCodec() const override;
	const VS_SIPField_Via* GetSipViaCurrent() const override;
	bool IsKeepAliveEnabled() const override;
	TimerExtention& GetTimerExtention() const override;
	std::vector<std::shared_ptr<const VS_SDPCodec>> GetLocalAudioCodecs() const override;
	std::vector<std::shared_ptr<const VS_SDPCodec>> GetLocalVideoCodecs() const override;
	std::vector<std::shared_ptr<const VS_SDPCodec>> GetLocalDataCodecs() const override;
	bool NoRtpmapForAudioStaticPayload() const override;
	bool NoRtpmapForVideoStaticPayload() const override;
	bool IceEnabled() const override;
	bool SrtpEnabled() const override;
	const net::address &GetMyCsAddress() const override;
	net::port GetMyCsPort() const override;
	net::protocol GetMyCsType() const override;
	const net::address& GetMyMediaAddress() const override;
	std::pair<std::uint32_t, std::uint32_t> GetSsrcRangeAudio() const override;
	std::pair<std::uint32_t, std::uint32_t> GetSsrcRangeVideo() const override;
	const std::string &GetIceUfrag() const override;
	const std::string &GetIcePwd() const override;
	unsigned int GetLocalBandwidth() const override;
	unsigned int GetRemoteBandwidth() const override;
	bool IsBfcpEnabled() const override;
	net::port GetBfcpLocalPort() const override;
	unsigned int GetBfcpSupportedRoles() const override;
	bool IsH224Enabled() const override;
	const std::string &GetSrtpKey() const override;
	bool HaveAuthenticatedTLSConnection() const override;
	const std::vector<VS_SDPField_MediaStream*>& MediaStreams() const override;
	const std::string &GetAliasMy() const override;
	bool IsSessionTimerEnabled() const override;
	bool IsSessionTimerUsed() const override;
	bool IsSessionTimerInRequire() const override;
	std::size_t GetSipRouteSetSize() const override;
	bool IsAuthInInvite() const override;
	const std::string &GetTagMy() const override;
	const std::string &GetTagSip() const override;
	bool IsAnswered() const override;
	std::size_t GetSipViaSize() const override;
	VS_SIPField_Contact* GetSipContact() const override;
	const VS_SIPURI* GetNextSipRouteFromSet() const override;
	std::chrono::steady_clock::duration GetRetryAfterValue() const override;
	const std::string& GetMyExternalCsAddress() const override;
	uint64_t GetSdpSessionId() const override;
	unsigned GetSdpSessionVersion() const override;
	const std::string &GetUser() const override;
	const std::string &GetPassword() const override;
	const std::string &GetSIPDialogId() const override;
	std::chrono::seconds GetExpires() const override;
	const std::string &GetContactHost() const override;
	const std::string &GetContactGruu() const override;
	const std::string &GetSipInstance() const override;
	eContentType GetContentType() const override;
	int GetMySequenceNumber() const override;
	int GetSipSequenceNumber() const override;
	const std::string &GetEpidMy() const override;
	const std::string &GetDisplayNameMy() const override;
	const std::string &GetAliasRemote() const override;
	const std::string &GetDisplayNameSip() const override;
	const std::string &GetEpidSip() const override;
	int GetSIPProtocol() const override;
	const std::string &GetSipRemoteTarget() const override;
	int GetResponseCode() const override;
	const std::string &GetResponseStr() const override;
	bool IsGroupConf() const override;
	const std::string& GetServerUserAgent() const override;
private:
	VS_SIPParserInfo& sipParser_;
};
