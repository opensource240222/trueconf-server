#include "VS_H225RASParser.h"
#include "VS_H323ParserInfo.h"
#include "VS_H225RASParserInfo.h"
#include "VS_H323ExternalGatekeeper.h"
#include "VS_H323GatekeeperStorage.h"

#include "net/UDPRouter.h"
#include "tools/Server/vs_messageQueue.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_UserData.h"
#include "std/cpplib/md5.h"
#include "../CallConfig/VS_IndentifierH323.h"

#include <openssl/objects.h>

#include <string>

#include "std/debuglog/VS_Debug.h"
#include <boost/make_shared.hpp>
#include "std/cpplib/event.h"
#include "std-generic/cpplib/scope_exit.h"

#define DEBUG_CURRENT_MODULE VS_DM_H323PARSER
#define NULL_TICK std::chrono::steady_clock::time_point()

namespace
{
	// A version of H225 RAS.
	constexpr unsigned H225RAS_VERSION[] = { 0, 0, 8, 2250, 0, 4 };
	const VS_GwH225ProtocolIdentifier H225RAS_PROTOCOL_IDENTIFIER(H225RAS_VERSION, sizeof(H225RAS_VERSION) / sizeof(*H225RAS_VERSION));

	// An identifier of the gatekeeper.
#ifdef _SVKS_M_BUILD_
	const VS_H323String GATEKEEPER_IDENTIFIER("SVKS-M Server");
#else
	const VS_H323String GATEKEEPER_IDENTIFIER("TrueConf Server");
#endif

	// Call reference value. Unused, so always set to this value.
	constexpr unsigned CALL_REFERENCE_VALUE = 665;

	// Default band width.						 // from SingleGatewayLib/CommonTypes.h, see RAS_DEFAULT_BANDWIDTH
	constexpr unsigned BAND_WIDTH = 2 * 50000;	// *100 unit = 5Mbit/sec per terminal (*2 - for 2 terminals)


	// Registry keys, that used to store last gatekeeper info.
	constexpr char LAST_GK_EPID_KEY[] = "LastGK.epid";
	constexpr char LAST_GK_IP_KEY[] = "LastGK.ip";
	constexpr char LAST_GK_PORT_KEY[] = "LastGK.port";

	//Period for expired registration check, in milliseconds.
	constexpr std::chrono::seconds EXPIRED_CHECK_PERIOD(10); // = 10 sec

	constexpr unsigned MD5_AUTH_ID[] = { OBJ_md5 };		// equals { 1, 2, 840, 113549, 2, 5 }
	constexpr unsigned SH1_AUTH_ID[] = { OBJ_sha1 };		// equals { 1, 3, 14, 3, 2, 26 }

	VS_AsnObjectId OBJ_ID_MD5 = static_cast<VS_AsnObjectId>(VS_GwH225ProtocolIdentifier(MD5_AUTH_ID, sizeof MD5_AUTH_ID / sizeof*MD5_AUTH_ID));
	VS_AsnObjectId OBJ_ID_SHA1 = static_cast<VS_AsnObjectId>(VS_GwH225ProtocolIdentifier(SH1_AUTH_ID, sizeof SH1_AUTH_ID / sizeof*SH1_AUTH_ID));

}

const std::chrono::seconds VS_H225RASParser::ARQ_EXPIRE_SEC(5);
const std::chrono::seconds VS_H225RASParser::DEFAULT_EXPIRES(300); //300sec = 5min

VS_H225RASParser::VS_H225RASParser(boost::asio::io_service::strand& strand, const std::shared_ptr<net::LoggerInterface> &logger)
	: VS_H323Parser(strand, logger)
	, m_out_queue(0)
	, m_out_discovery_queue(0)
	, m_arq_map(this)
	, m_urg_last_gk(false)
	, m_parser_mode(PM_GATEKEEPER)
	, m_is_shutdown(false)
{
	VS_H323GatekeeperStorage::Instance().RegisterRASParser(this);
	m_contexts_check_time = clock().now();
}

void VS_H225RASParser::Shutdown()
{
	VS_H323Parser::Shutdown();

	RemovePermanentRegistrations(false);
	for(auto &ctx : m_ctx)
	{
		UpdateRegistrationConfig(create_call_config_manager(ctx.second->GetConfig()).GetRegistrationIdentifierView(),
		[reg_state = ctx.second->GetRegistrationState(), expires = ctx.second->GetExpires()](RegistrartionConfig &item)
		{
			auto call_config_manager = create_call_config_manager(item.callConfig);
			item.callConfig.IsValid = false;

			if (call_config_manager.NeedVerification() && reg_state == VS_H225RASParserInfo::RS_UNREGISTRED && expires > std::chrono::seconds(0))
			{
				call_config_manager.SetVerificationResult(VS_CallConfig::e_ServerUnreachable, true);
			}
		});
	}
}

VS_H225RASParser::~VS_H225RASParser()
{
	VS_H323GatekeeperStorage::Instance().UnregisterRASParser(this);
	delete m_out_queue;
	delete m_out_discovery_queue;
}

int VS_H225RASParser::SetRecvBuf(const void* buf, const std::size_t sz, const VS_ChannelID channelId, const net::address& remoteAddr,
	net::port remotePort, const net::address& localAddr, net::port localPort)
{
	if (channelId == e_H225)
		return VS_H323Parser::SetRecvBuf(buf,sz, channelId, remoteAddr, remotePort, localAddr, localPort);

	dstream4 << "VS_H225RASParser::SetRecvBuf: " << sz << " bytes from " << remoteAddr << ":" << localPort;

	// Process only H225 RAS messages.
	if(channelId != e_RAS) return 0;
	// Try to decode ras message.
	VS_PerBuffer in_per_buff(buf, sz*8);
	VS_RasMessage mess;
	if(!mess.Decode(in_per_buff))
	{
		dprint4("VS_H225RASParser::SetRecvBuf: decode failed\n");
		return 0;
	}
	// Message was succesfully decoded.
	switch(mess.tag)
	{
	// Registration
	case mess.e_gatekeeperRequest:
		dprint3("VS_H225RASParser::SetRecvBuf: GRQ\n");
		Receive_GRQ(static_cast<VS_RasGatekeeperRequest*>(mess.choice));
		break;
	case mess.e_registrationRequest:
		dprint3("VS_H225RASParser::SetRecvBuf: RRQ\n");
		Receive_RRQ(static_cast<VS_RasRegistrationRequest*>(mess.choice));
		break;
	case mess.e_registrationConfirm:
		dprint3("VS_H225RASParser::SetRecvBuf: RCF\n");
		Receive_RCF(static_cast<VS_RasRegistrationConfirm*>(mess.choice));
		break;
	case mess.e_registrationReject:
		dprint3("VS_H225RASParser::SetRecvBuf: RRJ\n");
		Receive_RRJ(static_cast<VS_RasRegistrationReject*>(mess.choice));
		break;
	// Admission
	case mess.e_admissionRequest:
		dprint3("VS_H225RASParser::SetRecvBuf: ARQ\n");
		Receive_ARQ(static_cast<VS_RasAdmissionRequest*>(mess.choice));
		break;
	case mess.e_admissionConfirm:
		dprint3("VS_H225RASParser::SetRecvBuf: ACF\n");
		Receive_ACF(dynamic_cast<VS_RasAdmissionConfirm*>(mess.choice));
		HandleARQReply(static_cast<unsigned short>((static_cast<VS_RasAdmissionConfirm*>(mess.choice))->requestSeqNum.value), true);
		break;
	case mess.e_admissionReject:
		dprint3("VS_H225RASParser::SetRecvBuf: ARJ\n");
		Receive_ARJ(static_cast<VS_RasAdmissionReject*> (mess.choice));
		HandleARQReply(static_cast<unsigned short>(static_cast<VS_RasAdmissionReject*>(mess.choice)->requestSeqNum.value), false);
		break;
	// Disengage
	case mess.e_disengageRequest:
		dprint3("VS_H225RASParser::SetRecvBuf: DRQ\n");
		Receive_DRQ(static_cast<VS_RasDisengageRequest*>(mess.choice));
		break;
	case mess.e_disengageConfirm:
		dprint3("VS_H225RASParser::SetRecvBuf: DCF\n");
		Receive_DCF(static_cast<VS_RasDisengageConfirm*>(mess.choice));
		break;
	case mess.e_disengageReject:
		dprint3("VS_H225RASParser::SetRecvBuf: DRJ\n");
		Receive_DRJ(static_cast<VS_RasDisengageReject*>(mess.choice));
		break;
	// Unregistration
	case mess.e_unregistrationRequest:
		dprint3("VS_H225RASParser::SetRecvBuf: URQ\n");
		Receive_URQ(static_cast<VS_RasUnregistrationRequest*>(mess.choice));
		break;
	case mess.e_unregistrationConfirm:
		dprint3("VS_H225RASParser::SetRecvBuf: UCF\n");
		Receive_UCF(static_cast<VS_RasUnregistrationConfirm*>(mess.choice));
		break;
	case mess.e_unregistrationReject:
		dprint3("VS_H225RASParser::SetRecvBuf: URJ\n");
		Receive_URJ(static_cast<VS_RasUnregistrationReject*>(mess.choice));
		break;
	}
	return 1;
}

VS_MessageQueue* VS_H225RASParser::GetOutputQueue(VS_ChannelID channelId)
{
	if (channelId == e_H225)
		return VS_H323Parser::GetOutputQueue(channelId);
	// Process only H225 RAS messages.
	if(channelId == e_RAS)
	{
		if ( !m_out_queue ) m_out_queue = new VS_MessageQueue();
		return m_out_queue;
	}
	// Process only multicast discovery answers
	if(channelId == e_RAS_DISCOVERY)
	{
		if ( !m_out_discovery_queue ) m_out_discovery_queue = new VS_MessageQueue();
		return m_out_discovery_queue;
	}
	return 0;
}

VS_ChannelID VS_H225RASParser::GetChannelID(const void *buf, std::size_t sz, bool& isFragmented)
{
	if(!buf || !sz) return e_noChannelID;
	VS_PerBuffer buff(buf, sz * 8);
	VS_RasMessage msg;
	return msg.Decode(buff) ? e_RAS : e_noChannelID;
}

VS_ChannelID VS_H225RASParser::GetDefaultChannelID()
{
	return e_RAS;
}

std::string VS_H225RASParser::NewDialogID(string_view sipTo, string_view dtmf, const VS_CallConfig &config, string_view myName)
{
	std::string dialog_id;

	// Try to find user in registred h323-terminal list.
	std::shared_ptr<VS_H225RASParserInfo> reg_ctx = FindParserInfoByRegisteredUser(sipTo);
	if (reg_ctx && reg_ctx->GetExpires().count())
	{
		// The user was registred later, so this is outcoming call to registred h323-terminal.
		auto& cs = reg_ctx->GetRequestCSAddresses();
		if (cs.empty())
			return dialog_id; // The user does not give cs address.
		auto &addr = cs.front();

		if (!MakeNewConnection(udp_endpoint_t{ addr.first , addr.second }, e_H225))
			return dialog_id;
		// Create VS_H323Parser context and let VS_H323Parser work.
		dialog_id = VS_H323Parser::NewDialogID(sipTo, dtmf, config, myName);
		// Mark h323parser's context, that this is call through gatekeeper.
		std::shared_ptr<VS_H323ParserInfo> ctx = VS_H323Parser::GetParserContext(dialog_id);
		ctx->MarkAsGatekeeperCall();

		return dialog_id;
	}

	// The user was not registred later, so this is request to register us in h323-gatekeeper.
	// Create new VS_H225RASParser context.
	std::shared_ptr<VS_H225RASParserInfo> ctx = MakeParserInfo(dialog_id);
	// Fill context.
	ctx->SetEndpointIdentifier(VS_H323String(dialog_id));
	if(!config.h323.DialedDigit.empty())
		ctx->SetDialedDigits(config.h323.DialedDigit);

	ctx->SetConfig(config);
	ctx->SetExternalAddress(config.h323.ExternalAddress);

	return dialog_id;
}

