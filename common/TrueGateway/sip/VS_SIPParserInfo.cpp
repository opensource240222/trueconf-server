#include "VS_SIPParserInfo.h"

#include "../../SIPParserLib/VS_SIPURI.h"
#include "../../SIPParserLib/VS_SIPAuthScheme.h"
#include "../../SIPParserLib/VS_SIPField_Contact.h"
#include "../../SIPParserLib/VS_SIPField_Via.h"
#include "../../SIPParserLib/VS_SIPField_RecordRoute.h"
#include "../../SIPParserLib/VS_SDPObjectFactory.h"
#include "../../SIPParserLib/VS_SDPField_MediaStream.h"
#include "../../SIPParserLib/VS_SDPCodecH264.h"
#include "tools/Server/CommonTypes.h"
#include "../CallConfig/VS_CallConfig.h"
#include "std/cpplib/base64.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/VS_Utils.h"
#include "std/debuglog/VS_Debug.h"
#include <boost/algorithm/string/predicate.hpp>
#include "std-generic/compat/memory.h"
#include <boost/algorithm/string.hpp>

namespace
{
	const auto NTLM_SA_LIFETIME = std::chrono::hours(8) - std::chrono::minutes(5); // NTLM security association lifetime (https://msdn.microsoft.com/en-us/library/dd943926(v=office.12).aspx)
	const std::chrono::milliseconds DEFAULT_KEEP_ALIVE_INTERVAL(6400); // in ms (dependent on snom ONE TCP timeout)
}

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

VS_SIPParserInfo::VS_SIPParserInfo(const std::string& server_user_agent)
	: m_my_cs_address{ {}, 0, net::protocol::none }
	, m_ready_to_die(false)
	, m_is_request(false)
	, m_message_type(TYPE_INVALID)
	, m_my_sequence_number(0)
	, m_sip_sequence_number(0)
	, m_response_code(0)
	, m_content_type(CONTENTTYPE_INVALID)
	, m_local_bandwidth(0)
	, m_remote_bandwidth(0)
	, m_sdp_session_id(VS_GetTimeOfDayMs() * 1000 + (rand() % 1000))
	, m_sdp_session_version(1)
	, m_sip_via_index(0)
	, m_sip_route{ {}, 0 }
	, m_sip_contact(nullptr)
	/*m_audio_table_index(0), m_video_table_index(0),*/
	, m_my_branch_variable_part(0)
	, m_my_transaction_in_progress(false)
	, m_auth_in_invite(false)
	, m_create_ctx_tick(clock().now())
	, m_keep_alive_interval(DEFAULT_KEEP_ALIVE_INTERVAL)
	, m_keep_alive_enabled(false)
	, m_smc_mode(SMCContinuation::Nothing)
	, m_dialog_established(false)
	, m_answered_ack(false)
	, m_direction_to_sip(false)
	, m_bye_is_sent(false)
	, m_wait_dtmf_acknoledge(false)
	, m_has_dtmf_acknoledge(false)
	, m_dtmf_pause_time(0)
	, m_dtmf_treshold(0)
	, m_no_more_fup_this_call(false)
	, m_bfcp_is_enabled(false)
	, m_bfcp_supported_roles(SDP_FLOORCTRL_ROLE_INVALID)
	, m_slideshow_active(false)
	, m_need_to_retryafter(false)
	, m_retry_after_value(0)
	, m_listen_port(0)
	, m_currConfInfo(false, false)
	, m_update_options_branch(false)
	, m_can_i_choose_codec(true), m_timerExt{}
	, m_need_call_to_redirected(false)
	, m_created_by_chat_bot(false)
	, m_is_reg_ctx(false)
	, m_is_tel(false)
	, m_server_user_agent(server_user_agent)
{
	m_timerExt.refresher = REFRESHER::REFRESHER_INVALID;

	uint32_t s = rand() + 1ul;
	m_ssrc_range_audio = { s, s };
	s += 2147483520ul;
	m_ssrc_range_video = { s, s + 99 };

	unsigned char _key[16] = {};
	VS_GenKeyByMD5(_key);
	size_t len;
	{
		base64_encode(_key, sizeof(_key), nullptr, len);
		auto str = vs::make_unique_default_init<char[]>(len);
		base64_encode(_key, sizeof(_key), str.get(), len);
		m_ice_ufrag.assign(str.get(), len);
	}
	VS_GenKeyByMD5(_key);
	{
		base64_encode(_key, sizeof(_key), nullptr, len);
		auto str = vs::make_unique_default_init<char[]>(len);
		base64_encode(_key, sizeof(_key), str.get(), len);
		m_ice_pwd.assign(str.get(), len);
	}
	VS_GenKeyByMD5(_key);
	{
		base64_encode(_key, sizeof(_key), nullptr, len);
		auto str = vs::make_unique_default_init<char[]>(len);
		base64_encode(_key, sizeof(_key), str.get(), len);
		m_ice_pwd.append(str.get(), len);
	}

}

VS_SIPParserInfo::~VS_SIPParserInfo()
{
	ClearSIPVia();
	ClearSIPContact();
	ClearSIPRouteSet();

	ClearCodecs();

	for (auto& ms: m_media_streams)
		delete ms;
	m_media_streams.clear();

	m_signal_Die(m_DialogID);
}

void VS_SIPParserInfo::ClearCodecs(void)
{
	m_lac.clear();
	m_lvc.clear();
	m_ldc.clear();
}

void VS_SIPParserInfo::ReadyToDie(const bool val) {m_ready_to_die = val;}
bool VS_SIPParserInfo::ReadyToDie() const{return m_ready_to_die;}


const std::string& VS_SIPParserInfo::GetServerUserAgent() const
{
	return m_server_user_agent;
}
void VS_SIPParserInfo::CreatedByChatBot(const bool val)
{
	m_created_by_chat_bot = val;
}

bool VS_SIPParserInfo::CreatedByChatBot() const
{
	return m_created_by_chat_bot;
}

