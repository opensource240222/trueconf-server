#pragma once

#include <functional>
#include <chrono>

#include "VS_H323Parser.h"
#include "tools/H323Gateway/Lib/src/VS_RasMessages.h"
#include "std-generic/cpplib/StrCompare.h"

class VS_H225RASParserInfo;

class VS_H225RASParser : public VS_H323Parser
{
	friend class H323RASParserTestBase;

public:

	typedef boost::asio::ip::tcp::endpoint udp_endpoint_t;

	// Default expire time for TrueConf server (in registrator mode) or external terminal (in gatekeeper mode).
	static const std::chrono::seconds DEFAULT_EXPIRES;
	static const std::chrono::seconds ARQ_EXPIRE_SEC;

	// Parser can work in to modes:
	// 1) Registrator - it register VCS on external gatekkeeper as h323-terminal.
	// 2) Gatekeeper - it reveive and process H225 RAS requests from external h323-terminals.
	enum ParserMode
	{
		PM_UDEFINED,
		PM_REGISTRATOR,
		PM_GATEKEEPER
	};

	void Shutdown() override;
	// Destructor.
	virtual ~VS_H225RASParser();
	// Passes incoming buffer from net layer to parser.
	int SetRecvBuf(const void* buf, const std::size_t sz, const VS_ChannelID channelId, const net::address& remoteAddr,
	               net::port remotePort, const net::address& localAddr, net::port localPort) override;
	// Returns output message queue.
	VS_MessageQueue* GetOutputQueue(VS_ChannelID channelId) override;
	// Returns channel id for received buffer.
	VS_ChannelID GetChannelID(const void *buf, std::size_t sz, bool& is_fragmented) override;
	// Returns default channel id.
	VS_ChannelID GetDefaultChannelID() override;
	// Makes new dialog id and binded VS_ParserInfo.
	std::string NewDialogID(string_view sipTo, string_view dtmf, const VS_CallConfig &config, string_view myName = {}) override;
	// Returns base pointer to a parser context for this dialog_id.
	std::shared_ptr<VS_ParserInfo> GetParserContextBase(string_view dialogId, bool create = false) override;
	// Send DRQ, must be invoked at the end of the call.
	void Disengage(const std::string &confId, const std::string &callId, bool isIncomingCall, VS_H323String endpointId = {});
	// Do registration act.
	bool DoRegister(string_view callId, bool updateData = false) override;
	// Cancel registration on all external gatekeepers by sending URQ.
	void UnregisterAll();
	// Cancel registration for <dialog_id>.
	void Unregister(string_view dialogId, bool sendUrq = true);
	// LoggedOutAsUser event handler
	void LoggedOutAsUser(string_view dialogId) override;
	// Sends ARQ for incoming SETUP request and returns:
	// true - if we receive ACF
	// false - if we receive ARJ or time is out or there are another problem.
	// If <dialog_id> is not specified, that means there are only one registrator
	// and we want to use it.
	bool ARQForIncomingCall(VS_CsSetupUuie* setup, std::function<bool(const bool, const bool, net::address, net::port)>);
	// Requests call permission on external gatekeeper and resolves ip:port to call.
	bool ResolveOnExternalGatekeeper(string_view myName, string_view callId, net::address &addr, net::port &port) override;

	// MD5-encrypted ClearToken constructed, using
	// <alias>, <password> and <timestamp> parameters.
	static std::string MakeEncryptedToken_MD5_String(string_view alias, string_view password, time_t timestamp);
	// Do periodically work.
	void Timeout() override;
protected:
	// Constructor.
	VS_H225RASParser(boost::asio::io_service::strand& strand, const std::shared_ptr<net::LoggerInterface>& logger);
	static void PostConstruct(std::shared_ptr<VS_H225RASParser>& /*p*/) {}

	void SetRegistrationConfigurationImpl(const VS_CallConfig &config) override;

private:
	typedef vs::fast_recursive_mutex mutex_t;

private:
	// Map of ARQ requests.
	class ARQMap final
	{
	public:
		explicit ARQMap(VS_H225RASParser* parser);

		struct ARQRequestData;

		typedef struct {
			net::address addr = {};
			net::port port = 0;
		} ResolvedAddress;