void VS_H225RASParser::SetRegistrationConfigurationImpl(const VS_CallConfig &config)
{
	if (config.SignalingProtocol == VS_CallConfig::H225RAS)
	{
		if (config.UseAsDefault || config.UseAsTel)
			VS_H323ExternalGatekeeper::Instance().SetRegistrationContext(weak_from_this());
	}
}

VS_H225RASParser::ARQMap::ARQMap(VS_H225RASParser* parser): m_parser(parser)
{
}

// Structure, that contains ARQ request data.
struct VS_H225RASParser::ARQMap::ARQRequestData final
{
	// Constructor.
	ARQRequestData(std::chrono::steady_clock::time_point time)
		: creationTime(time), wasResolved(false)
	{
	}

#define MakeARQRequestData() new VS_H225RASParser::ARQMap::ARQRequestData(this->clock().now())

	// Address from ACF response. By default ip and port contains zero.
	ResolvedAddress resolvedAddr;

	// Signalize, that name was resolved (means, ACF or ARJ response received)
	// Call on ACF, ARJ or timeout.
	std::function<bool(const bool, const bool, net::address, net::port)> updateCallState;
	std::chrono::steady_clock::time_point creationTime;
	bool wasResolved;
};


inline bool VS_H225RASParser::ARQMap::Put(unsigned short seqNum, ARQRequestData* ard)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);

	return m_map.emplace(seqNum, ard).second;
}

inline VS_H225RASParser::ARQMap::ARQRequestData* VS_H225RASParser::ARQMap::Find(unsigned short seqNum)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);

	auto it = m_map.find(seqNum);
	if (it == m_map.end())
	{
		return NULL;
	}

	return it->second;
}

inline void VS_H225RASParser::ARQMap::Fill(unsigned short seqNum, const ResolvedAddress* addr)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	auto res = m_map.find(seqNum);
	if (res == m_map.cend())
		return;

	if (addr)
	{
		res->second->resolvedAddr = *addr;
		res->second->wasResolved = true;
	}
}

inline void VS_H225RASParser::ARQMap::Remove(unsigned short seqNum)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	m_map.erase(seqNum);
}

inline void VS_H225RASParser::ARQMap::ClearExpired(std::chrono::seconds expirationSec) noexcept
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	if (m_map.empty())
		return;

	auto current_time = clock().now();

	const auto expiration_predicate = [&](const ARQRequestData* rqd) -> bool
	{
		return current_time > rqd->creationTime + expirationSec;
	};

	for (auto it = m_map.begin(); it != m_map.end();)
	{
		auto rqd = it->second;
		if (expiration_predicate(it->second))
		{
			if (rqd->updateCallState != nullptr)
			{
				rqd->updateCallState(false, true, std::move(rqd->resolvedAddr.addr), rqd->resolvedAddr.port);
			}
			it = m_map.erase(it); // in C++11 erase for map will return iterator for next element
			delete rqd;
		}
		else
		{
			++it;
		}
	}
}

inline steady_clock_wrapper& VS_H225RASParser::ARQMap::clock() const
{
	return m_parser->clock();
}

std::shared_ptr<VS_ParserInfo> VS_H225RASParser::GetParserContextBase(string_view dialoId, bool create)
{
	if (dialoId.empty())
		return {};
	{
		std::lock_guard<decltype(m_ctx_lock)> lock(m_ctx_lock);
		auto &&pos = m_ctx.find(dialoId);
		if (pos != m_ctx.cend())
		{
			return pos->second;
		}
	}
	return VS_H323Parser::GetParserContextBase(dialoId, create);
}

void VS_H225RASParser::Disengage(const std::string &confId, const std::string &callId, bool isIncomingCall, VS_H323String endpointId)
{
	// Take a context.
	std::shared_ptr<VS_H225RASParserInfo> ctx;
	if(endpointId.IsEmpty())
		ctx = FindFirstParserInfo();
	else
		ctx = FindParserInfoByEndpointID(endpointId);

	if(!ctx)
		return;
	// Start conditions.
	if(!ctx->IsRegistred()) return;
	if(confId.empty()) return;
	if(callId.empty()) return;
	// Decode id.
	unsigned char new_conf_id[CONFERENCEID_LENGTH];
	VS_H323Parser::DecodeDialogID(confId.c_str(), new_conf_id);
	unsigned char new_call_id[CONFERENCEID_LENGTH];
	VS_H323Parser::DecodeDialogID(confId.c_str(), new_call_id);
	// Make request.
	unsigned short seqnum = GetSequenceNumber(ctx.get());
	Send_DRQ(seqnum, (const char*)new_conf_id, (const char*)new_call_id, isIncomingCall, ctx->GetEndpointIdentifier());
}

bool VS_H225RASParser::DoRegister(string_view callId, const bool updateData)
{
	if (!VS_ParserInterface::DoRegister(callId, updateData))
		return false;

	m_parser_mode = PM_REGISTRATOR;

	std::shared_ptr<VS_H225RASParserInfo> ctx = FindParserInfoByDialogID(callId);
	if (!ctx)
		return false;
	auto ras = std::make_pair(m_myCsEp.addr, m_myCsEp.port);
	auto cs = std::make_pair(ras.first, VS_H323Parser::DEFAULT_H225CS_PORT);

	ctx->ClearRequestCSAddresses();
	ctx->AddRequestCSAddress(cs);

	std::vector<std::pair<net::address, net::port>> ras_addresses;
	std::vector<std::pair<net::address, net::port>> cs_addresses;

	ras_addresses.push_back(std::move(ras));
	cs_addresses.push_back(std::move(cs));

	const auto& external_address = ctx->GetExternalAddress();

	if (!external_address.is_unspecified())
	{
		ras_addresses.emplace_back(external_address, m_myCsEp.port);

		cs_addresses.emplace_back(external_address, VS_H323Parser::DEFAULT_H225CS_PORT);

		ctx->AddRequestCSAddress({ external_address, VS_H323Parser::DEFAULT_H225CS_PORT });
	}

	if (ctx->GetH323ID().empty()) // for using in unit tests
	{
		VS_RealUserLogin r(ctx->GetAliasMy());
		ctx->SetH323ID(r.GetUser());
	}
	bool keepAlive = !!ctx->IsRegistred();

	std::uint32_t disable_keep_alive(0);
	VS_RegistryKey cfg(false, CONFIGURATION_KEY);
	if (cfg.IsValid())
	{
		cfg.GetValue(&disable_keep_alive, sizeof(std::uint32_t), VS_REG_INTEGER_VT, "H323 GK DisableKeepAlive");
		if (disable_keep_alive)
			keepAlive = false;
	}

	UnregisterLastGK();
	unsigned short seqnum = GetSequenceNumber(ctx.get());
	ctx->SetLastRRQSequenceNumber(seqnum);

	auto expires = ctx->GetExpires();
	if (expires == std::chrono::seconds(0))
	{
		ctx->SetRegistrationState(VS_H225RASParserInfo::RS_UNREGISTRED);
		Send_URQ(seqnum, cs_addresses, ctx->GetEndpointIdentifier());
	}
	else
	{
		Send_RRQ(seqnum, ras_addresses, cs_addresses, ctx->GetH323ID(), ctx->GetPassword(),
			ctx->GetDialedDigits(), keepAlive, expires, ctx->GetEndpointIdentifier(), ctx->GetGatekeeperIdentifier());
	}
	return true;
}

void VS_H225RASParser::UnregisterAll()
{
	m_is_shutdown = true;
	std::shared_ptr<VS_H225RASParserInfo> ctx;
	while(true)
	{
		{
			std::lock_guard<decltype(m_ctx_lock)> lock(m_ctx_lock);
			const auto pos = m_ctx.cbegin();
			if(pos == m_ctx.cend())
				break;
			ctx = pos->second;
		}
		Unregister(ctx->GetDialogID(), true);
		ctx.reset();
	}
}

void VS_H225RASParser::Unregister(string_view dialogId, bool sendUrq)
{
	std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };
	auto pos = m_ctx.find(dialogId);
	if(pos == m_ctx.end())
		return;
	std::shared_ptr<VS_H225RASParserInfo> ctx = pos->second;
	m_ctx.erase(pos);
	// Start conditions.
	if(ctx->GetRequestCSAddresses().empty())
		return;
	// Unregistration.
	if(sendUrq)
	{
		auto &sig_addrs = ctx->GetRequestCSAddresses();
		const unsigned short seqnum = GetSequenceNumber(ctx.get());
		Send_URQ(seqnum, sig_addrs, ctx->GetEndpointIdentifier());
	}
	if(IsRegistratorMode())
	{
		ClearLastGK();
	}
	VS_H323Parser::OnContextDestructor(ctx->GetDialogID());
}

void VS_H225RASParser::LoggedOutAsUser(string_view dialogId)
{
	Unregister(dialogId, true);
}

bool VS_H225RASParser::HandleARQReply(const unsigned short seqNo, const bool opResult)
{
	auto ard = m_arq_map.Find(seqNo);

	if (ard == nullptr)
		return false;

	bool result = ard->wasResolved;
	auto func = ard->updateCallState;
	auto resolved_addr = ard->resolvedAddr;
	m_arq_map.Remove(seqNo);
	delete ard;

	// we do not need to call anything
	if (func == nullptr)
		return true;

	if (!result || !opResult)
	{
		result = func(false, false, std::move(resolved_addr.addr), resolved_addr.port);
		return result;
	}

	return func(true, false, std::move(resolved_addr.addr), resolved_addr.port);
}

bool VS_H225RASParser::ARQForIncomingCall(VS_CsSetupUuie* setup, std::function<bool(const bool, const bool timeout, net::address, net::port)> updateCallState)
{
	std::shared_ptr<VS_H225RASParserInfo> ctx = FindFirstParserInfo();
	if(!ctx) return false;
	// Start conditions.
	if(!ctx->IsRegistred()) return false;

	//auto ard = std::make_shared<ARQMap::ARQRequestData>();
	auto ard = MakeARQRequestData();
	unsigned short seq_num = GetSequenceNumber(ctx.get());

	while (!m_arq_map.Put(seq_num, ard))
	{
		seq_num = GetSequenceNumber(ctx.get());
	}

	ard->updateCallState = std::move(updateCallState);

	Send_ARQ(setup, seq_num, ctx->GetEndpointIdentifier());

	return true;
}