void VS_SIPParserInfo::MsgAliveTick(const std::chrono::steady_clock::time_point t)
{
	m_msg_ctx_activity_tick = t;
}

std::chrono::steady_clock::time_point VS_SIPParserInfo::MsgAliveTick() const
{
	return m_msg_ctx_activity_tick;
}

bool VS_SIPParserInfo::ActiveMsgCtx() const
{
	return m_msg_ctx_activity_tick != std::chrono::steady_clock::time_point();
}

bool VS_SIPParserInfo::RTFSupported() const
{
	return std::find(m_accepted_text_types_remote.cbegin(), m_accepted_text_types_remote.cend(), "text/rtf") != m_accepted_text_types_remote.cend();
}

void VS_SIPParserInfo::SetAcceptedTextTypesRemote(const std::vector<std::string>& types)
{
	m_accepted_text_types_remote = types;
}

bool VS_SIPParserInfo::IsTel(void) const
{
	return m_is_tel;
}

void VS_SIPParserInfo::SetIsTel(const bool is_tel)
{
	m_is_tel = is_tel;
}

void VS_SIPParserInfo::AddCodec(string_view codec)
{
	VS_SDPObjectFactory* f = VS_SDPObjectFactory::Instance();
	assert(f);
	const auto create_codec_res = f->CreateCodec(codec);

	if (create_codec_res.error_code == TSIPErrorCodes::e_ok)
	{
		auto codec_instance = create_codec_res.p_field;
		// custom codec params
		if (codec_instance->GetCodecType() == e_videoH264)
		{
			auto &&cfg = GetConfig();
			if (cfg.codecParams.h264_payload_type.is_initialized())
			{
				auto cloned_codec = codec_instance->Clone();
				cloned_codec->SetPT(static_cast<int>(*(cfg.codecParams.h264_payload_type)));
				codec_instance = decltype(codec_instance)(std::move(cloned_codec));
			}
		}

		if (codec_instance->GetMediaType() == SDPMediaType::application_fecc)
			m_ldc.push_back(codec_instance);
		else if (codec_instance->GetMediaType() == SDPMediaType::audio)
			m_lac.push_back(codec_instance);
		else
			m_lvc.push_back(codec_instance);
	}
}

void VS_SIPParserInfo::FillCodecsFromString(string_view codecs)
{
	ClearCodecs();

	if (codecs.empty())
		return;

	for (auto it = boost::make_find_iterator(codecs, boost::algorithm::token_finder([](char x) { return x != ' '; }, boost::algorithm::token_compress_on)); !it.eof(); ++it)
	{
		AddCodec(string_view{ it->begin(), static_cast<std::size_t>(std::distance(it->begin(), it->end())) });
	}

	// additional codecs from registry
	auto &&cfg = GetConfig();
	if (cfg.H224Enabled.is_initialized() && cfg.H224Enabled.get()) {
		AddCodec("H224");
	}
	if (cfg.sip.TelephoneEventSignallingEnabled.is_initialized() && cfg.sip.TelephoneEventSignallingEnabled.get()) {
		AddCodec("telephone-event");
	}
}

std::vector<std::shared_ptr<const VS_SDPCodec>> VS_SIPParserInfo::GetLocalAudioCodecs() const
{
	return std::vector<std::shared_ptr<const VS_SDPCodec>>(m_lac.begin(), m_lac.end());
}

std::vector<std::shared_ptr<const VS_SDPCodec>> VS_SIPParserInfo::GetLocalVideoCodecs() const
{
	return std::vector<std::shared_ptr<const VS_SDPCodec>>(m_lvc.begin(), m_lvc.end());
}

std::vector<std::shared_ptr<const VS_SDPCodec>> VS_SIPParserInfo::GetLocalDataCodecs() const
{
	return std::vector<std::shared_ptr<const VS_SDPCodec>>(m_ldc.begin(), m_ldc.end());
}

bool VS_SIPParserInfo::IsRequest() const
{
	return m_is_request;
}

void VS_SIPParserInfo::IsRequest(bool b)
{
	m_is_request = b;
}

void VS_SIPParserInfo::SetMessageType(eStartLineType messageType)
{
	m_message_type = messageType;
}

eStartLineType VS_SIPParserInfo::GetMessageType() const
{
	return m_message_type;
}

int VS_SIPParserInfo::GetMySequenceNumber() const
{
	return m_my_sequence_number;
}

int VS_SIPParserInfo::IncreaseMySequenceNumber()
{
	return m_my_sequence_number++;
}

int VS_SIPParserInfo::GetSIPSequenceNumber() const
{
	return m_sip_sequence_number;
}

void VS_SIPParserInfo::SetSIPSequenceNumber(int seq)
{
	m_sip_sequence_number = seq;
}

int VS_SIPParserInfo::GetSIPProtocol() const
{
	return SIPPROTO_SIP20;
}

int VS_SIPParserInfo::GetResponseCode() const
{
	return m_response_code;
}
void VS_SIPParserInfo::SetResponseCode(int code)
{
	m_response_code = code;
}

void VS_SIPParserInfo::ResponseStr(std::string str)
{
	m_response_str = std::move(str);
}

const std::string& VS_SIPParserInfo::ResponseStr() const
{
	return m_response_str;
}

bool VS_SIPParserInfo::SetTagMy(std::string tag)
{
	if (tag.empty()) return false;
	m_my_tag = std::move(tag);
	return true;
}

const std::string& VS_SIPParserInfo::GetTagMy() const
{
	return m_my_tag;
}

void VS_SIPParserInfo::SetTagSip(std::string tag)
{
	m_sip_tag = std::move(tag);
}

const std::string& VS_SIPParserInfo::GetTagSip() const
{
	return m_sip_tag;
}

void VS_SIPParserInfo::SetEpidMy(std::string id)
{
	m_my_epid = std::move(id);
}

const std::string& VS_SIPParserInfo::GetEpidMy() const
{
	return m_my_epid;
}