		// Puts a request to the map.
		// Return false if this seq_num is already exists.
		bool Put(unsigned short seqNum, ARQRequestData* ard);
		// Find ARQ request data by Sequence Number
		ARQRequestData* Find(unsigned short seqNum);
		// Fills request data when we take a response (ARC, ARJ).
		void Fill(unsigned short seqNum, const ResolvedAddress* addr = nullptr);
		// Removes request from queue.
		void Remove(unsigned short seqNum);
		void ClearExpired(std::chrono::seconds expirationSec) noexcept;
	protected:
		steady_clock_wrapper& clock() const;
	private:
		// Map that contains all ARQ requests.
		// key   - sequence number frim ARQ
		// value - structure, that contains request/response data
		std::map<unsigned short, ARQRequestData*> m_map;
		mutex_t m_mutex;
		VS_H225RASParser *m_parser;
	};

private:
	// Parser context map.
	vs::map<std::string, std::shared_ptr<VS_H225RASParserInfo>, vs::str_less> m_ctx;
	// A queue, that contains outcoming messages, that have not already passed to the net layer.
	VS_MessageQueue* m_out_queue;
	// A queue, that contains outcoming unicast answer to multicast discovery
	VS_MessageQueue* m_out_discovery_queue;
	// Map of arq requests.
	ARQMap m_arq_map;
	// Indicates that UnregisterLastGK() method was invoked.
	bool m_urg_last_gk;
	// A mode of parser.
	VS_H225RASParser::ParserMode m_parser_mode;
	// A lock for parser context.
	mutex_t m_ctx_lock;
	// when shutdown prevent RRQ from same terminal (Polycom RealPresence)
	bool m_is_shutdown;