bool VS_H225RASParser::ResolveOnExternalGatekeeper(string_view myName, string_view callId, net::address &addr, net::port &port)
{
	// Use first parser interface.
	std::shared_ptr<VS_H225RASParserInfo> ctx = FindFirstParserInfo();
	if (!ctx) return false;
	// Start conditions.
	if (!ctx->IsRegistred()) return false;
	// Only "#h323:username" format!
	// "#h323:username@server" is not for us.
	VS_IndentifierH323::H323CallID hci(callId);
	if (!hci.host.empty()) return false;
	// Must have prefix "#h323:"
	//if (hci.prefix.empty()) return false;	prefix can be empty when call +123
	string_view othername = callId.substr(hci.prefix.length());
	// Other name (from call_id) cannot be empty.
	if (othername.empty())
		return false;
	// My name cannot be empty.
	if (myName.empty())
		return false;

	ARQMap::ARQRequestData* ard = MakeARQRequestData();
	unsigned short seq_num = GetSequenceNumber(ctx.get());
	while (!m_arq_map.Put(seq_num, ard)) seq_num = GetSequenceNumber(ctx.get());

	std::string conf_id = VS_H323ExternalGatekeeper::Instance().MakeConferenceID(myName, callId);

	unsigned char new_dialog_id[CONFERENCEID_LENGTH];
	VS_H323Parser::DecodeDialogID(conf_id.c_str(), new_dialog_id);

	// WARNING: when we make h225 SETUP request, we use conf_id as call_identifier.guid.
	// RAS ARQ is sending before SETUP and use another VS_H323Parser, so there is some troubles
	// to send different conference_id and call_identifier to SETUP message.
	// This server in outcoming call use call_identifier as conference_id and dialog_id so we just
	// pass new_dialog_id as call_identifier and conference_id.
	// If call_id, dialog_id and conf_id become different, this code will make some problems.

	// TODO: We need to use callback here, like we did for the calls to the server.
	// We are pausing thread which executes H225 RAS Parser code untill GK had sent us response
	// for outgoing (client to terminal) calls. It was like this for a while so I (Artem Boldarev)
	// daren't to change it, at least right now.

	struct ResolveData final
	{
		net::address addr{};
		net::port port = 0;
		bool result = false;
		vs::event event{ false };
	};

	auto resolve_result = std::make_shared<ResolveData>();

	ard->updateCallState = [resolve_result, conf_id = std::string(conf_id)](
		const bool ok, const bool timeout, net::address resolveAddr, net::port resolvePort) -> bool
		{

		VS_SCOPE_EXIT{ resolve_result->event.set(); };

		if (timeout)
			return false;

		if (!ok)
		{
			VS_H323ExternalGatekeeper::Instance().RemoveConferenceID(conf_id);
			return false;
		}

		resolve_result->addr = std::move(resolveAddr);
		resolve_result->port = resolvePort;
		resolve_result->result = true;

		return true;
	};

	auto src_alias = ctx->GetH323ID();
	if (src_alias.empty()) {
		src_alias = ctx->GetDialedDigits();
		if (src_alias.empty()) {
			src_alias = myName;
		}
	}
	// othername starts with "/e/" prefix.
	if(othername.find("\\e\\") == 0 && othername.length() > 3)
	{
		// Make ARQ using dst Dialed Digit
		Send_ARQ(src_alias, {}, othername.substr(3), (const char*)new_dialog_id, (const char*)new_dialog_id, seq_num, ctx->GetEndpointIdentifier());
	}
	else
	{
		// Make ARQ using H323-ID
		Send_ARQ(src_alias, othername, {}, (const char*)new_dialog_id, (const char*)new_dialog_id, seq_num, ctx->GetEndpointIdentifier());
	}

	// Waiting for ACF or ARJ
	if (!resolve_result->event.wait_for(ARQ_EXPIRE_SEC))
	{
		VS_H323ExternalGatekeeper::Instance().RemoveConferenceID(conf_id);
		return false;
	}

	if (!resolve_result->result)
		return false;

	addr = std::move(resolve_result->addr);
	port = resolve_result->port;

	return true;
}

std::string VS_H225RASParser::MakeEncryptedToken_MD5_String(string_view alias, string_view password, time_t timestamp)
{
	VS_PerBuffer buff = VS_H225RASParser::MakeEncryptedToken_MD5(alias, password, timestamp);
	unsigned char* data = static_cast<unsigned char*>(buff.GetData());
	char tmp[32 + 1]{ 0 };
	VS_MD5ToString(data, tmp);
	return std::string(tmp);
}

void VS_H225RASParser::Timeout()
{
	CheckPermanentRegistrations();
	// Superclass call.
	VS_H323Parser::Timeout();
	m_arq_map.ClearExpired(ARQ_EXPIRE_SEC);
	// Check for expired registrations.
	if (IsGatekeeperMode()){
		if (clock().now() - m_contexts_check_time > EXPIRED_CHECK_PERIOD)
		{
			ClearExpiredContextes();
			m_contexts_check_time = clock().now();
		}
	}
}


void VS_H225RASParser::Receive_GRQ(VS_RasGatekeeperRequest* grq)
{
	std::unique_ptr<VS_RasGatekeeperConfirm> gcf = vs::make_unique<VS_RasGatekeeperConfirm>();
	gcf->filled = true;

	// Sequence Number.
	gcf->requestSeqNum.value = grq->requestSeqNum.value;
	gcf->requestSeqNum.filled = true;

	// Protocol Identifier.
	gcf->protocolIdentifier = H225RAS_PROTOCOL_IDENTIFIER;

	bool has_pwdHash(false);
	for (auto& m: grq->authenticationCapability)
	{
		if (m.tag == VS_H235AuthenticationMechanism::e_pwdHash)
		{
			has_pwdHash = true;
		}
	}

	bool has_md5(false);
	for (auto& obj_id: grq->algorithmOIDs)
	{
		if (OBJ_ID_MD5 == obj_id) {
			has_md5 = true;
		}
	}

	if (has_pwdHash && has_md5)
	{
		// Algorithm OID
		gcf->algorithmOID = OBJ_ID_MD5;
		gcf->algorithmOID.filled = true;

		// Authentication Mode
		gcf->authenticationMode.tag = VS_H235AuthenticationMechanism::e_pwdHash;
		gcf->authenticationMode.choice = new VS_AsnNull;
		gcf->authenticationMode.filled = true;
	}

	constexpr net::port ras_port = 1719;
	// RAS Address
	net::address his_ras_addr;
	net::port his_ras_port;
	get_ip_address(grq->rasAddress, his_ras_addr, his_ras_port);
	bool nat_addr = false;
	auto cfg = CreateCallConfig({ his_ras_addr, his_ras_port, net::protocol::UDP }, {});
	if (!cfg.h323.ExternalAddress.is_unspecified())
	{
		if (cfg.h323.ExternalAddressScheme == VS_CallConfig::eExternalAddressScheme::ALWAYS_EXTERNAL)
			nat_addr = true;
		else if (cfg.h323.ExternalAddressScheme == VS_CallConfig::eExternalAddressScheme::ONLY_INTERNET_ADDRESS) {
			if (!net::is_private_address(his_ras_addr))
				nat_addr = true;
		}
	}
	if (nat_addr && !cfg.h323.ExternalAddress.is_unspecified())
		set_ip_address(gcf->rasAddress, cfg.h323.ExternalAddress, ras_port);
	else
		set_ip_address(gcf->rasAddress, m_myCsEp.addr, ras_port);

	VS_RasGatekeeperConfirm* gcf_tmp = new VS_RasGatekeeperConfirm;
	*gcf_tmp = *gcf;

	boost::system::error_code ec;
	auto connection = net::UDPRouter::Connect(m_strand.get_io_service(), ras_port, boost::asio::ip::udp::endpoint(his_ras_addr, his_ras_port), ec);
	if (ec == boost::asio::error::already_open)
	{
		PackAndSendRasMessage(gcf_tmp, VS_RasMessage::e_gatekeeperConfirm);
		return;
	}

	if (ec)
	{
		dstream2 << "VS_H323Parser:Receive_GRQ: Can't create UDPConnection to " << his_ras_addr << ':' << his_ras_port << ": " << ec.message();
		return;
	}

	PackAndSendRasMessage(gcf_tmp, VS_RasMessage::e_gatekeeperConfirm, e_RAS_DISCOVERY);

	std::size_t sz = 2048;
	std::unique_ptr<char []> buf = vs::make_unique<char[]>(sz);
	if (GetBufForSend(buf.get(), sz, e_RAS_DISCOVERY, net::address{}, 0, net::address{}, 0) > 0 && sz > 0)
	{
		assert(connection.is_open());
		const auto buffer = boost::asio::buffer(buf.get(), sz);
		connection.async_send(buffer, vs::move_handler([buf = std::move(buf)](const boost::system::error_code&, size_t) {}));
	}
}

void VS_H225RASParser::Receive_RRQ(VS_RasRegistrationRequest* rrq)
{
	if(!IsGatekeeperMode()) return;
	if (m_is_shutdown)
		return;

	if(rrq->keepAlive.value)
		LiteRegistration(rrq);
	else
		FullRegistration(rrq);
}

void VS_H225RASParser::Receive_RCF(VS_RasRegistrationConfirm* rcf)
{
	if(!IsRegistratorMode()) return;

	// Sequence number check
	std::shared_ptr<VS_H225RASParserInfo> ctx = FindParserInfoByRRQSeqnum(rcf->requestSeqNum.value);
	if(ctx)
	{
		ctx->SetLastRRQSequenceNumber(-1); // Set to unused state.
		ctx->SetRegistrationState(VS_H225RASParserInfo::RS_REGISTRATION_SUCCESS);
		ctx->SetEndpointIdentifier(VS_H323String(rcf->endpointIdentifier.value));
		ctx->SetGatekeeperIdentifier(VS_H323String(rcf->gatekeeperIdentifier.value));
		if (rcf->timeToLive.filled && rcf->timeToLive.value > 0)
			ctx->SetExpires(std::chrono::seconds(rcf->timeToLive.value));
		else
			ctx->SetExpires(DEFAULT_EXPIRES);
		// Save last gk values.
		SetLastGK(ctx->GetDialogID());

		UpdateRegistrationConfig(create_call_config_manager(ctx->GetConfig()).GetRegistrationIdentifierView(), [](RegistrartionConfig &item)
		{
			auto &&config_manager = create_call_config_manager(item.callConfig);
			if (config_manager.NeedVerification())
			{
				config_manager.SetValidVerification();
			}
		});
	}
}

void VS_H225RASParser::Receive_RRJ(VS_RasRegistrationReject* rrj)
{
	if(!IsRegistratorMode()) return;

	// Sequence number check
	std::shared_ptr<VS_H225RASParserInfo> ctx = FindParserInfoByRRQSeqnum(rrj->requestSeqNum.value);
	if(ctx)
	{
		ctx->SetRegistrationState(VS_H225RASParserInfo::RS_REGISTRATION_FAILED);
		ctx->SetEndpointIdentifier(VS_H323String());
		ctx->SetGatekeeperIdentifier(VS_H323String());
		ctx->SetExpires(DEFAULT_EXPIRES);

		UpdateRegistrationConfig(create_call_config_manager(ctx->GetConfig()).GetRegistrationIdentifierView(), [](RegistrartionConfig &item)
		{
			auto &&config_manager = create_call_config_manager(item.callConfig);
			if (config_manager.NeedVerification())
			{
				config_manager.SetForbiddenVerification();
			}
		});
	}
}