void VS_SIPParserInfo::SetEpidSip(std::string id)
{
	m_sip_epid = std::move(id);
}

const std::string& VS_SIPParserInfo::GetEpidSip() const
{
	return m_sip_epid;
}



std::vector<VS_SDPField_MediaStream*>& VS_SIPParserInfo::MediaStreams()
{
	return m_media_streams;
}

VS_SDPField_MediaStream* VS_SIPParserInfo::MediaStream(size_t index, const bool create)
{
	if (index >= m_media_streams.size())
	{
		if (create)
		{
			while (index != m_media_streams.size())
				m_media_streams.push_back(nullptr);

			VS_SDPObjectFactory* factory = VS_SDPObjectFactory::Instance();
			assert(factory);

			VS_ObjectFactory::CreateFieldResult result(factory->CreateField(VS_SDPObjectFactory::SDPHeader::SDPHeader_MediaStream));
			if (TSIPErrorCodes::e_ok != result.error_code)
				return nullptr;

			VS_SDPField_MediaStream* ms = dynamic_cast<VS_SDPField_MediaStream*> (result.p_field.get());
			result.p_field.release();
			if (!ms)
				return nullptr;

			ms->SetValid(true);
			ms->SetError(TSIPErrorCodes::e_ok);
			m_media_streams.push_back(ms);
		}
		else
			return nullptr;
	}
	assert(index < m_media_streams.size());
	return m_media_streams[index];
}

/*void VS_SIPParserInfo::SetMyCsAddress(const unsigned long ip, const unsigned short port, const eConnectionType conn_type)
{
	VS_AutoLock lock(this);
	m_my_cs_address_ipport.ipv4(ip);
	m_my_cs_address_ipport.port(port);
	m_my_cs_address_ipport.type = conn_type;
}

void VS_SIPParserInfo::GetMyCsAddress(unsigned long &ip, unsigned short &port, eConnectionType &conn_type)
{
	VS_AutoLock lock(this);
	ip = m_my_cs_address_ipport.ipv4();
	port = m_my_cs_address_ipport.port();
	conn_type = m_my_cs_address_ipport.type;
}

void VS_SIPParserInfo::SetMyCsAddressV6(const in6_addr ip, const unsigned short port, const eConnectionType conn_type)
{
	VS_AutoLock lock(this);
	m_my_cs_address_ipport.ipv6(ip);
	m_my_cs_address_ipport.port(port);
	m_my_cs_address_ipport.type = conn_type;
}

void VS_SIPParserInfo::GetMyCsAddressV6(in6_addr &ip, unsigned short &port, eConnectionType &conn_type)
{
	VS_AutoLock lock(this);
	ip = m_my_cs_address_ipport.ipv6();
	port = m_my_cs_address_ipport.port();
	conn_type = m_my_cs_address_ipport.type;
}

unsigned VS_SIPParserInfo::GetAddressType() const
{
	return m_my_cs_address_ipport.getAddressType();
}*/

void VS_SIPParserInfo::SetMyMediaAddress(net::address addr)
{
	m_my_media_address = std::move(addr);
}

const net::address & VS_SIPParserInfo::GetMyMediaAddress() const
{
	return m_my_media_address;
}

void VS_SIPParserInfo::SetViaHost(std::string host)
{
	m_via_host = std::move(host);
}

const std::string &VS_SIPParserInfo::GetViaHost() const
{
	return m_via_host;
}

void VS_SIPParserInfo::SetContactHost(std::string host)
{
	m_contact_host = std::move(host);
}

const std::string& VS_SIPParserInfo::GetContactHost() const
{
	return m_contact_host;
}

void VS_SIPParserInfo::SetMyExternalCsAddress(std::string host)
{
	// Set string format value.
	m_my_external_cs_host = std::move(host);
	//TODO:FIXME
	// Set VS_IPPortAddress format value.
	//m_my_external_cs_ipport.SetIPFromHostName(m_my_external_cs_host.c_str());
}

const std::string &VS_SIPParserInfo::GetMyExternalCsAddress() const
{
	return m_my_external_cs_host;
}

VS_SIPField_Via* VS_SIPParserInfo::GetSIPVia(std::size_t index)
{
	assert(!(index > m_sip_via.size()));
	if (index >= m_sip_via.size())
		return nullptr;

	return m_sip_via[index];
}

bool VS_SIPParserInfo::SetSIPVia(const VS_SIPField_Via* via)
{
	if ( !via )
		return false;
	VS_SIPField_Via* v = new VS_SIPField_Via;
	*v = *via;
	m_sip_via.push_back( v );
	return true;
}

std::size_t VS_SIPParserInfo::GetSIPViaSize() const
{
	return m_sip_via.size();
}

void VS_SIPParserInfo::SIPDialogID(std::string id)
{
	m_DialogID = std::move(id);
}

const std::string& VS_SIPParserInfo::SIPDialogID() const
{
	return m_DialogID;
}

VS_SIPField_Via* VS_SIPParserInfo::GetSIPViaCurrent()
{
	if ( m_sip_via_index >= m_sip_via.size() )
		return nullptr;

	VS_SIPField_Via* via = m_sip_via[m_sip_via_index];

	m_sip_via_index++;
	return via;
}

const VS_SIPField_Via* VS_SIPParserInfo::GetViaTop()
{
	return GetSIPVia(0);
}


void VS_SIPParserInfo::ResetIndexSIPVia()
{
	m_sip_via_index = 0;
}

void VS_SIPParserInfo::ClearSIPVia()
{
	for(size_t i=0; i < m_sip_via.size(); i++)
	{
		if ( m_sip_via[i] )
		{
			delete m_sip_via[i];
		}
	}

	m_sip_via.clear();
}

VS_SIPField_Contact* VS_SIPParserInfo::GetSIPContact() const
{
	return m_sip_contact;
}