	std::chrono::steady_clock::time_point m_contexts_check_time;
	// =============== INCOMING RAS PROCESSING ===============
	// *** Registration
	void Receive_GRQ(VS_RasGatekeeperRequest* grq);
	void Receive_RRQ(VS_RasRegistrationRequest* rrq);
	void Receive_RCF(VS_RasRegistrationConfirm* rcf);
	void Receive_RRJ(VS_RasRegistrationReject* rrj);
	// *** Admission
	void Receive_ARQ(VS_RasAdmissionRequest* arq);
	void Receive_ACF(VS_RasAdmissionConfirm* acf);
	void Receive_ARJ(VS_RasAdmissionReject* arj);
	// *** Disengage
	void Receive_DRQ(VS_RasDisengageRequest* drq);
	void Receive_DCF(VS_RasDisengageConfirm* drq);
	void Receive_DRJ(VS_RasDisengageReject* drq);
	// *** Unregistration
	void Receive_URQ(VS_RasUnregistrationRequest* urq);
	void Receive_UCF(VS_RasUnregistrationConfirm* ucf);
	void Receive_URJ(VS_RasUnregistrationReject* urj);
	// =============== OUTCOMING RAS PROCESSING ==============
	// *** Registration
	void Send_RRQ(unsigned short seqnum, const std::vector<std::pair<net::address, net::port>> &rasAddrs, const std::vector<std::pair<net::address, net::port>> &sigAddrs,
		string_view userName, string_view password, string_view dialedDigit, bool keepAlive, std::chrono::seconds expires,
		const VS_H323String& endpointId, const VS_H323String& gatekeeperId);
	void Send_RCF(unsigned seqnum, const net::address &csAddr, net::port csPort, const std::vector<VS_H225AliasAddress>& aliases,
					const VS_H323String& epid, std::chrono::seconds expiresTime);
	void Send_RRJ(unsigned sqenum, unsigned reject_reason);
	// *** Admission
	// ARQ for outcoming call.
	void Send_ARQ(string_view myName, string_view otherName, string_view otherDigit, const char conferenceId[CONFERENCEID_LENGTH],
					const char callIdentifier[CONFERENCEID_LENGTH], unsigned short seqNum, const VS_H323String& endpointId);
	// ARQ for incoming call. Use incoming SETUP request to make a packet.
	void Send_ARQ(VS_CsSetupUuie *setup, unsigned seqNum, const VS_H323String &endpointId);
	void Send_ACF(unsigned int seqnum, const net::address &csAddr, net::port csPort, unsigned callModel);
	void Send_ARJ(unsigned int seqnum, unsigned reason);
	// *** Disengage
	void Send_DRQ(unsigned short seqnum, const char conferenceId[CONFERENCEID_LENGTH],
		          const char callIdentifier[CONFERENCEID_LENGTH], bool isIncomingCall, const VS_H323String &endpointId);
	void Send_DCF(unsigned short seq_num);
	void Send_DRJ();
	// *** Unregistration.
	void Send_URQ(unsigned short seqnum, const std::vector<std::pair<net::address, net::port>> &sigAddrs, const VS_H323String &endpointId);
	void Send_UCF(unsigned short seqNum);
	void Send_URJ(unsigned short seq_num, unsigned reason);
	// =======================================================
	// Pack <choice> to a RAS message structure and send it to the output queue.
	// Returns true if message was successfully puted into the output queue.
	bool PackAndSendRasMessage(VS_Asn* choice, VS_RasMessage::Choices tag, VS_ChannelID channel_id = e_RAS);
	// Makes VS_PerBuffer containing MD5-encrypted ClearToken constructed, using
	// <alias>, <password> and <timestamp> parameters.
	static VS_PerBuffer MakeEncryptedToken_MD5(string_view alias, string_view password, time_t timestamp);
	// Creates new parser info structure.
	// <dialog_id> param will contains new dialog id value.
	std::shared_ptr<VS_H225RASParserInfo> MakeParserInfo(std::string &dialogId);
	// Returns first prser info structure.
	std::shared_ptr<VS_H225RASParserInfo> FindFirstParserInfo();
	// Finds parser info structure by dialog id.
	std::shared_ptr<VS_H225RASParserInfo> FindParserInfoByDialogID(string_view dialogId);
	// Finds parser info structure by endpoint identifier.
	std::shared_ptr<VS_H225RASParserInfo> FindParserInfoByEndpointID(const VS_H323String& endpointId);
	// Finds parser info structure by last RRQ sequence number.
	std::shared_ptr<VS_H225RASParserInfo> FindParserInfoByRRQSeqnum(unsigned short seqnum);
	// Finds parser info structure by registered h323-ID or dialedDigits.
	std::shared_ptr<VS_H225RASParserInfo> FindParserInfoByRegisteredUser(string_view id);
	// Set last gatekeeper info.
	void SetLastGK(string_view callId);
	// Reads last gatekeeper info from register.
	bool GetLastGK(VS_H323String& epid, net::address &csAddr, net::port &csPort);
	// Sends URQ request to cancel last reqistration after server's crash
	// and be able to send new RRQ to this gatekeeper.
	// Must be invoked once at the start of the application.
	// If there was no application crash or this method was invoked, do nothing.
	void UnregisterLastGK();
	// Remove last gatekeeper info from registry (we need it to be public for unit tests).
	void ClearLastGK() noexcept;
	// Generates random dword for sequence number.
	static unsigned short GenerateSequenceNumber();
	// Gets current request sequence number for given context
	unsigned short GetSequenceNumber(VS_H225RASParserInfo *ctx);
	// Login external h323 terminal as TrueConf user.
	// An external h323 terminal must have its own context (VS_H225RASParserInfo) before.
	void LoginAsUser(string_view dialogId, string_view password, const std::vector<std::string>& aliases);
	// Logout external h323 terminal.
	void LogoutAsUser(string_view dialogId);
	// Result of login handler.
	void OnLoginResult(string_view dialogId, bool result);
	// Logout handler.
	void OnLogout(string_view dialogId);
	// Returns true, if this parser exists for registring VCS in external gatekeeper.
	bool IsRegistratorMode() const;
	// Returns true, if this parser exists for registring external terminals in VCS.
	bool IsGatekeeperMode() const;
	// Process full registration request.
	void FullRegistration(VS_RasRegistrationRequest* rrq);
	// Process keepalive request.
	void LiteRegistration(VS_RasRegistrationRequest* rrq);
	// Send RCF with data for specified context.
	void MakeAndSendRCF(string_view dialogId);
	// Remove all contextes, that have expired time to live.
	void ClearExpiredContextes();
	bool HandleARQReply(const unsigned short seqNo, const bool opResult);
	void CleanParserContext(string_view dialogId, SourceClean source) override;
};