void VS_H225RASParser::Receive_ARQ(VS_RasAdmissionRequest* arq)
{
	if(!IsGatekeeperMode()) return;

	VS_H323String epid = VS_H323String(arq->endpointIdentifier.value);
	std::shared_ptr<VS_H225RASParserInfo> ctx = FindParserInfoByEndpointID(epid);
	if(!ctx)
	{
		dprint3("Receive_ARQ, but ctx not found!\n");
	}
	// Conference ID
	char conf_id[256];
	EncodeDialogID(static_cast<unsigned char*>(arq->conferenceID.value.GetData()), conf_id);
	// Call Identifier
	char call_id[256];
	EncodeDialogID(static_cast<unsigned char*>(arq->callIdentifier.guid.value.GetData()), call_id);
	// Source Alias
	std::string alias_from;
	for (auto& addr: arq->srcInfo)
	{
		if(addr.tag == VS_H225AliasAddress::e_h323_ID)
		{
			alias_from = addr.String();
			break;
		}
	}
	// Destination Alias
	// It was good idea to check does <alias_to> user exists and online,
	// but is in not possible now :-(
	std::string alias_to;
	for (auto& addr: arq->destinationInfo)
	{
		if(addr.tag == VS_H225AliasAddress::e_h323_ID ||
			addr.tag == VS_H225AliasAddress::e_dialedDigits ||
			addr.tag == VS_H225AliasAddress::e_url_ID)
		{
			alias_to = addr.String();
			break;
		}
	}

	// We provide only gatekeeper-routed call, so in ACF we send out cs address.
	auto cs_addr = std::make_pair(m_myCsEp.addr, VS_H323Parser::DEFAULT_H225CS_PORT);
	unsigned call_type = VS_H225CallModel::e_gatekeeperRouted;

	if (!alias_to.empty())
	{
		std::vector<std::pair<net::address, net::port>> ip;
		const auto dialed_digits = "\\e\\" + alias_to;
		if (VS_H323GatekeeperStorage::Instance().GetRegisteredTerminalInfo(alias_to, ip) ||
			VS_H323GatekeeperStorage::Instance().GetRegisteredTerminalInfo(dialed_digits, ip) && !ip.empty())
		{
			cs_addr = std::move(ip.front());
			call_type = VS_H225CallModel::e_direct;
		}
	}

	// Store call identifier.
	{
		std::shared_ptr<VS_H323GatekeeperStorage::Info> info = std::make_shared<VS_H323GatekeeperStorage::Info>();
		info->parser = shared_from_this();
		info->context = ctx;
		VS_H323GatekeeperStorage::Instance().SaveTerminalInfo(call_id, std::move(info));
	}
	// Send response.
	Send_ACF(static_cast<unsigned short>(arq->requestSeqNum.value), cs_addr.first, cs_addr.second, call_type);
}

void VS_H225RASParser::Receive_ACF(VS_RasAdmissionConfirm* acf)
{
	if(!IsRegistratorMode())
		return;

	decltype(ARQMap::ARQRequestData::resolvedAddr) address;
	if (!get_ip_address(acf->destCallSignalAddress, address.addr, address.port))
		return;
	m_arq_map.Fill(static_cast<unsigned short>(acf->requestSeqNum.value), &address);
}

void VS_H225RASParser::Receive_ARJ(VS_RasAdmissionReject* arj)
{
	if(!IsRegistratorMode()) return;

	m_arq_map.Fill(static_cast<unsigned short>(arj->requestSeqNum.value));
}

void VS_H225RASParser::Receive_DRQ(VS_RasDisengageRequest* drq)
{
	Send_DCF(static_cast<unsigned short>(drq->requestSeqNum.value));
}

void VS_H225RASParser::Receive_DCF(VS_RasDisengageConfirm* dcf)
{

}

void VS_H225RASParser::Receive_DRJ(VS_RasDisengageReject* drj)
{

}

void VS_H225RASParser::Receive_URQ(VS_RasUnregistrationRequest* urq)
{
	// get IPs to unregister
	std::vector<std::pair<net::address, net::port>> ips;
	for (auto& addr: urq->callSignalAddress)
	{
		net::address ip{};
		net::port port = 0;
		if(get_ip_address(addr, ip, port) && !ip.is_unspecified() && port != 0)
		{
			ips.emplace_back(std::move(ip), port);
		}
	}
	// try unregister on our gatekeeper
	bool unregistered = VS_H323GatekeeperStorage::Instance().UnregisterH323Terminal(ips);

	VS_H323String epid(urq->endpointIdentifier.value);
	std::shared_ptr<VS_H225RASParserInfo> ctx = FindParserInfoByEndpointID(epid);
	if(!ctx)
	{
		if (unregistered)
			Send_UCF(static_cast<unsigned short>(urq->requestSeqNum.value));
		else
			Send_URJ(static_cast<unsigned short>(urq->requestSeqNum.value), VS_H225UnregRejectReason::e_notCurrentlyRegistered);
		return;
	}
	// Remove registration.
	ctx->SetRegistrationState(VS_H225RASParserInfo::RS_UNREGISTRED);
	ctx->SetEndpointIdentifier(VS_H323String());
	ctx->SetGatekeeperIdentifier(VS_H323String());
	if(IsRegistratorMode())
	{
		ClearLastGK();
	}
	if(IsGatekeeperMode())
	{
		LogoutAsUser(ctx->GetDialogID());
	}
	// Send UCF
	Send_UCF(static_cast<unsigned short>(urq->requestSeqNum.value));

	ctx->SetExpires(std::chrono::seconds(0));
	UpdateRegistrationConfig(create_call_config_manager(ctx->GetConfig()).GetRegistrationIdentifierView(), [](RegistrartionConfig &item)
	{
		auto config_manager = create_call_config_manager(item.callConfig);
		if (config_manager.NeedVerification())
		{
			config_manager.SetForbiddenVerification();
		}
	});

	Unregister(ctx->GetDialogID(), false);
}

void VS_H225RASParser::Receive_UCF(VS_RasUnregistrationConfirm* ucf)
{

}

void VS_H225RASParser::Receive_URJ(VS_RasUnregistrationReject* urj)
{

}

void VS_H225RASParser::Send_RRQ(unsigned short seqnum, const std::vector<std::pair<net::address, net::port>> &rasAddrs, const std::vector<std::pair<net::address, net::port>> &sigAddrs,
	string_view userName, string_view password, string_view dialedDigit, bool keepAlive, std::chrono::seconds expires,
	const VS_H323String& endpointId, const VS_H323String& gatekeeperId)
{

	if (sigAddrs.empty() || rasAddrs.empty())
		return;


	dprint3("VS_H225RASParser::Send_RRQ\n");
	// Aliases
	/*const char* my_alias = username;
	const char* my_digit = dialedDigit;*/

	size_t aliasCount = 0; // Number of aliases

	if(!userName.empty()) aliasCount++;
	if(!dialedDigit.empty()) aliasCount++;

	// Must be at least one alias.
	if(aliasCount == 0)
		return;

	// === Make RRQ Message ===
	VS_RasRegistrationRequest* rrq =  new VS_RasRegistrationRequest;

	// Sequence Number.
	rrq->requestSeqNum.value = seqnum;
	rrq->requestSeqNum.filled = true;

	// Protocol Identifier.
	rrq->protocolIdentifier = H225RAS_PROTOCOL_IDENTIFIER;

	// Discovery Complete
	rrq->discoveryComplete.value = false;
	rrq->discoveryComplete.filled = true;

	// Call Signalling Address
	const std::size_t ta_len1 = sigAddrs.size();
	VS_H225TransportAddress* ta1 = ta_len1 == 0 ? nullptr : new VS_H225TransportAddress[ta_len1];
	for (size_t i = 0; i < ta_len1; ++i) {
		assert(!!ta1);
		auto &enpoint = sigAddrs[i];
		set_ip_address(ta1[i], enpoint.first, enpoint.second);
	}
	ta1->filled = true;
	rrq->callSignalAddress.reset(ta1, ta_len1);

	// RAS Address
	const std::size_t ta_len2 = rasAddrs.size();
	VS_H225TransportAddress* ta2 = ta_len2 == 0 ? nullptr : new VS_H225TransportAddress[ta_len2];
	for (std::size_t i = 0; i < ta_len2; ++i) {
		assert(!!ta2);
		auto &endpoint = rasAddrs[i];
		set_ip_address(ta2[i], endpoint.first, endpoint.second);
	}
	ta2->filled = true;
	rrq->rasAddress.reset(ta2, ta_len2);

	// Terminal Type
	rrq->terminalType.terminalInfo.filled = true;
	rrq->terminalType.mc.value = false;
	rrq->terminalType.mc.filled = true;
	rrq->terminalType.undefinedNode.value = false;
	rrq->terminalType.undefinedNode.filled = true;
	rrq->terminalType.vendor.productId.value.AddBits(H323Gateway_Product, (sizeof(H323Gateway_Product) - 1) * 8);
	rrq->terminalType.vendor.productId.filled = true;
	rrq->terminalType.vendor.versionId.value.AddBits(H323Gateway_Version, (sizeof(H323Gateway_Version) - 1) * 8);
	rrq->terminalType.vendor.versionId.filled = true;
	rrq->terminalType.vendor.filled =
		rrq->terminalType.vendor.versionId.filled
		&
		rrq->terminalType.vendor.productId.filled;
	rrq->terminalType.vendor.h221NonStandard.manufacturerCode.value = 0;//поставлено нашару
	rrq->terminalType.vendor.h221NonStandard.manufacturerCode.filled = true;
	rrq->terminalType.vendor.h221NonStandard.t35CountryCode.value = T35_COUNTRY_CODE_RUSSIA;	// Russia
	rrq->terminalType.vendor.h221NonStandard.t35CountryCode.filled = true;
	rrq->terminalType.vendor.h221NonStandard.t35Extension.value   = 0;//поставлено нашару
	rrq->terminalType.vendor.h221NonStandard.t35Extension.filled = true;
	rrq->terminalType.vendor.h221NonStandard.filled = true;
	rrq->terminalType.filled = true;

	if( ! keepAlive )
	{
		// Terminal Alias
		rrq->terminalAlias.reset(aliasCount == 0 ? nullptr : new VS_H225AliasAddress[aliasCount], aliasCount);

		size_t iter = 0;

		// H323-ID
		if(!userName.empty())
		{
			rrq->terminalAlias[iter].tag = VS_H225AliasAddress::e_h323_ID;
			rrq->terminalAlias[iter].filled = true;
			TemplBmpString<1,256> * tmpAlias = new TemplBmpString<1,256>;
			tmpAlias->value = VS_H323String(userName).MakePerBuffer();
			tmpAlias->filled = true;
			rrq->terminalAlias[iter].choice = tmpAlias;

			iter++;
		}

		// DialedDigit
		if(!dialedDigit.empty())
		{
			rrq->terminalAlias[iter].tag = VS_H225AliasAddress::e_dialedDigits;
			rrq->terminalAlias[iter].filled = true;
			VS_AsnIA5String* tmpDigit = new VS_AsnIA5String( VS_H225AliasAddress::dialedDigits_alphabet,
														sizeof(VS_H225AliasAddress::dialedDigits_alphabet),
														VS_H225AliasAddress::dialedDigits_inverse_table, 1, 128);
			tmpDigit->SetNormalString(dialedDigit.data(), dialedDigit.length());
			rrq->terminalAlias[iter].choice = tmpDigit;

			iter++;
		}
	}

	if( keepAlive )
	{
		// Gatekeeper Identifier
		if (!gatekeeperId.IsEmpty())
		{
			rrq->gatekeeperIdentifier.value = gatekeeperId.MakePerBuffer();
			rrq->gatekeeperIdentifier.filled = true;
		}
	}

	// Endpoint Vendor
	rrq->endpointVendor.productId.value.AddBits(H323Gateway_Product, (sizeof(H323Gateway_Product) - 1) * 8);
	rrq->endpointVendor.productId.filled = true;
	rrq->endpointVendor.versionId.value.AddBits(H323Gateway_Version, (sizeof(H323Gateway_Version) - 1) * 8);
	rrq->endpointVendor.versionId.filled = true;

	rrq->endpointVendor.filled =
		rrq->endpointVendor.versionId.filled
		&
		rrq->endpointVendor.productId.filled;

	rrq->endpointVendor.h221NonStandard.manufacturerCode.value = 0;//поставлено нашару
	rrq->endpointVendor.h221NonStandard.manufacturerCode.filled = true;
	rrq->endpointVendor.h221NonStandard.t35CountryCode.value = T35_COUNTRY_CODE_RUSSIA;
	rrq->endpointVendor.h221NonStandard.t35CountryCode.filled = true;
	rrq->endpointVendor.h221NonStandard.t35Extension.value   = 0;//поставлено нашару
	rrq->endpointVendor.h221NonStandard.t35Extension.filled = true;
	rrq->endpointVendor.h221NonStandard.filled = true;

	// Time To Live
	rrq->timeToLive.value = static_cast<std::uint32_t>(expires.count());
	rrq->timeToLive.filled = true;

	// Keep Alive
	rrq->keepAlive.value = keepAlive;
	rrq->keepAlive.filled = true;

	// Endpoint Identifier
	if( keepAlive )
	{
		if (!endpointId.IsEmpty())
		{
			rrq->endpointIdentifier.value = endpointId.MakePerBuffer();
			rrq->endpointIdentifier.filled = true;
		}
	}

	// Will Supply UUIEs
	rrq->willSupplyUUIEs.value = false;
	rrq->willSupplyUUIEs.filled = true;

	// Maintain Connection
	rrq->maintainConnection.value = false;
	rrq->maintainConnection.filled = true;

	// Support Assigned GK
	rrq->supportsAssignedGK.value = false;
	rrq->supportsAssignedGK.filled = true;

	// Crypto Tokens
	if(!password.empty()) // If password is not specified, then we use registration without password.
	{
		rrq->cryptoTokens.reset(aliasCount == 0 ? nullptr : new VS_H225CryptoH323Token[aliasCount], aliasCount);

		int iter = 0;

		// H323-ID
		if(!userName.empty())
		{
			rrq->cryptoTokens[iter].tag = VS_H225CryptoH323Token::e_cryptoEPPwdHash;
			VS_H225CryptoEPPwdHash* eppwdhash = new VS_H225CryptoEPPwdHash;
			{
				// Alias (H323-ID)
				eppwdhash->alias.tag = VS_H225AliasAddress::e_h323_ID;
				eppwdhash->alias.filled = true;
				eppwdhash->alias.choice = new TemplBmpString<1,256>;
				(dynamic_cast<TemplBmpString<1,256>*>(eppwdhash->alias.choice))->value = VS_H323String(userName).MakePerBuffer();
				eppwdhash->alias.choice->filled = true;
				// Timestamp
				eppwdhash->timestamp.filled = true;
				eppwdhash->timestamp.value = (std::uint32_t)time(NULL);
				// Algorithm OID
				eppwdhash->token.algorithmOID = OBJ_ID_MD5;
				eppwdhash->token.algorithmOID.filled = true;
				// Parameters
				eppwdhash->token.paramS.filled = true;
				eppwdhash->token.filled = true;
				// Hash
				eppwdhash->token.hash.value = MakeEncryptedToken_MD5(userName, password, eppwdhash->timestamp.value);
				eppwdhash->token.hash.filled = true;
				eppwdhash->filled = true;
			}
			rrq->cryptoTokens[iter].choice = eppwdhash;
			rrq->cryptoTokens[iter].filled = true;

			iter++;
		}
		// DialedDigits
		if(!dialedDigit.empty())
		{
			rrq->cryptoTokens[iter].tag = VS_H225CryptoH323Token::e_cryptoEPPwdHash;
			VS_H225CryptoEPPwdHash* eppwdhash = new VS_H225CryptoEPPwdHash;
			{
				// Alias (DialedDigits).
				eppwdhash->alias.tag = VS_H225AliasAddress::e_dialedDigits;
				eppwdhash->alias.filled = true;
				VS_AsnIA5String* tmpDigit = new VS_AsnIA5String( VS_H225AliasAddress::dialedDigits_alphabet,
														sizeof(VS_H225AliasAddress::dialedDigits_alphabet),
														VS_H225AliasAddress::dialedDigits_inverse_table, 1, 128);
				tmpDigit->SetNormalString(dialedDigit.data(), dialedDigit.length());
				eppwdhash->alias.choice = tmpDigit;
				// Timestamp.
				eppwdhash->timestamp.filled = true;
				eppwdhash->timestamp.value = (std::uint32_t) time(NULL);
				// Algorithm OID
				eppwdhash->token.algorithmOID = OBJ_ID_MD5;
				eppwdhash->token.algorithmOID.filled = true;
				// Parameters
				eppwdhash->token.paramS.filled = true;
				eppwdhash->token.filled = true;
				// Hash
				eppwdhash->token.hash.value = MakeEncryptedToken_MD5(dialedDigit, password, eppwdhash->timestamp.value);
				eppwdhash->token.hash.filled = true;
				eppwdhash->filled = true;
			}
			rrq->cryptoTokens[iter].choice = eppwdhash;
			rrq->cryptoTokens[iter].filled = true;

			iter++;
		}
	}

	// kt: we dont support this
	rrq->additiveRegistration.filled = false;
	rrq->supportsACFSequences.filled = false;
	rrq->supportsAltGK.filled = false;

	// Restart
	rrq->restart.filled = true;

	rrq->filled = true;

	PackAndSendRasMessage(rrq, VS_RasMessage::e_registrationRequest);
}