bool VS_SIPParserInfo::SetSIPContact(const VS_SIPField_Contact* contact)
{
	if (!contact) {
		return false;
	}
	VS_SIPField_Contact* c = new VS_SIPField_Contact;
	*c = *contact;
	delete m_sip_contact;
	m_sip_contact = c;
	return true;
}

void VS_SIPParserInfo::ClearSIPContact()
{
	delete m_sip_contact;
	m_sip_contact = nullptr;
}

void VS_SIPParserInfo::FillUriSetForEstablishedDialog(const VS_SIPURI *contact, const std::vector<VS_SIPField_RecordRoute*> &routeSet, bool isClientUA)
{
	if (!contact)
		return;

	ClearSIPRouteSet();
	const size_t routeSetSize = routeSet.size();

	if (!routeSetSize)
	{
		std::string uri;
		const auto res = contact->GetRequestURI(uri);
		assert(res);
		SetSIPRemoteTarget(std::move(uri));
	}
	else
	{
		auto r = routeSet;
		if (isClientUA)
			std::reverse(r.begin(), r.end());		// reverse headers
		auto uris = r[0]->GetURIs();
		if (isClientUA)	// reverse order
			std::reverse(uris.begin(), uris.end());	// reverse <uri>'s inside one header
		if (uris.empty())
			return;
		bool looseRouting = uris[0]->lr();
		std::string requestUri;

		// for strict routing, the first URI from the route set goes to the Request-URI
		// and the Contact URI is added to the end of the route set
		if (looseRouting)
			contact->GetRequestURI(requestUri);
		else {
			uris[0]->GetRequestURI(requestUri);
			uris.erase(uris.begin());
		}

		for (const auto& uri : uris)
			AddRouteToSet(uri.get());

		r.erase(r.begin());		// skip first Record-Route header

		for (const auto& rest_routes : r)	// just add other route headers not processing them
		{
			auto uris = rest_routes->GetURIs();
			if (isClientUA)
				std::reverse(uris.begin(), uris.end());
			for (const auto& uri : uris)
				AddRouteToSet(uri.get());
		}

		if (!looseRouting)
			AddRouteToSet(contact);
		SetSIPRemoteTarget(std::move(requestUri));
	}
}

void VS_SIPParserInfo::AddRouteToSet(const VS_SIPURI *route)
{
	if (route && route->IsValid())
	{
		auto uri = new VS_SIPURI;
		*uri = *route;
		m_sip_route.set.push_back(uri);
	}
}

void VS_SIPParserInfo::ClearSIPRouteSet()
{
	for (size_t i = 0; i < m_sip_route.set.size(); ++i)
		delete m_sip_route.set[i];

	m_sip_route.set.clear();
	m_sip_route.index = 0;
}

size_t VS_SIPParserInfo::GetSIPRouteSetSize() const
{
	return m_sip_route.set.size();
}

const VS_SIPURI* VS_SIPParserInfo::GetNextSIPRouteFromSet()
{
	if (m_sip_route.set.size() > m_sip_route.index)
	{
		return !m_is_request ? m_sip_route.set[m_sip_route.index++] : m_sip_route.set[m_sip_route.index--];
	}
	return nullptr;
}

void VS_SIPParserInfo::ResetSIPRouteIndex()
{
	if(!m_is_request) m_sip_route.index = 0;
	else m_sip_route.index = m_sip_route.set.size() - 1;
}

void VS_SIPParserInfo::MyBranch(std::string branch)
{
	m_my_branch_const_part = std::move(branch);
	m_my_branch_variable_part = 0;
}

std::string VS_SIPParserInfo::MyBranch() const
{

	char var_part[1 /*-*/ + std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 + 1 /*0-terminator*/] = { '-' };
	snprintf(var_part + 1, sizeof(var_part) - 1 /*-*/, "%d", m_my_branch_variable_part);
	std::string my_branch = m_my_branch_const_part;
	my_branch += var_part;
	return my_branch;
}

void VS_SIPParserInfo::AlterMyBranch()
{
	++m_my_branch_variable_part;
}

void VS_SIPParserInfo::SetDisplayNameMy(std::string name)
{
	m_my_display_name = std::move(name);
}

const std::string& VS_SIPParserInfo::GetDisplayNameMy() const
{
	return m_my_display_name;
}

void VS_SIPParserInfo::SetDisplayNameSip(std::string name)
{
	m_sip_display_name = std::move(name);
}

const std::string& VS_SIPParserInfo::GetDisplayNameSip() const
{
	return m_sip_display_name;
}

void VS_SIPParserInfo::SipInstance(std::string str)
{
	m_sip_instance = std::move(str);
}

const std::string& VS_SIPParserInfo::SipInstance() const
{
	return this->m_sip_instance;
}

eContentType VS_SIPParserInfo::GetContentType() const
{
	return m_content_type;
}

void VS_SIPParserInfo::SetContentType(const eContentType type)
{
	m_content_type = type;
}

void VS_SIPParserInfo::SetLocalBandwidth(const unsigned int bandwidth)
{
	m_local_bandwidth = bandwidth;
}

unsigned int VS_SIPParserInfo::GetLocalBandwidth() const
{
	return m_local_bandwidth;
}

void VS_SIPParserInfo::SetRemoteBandwidth(const unsigned int bandwidth)
{
	m_remote_bandwidth = bandwidth;
}

unsigned int VS_SIPParserInfo::GetRemoteBandwidth() const
{
	return m_remote_bandwidth;
}

uint64_t VS_SIPParserInfo::GetSDPSessionId() const
{
	return m_sdp_session_id;
}

unsigned int VS_SIPParserInfo::GetSDPSessionVersion() const
{
	return m_sdp_session_version;
}

void VS_SIPParserInfo::IncreaseSDPSessionVersion()
{
	++m_sdp_session_version;
}

std::shared_ptr<VS_SIPAuthScheme> VS_SIPParserInfo::GetAuthScheme() const
{
	return m_auth_scheme;
}

