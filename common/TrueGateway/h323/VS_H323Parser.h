#pragma once
#include "TrueGateway/interfaces/VS_ParserInterface.h"
#include "TrueGateway/VS_NetworkConnectionACL.h"
#include "tools/H323Gateway/Lib/VS_H323Lib.h"
#include "tools/Server/CommonTypes.h"
#include "std-generic/cpplib/StrCompare.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

#include <string>
#include <vector>
#include <boost/make_shared.hpp>
#include "VS_H323ParserInfo.h"

class VS_H323ParserInfo;
class VS_MessageQueue;
class VS_OutputMessageQueue;
class VS_TearMessageQueue;
struct VS_Q931;
class VS_Container;
namespace net {
	class LoggerInterface;
} //namespace net

class VS_H323Parser: public VS_ParserInterface
{
public:
	typedef boost::asio::ip::tcp::endpoint tcp_endpoint_t;

private:
	void CleanParserContext(string_view dialogId, SourceClean source) override;
	std::shared_ptr<VS_H323ParserInfo> GetDefaultParserContext (const net::address &fromAddr, const net::port fromPort);

	void SendSetMediaChannels(const std::shared_ptr<VS_H323ParserInfo>& ctx) const;

	std::function<void(string_view login, string_view dialogId)> m_setUserDialogID;

	int RecvH225Buf(const void *buf, std::size_t sz, const net::address &fromAddr, net::port fromPort);

	bool MakeH225Setup(const std::shared_ptr<VS_H323ParserInfo>& info);
	bool MakeH225Alerting(string_view dialogId);
	bool MakeH225Connect(string_view dialogId);
	bool OnConnectArrived(VS_PerBuffer &aInBuffer, VS_Q931 &aQ931_In);
	bool OnSetupArrived(VS_PerBuffer &aInBuffer, VS_Q931 &aQ931_In, net::address fromAddr, net::port fromPort);
	bool OnReleaseCompleteArrived(const VS_PerBuffer &aInBuffer, const net::address &fromAddr, net::port fromPort);
	bool OnStatusInquiryArrived(VS_PerBuffer &aInBuffer);
	bool OnFacilityArrived(VS_PerBuffer &aInBuffer, VS_Q931 &aQ931_In, const net::address &fromAddr, net::port fromPort);
	void TerminateSession(const std::shared_ptr<VS_H323ParserInfo>& ctx, int term_reason = -1);

	void SerializeAndSendContainerToKostya(VS_Container &cnt);
	bool SendMSDA(const std::shared_ptr<VS_H323ParserInfo>& ctx, int our_decision); // 'our_decision' gets reversed in MSDA Message (according to specification).
	bool SendMSDR(const std::shared_ptr<VS_H323ParserInfo>& ctx, int cause);
	bool SendMSDRel(const std::shared_ptr<VS_H323ParserInfo>& ctx);