void VS_H225RASParser::Send_RCF(unsigned seqnum, const net::address& csAddr, net::port csPort,
                                const std::vector<VS_H225AliasAddress>& aliases, const VS_H323String& epid,
                                std::chrono::seconds expiresTime)
{
	dprint3("VS_H225RASParser::Send_RCF\n");
	VS_RasRegistrationConfirm* rcf = new VS_RasRegistrationConfirm();

	// Request Sequence Number
	rcf->requestSeqNum.value = seqnum;
	rcf->requestSeqNum.filled = true;
	// Protocol Identifier
	rcf->protocolIdentifier = H225RAS_PROTOCOL_IDENTIFIER;
	// Call Signalling Address
	VS_H225TransportAddress *ta1 = new VS_H225TransportAddress[1];
	set_ip_address(ta1[0], csAddr, csPort);

	rcf->callSignalAddress.reset(ta1, 1);
	// Terminal Alias
	const auto aliases_size = aliases.size();
	rcf->terminalAlias.reset(aliases_size == 0 ? nullptr : new VS_H225AliasAddress[aliases_size], aliases_size);
	int index = 0;
	for(const auto& alias: aliases)
	{
		rcf->terminalAlias[index] = alias;
		index++;
	}
	// Gatekeeper Identifier
	rcf->gatekeeperIdentifier.value = GATEKEEPER_IDENTIFIER.MakePerBuffer();
	rcf->gatekeeperIdentifier.filled = true;
	// Endpoint Identifier
	rcf->endpointIdentifier.value = epid.MakePerBuffer();
	rcf->endpointIdentifier.filled = true;
	// Time To Live
	rcf->timeToLive.value = static_cast<std::uint32_t>(expiresTime.count());
	rcf->timeToLive.filled = true;
	// Will Respond To IRR
	rcf->willRespondToIRR.value = false;
	rcf->willRespondToIRR.filled = true;
	// Maintain Connection
	rcf->maintainConnection.value = false;
	rcf->maintainConnection.filled = true;

	rcf->filled = true;

	PackAndSendRasMessage(rcf, VS_RasMessage::e_registrationConfirm);
}

void VS_H225RASParser::Send_RRJ(unsigned seqnum, unsigned rejectReason)
{
	dprint3("VS_H225RASParser::Send_RRJ\n");
	VS_RasRegistrationReject* rrj = new VS_RasRegistrationReject();

	// Sequence Number
	rrj->requestSeqNum.value = seqnum;
	rrj->requestSeqNum.filled = true;
	// Protocol Identifier
	rrj->protocolIdentifier = H225RAS_PROTOCOL_IDENTIFIER;
	// Reject Reason
	rrj->rejectReason.tag = rejectReason;
	rrj->rejectReason.choice = new VS_AsnNull;
	rrj->rejectReason.filled = true;
	// Gatekeeper Identifier
	rrj->gatekeeperIdentifier.value = GATEKEEPER_IDENTIFIER.MakePerBuffer();
	rrj->gatekeeperIdentifier.filled = true;

	rrj->filled = true;

	PackAndSendRasMessage(rrj, VS_RasMessage::e_registrationReject);
}

void VS_H225RASParser::Send_ARQ(string_view myName, string_view otherName, string_view otherDigit, const char conferenceId[CONFERENCEID_LENGTH],
	const char callIdentifier[CONFERENCEID_LENGTH], unsigned short seqNum, const VS_H323String& endpointId)
{
	dprint3("VS_H225RASParser::Send_ARQ\n");
	// === Make ARQ Message ===
	VS_RasAdmissionRequest* arq = new VS_RasAdmissionRequest;

	// Sequence Number.
	arq->requestSeqNum.value = seqNum;
	arq->requestSeqNum.filled = true;

	// Call Type
	arq->callType.tag = VS_H225CallType::e_pointToPoint;
	VS_AsnNull* nuller_calltype = new VS_AsnNull;
	nuller_calltype->filled = true;
	arq->callType.choice = nuller_calltype;
	arq->callType.filled = true;

	// Call Model
	arq->callModel.tag = VS_H225CallModel::e_direct;
	VS_AsnNull* nuller_callmodel = new VS_AsnNull;
	nuller_callmodel->filled = true;
	arq->callModel.choice = nuller_callmodel;
	arq->callModel.filled = true;

	// Endpoint Identifier
	arq->endpointIdentifier.value = endpointId.MakePerBuffer();
	arq->endpointIdentifier.filled = true;

	// Destination Info

	size_t aliasCount = 0;

	if(!otherName.empty())   aliasCount++;
	if(!otherDigit.empty()) aliasCount++;

	arq->destinationInfo.reset(aliasCount == 0 ? nullptr : new VS_H225AliasAddress[aliasCount], aliasCount);

	size_t iter = 0;

	if(!otherName.empty())
	{
		arq->destinationInfo[iter].tag = VS_H225AliasAddress::e_h323_ID;
		arq->destinationInfo[iter].filled = true;
		TemplBmpString<1,256> * dstTmpAlias = new TemplBmpString<1,256>;
		dstTmpAlias->value = VS_H323String(otherName).MakePerBuffer();
		dstTmpAlias->filled = true;
		arq->destinationInfo[iter].choice = dstTmpAlias;

		iter++;
	}

	if(!otherDigit.empty())
	{
		arq->destinationInfo[iter].tag = VS_H225AliasAddress::e_dialedDigits;
		arq->destinationInfo[iter].filled = true;
		VS_AsnIA5String* tmpDigit = new VS_AsnIA5String( VS_H225AliasAddress::dialedDigits_alphabet,
														sizeof(VS_H225AliasAddress::dialedDigits_alphabet),
														VS_H225AliasAddress::dialedDigits_inverse_table, 1, 128);
		tmpDigit->SetNormalString(otherDigit.data(), otherDigit.length());
		arq->destinationInfo[iter].choice = tmpDigit;

		iter++;
	}

	// Source Info
	arq->srcInfo.filled = true;
	arq->srcInfo.reset(new VS_H225AliasAddress[1], 1);
	arq->srcInfo[0].tag = VS_H225AliasAddress::e_h323_ID;
	arq->srcInfo[0].filled = true;
	TemplBmpString<1,256> * tmpAlias = new TemplBmpString<1,256>;
	tmpAlias->value = VS_H323String(myName).MakePerBuffer();
	tmpAlias->filled = true;
	arq->srcInfo[0].choice = tmpAlias;

	// Band Width
	arq->bandWidth.value = BAND_WIDTH;
	arq->bandWidth.filled = true;

	// Call Reference Value
	arq->callReferenceValue.value = CALL_REFERENCE_VALUE;
	arq->callReferenceValue.filled = true;

	// Conference ID
	VS_PerBuffer conf_id_buff(conferenceId, CONFERENCEID_LENGTH * 8);
	arq->conferenceID.value = conf_id_buff;
	arq->conferenceID.filled = true;

	// Active MC
	arq->activeMC.value = false;
	arq->activeMC.filled = true;

	// Answer Call
	arq->answerCall.value = false;
	arq->answerCall.filled = true;

	// Call Identifier
	VS_PerBuffer call_id_buff(callIdentifier, CONFERENCEID_LENGTH * 8);
	arq->callIdentifier.guid.value = call_id_buff;
	arq->callIdentifier.guid.filled = true;
	arq->callIdentifier.filled = true;

	arq->filled = true;

	PackAndSendRasMessage(arq, VS_RasMessage::e_admissionRequest);
}