void VS_SIPParserInfo::SetAuthScheme(const std::shared_ptr<VS_SIPAuthScheme>& scheme)
{
	if ( !scheme )
		return;
	m_auth_scheme = scheme;
}

bool VS_SIPParserInfo::IsAuthInInvite() const
{
	return m_auth_in_invite;
}

void VS_SIPParserInfo::AuthInInvite(const bool isAuth)
{
	m_auth_in_invite = isAuth;
}

bool VS_SIPParserInfo::VerifyNTLM_SA_lifetime() const
{
	if (!IsNTLMContext())  return true;
	return !(clock().now() - m_create_ctx_tick > NTLM_SA_LIFETIME);
}

bool VS_SIPParserInfo::IsNTLMContext() const
{
	return m_auth_scheme && m_auth_scheme->scheme() == SIP_AUTHSCHEME_NTLM;
}

std::chrono::steady_clock::time_point VS_SIPParserInfo::GetCreateCtxTick() const
{
	return m_create_ctx_tick;
}

std::chrono::steady_clock::time_point VS_SIPParserInfo::GetRingingStartTick() const
{
	return m_ringing_start_tick;
}

void VS_SIPParserInfo::SetRingingStartTick(const std::chrono::steady_clock::time_point tick)
{
	m_ringing_start_tick = tick;
}

std::chrono::steady_clock::time_point VS_SIPParserInfo::GetByeTick() const
{
	return m_bye_tick;
}

void VS_SIPParserInfo::SetByeTick(const std::chrono::steady_clock::time_point tick)
{
	m_bye_tick = tick;
}

bool VS_SIPParserInfo::IsByeSent() const
{
	return m_bye_is_sent;
}

void VS_SIPParserInfo::ByeIsSent()
{
	m_bye_is_sent = true;
}

std::chrono::steady_clock::time_point VS_SIPParserInfo::GetRegisterTick() const
{
	return m_register_tick;
}

void VS_SIPParserInfo::SetRegisterTick(std::chrono::steady_clock::time_point tick)
{
	m_register_tick = tick;
}

void VS_SIPParserInfo::SetRegCtxDialogID(const std::string& id)
{
	m_regCtxDialogID = id;
}

std::string VS_SIPParserInfo::GetRegCtxDialogID() const
{
	return m_regCtxDialogID;
}

std::chrono::steady_clock::time_point VS_SIPParserInfo::GetOptionsTick() const
{
    return m_options_tick;
}

void VS_SIPParserInfo::SetOptionsTick(std::chrono::steady_clock::time_point tick) {
    m_options_tick = tick;
}

bool VS_SIPParserInfo::IsKeepAliveSendNeeded()
{
	if (m_direction_to_sip && m_keep_alive_enabled && clock().now() - m_keep_alive_tick >= m_keep_alive_interval)
	{
		m_keep_alive_tick = clock().now();
		return true;
	}

	return false;
}

bool VS_SIPParserInfo::IsKeepAliveEnabled() const
{
	return m_keep_alive_enabled;
}

void VS_SIPParserInfo::EnableKeepAlive()
{
	m_keep_alive_enabled = true;
	m_keep_alive_tick = clock().now();
}

void VS_SIPParserInfo::SetKeepAliveInterval(std::chrono::steady_clock::duration interval)
{
	m_keep_alive_interval = interval;
	if (m_keep_alive_enabled)
		m_keep_alive_tick = clock().now();
}

bool VS_SIPParserInfo::IsSessionTimerEnabled() const
{
	return m_session_timer_enabled;
}

void VS_SIPParserInfo::DisableSessionTimer()
{
	m_session_timer_enabled = false;
}

void VS_SIPParserInfo::EnableSessionTimer()
{
	m_session_timer_enabled = true;
}

bool VS_SIPParserInfo::IsSessionTimerUsed() const
{
	return m_use_session_timer;
}

void VS_SIPParserInfo::UnuseSessionTimer()
{
	m_use_session_timer = false;
}

bool VS_SIPParserInfo::IsSessionTimerInRequire() const
{
	return m_session_timer_in_require;
}

void VS_SIPParserInfo::AddSessionTimerToRequireHeader()
{
	m_session_timer_in_require = true;
}

void VS_SIPParserInfo::RemoveSessionTimerFromRequireHeader()
{
	m_session_timer_in_require = false;
}

bool VS_SIPParserInfo::IsWeRefresh() const
{
	return m_is_we_refresh;
}

void VS_SIPParserInfo::IsWeRefresh(bool b)
{
	m_is_we_refresh = b;
}

void VS_SIPParserInfo::UseSessionTimer()
{
	m_use_session_timer = true;
}

bool VS_SIPParserInfo::IsAnswered() const
{
	return m_answered_tick != std::chrono::steady_clock::time_point() && m_answered_ack;
}
void VS_SIPParserInfo::IsAnswered(const bool isAnswered)
{
	m_answered_ack = isAnswered;
}
void VS_SIPParserInfo::SetAnswered(const std::chrono::steady_clock::time_point tick)
{
	m_answered_tick = tick;
}
std::chrono::steady_clock::time_point VS_SIPParserInfo::GetAnswered() const
{
	return m_answered_tick;
}

std::shared_ptr<VS_SIPRequest> VS_SIPParserInfo::GetLastInvite() const
{
	return m_last_invite;
}

void VS_SIPParserInfo::SetLastInvite(const std::shared_ptr<VS_SIPRequest>& invite)
{
	m_last_invite = invite;
}

VS_SIPParserInfo::SMCContinuation VS_SIPParserInfo::GetSMCContinuation() const
{
	return m_smc_mode;
}

void VS_SIPParserInfo::SetSMCContinuation(const SMCContinuation mode)
{
	m_smc_mode = mode;
}

std::chrono::steady_clock::time_point VS_SIPParserInfo::GetSMCTick() const
{
	return m_smc_tick;
}