	// H245 methods
	void OnH245RawData(const void *buf, std::size_t sz, std::shared_ptr<VS_H323ParserInfo>& ctx);
	void OnH245RawMessage(VS_PerBuffer & buffer, std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool OnH245Request(const VS_H245RequestMessage* req, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool OnH245Response(const VS_H245ResponseMessage* resp, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool OnH245Command(const VS_H245CommandMessage* cmd, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool OnH245Indication(const VS_H245IndicationMessage* ind, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	void OnH245Command_FlowControlArrived(const VS_H245FlowControlCommand* cmd, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	void OnH245Command_MiscellaneousArrived(const VS_H245MiscellaneousCommand* cmd, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool OnEncryptionUpdateCommand(const VS_H245MiscellaneousCommand * cmd, const std::shared_ptr<VS_H323ParserInfo>& ctx) const;
	bool OnEncryptionUpdateAck(const VS_H245MiscellaneousCommand * cmd, const std::shared_ptr<VS_H323ParserInfo>& ctx) const;
	bool OnEncryptionUpdateReq(const VS_H245MiscellaneousCommand * cmd, const std::shared_ptr<VS_H323ParserInfo>& ctx) const;
	bool SendEncryptionUpdateAck(const std::shared_ptr<VS_H323ParserInfo>& ctx, const std::uint32_t lcNumber, const unsigned newSyncFlag) const;
	bool SendEncryptionUpdateRequest(const std::shared_ptr<VS_H323ParserInfo>& ctx, const std::uint32_t lcNumber) const;
	bool SendEncryptionUpdateCommand(const std::shared_ptr<VS_H323ParserInfo>& ctx, const std::uint32_t lcNumber, const unsigned syncFlag) const;
	bool OnH239Message(const VS_H245GenericMessage* msg, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	void SendH239PresentationTokenRequest(const std::shared_ptr<VS_H323ParserInfo>& ctx);
	void SendH239PresentationTokenResponse(bool acknowledge, unsigned terminalLabel, const std::uint32_t lcNumber, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	void SendH239PresentationTokenRelease(const std::shared_ptr<VS_H323ParserInfo>& ctx);
	void SendH239PresentationTokenIndicateOwner(const std::shared_ptr<VS_H323ParserInfo>& ctx);
	void SendH245LogicalChannelActiveIndication(unsigned lc_number, bool active, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool CreateOLCs(const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool SendTCS(const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool SendMSD(const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool CheckMSD(const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool CheckTCS(const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool RecvMSD(VS_H245MasterSlaveDetermination * msd, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool RecvTCS(VS_H245TerminalCapabilitySet * tcs, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool SendTCSA(std::uint32_t sequenceNumber, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	void SendRejectOLC(VS_H245OpenLogicalChannel* olc, const std::shared_ptr<VS_H323ParserInfo>& ctx, const unsigned int cause = VS_H245OpenLogicalChannelReject_Cause::e_dataTypeNotSupported);
	void MakeCloseLogicalChannel(const std::uint32_t num, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	void MakeCloseLogicalChannelAck(const std::uint32_t num, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool RecvOLCA(VS_H245OpenLogicalChannelAck * ack, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool RecvOLC(VS_H245OpenLogicalChannel* olc, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool CreateOLCA(std::uint32_t number, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool SendOLC(std::uint32_t number, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool RecvMSDA(VS_H245MasterSlaveDeterminationAck * msda, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool SendVFUP(std::uint32_t number, const std::shared_ptr<VS_H323ParserInfo>& ctx);

	unsigned FindAudioCapability(unsigned tag, std::uint32_t maxBr, VS_H245TerminalCapabilitySet* tcs, VS_GatewayAudioMode& mode, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	unsigned FindVideoCapability(VS_H245VideoCapability* our_cap, VS_H245TerminalCapabilitySet* tcs, VS_GatewayVideoMode& mode, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	unsigned FindSlidesVideoCapability(VS_H245VideoCapability* ourCap, VS_H245TerminalCapabilitySet* tcs, VS_GatewayVideoMode& mode, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	unsigned FindDataCapability(VS_H245DataApplicationCapability* ourCap, VS_H245TerminalCapabilitySet* tcs, VS_GatewayDataMode& mode, const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool FillVideoMode(VS_H245VideoCapability* ourCap, VS_H245VideoCapability* peerCap, VS_GatewayVideoMode& mode, const std::shared_ptr<VS_H323ParserInfo>& ctx);

	bool InitH245(const std::shared_ptr<VS_H323ParserInfo>& ctx, const net::address &remoteAddr, net::port remotePort);
	bool MakeEndSessionCommand(const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool MakeReleaseComplete(const std::shared_ptr<VS_H323ParserInfo>& ctx, int term_reason = -1);
	bool MakeStatus(const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool UpdateCallState(const std::shared_ptr<VS_H323ParserInfo>& ctx);
	bool UpdateSlideshowState(const std::shared_ptr<VS_H323ParserInfo>& ctx, bool active);

	unsigned int GetAudioIndex( const int value );
	unsigned int GetVideoIndex( const int value );
	unsigned int GetCodecIndex( const int value ) const;

	bool TryStartDialog(const std::shared_ptr<VS_H323ParserInfo>& ctx);
//	void TestDialogsReady();

	// Must be invoked at the end of call instead of m_confMethods->Hangup().
	void HangupCall(string_view dialogId);
	int DetermineMSDStatus(const std::shared_ptr<VS_H323ParserInfo>& ctx);
protected:
	std::shared_ptr<VS_ParserInfo> GetParserContextBase(string_view dialogId, bool create = false) override;
	void OnContextDestructor(string_view dialogId);

	template<class Info = VS_H323ParserInfo>
	std::shared_ptr<Info> GetParserContext(string_view dialogId, bool create=false)
	{
		//Info is extend or class VS_H323ParserInfo
		static_assert((std::is_same<VS_H323ParserInfo, Info>::value || std::is_base_of<VS_H323ParserInfo, Info>::value), "!");

		if (dialogId.empty())
			return {};

		std::lock_guard<decltype(m_ctx_lock)> lock(m_ctx_lock);
		auto it = m_ctx.find(dialogId);
		if (it != m_ctx.cend())
			return std::static_pointer_cast<Info>(it->second);

		if (create)
		{
			VS_CallConfig config = CreateCallConfig(m_remote_ip.c_str(), {});
			std::shared_ptr<Info> new_ctx = std::make_shared<Info>();
			new_ctx->SetH239Enabled(config.h323.H239Enabled.get_value_or(true));
			new_ctx->SetH224Enable(config.H224Enabled.get_value_or(true));
			new_ctx->SetH235Enabled(config.h323.EnabledH235.get_value_or(false));
			new_ctx->SetMyExternalCsAddress(config.h323.ExternalAddress);
			m_ctx_connections.emplace_back(new_ctx->ConnectToDie([this](string_view dialogId) { OnContextDestructor(dialogId); }));
			const auto res = m_ctx.emplace(std::string(dialogId), new_ctx);

			assert(res.second);
			it = res.first;

			it->second->SetMyCsAddress(m_myCsEp.addr, m_myCsEp.port);
			it->second->SetDialogID(std::string(dialogId));
			it->second->SetConferenceID(std::string(dialogId));
			it->second->SetCallIdentifier(std::string(dialogId));
			it->second->SetConfig(std::move(config));
			return std::static_pointer_cast<Info>(it->second);
		}
		return {};
	}

	// Encode dialog id to use it as a string.
	// - raw_dialog_id - a pointer to a 16-bytes array, that represents dialog_id.
	// - dialog_id - c-string - encoded representation of dialog_id that must be used in server.
	// dialog_id must be at least 16 * 2 + 1 bytes array.
	static void EncodeDialogID(const unsigned char rawDialogId[CONFERENCEID_LENGTH], char *dialogId);
	// Decode dialog id from c-string representation (dialog_id) to 16-bytes array.
	static void DecodeDialogID(const char* dialogId, unsigned char rawDialogId[CONFERENCEID_LENGTH]);

	std::shared_ptr<VS_H323ParserInfo> FindParserInfoByRemoteTarget(string_view remote);

	bool PutH245Message(VS_H245MultimediaSystemControlMessage &aMessage, const std::shared_ptr<VS_H323ParserInfo>& ctx) const;
	bool OnH245Message(const VS_H245MultimediaSystemControlMessage* msg, const std::shared_ptr<VS_H323ParserInfo>& ctx);
public:
	// Default port used for call signalling.
	const static net::port DEFAULT_H225CS_PORT;

	virtual ~VS_H323Parser(void);

	int SetRecvBuf(const void *buf, const std::size_t sz, const VS_ChannelID channelId,
		const net::address &remoteAddr, net::port remotePort, const net::address &localAddr, net::port localPort) override;

	int GetBufForSend(void *buf, std::size_t &sz, const VS_ChannelID channelId,
		const net::address &remoteAddr, net::port remotePort, const net::address &localAddr, net::port localPort) override;

	acs::Response Protocol(const void *buf, std::size_t sz) override;
	std::string NewDialogID(string_view sipTo, string_view dtmf, const VS_CallConfig &config, string_view myName = {}) override;
	std::string SetNewDialogTest(string_view newDialog, string_view sipTo, string_view dtmf,
		const VS_CallConfig &config, string_view myName = {}) override;

	void Shutdown() override;
	void Timeout() override;
	bool SetMediaChannels(string_view dialogId, const std::vector<VS_MediaChannelInfo>& channels, const std::string& existingConfID, std::int32_t bandwRcv = 0) override;
	bool FillMediaChannels(string_view dialogId, std::vector<VS_MediaChannelInfo>& channels) override;

	bool IsTrunkFull() override;
	void SetUserToDialogIdCallback(std::function<void(string_view login, string_view dialogId)> func) override
	{
		m_setUserDialogID = std::move(func);
	}

	void SetPeerCSAddress(string_view dialogId, const net::Endpoint &ep) override;

	// VS_ConferenceProtocolInterface
	bool InviteMethod(string_view dialogId, string_view fromId, string_view toId, const VS_ConferenceInfo &cfgInfo,
		string_view dnFromUTF8 = {}, bool newSession = true, bool forceCreate = false) override;
	bool InviteReplay(string_view dialogId, VS_CallConfirmCode confirmCode, bool isGroupConf, string_view confName = {}, string_view to_displayName = {}) override;
	void Hangup(string_view dialogId) override;
	//void Hangup(const std::shared_ptr<VS_H323ParserInfo>& ctx);

	void LoggedOutAsUser(string_view dialogId) override;

	VS_ChannelID GetChannelID(const void *buf, std::size_t sz, bool& isFragmented) override;
	VS_ChannelID GetDefaultChannelID() override { return e_H225; }

	bool PutOutputMessage(std::unique_ptr<unsigned char[]> buf, std::size_t sz, VS_ChannelID channelId);
	VS_TearMessageQueue* GetInputQueue(VS_ChannelID channelId);
	virtual VS_MessageQueue* GetOutputQueue(VS_ChannelID channelId);

	bool MakeNewConnection(const tcp_endpoint_t& endp, VS_ChannelID channelId)
	{
		auto confMethods = m_confMethods.lock();
		if (!confMethods)
			return false;
		// Erase all messages, that was not sent at previous session in this channel.
		VS_MessageQueue* queue_out = GetOutputQueue(channelId);
		queue_out->EraseAll();
		confMethods->RegisterNewConnection(endp.address(), endp.port(), net::protocol::TCP, channelId);
		return true;

	}

	void CloseConnection(const net::address &addr, net::port port, net::protocol protocol) override;

	void FastUpdatePicture(string_view dialogId) override;
	void Chat(string_view dialogId, const std::string &from, const std::string &to, const std::string &dn, const char *mess) override;
	void Command(string_view dialogId, string_view from, string_view command) override;

	VS_CallConfig::eSignalingProtocol MySignallingProtocol() override
	{	return VS_CallConfig::H323; }

	bool ResolveOnExternalGatekeeper(string_view myName, string_view callId, net::address &addr, net::port &port) override;

	// This method is public because we need it to generate conference_id in
	// VS_H323ExternalGatekeeper::MakeConferenceID().
	static void GenerateNewDialogID(char (&buffer)[32 + 1 /*0-terminator*/]);

	bool GetAudioMode(string_view dialogId, VS_GatewayAudioMode &res);
	bool GetVideoMode(string_view dialogId, VS_GatewayVideoMode &res);
	bool GetDataMode(string_view dialogId, VS_GatewayDataMode &res);

	void UseACL(bool use);
	bool IsACLUsed(void) const;

	void SendDTMF(const std::shared_ptr<VS_H323ParserInfo> &ctx);
private:
	typedef vs::fast_recursive_mutex mutex_t;
protected:
	VS_H323Parser(boost::asio::io_service::strand& strand, const std::shared_ptr<net::LoggerInterface> &logger);
	static void PostConstruct(std::shared_ptr<VS_H323Parser>& /*p*/) {}

	boost::asio::io_service::strand &m_strand;
	std::shared_ptr<net::LoggerInterface> m_logger;
private:
	// parser contexts
	vs::map<std::string, std::shared_ptr<VS_H323ParserInfo>, vs::str_less> m_ctx;
	mutable mutex_t		m_ctx_lock;
	std::vector<boost::signals2::scoped_connection> m_ctx_connections;
	VS_NetworkConnectionACL         m_acl;

	VS_OutputMessageQueue*			m_queue_out;
	mutable mutex_t					m_queue_out_lock;

	VS_TearMessageQueue*			m_queue_in_h225;
	mutable mutex_t					m_queue_in_h225_lock;
	mutable mutex_t					m_queue_in_ras_lock;

	mutable mutex_t					m_hangup_terminate_lock;

	mutable mutex_t					m_ctx_garbage_lock;
	std::vector<std::string>		m_ctx_garbage;

	std::string						m_remote_ip;
	bool                            m_use_acl;
};