void VS_H225RASParser::Send_ARQ(VS_CsSetupUuie* setup, unsigned seqNum, const VS_H323String& endpointId)
{
	dprint3("VS_H225RASParser::Send_ARQ\n");
	// === Make ARQ Message ===
	VS_RasAdmissionRequest* arq = new VS_RasAdmissionRequest;

	// Sequence Number.
	arq->requestSeqNum.value = seqNum;
	arq->requestSeqNum.filled = true;

	// Call Type
	arq->callType.tag = VS_H225CallType::e_pointToPoint;
	VS_AsnNull* nuller_calltype = new VS_AsnNull;
	nuller_calltype->filled = true;
	arq->callType.choice = nuller_calltype;
	arq->callType.filled = true;

	// Call Model
	arq->callModel.tag = VS_H225CallModel::e_direct;
	VS_AsnNull* nuller_callmodel = new VS_AsnNull;
	nuller_callmodel->filled = true;
	arq->callModel.choice = nuller_callmodel;
	arq->callModel.filled = true;

	// Endpoint Identifier
	arq->endpointIdentifier.value = endpointId.MakePerBuffer();
	arq->endpointIdentifier.filled = true;

	// Destination Info
	arq->destinationInfo.filled = true;
	const auto dest_address_size = setup->destinationAddress.size();
	arq->destinationInfo.reset(dest_address_size == 0 ? nullptr : new VS_H225AliasAddress[dest_address_size], dest_address_size);
	for(std::size_t i = 0; i < arq->destinationInfo.size(); i++)
	{
		arq->destinationInfo[i] = setup->destinationAddress[i];
	}

	// Destination Call Signalling Address
	arq->destCallSignalAddress = setup->destCallSignalAddress;

	// Source Info
	arq->srcInfo.filled = true;
	const auto src_addr_size = setup->sourceAddress.size();
	arq->srcInfo.reset(src_addr_size == 0 ? nullptr : new VS_H225AliasAddress[src_addr_size], src_addr_size);
	for(std::size_t i = 0; i < arq->srcInfo.size(); i++)
	{
		arq->srcInfo[i] = setup->sourceAddress[i];
	}

	// Source Call Signalling Address
	arq->srcCallSignalAddress = setup->sourceCallSignalAddress;

	// Band Width
	arq->bandWidth.value = BAND_WIDTH;
	arq->bandWidth.filled = true;

	// Call Reference Value
	arq->callReferenceValue.value = CALL_REFERENCE_VALUE;
	arq->callReferenceValue.filled = true;

	// ConferenceID
	arq->conferenceID = setup->conferenceID;

	// Active MC
	arq->activeMC.value = false;
	arq->activeMC.filled = true;

	// Answer Call
	arq->answerCall.value = true;
	arq->answerCall.filled = true;

	// Call Identifier
	arq->callIdentifier = setup->callIdentifier;

	arq->filled = true;

	PackAndSendRasMessage(arq, VS_RasMessage::e_admissionRequest);
}

void VS_H225RASParser::Send_ACF(unsigned int seqnum, const net::address &csAddr, net::port csPort, unsigned callModel)
{
	dprint3("VS_H225RASParser::Send_ACF\n");
	// === Make ARJ Mesage ===
	VS_RasAdmissionConfirm* acf = new VS_RasAdmissionConfirm;

	// Sequence Number
	acf->requestSeqNum.value = seqnum;
	acf->requestSeqNum.filled = true;

	// Band Width
	acf->bandWidth.value = BAND_WIDTH;
	acf->bandWidth.filled = true;

	// Call Model
	acf->callModel.tag = callModel;
	acf->callModel.choice = new VS_AsnNull;
	acf->callModel.filled = true;

	// Destination Call Signal Address
	set_ip_address(acf->destCallSignalAddress, csAddr, csPort);

	// Will Respond To IRR
	acf->willRespondToIRR.value = false;
	acf->willRespondToIRR.filled = true;

	// UUIEs Requested
	acf->uuiesRequested.setup.filled			= true;
	acf->uuiesRequested.callProceeding.filled	= true;
	acf->uuiesRequested.connect.filled			= true;
	acf->uuiesRequested.alerting.filled			= true;
	acf->uuiesRequested.information.filled		= true;
	acf->uuiesRequested.releaseComplete.filled	= true;
	acf->uuiesRequested.facility.filled			= true;
	acf->uuiesRequested.progress.filled			= true;
	acf->uuiesRequested.empty.filled			= true;
	acf->uuiesRequested.status.filled			= true;
	acf->uuiesRequested.statusInquiry.filled	= true;
	acf->uuiesRequested.setupAcknowledge.filled = true;
	acf->uuiesRequested.notify.filled			= true;
	acf->uuiesRequested.filled = true;

	acf->filled = true;

	PackAndSendRasMessage(acf, VS_RasMessage::e_admissionConfirm);
}

void VS_H225RASParser::Send_ARJ(unsigned int seqnum, unsigned reason)
{
	dprint3("VS_H225RASParser::Send_ARJ\n");
	// === Make ARJ Mesage ===
	VS_RasAdmissionReject* arj = new VS_RasAdmissionReject;

	// Sequence Number
	arj->requestSeqNum.value = seqnum;
	arj->requestSeqNum.filled = true;

	// Reject Reason
	arj->rejectReason.tag = reason;
	arj->rejectReason.choice = new VS_AsnNull;
	arj->rejectReason.filled = true;

	PackAndSendRasMessage(arj, VS_RasMessage::e_admissionReject);
}

void VS_H225RASParser::Send_DRQ(unsigned short seqnum, const char conferenceId[CONFERENCEID_LENGTH],
					const char callIdentifier[CONFERENCEID_LENGTH], bool isIncomingCall, const VS_H323String& endpointId)
{
	dprint3("VS_H225RASParser::Send_DRQ\n");
	// === Make DRQ Message ===
	VS_RasDisengageRequest* drq = new VS_RasDisengageRequest();

	// Sequence Number.
	drq->requestSeqNum.value = seqnum;
	drq->requestSeqNum.filled = true;

	// Endpoint Identifier
	drq->endpointIdentifier.value = endpointId.MakePerBuffer();
	drq->endpointIdentifier.filled = true;

	// Conference ID
	VS_PerBuffer conf_id_buff(conferenceId, CONFERENCEID_LENGTH * 8);
	drq->conferenceID.value = conf_id_buff;
	drq->conferenceID.filled = true;

	// Call Reference Value
	drq->callReferenceValue.value = CALL_REFERENCE_VALUE;
	drq->callReferenceValue.filled = true;

	// Disengage Reason
	VS_AsnNull* nuller = new VS_AsnNull;
	nuller->filled = true;
	drq->disengageReason.choice = nuller;
	drq->disengageReason.tag = VS_H225DisengageReason::e_normalDrop;
	drq->disengageReason.filled = true;

	// Call Identifier
	VS_PerBuffer call_id_buff(callIdentifier, CONFERENCEID_LENGTH * 8);
	drq->callIdentifier.guid.value = call_id_buff;
	drq->callIdentifier.guid.filled = true;
	drq->callIdentifier.filled = true;

	// Answered Call
	drq->answeredCall.value = isIncomingCall;
	drq->answeredCall.filled = true;

	drq->filled = true;

	PackAndSendRasMessage(drq, VS_RasMessage::e_disengageRequest);
}
void VS_H225RASParser::Send_DCF(unsigned short seqNum)
{
	dprint3("VS_H225RASParser::Send_DCF\n");
	// === Make DCF Message ===
	VS_RasDisengageConfirm* dcf = new VS_RasDisengageConfirm();

	// Sequence Number
	dcf->requestSeqNum.value = seqNum;
	dcf->requestSeqNum.filled = true;

	dcf->filled = true;

	PackAndSendRasMessage(dcf, VS_RasMessage::e_disengageConfirm);
}

void VS_H225RASParser::Send_DRJ()
{

}

void VS_H225RASParser::Send_URQ(unsigned short seqnum, const std::vector<std::pair<net::address, net::port>> &sigAddrs, const VS_H323String& endpointId)
{
	dprint3("VS_H225RASParser::Send_URQ\n");

	// Call Signal Address
	const std::size_t sig_addr_len = sigAddrs.size();
	VS_H225TransportAddress *ta1 = new VS_H225TransportAddress[sig_addr_len];
	for (std::size_t i = 0; i < sig_addr_len; ++i)
	{
		const auto &endpoint = sigAddrs[i];
		set_ip_address(ta1[i], endpoint.first, endpoint.second);
	}
	ta1->filled = true;


	// === Make URQ Message ===
	VS_RasUnregistrationRequest* urq = new VS_RasUnregistrationRequest;

	// Sequence Number
	urq->requestSeqNum.value = seqnum;
	urq->requestSeqNum.filled = true;
	urq->callSignalAddress.reset(ta1, sig_addr_len);

	// Endpoint Identifier
	if (!endpointId.IsEmpty())
	{
		urq->endpointIdentifier.value = endpointId.MakePerBuffer();
		urq->endpointIdentifier.filled = true;
	}

	urq->filled = true;

	PackAndSendRasMessage(urq, VS_RasMessage::e_unregistrationRequest);
}

void VS_H225RASParser::Send_UCF(unsigned short seqNum)
{
	dprint3("VS_H225RASParser::Send_UCF\n");
	// === Make UCF Mesage ===
	VS_RasUnregistrationConfirm* ucf = new VS_RasUnregistrationConfirm;

	// Sequence Number
	ucf->requestSeqNum.value = seqNum;
	ucf->requestSeqNum.filled = true;

	ucf->filled = true;

	PackAndSendRasMessage(ucf, VS_RasMessage::e_unregistrationConfirm);
}

void VS_H225RASParser::Send_URJ(unsigned short seqNum, unsigned reason)
{
	dprint3("VS_H225RASParser::Send_URJ\n");
	// === Make URJ Mesage ===
	VS_RasUnregistrationReject* urj = new VS_RasUnregistrationReject;

	// Sequence Number
	urj->requestSeqNum.value = seqNum;
	urj->requestSeqNum.filled = true;

	// Reject Reason
	urj->rejectReason.tag = reason;
	urj->rejectReason.choice = new VS_AsnNull;
	urj->rejectReason.filled = true;

	urj->filled = true;

	PackAndSendRasMessage(urj, VS_RasMessage::e_unregistrationReject);
}

bool VS_H225RASParser::PackAndSendRasMessage(VS_Asn* choice, VS_RasMessage::Choices tag, VS_ChannelID channelId)
{
	// Fill RAS message.
	VS_RasMessage ras;
	ras.tag = tag;
	ras.choice = choice;
	ras.filled = true;
	// Encode RAS message.
	VS_PerBuffer out;
	if(!ras.Encode(out)) return false;
	// Send message.
	void* data = out.GetData();
	const std::size_t data_sz = out.ByteSize();
	auto odata = vs::make_unique_default_init<unsigned char[]>(data_sz);
	memcpy(odata.get(), data, data_sz);
	return PutOutputMessage(std::move(odata), data_sz, channelId);
}