void VS_SIPParserInfo::SetSMCTick(const std::chrono::steady_clock::time_point tick)
{
	m_smc_tick = tick;
}

bool VS_SIPParserInfo::IsDialogEstablished() const
{
	return m_dialog_established;
}

void VS_SIPParserInfo::DialogEstablished(bool val)
{
	m_dialog_established = val;
}

bool VS_SIPParserInfo::IsDirection_ToSIP() const
{
	return m_direction_to_sip;
}

void VS_SIPParserInfo::SetDirection(const bool IsToSIP)
{
	m_direction_to_sip = IsToSIP;
}

void VS_SIPParserInfo::AddDTMF(const char digit)
{
		m_dtmf_digits.push(digit);
}

char VS_SIPParserInfo::GetDTMF()
{
	if (m_dtmf_digits.empty())
		return 0;
	char digit = m_dtmf_digits.front();
	m_dtmf_digits.pop();
	return digit;
}

void VS_SIPParserInfo::SetDTMF_Treshold(std::uint32_t treshold)
{
	m_dtmf_treshold = treshold;
}

std::uint32_t VS_SIPParserInfo::GetDTMF_Treshold() const
{
	return m_dtmf_treshold;
}

void VS_SIPParserInfo::NoMoreFUP()
{
	m_no_more_fup_this_call = true;
}

bool VS_SIPParserInfo::IsNoMoreFUP() const
{
	return m_no_more_fup_this_call;
}

void VS_SIPParserInfo::LimitH264Level(int level)
{
	for (size_t i = 0; i < m_lvc.size(); ++i)
	{
		if (m_lvc[i]->GetCodecType() == e_videoH264)
		{
			VS_SDPCodecH264 *h264 = dynamic_cast<VS_SDPCodecH264*>(const_cast<VS_SDPCodec*>(m_lvc[i].get()));
			if (h264)
			{
				// Create copy of the codec as we changing the value inside of it. Otherwise we would modify the global object.
				// Fixes #45659.
				auto copy = std::make_shared<VS_SDPCodecH264>(*h264);
				copy->LimitLevel(level);
				m_lvc[i] = copy;
			}
			break;
		}
	}
}

void VS_SIPParserInfo::SetPasswordTranscoder(std::string password)
{
	m_password_transcoder = std::move(password);
}

const std::string &VS_SIPParserInfo::GetPasswordTranscoder() const
{
	return m_password_transcoder;
}

void VS_SIPParserInfo::SetListenPort(net::port port)
{
	m_listen_port = port;
}

net::port VS_SIPParserInfo::GetListenPort() const
{
	return m_listen_port;
}

void VS_SIPParserInfo::SetInviteAfterRegister(std::shared_ptr<VS_SIPParserInfo> val)
{
	m_IsInviteAfterRegister = std::move(val);
}

std::shared_ptr<VS_SIPParserInfo> VS_SIPParserInfo::IsInviteAfterRegister() const
{
	return m_IsInviteAfterRegister;
}

bool VS_SIPParserInfo::IsRegisterContext() const
{
	return m_is_reg_ctx;
}

void VS_SIPParserInfo::IsRegisterContext(bool val)
{
	m_is_reg_ctx = val;
}

bool VS_SIPParserInfo::SetConfig(VS_CallConfig cfg)
{
	VS_ParserInfo::SetConfig(std::move(cfg));

	auto &c = GetConfig();
	if (!c.Codecs.empty())
	{
		FillCodecsFromString(c.Codecs);
	}
	else
	{
		FillCodecsFromString(DEFAULT_ENABLED_CODECS);
	}
	return true;
}

bool VS_SIPParserInfo::IsCompactHeaderAllowed() const {
	return GetConfig().sip.CompactHeader.get_value_or(false);
}

bool VS_SIPParserInfo::UseSingleBestCodec() const
{
	return !m_direction_to_sip && GetConfig().sip.UseSingleBestCodec.get_value_or(false);
}

bool VS_SIPParserInfo::IsGroupConf() const
{
	return m_currConfInfo.is_group_conf;
}

void VS_SIPParserInfo::SetGroupConf(bool b) {
	m_currConfInfo.is_group_conf = b;
}

bool VS_SIPParserInfo::IsPublicConf() const
{
	return m_currConfInfo.is_public_conf;
}

void VS_SIPParserInfo::SetPublicConf(const bool b)
{
	m_currConfInfo.is_public_conf = b;
}

bool VS_SIPParserInfo::NoRtpmapForAudioStaticPayload() const
{
	return GetConfig().sip.NoRtpmapForAudioStaticPT.get_value_or(false);
}

bool VS_SIPParserInfo::NoRtpmapForVideoStaticPayload() const
{
	return GetConfig().sip.NoRtpmapForVideoStaticPT.get_value_or(false);
}

bool VS_SIPParserInfo::ICEEnabled() const
{
	return  GetConfig().sip.ICEEnabled.get_value_or(false);
}

bool VS_SIPParserInfo::SRTPEnabled() const
{
	return  GetConfig().sip.SRTPEnabled.get_value_or(false);
}

bool VS_SIPParserInfo::HaveAuthenticatedTLSConnection() const {
	if (!secureCtx) return false;
	return secureCtx->sspi.ContexInited();
}

unsigned VS_SIPParserInfo::GetCnum() const
{
	return !secureCtx ? 0 : secureCtx->cnum();
}

unsigned VS_SIPParserInfo::GetIncrCnum() const
{
	return !secureCtx ? 0 : secureCtx->get_incr_cnum();
}

void VS_SIPParserInfo::SetSRTPKey(std::string s) {
	m_srtp_key = std::move(s);
}

const std::string &VS_SIPParserInfo::GetSRTPKey() const
{
	return m_srtp_key;
}

const std::string& VS_SIPParserInfo::IceUfrag() const
{
	return m_ice_ufrag;
}

const std::string& VS_SIPParserInfo::IcePwd() const
{
	return m_ice_pwd;
}

