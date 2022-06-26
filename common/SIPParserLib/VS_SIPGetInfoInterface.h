#pragma once

#include <chrono>
#include <vector>
#include <memory>
#include "net/Address.h"
#include "net/Port.h"
#include "net/Protocol.h"

class VS_SIPURI;
class VS_SIPField_RecordRoute;
class VS_SIPField_Via;
class VS_SDPCodec;
class VS_SDPField_MediaStream;
class VS_SIPAuthScheme;
class VS_SIPField_Contact;

struct TimerExtention;

enum eStartLineType : int;
enum eContentType : int;

class VS_SIPGetInfoInterface
{
public:
	//constructor, operator= and destructor
	VS_SIPGetInfoInterface() = default;
	VS_SIPGetInfoInterface(const VS_SIPGetInfoInterface& other) = delete;
	VS_SIPGetInfoInterface& operator=(const VS_SIPGetInfoInterface& other) = delete;
	virtual ~VS_SIPGetInfoInterface() {}
	virtual TimerExtention &GetTimerExtention() const = 0;
	virtual std::vector<std::shared_ptr<const VS_SDPCodec>> GetLocalAudioCodecs() const = 0;
	virtual std::vector<std::shared_ptr<const VS_SDPCodec>> GetLocalVideoCodecs() const = 0;
	virtual std::vector<std::shared_ptr<const VS_SDPCodec>> GetLocalDataCodecs() const = 0;
	virtual bool NoRtpmapForAudioStaticPayload() const = 0;
	virtual bool NoRtpmapForVideoStaticPayload() const = 0;
	virtual bool IceEnabled() const = 0;
	virtual bool IsRequest() const = 0;
	virtual bool SrtpEnabled() const = 0;
	virtual net::port GetListenPort() const = 0;
	virtual const std::string &GetViaHost() const = 0;
	virtual std::string GetMyBranch() const = 0;
	virtual eStartLineType GetMessageType() const = 0;
	virtual bool IsDirectionToSip() const = 0;
	virtual bool IsCompactHeaderAllowed() const = 0;
	virtual bool UseSingleBestCodec() const = 0;
	virtual const VS_SIPField_Via* GetSipViaCurrent() const = 0;
	virtual bool IsKeepAliveEnabled() const = 0;
	virtual const net::address &GetMyCsAddress() const = 0;
	virtual net::port GetMyCsPort() const = 0;
	virtual net::protocol GetMyCsType() const = 0;
	virtual const net::address& GetMyMediaAddress() const = 0;
	virtual std::pair<std::uint32_t, std::uint32_t> GetSsrcRangeAudio() const = 0;
	virtual std::pair<std::uint32_t, std::uint32_t> GetSsrcRangeVideo() const = 0;
	virtual const std::string &GetIceUfrag() const = 0;
	virtual const std::string &GetIcePwd() const = 0;
	virtual unsigned int GetLocalBandwidth() const = 0;
	virtual unsigned int GetRemoteBandwidth() const = 0;
	virtual bool IsBfcpEnabled() const = 0;
	virtual net::port GetBfcpLocalPort() const = 0;
	virtual unsigned int GetBfcpSupportedRoles() const = 0;
	virtual bool IsH224Enabled() const = 0;
	virtual const std::string &GetSrtpKey() const = 0;
	virtual bool HaveAuthenticatedTLSConnection() const = 0;
	virtual const std::vector<VS_SDPField_MediaStream*>& MediaStreams() const = 0;
	virtual std::shared_ptr<VS_SIPAuthScheme> GetAuthScheme() const = 0;
	virtual const std::string &GetAliasMy() const = 0;
	virtual bool IsSessionTimerEnabled() const = 0;;
	virtual bool IsSessionTimerUsed() const = 0;
	virtual bool IsSessionTimerInRequire() const = 0;
	virtual size_t GetSipRouteSetSize() const = 0;
	virtual bool IsAuthInInvite() const = 0;
	virtual const std::string &GetTagMy() const = 0;
	virtual const std::string &GetTagSip() const = 0;
	virtual bool IsAnswered() const = 0;
	virtual std::size_t GetSipViaSize() const = 0;
	virtual VS_SIPField_Contact* GetSipContact() const = 0;
	virtual const VS_SIPURI* GetNextSipRouteFromSet() const = 0;
	virtual std::chrono::steady_clock::duration GetRetryAfterValue() const = 0;
	virtual const std::string &GetMyExternalCsAddress() const = 0;
	virtual uint64_t GetSdpSessionId() const = 0;
	virtual unsigned GetSdpSessionVersion() const = 0;
	virtual const std::string &GetUser() const = 0;
	virtual const std::string &GetPassword() const = 0;
	virtual const std::string &GetSIPDialogId() const = 0;
	virtual std::chrono::seconds GetExpires() const = 0;
	virtual const std::string &GetContactHost() const = 0;
	virtual const std::string &GetContactGruu() const = 0;
	virtual const std::string &GetSipInstance() const = 0;
	virtual eContentType GetContentType() const = 0;
	virtual int GetMySequenceNumber() const = 0;
	virtual int GetSipSequenceNumber() const = 0;
	virtual const std::string &GetEpidMy() const = 0;
	virtual const std::string &GetDisplayNameMy() const = 0;
	virtual const std::string &GetDisplayNameSip() const = 0;
	virtual const std::string &GetEpidSip() const = 0;
	virtual const std::string &GetAliasRemote() const = 0;
	virtual int GetSIPProtocol() const = 0;
	virtual const std::string &GetSipRemoteTarget() const = 0;
	virtual int GetResponseCode() const = 0;
	virtual const std::string &GetResponseStr() const = 0;
	virtual bool IsGroupConf() const = 0;
	virtual const std::string& GetServerUserAgent() const = 0;
};