VS_PerBuffer VS_H225RASParser::MakeEncryptedToken_MD5(string_view alias, string_view password, time_t timestamp)
{
	// Token to be hashed.
	VS_H235PwdCertToken ct;
	{
		// TokenOID (must be always "0.0").
		ct.tokenOID.value[0] = 0;
		ct.tokenOID.value[1] = 0;
		ct.tokenOID.size = 2;
		ct.tokenOID.filled = true;
		// GeneralID (DialedDigits).
		ct.generalID.value = VS_H323String(alias).MakePerBuffer(true); // use string-representation of the dialedDigits.
		ct.generalID.filled = true;
		// Password.
		ct.password.value = VS_H323String(password).MakePerBuffer(true);
		ct.password.filled = true;
		// Timestamp.
		ct.timeStamp.value = (unsigned) timestamp;
		ct.timeStamp.filled = true;
		ct.filled = true;
	}
	// Make byte buffer.
	VS_PerBuffer out;
	if(!ct.Encode(out)) return VS_PerBuffer();
	// Encode bytes using MD5 algorithm.
	MD5 md5;
	md5.Update(out.GetData(), out.ByteSize());
	md5.Final();
	unsigned char digest[16];
	md5.GetBytes(digest);

	return VS_PerBuffer(digest, 16 * 8);
}

std::shared_ptr<VS_H225RASParserInfo> VS_H225RASParser::MakeParserInfo(std::string &dialogId)
{
	// Make dialog id.
	char gen_dialog_id[32 + 1]{0};
	VS_H323Parser::GenerateNewDialogID(gen_dialog_id);
	dialogId.assign(gen_dialog_id);
	// Make context.
	std::shared_ptr<VS_H225RASParserInfo> ctx = std::make_shared<VS_H225RASParserInfo>();
	ctx->SetDialogID(dialogId);
	ctx->SetExpires(DEFAULT_EXPIRES);

	// Save context.
	std::lock_guard<decltype(m_ctx_lock)> lock(m_ctx_lock);
	m_ctx.emplace(dialogId, ctx);

	return ctx;
}

std::shared_ptr<VS_H225RASParserInfo> VS_H225RASParser::FindFirstParserInfo()
{
	std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };
	return m_ctx.empty() ? std::shared_ptr<VS_H225RASParserInfo>() : m_ctx.cbegin()->second;
}

std::shared_ptr<VS_H225RASParserInfo> VS_H225RASParser::FindParserInfoByDialogID(string_view dialogId)
{
	std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };
	auto pos = m_ctx.find(dialogId);
	return pos == m_ctx.cend()? std::shared_ptr<VS_H225RASParserInfo>() : pos->second;
}

std::shared_ptr<VS_H225RASParserInfo> VS_H225RASParser::FindParserInfoByEndpointID(const VS_H323String& endpointId)
{
	std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };
	auto res = std::find_if(m_ctx.cbegin(), m_ctx.cend(),
	[&endpointId](const std::pair<std::string, std::shared_ptr<VS_H225RASParserInfo>> &item)
	{
		return item.second->GetEndpointIdentifier() == endpointId;
	});

	return res != m_ctx.cend() ? res->second : nullptr;
}

std::shared_ptr<VS_H225RASParserInfo> VS_H225RASParser::FindParserInfoByRegisteredUser(string_view id)
{
	std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };

	const auto res = std::find_if(m_ctx.cbegin(), m_ctx.cend(),
		[id](const std::pair<std::string, std::shared_ptr<VS_H225RASParserInfo>> &item)
	{
		return item.second->GetH323ID() == id || item.second->GetDialedDigits() == id;
	});

	return res != m_ctx.cend() ? res->second : nullptr;
}

std::shared_ptr<VS_H225RASParserInfo> VS_H225RASParser::FindParserInfoByRRQSeqnum(unsigned short seqnum)
{
	std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };
	auto res = std::find_if(m_ctx.cbegin(), m_ctx.cend(),
		[seqnum](const std::pair<std::string, std::shared_ptr<VS_H225RASParserInfo>> &item)
	{
		return item.second->GetLastRRQSequenceNumber() == seqnum;
	});
	return res != m_ctx.cend() ? res->second : nullptr;
}