const std::string& VS_SIPParserInfo::GetContactGruu() const
{
	return m_contact_gruu;
}

void VS_SIPParserInfo::SetContactGruu(std::string s) {
	m_contact_gruu = std::move(s);
}

void VS_SIPParserInfo::AddPendingInviteUser(const std::string fromId) {
	m_pending_invites.push_back(std::move(fromId));
}

std::vector<std::string> VS_SIPParserInfo::PopPendingInvites() {
	return std::move(m_pending_invites);
}

size_t VS_SIPParserInfo::GetPendingInvitesSize() const {
	return m_pending_invites.size();
}

void VS_SIPParserInfo::AddPendingReqInviteUser(std::string fromId)
{
	m_pending_req_invites.insert(std::move(fromId));
}

bool VS_SIPParserInfo::GetPendingReqInvite(string_view user, std::string &fullId)
{
	// one req invite, just pop out last
	if (m_pending_req_invites.size() == 1) {
		fullId = *m_pending_req_invites.begin();
		m_pending_req_invites.clear();
		return true;
}

	// several req invites, try to find unique
	auto it = std::find_if(m_pending_req_invites.cbegin(), m_pending_req_invites.cend(), [&](const std::string& user_id) { return boost::starts_with(user_id, user); });
	if (it == m_pending_req_invites.cend()) return false;

	auto it2 = std::find_if(std::next(it), m_pending_req_invites.cend(), [&](const std::string& user_id) { return boost::starts_with(user_id, user); });
	if (it2 != m_pending_req_invites.cend()) return false;	// not unique entry

	fullId = *it;
	m_pending_req_invites.erase(it);
	return true;
}

void VS_SIPParserInfo::AddPengingMessage(string_view msg, eContentType ct) {
	m_pending_messages.emplace_back(std::string(msg), ct);
}

std::vector<std::tuple<std::string, eContentType>> VS_SIPParserInfo::PopPendingMessages() {
		return std::move(m_pending_messages);
}

void VS_SIPParserInfo::SetConfTopic(const std::string &s) {
	m_currConfInfo.topic = s;
}

const std::string &VS_SIPParserInfo::GetConfTopic() const
{
	return m_currConfInfo.topic;
}

void VS_SIPParserInfo::SetConfID(const std::string & s)
{
	m_currConfInfo.ID = s;
}

const std::string & VS_SIPParserInfo::GetConfID() const
{
	return m_currConfInfo.ID;
}

const VS_ConferenceInfo & VS_SIPParserInfo::GetConfInfo() const
{
	return m_currConfInfo;
}

std::pair<std::uint32_t, std::uint32_t> VS_SIPParserInfo::GetSsrcRangeAudio() const {
	return m_ssrc_range_audio;
}

std::pair<std::uint32_t, std::uint32_t> VS_SIPParserInfo::GetSsrcRangeVideo() const {
	return m_ssrc_range_video;
}

TimerExtention& VS_SIPParserInfo::GetTimerExtention()
{
	return m_timerExt;
}

bool VS_SIPParserInfo::CanIChooseCodec() const
{
	return m_can_i_choose_codec;
}

bool VS_SIPParserInfo::CanIChooseCodec(const bool val)
{
	return m_can_i_choose_codec = val;
}

bool VS_SIPParserInfo::InCall() const
{
		const bool has_media_stream = std::any_of(m_media_streams.begin(), m_media_streams.end(), [](VS_SDPField_MediaStream *pMS) {
		return pMS && (pMS->GetMediaType() == SDPMediaType::audio || pMS->GetMediaType() == SDPMediaType::video); });
	return (this->IsAnswered() && has_media_stream) || this->GetRegisterTick() != std::chrono::steady_clock::time_point();
}

const std::string &VS_SIPParserInfo::GetInviteAfterOptions() const
{
	return m_invite_after_options;
}


void VS_SIPParserInfo::SetInviteAfterOptions(std::string val) {
	m_invite_after_options = std::move(val);
}


void VS_SIPParserInfo::EnableBFCP()
{
	m_bfcp_is_enabled = true;
}

void VS_SIPParserInfo::DisableBFCP()
{
	m_bfcp_is_enabled = false;
}

bool VS_SIPParserInfo::IsBFCPEnabled() const
{
	return m_bfcp_is_enabled;
}

unsigned int VS_SIPParserInfo::GetBFCPSupportedRoles() const
{
	return m_bfcp_supported_roles;
}

void VS_SIPParserInfo::SetBFCPSupportedRoles(const unsigned int roles)
{
	m_bfcp_supported_roles = roles;
	if (m_bfcp_supported_roles == SDP_FLOORCTRL_ROLE_INVALID)
		m_bfcp_is_enabled = false;
}

net::port VS_SIPParserInfo::GetBFCPLocalPort() const
{
	auto bfcp_ms_it = std::find_if(m_media_streams.cbegin(), m_media_streams.cend(), [](const VS_SDPField_MediaStream* ms) {
		return ms->GetMediaType() == SDPMediaType::application_bfcp;
	});
	return bfcp_ms_it != m_media_streams.cend() ? (*bfcp_ms_it)->GetLocalPort() : DISCARD_PROTOCOL_PORT;
}

boost::shared_ptr<bfcp::ClientSession> VS_SIPParserInfo::GetBFCPClientSession() const
{
	return m_bfcp_client_session;
}

void VS_SIPParserInfo::SetBFCPClientSession(const boost::shared_ptr<bfcp::ClientSession>& session)
{
	m_bfcp_client_session = session;
}


boost::shared_ptr<bfcp::ServerSession> VS_SIPParserInfo::GetBFCPServerSession() const
{
	return m_bfcp_server_session;
}

void VS_SIPParserInfo::SetBFCPServerSession(const boost::shared_ptr<bfcp::ServerSession>& session)
{
	m_bfcp_server_session = session;
}

std::shared_ptr<VS_SignalChannel> VS_SIPParserInfo::GetBFCPChannel() const
{
	return m_bfcp_channel;
}