void VS_H225RASParser::SetLastGK(string_view callId)
{
	std::shared_ptr<VS_H225RASParserInfo> ctx = FindParserInfoByDialogID(callId);
	if(!ctx)
		return;

	VS_RegistryKey cfg(false, CONFIGURATION_KEY, false, true);
	// Endpoint id.
	auto &&epid = ctx->GetEndpointIdentifier().MakeString();

	cfg.SetString(epid.c_str(), LAST_GK_EPID_KEY);
	// CS ip address and port.
	if(!ctx->GetRequestCSAddresses().empty())
	{
		auto &endpoint = ctx->GetRequestCSAddresses().front();
		auto &&addr = endpoint.first;
		if(!addr.is_unspecified())
		{
			cfg.SetString(addr.to_string().c_str(), LAST_GK_IP_KEY);

			char buff[std::numeric_limits<net::port>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
			snprintf(buff, sizeof buff, "%hu", endpoint.second);
			cfg.SetString(buff, LAST_GK_PORT_KEY);
		}
	}
}

bool VS_H225RASParser::GetLastGK(VS_H323String &epid, net::address &csAddr, net::port &csPort)
{
	const size_t buff_sz = 512;
	char buff[buff_sz] = { 0 };
	VS_RegistryKey cfg(false, CONFIGURATION_KEY, false, true);
	// Endpoint id.
	if(!cfg.GetValue(buff, buff_sz, VS_REG_STRING_VT, LAST_GK_EPID_KEY)) return false;
	epid = VS_H323String(buff);
	// CS ip address and port.
	if(!cfg.GetValue(buff, buff_sz, VS_REG_STRING_VT, LAST_GK_IP_KEY)) return false;
	boost::system::error_code ec;
	csAddr = net::address::from_string(buff, ec);
	if (ec)
		return false;
	if(!cfg.GetValue(buff, buff_sz, VS_REG_STRING_VT, LAST_GK_PORT_KEY)) return false;
	csPort = atoi(buff);
	return true;
}

void VS_H225RASParser::ClearLastGK() noexcept
{
	VS_RegistryKey cfg(false, CONFIGURATION_KEY, false, true);
	cfg.SetString("", LAST_GK_EPID_KEY);
	cfg.SetString("", LAST_GK_IP_KEY);
	cfg.SetString("", LAST_GK_PORT_KEY);
}

void VS_H225RASParser::UnregisterLastGK()
{
	// WARNING: does not work yet with more than one registrator.
	// Must be invoked only once.
	if(m_urg_last_gk) return;
	m_urg_last_gk = true;

	VS_H323String epid;
	net::address cs_addr;
	net::port cs_port;
	// If all properties are present.
	if(GetLastGK(epid, cs_addr, cs_port))
	{
		// Send URQ.
		std::shared_ptr<VS_H225RASParserInfo> ctx = FindParserInfoByEndpointID(epid);
		unsigned short seqnum = ctx ? GetSequenceNumber(ctx.get()) : GenerateSequenceNumber();
		std::vector<std::pair<net::address, net::port>> cs_addrs;
		cs_addrs.emplace_back(std::move(cs_addr), cs_port);
		Send_URQ(seqnum, cs_addrs, VS_H323String(epid));
	}
	ClearLastGK();
}

unsigned short VS_H225RASParser::GenerateSequenceNumber()
{
	srand(static_cast<unsigned int>(time(nullptr)));
	return rand() % 59001 + 1000; // from 1000 to 60000
}

unsigned short VS_H225RASParser::GetSequenceNumber(VS_H225RASParserInfo *ctx)
{
	unsigned short seqnum = ctx->GetCurrentRequestSequenceNumber();
	if (!seqnum)
	{
		seqnum = GenerateSequenceNumber();
		ctx->SetCurrentRequestSequenceNumber(seqnum);
	}
	ctx->IncreaseCurrentRequestSequenceNumber();

	return seqnum;
}

void VS_H225RASParser::LoginAsUser(string_view dialogId, string_view password, const std::vector<std::string>& aliases)
{
	std::shared_ptr<VS_H225RASParserInfo> ctx = FindParserInfoByDialogID(dialogId);
	if(!ctx) return; // This code must not be invoked.
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;

	// Save password.
	ctx->SetTranscoderPassword(std::string(password));
	// Try to login transcoder.
	const auto expire = DEFAULT_EXPIRES + clock().now();
	auto result_callback =
		[self = std::static_pointer_cast<VS_H225RASParser>(shared_from_this()), dialog_id = std::string(dialogId)](bool res)
	{
		self->VS_H225RASParser::OnLoginResult(dialog_id, res);
	};

	auto logout_callback = [self = std::static_pointer_cast<VS_H225RASParser>(shared_from_this()), dialog_id = std::string(dialogId)]
	{
		self->VS_H225RASParser::OnLogout(dialog_id);
	};

	// Now we use h323-id or dialedDigits to identify TrueConf user.
	string_view login = !ctx->GetH323ID().empty() ? ctx->GetH323ID() : ctx->GetDialedDigits();
	confMethods->LoginUser(ctx->GetDialogID(), login, password,
		expire, login, std::move(result_callback), std::move(logout_callback), aliases);

	ctx->SetTranscoder(confMethods->GetTranscoder(ctx->GetDialogID()));
}

void VS_H225RASParser::LogoutAsUser(string_view dialogId)
{
	std::shared_ptr<VS_H225RASParserInfo> ctx = FindParserInfoByDialogID(dialogId);
	if(!ctx)
		return;

	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;

	string_view login = !ctx->GetH323ID().empty() ? ctx->GetH323ID() : ctx->GetDialedDigits();

	// Send logout request to transcoder.
	confMethods->LoginUser(ctx->GetDialogID(), login, ctx->GetTranscoderPassword(),
		NULL_TICK, login, [](bool){}, [](){}, std::vector<std::string>());
}

void VS_H225RASParser::OnLoginResult(string_view dialogId, bool result)
{
	std::shared_ptr<VS_H225RASParserInfo> ctx = FindParserInfoByDialogID(dialogId);
	if(!ctx) return;

	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;

	if(result)
	{
		MakeAndSendRCF(dialogId);
		// Mark transport connection as 'for registred user'.
		confMethods->SetForRegisteredUser();
	}
	else
	{
		// Send response.
		Send_RRJ(ctx->GetLastRRQSequenceNumber(), VS_H225RegistrationRejectReason::e_resourceUnavailable);
		Unregister(ctx->GetDialogID(), false);
	}
}

void VS_H225RASParser::OnLogout(string_view dialogId)
{
	// Artem Boldarev: As far as I could tell every statement in this method should be executed in
	// the context of a transport connection thread to avoid multiple thread synchronization issues.
	// Beware!
	//
	// This is not an issue right now because this method does nothing.
	//
	// Uncomment the following block and add some code where specified  if you need to.

	/*VS_TransportConnection *tc = dynamic_cast<VS_TransportConnection *>(m_confMethods);
	if (tc != nullptr)
	{
		tc->CallInWorkerThreadContext([this]() {
			// Add your code here.
		});
	}*/
}

bool VS_H225RASParser::IsRegistratorMode() const
{
	return m_parser_mode == PM_REGISTRATOR;
}

bool VS_H225RASParser::IsGatekeeperMode() const
{
	return m_parser_mode == PM_GATEKEEPER;
}

void VS_H225RASParser::FullRegistration(VS_RasRegistrationRequest* rrq)
{
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;

	// A password string for transcoder in format:
	// $5[alias1_size*alias1*timestamp1*md5_token1][alias1_size*alias1*timestamp1*md5_token1]...
	std::string transcoder_pass("$5*");
	// For each alias.
	std::vector<VS_H225AliasAddress*> aliases;
	std::vector<std::string> ids;	// h323-ID, or \e\dialedDigits
	std::vector<std::string> aliases_str; //aliases with prefix #h323
	std::vector< std::pair<std::string, VS_H225AliasAddress::Type>> pair_alias; // aliases in format: alias name, alias type

	for (auto& alias_addr : rrq->terminalAlias)
	{

		// We use only h323-id and dialed digits.
		if (alias_addr.tag == VS_H225AliasAddress::e_h323_ID ||
			alias_addr.tag == VS_H225AliasAddress::e_dialedDigits)
		{
			auto alias_str = alias_addr.String();

			if (alias_addr.tag == VS_H225AliasAddress::e_h323_ID)
				aliases_str.push_back("#h323:" + alias_str);
			else if (alias_addr.tag == VS_H225AliasAddress::e_dialedDigits)
				aliases_str.push_back("#h323:\\e\\" + alias_str);

			pair_alias.emplace_back(alias_str,
				alias_addr.tag == VS_H225AliasAddress::e_h323_ID ? VS_H225AliasAddress::e_h323_ID : VS_H225AliasAddress::e_dialedDigits);

			{		// todo(kt): DELETE !!! TEST!!!
				unsigned tag = alias_addr.tag;
				aliases.push_back(&alias_addr);
				if (tag == VS_H225AliasAddress::e_h323_ID)
					ids.push_back(std::move(alias_str));
				else if (tag == VS_H225AliasAddress::e_dialedDigits)
					ids.push_back("\\e\\" + std::move(alias_str));
			}
		}
	}

	// Create parser context.
	std::string dialog_id;
	std::shared_ptr<VS_H225RASParserInfo> ctx = MakeParserInfo(dialog_id);
	ctx->SetEndpointIdentifier(VS_H323String(dialog_id));
	// Save registration time.
	ctx->SetLastRegTime(clock().now());

	// We have found supported alias.
	// Try to find crypto token, that subscribes this alias.
	for (auto& ct : rrq->cryptoTokens)
	{
		if (ct.tag == VS_H225CryptoH323Token::e_cryptoEPPwdHash)
		{
			VS_H225CryptoEPPwdHash* hash = static_cast<VS_H225CryptoEPPwdHash*>(ct.choice);
			auto alias = hash->alias.String();
			std::shared_ptr<VS_H225RASParserInfo> exist_ctx = FindParserInfoByRegisteredUser(alias);
			if (!!exist_ctx && exist_ctx->IsRegistred())
			{
				exist_ctx->SetLastRegTime(clock().now());
				exist_ctx->SetLastRRQSequenceNumber(rrq->requestSeqNum.value);
				MakeAndSendRCF(exist_ctx->GetDialogID());
				return;
			}
			if (hash->token.algorithmOID == OBJ_ID_MD5)
			{
				// We have found supported alias, that was subscribed.
				// Add authorization info.
				static_assert(std::numeric_limits<std::size_t>::digits10 + 1 + 1 <= 32 + 1, "!");
				char buff[32 + 1] = { 0 };
				transcoder_pass.push_back('[');
				::snprintf(buff, sizeof(buff), "%zu", alias.length());
				transcoder_pass.append(buff).push_back('*');
				transcoder_pass.append(alias).push_back('*');
				buff[0] = '\0';
				::snprintf(buff, sizeof(buff), "%u", hash->timestamp.value);
				transcoder_pass.append(buff).push_back('*');
				VS_MD5ToString(static_cast<unsigned char*>(hash->token.hash.value.GetData()), buff);
				transcoder_pass.append(buff).push_back(']');
				ctx->SetLogin(hash->alias.String());
			}
			else if (hash->token.algorithmOID == OBJ_ID_SHA1) {
				// todo(kt): not implemented yet
			}
		}
	}

	for(std::vector<VS_H225AliasAddress*>::iterator it=aliases.begin(); it!=aliases.end(); ++it)
	{
		ctx->SetAlias(*it);
	}
	// If we have found at least one subscribed alias.
	for(size_t i = 0; i < rrq->callSignalAddress.size(); i++)
	{
		net::address addr{};
		net::port port = 0;
		if (get_ip_address(rrq->callSignalAddress[i], addr, port) && !addr.is_unspecified() && port != 0)
		{
			ctx->AddRequestCSAddress({ addr, port });
		}
	}
	if(ctx->GetRequestCSAddresses().empty())
	{
		// Invalid or absent CS address.
		Send_RRJ(rrq->requestSeqNum.value, VS_H225RegistrationRejectReason::e_invalidCallSignalAddress);
		Unregister(ctx->GetDialogID(), false);
		return;
	}
	ctx->SetLastRRQSequenceNumber(rrq->requestSeqNum.value);
	// Call signalling address of the gatekeeper to send in RCF.
	// We provides only gatekeeper-routed calls.
	ctx->AddResponseCSAddress({ m_myCsEp.addr, VS_H323Parser::DEFAULT_H225CS_PORT });

	if(transcoder_pass.length() > 3)
	{
		VS_BitBuffer *productIdValue = rrq->terminalType.vendor.productId.filled ?
			&rrq->terminalType.vendor.productId.value :
			(rrq->endpointVendor.productId.filled ? &rrq->endpointVendor.productId.value : 0);
		VS_BitBuffer *versionIdValue = rrq->terminalType.vendor.versionId.filled ?
			&rrq->terminalType.vendor.versionId.value :
			(rrq->endpointVendor.versionId.filled ? &rrq->endpointVendor.versionId.value : 0);

		string_view productId = productIdValue ?
			string_view(static_cast<char*>(productIdValue->GetData()), productIdValue->ByteSize()) : string_view{};
		string_view versionId = versionIdValue ?
			string_view(static_cast<char*>(versionIdValue->GetData()), versionIdValue->ByteSize()) : string_view{};

		confMethods->SetUserEndpointAppInfo(ctx->GetDialogID(), productId, versionId);

		// Try to login terminal as TrueConf user.
		LoginAsUser(dialog_id, transcoder_pass,aliases_str);
	}
	else
	{
		VS_H323GatekeeperStorage::RegisterResult res = VS_H323GatekeeperStorage::Instance().RegisterH323Terminal(ids, ctx->GetRequestCSAddresses(), ctx->GetEndpointIdentifier());
		if (res == VS_H323GatekeeperStorage::success)
		{
			// todo(kt): TEST!! Not sure, that should call ForRegisteredUser()
			MakeAndSendRCF(dialog_id);
			// Mark transport connection as 'for registred user'.
			confMethods->SetForRegisteredUser();
		}else{
			// If we have not found any correct subscribed h323-id.
			Send_RRJ(rrq->requestSeqNum.value, VS_H225RegistrationRejectReason::e_securityDenial);
			Unregister(ctx->GetDialogID(), false);
		}
	}
}

void VS_H225RASParser::LiteRegistration(VS_RasRegistrationRequest* rrq)
{
	VS_H323String endpoint_id = VS_H323String(rrq->endpointIdentifier.value);
	VS_H323String gatekeeper_id = VS_H323String(rrq->gatekeeperIdentifier.value);

	if (VS_H323GatekeeperStorage::Instance().IsKeepAliveOfRegisteredTerminal(endpoint_id))
	{
		Send_RCF(rrq->requestSeqNum.value, m_myCsEp.addr, VS_H323Parser::DEFAULT_H225CS_PORT,
		         std::vector<VS_H225AliasAddress>(), endpoint_id, DEFAULT_EXPIRES);
		return;
	}

	std::shared_ptr<VS_H225RASParserInfo> ctx = FindParserInfoByEndpointID(endpoint_id);
	if(!ctx)
	{
		Send_RRJ(rrq->requestSeqNum.value, VS_H225RegistrationRejectReason::e_fullRegistrationRequired);
		return;
	}
	ctx->SetLastRRQSequenceNumber(rrq->requestSeqNum.value);
	MakeAndSendRCF(ctx->GetDialogID());
}

void VS_H225RASParser::MakeAndSendRCF(string_view dialogId)
{
	std::shared_ptr<VS_H225RASParserInfo> ctx = FindParserInfoByDialogID(dialogId);
	if(!ctx) return;
	// Use call signalling address of our server.
	const net::address &cs_addr = m_myCsEp.addr;
	const net::port cs_port(VS_H323Parser::DEFAULT_H225CS_PORT);
	// Support only H323-ID.
	auto &aliases_list = ctx->GetAliases();
	const auto alias_count = aliases_list.size();
	const int max_alias_sz = 2;

	if (alias_count==0) {
		return;
	}
	std::vector<VS_H225AliasAddress> aliases(alias_count);
	std::size_t index = 0;
	for (const auto& alias_name : aliases_list)
	{
		if (alias_name.second== VS_H225AliasAddress::e_dialedDigits)
		{
			aliases[index].tag = VS_H225AliasAddress::e_dialedDigits;
			VS_AsnIA5String* tmpDigit = new VS_AsnIA5String(VS_H225AliasAddress::dialedDigits_alphabet,
				sizeof(VS_H225AliasAddress::dialedDigits_alphabet),
				VS_H225AliasAddress::dialedDigits_inverse_table, 1, 128);
			tmpDigit->SetNormalString(alias_name.first.c_str(), alias_name.first.length());
			aliases[index].choice = tmpDigit;
			aliases[index].filled = true;
		}
		else if(alias_name.second == VS_H225AliasAddress::e_h323_ID)
		{
			aliases[index].tag = VS_H225AliasAddress::e_h323_ID;
			TemplBmpString<1, 256> * tmpAlias = new TemplBmpString<1, 256>;
			tmpAlias->filled = true;
			tmpAlias->value = VS_H323String(alias_name.first).MakePerBuffer();
			aliases[index].choice = tmpAlias;
			aliases[index].filled = true;
		}
		index++;
	}

	// Update last registration time.
	ctx->SetLastRegTime(clock().now());
	// Set registration state.
	ctx->SetRegistrationState(VS_H225RASParserInfo::RS_REGISTRATION_SUCCESS);
	// Send response.
	Send_RCF(ctx->GetLastRRQSequenceNumber(), cs_addr, cs_port, aliases, ctx->GetEndpointIdentifier(), ctx->GetExpires());
}

void VS_H225RASParser::ClearExpiredContextes()
{
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;

	std::list<std::pair<std::string, std::shared_ptr<VS_H225RASParserInfo>>> to_delete;
	// Find expired contextes.
	{
		std::lock_guard<decltype(m_ctx_lock)> lock(m_ctx_lock);
		auto pos = m_ctx.begin();
		while(pos != m_ctx.end())
		{
			std::shared_ptr<VS_H225RASParserInfo> ctx = pos->second;
			if(clock().now() - ctx->GetLastRegTime() > ctx->GetExpires())
			{
				std::shared_ptr<VS_H323ParserInfo> info = VS_H323Parser::FindParserInfoByRemoteTarget(ctx->GetH323ID());
				if (info && info->IsInDialog()) {
					ctx->SetExpires(ctx->GetExpires() + std::chrono::seconds(30));
				} else {
					to_delete.emplace_back(pos->first, pos->second);
				}
			}
			++pos;
		}
	}
	// Remove expired contextes.
	auto it = to_delete.begin();
	while(it != to_delete.end())
	{
		string_view login = it->second->GetH323ID().empty() ? it->second->GetH323ID():  it->second->GetDialedDigits();

		// Logout user
		confMethods->LoginUser(it->second->GetDialogID(), login, it->second->GetTranscoderPassword(), NULL_TICK, {},
			[](bool) {},
			[] {}, std::vector<std::string>());

		Unregister(it->first, true);
		++it;
	}
}

void VS_H225RASParser::CleanParserContext(string_view dialogId, SourceClean source) {
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;

	auto it = m_ctx.find(dialogId);
    if (it != m_ctx.cend())
	{
        string_view login = !it->second->GetH323ID().empty() ? it->second->GetH323ID() : it->second->GetDialedDigits();

        // Logout user
		confMethods->LoginUser(it->second->GetDialogID(), login, it->second->GetTranscoderPassword(), NULL_TICK, {}, [](bool) {}, []() {}, {});

        Unregister(it->first, true);
    }
}

#undef MakeARQRequestData