void VS_SIPParserInfo::SetBFCPChannel(const std::shared_ptr<VS_SignalChannel>& channel)
{
	m_bfcp_channel = channel;
}

bool VS_SIPParserInfo::GetSlideshowState() const
{
	return m_slideshow_active;
}

void VS_SIPParserInfo::SetSlideshowState(const bool active)
{
	m_slideshow_active = active;
}

bool VS_SIPParserInfo::WaitDTMFAcknowledge() const {
	return m_wait_dtmf_acknoledge;
}

void VS_SIPParserInfo::WaitDTMFAcknowledge(const bool b)
{
	m_wait_dtmf_acknoledge = b;
}

bool VS_SIPParserInfo::HasDTMFAcknowledge() const {
	return m_has_dtmf_acknoledge;
}

void VS_SIPParserInfo::HasDTMFAcknowledge(const bool b)
{
	m_has_dtmf_acknoledge = b;
}

void VS_SIPParserInfo::SetDTMFRequestTime(const std::chrono::steady_clock::time_point time) {
	m_last_dtmf_request_time = time;
}

std::chrono::steady_clock::time_point VS_SIPParserInfo::GetLastDTMFRequestTime() const {
	return m_last_dtmf_request_time;
}

bool VS_SIPParserInfo::HasDTMFAnswerTimeout() const
{
	return std::chrono::steady_clock::now() - m_last_dtmf_request_time >= std::chrono::seconds(2); // wait for 2 seconds
}

void VS_SIPParserInfo::SetDTMFPauseTime(std::chrono::steady_clock::duration time)
{
	m_dtmf_pause_time = time;
}

std::chrono::steady_clock::duration VS_SIPParserInfo::GetDTMFPauseTime() const
{
	return m_dtmf_pause_time;
}

bool VS_SIPParserInfo::DTMFPausePassed() const
{
	return clock().now() - m_last_dtmf_request_time >= m_dtmf_pause_time;
}

void VS_SIPParserInfo::SetMyCsAddress(const net::Endpoint &addr)
{
	m_my_cs_address = std::move(addr);
}

const net::Endpoint &VS_SIPParserInfo::GetMyCsAddress() const
{
	return m_my_cs_address;
}


void VS_SIPParserInfo::AddDTMFPauseTime(std::chrono::steady_clock::duration time)
{
	m_dtmf_pause_time += time;
}

void VS_SIPParserInfo::SetLastDTMFBarnch(std::string branch)
{
	m_last_dtmf_branch = std::move(branch);
}

const std::string &VS_SIPParserInfo::GetLastDTMFBranch() const
{
	return m_last_dtmf_branch;
}

void  VS_SIPParserInfo::RetryAfterTime(std::chrono::steady_clock::time_point p) {
	m_time_to_retryafter = p;
}

std::chrono::steady_clock::time_point VS_SIPParserInfo::RetryAfterTime() const {
	return m_time_to_retryafter;
}

void VS_SIPParserInfo::NeedUpdateOptionsBranch(const bool val)
{
	m_update_options_branch = val;
}

bool VS_SIPParserInfo::DoINeedUpdateOptionsBranch() const
{
	return m_update_options_branch;
}

void VS_SIPParserInfo::NeedRetryAfter(bool b) {
	m_need_to_retryafter = b;
}
bool VS_SIPParserInfo::NeedRetryAfter() const {
	return m_need_to_retryafter;
}

std::chrono::steady_clock::duration VS_SIPParserInfo::GetRetryAfterValue() const {
	return m_retry_after_value;
}

void VS_SIPParserInfo::SetRetryAfterValue(std::chrono::steady_clock::duration value)
{
	m_retry_after_value = value;
}

VS_SIPParserInfo::RedirectCache::RedirectCache(): m_not_called_addresses(0)
{
}

bool VS_SIPParserInfo::RedirectCache::InsertNewAddress(string_view address) {

	if (m_cache.find(address) == m_cache.cend())
	{
		m_cache.emplace(std::string(address), false);	// mark new address as not called
		++m_not_called_addresses;
		return true;
	}
	return false;
}
bool VS_SIPParserInfo::RedirectCache::GetAddressToCall(std::string &outAddress) {

	if (!HaveNotCalledAddresses())
		return false;

	for (auto &address : m_cache)
	{
		if (!address.second)
		{
			outAddress = address.first;
			address.second = true;
			--m_not_called_addresses;
			break;
		}
	}
	return true;
}
bool VS_SIPParserInfo::RedirectCache::MarkAsCalled(string_view address) {
	const auto it = m_cache.find(address);
	if (it != m_cache.cend())
	{
		it->second = true;
		--m_not_called_addresses;
		return true;
	}
	return false;
}
bool VS_SIPParserInfo::RedirectCache::HaveNotCalledAddresses() const {
	return m_not_called_addresses > 0;
}

void VS_SIPParserInfo::InsertCallIdsToRedirect(const std::vector<std::string> &IDs) {
	auto end = IDs.end();
	for (auto it = IDs.begin(); it != end; ++it){
		m_redirect_cache.InsertNewAddress(*it);
	}
}
bool VS_SIPParserInfo::HaveAddressesToRedirect() const {
	return m_redirect_cache.HaveNotCalledAddresses();
}
bool VS_SIPParserInfo::GetAddressToRedirect(std::string &outAddress) {
	return m_redirect_cache.GetAddressToCall(outAddress);
}
bool VS_SIPParserInfo::NeedAddressRedirection() const {
	return m_need_call_to_redirected;
}
void VS_SIPParserInfo::NeedAddressRedirection(const bool val) {
	m_need_call_to_redirected = val;
}

bool VS_SIPParserInfo::NeedsOptionsBeforeInvite(void) const
{
	//return !cfg->sip.SkipOPTIONS.get_value_or(false);
	return GetConfig().sip.RequestOPTIONS.get_value_or(false);
}