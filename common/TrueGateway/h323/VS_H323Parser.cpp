#include "VS_H323Parser.h"
#include "net/QoSSettings.h"
#include "FakeClient/VS_ConferenceInfo.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std-generic/cpplib/ignore.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_Utils.h"
#include "std/debuglog/VS_Debug.h"
#include "TrueGateway/CallConfig/VS_Indentifier.h"
#include "TrueGateway/h323/VS_H225RASParserInfo.h"
#include "TrueGateway/h323/VS_H323ExternalGatekeeper.h"
#include "TrueGateway/h323/VS_H323GatekeeperStorage.h"
#include "TrueGateway/h323/VS_H323ParserInfo.h"
#include "TrueGateway/net/VS_SignalChannel.h"
#include "tools/H323Gateway/Lib/src/VS_Q931.h"
#include "tools/H323Gateway/Lib/VS_H323CapabilityGenerator.h"
#include "tools/Server/vs_messageQueue.h"

#include <boost/optional.hpp>
#include <boost/algorithm/string/predicate.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_H323PARSER
#define NULL_TICK std::chrono::steady_clock::time_point()

// glumov>>> ktrushnikov's mail, hotfix for sjphone
//#define H323_SJPHONE_HOTFIX

namespace
{
	const VS_GwAsnObjectId h225ProtocolIdentifier{ 0, 0, 8, 2250, 0, 4 };
	const VS_GwAsnObjectId h245ProtocolIdentifier{ 0, 0, 8, 245, 0, 10 };

	const VS_GwAsnObjectId oid_h264_gc{ 0, 0, 8, 241, 0, 0, 1 }; // H.264 generic capabilities (see H.241 #8.3.2)
	const VS_GwAsnObjectId oid_h264_annex_a{ 0, 0, 8, 241, 0, 0, 0, 0 }; // H.264 Annex A packetization
	const VS_GwAsnObjectId oid_h264_non_interveaved{ 0, 0, 8, 241, 0, 0, 0, 1 }; // H.264 Non Interveaved packetization

	const VS_GwAsnObjectId oid_h239_cc{ 0, 0, 8, 239, 1, 1 }; // h239ControlCapability
	const VS_GwAsnObjectId oid_h239_evc{ 0, 0, 8, 239, 1, 2 }; // h239ExtendedVideoCapability
	const VS_GwAsnObjectId oid_h239_gm{ 0, 0, 8, 239, 2 }; // GenericMessage identifier for H.239

	const VS_GwAsnObjectId oid_h224_gc{ 0, 0, 8, 224, 1, 0 }; // h224 generic capability

	constexpr unsigned c_h239_smid_flowControlReleaseRequest = 1;
	constexpr unsigned c_h239_smid_flowControlReleaseResponse = 2;
	constexpr unsigned c_h239_smid_presentationTokenRequest = 3;
	constexpr unsigned c_h239_smid_presentationTokenResponse = 4;
	constexpr unsigned c_h239_smid_presentationTokenRelease = 5;
	constexpr unsigned c_h239_smid_presentationTokenIndicateOwner = 6;

	constexpr unsigned c_h239_pid_bitRate = 41;
	constexpr unsigned c_h239_pid_channelId = 42;
	constexpr unsigned c_h239_pid_symmetryBreaking = 43;
	constexpr unsigned c_h239_pid_terminalLabel = 44;
	constexpr unsigned c_h239_pid_acknowledge = 126;
	constexpr unsigned c_h239_pid_reject = 127;

	const std::chrono::steady_clock::duration c_h239_indication_period = std::chrono::seconds(15);


	inline bool CheckH245CapabilityIdentifier(const VS_H245CapabilityIdentifier& cid, const VS_AsnObjectId& oid)
	{
		auto value = dynamic_cast<VS_AsnObjectId*>(cid.choice);
		return cid.tag == VS_H245CapabilityIdentifier::e_standard && value && *value == oid;
	}

	inline void SetH245CapabilityIdentifier(VS_H245CapabilityIdentifier& cid, const VS_AsnObjectId& oid)
	{
		cid.choice = new VS_AsnObjectId(oid);
		cid.tag = VS_H245CapabilityIdentifier::e_standard;
		cid.filled = true;
	}

	inline bool CheckH245ParameterIdentifier(const VS_H245ParameterIdentifier& pid, unsigned id)
	{
		auto value = dynamic_cast<VS_AsnInteger*>(pid.choice);
		return pid.tag == VS_H245ParameterIdentifier::e_standard && value && value->value == id;
	}

	inline void SetH245ParameterIdentifier(VS_H245ParameterIdentifier& pid, unsigned id)
	{
		pid.choice = new VS_AsnInteger(id, VS_Asn::FixedConstraint, 0, 127);
		pid.tag = VS_H245ParameterIdentifier::e_standard;
		pid.filled = true;
	}

	inline bool GetH245ParameterValue_logical(const VS_H245GenericParameter& param, unsigned pid)
	{
		if (!CheckH245ParameterIdentifier(param.parameterIdentifier, pid))
			return false;
		/* Polycom sends H.239 acknowledge/reject as INTEGER instead of NULL, so this sanity checks are disabled.
		auto value = dynamic_cast<VS_AsnNull*>(param.parameterValue.choice);
		if (param.parameterValue.tag != VS_H245ParameterValue::e_logical || !value)
			return false;
		*/
		return true;
	}

	inline void SetH245ParameterValue_logical(VS_H245GenericParameter& param, unsigned pid)
	{
		SetH245ParameterIdentifier(param.parameterIdentifier, pid);
		param.parameterValue.choice = new VS_AsnNull;
		param.parameterValue.tag = VS_H245ParameterValue::e_logical;
		param.parameterValue.filled = true;
		param.filled = true;
	}

template <class InputIterator>
	inline bool FindH245ParameterValue_logical(InputIterator first, InputIterator last, unsigned pid)
{
	for (; first != last; ++first)
	{
		auto result = GetH245ParameterValue_logical(*first, pid);
		if (result)
			return result;
	}
	return false;
}

	inline boost::optional<unsigned> GetH245ParameterValue_booleanArray(const VS_H245GenericParameter& param, unsigned pid)
{
	if (!CheckH245ParameterIdentifier(param.parameterIdentifier, pid))
		return boost::none;

	auto value = dynamic_cast<VS_AsnInteger*>(param.parameterValue.choice);
	if (param.parameterValue.tag != VS_H245ParameterValue::e_booleanArray || !value)
		return boost::none;

	return value->value;
}

	inline void SetH245ParameterValue_booleanArray(VS_H245GenericParameter& param, unsigned pid, unsigned value, VS_Asn::ConstraintType c = VS_Asn::FixedConstraint, unsigned cl = 0, unsigned ch = 255)
{
	SetH245ParameterIdentifier(param.parameterIdentifier, pid);
	param.parameterValue.choice = new VS_AsnInteger(value, c, cl, ch);
	param.parameterValue.tag = VS_H245ParameterValue::e_booleanArray;
	param.parameterValue.filled = true;
	param.filled = true;
}

template <class InputIterator>
	inline boost::optional<unsigned> FindH245ParameterValue_booleanArray(InputIterator first, InputIterator last, unsigned pid)
{
	for (; first != last; ++first)
	{
		auto result = GetH245ParameterValue_booleanArray(*first, pid);
		if (result)
			return result;
	}
	return boost::none;
}

	inline boost::optional<unsigned> GetH245ParameterValue_unsignedMin(const VS_H245GenericParameter& param, unsigned pid)
{
	if (!CheckH245ParameterIdentifier(param.parameterIdentifier, pid))
		return boost::none;

	auto value = dynamic_cast<VS_AsnInteger*>(param.parameterValue.choice);
	if (param.parameterValue.tag != VS_H245ParameterValue::e_unsignedMin || !value)
		return boost::none;

	return value->value;
}

	inline void SetH245ParameterValue_unsignedMin(VS_H245GenericParameter& param, unsigned pid, const std::uint32_t value, VS_Asn::ConstraintType c = VS_Asn::FixedConstraint,
		std::int32_t cl = 0, std::uint32_t ch = 65535)
{
	SetH245ParameterIdentifier(param.parameterIdentifier, pid);
	param.parameterValue.choice = new VS_AsnInteger(value, c, cl, ch);
	param.parameterValue.tag = VS_H245ParameterValue::e_unsignedMin;
	param.parameterValue.filled = true;
	param.filled = true;
}

	template <class InputIterator>
	inline boost::optional<unsigned> FindH245ParameterValue_unsignedMin(InputIterator first, InputIterator last, unsigned pid)
	{
		for (; first != last; ++first)
		{
			auto result = GetH245ParameterValue_unsignedMin(*first, pid);
			if (result)
				return result;
		}
		return boost::none;
	}

	inline VS_H323VideoCodec GetVideoCapabilityCodec(const VS_H245VideoCapability* cap)
	{
		switch (cap->tag)
		{
		case VS_H245VideoCapability::e_h261VideoCapability:
			return e_videoH261;
		case VS_H245VideoCapability::e_h263VideoCapability:
		{
			auto h263_cap = dynamic_cast<const VS_H245H263VideoCapability*>(cap->choice);
			if (!h263_cap)
				return e_videoNone;
			if (h263_cap->h263Options.filled)
			{
				if (h263_cap->h263Options.h263Version3Options.filled)
					return e_videoH263plus2;
				return e_videoH263plus;
			}
			return e_videoH263;
		}
		break;
		case VS_H245VideoCapability::e_genericVideoCapability:
		{
			auto gvc = dynamic_cast<const VS_H245GenericCapability*>(cap->choice);
			if (!gvc)
				return e_videoNone;
			if (CheckH245CapabilityIdentifier(gvc->capabilityIdentifier, oid_h264_gc))
				return e_videoH264;
			return e_videoNone;
		}
		break;
		case VS_H245VideoCapability::e_extendedVideoCapability:
		{
			auto evc = dynamic_cast<const VS_H245ExtendedVideoCapability*>(cap->choice);
			if (!evc || evc->videoCapability.empty())
				return e_videoNone;
			return GetVideoCapabilityCodec(&evc->videoCapability[0]);
		}
		break;
		}
		return e_videoNone;
	}

	inline char GetVCPayloadType(VS_H323VideoCodec codec)
	{
		switch (codec)
		{
		case e_videoH261: return SDP_PT_H261;
		case e_videoH263: return SDP_PT_H263;
		case e_videoH263plus: return SDP_PT_DYNAMIC_H263plus;
		case e_videoH263plus2: return SDP_PT_DYNAMIC_H263plus2;
		case e_videoH264: return SDP_PT_DYNAMIC_H264;
		}
		return SDP_PT_INVALID;
	}


	inline VS_H323DataCodec GetDataCapabilityCodec(const VS_H245DataApplicationCapability* cap)
	{
		switch (cap->application.tag)
		{
		case VS_H245DataApplicationCapability_Application::e_h224:
			return VS_H323DataCodec::FECC;
		}
		return VS_H323DataCodec::dataNone;
	}

	inline char GetDCPayloadType(VS_H323DataCodec codec)
	{
		switch (codec)
		{
		case VS_H323DataCodec::FECC: return SDP_PT_DYNAMIC_H224;
		}
		return SDP_PT_INVALID;
	}

	inline void FillVCMediaPacketization(VS_H323VideoCodec codec, char pt_num, VS_H245H2250LogicalChannelParameters_MediaPacketization* pack)
	{
		if (codec != e_videoH263plus && codec != e_videoH263plus2 && codec != e_videoH264)
			return;

		VS_H245RTPPayloadType* pt = new VS_H245RTPPayloadType;
		pt->payloadType.value = pt_num;
		pt->payloadType.filled = true;

		switch (codec)
		{
		case e_videoH263plus:
		case e_videoH263plus2:
			pt->payloadDescriptor.choice = new VS_AsnInteger(2429, VS_AsnInteger::FixedConstraint, 1, 32768, true); // RFC2429 = H.263+
			pt->payloadDescriptor.tag = VS_H245RTPPayloadType_PayloadDescriptor::e_rfc_number;
			pt->payloadDescriptor.filled = true;
			break;
		case e_videoH264:
			pt->payloadDescriptor.choice = new VS_AsnObjectId(oid_h264_non_interveaved);
			pt->payloadDescriptor.tag = VS_H245RTPPayloadType_PayloadDescriptor::e_oid;
			pt->payloadDescriptor.filled = true;
		}
		pt->filled = true;

		pack->choice = pt;
		pack->tag = VS_H245H2250LogicalChannelParameters_MediaPacketization::e_rtpPayloadType;
		pack->filled = true;
	}

	inline void FillDCMediaPacketization(VS_H323DataCodec codec, char pt_num, VS_H245H2250LogicalChannelParameters_MediaPacketization* pack)
	{
		if (codec != VS_H323DataCodec::FECC) // the only supported for now
			return;

		VS_H245RTPPayloadType* pt = new VS_H245RTPPayloadType;
		pt->payloadType.value = pt_num;
		pt->payloadType.filled = true;

		switch (codec)
		{
		case VS_H323DataCodec::FECC:
			pt->payloadDescriptor.choice = new VS_AsnObjectId(oid_h224_gc);
			pt->payloadDescriptor.tag = VS_H245RTPPayloadType_PayloadDescriptor::e_oid;
			pt->payloadDescriptor.filled = true;
		}
		pt->filled = true;

		pack->choice = pt;
		pack->tag = VS_H245H2250LogicalChannelParameters_MediaPacketization::e_rtpPayloadType;
		pack->filled = true;
	}

	inline void FillACMediaPacketization(VS_H323AudioCodec codec, char pt_num, VS_H245H2250LogicalChannelParameters_MediaPacketization* pack)
	{
		if (codec != e_rcvG722124 && codec != e_rcvG722132 && codec != e_rcvSIREN14_24 && codec != e_rcvSIREN14_32 && codec != e_rcvSIREN14_48)
			return;

		VS_H245RTPPayloadType* pt = new VS_H245RTPPayloadType;
		pt->payloadType.value = pt_num;
		pt->payloadType.filled = true;

		VS_AsnObjectId* oid = new VS_AsnObjectId;
		VS_H323ParserInfo::FillACObjectId(codec, oid);
		pt->payloadDescriptor.choice = oid;
		pt->payloadDescriptor.tag = VS_H245RTPPayloadType_PayloadDescriptor::e_oid;
		pt->payloadDescriptor.filled = true;
		pt->filled = true;

		pack->choice = pt;
		pack->tag = VS_H245H2250LogicalChannelParameters_MediaPacketization::e_rtpPayloadType;
		pack->filled = true;
	}

	inline VS_H245MiscellaneousCommand* MakeMiscellaneousCommand(VS_Asn* choice, const std::uint32_t lcNumber, const unsigned tag, const unsigned* direction = nullptr) {
		VS_H245MiscellaneousCommand* misc = new VS_H245MiscellaneousCommand;
		misc->type.tag = tag;
		misc->type.choice = choice;
		misc->type.filled = true;
		misc->logicalChannelNumber.value = lcNumber;
		misc->logicalChannelNumber.filled = true;
		if (direction) {
			misc->direction.tag = *direction;
			misc->direction.choice = new VS_AsnNull;
			misc->direction.filled = true;
		}
		misc->filled = true;
		return misc;
	}

	inline VS_H245CommandMessage* MakeCommandMessage(VS_Asn* choice, const unsigned tag) {
		VS_H245CommandMessage* m = new VS_H245CommandMessage;
		m->tag = tag;
		m->choice = choice;
		m->filled = true;
		return m;
	}

	inline void AddSecureToAlternative(VS_H245CapabilityTableEntryNumber * capsNumbers, unsigned &capsTotalCount, unsigned &capsAlternativeCount) {
		assert(capsNumbers);

		capsNumbers[capsAlternativeCount].value = capsTotalCount + 1;
		capsNumbers[capsAlternativeCount].filled = true;
		++capsTotalCount;
		++capsAlternativeCount;
	}


	// get the first one with encryption
	template <class MediaMode>
	inline VS_H235SecurityCapability* GetFirstRecvH235Caps(std::vector<MediaMode> &modes) {
		auto _end = modes.end();
		auto it = std::find_if(modes.begin(), _end, [](const MediaMode& m) {return !m.sec_cap.h235_sessionKey.empty() && m.sec_cap.m != encryption_mode::no_encryption; });
		if (it != _end)
			return  &it->sec_cap;

		dstream4 << "Recv h235 capabilities weren't found.\n";
		return nullptr;
	}

	inline VS_H235SecurityCapability *GetCapability(const std::uint32_t lcNumber, const std::shared_ptr<VS_H323ParserInfo> &ctx) {
		VS_H245Data* h245params = ctx->GetH245Params();
		assert(h245params != nullptr);

		if (!h245params)
			return nullptr;

		if (lcNumber == h245params->m_audioNumberLCSender)		return &ctx->audio_channel.snd_mode_audio.sec_cap;
		if (lcNumber == h245params->m_videoNumberLCSender)		return &ctx->video_channel.snd_mode_video.sec_cap;
		if (lcNumber == h245params->m_slidesNumberLCSender)		return &ctx->slides_channel.snd_mode_video.sec_cap;
		if (lcNumber == h245params->m_audioNumberLCReciver)		return GetFirstRecvH235Caps(ctx->audio_channel.rcv_modes_audio);
		if (lcNumber == h245params->m_videoNumberLCReciver)		return GetFirstRecvH235Caps(ctx->video_channel.rcv_modes_video);
		if (lcNumber == h245params->m_slidesNumberLCReciver)	return GetFirstRecvH235Caps(ctx->slides_channel.rcv_modes_video);
		return nullptr;
	}

	inline void UpdateDynamicPayloadTypeAndH235Caps(const unsigned dynamicPt, const VS_H235SecurityCapability &secCap,
		const unsigned channelNumber, VS_GatewayMediaMode& outMode,
		std::shared_ptr<VS_H323ParserInfo>& ctx)
	{
		auto &out_sc = outMode.sec_cap;
		if (dynamicPt) {
			outMode.PayloadType = dynamicPt;
			out_sc = secCap;
			if (out_sc.syncFlag != dynamicPt) {
				dstream3 << "ERROR\tH235 syncFlag = '" << out_sc.syncFlag << "' and channel PayloadType = '" << outMode.PayloadType << "' must be equal.\n";
			}
		}
		else {
			std::tie(out_sc.h235_sessionKey, out_sc.m) = std::make_tuple(secCap.h235_sessionKey, secCap.m);
		}

		const bool master = ctx->GetH245Params()->m_msd_type == 1;
		if (master)
			ctx->channels_h235caps.emplace(channelNumber, out_sc);
	}
}

const net::port	VS_H323Parser::DEFAULT_H225CS_PORT = 1720;

class VS_H323CapabilityState;


VS_H323Parser::VS_H323Parser(boost::asio::io_service::strand &strand, const std::shared_ptr<net::LoggerInterface> &logger)
	: m_strand(strand)
	, m_queue_out(0)
	, m_queue_in_h225(0)
	, m_use_acl(true)
	, m_logger(logger)
{
	m_acl.LoadACL(VS_NetworkConnectionACL::CONN_H323);
}

VS_H323Parser::~VS_H323Parser(void)
{
	std::vector<std::string> to_fire;

	m_ctx_lock.lock();
	for(auto it=m_ctx.cbegin(); it!=m_ctx.cend(); ++it)
	{
		if (auto channel = it->second->GetH245Channel())
			channel->Close(true);
		to_fire.push_back(it->first);
	}
	m_ctx_connections.clear();
	m_ctx.clear();
	m_ctx_lock.unlock();

	m_ctx_garbage_lock.lock();
	for(auto it2=m_ctx_garbage.cbegin(); it2!=m_ctx_garbage.cend(); ++it2)
	{
		to_fire.push_back(*it2);
	}
	m_ctx_garbage.clear();
	m_ctx_garbage_lock.unlock();


	for(auto it3=to_fire.cbegin(); it3!=to_fire.cend(); ++it3)
	{
		m_fireDialogFinished(*it3);
	}

	delete m_queue_out;
	delete m_queue_in_h225;
}

int VS_H323Parser::SetRecvBuf(const void* buf, const std::size_t sz, const VS_ChannelID channelId,
	const net::address & remoteAddr, net::port remotePort, const net::address & localAddr, net::port localPort)
{
	if (IsACLUsed() && !m_acl.IsAllowed(m_remote_ip.c_str()))
	{
		dprint2("H323 ACL: address %s is not allowed. ACL enabled: %s, mode: %s.\n",
			m_remote_ip.c_str(), IsACLUsed() ? "true" : "false",
			(m_acl.GetMode() == VS_NetworkConnectionACL::ACL_NONE ? "none" :
			(m_acl.GetMode() == VS_NetworkConnectionACL::ACL_BLACKLIST ? "backlist" : "whitelist")));
		return 0;
	}

	dstream4 << "VS_H323Parser::SetRecvBuf: " << sz << " bytes from " << remoteAddr << ":" << remotePort;
	switch (channelId) {
		case e_H225:
			return RecvH225Buf(buf, sz, remoteAddr, remotePort);
		default:
			return 0;
	}
}

int VS_H323Parser::RecvH225Buf(const void *buf, std::size_t sz, const net::address &fromAddr, net::port fromPort) {

	unsigned char* in_buff = new unsigned char[sz];
	memcpy(in_buff, buf, sz);

	std::lock_guard<decltype(m_queue_in_h225_lock)> lock(m_queue_in_h225_lock);
	VS_TearMessageQueue* queue_in = GetInputQueue(e_H225);

	queue_in->PutTearMessage(in_buff, sz);

	while( queue_in->IsExistEntireMessage() )
	{
		std::unique_ptr<unsigned char[]> ent_mess;
		uint32_t ent_mess_sz = 0;

		if ( !queue_in->GetEntireMessage(ent_mess.get(), ent_mess_sz) )
		{
			if (ent_mess_sz)
			{
				ent_mess.reset(new unsigned char[ent_mess_sz]);
				if ( !queue_in->GetEntireMessage(ent_mess.get(), ent_mess_sz) )
					return 0;
			}else{
				// ошибка внутри очереди
				return 0;
			}
		}else{
			// Память не выделили, а данные нам дали
			// Не знаю, как сюда можно попасть :)
			return 0;
		}

		VS_PerBuffer in_per_buff{ ent_mess.get(), ent_mess_sz * 8 };

		VS_Q931 theQ931_In;
		if (!theQ931_In.DecodeMHeader(in_per_buff))
		{
			dprint4("RecvH225Buf: DecodeMHeader failed\n");
			return 0;
		}

		unsigned int crv = theQ931_In.callReference;
		//bool isVisicronToH323 = m_mgr->GetCallInfo()->GetH323CallInfoInterface()->GetCallDirection();

		switch (theQ931_In.messageType)
		{
		case VS_Q931::e_setupMsg:
			{
				/*if ( isVisicronToH323 || theQ931_In.fromDestination )
				{
					m_Log->TPrintf("[-] VS_CallSetup: Q.931 header error");
					if (in_per_buff) { delete in_per_buff; in_per_buff = 0; }
					return 0;
				}*/

				//ctx->SetCRV(crv);

				dprint3("VS_H323Parser::RecvH225Buf: setup\n");
				if ( !OnSetupArrived(in_per_buff, theQ931_In, fromAddr, fromPort) )
				{
					dprint4("RecvH225Buf: OnSetupArrived failed\n");
					return 0;
				}
			}
			break;

		    case VS_Q931::e_alertingMsg:
			case VS_Q931::e_callProceedingMsg:
			case VS_Q931::e_notifyMsg: break; // Income Alerting Message - Nothing to do...

		case VS_Q931::e_connectMsg:
			{
				dprint3("VS_H323Parser::RecvH225Buf: connect\n");
				if ( !OnConnectArrived(in_per_buff, theQ931_In) )
				{
					dprint4("RecvH225Buf: OnConnectArrived failed\n");
					return 0;
				}
			}
			break;

		case VS_Q931::e_releaseCompleteMsg:
			{
				dprint3("VS_H323Parser::RecvH225Buf: release complete\n");
				if ( !OnReleaseCompleteArrived(in_per_buff, fromAddr, fromPort) )
				{
					dprint4("RecvH225Buf: OnReleaseCompleteArrived failed\n");
					return 0;
				}
			}
			break;

		case VS_Q931::e_statusInquiryMsg:
			{
				dprint3("VS_H323Parser::RecvH225Buf: statusInquiry\n");
				if (!OnStatusInquiryArrived(in_per_buff))
				{
					dprint4("RecvH225Buf: OnStatusInquiry failed\n");
					return 1;
				}
			}
			break;
		case VS_Q931::e_facilityMsg:
			{
				dprint3("VS_H323Parser::RecvH225Buf: OnFacilityArrived()\n");
				if (!OnFacilityArrived(in_per_buff, theQ931_In, fromAddr, fromPort))
				{
					dprint4("RecvH225Buf: OnFacilityArrived() failed\n");
					return 1;
				}
			}
		break;
		default:
			{
				dprint4("RecvH225Buf: Unknown Mesasge Received\n");
				return 1;
			}
			break;
		}
		//TestMySubStatus();

	}
	return 1;
}

bool VS_H323Parser::OnReleaseCompleteArrived(const VS_PerBuffer &aInBuffer, const net::address &fromAddr, net::port fromPort)
{
	// TODO select context
	std::shared_ptr<VS_H323ParserInfo> ctx = GetDefaultParserContext(fromAddr, fromPort);
	if (!ctx)
		return false;

	auto confMethods = m_confMethods.lock();
	if (!ctx->IsInDialog() && ctx->GetCallDirection() == e_out && confMethods)
		confMethods->InviteReplay(ctx->GetDialogID(), e_call_busy, false, {}, {});

	//VS_IPPortAddress peer_addr;
	//ctx->GetPeerCsAddress(peer_addr);
	HangupCall(ctx->GetDialogID());
	CleanParserContext(ctx->GetDialogID(), SourceClean::PARSER);
	//CloseConnection(peer_addr);

	return true;
}

bool VS_H323Parser::OnStatusInquiryArrived(VS_PerBuffer &aInBuffer)
{
	unsigned char dn[83] = { 0 };
	unsigned char e164[50] = { 0 };
	if (!VS_Q931::GetUserUserIE(aInBuffer, dn, e164))
		return false;

	VS_CsH323UserInformation ui;

	if (!ui.Decode(aInBuffer))
		return false;

	VS_H225StatusInquiry_UUIE* si = ui.h323UuPdu.h323MessageBody;
	if (!si)				// not statusInquiry message
		return false;

	char call_identifier[CONFERENCEID_LENGTH * 2 + 1] = { 0 };

	// encoded call identifier
	if (si->callIdentifier.guid.filled)
	{
		VS_BitBuffer* bitBuffer = &(si->callIdentifier.guid.value);
		EncodeDialogID(static_cast<const unsigned char*>(bitBuffer->GetData()), call_identifier);
	}
	if (!*call_identifier)
		return false;

	std::shared_ptr<VS_H323GatekeeperStorage::Info> info =
		VS_H323GatekeeperStorage::Instance().GetTerminalInfo(call_identifier);
	std::shared_ptr<VS_H323ParserInfo> ctx;
	// If this is incoming call from registred h323-terminal, we try to create
	// the same dialog_id as used for registration.
	std::shared_ptr<VS_H225RASParserInfo> nfo;
	if (info)
		nfo = info->context.lock();
	if (nfo)
		ctx = GetParserContext(nfo->GetDialogID());
	else
		ctx = GetParserContext(call_identifier);

	if (!ctx)
		return false;

	MakeStatus(ctx);
	return true;
}

void VS_H323Parser::OnH245RawData(const void *buf, std::size_t sz, std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (!ctx)
		return;
	if (sz<1)
		return;

	unsigned char* in_buff = new unsigned char[sz];
	memcpy(in_buff, buf, sz);

	ctx->GetH245InputQueue().PutTearMessage(in_buff, sz);

	while (ctx->GetH245InputQueue().IsExistEntireMessage())
	{
		std::unique_ptr<unsigned char[]> ent_mess;
		uint32_t ent_mess_sz = 0;

		if (!ctx->GetH245InputQueue().GetEntireMessage(ent_mess.get(), ent_mess_sz))
		{
			if (ent_mess_sz)
			{
				ent_mess.reset(new unsigned char[ent_mess_sz]);
				if (!ent_mess || !ctx->GetH245InputQueue().GetEntireMessage(ent_mess.get(), ent_mess_sz))
					return;
			}else
			{
				// ошибка внутри очереди
				return;
			}
		}else
		{
			// Не знаю, как сюда можно попасть :)
			return;
		}
		VS_PerBuffer buffer(ent_mess.get(),ent_mess_sz*8);
		OnH245RawMessage( buffer, ctx );
	}
}

void VS_H323Parser::OnH245RawMessage(VS_PerBuffer & buffer, std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245MultimediaSystemControlMessage theMessage;
	unsigned char tmp;
	bool isDone;

	do{
		isDone = true;
		int res = 1;
		size_t error_bit = 0;
		size_t error_byte = 0;
		buffer.GetPositionIndex(error_byte,error_bit);
		if (!theMessage.Decode( buffer ))
		{
			///Decoding is bad.
			dprint4("SetH245Buf: decode failed\n");

			buffer.SetPositionIndex(error_byte,error_bit);
			std::size_t error_size_byte = 0;
			std::size_t error_size_bit = 0;
			buffer.GetPositionSize(error_size_byte,error_size_bit);
//			m_Logger->TPrintf("%s Index byte: %u Index bit: %u Size byte: %u Size bit: %u",m_LogIdent,
//				error_byte,error_bit,error_size_byte,error_size_bit);
//			m_Logger->TPrintf("%s Buffer Length : %u Buffer content follow:",m_LogIdent,
//				error_size_byte - error_byte + (error_size_bit!=8)?(1):(-1));
//			m_Logger->Printf("\n");
			tmp = 0;
			buffer.SetPositionIndex(error_byte,error_bit);
			while(buffer.GetBits(&tmp,8))
			{
//				m_Logger->Printf(" 0x%2.2x,",tmp);
			}
			return;
		} //< Decoding
		///Decoding is ok.
		if (buffer.BitsLeft() >= 8)		// more than 1 bytes exists
		{
			isDone = false;
		}
		res = OnH245Message(&theMessage, ctx);
	}while(!isDone);
	return;
}

bool VS_H323Parser::OnH245Message(const VS_H245MultimediaSystemControlMessage* msg, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	switch (msg->tag)
	{
	case VS_H245MultimediaSystemControlMessage::e_command:
		dprint3("VS_H323Parser::OnH245Message: command\n");
		return OnH245Command(static_cast<VS_H245CommandMessage*>(msg->choice), ctx);
	case VS_H245MultimediaSystemControlMessage::e_indication:
		dprint3("VS_H323Parser::OnH245Message: indication\n");
		return OnH245Indication(static_cast<VS_H245IndicationMessage*>(msg->choice), ctx);
	case VS_H245MultimediaSystemControlMessage::e_request:
		dprint3("VS_H323Parser::OnH245Message: request\n");
		return OnH245Request(static_cast<VS_H245RequestMessage*>(msg->choice), ctx);
	case VS_H245MultimediaSystemControlMessage::e_response:
		dprint3("VS_H323Parser::OnH245Message: response\n");
		return OnH245Response(static_cast<VS_H245ResponseMessage*>(msg->choice), ctx);
	}
	return false;
}

bool VS_H323Parser::OnH245Request(const VS_H245RequestMessage* req, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	switch (req->tag)
	{
	case VS_H245RequestMessage::e_masterSlaveDetermination:
		RecvMSD(dynamic_cast<VS_H245MasterSlaveDetermination*> (req->choice), ctx);
		break;
	case VS_H245RequestMessage::e_terminalCapabilitySet:
		{
			VS_H245TerminalCapabilitySet * tcs = static_cast<VS_H245TerminalCapabilitySet*>( req->choice);
			if (!RecvTCS(tcs, ctx))
				return false;

			if (!SendTCSA(tcs->sequenceNumber.value, ctx))
				return false;

			if (!SendTCS(ctx))
				return false;

// 			if (!CreateOLCs(ctx))
// 			{
// 				TerminateSession(ctx);
// 				return true;
// 			}
		}
		break;
	case VS_H245RequestMessage::e_openLogicalChannel:
		{
			VS_H245OpenLogicalChannel   *olc = static_cast<VS_H245OpenLogicalChannel*>(req->choice);
			if (!RecvOLC( olc, ctx ))
				return false;
			UpdateCallState(ctx);
		}
		break;
	case VS_H245RequestMessage::e_requestMode:
		break;
	case VS_H245RequestMessage::e_closeLogicalChannel:
		{

			VS_H245CloseLogicalChannel* clc = static_cast<VS_H245CloseLogicalChannel*>(req->choice);

			if ( clc->forwardLogicalChannelNumber.filled )
			{
				MakeCloseLogicalChannelAck(clc->forwardLogicalChannelNumber.value, ctx);

				ctx->CloseH245LogicalChannel( clc->forwardLogicalChannelNumber.value );
			}

		}
		break;
	case VS_H245RequestMessage::e_roundTripDelayRequest:
		{
			VS_H245RoundTripDelayResponse* rtd_resp = new VS_H245RoundTripDelayResponse;
			VS_H245RoundTripDelayRequest* rtd_req = static_cast<VS_H245RoundTripDelayRequest*> (req->choice);

			rtd_resp->sequenceNumber = rtd_req->sequenceNumber;
			rtd_resp->filled = true;

			VS_H245ResponseMessage* resp = new VS_H245ResponseMessage;
			resp->filled = true;
			resp->choice = rtd_resp;
			resp->tag = resp->e_roundTripDelayResponse;

			VS_H245MultimediaSystemControlMessage mscm;
			mscm.choice = resp;
			mscm.filled = true;
			mscm.tag = VS_H245MultimediaSystemControlMessage::e_response;

			PutH245Message(mscm, ctx);
		}
		break;
	case VS_H245RequestMessage::e_genericRequest:
		VS_H245GenericMessage* gm = static_cast<VS_H245GenericMessage*>(req->choice);
		if (gm && CheckH245CapabilityIdentifier(gm->messageIdentifier, oid_h239_gm))
			return OnH239Message(gm, ctx);
	}
	//	TestForChangeState();
	return true;
}

bool VS_H323Parser::OnH245Response(const VS_H245ResponseMessage* resp, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{

	VS_H245Data* h245Params = ctx->GetH245Params();
	switch(resp->tag)
	{
	case VS_H245ResponseMessage::e_terminalCapabilitySetAck:
		{
			h245Params->H245Flags[VS_H323ParserInfo::e_tcs] |= VS_H245Data::h245_rsp_recv;//0x10;
		}
		break;
	case VS_H245ResponseMessage::e_terminalCapabilitySetReject:
		{
			TerminateSession(ctx);
			return false;
		}break;
	case VS_H245ResponseMessage::e_masterSlaveDeterminationAck:
		RecvMSDA(static_cast<VS_H245MasterSlaveDeterminationAck*>(resp->choice), ctx);
		break;
	case VS_H245ResponseMessage::e_masterSlaveDeterminationReject:
		{
			MSDState state = ctx->MsdState();
			if (state == MSDOutgoingAwaitingResponse)
			{
				ctx->StopMSDTimer();
				if (ctx->GenNewMSDNums())
				{
					ctx->StartMSDTimer();
					return SendMSD(ctx);
				} else
				{
					dprint3("VS_H323Parser::ResponseH245: MSDError (F) max number of retries exceeded\n");
					ctx->SetMsdState(MSDIdle);
					TerminateSession(ctx);
				}
			} else if (state == MSDIncomingAwaitingResponse)
			{
				dprint3("VS_H323Parser::ResponseH245: MSDError (D) inappropriate MSD message (MSDAck expected)\n");
				ctx->SetMsdState(MSDIdle);
				ctx->StopMSDTimer();
				TerminateSession(ctx);
			}
		}break;
	case VS_H245ResponseMessage::e_openLogicalChannelAck:
		{
			VS_H245OpenLogicalChannelAck * ack =
				dynamic_cast<VS_H245OpenLogicalChannelAck*>(resp->choice);
			if (!RecvOLCA( ack, ctx ))
				return false;
			UpdateCallState(ctx);
			UpdateSlideshowState(ctx, ctx->GetSlideshowState());
		}
		break;
	case VS_H245ResponseMessage::e_openLogicalChannelReject:
		{
			VS_H245OpenLogicalChannelReject* olcRj = static_cast<VS_H245OpenLogicalChannelReject*>(resp->choice);

			if ( olcRj->forwardLogicalChannelNumber.filled )
			{
				//m_mgr->GetCallInfo()->GetH323CallInfoInterface()->CloseH245LogicalChannel( olcRj->forwardLogicalChannelNumber.value );

				// SJPhone fix
				{
					if (olcRj->forwardLogicalChannelNumber.value == h245Params->m_audioNumberLCSender)
					{
						h245Params->H245Flags[VS_H323ParserInfo::e_olc_audio] |= VS_H245Data::h245_rsp_recv;
						h245Params->H245Flags[VS_H323ParserInfo::e_olc_audio] |= VS_H245Data::h245_rsp_rejected;
					}
					else if (olcRj->forwardLogicalChannelNumber.value == h245Params->m_videoNumberLCSender)
					{
						h245Params->H245Flags[VS_H323ParserInfo::e_olc_video] |= VS_H245Data::h245_rsp_recv;
						h245Params->H245Flags[VS_H323ParserInfo::e_olc_video] |= VS_H245Data::h245_rsp_rejected;
					}
					else if (olcRj->forwardLogicalChannelNumber.value == h245Params->m_slidesNumberLCSender)
					{
						h245Params->H245Flags[VS_H323ParserInfo::e_olc_slides] |= VS_H245Data::h245_rsp_recv;
						h245Params->H245Flags[VS_H323ParserInfo::e_olc_slides] |= VS_H245Data::h245_rsp_rejected;
					}
				}
			}

			//			Terminate();
			return false;
		}
		break;
	case VS_H245ResponseMessage::e_genericResponse:
		VS_H245GenericMessage* gm = static_cast<VS_H245GenericMessage*>(resp->choice);
		assert(gm);
		if (CheckH245CapabilityIdentifier(gm->messageIdentifier, oid_h239_gm))
			return OnH239Message(gm, ctx);
	}
	//ShowFlags();

	return true;
}

bool VS_H323Parser::OnH245Command(const VS_H245CommandMessage* cmd, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (!cmd)
		return false;

	switch (cmd->tag)
	{
	case VS_H245CommandMessage::e_flowControlCommand:
		OnH245Command_FlowControlArrived(static_cast<VS_H245FlowControlCommand*> (cmd->choice), ctx);
		break;
	case VS_H245CommandMessage::e_miscellaneousCommand:
		OnH245Command_MiscellaneousArrived(static_cast<VS_H245MiscellaneousCommand*>(cmd->choice), ctx);
		break;
	case VS_H245CommandMessage::e_endSessionCommand:
		if (ctx->GetHangupMode() == e_my_hangup)
		{
			MakeReleaseComplete(ctx);
		}
		else
		{
			MakeEndSessionCommand(ctx);
		}
		break;
	case VS_H245CommandMessage::e_genericCommand:
		VS_H245GenericMessage* gm = static_cast<VS_H245GenericMessage*>(cmd->choice);
		assert(gm);
		if (CheckH245CapabilityIdentifier(gm->messageIdentifier, oid_h239_gm))
			return OnH239Message(gm, ctx);
	}
	return true;
}

bool VS_H323Parser::OnH245Indication(const VS_H245IndicationMessage* ind, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (!ind)
		return false;

	switch (ind->tag)
	{
	case VS_H245IndicationMessage::e_genericIndication:
	{
		VS_H245GenericMessage* gm = static_cast<VS_H245GenericMessage*>(ind->choice);
		assert(gm);
		if (CheckH245CapabilityIdentifier(gm->messageIdentifier, oid_h239_gm))
			return OnH239Message(gm, ctx);
	}
		break;
	case VS_H245IndicationMessage::e_masterSlaveDeterminationRelease:
	{
		MSDState state = ctx->MsdState();
		if (state == MSDOutgoingAwaitingResponse ||
			state == MSDIncomingAwaitingResponse)
		{
			dprint3("VS_H323Parser::MessageH245Processing: MSDError (B) remote sees no response from local MSDSE\n");
			ctx->SetMsdState(MSDIdle);
			ctx->StopMSDTimer();
			TerminateSession(ctx);
		}
	}
		break;
	default:
		break;
	}
	return true;
}

void VS_H323Parser::OnH245Command_FlowControlArrived(const VS_H245FlowControlCommand* cmd,
													 const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (!cmd)
		return;

	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;

	const unsigned int scope = cmd->scope.tag;
	// 0 - e_logicalChannelNumber
	// 1 - e_resourceID
	// 2 - e_wholeMultiplex

	TemplInteger<0,16777215>* tmp = static_cast<TemplInteger<0,16777215>*>(cmd->restriction.choice);
	unsigned int value = tmp->value;

	VS_Container cnt;

	switch (cmd->restriction.tag)
	{
	case VS_H245FlowControlCommand_Restriction::e_maximumBitRate:
		{
			//	scope - тип
			//	value - значение
			cnt.AddValueI32(H323_APP_DATA_TYPE, e_restrictBitRate);
			cnt.AddValueI32(H323_APP_DATA_SOME_LONG_1, value * 100);	// See ASN.1: H.245: "-- units 100 bit/s"
			cnt.AddValueI32(H323_APP_DATA_SOME_LONG_2, scope);

			confMethods->BitrateRestriction(ctx->GetDialogID(), e_restrictBitRate, value * 100, scope);
		}
		break;

	case VS_H245FlowControlCommand_Restriction::e_noRestriction:
		{
			cnt.AddValueI32(H323_APP_DATA_TYPE, e_noRestrictions);

			confMethods->BitrateRestriction(ctx->GetDialogID(), e_noRestrictions, 0, 0);
		}
		break;

	default:
		return ;
		break;
	}

	SerializeAndSendContainerToKostya(cnt);
}

void VS_H323Parser::OnH245Command_MiscellaneousArrived(const VS_H245MiscellaneousCommand* cmd,
													   const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (!cmd)
		return ;

	VS_Container cnt;
	switch (cmd->type.tag)
	{
	case VS_H245MiscellaneousCommand_Type::e_videoFastUpdateGOB:
		{
			VS_H245MiscellaneousCommand_Type_VideoFastUpdateGOB* gob =
				static_cast<VS_H245MiscellaneousCommand_Type_VideoFastUpdateGOB *>( cmd->type.choice);

			unsigned int first = gob->firstGOB.value;
			unsigned int number = gob->numberOfGOBs.value;

			cnt.AddValueI32(H323_APP_DATA_TYPE, e_fastUpdateGob);
			cnt.AddValueI32(H323_APP_DATA_SOME_LONG_1, first);
			cnt.AddValueI32(H323_APP_DATA_SOME_LONG_2, number);
		}
		break;

	case VS_H245MiscellaneousCommand_Type::e_videoFreezePicture:
		{
			cnt.AddValueI32(H323_APP_DATA_TYPE, e_freezePicture);
		}
		break;

	case VS_H245MiscellaneousCommand_Type::e_videoSendSyncEveryGOB:
		{
			cnt.AddValueI32(H323_APP_DATA_TYPE, e_sendSyncEveryGob);
		}
		break;

	case VS_H245MiscellaneousCommand_Type::e_videoFastUpdatePicture:
		{
			auto confMethods = m_confMethods.lock();
			if (!confMethods)
				break;
			cnt.AddValueI32(H323_APP_DATA_TYPE, e_fastUpdatePicture);
			confMethods->FastUpdatePicture(ctx->GetDialogID());
		}
		break;
	case VS_H245MiscellaneousCommand_Type::e_encryptionUpdate:
		dstream3 << "Error\tEncryptionUpdate command is not supported!\n";
		assert(false);
		break;
	case VS_H245MiscellaneousCommand_Type::e_encryptionUpdateRequest:
		if (!OnEncryptionUpdateReq(cmd, ctx))
			dstream3 << "Error\tOnEncryptionUpdateCommand is failed!\n";
		break;
	case VS_H245MiscellaneousCommand_Type::e_encryptionUpdateCommand:
		if(!OnEncryptionUpdateCommand(cmd, ctx))
			dstream3 << "Error\tOnEncryptionUpdateCommand is failed!\n";
		break;
	case VS_H245MiscellaneousCommand_Type::e_encryptionUpdateAck:
		if (!OnEncryptionUpdateAck(cmd, ctx))
			dstream3 << "Error\tOnEncryptionUpdateAck is failed!\n";
		break;
	default:
		return ;
		break;
	}

	SerializeAndSendContainerToKostya(cnt);
}

bool VS_H323Parser::OnEncryptionUpdateCommand(const VS_H245MiscellaneousCommand* cmd, const std::shared_ptr<VS_H323ParserInfo> &ctx) const {
	if (!cmd || !cmd->filled || !cmd->logicalChannelNumber.filled) return false;
	if (ctx->IsMaster()) return false;	// we must be slave

	const auto upd_cmd = static_cast<VS_H245MiscellaneousCommand_Type_EncryptionUpdateCommand *>(cmd->type.choice);

	if (!upd_cmd) {
		dstream3 << "VS_H323Parser::OnEncryptionUpdateCommand: Error, no EncryptionUpdateCommand provided!\n";
		return false;
	}

	auto lc_number = cmd->logicalChannelNumber.value;
	VS_H235SecurityCapability *pCapToUpdate = GetCapability(lc_number, ctx);

	if (!pCapToUpdate) {
		dstream3 << "VS_H323Parser::OnEncryptionUpdateCommand: Can't find VS_H235SecurityCapability to update channel no='"<< lc_number << "'.\n";
		return false;
	}

	if (!ctx->h235_auth.ReadEncryptionSync(upd_cmd->encryptionSync, *pCapToUpdate)) return false;

	SendSetMediaChannels(ctx);

	// if direction of channel is e_masterToSlave we must send EncryptionUpdateAck command
	return cmd->direction.tag == VS_H245EncryptionUpdateDirection::e_slaveToMaster || SendEncryptionUpdateAck(ctx, lc_number, pCapToUpdate->syncFlag);
}

bool VS_H323Parser::OnEncryptionUpdateAck(const VS_H245MiscellaneousCommand* cmd, const std::shared_ptr<VS_H323ParserInfo> &ctx) const{
	if (!cmd || !cmd->filled || !cmd->logicalChannelNumber.filled) return false;
	if (!ctx->IsMaster()) return false;	// we must be master

	const auto upd_ack = static_cast<VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck *>(cmd->type.choice);
	if (!upd_ack) {
		dstream3 << "VS_H323Parser::OnEncryptionUpdateAck: Error, no EncryptionUpdateAck provided!\n";
		return false;
	}

	auto lc_number = cmd->logicalChannelNumber.value;
	VS_H235SecurityCapability *pCapToUpdate = GetCapability(lc_number, ctx);
	if (!pCapToUpdate) {
		dstream3 << "VS_H323Parser::OnEncryptionUpdateAck: Can't find VS_H235SecurityCapability to verify and update channel no='" << lc_number << "'.\n";
		return false;
	}

	if (!upd_ack->synchFlag.filled) {
		dstream3 << "VS_H323Parser::OnEncryptionUpdateAck: Error, synchFlag is not present!\n";
		return false;
	}

	if (upd_ack->synchFlag.value != pCapToUpdate->syncFlag) {
		dstream3 << "VS_H323Parser::OnEncryptionUpdateAck: synchFlag='" << upd_ack->synchFlag.value << "' in EncryptionUpdateAck message is not equal to our synchFlag='" << pCapToUpdate->syncFlag << "'!\n";
		return false;
	}

	SendSetMediaChannels(ctx);	// make update wich we've prepared in SendEncryptionUpdateCommand
	return true;
}

bool VS_H323Parser::OnEncryptionUpdateReq(const VS_H245MiscellaneousCommand* cmd, const std::shared_ptr<VS_H323ParserInfo> &ctx) const {
	if (!cmd || !cmd->filled || !cmd->logicalChannelNumber.filled) return false;
	if (!ctx->IsMaster()) return false;	// we must be master

	const auto upd_req = static_cast<VS_H245EncryptionUpdateRequest *>(cmd->type.choice);
	if (!upd_req) {
		dstream3 << "VS_H323Parser::OnEncryptionUpdateReq: Error, no EncryptionUpdateRequest provided!\n";
		return false;
	}

	if(!upd_req->keyProtectionMethod.sharedSecret.filled || !upd_req->keyProtectionMethod.sharedSecret.value)
		dstream3 << "Warning, OnEncryptionUpdateReq sharedSecret is not set!\n";

	const auto lc_number = cmd->logicalChannelNumber.value;
	unsigned new_synch_flag;
	if (ctx->IsOurGenericChannel(lc_number)) {
		srand(static_cast<unsigned int>(time(nullptr)));
		new_synch_flag = static_cast<unsigned>(rand() % 128);

		if(cmd->direction.tag != VS_H245EncryptionUpdateDirection::e_masterToSlave)
			dstream3 << "Warning, OnEncryptionUpdateReq direction tag is wrong!\n";
	}
	else {
		if (cmd->direction.tag != VS_H245EncryptionUpdateDirection::e_slaveToMaster)
			dstream3 << "Warning, OnEncryptionUpdateReq direction tag is wrong!\n";

		if (!upd_req->synchFlag.filled) {
			dstream3 << "VS_H323Parser::OnEncryptionUpdateReq: Error, no synchFlag provided!\n";
			return false;
		}
		new_synch_flag = upd_req->synchFlag.value;
	}

	return SendEncryptionUpdateCommand(ctx, lc_number, new_synch_flag);
}

bool VS_H323Parser::SendEncryptionUpdateAck(const std::shared_ptr<VS_H323ParserInfo> &ctx, const std::uint32_t lcNumber, const unsigned newSyncFlag) const{
	VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck* ack = new VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck;
	ack->synchFlag.value = newSyncFlag;
	ack->synchFlag.filled = true;
	ack->filled = true;

	VS_H245MiscellaneousCommand* misc = MakeMiscellaneousCommand(ack, lcNumber, VS_H245MiscellaneousCommand_Type::e_encryptionUpdateAck);
	VS_H245CommandMessage* m = MakeCommandMessage(misc, VS_H245CommandMessage::e_miscellaneousCommand);

	VS_H245MultimediaSystemControlMessage mscm;
	mscm.tag = VS_H245MultimediaSystemControlMessage::e_command;
	mscm.choice = m;
	mscm.filled = true;

	return PutH245Message(mscm, ctx);
}

bool VS_H323Parser::SendEncryptionUpdateRequest(const std::shared_ptr<VS_H323ParserInfo> &ctx, const std::uint32_t lcNumber) const{
	auto req = new VS_H245EncryptionUpdateRequest;

	unsigned int direction = VS_H245EncryptionUpdateDirection::e_masterToSlave;

	if (ctx->IsOurGenericChannel(lcNumber)){
		srand(static_cast<unsigned int>(time(nullptr)));
		req->synchFlag.value = static_cast<unsigned>(rand() % 128);
		req->synchFlag.filled = true;
		direction = VS_H245EncryptionUpdateDirection::e_slaveToMaster;
	}

	req->keyProtectionMethod.sharedSecret.value = true;
	req->keyProtectionMethod.sharedSecret.filled = true;
	req->keyProtectionMethod.certProtectedKey.value = false;
	req->keyProtectionMethod.certProtectedKey.filled = true;
	req->keyProtectionMethod.secureChannel.value = false;
	req->keyProtectionMethod.secureChannel.filled = true;
	req->keyProtectionMethod.filled = true;
	req->filled = true;

	VS_H245MiscellaneousCommand* misc = MakeMiscellaneousCommand(req, lcNumber, VS_H245MiscellaneousCommand_Type::e_encryptionUpdateRequest,&direction);
	VS_H245CommandMessage* m = MakeCommandMessage(misc, VS_H245CommandMessage::e_miscellaneousCommand);

	VS_H245MultimediaSystemControlMessage mscm;
	mscm.tag = VS_H245MultimediaSystemControlMessage::e_command;
	mscm.choice = m;
	mscm.filled = true;

	return PutH245Message(mscm, ctx);
}

bool VS_H323Parser::SendEncryptionUpdateCommand(const std::shared_ptr<VS_H323ParserInfo> &ctx, const std::uint32_t lcNumber, const unsigned syncFlag) const {
	VS_H235SecurityCapability *pCapToUpdate = GetCapability(lcNumber, ctx);
	if (!pCapToUpdate) {
		dstream3 << "VS_H323Parser::SendEncryptionUpdateCommand: Can't find VS_H235SecurityCapability to update channel no='" << lcNumber << "'.\n";
		return false;
	}
	pCapToUpdate->h235_sessionKey.clear();

	unsigned int direction = VS_H245EncryptionUpdateDirection::e_slaveToMaster;
	if (ctx->IsOurGenericChannel(lcNumber)){
		direction = VS_H245EncryptionUpdateDirection::e_masterToSlave;
		if(pCapToUpdate->syncFlag != syncFlag) pCapToUpdate->syncFlag = syncFlag;
		else ++pCapToUpdate->syncFlag;
	}

	auto upd_cmd = new VS_H245MiscellaneousCommand_Type_EncryptionUpdateCommand;
	ctx->h235_auth.BuildEncryptionSync(upd_cmd->encryptionSync, *pCapToUpdate);
	upd_cmd->filled = true;

	VS_H245MiscellaneousCommand* misc = MakeMiscellaneousCommand(upd_cmd, lcNumber, VS_H245MiscellaneousCommand_Type::e_encryptionUpdateCommand, &direction);
	VS_H245CommandMessage* m = MakeCommandMessage(misc, VS_H245CommandMessage::e_miscellaneousCommand);

	VS_H245MultimediaSystemControlMessage mscm;
	mscm.tag = VS_H245MultimediaSystemControlMessage::e_command;
	mscm.choice = m;
	mscm.filled = true;

	if (PutH245Message(mscm, ctx)) {
		if(!ctx->IsOurGenericChannel(lcNumber)) SendSetMediaChannels(ctx);
		// else we must wait until slave's EncryptionUpdateAck
		return true;
	}

	return false;
}

bool VS_H323Parser::OnH239Message(const VS_H245GenericMessage* msg, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	// As there is only one presentation token in the conferrence we are not checking channelID
	// and assuming it refers to slides logical channel.
	// When mandatory parameter is missing or there both 'acknowledge' and 'reject' present we are choosing
	// to ingone the message. Standard doesn't say anything about what to do in that case.

	switch (msg->subMessageIdentifier.value)
	{
	case c_h239_smid_flowControlReleaseRequest:
		dprint3("VS_H323Parser::OnH239Message: flowControlReleaseRequest\n");
		// Nothing to do, this message is only useful to MCUs.
		break;
	case c_h239_smid_flowControlReleaseResponse:
		dprint3("VS_H323Parser::OnH239Message: flowControlReleaseResponse\n");
		// Nothing to do, this message is only useful to MCUs.
		break;
	case c_h239_smid_presentationTokenRequest:
		dprint3("VS_H323Parser::OnH239Message: presentationTokenRequest\n");
	{
		auto terminal_label = FindH245ParameterValue_unsignedMin(msg->messageContent.begin(), msg->messageContent.end(), c_h239_pid_terminalLabel);
		auto lc_number = FindH245ParameterValue_unsignedMin(msg->messageContent.begin(), msg->messageContent.end(), c_h239_pid_channelId);
		auto symmetry_breaking = FindH245ParameterValue_unsignedMin(msg->messageContent.begin(), msg->messageContent.end(), c_h239_pid_symmetryBreaking);
		if (!terminal_label || !lc_number || !symmetry_breaking)
			break;

		auto token = ctx->GetH239PresentationToken();
		if (token->owned || token->symmetry_breaking_of_request < *symmetry_breaking)
			SendH239PresentationTokenResponse(true, *terminal_label, *lc_number, ctx);
		else if (token->symmetry_breaking_of_request > *symmetry_breaking)
			SendH239PresentationTokenResponse(false, *terminal_label, *lc_number, ctx);
		else
			SendH239PresentationTokenRequest(ctx);
	}
		break;
	case c_h239_smid_presentationTokenResponse:
		dprint3("VS_H323Parser::OnH239Message: presentationTokenResponse\n");
	{
		auto terminal_label = FindH245ParameterValue_unsignedMin(msg->messageContent.begin(), msg->messageContent.end(), c_h239_pid_terminalLabel);
		auto lc_number = FindH245ParameterValue_unsignedMin(msg->messageContent.begin(), msg->messageContent.end(), c_h239_pid_channelId);
		auto acknowledge = FindH245ParameterValue_logical(msg->messageContent.begin(), msg->messageContent.end(), c_h239_pid_acknowledge);
		auto reject = FindH245ParameterValue_logical(msg->messageContent.begin(), msg->messageContent.end(), c_h239_pid_reject);
		if (!terminal_label || !lc_number || (acknowledge == reject))
			break;

		auto token = ctx->GetH239PresentationToken();
		if (acknowledge)
		{
			token->owned = true;
			if (token->symmetry_breaking_of_request == 0) // Request wasn't in progress, so we didn't wanted the token
				SendH239PresentationTokenRelease(ctx);
			else
				SendH245LogicalChannelActiveIndication(ctx->GetH245Params()->m_slidesNumberLCSender, true, ctx);
		}
		else
		{
			if (token->symmetry_breaking_of_request != 0) // Request was in progress, so we should send new request
				SendH239PresentationTokenRequest(ctx);
		}
	}
		break;
	case c_h239_smid_presentationTokenRelease:
		dprint3("VS_H323Parser::OnH239Message: presentationTokenRelease\n");
	{
		auto terminal_label = FindH245ParameterValue_unsignedMin(msg->messageContent.begin(), msg->messageContent.end(), c_h239_pid_terminalLabel);
		auto lc_number = FindH245ParameterValue_unsignedMin(msg->messageContent.begin(), msg->messageContent.end(), c_h239_pid_channelId);
		if (!terminal_label || !lc_number)
			break;

		// TODO: inform transcoder to stop receiving slides (and switch client video to main channel).
	}
		break;
	case c_h239_smid_presentationTokenIndicateOwner:
		dprint3("VS_H323Parser::OnH239Message: presentationTokenIndicateOwner\n");
		// Nothing to do, this message is only useful to MCUs.
		break;
	default:
		dprint3("VS_H323Parser::OnH239Message: unknown subMessageIdentifier=%u\n", msg->subMessageIdentifier.value);
		break;
	}
	return true;
}

void VS_H323Parser::SendH239PresentationTokenRequest(const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	auto token = ctx->GetH239PresentationToken();

	if (token->owned || token->symmetry_breaking_of_request != 0) // We already own the token or there is ongoing request, so no need to request it again
		return;
	token->symmetry_breaking_of_request = static_cast<unsigned>(rand()) % 127 + 1; // TODO: migrate to std::randint when it's ready

	VS_H245MultimediaSystemControlMessage mscm;
	VS_H245RequestMessage* m = new VS_H245RequestMessage;
	VS_H245GenericMessage* gm = new VS_H245GenericMessage;

	SetH245CapabilityIdentifier(gm->messageIdentifier, oid_h239_gm);
	gm->subMessageIdentifier.value = c_h239_smid_presentationTokenRequest;
	gm->subMessageIdentifier.filled = true;

	gm->messageContent.reset(new VS_H245GenericParameter[3], 3);
	SetH245ParameterValue_unsignedMin(gm->messageContent[0], c_h239_pid_terminalLabel, 0);
	SetH245ParameterValue_unsignedMin(gm->messageContent[1], c_h239_pid_channelId, ctx->GetH245Params()->m_slidesNumberLCSender);
	SetH245ParameterValue_unsignedMin(gm->messageContent[2], c_h239_pid_symmetryBreaking, token->symmetry_breaking_of_request);

	gm->filled = true;

	m->choice = gm;
	m->tag = VS_H245RequestMessage::e_genericRequest;
	m->filled = true;

	mscm.choice = m;
	mscm.tag = VS_H245MultimediaSystemControlMessage::e_request;
	mscm.filled = true;

	PutH245Message(mscm, ctx);
}

void VS_H323Parser::SendH239PresentationTokenResponse(bool acknowledge, unsigned terminalLabel, const std::uint32_t lcNumber, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	auto token = ctx->GetH239PresentationToken();

	if (acknowledge) // By acknowledging foreign request we are losing ownership of the token
	{
		token->owned = false;
		token->symmetry_breaking_of_request = 0;
	}

	VS_H245MultimediaSystemControlMessage mscm;
	VS_H245ResponseMessage* m = new VS_H245ResponseMessage;
	VS_H245GenericMessage* gm = new VS_H245GenericMessage;

	SetH245CapabilityIdentifier(gm->messageIdentifier, oid_h239_gm);
	gm->subMessageIdentifier.value = c_h239_smid_presentationTokenResponse;
	gm->subMessageIdentifier.filled = true;

	gm->messageContent.reset(new VS_H245GenericParameter[3], 3);
	SetH245ParameterValue_logical(gm->messageContent[0], acknowledge ? c_h239_pid_acknowledge : c_h239_pid_reject);
	SetH245ParameterValue_unsignedMin(gm->messageContent[1], c_h239_pid_terminalLabel, terminalLabel);
	SetH245ParameterValue_unsignedMin(gm->messageContent[2], c_h239_pid_channelId, lcNumber);

	gm->filled = true;

	m->choice = gm;
	m->tag = VS_H245ResponseMessage::e_genericResponse;
	m->filled = true;

	mscm.choice = m;
	mscm.tag = VS_H245MultimediaSystemControlMessage::e_response;
	mscm.filled = true;

	PutH245Message(mscm, ctx);
}

void VS_H323Parser::SendH239PresentationTokenRelease(const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	auto token = ctx->GetH239PresentationToken();

	if (!token->owned) // We shall not send this message if we don't own the token
		return;
	token->owned = false;
	token->symmetry_breaking_of_request = 0;

	VS_H245MultimediaSystemControlMessage mscm;
	VS_H245CommandMessage* m = new VS_H245CommandMessage;
	VS_H245GenericMessage* gm = new VS_H245GenericMessage;

	SetH245CapabilityIdentifier(gm->messageIdentifier, oid_h239_gm);
	gm->subMessageIdentifier.value = c_h239_smid_presentationTokenRelease;
	gm->subMessageIdentifier.filled = true;

	gm->messageContent.reset(new VS_H245GenericParameter[2], 2);
	SetH245ParameterValue_unsignedMin(gm->messageContent[0], c_h239_pid_terminalLabel, 0);
	SetH245ParameterValue_unsignedMin(gm->messageContent[1], c_h239_pid_channelId, ctx->GetH245Params()->m_slidesNumberLCSender);
	gm->messageContent.filled = true;

	gm->filled = true;

	m->choice = gm;
	m->tag = VS_H245CommandMessage::e_genericCommand;
	m->filled = true;

	mscm.choice = m;
	mscm.tag = VS_H245MultimediaSystemControlMessage::e_command;
	mscm.filled = true;

	PutH245Message(mscm, ctx);
}

void VS_H323Parser::SendH239PresentationTokenIndicateOwner(const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	auto token = ctx->GetH239PresentationToken();

	if (!token->owned) // We shall not send this message if we don't own the token
		return;
	token->last_indication_time = std::chrono::steady_clock::now();

	VS_H245MultimediaSystemControlMessage mscm;
	VS_H245IndicationMessage* m = new VS_H245IndicationMessage;
	VS_H245GenericMessage* gm = new VS_H245GenericMessage;

	SetH245CapabilityIdentifier(gm->messageIdentifier, oid_h239_gm);
	gm->subMessageIdentifier.value = c_h239_smid_presentationTokenIndicateOwner;
	gm->subMessageIdentifier.filled = true;

	gm->messageContent.reset(new VS_H245GenericParameter[2], 2);
	SetH245ParameterValue_unsignedMin(gm->messageContent[0], c_h239_pid_terminalLabel, 0);
	SetH245ParameterValue_unsignedMin(gm->messageContent[1], c_h239_pid_channelId, ctx->GetH245Params()->m_slidesNumberLCSender);
	gm->messageContent.filled = true;

	gm->filled = true;

	m->choice = gm;
	m->tag = VS_H245IndicationMessage::e_genericIndication;
	m->filled = true;

	mscm.choice = m;
	mscm.tag = VS_H245MultimediaSystemControlMessage::e_indication;
	mscm.filled = true;

	const bool res = PutH245Message(mscm, ctx);
	assert(res);
}

void VS_H323Parser::SendH245LogicalChannelActiveIndication(unsigned lc_number, bool active, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245MultimediaSystemControlMessage mscm;
	auto m = new VS_H245IndicationMessage;
	auto mi = new VS_H245MiscellaneousIndication;

	mi->logicalChannelNumber.value = lc_number;
	mi->logicalChannelNumber.filled = true;
	mi->type.choice = new VS_AsnNull;
	mi->type.tag = active ? VS_H245MiscellaneousIndication_Type::e_logicalChannelActive : VS_H245MiscellaneousIndication_Type::e_logicalChannelInactive;
	mi->type.filled = true;
	mi->filled = true;

	m->choice = mi;
	m->tag = VS_H245IndicationMessage::e_miscellaneousIndication;
	m->filled = true;

	mscm.choice = m;
	mscm.tag = VS_H245MultimediaSystemControlMessage::e_indication;
	mscm.filled = true;

	const bool res = PutH245Message(mscm, ctx);
	assert(res);
}

void VS_H323Parser::TerminateSession(const std::shared_ptr<VS_H323ParserInfo>& ctx, int term_reason)
{
	if (ctx->GetHangupMode() == e_my_hangup)
	{
		MakeReleaseComplete(ctx);
	}
	else
	{
		MakeEndSessionCommand(ctx);
	}
	HangupCall(ctx->GetDialogID());
	CleanParserContext(ctx->GetDialogID(), SourceClean::PARSER);
}

void VS_H323Parser::SerializeAndSendContainerToKostya(VS_Container &cnt)
{
	////TODO

	/*void* data = 0;
	unsigned long data_sz = 0;

	if (!cnt.SerializeAlloc(data, data_sz))
		return ;

	if (!m_mgr->GetCallInfo()->SetAppData((unsigned char*) data, data_sz))
	{
		// Если мы не можем полсать, то это значит, что Матвей не готов отослать Косте данные
		m_Logger->TPrintf("[send] VS_CallService: SerializeAndSendContainerToKostya: SetAppData failed");
	}else{
		m_Logger->TPrintf("[send] VS_CallService: SerializeAndSendContainerToKostya: SetAppData: OK");
	}

	if (data) { delete data; data = 0; }*/
}

bool VS_H323Parser::OnSetupArrived(VS_PerBuffer &aInBuffer, VS_Q931 &aQ931_In, net::address fromAddr, net::port fromPort)
{

	unsigned char dn[83] = { 0 };
	unsigned char e164[50] = { 0 };

	if ( !VS_Q931::GetUserUserIE(  aInBuffer, dn, e164 ) )
		return false;

	bool isE164 = false;
	VS_CsH323UserInformation ui;
	if ( !ui.Decode(aInBuffer) )
		return false;

	VS_CsSetupUuie *setup = ui.h323UuPdu.h323MessageBody;

	if ( !setup )		// non-Setup message
		return false;

	char call_identifier[CONFERENCEID_LENGTH * 2 + 1] = {0};

	// encoded call identifier
	if ( setup->callIdentifier.guid.filled )
	{
		VS_BitBuffer* bitBuffer = &(setup->callIdentifier.guid.value);
		void* data = bitBuffer->GetData();
		EncodeDialogID(static_cast<const unsigned char*>(data), call_identifier);
	}

	std::shared_ptr<VS_H323GatekeeperStorage::Info> info =
			VS_H323GatekeeperStorage::Instance().GetTerminalInfo(call_identifier);
	std::shared_ptr<VS_H323ParserInfo> ctx;
	// If this is incoming call from registred h323-terminal, we try to create
	// the same dialog_id as used for registration.
	std::shared_ptr<VS_H225RASParserInfo> nfo;
	if(info)
		nfo = info->context.lock();
	if (nfo)
	{
		ctx = GetParserContext(nfo->GetDialogID(), true);
		ctx->SetDialogID(nfo->GetDialogID());
	}
	else
	{
		ctx = GetParserContext(call_identifier, true);
		ctx->SetDialogID(call_identifier);
	}
	if (!ctx)
		return false;

	if (!ctx->h235_auth.ValidateTokens(setup->tokens)) {
		// In the cases in which there are no overlapping capabilities
		ctx->SetHangupMode(e_my_hangup);
		TerminateSession(ctx, VS_H225ReleaseCompleteReason::e_securityDenied);
		return false;
	}

	ctx->SetCallDirection(e_in);

	ctx->SetCallIdentifier(call_identifier);

	if( setup->conferenceID.filled )
	{
		char temp[CONFERENCEID_LENGTH * 2 + 1] = {0};

		VS_BitBuffer* bitBuffer = &(setup->conferenceID.value);
		void* data = bitBuffer->GetData();
		EncodeDialogID(static_cast<const unsigned char*>(data), temp);

		ctx->SetConferenceID(temp);
	}

	{
		std::string from_id(H323_CALL_ID_PREFIX);
		if (setup->sourceAddress.filled && !setup->sourceAddress.empty())
		{
			VS_GwH225ArrayOf_AliasAddress* sourceAddress_array = (VS_GwH225ArrayOf_AliasAddress*)&(setup->sourceAddress);

			char src_alias[256] = { 0 };
			std::size_t src_alias_sz = 0;
			char src_digit[256] = { 0 };
			std::size_t src_digit_sz = 0;

			const bool alias_present = sourceAddress_array->GetAlias(src_alias, src_alias_sz);
			if (alias_present)
			{
				from_id += src_alias;
				ctx->SetSrcAlias(std::string{ src_alias, src_alias_sz });
			}

			if (sourceAddress_array->GetDigit(src_digit, src_digit_sz))
			{
				if (!alias_present)
				{
					from_id += "\\e\\";
					from_id += src_digit;
				}
				ctx->SetSrcDigit(std::string{ src_digit,  src_digit_sz });
			}
		}

		from_id += "@";
		net::address srcAddr;
		if (setup->sourceCallSignalAddress.filled && get_ip_address(setup->sourceCallSignalAddress, srcAddr, vs::ignore<net::port>()))
			from_id += srcAddr.to_string();
		else
			from_id += fromAddr.to_string();
		ctx->SetAliasRemote(from_id);
	}

	if (*dn)
		ctx->SetDisplayNamePeer((const char*)&dn[0]);
	else if (*e164)
	{
		std::string str = "\\e\\";
		str += (const char*)&e164[0];
		ctx->SetDisplayNamePeer(str);
	}
	else
		ctx->SetDisplayNamePeer(ctx->GetAliasRemote());

	const size_t e164_len = strlen((char*)e164);
	if (e164_len > 0)
	{
		//		bool ret = m_mgr->GetCallInfo()->GetH323CallInfoInterface()->SetQ931CalledPartyNumber((char* )e164);
		ctx->SetDstDigit(std::string{ (const char*)e164, e164_len });
		isE164 = true;
	}

	if ( setup->destinationAddress.filled && !setup->destinationAddress.empty() )
	{
		VS_GwH225ArrayOf_AliasAddress* destinationAddress_array = (VS_GwH225ArrayOf_AliasAddress*) &(setup->destinationAddress);

		char dst_alias_digit[256] = { 0 };
		std::size_t dst_alias_digit_sz = 0;

		if ( destinationAddress_array->GetAlias(dst_alias_digit, dst_alias_digit_sz) )
		{
			ctx->SetDstAlias(std::string{ dst_alias_digit, dst_alias_digit_sz });
		}

		char dst_url[512] = { 0 };
		std::size_t dst_url_sz = 0;

		if ( destinationAddress_array->GetUrl(dst_url, dst_url_sz) )
		{
			// remove protocol "h323:"
			char* p = strchr(dst_url,':');
			if (!p)
				p = dst_url;
			else
				++p;
			assert(p);
			ctx->SetDstAlias(std::string(p));
		}

		if ( destinationAddress_array->GetDigit(dst_alias_digit, dst_alias_digit_sz) )
		{
			if ( !isE164 )
				ctx->SetDstDigit(std::string{ dst_alias_digit, dst_alias_digit_sz });
		}
	}

	ctx->SetCRV(aQ931_In.callReference);

	// extract Product ID and Version ID from setup message
	{
		auto extract_product_id = [&setup](void) -> std::string
		{
			if (setup->sourceInfo.filled &&
				setup->sourceInfo.vendor.filled &&
				setup->sourceInfo.vendor.productId.filled)
			{
				auto data = setup->sourceInfo.vendor.productId.value.GetData();
				return static_cast<char *>(data);
			}
			return {};
		};

		auto extract_version_id = [&setup](void) -> std::string
		{

			if (setup->sourceInfo.filled &&
				setup->sourceInfo.vendor.filled &&
				setup->sourceInfo.vendor.versionId.filled)
			{
				auto data = setup->sourceInfo.vendor.versionId.value.GetData();
				return static_cast<char *>(data);
			}

			return {};
		};

		auto product_id = extract_product_id();
		auto version_id = extract_version_id();
		if (!product_id.empty() && !version_id.empty())
		{
			ctx->SetUserAgent(std::move(product_id) + "::" + std::move(version_id));

			auto &old_config = ctx->GetConfig();
			const auto new_config = CreateCallConfig(old_config.Address, {}, ctx->GetUserAgent());
			create_call_config_manager(old_config).MergeWith(new_config);
		}
	}

	// Auto Detect Terminal Type on Connect-message
	if ( ctx->IsH323UserTypeAutoDetect() )
	{
		// POLYCOM
		if ( setup->sourceInfo.vendor.filled &&
			setup->sourceInfo.vendor.h221NonStandard.filled &&
			setup->sourceInfo.vendor.h221NonStandard.t35CountryCode.filled &&
			setup->sourceInfo.vendor.h221NonStandard.t35Extension.filled &&
			setup->sourceInfo.vendor.h221NonStandard.manufacturerCode.filled )
		{
			if ( ( T35_COUNTRY_CODE_USA == setup->sourceInfo.vendor.h221NonStandard.t35CountryCode.value ) &&		// USA
				( 0x00 == setup->sourceInfo.vendor.h221NonStandard.t35Extension.value ) &&
				( T35_MANUFACTURER_CODE_POLYCOM == setup->sourceInfo.vendor.h221NonStandard.manufacturerCode.value ) )	// Polycom
			{
				ctx->SetH323UserType(H323UT_POLICAMVV);
			}
		}

		// Tandberg
		if ( setup->sourceInfo.vendor.filled &&
			setup->sourceInfo.vendor.productId.filled )
		{
			char* productId = static_cast<char*>(setup->sourceInfo.vendor.productId.value.GetData());
			if (productId && !strcasecmp("tandberg", productId) )
				ctx->SetH323UserType(H323UT_TANDBERG);
		}

		// CISCO
		if ( ui.h323UuPdu.filled &&
			ui.h323UuPdu.nonStandardControl.filled &&
			!ui.h323UuPdu.nonStandardControl.empty())
		{
			VS_H225NonStandardIdentifier& id = ui.h323UuPdu.nonStandardControl[0].nonStandardIdentifier;

			VS_H225H221NonStandard* h221 = static_cast<VS_H225H221NonStandard*>(id);

			if ( h221 && h221->t35CountryCode.filled && h221->t35Extension.filled && h221->manufacturerCode.filled )
				if ( ( T35_COUNTRY_CODE_USA == h221->t35CountryCode.value ) &&					// USA
					( 0x00 == h221->t35Extension.value ) &&
					( T35_MANUFACTURER_CODE_CISCO == h221->manufacturerCode.value ) )				// Cisco
				{
					ctx->SetH323UserType(H323UT_CISCO);
				}
		}

		// NetMeeting
		if ( ui.h323UuPdu.filled &&
			ui.h323UuPdu.nonStandardData.filled &&
			ui.h323UuPdu.nonStandardData.nonStandardIdentifier.filled )
		{
			VS_H225H221NonStandard* h221 = static_cast<VS_H225H221NonStandard*>(ui.h323UuPdu.nonStandardData.nonStandardIdentifier);

			if ( h221 && h221->t35CountryCode.filled && h221->t35Extension.filled && h221->manufacturerCode.filled )
				if ( ( T35_COUNTRY_CODE_USA == h221->t35CountryCode.value ) &&					// USA
					( 0x00 == h221->t35Extension.value ) &&
					( T35_MANUFACTURER_CODE_NETMEETING == h221->manufacturerCode.value ) )		// NetMeeting
				{
					ctx->SetH323UserType(H323UT_MSNET);
				}
		}
	}

	auto update_call_state = [this, wctx = std::weak_ptr<VS_H323ParserInfo>(ctx)](const bool lastOpResult, const bool timeout, net::address, net::port) -> bool {
		auto ctx = wctx.lock();

		dprint3("update_call_state(): ctx: %s, last_op_result: %s, timeout: %s\n",
			ctx == nullptr ? "NOT FOUND" : "found",
			lastOpResult ? "true" : "false",
			timeout ? "true" : "false");

		if (ctx == nullptr)
			return false;

		if (!lastOpResult || timeout || !MakeH225Alerting(ctx->GetDialogID()))
		{
			MakeReleaseComplete(ctx);
			return false;
		}

		auto update_status = UpdateCallState(ctx);
		if (!update_status)
		{
			dprint3("update_call_state(): call to UpdateCallState() failed!\n");
		}
		return update_status;
	};

	if(!VS_H323ExternalGatekeeper::Instance().ARQForIncomingCall(setup, update_call_state))
	{
		dprint3("VS_H323Parser::OnSetupArrived(): call to ARQForIncomingCall() failed!\n");
		MakeReleaseComplete(ctx);
		return false;
	}

	return true;
}

bool VS_H323Parser::OnConnectArrived(VS_PerBuffer &aInBuffer, VS_Q931 &aQ931_In)
{
	//std::shared_ptr<VS_H323ParserInfo> ctx = GetParserContext(dialog_id);
	//if (!ctx) return false;
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return false;

	VS_CsH323UserInformation ui;
	unsigned char dn[83] = { 0 };
	unsigned char e164[50] = { 0 };
	if (!VS_Q931::GetUserUserIE( aInBuffer, dn, e164))
		return false;

	if ( !ui.Decode(aInBuffer) )
		return false;

	VS_CsConnectUuie *connect = ui.h323UuPdu.h323MessageBody;

	if ( !connect )		// non-Connect message
		return false;


	char dialog_id[CONFERENCEID_LENGTH * 2 + 1] = { 0 };

	// dialog id
	if ( connect->callIdentifier.guid.filled )
	{
		VS_BitBuffer* bitBuffer = &(connect->callIdentifier.guid.value);
		EncodeDialogID(static_cast<const unsigned char*>(bitBuffer->GetData()), dialog_id);
	}

	if (!*dialog_id)
		return false;

	//char* dialog_id = (char*) connect->conferenceID.value.GetData();
	std::shared_ptr<VS_H323ParserInfo> ctx = GetParserContext(dialog_id);

	if (!ctx)
		return false;

	// Мы пока не поддерживаем туннелинг H.245 - значит только отдельный канал для него
	if ( !connect->h245Address.filled )
		return false;

	net::address h245_ip;
	net::port h245_port;
	if(!get_ip_address(connect->h245Address, h245_ip, h245_port) || h245_ip.is_unspecified())
		return false;
	//dprint4("H245 port: %s", h245_port);

	//iSubStatus = e_connect_received;

	if (!ctx->h235_auth.ValidateTokens(connect->tokens)) {
		ctx->SetHangupMode(e_my_hangup);
		TerminateSession(ctx, VS_H225ReleaseCompleteReason::e_securityDenied);
		return false;
	}

	// extract Product ID and Version ID from setup message
	{
		auto extract_product_id = [&connect](void) -> std::string {

			if (connect->destinationInfo.filled &&
				connect->destinationInfo.vendor.filled &&
				connect->destinationInfo.vendor.productId.filled)
			{
				auto &&data = connect->destinationInfo.vendor.productId.value.GetData();
				return std::string(static_cast<const char *>(data));
			}
			return {};
		};

		auto extract_version_id = [&connect](void) -> std::string {

			if (connect->destinationInfo.filled &&
				connect->destinationInfo.vendor.filled &&
				connect->destinationInfo.vendor.versionId.filled)
			{
				auto &&data = connect->destinationInfo.vendor.versionId.value.GetData();
				return std::string(static_cast<const char *>(data));
			}

			return {};
		};

		auto product_id = extract_product_id();
		auto version_id = extract_version_id();
		if (!product_id.empty() && !version_id.empty())
		{
			ctx->SetUserAgent(std::move(product_id) + "::" + std::move(version_id));

			auto &old_config = ctx->GetConfig();
			const auto new_config =	CreateCallConfig(old_config.Address, {}, ctx->GetUserAgent());
			create_call_config_manager(old_config).MergeWith(new_config);
		}
	}

	// Auto Detect Terminal Type on Connect-message
	if ( ctx->IsH323UserTypeAutoDetect() )
	{
		// POLYCOM
		if ( connect->destinationInfo.vendor.filled &&
			connect->destinationInfo.vendor.h221NonStandard.filled &&
			connect->destinationInfo.vendor.h221NonStandard.t35CountryCode.filled &&
			connect->destinationInfo.vendor.h221NonStandard.t35Extension.filled &&
			connect->destinationInfo.vendor.h221NonStandard.manufacturerCode.filled )
		{
			if ( ( T35_COUNTRY_CODE_USA == connect->destinationInfo.vendor.h221NonStandard.t35CountryCode.value ) &&		// USA
				( 0x00 == connect->destinationInfo.vendor.h221NonStandard.t35Extension.value ) &&
				( T35_MANUFACTURER_CODE_POLYCOM == connect->destinationInfo.vendor.h221NonStandard.manufacturerCode.value ) )	// Polycom
			{
				ctx->SetH323UserType(H323UT_POLICAMVV);
			}
		}

		// Tandberg
		if ( connect->destinationInfo.vendor.filled &&
			connect->destinationInfo.vendor.productId.filled )
		{
			const char *productId = static_cast<const char*>(connect->destinationInfo.vendor.productId.value.GetData());
			if (productId && !strcasecmp("tandberg", productId))
				ctx->SetH323UserType(H323UT_TANDBERG);
		}

		// CISCO
		if ( ui.h323UuPdu.filled &&
			ui.h323UuPdu.nonStandardControl.filled &&
			!ui.h323UuPdu.nonStandardControl.empty())
		{
			VS_H225NonStandardIdentifier& id = ui.h323UuPdu.nonStandardControl[0].nonStandardIdentifier;

			VS_H225H221NonStandard* h221 = static_cast<VS_H225H221NonStandard*>(id);

			if ( h221 && h221->t35CountryCode.filled && h221->t35Extension.filled && h221->manufacturerCode.filled )
				if ( ( T35_COUNTRY_CODE_USA == h221->t35CountryCode.value ) &&					// USA
					( 0x00 == h221->t35Extension.value ) &&
					( T35_MANUFACTURER_CODE_CISCO == h221->manufacturerCode.value ) )				// Cisco
				{
					ctx->SetH323UserType(H323UT_CISCO);
				}
		}

		// NetMeeting
		if ( ui.h323UuPdu.filled &&
			ui.h323UuPdu.nonStandardData.filled &&
			ui.h323UuPdu.nonStandardData.nonStandardIdentifier.filled )
		{
			VS_H225H221NonStandard* h221 = static_cast<VS_H225H221NonStandard*>(ui.h323UuPdu.nonStandardData.nonStandardIdentifier);

			if ( h221 && h221->t35CountryCode.filled && h221->t35Extension.filled && h221->manufacturerCode.filled )
				if ( ( T35_COUNTRY_CODE_USA == h221->t35CountryCode.value ) &&					// USA
					( 0x00 == h221->t35Extension.value ) &&
					( T35_MANUFACTURER_CODE_NETMEETING == h221->manufacturerCode.value ) )		// NetMeeting
				{
					ctx->SetH323UserType(H323UT_MSNET);
				}
		}
	}

	confMethods->UpdateDisplayName(ctx->GetDialogID(), string_view{ (const char *)dn }, true);

	if (!InitH245(ctx, h245_ip, h245_port))
		return false;
	ctx->SetTCSWaitingTick(clock().now());
	return true;
}

int VS_H323Parser::GetBufForSend(void *buf, std::size_t &sz, const VS_ChannelID channelId,
	const net::address &remoteAddr, net::port remotePort, const net::address &localAddr, net::port localPort)
{
	std::lock_guard<decltype(m_queue_out_lock)> lock{ m_queue_out_lock };
	VS_MessageQueue* queue_out = GetOutputQueue(channelId);
	if (!queue_out)
		return 0;

	auto msg_sz = queue_out->GetChannelMessageSize(channelId);
	if (msg_sz == 0 || sz < msg_sz)
	{
		sz = msg_sz;
		return false;
	}

	std::unique_ptr<unsigned char[]> tmp_buf = queue_out->GetChannelMessage(msg_sz, channelId);
	if (!tmp_buf)
		return false;

	dprint4("VS_H323Parser::GetBufForSend: %u bytes\n", msg_sz);

	memcpy(buf, tmp_buf.get(), msg_sz);
	sz = msg_sz;
	return true;
}

acs::Response VS_H323Parser::Protocol(const void *buf, std::size_t sz)
{
	if (!buf || sz == 0)
		return acs::Response::next_step;

	bool is_fragmented;
	const VS_ChannelID channel_id = GetChannelID(buf, sz, is_fragmented);

	if(is_fragmented)
		return acs::Response::next_step;

	if (channel_id == e_H225)
	{
		return acs::Response::accept_connection;
	}
	return acs::Response::not_my_connection;
}

VS_ChannelID VS_H323Parser::GetChannelID(const void *buf, std::size_t sz, bool& isFragmented)
{
	isFragmented = false;
	VS_ChannelID res = e_noChannelID;

	unsigned char* in_buff = new unsigned char[sz];
	memcpy(in_buff, buf, sz);

	VS_TearMessageQueue queue;
	queue.PutTearMessage(in_buff, sz);

	if (queue.GetState() == VS_TearMessageQueue::e_tearBody)
	{
		isFragmented = true;
		return res;
	}

	if ( queue.IsExistEntireMessage() )
	{
		std::unique_ptr<unsigned char[]> ent_mess = nullptr;
		uint32_t ent_mess_sz = 0;

		if ( !queue.GetEntireMessage(ent_mess.get(), ent_mess_sz) )
		{
			if (ent_mess_sz)
			{
				ent_mess = vs::make_unique<unsigned char[]>(ent_mess_sz);
				if (!queue.GetEntireMessage(ent_mess.get(), ent_mess_sz) )
				{
					return res;
				}
			}else{
				// ошибка внутри очереди
				return res;
			}
		}else{
			// Память не выделили, а данные нам дали
			// Не знаю, как сюда можно попасть :)
			return res;
		}

		VS_PerBuffer in_per_buff_h225{ ent_mess.get(), ent_mess_sz * 8 };

		VS_Q931 theQ931_In;
		if ( theQ931_In.DecodeMHeader(in_per_buff_h225) )
		{
			res = e_H225;
		}
	}

	return res;
}

std::string VS_H323Parser::NewDialogID(string_view sipTo, string_view dtmf, const VS_CallConfig &config, string_view myName)
{
	if (sipTo.empty())
		return {};

	auto dialog_id_new = VS_H323ExternalGatekeeper::Instance().MakeConferenceID(myName, sipTo);

	std::string sip_to;
	if (sipTo.find('@') == string_view::npos && !config.HostName.empty())
	{
		sip_to.reserve(sipTo.length() + 1 /*char @*/ + config.HostName.length());
		sip_to.assign(sipTo.cbegin(), sipTo.cend());
		sip_to.push_back('@');
		sip_to.append(config.HostName);
	}
	else
	{
		sip_to.assign(sipTo.cbegin(), sipTo.cend());
	}

	std::shared_ptr<VS_H323ParserInfo> ctx = GetParserContext(dialog_id_new, true);
	ctx->SetDialogID(dialog_id_new);
	ctx->SetMyDialedDigit(config.h323.DialedDigit);
	ctx->SetConfig(config);

	ctx->SetSIPRemoteTarget(std::move(sip_to));
	ctx->SetDTMF(std::string(dtmf));
	dstream3 << "NewDialogID: " << dialog_id_new << ", sip_to: " << ctx->GetSIPRemoteTarget();

	return dialog_id_new;
}

std::string VS_H323Parser::SetNewDialogTest(string_view newDialog, string_view sipTo, string_view dtmf,
	const VS_CallConfig &config, string_view myName)
{
	if (sipTo.empty())
		return {};

	VS_H323ExternalGatekeeper::Instance().SetConferenceIDTest(myName, sipTo, newDialog);

	std::shared_ptr<VS_H323ParserInfo> ctx = GetParserContext(newDialog, true);
	ctx->SetDialogID(std::string(newDialog));
	if(!config.h323.DialedDigit.empty())
		ctx->SetMyDialedDigit(config.h323.DialedDigit);
	ctx->SetDTMF(std::string(dtmf));
	dstream3 << "NewDialogID: " << newDialog << ", sip_to: " << sipTo;

	return std::string(newDialog);
}

void VS_H323Parser::GenerateNewDialogID(char(&buffer)[32 + 1 /*0-terminator*/])
{
	VS_GenKeyByMD5(buffer);
	// After decoding using DecodeDialogID() it will contains
	// 16-bytes binary representation of md5 - our conference_id.
}

void VS_H323Parser::Shutdown()
{
	std::vector<std::string> to_hangup;
	{
		std::lock_guard<decltype(m_ctx_lock)> lock(m_ctx_lock);
		for (auto it = m_ctx.cbegin(); it != m_ctx.cend(); ++it)
		{
			to_hangup.emplace_back(it->second->GetDialogID());
		}
	}

	for(auto it=to_hangup.cbegin(); it!=to_hangup.cend(); ++it)
	{
		Hangup(*it);
	}
}

void VS_H323Parser::Timeout()
{
	const auto now = clock().now();

	std::vector<std::shared_ptr<VS_H323ParserInfo>> terminated_contexts;

	std::lock_guard<decltype(m_ctx_lock)> lock(m_ctx_lock);
 	for (auto it = m_ctx.begin(); it != m_ctx.end(); ++it)
 	{
 		std::shared_ptr<VS_H323ParserInfo> ctx = it->second;
		if (ctx->IsTCSSendNeeded(now))
		{
			SendTCS(ctx);
		}
		VS_H245Data* h245params = ctx->GetH245Params();
		MSDState state = ctx->MsdState();
		if (state == MSDIdle && ctx->IsMSDSendNeeded())
		{
			SendMSD(ctx);
			ctx->StartMSDTimer();
			ctx->SetMsdState(MSDOutgoingAwaitingResponse);
		} else if (state == MSDOutgoingAwaitingResponse && ctx->IsMSDTimerExpired())
		{
			dprint3("VS_H323Parser::Timeout: MSDError (A) no response from remote MSDSE\n");
			SendMSDRel(ctx);
			ctx->SetMsdState(MSDIdle);
			terminated_contexts.push_back(ctx);
		} else if (state == MSDIncomingAwaitingResponse && ctx->IsMSDTimerExpired())
		{
			dprint3("VS_H323Parser::Timeout: MSDError (A) no response from remote MSDSE\n");
			ctx->SetMsdState(MSDIdle);
			terminated_contexts.push_back(ctx);
		}
		if (CheckMSD(ctx) && CheckTCS(ctx))
		{
			if (
				((h245params->H245Flags[VS_H323ParserInfo::e_olc_video] & h245params->h245_req_send)==0) ||
				((h245params->H245Flags[VS_H323ParserInfo::e_olc_audio] & h245params->h245_req_send)==0)
				)  ///OLC Send is not begin
			{
				if (!CreateOLCs(ctx))
				{
					terminated_contexts.push_back(ctx);
				}
			}
		}
		if (ctx->IsH239Enabled() && ctx->IsH239CapabilityPresent())
		{
			auto token = ctx->GetH239PresentationToken();
			if (token->owned && now - token->last_indication_time > c_h239_indication_period)
				SendH239PresentationTokenIndicateOwner(ctx);
		}
		//UpdateMediaModes(ctx);
 	}

	for(auto it = terminated_contexts.begin();
		it != terminated_contexts.end(); ++it)
	{
		CleanParserContext((*it)->GetDialogID(), SourceClean::PARSER);
		(*it)->SetHangupMode(e_my_hangup);
		TerminateSession(*it);
	}

	while(true)
	{
		std::string dialog_id;
		{
			std::lock_guard<decltype(m_ctx_garbage_lock)> _(m_ctx_garbage_lock);
			auto iter = m_ctx_garbage.cbegin();
			if (iter!=m_ctx_garbage.cend()) {
				dialog_id=*iter;
				m_ctx_garbage.erase(iter);
			} else {
				break;
			}
		}
		dstream3 << "fireDialogFinished(" <<  dialog_id << ")";
		m_fireDialogFinished(dialog_id);
	}
}

void VS_H323Parser::SendSetMediaChannels(const std::shared_ptr<VS_H323ParserInfo>& ctx) const
{
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;

	std::vector<VS_MediaChannelInfo> media_channels;
	media_channels.reserve(4);
	auto audio_channel_it = media_channels.insert(media_channels.end(), ctx->audio_channel);
	auto video_channel_it = media_channels.insert(media_channels.end(), ctx->video_channel);
	auto slides_channel_it = media_channels.insert(media_channels.end(), ctx->slides_channel);
	auto data_channel_it = media_channels.insert(media_channels.end(), ctx->data_channel);

	// scaling due to values in x1000, not x1024
	for (auto& mode: video_channel_it->rcv_modes_video)
		mode.Bitrate = mode.Bitrate / 1000 * 1024;
	video_channel_it->snd_mode_video.Bitrate = video_channel_it->snd_mode_video.Bitrate / 1000 * 1024;
	for (auto& mode: slides_channel_it->rcv_modes_video)
		mode.Bitrate = mode.Bitrate / 1000 * 1024;
	slides_channel_it->snd_mode_video.Bitrate = slides_channel_it->snd_mode_video.Bitrate / 1000 * 1024;

	const VS_CallConfig& config = ctx->GetConfig();
	if (config.codecParams.h264_snd_preferred_width.is_initialized() && *config.codecParams.h264_snd_preferred_width > 0 &&
		config.codecParams.h264_snd_preferred_height.is_initialized() && *config.codecParams.h264_snd_preferred_height > 0)
	{
		video_channel_it->snd_mode_video.preferred_width = *config.codecParams.h264_snd_preferred_width;
		video_channel_it->snd_mode_video.preferred_height = *config.codecParams.h264_snd_preferred_height;
	}
	if (config.codecParams.h264_force_cif_mixer.is_initialized() && *config.codecParams.h264_force_cif_mixer)
	{
		video_channel_it->snd_mode_video.IsMixerCIFMode = true;
	}
	const bool siren_swap_bytes = config.codecParams.siren_swap_bytes.get_value_or(false);
	for (auto& mode: audio_channel_it->rcv_modes_audio)
	{
		switch (mode.CodecType)
		{
		case e_rcvSIREN14_24:
		case e_rcvSIREN14_32:
		case e_rcvSIREN14_48:
			mode.SwapBytes = siren_swap_bytes;
			break;
		}
	}
	switch (audio_channel_it->snd_mode_audio.CodecType)
	{
	case e_rcvSIREN14_24:
	case e_rcvSIREN14_32:
	case e_rcvSIREN14_48:
		audio_channel_it->snd_mode_audio.SwapBytes = siren_swap_bytes;
		break;
	}

	if (config.codecParams.gconf_to_term_width.is_initialized() && *config.codecParams.gconf_to_term_width &&
		config.codecParams.gconf_to_term_height.is_initialized() && *config.codecParams.gconf_to_term_height)
	{
		video_channel_it->snd_mode_video.gconf_to_term_width = *config.codecParams.gconf_to_term_width;
		video_channel_it->snd_mode_video.gconf_to_term_height = *config.codecParams.gconf_to_term_height;
	}

	// We should set remote addresses to allow transceiver properly support both IPv4 and IPv6.
	if (!m_remote_ip.empty())
	{
		boost::system::error_code ec;
		net::address remote_addr = net::address::from_string(m_remote_ip, ec);

		if (!remote_addr.is_unspecified())
		{
			for (auto &chan : media_channels)
			{
				if (chan.remote_rtp_address.address().is_unspecified())
				{
					chan.remote_rtp_address.address(remote_addr);
				}

				if (chan.remote_rtcp_address.address().is_unspecified())
				{
					chan.remote_rtcp_address.address(remote_addr);
				}
			}
		}
	}
	const std::string &remote_peer = ctx->GetAliasMy();
	dstream3 << "VS_H323Parser::SendSetMediaChannels. Remote peer:" << remote_peer;
	confMethods->SetMediaChannels(ctx->GetDialogID(), media_channels, remote_peer);
}

bool VS_H323Parser::SetMediaChannels(string_view dialogId, const std::vector<VS_MediaChannelInfo>& channels, const std::string& /*existingConfID*/, std::int32_t bandwRcv)
{
	if (dialogId.empty())
		return false;

	std::shared_ptr<VS_H323ParserInfo> ctx = GetParserContext(dialogId);
	if (!ctx)
		return false;

	for (const auto& channel : channels)
	{

		net::address media_ip;
		if (ctx->UseNAT())
		{
			media_ip = ctx->GetMyCsAddress();
		} else
		{
			media_ip = m_my_media_ip;
		}

		if (channel.type == SDPMediaType::audio && channel.content == SDP_CONTENT_MAIN)
		{
			ctx->audio_channel.our_rtp_address = { media_ip, channel.our_rtp_address.port() };
			ctx->audio_channel.our_rtcp_address = { media_ip, channel.our_rtcp_address.port() };
		}
		else if (channel.type == SDPMediaType::video && channel.content == SDP_CONTENT_MAIN)
		{
			ctx->video_channel.our_rtp_address = { media_ip, channel.our_rtp_address.port() };
			ctx->video_channel.our_rtcp_address = { media_ip, channel.our_rtcp_address.port() };
		}
		else if (channel.type == SDPMediaType::video && channel.content == SDP_CONTENT_SLIDES)
		{
			ctx->slides_channel.our_rtp_address = { media_ip, channel.our_rtp_address.port() };
			ctx->slides_channel.our_rtcp_address = { media_ip, channel.our_rtcp_address.port() };
		}
		else if (channel.type == SDPMediaType::application_fecc)
		{
			ctx->data_channel.our_rtp_address = { media_ip, channel.our_rtp_address.port() };
			ctx->data_channel.our_rtcp_address = { media_ip, channel.our_rtcp_address.port() };
		}
	}

	auto numbers = ctx->GetPendingLCList();
	for (auto number : numbers)
	{
		VS_H245LogicalChannelInfo info;
		if (!ctx->GetH245LogicalChannel(number, info))
			continue;
		const auto channel = ctx->GetMediaChannel(info.m_dataType);
		if (!channel)
			continue;
		if (channel->our_rtp_address.port() == 0 || channel->our_rtcp_address.port() == 0)
			continue;

		net::address media_ip;
		if (ctx->UseNAT())
		{
			media_ip = ctx->GetMyCsAddress();
		} else
		{
			media_ip = m_my_media_ip;
		}

		info.m_our_rtp_address = { media_ip, channel->our_rtp_address.port() };
		info.m_our_rtcp_address = { media_ip, channel->our_rtcp_address.port() };

		if (!ctx->SetH245LogicalChannel(number, info))
			continue;
		if (info.m_isSender)
		{
			if (!SendOLC(number, ctx))
				continue;
		}
		else
		{
			if (!CreateOLCA(number, ctx))
				continue;
		}
		ctx->SetLCPending(number, false);
	}
	UpdateCallState(ctx);
	return true;
}

bool VS_H323Parser::FillMediaChannels(string_view dialogId, std::vector<VS_MediaChannelInfo>& channels)
{
	if (dialogId.empty())
		return false;

	std::shared_ptr<VS_H323ParserInfo> ctx = GetParserContext(dialogId);
	if (!ctx)
		return false;

	channels.push_back(ctx->audio_channel);
	channels.push_back(ctx->video_channel);
	if (ctx->IsH239Enabled() && ctx->IsH239CapabilityPresent())
		channels.push_back(ctx->slides_channel);
	if (ctx->IsH224Enabled() && ctx->IsH224CapabilityPresent())
		channels.push_back(ctx->data_channel);

	return true;
}

bool VS_H323Parser::IsTrunkFull()
{
	//todo: In h323 now we do not support several calls through one signal connection. But it's needed to support;
	return true;
}


void VS_H323Parser::SetPeerCSAddress(string_view dialogId, const net::Endpoint & ep)
{
	if (m_remote_ip.empty())
	{
		m_remote_ip = ep.addr.to_string();
		}

	if (dialogId.empty())
		return ;

	std::shared_ptr<VS_H323ParserInfo> ctx = GetParserContext(dialogId);
	if (!ctx)
		return ;

	ctx->SetPeerCsAddress(ep.addr, ep.port);
}

bool VS_H323Parser::InviteMethod(string_view dialogId, string_view fromId, string_view toId, const VS_ConferenceInfo &cfgInfo,
	string_view dnFromUTF8, bool newSession, bool forceCreate)
{
	dstream1 << "VS_H323Parser::InviteMethod(dialog:" << dialogId << ", from:" << fromId << ", to:" << toId;

	std::shared_ptr<VS_H323ParserInfo> ctx = GetParserContext(dialogId);
	if (!ctx)
		return false;
	ctx->SetGroupConf(cfgInfo.is_group_conf);

	if (ctx->GetMyLocalCsAddress().is_unspecified())
	{
		ctx->SetMyCsAddress(m_myCsEp.addr, m_myCsEp.port);
	}

	if (ctx->UseNAT())
	{
		const auto &cfg = ctx->GetConfig();
		bool nat_addr = false;
		if (!cfg.h323.ExternalAddress.is_unspecified())
		{
			if (cfg.h323.ExternalAddressScheme == VS_CallConfig::eExternalAddressScheme::ALWAYS_EXTERNAL)
				nat_addr = true;
			else if (cfg.h323.ExternalAddressScheme == VS_CallConfig::eExternalAddressScheme::ONLY_INTERNET_ADDRESS) {
				if (!net::is_private_address(ctx->GetPeerCsAddress()))
					nat_addr = true;
			}
		}
		if (nat_addr && !cfg.h323.ExternalAddress.is_unspecified())
		{
			dstream4 << "InviteMethod: NAT for [" << dialogId << ":" << ctx->GetMyExternalCsAddress() << "]";
		}
		else
		{
			ctx->SetMyExternalCsAddress({});
			dstream4 << "InviteMethod: NAT for [" << dialogId << "] is not used";
		}
	}
	else
	{
		dstream4 << "InviteMethod: NAT for [" << dialogId << "] is disabled";
	}

	ctx->SetAliasMy( std::string(fromId) );
	ctx->SetSrcAlias( std::string(fromId) );

	if (!dnFromUTF8.empty())
		ctx->SetDisplayNameMy(std::string(dnFromUTF8));
	else if (!fromId.empty())
		ctx->SetDisplayNameMy(std::string(fromId));

	// extract alias and digit information from call_id
	string_view to(toId);
	if (!to.empty())
	{
		if (to.front() == '#')
		{
			const auto pos = to.find(':');
			if(pos != string_view::npos)
			{
				to = to.substr(pos + 1);
			}
		}
		const auto pos = to.find('/');
		if(pos != string_view::npos)
		{
			to = to.substr(0, pos);
		}
	//ctx->SetDisplayName_sip(to.c_str());
	}

	const auto at_sign = to.find('@');
	if (at_sign != string_view::npos && at_sign != 0)
	{
		string_view user_part{ to.data(), at_sign };
		const char prefix_e[] = "\\e\\";
		const auto res = user_part.find(prefix_e);
		if (res != string_view::npos && res == 0)
		{
			ctx->SetDstDigit(std::string{ user_part.data() + (sizeof(prefix_e) - 1), user_part.length() - (sizeof(prefix_e) - 1) }); // skip "\e\"
		}
		else
		{
			ctx->SetDstAlias(std::string(to));

			if (std::all_of(user_part.begin(), user_part.end(), [](char c){ return isdigit(c); }))	// add second AliasAddress as e164 if only 0-9
				ctx->SetDstDigit(std::string(user_part));
		}
	}


	return MakeH225Setup(ctx);
}

bool VS_H323Parser::InviteReplay(string_view dialogId, VS_CallConfirmCode confirmCode, bool isGroupConf, string_view confName, string_view to_displayName)
{
	dstream1 << "VS_H323Parser::InviteReply(dialog:" << dialogId << ", confirm_code:" << confirmCode << ", isGroupConf:" << isGroupConf;
	std::shared_ptr<VS_H323ParserInfo> ctx = GetParserContext(dialogId);
	if (!ctx)
		return false;
	ctx->SetGroupConf(isGroupConf);

	if (confirmCode == e_call_ok) {
		if (!to_displayName.empty())
			ctx->SetDisplayNameMy(std::string(to_displayName));
		if (!MakeH225Connect(dialogId))
			return false;
		ctx->SetTCSWaitingTick(clock().now());
		//SendTCS(ctx);
	}else{
		Hangup(dialogId);
	}
	return true;
}

void VS_H323Parser::Hangup(string_view dialogId)
{
	std::shared_ptr<VS_H323ParserInfo> ctx = GetParserContext(dialogId, false);
	if (!ctx)
		return;

	ctx->SetHangupMode(e_my_hangup);

	if (ctx->IsInDialog())
	{
		ctx->SetInDialog(false);
		MakeEndSessionCommand(ctx);
	}
	else
	{
		TerminateSession(ctx);
	}
}

void VS_H323Parser::LoggedOutAsUser(string_view dialogId)
{

}

std::shared_ptr<VS_H323ParserInfo> VS_H323Parser::GetDefaultParserContext(const net::address &fromAddr, const net::port fromPort)
{
	std::lock_guard<decltype(m_ctx_lock)> lock(m_ctx_lock);

	for(auto it=m_ctx.cbegin(); it!=m_ctx.cend(); ++it)
	{
		auto &&channel = it->second->GetH245Channel();
		if (channel && channel->RemoteAddress() == fromAddr && channel->RemotePort() == fromPort)
		{
			return it->second;
		}
	}
	if (!m_ctx.empty())		// fallback if not found by h245 address
		return m_ctx.begin()->second;
	return {};
}

std::shared_ptr<VS_ParserInfo> VS_H323Parser::GetParserContextBase(string_view dialogid, bool create)
{
	return GetParserContext(dialogid,create);
}

void VS_H323Parser::CleanParserContext(string_view dialogId, SourceClean source)
{
	if (dialogId.empty())
		return ;

	std::lock_guard<decltype(m_ctx_lock)> lock(m_ctx_lock);
	auto it = m_ctx.find(dialogId);

	if (it == m_ctx.cend()) 	return ;

	auto channel = it->second->GetH245Channel();
	if (channel) channel->Close(true);

	// if dialog_id is used for invite and RegestryStrategy=2, call is finished, do unregister.
/*	if (it->second->IsInviteAfterRegister() && (m_register_dialog_id.find(it->second->GetUser()) == m_register_dialog_id.end()
		 ||  m_register_dialog_id.find( it->second->GetUser() )->second != dialog_id) )
	{
		it->second->IsInviteAfterRegister()->SetExpires( 0 );
		DoRegister( it->second->IsInviteAfterRegister()->GetSIPDialogID() );
		it->second->SetInviteAfterRegister( boost::shared_ptr<VS_SIPParserInfo>() );
	}

	if (m_register_dialog_id.find(it->second->GetUser()) != m_register_dialog_id.end() &&
		m_register_dialog_id.find( it->second->GetUser() )->second == dialog_id)
			m_register_dialog_id.erase(it->second->GetUser()); */

	m_ctx.erase(it);
}

bool VS_H323Parser::PutH245Message(VS_H245MultimediaSystemControlMessage &aMessage, const std::shared_ptr<VS_H323ParserInfo>& ctx) const
{
	auto channel = ctx->GetH245Channel();
	if (!channel)
		return false;

	VS_PerBuffer theBuffer;
	if (!aMessage.Encode(theBuffer))
		return false;

	const auto msg_size = theBuffer.ByteSize();
	if (msg_size == 0)
		return false;

	size_t size = 4 + msg_size;
	vs::SharedBuffer buffer(size);
	auto p = buffer.data<unsigned char>();
	p[0] = 3;
	p[1] = 0;
	p[2] = (size >> 8) & 0xff;
	p[3] = (size)& 0xff;
	if (!theBuffer.GetBits(p + 4, theBuffer.BitSize()))
		return false;

	{
		auto flow = net::QoSSettings::GetInstance().GetH323QoSFlow(false, channel->RemoteAddress().is_v6());
		channel->Open(flow);
		channel->Send(std::move(buffer));
	}
	return true;
}

bool VS_H323Parser::PutOutputMessage(std::unique_ptr<unsigned char[]> buf, std::size_t sz, VS_ChannelID channelId)
{
	std::lock_guard<decltype(m_queue_out_lock)> lock{ m_queue_out_lock };
	VS_MessageQueue* queue = GetOutputQueue(channelId);
	if(!queue)
		return false;

	if(queue->PutMessage(buf.get(), sz, channelId))
	{
		buf.release();
		return true;
}

	return false;
}

VS_TearMessageQueue* VS_H323Parser::GetInputQueue(VS_ChannelID channelId)
{
	switch (channelId)
	{
	case e_H225:
		if ( !m_queue_in_h225 )
			m_queue_in_h225 = new VS_TearMessageQueue();

		return m_queue_in_h225;
	default:
		return {};
	}
}

VS_MessageQueue* VS_H323Parser::GetOutputQueue(VS_ChannelID channelId)
{
	if(channelId == e_H225)
	{
		if ( !m_queue_out ) m_queue_out = new VS_OutputMessageQueue();
		return m_queue_out;
	}
	return nullptr;
}

void VS_H323Parser::CloseConnection(const net::address &addr, net::port port, net::protocol protocol)
{
	if (auto confMethods = m_confMethods.lock())
		confMethods->CloseConnection(addr, port, protocol);;
}

bool VS_H323Parser::MakeH225Setup(const std::shared_ptr<VS_H323ParserInfo>& info)
{
//	if ( !isValid() )
//		return 0;

	info->SetCallDirection(e_out);

	H323_User_Type H323UserType = H323UT_POLICAMVV;
	//if ( !m_mgr->GetCallInfo()->GetH323CallInfoInterface()->GetH323UserType(H323UserType) )
	//	return 0;
	//H323UserType = H323UT_NONE;

	VS_CsSetupUuie *setup = new VS_CsSetupUuie;
	setup->protocolIdentifier = h225ProtocolIdentifier;
	setup->protocolIdentifier.filled = true;
	setup->h245Address.filled = false;

	//std::shared_ptr<VS_H323ParserInfo> info = GetParserContext(dialog_id);

	{// sourceAddress
		unsigned srcAddrCount = 0;

		auto &alias_my = info->GetSrcAlias();
		auto &digit_my = info->GetSrcDigit();

		const bool is_empty_alias_my = alias_my.empty();
		const bool is_empty_didit_my = digit_my.empty();

		if(!is_empty_alias_my) srcAddrCount++;
		if(!is_empty_didit_my) srcAddrCount++;

		VS_H225AliasAddress* aliasAddress = srcAddrCount == 0 ? nullptr : new  VS_H225AliasAddress[srcAddrCount];

		unsigned iter = 0;

		// H323-ID
		if(!is_empty_alias_my)
		{
			assert(!!aliasAddress);
			VS_H225AliasAddress* h323id = &aliasAddress[iter];
			h323id->tag = VS_H225AliasAddress::e_h323_ID;
			h323id->filled = true;
			TemplBmpString<1,256> * tmpAlias = new TemplBmpString<1,256>;
			tmpAlias->value = VS_H323String(alias_my).MakePerBuffer();
			tmpAlias->filled = true;
			h323id->choice = tmpAlias;

			iter++;
		}

		// DialedDigit
		if(!is_empty_didit_my)
		{
			assert(!!aliasAddress);
			VS_H225AliasAddress* h323id = &aliasAddress[iter];
			h323id->tag = VS_H225AliasAddress::e_dialedDigits;
			h323id->filled = true;

			VS_AsnIA5String* tmpDigit = new VS_AsnIA5String( VS_H225AliasAddress::dialedDigits_alphabet,
														sizeof(VS_H225AliasAddress::dialedDigits_alphabet),
														VS_H225AliasAddress::dialedDigits_inverse_table, 1, 128);

			tmpDigit->SetNormalString(digit_my.c_str(), digit_my.length());

			h323id->choice = tmpDigit;

			iter++;
		}


		setup->sourceAddress.reset(aliasAddress, srcAddrCount);
	}

	{// sourceInfo
		setup->sourceInfo.terminalInfo.filled = true;
		setup->sourceInfo.mc.value = false;
		setup->sourceInfo.mc.filled = true;
		setup->sourceInfo.undefinedNode.value = false;
		setup->sourceInfo.undefinedNode.filled = true;

		setup->sourceInfo.vendor.productId.value.AddBits(H323Gateway_Product, (sizeof(H323Gateway_Product) - 1 ) * 8);
		setup->sourceInfo.vendor.productId.filled = true;
		setup->sourceInfo.vendor.versionId.value.AddBits(H323Gateway_Version, (sizeof(H323Gateway_Version) - 1) * 8);
		setup->sourceInfo.vendor.versionId.filled = true;

		setup->sourceInfo.vendor.filled =
			setup->sourceInfo.vendor.versionId.filled
			&
			setup->sourceInfo.vendor.productId.filled ;

		setup->sourceInfo.vendor.h221NonStandard.manufacturerCode.value = 0;//поставлено нашару
		setup->sourceInfo.vendor.h221NonStandard.manufacturerCode.filled = true;
		setup->sourceInfo.vendor.h221NonStandard.t35CountryCode.value = T35_COUNTRY_CODE_RUSSIA;	// Russia
		setup->sourceInfo.vendor.h221NonStandard.t35CountryCode.filled = true;
		setup->sourceInfo.vendor.h221NonStandard.t35Extension.value   = 0;//поставлено нашару
		setup->sourceInfo.vendor.h221NonStandard.t35Extension.filled = true;
		setup->sourceInfo.vendor.h221NonStandard.filled = true;

		setup->sourceInfo.filled = true;
	}

	// destinationAddress - nothing

	{
		// TODO: For UserTerminal - false (MCU, Polycam - true)
		// ! TEMPORARY

		bool isWeHaveInformation = true;

		if (isWeHaveInformation)
		{
			// destCallSignallAddress-H225IpAddres ip port - 1720
			// неясная документация - возможно, что это поле не требуется

			set_ip_address(setup->destCallSignalAddress, info->GetPeerCsAddress(), info->GetPeerCsPort());

			{// destinationAddress
				unsigned dstAddrCount = 0;

				auto &alias = info->GetDstAlias();
				auto &digit = info->GetDstDigit();

				const bool is_alias_empty = alias.empty();
				const bool is_digit_empty = digit.empty();

				if (!is_alias_empty) dstAddrCount++;
				if (!is_digit_empty) dstAddrCount++;

				auto aliasAddress = std::make_unique<VS_H225AliasAddress[]>(dstAddrCount);

				unsigned iter = 0;

				// H323-ID
				if (!is_alias_empty)
				{
					VS_H225AliasAddress* h323id = &aliasAddress[iter];
					h323id->tag = VS_H225AliasAddress::e_h323_ID;
					h323id->filled = true;
					TemplBmpString<1, 256> * tmpAlias = new TemplBmpString<1, 256>;
					tmpAlias->value = VS_H323String(alias).MakePerBuffer();
					tmpAlias->filled = true;
					h323id->choice = tmpAlias;

					iter++;
				}

				// DialedDigit
				if (!is_digit_empty)
				{
					VS_H225AliasAddress* h323id = &aliasAddress[iter];
					h323id->tag = VS_H225AliasAddress::e_dialedDigits;
					h323id->filled = true;

					VS_AsnIA5String* tmpDigit = new VS_AsnIA5String(VS_H225AliasAddress::dialedDigits_alphabet,
						sizeof(VS_H225AliasAddress::dialedDigits_alphabet),
						VS_H225AliasAddress::dialedDigits_inverse_table, 1, 128);

					tmpDigit->SetNormalString(digit.c_str(), digit.length());

					h323id->choice = tmpDigit;

					iter++;
				}

				if(dstAddrCount > 0)
					setup->destinationAddress.reset(aliasAddress.release(), dstAddrCount);
			}
		}
		if (isWeHaveInformation)
		{
			///неясная документация - возможно, что это поле не требуется
			///для звонка в Polycom MGC-100 это поле НЕОБХОДИМО.
			set_ip_address(setup->sourceCallSignalAddress, info->GetMyCsAddress(), info->GetMyCsPort());
			}
		}

	setup->activeMC.filled = true;
	setup->activeMC.value = false;

	/*{
		auto *sec = new VS_H225H245Security[1];
		auto *null = new VS_AsnNull;
		sec[0].filled = true;
		sec[0].tag = VS_H225H245Security::e_noSecurity;
		sec[0].choice = null;
		setup->h245SecurityCapability.filled = true;
		setup->h245SecurityCapability.asns = &sec[0];
		setup->h245SecurityCapability.length = 1;
	}*/

	setup->multipleCalls.filled = true;
	setup->multipleCalls.value = false;

	setup->maintainConnection.filled = true;
	setup->maintainConnection.value = false;

	setup->symmetricOperationRequired.filled = false;

	/*{
		auto *null = new VS_AsnNull;
		setup->presentationIndicator.filled = true;
		setup->presentationIndicator.choice = null;
		setup->presentationIndicator.tag = VS_H225PresentationIndicator::e_presentationAllowed;
	}


	setup->screeningIndicator.value = VS_H225ScreeningIndicator::e_userProvidedVerifiedAndFaile;
	setup->screeningIndicator.filled = true;*/

	/*char* ConfID = new char[CONFERENCEID_LENGTH];

	if ( !info->GetConfID(ConfID) )
	{
		if (setup) { delete setup; setup = 0; }
		if (ConfID) { delete [] ConfID; ConfID = 0; }
		return 0;
	}

	VS_PerBuffer buf(ConfID, CONFERENCEID_LENGTH*8);
	setup->conferenceID.filled = true;
	setup->conferenceID.value =  buf;
	if (ConfID) { delete ConfID; ConfID = 0; } */

	if (info->GetDialogID().empty())
	{
		return false;
	}

	unsigned char ConferenceID[CONFERENCEID_LENGTH];
	DecodeDialogID(info->GetConferenceID().c_str(), ConferenceID);
	VS_PerBuffer buf_confid(ConferenceID, CONFERENCEID_LENGTH*8);

	setup->conferenceID.filled = true;
	setup->conferenceID.value =  buf_confid;


	{// conferenceGoal - tag = 0
		setup->conferenceGoal.filled = true;
		setup->conferenceGoal.tag = 0;

		VS_AsnNull * nuller = new VS_AsnNull;
		(*nuller).filled = true;
		setup->conferenceGoal.choice = nuller;
	}

	// callServices - nothing

	{// callType - tag = 0
		VS_AsnNull *nuller2 = new VS_AsnNull;
		(*nuller2).filled = true;
		setup->callType.filled= true;
		setup->callType.tag = 0;
		setup->callType.choice = nuller2;
	}

	// Extended Attributes
	// sourceCallSignalAddress - nothig
	// remoteExtensionAddress - nothing

	// callIdentifier - 16 byte
	unsigned char CallIdentifier[CONFERENCEID_LENGTH];
	DecodeDialogID(info->GetCallIdentifier().c_str(), CallIdentifier);
	VS_PerBuffer buf_callid(CallIdentifier, CONFERENCEID_LENGTH*8);

	setup->callIdentifier.guid.filled = true;
	setup->callIdentifier.filled = true;
	setup->callIdentifier.guid.value =  buf_callid;

	setup->mediaWaitForConnect.value = false;
	setup->mediaWaitForConnect.filled = true;
	setup->canOverlapSend.value = false;
	setup->canOverlapSend.filled = true;

	info->h235_auth.PrepareTokens(setup->tokens);

	setup->filled = true;


	VS_CsH323UserInformation   ui;
	ui.h323UuPdu.h323MessageBody = setup;
	ui.h323UuPdu.nonStandardData.filled = false;
	ui.h323UuPdu.filled = true;

	ui.h323UuPdu.provisionalRespToH245Tunneling.filled = false;

	ui.h323UuPdu.h245Tunneling.value = false;
	ui.h323UuPdu.h245Tunneling.filled = true;

	ui.filled = true;
	VS_PerBuffer ui_buffer;
	if ( !ui.Encode(ui_buffer) )
		return false;

	const unsigned int crv = rand();
	info->SetCRV( crv );

	VS_PerBuffer OutBuffer;
	VS_Q931 q931;
	q931.fromDestination = 0;
	q931.callReference = crv;
	q931.messageType = VS_Q931::e_setupMsg;

	if ( !q931.EncodeMHeader(OutBuffer) )
		return false;

	//unsigned int dn_sz = 0;

	const std::string &dn = info->GetDisplayNameMy();

	unsigned char* num = nullptr;
	//unsigned int num_sz = 0;

// 	num = (unsigned char*) info->GetDstDigit();
// 	if (!num)
// 	{
// 		return false;
// 	}

	int8_t rate_multiplier = info->GetConfig().h323.Q931_rate_multiplier.get_value_or(0);

	if ( !VS_Q931::SetUserUserIE(OutBuffer, ui_buffer, &rate_multiplier, true, (const unsigned char *)dn.c_str()/*, num*/) )
	{
		return false;
	}

	void* data = OutBuffer.GetData();
	const std::size_t sz = OutBuffer.ByteSize();

	auto out = vs::make_unique_default_init<unsigned char[]>(sz);
	memcpy(out.get(), data, sz);

	return PutOutputMessage(std::move(out), sz, e_H225);
	}

bool VS_H323Parser::MakeH225Alerting(string_view dialogId)
{
	std::shared_ptr<VS_H323ParserInfo> ctx = GetParserContext(dialogId);
	if (!ctx)
		return false;

	std::unique_ptr<VS_CsAlertingUuie> alerting = vs::make_unique<VS_CsAlertingUuie>();
	alerting->filled = true;
	alerting->protocolIdentifier = h225ProtocolIdentifier;
	alerting->destinationInfo.terminalInfo.filled = true;
	alerting->destinationInfo.mc.value = false;
	alerting->destinationInfo.mc.filled = true;
	alerting->destinationInfo.undefinedNode.value = false;
	alerting->destinationInfo.undefinedNode.filled = true;
	alerting->destinationInfo.filled = true;
	{
		//char* dialog_id = ctx->GetDialogID();

		//unsigned char* call_ident = new unsigned char[16];
		//if ( !m_mgr->GetCallInfo()->GetH323CallInfoInterface()->GetCallIdentifier(call_ident) )
		//{
		//	if (alerting) { delete alerting; alerting = 0; }
		//	return false;
		//}
		//VS_BitBuffer callIdentifier(call_ident, 16*8);
		//if (call_ident) { delete call_ident; call_ident = 0; }

		//VS_BitBuffer callIdentifier(dialog_id, strlen(dialog_id)*8);
		unsigned char raw_call_id[CONFERENCEID_LENGTH];
		DecodeDialogID(ctx->GetCallIdentifier().c_str(), raw_call_id);
		VS_BitBuffer call_identifier(raw_call_id, CONFERENCEID_LENGTH * 8);

		alerting->callIdentifier.guid.value = call_identifier;
		alerting->callIdentifier.guid.filled = true;
		alerting->callIdentifier.filled = true;
	}

	VS_CsH323UserInformation ui{};
	ui.h323UuPdu.h323MessageBody = alerting.release();
	ui.h323UuPdu.filled = true;
	ui.filled = true;

	VS_PerBuffer ui_buffer;
	if ( !ui.Encode( ui_buffer ) )
	{
		return false;
	}

	VS_Q931 q931;
	q931.messageType = VS_Q931::e_alertingMsg;
	q931.fromDestination = true;
	q931.callReference = ctx->GetCRV();

	VS_BitBuffer out;
	if ( !q931.EncodeMHeader(out) )
	{
		return false;
	}

	if ( !VS_Q931::SetUserUserIE( out, ui_buffer ) )
	{
		return false;
	}

	void* data = out.GetData();
	const std::size_t data_sz = out.ByteSize();

	auto odata = vs::make_unique_default_init<unsigned char[]>(data_sz);
	memcpy(odata.get(), data, data_sz);

	return PutOutputMessage(std::move(odata), data_sz, e_H225);
	}

bool VS_H323Parser::MakeH225Connect(string_view dialogId)
{
	std::shared_ptr<VS_H323ParserInfo> ctx = GetParserContext(dialogId);
	if (!ctx)
		return false;

	if (!InitH245(ctx, {}, 0))
		return false;

	auto channel = ctx->GetH245Channel();
	assert(channel);

	VS_CsConnectUuie* connect = new VS_CsConnectUuie;
	connect->protocolIdentifier = h225ProtocolIdentifier;
	ctx->SetMyCsAddress(ctx->GetMyCsAddress(), channel->LocalPort());

	set_ip_address(connect->h245Address, ctx->GetMyCsAddress(), ctx->GetMyCsPort());

	connect->destinationInfo.terminalInfo.filled = true;
	connect->destinationInfo.mc.value = false;
	connect->destinationInfo.mc.filled = true;
	connect->destinationInfo.undefinedNode.value = false;
	connect->destinationInfo.undefinedNode.filled = true;

	ctx->h235_auth.PrepareTokens(connect->tokens);

	connect->destinationInfo.vendor.productId.value.AddBits(H323Gateway_Product, (sizeof(H323Gateway_Product) - 1) * 8);
	connect->destinationInfo.vendor.productId.filled = true;

	connect->destinationInfo.vendor.versionId.value.AddBits(H323Gateway_Version, (sizeof(H323Gateway_Version) - 1) * 8);
	connect->destinationInfo.vendor.versionId.filled = true;

	connect->destinationInfo.vendor.filled =
		connect->destinationInfo.vendor.versionId.filled
		&
		connect->destinationInfo.vendor.productId.filled ;

	connect->destinationInfo.vendor.h221NonStandard.manufacturerCode.value = 12000;
	connect->destinationInfo.vendor.h221NonStandard.manufacturerCode.filled = true;
	connect->destinationInfo.vendor.h221NonStandard.t35CountryCode.value = T35_COUNTRY_CODE_RUSSIA;
	connect->destinationInfo.vendor.h221NonStandard.t35CountryCode.filled = true;
	connect->destinationInfo.vendor.h221NonStandard.t35Extension.value   = 14;
	connect->destinationInfo.vendor.h221NonStandard.t35Extension.filled = true;
	connect->destinationInfo.vendor.h221NonStandard.filled = true;
	connect->destinationInfo.filled = true;
	{
		//char* dialog_id = ctx->GetDialogID();

		//unsigned char* call_ident = new unsigned char[16];
		//if ( !m_mgr->GetCallInfo()->GetH323CallInfoInterface()->GetCallIdentifier(call_ident) )
		//{
		//	if (connect) { delete connect; connect = 0; }
		//	return false;
		//}
		//VS_BitBuffer callIdentifier(call_ident, 16*8);
		//if (call_ident) { delete call_ident; call_ident = 0; }

		unsigned char raw_call_id[CONFERENCEID_LENGTH];
		DecodeDialogID(ctx->GetCallIdentifier().c_str(), raw_call_id);
		VS_BitBuffer callIdentifier(raw_call_id, CONFERENCEID_LENGTH * 8);
		connect->callIdentifier.guid.value = callIdentifier;
		connect->callIdentifier.guid.filled = true;
		connect->callIdentifier.filled = true;

		unsigned char raw_conference_id[CONFERENCEID_LENGTH];
		DecodeDialogID(ctx->GetConferenceID().c_str(), raw_conference_id);
		VS_BitBuffer conferenceID(raw_conference_id, CONFERENCEID_LENGTH * 8);
		connect->conferenceID.value = conferenceID;
		connect->conferenceID.filled = true;
	}
	connect->filled = true;

	VS_CsH323UserInformation ui;
	ui.h323UuPdu.h323MessageBody = connect;
	ui.h323UuPdu.filled = true;
	ui.filled = true;

	VS_PerBuffer ui_buffer;
	if ( !ui.Encode( ui_buffer ) )
	{
		return false;
	}

	VS_Q931 q931;
	q931.messageType = VS_Q931::e_connectMsg;
	q931.fromDestination = true;
	q931.callReference = ctx->GetCRV();

	VS_BitBuffer out;
	if ( !q931.EncodeMHeader(out) )
	{
		return false;
	}

	int8_t rate_multiplier = ctx->GetConfig().h323.Q931_rate_multiplier.get_value_or(0);

	if ( !VS_Q931::SetUserUserIE( out, ui_buffer, &rate_multiplier, true, (const unsigned char *) ctx->GetDisplayNameMy().c_str() ) )
	{
		return false;
	}

	void* data = out.GetData();
	const auto data_sz = out.ByteSize();

	auto odata = vs::make_unique_default_init<unsigned char[]>(data_sz);
	memcpy(odata.get(), data, data_sz);

	return PutOutputMessage(std::move(odata), data_sz, e_H225);
	}

bool VS_H323Parser::CreateOLCs(const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245Data* h245Params = ctx->GetH245Params();


	if (!ctx->CreateH245LogicalChannel(e_audio, h245Params->m_audioNumberLCSender,true))
	{
		TerminateSession(ctx);///TERMINATE
		return false;
	}

	if (!SendOLC(h245Params->m_audioNumberLCSender, ctx))
		return false;

#ifndef H323_SJPHONE_HOTFIX
	if (ctx->HasVCDefault())
	{
		if (!ctx->CreateH245LogicalChannel(e_video, h245Params->m_videoNumberLCSender,true))
		{
			TerminateSession(ctx);///TERMINATE
			return false;
		}

		if (!SendOLC(h245Params->m_videoNumberLCSender, ctx))
			return false;
	}
	else
#endif
	{
		h245Params->H245Flags[VS_H323ParserInfo::e_olc_video] |= h245Params->h245_rsp_rejected;
	}

	if (ctx->IsH239Enabled() && ctx->IsH239CapabilityPresent())
	{
		if (!ctx->CreateH245LogicalChannel(e_slides, h245Params->m_slidesNumberLCSender, true))
		{
			TerminateSession(ctx);///TERMINATE
			return false;
		}

		if (!SendOLC(h245Params->m_slidesNumberLCSender, ctx))
			return false;
	}

	if (ctx->IsH224Enabled() && ctx->IsH224CapabilityPresent())
	{
		if (!ctx->CreateH245LogicalChannel(e_data, h245Params->m_dataNumberLCSender, true))
		{
			TerminateSession(ctx);
			return false;
		}

		if (!SendOLC(h245Params->m_dataNumberLCSender, ctx))
			return false;
	}

	return true;
}


bool VS_H323Parser::SendTCS(const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245Data* h245params = ctx->GetH245Params();
	unsigned h245_version = ctx->GetPeerH245Version();

	if (h245params->H245Flags[VS_H323ParserInfo::e_tcs] & h245params->h245_req_send)
		return true;

	////////////////////////////////////////////////////////////////
	unsigned char   preset_mc[] = { 0x80, 0x0D, 0x00, 0x00, 0x3C, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00 };
	unsigned int  preset_mc_sz = sizeof(preset_mc);
	VS_H245MultimediaSystemControlMessage   mscm;
	VS_H245RequestMessage   *rm = new VS_H245RequestMessage;
	VS_H245TerminalCapabilitySet   *tcs = new VS_H245TerminalCapabilitySet;
	tcs->sequenceNumber.value = ++h245params->m_SequenceNumber;
	tcs->sequenceNumber.filled = true;
	tcs->protocolIdentifier = h245ProtocolIdentifier;
	VS_PerBuffer   buffer( preset_mc, preset_mc_sz * 8 );
	if (!tcs->multiplexCapability.Decode( buffer ))
	{
		return false;
	}
	VS_H245H2250Capability * cap =
		static_cast<VS_H245H2250Capability*>(tcs->multiplexCapability.choice);

	unsigned pts_total = 0;
	VS_H245RTPPayloadType*  rtp_pt = new VS_H245RTPPayloadType[2/*H.264*/ + 1/*G.722.1*/];

	// H.264 AnnexA
	{
		const unsigned pt_id = pts_total;
		unsigned int oid[] = {0,0,8,241,0,0,0,0};
		rtp_pt[pt_id].payloadDescriptor.choice = new VS_AsnObjectId(oid, sizeof(oid) / sizeof(oid[0]));
		rtp_pt[pt_id].payloadDescriptor.tag = VS_H245RTPPayloadType_PayloadDescriptor::e_oid;
		rtp_pt[pt_id].payloadDescriptor.filled = true;
		rtp_pt[pt_id].filled = true;
		++pts_total;
	}

	// H.264 NonInterleaved
	{
		const unsigned pt_id = pts_total;
		unsigned int oid[] = {0,0,8,241,0,0,0,1};
		rtp_pt[pt_id].payloadDescriptor.choice = new VS_AsnObjectId(oid, sizeof(oid) / sizeof(oid[0]));
		rtp_pt[pt_id].payloadDescriptor.tag = VS_H245RTPPayloadType_PayloadDescriptor::e_oid;
		rtp_pt[pt_id].payloadDescriptor.filled = true;
		rtp_pt[pt_id].filled = true;
		++pts_total;
	}

	// G.722.1
	if (OPTIONAL_DATA_EXIST)
	{
		const unsigned pt_id = pts_total;
		unsigned int oid[] = {0,0,7,7221,1,0};
		rtp_pt[pt_id].payloadDescriptor.choice = new VS_AsnObjectId(oid, sizeof(oid) / sizeof(oid[0]));
		rtp_pt[pt_id].payloadDescriptor.tag = VS_H245RTPPayloadType_PayloadDescriptor::e_oid;
		rtp_pt[pt_id].payloadDescriptor.filled = true;
		rtp_pt[pt_id].payloadType.value = 99;
		rtp_pt[pt_id].payloadType.filled = true;
		rtp_pt[pt_id].filled = true;
		++pts_total;
	}

	cap->mediaPacketizationCapability.h261aVideoPacketization.value = false;
	cap->mediaPacketizationCapability.h261aVideoPacketization.filled = true;
	cap->mediaPacketizationCapability.rtpPayloadType.reset(rtp_pt, pts_total);
	cap->mediaPacketizationCapability.filled = true;

	////////////////////////////////////////////////////////////////
	// WHOLE
	////////////////////////////////////////////////////////////////
	unsigned caps_total = 0;
	unsigned caps_audio = 0;
	unsigned caps_video = 0;
	unsigned caps_slides = 0;
	unsigned alt_caps_total = 0;
	const unsigned capabilityTableEntrySize = 2* (ac_number + vc_number + vc_number) + dc_number;					// (audio, video, slides) + secure analogs, h224
	VS_H245CapabilityTableEntry   *capabilityTableEntry = new VS_H245CapabilityTableEntry[capabilityTableEntrySize];
	VS_H245AlternativeCapabilitySet   *alternativeCapabilitySet = new VS_H245AlternativeCapabilitySet[4 + 1];		// audio, video, extra channel control (h239ControlCapability), slides, h224
	VS_H245CapabilityTableEntryNumber* audio_cap_numbers = new VS_H245CapabilityTableEntryNumber[ac_number * 2];	// audio + secure analogs
	VS_H245CapabilityTableEntryNumber* video_cap_numbers = new VS_H245CapabilityTableEntryNumber[vc_number * 2];	// video + secure analogs
	VS_H245CapabilityTableEntryNumber* slides_cap_numbers = new VS_H245CapabilityTableEntryNumber[vc_number * 2];	// slides + secure analogs
	VS_H245CapabilityTableEntryNumber* data_cap_numbers = new VS_H245CapabilityTableEntryNumber[dc_number * 2];		// data + secure analogs
	auto& h235_auth = ctx->h235_auth;

	// h239ControlCapability
	if (ctx->IsH239Enabled())
	{
		const unsigned cap_id = caps_total; // index in capabilityTable for this capability
		VS_H245GenericCapability* cc = new VS_H245GenericCapability;
		SetH245CapabilityIdentifier(cc->capabilityIdentifier, oid_h239_cc);
		cc->filled = true;

		capabilityTableEntry[cap_id].capability.choice = cc;
		capabilityTableEntry[cap_id].capability.tag = VS_H245Capability::e_genericControlCapability;
		capabilityTableEntry[cap_id].capability.filled = true;

		capabilityTableEntry[cap_id].capabilityTableEntryNumber.value = cap_id + 1;
		capabilityTableEntry[cap_id].capabilityTableEntryNumber.filled = true;

		capabilityTableEntry[cap_id].filled = true;
		++caps_total;

		const unsigned alt_cap_id = alt_caps_total; // index in alternativeCapabilitySet for extra channel control capabilities
		alternativeCapabilitySet[alt_cap_id].reset(new VS_H245CapabilityTableEntryNumber[1], 1);
		alternativeCapabilitySet[alt_cap_id][0].value = cap_id + 1;
		alternativeCapabilitySet[alt_cap_id][0].filled = true;
		alternativeCapabilitySet[alt_cap_id].filled = true;
		++alt_caps_total;
	}

	////////////////////////////////////////////////////////////////
	// Audio + secure Audio
	////////////////////////////////////////////////////////////////
	VS_H245AudioCapability** ac = ctx->GetAudioCapability();
	for (unsigned i = 0; i < ac_number; ++i)
	{
		const auto& our_ac = ac[ac_number - i - 1];

		if (!our_ac->filled)
			continue;
		if (h245_version < 5 && (our_ac->tag == VS_H245AudioCapability::e_genericAudioCapability)) // bug#24745: exclude g722.1 both 24 and 32 kHz
			continue;

		const unsigned cap_id = caps_total; // index in capabilityTable for this capability
		const unsigned cap_audio_id = caps_audio; // index in corresponding AlternativeCapabilitySet for this capability
		////////////////////////////////////////////////////////////
		capabilityTableEntry[cap_id].capability.tag = VS_H245Capability::e_receiveAudioCapability;
		//VS_H245Capability::e_receiveAudioCapability;
		capabilityTableEntry[cap_id].capability.filled = true;
		VS_H245AudioCapability * ac_temp = new VS_H245AudioCapability;
		*(ac_temp) = *(our_ac);
		capabilityTableEntry[cap_id].capability.choice = ac_temp;
#ifdef _LOG_DEBUG_
		ac_temp->Show();
#endif
		////////////////////////////////////////////////////////////
		capabilityTableEntry[cap_id].capabilityTableEntryNumber.value = cap_id + 1;
		capabilityTableEntry[cap_id].capabilityTableEntryNumber.filled = true;
		////////////////////////////////////////////////////////////
		capabilityTableEntry[cap_id].filled = true;
		////////////////////////////////////////////////////////////
		const unsigned &capabilityTableEntryNumber = capabilityTableEntry[cap_id].capabilityTableEntryNumber.value;
		audio_cap_numbers[cap_audio_id].value = capabilityTableEntryNumber;
		audio_cap_numbers[cap_audio_id].filled = true;
		////////////////////////////////////////////////////////////
		++caps_total;
		++caps_audio;

		if (h235_auth.AddSecureCapability(capabilityTableEntry[caps_total], caps_total + 1, capabilityTableEntryNumber)) {
			AddSecureToAlternative(audio_cap_numbers, caps_total, caps_audio);
		}
		////////////////////////////////////////////////////////////
	}
	{
		const unsigned alt_cap_id = alt_caps_total; // index in alternativeCapabilitySet for audio capabilities
		alternativeCapabilitySet[alt_cap_id].reset(audio_cap_numbers, caps_audio);
		++alt_caps_total;
	}
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	// Video + secure Video
	////////////////////////////////////////////////////////////////
	VS_H245VideoCapability** vc = ctx->GetVideoCapability();
	for (unsigned i = 0; i < vc_number; ++i)
	{
		const auto& our_vc = vc[vc_number - i - 1];

		if (!our_vc->filled)
			continue;
		if (h245_version < 5 && (our_vc->tag == VS_H245VideoCapability::e_genericVideoCapability)) // bug#24745: exclude h264
			continue;

		const unsigned cap_id = caps_total; // index in capabilityTable for this capability
		const unsigned cap_video_id = caps_video; // index in corresponding AlternativeCapabilitySet for this capability
		//////////////////////////////////////////////////////////////
		capabilityTableEntry[cap_id].capability.tag = VS_H245Capability::e_receiveVideoCapability;
		//VS_H245Capability::e_receiveVideoCapability;
		capabilityTableEntry[cap_id].capability.filled = true;
		VS_H245VideoCapability * vc_temp = new VS_H245VideoCapability;
		*(vc_temp) = *(our_vc);

		capabilityTableEntry[cap_id].capability.choice = vc_temp;
		////////////////////////////////////////////////////////////
		capabilityTableEntry[cap_id].capabilityTableEntryNumber.value = cap_id + 1;
		capabilityTableEntry[cap_id].capabilityTableEntryNumber.filled = true;
		////////////////////////////////////////////////////////////
		capabilityTableEntry[cap_id].filled = true;
		////////////////////////////////////////////////////////////
		const unsigned &capabilityTableEntryNumber = capabilityTableEntry[cap_id].capabilityTableEntryNumber.value;
		video_cap_numbers[cap_video_id].value = capabilityTableEntryNumber;
		video_cap_numbers[cap_video_id].filled = true;
		////////////////////////////////////////////////////////////
		++caps_total;
		++caps_video;

		if (h235_auth.AddSecureCapability(capabilityTableEntry[caps_total], caps_total + 1, capabilityTableEntryNumber)) {
			AddSecureToAlternative(video_cap_numbers, caps_total, caps_video);
		}
		////////////////////////////////////////////////////////////
	}
	{
		const unsigned alt_cap_id = alt_caps_total; // index in alternativeCapabilitySet for video capabilities
		alternativeCapabilitySet[alt_cap_id].reset(video_cap_numbers, caps_video);
		++alt_caps_total;
	}
	////////////////////////////////////////////////////////////////

	// Slides + secure Slides
	if (ctx->IsH239Enabled())
	{
		VS_H245VideoCapability** vc = ctx->GetVideoCapability();
		for (unsigned i = 0; i < vc_number; ++i)
		{
			const auto& our_vc = vc[vc_number - i - 1];

			if (!our_vc->filled)
				continue;
			if (h245_version < 5 && (our_vc->tag == VS_H245VideoCapability::e_genericVideoCapability)) // bug#24745: exclude h264
				continue;

			const unsigned cap_id = caps_total; // index in capabilityTable for this capability
			const unsigned cap_slides_id = caps_slides; // index in corresponding AlternativeCapabilitySet for this capability
			VS_H245VideoCapability* vc_temp = new VS_H245VideoCapability;
			VS_H245ExtendedVideoCapability* evc = new VS_H245ExtendedVideoCapability;

			evc->videoCapability.reset(new VS_H245VideoCapability[1], 1);
			evc->videoCapability[0] = *our_vc;

			evc->videoCapabilityExtension.reset(new VS_H245GenericCapability[1],1); // h239ExtendedVideoCapability
			SetH245CapabilityIdentifier(evc->videoCapabilityExtension[0].capabilityIdentifier, oid_h239_evc);

			evc->videoCapabilityExtension[0].collapsing.reset(new VS_H245GenericParameter[1], 1); // roleLabel
			SetH245ParameterValue_booleanArray(evc->videoCapabilityExtension[0].collapsing[0], 1, 1); // Presentation role
			evc->videoCapabilityExtension[0].collapsing.filled = true;

			evc->videoCapabilityExtension[0].filled = true;
			evc->videoCapabilityExtension.filled = true;
			evc->filled = true;

			vc_temp->choice = evc;
			vc_temp->tag = VS_H245VideoCapability::e_extendedVideoCapability;
			vc_temp->filled = true;

			capabilityTableEntry[cap_id].capability.choice = vc_temp;
			capabilityTableEntry[cap_id].capability.tag = VS_H245Capability::e_receiveVideoCapability;
			capabilityTableEntry[cap_id].capability.filled = true;
			capabilityTableEntry[cap_id].capabilityTableEntryNumber.value = cap_id + 1;
			capabilityTableEntry[cap_id].capabilityTableEntryNumber.filled = true;
			capabilityTableEntry[cap_id].filled = true;

			const unsigned &capabilityTableEntryNumber = capabilityTableEntry[cap_id].capabilityTableEntryNumber.value;
			slides_cap_numbers[cap_slides_id].value = capabilityTableEntryNumber;
			slides_cap_numbers[cap_slides_id].filled = true;

			++caps_total;
			++caps_slides;

			if (h235_auth.AddSecureCapability(capabilityTableEntry[caps_total], caps_total + 1, capabilityTableEntryNumber)) {
				AddSecureToAlternative(slides_cap_numbers, caps_total, caps_slides);
			}
		}
		const unsigned alt_cap_id = alt_caps_total; // index in alternativeCapabilitySet for slides capabilities
		alternativeCapabilitySet[alt_cap_id].reset(slides_cap_numbers, caps_slides);
		++alt_caps_total;
	}

	// H224
	if (ctx->IsH224Enabled())
	{
		unsigned caps_data = 0;
		// DataProtocolCapability
		{
			const unsigned cap_id = caps_total; // index in capabilityTable for this capability

			VS_H245DataProtocolCapability* dpc = new VS_H245DataProtocolCapability;
			dpc->choice = new VS_AsnNull;
			dpc->tag = VS_H245DataProtocolCapability::e_hdlcFrameTunnelling;
			dpc->filled = true;

			VS_H245DataApplicationCapability *dac = new VS_H245DataApplicationCapability;
			dac->application.choice = dpc;
			dac->application.tag = VS_H245DataApplicationCapability_Application::e_h224;
			dac->application.filled = true;
			dac->maxBitRate.value = 50;
			dac->maxBitRate.filled = true;
			dac->filled = true;

			capabilityTableEntry[cap_id].capability.choice = dac;
			capabilityTableEntry[cap_id].capability.tag = VS_H245Capability::e_receiveAndTransmitDataApplicationCapability;
			capabilityTableEntry[cap_id].capability.filled = true;

			capabilityTableEntry[cap_id].capabilityTableEntryNumber.value = cap_id + 1;
			capabilityTableEntry[cap_id].capabilityTableEntryNumber.filled = true;
			capabilityTableEntry[cap_id].filled = true;
			++caps_total;

			const unsigned& capabilityTableEntryNumber = capabilityTableEntry[cap_id].capabilityTableEntryNumber.value;
			data_cap_numbers[caps_data].value = capabilityTableEntryNumber;
			data_cap_numbers[caps_data].filled = true;
			++caps_data;

			if (h235_auth.AddSecureCapability(capabilityTableEntry[caps_total], caps_total + 1, capabilityTableEntryNumber)) {
				AddSecureToAlternative(data_cap_numbers, caps_total, caps_data);
			}
		}
		// GenericDataCapability
		{
			const unsigned cap_id = caps_total; // index in capabilityTable for this capability

			VS_H245GenericCapability* gdc = new VS_H245GenericCapability;
			SetH245CapabilityIdentifier(gdc->capabilityIdentifier, oid_h224_gc);
			gdc->filled = true;

			VS_H245DataApplicationCapability *dac = new VS_H245DataApplicationCapability;
			dac->application.choice = gdc;
			dac->application.tag = VS_H245DataApplicationCapability_Application::e_genericDataCapability;
			dac->application.filled = true;
			dac->maxBitRate.value = 50;
			dac->maxBitRate.filled = true;
			dac->filled = true;

			capabilityTableEntry[cap_id].capability.choice = dac;
			capabilityTableEntry[cap_id].capability.tag = VS_H245Capability::e_receiveAndTransmitDataApplicationCapability;
			capabilityTableEntry[cap_id].capability.filled = true;

			capabilityTableEntry[cap_id].capabilityTableEntryNumber.value = cap_id + 1;
			capabilityTableEntry[cap_id].capabilityTableEntryNumber.filled = true;
			capabilityTableEntry[cap_id].filled = true;
			++caps_total;

			const unsigned& capabilityTableEntryNumber = capabilityTableEntry[cap_id].capabilityTableEntryNumber.value;
			data_cap_numbers[caps_data].value = capabilityTableEntryNumber;
			data_cap_numbers[caps_data].filled = true;

			if (h235_auth.AddSecureCapability(capabilityTableEntry[caps_total], caps_total + 1, capabilityTableEntryNumber)) {
				AddSecureToAlternative(data_cap_numbers, caps_total, caps_data);
			}

			data_cap_numbers->filled = true;
		}

		{
			const unsigned alt_cap_id = alt_caps_total; // index in alternativeCapabilitySet for extra channel control capabilities
			alternativeCapabilitySet[alt_cap_id].reset(data_cap_numbers, caps_data);
			++alt_caps_total;
		}
	}
	else { // IsH224Enabled() == false
		delete[] data_cap_numbers;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// WHOLE
	//////////////////////////////////////////////////////////////////////////////////
	tcs->capabilityTable.reset(capabilityTableEntry, caps_total);
	tcs->capabilityTable.filled = true;

	tcs->capabilityDescriptors.reset(new VS_H245CapabilityDescriptor[1], 1);
	tcs->capabilityDescriptors[0].capabilityDescriptorNumber.value = 0;
	tcs->capabilityDescriptors[0].capabilityDescriptorNumber.filled = true;
	tcs->capabilityDescriptors[0].simultaneousCapabilities.reset(alternativeCapabilitySet, alt_caps_total);
	tcs->capabilityDescriptors[0].simultaneousCapabilities.filled = true;
	tcs->capabilityDescriptors[0].filled = true;
	tcs->capabilityDescriptors.filled = true;

	tcs->filled = true;
	*rm = tcs;		mscm = rm;
	if (PutH245Message(mscm, ctx))
	{
		h245params->H245Flags[VS_H323ParserInfo::e_tcs] |= h245params->h245_req_send;
		ctx->SetTCSWaitingTick(NULL_TICK);
		return true;
	}
	return false;
}

bool VS_H323Parser::SendMSD(const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245Data* h245params = ctx->GetH245Params();

	if (h245params->H245Flags[VS_H323ParserInfo::e_msd] & h245params->h245_req_send)
		return true;
	if (h245params->H245Flags[VS_H323ParserInfo::e_msd] & h245params->h245_req_recv)
		return true;

	VS_H245MultimediaSystemControlMessage   mscm;
	VS_H245RequestMessage   *rm = new VS_H245RequestMessage;
	VS_H245MasterSlaveDetermination   *msd = new VS_H245MasterSlaveDetermination;

	msd->terminalType.value = h245params->m_my_msd_type; ///< 60 - Gateways! 50 - Terminals. Look H.323 document.
	msd->terminalType.filled = true;
	msd->statusDeterminationNumber.value = 	h245params->m_my_msd_num;
	msd->statusDeterminationNumber.filled = true;
	msd->filled = true;
	*rm = msd;		mscm = rm;
	if (PutH245Message(mscm, ctx))
	{
		dprint2("VS_H323Parser: MSD sent, type=%d num=%d\n", msd->terminalType.value, msd->statusDeterminationNumber.value);
		h245params->H245Flags[VS_H323ParserInfo::e_msd] |= h245params->h245_req_send;
		ctx->StopWaitingMSD();
		return true;
	}
	return false;
}

bool VS_H323Parser::CheckTCS(const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245Data* h245params = ctx->GetH245Params();

	bool req_send = (h245params->H245Flags[VS_H323ParserInfo::e_tcs] & h245params->h245_req_send)!=0;
	bool rsp_send = (h245params->H245Flags[VS_H323ParserInfo::e_tcs] & h245params->h245_rsp_send)!=0;
	bool req_recv = (h245params->H245Flags[VS_H323ParserInfo::e_tcs] & h245params->h245_req_recv)!=0;
	bool rsp_recv = (h245params->H245Flags[VS_H323ParserInfo::e_tcs] & h245params->h245_rsp_recv)!=0;

	bool res = req_send && rsp_send && req_recv && rsp_recv;

	return res;
}

bool VS_H323Parser::CheckMSD(const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245Data* h245params = ctx->GetH245Params();

	if (h245params->m_msd_mode == VS_H323ParserInfo::MSDSkip)
	{
		return true;
	}

	bool req_send = (h245params->H245Flags[VS_H323ParserInfo::e_msd] & h245params->h245_req_send)!=0;
	bool rsp_send = (h245params->H245Flags[VS_H323ParserInfo::e_msd] & h245params->h245_rsp_send)!=0;
	bool req_recv = (h245params->H245Flags[VS_H323ParserInfo::e_msd] & h245params->h245_req_recv)!=0;
	bool rsp_recv = (h245params->H245Flags[VS_H323ParserInfo::e_msd] & h245params->h245_rsp_recv)!=0;


	bool res = false;
	if ((req_send || req_recv) && rsp_send && rsp_recv)
		res = true;

	return res;
}

bool VS_H323Parser::InitH245(const std::shared_ptr<VS_H323ParserInfo> &ctx, const net::address &remoteAddr, net::port remotePort)
{
	// initialize media parameters in the parser context
	{
		auto &&call_config = ctx->GetConfig();

		if (!ctx->InitH245Params(call_config.Codecs,
			call_config.Bandwidth.get_value_or(0),
			call_config.h323.ConventionalSirenTCE.get_value_or(false),
			call_config.h323.EnableH263plus2.get_value_or(false)))
			return false;
	}

	auto channel = ctx->GetH245Channel();
	if (!channel)
	{
		channel = VS_SignalChannel::Create(m_strand.get_io_service(), m_logger);
		std::weak_ptr<VS_H323ParserInfo> weak_ctx(ctx);
		channel->ConnectToDataReceived([this, weak_ctx](const void* data, size_t size) {
			vs::SharedBuffer data_tmp(size);
			::memcpy(data_tmp.data(), data, size);
			m_strand.post([this, weak_ctx, data_tmp = std::move(data_tmp)]() {
				if (auto ctx = weak_ctx.lock())
					OnH245RawData(data_tmp.data<const void>(), data_tmp.size(), ctx);
			});
		});
		ctx->SetH245Channel(channel);
	}

	std::uint32_t channel_flags(VS_SignalChannel::LISTEN_TCP | VS_SignalChannel::CONNECT_TCP);
	auto local_addr = ctx->GetMyLocalCsAddress();
	net::port port = channel->LocalPort();

	auto flow = net::QoSSettings::GetInstance().GetH323QoSFlow(false, remoteAddr.is_v6());
	if (port == 0)
	{
		VS_RegistryKey key(false, CONFIGURATION_KEY);
		int32_t minPort(0), maxPort(0);
		key.GetValue(&minPort, sizeof(minPort), VS_REG_INTEGER_VT, "H245 MinPort");
		key.GetValue(&maxPort, sizeof(maxPort), VS_REG_INTEGER_VT, "H245 MaxPort");
		if (!minPort || !maxPort || minPort > maxPort)
		{
			minPort = DEFAULT_H245_MINPORT;
			maxPort = DEFAULT_H245_MAXPORT;
		}
		static std::atomic<net::port> lastUsedPort{ 0 };
		int32_t attempt;
		for (attempt = 0; attempt < maxPort - minPort + 1; ++attempt)
		{
//			Obtain a free port
			auto prevPort = lastUsedPort.load(std::memory_order_relaxed);// load last
			do
			{
				port = prevPort + 1;// set next
				if (port < minPort || port > maxPort) // check borders
					port = minPort;
			} while (!lastUsedPort.compare_exchange_weak(prevPort, port, std::memory_order_relaxed)); // check if someone else hasn't changed the value{
			if (channel->Open(channel_flags, net::address_v4::any(), port, remoteAddr, remotePort, flow))
				break;
		}
		if (attempt == maxPort - minPort + 1)
			return false;
	}
	else
		if (!channel->Open(channel_flags, local_addr, port, remoteAddr, remotePort, flow))
			return false;

	auto dialog_id = ctx->GetDialogID();

	if (ctx->UseNAT()) {
		const auto &cfg = ctx->GetConfig();
		bool nat_addr = false;
		if (!cfg.h323.ExternalAddress.is_unspecified())
		{
			if (cfg.h323.ExternalAddressScheme == VS_CallConfig::eExternalAddressScheme::ALWAYS_EXTERNAL)
				nat_addr = true;
			else if (cfg.h323.ExternalAddressScheme == VS_CallConfig::eExternalAddressScheme::ONLY_INTERNET_ADDRESS) {
				if (!net::is_private_address(cfg.Address.addr))
					nat_addr = true;
			}
		}
		if (nat_addr && !cfg.h323.ExternalAddress.is_unspecified())
		{
			dstream4 << "OnSetupArrived NAT for [" << dialog_id << ":" << ctx->GetMyExternalCsAddress() << ":" << ctx->GetMyCsPort() << "]";
		}
		else
		{
			ctx->SetMyExternalCsAddress({});
			dstream4 << "OnSetupArrived NAT for [" << dialog_id << "] is not used";
		}
	}
	else {
		dstream4 << "OnSetupArrived NAT for [" << dialog_id << "] is disabled";
	}

	if (ctx->GenNewMSDNums()) {

		ctx->StartWaitingMSD();
	}
	return true;
}

bool VS_H323Parser::RecvMSD(VS_H245MasterSlaveDetermination * msd, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (!msd)
		return false;

	VS_H245Data* h245params = ctx->GetH245Params();
	MSDState state = ctx->MsdState();
	if (state == MSDIdle)
	{
	h245params->m_their_msd_type =  msd->terminalType.value;
	h245params->m_their_msd_num = msd->statusDeterminationNumber.value;

		int decision = DetermineMSDStatus(ctx);
		if (decision == VS_H245MasterSlaveDeterminationAck_Decision::e_indeterminate)
		{
			return SendMSDR(ctx, VS_H245MasterSlaveDeterminationReject_Cause::e_identicalNumbers);
		}
		h245params->H245Flags[VS_H323ParserInfo::e_msd] |= VS_H245Data::h245_req_recv;
			ctx->StopWaitingMSD();
		ctx->SetMsdState(MSDIncomingAwaitingResponse);
			ctx->StartMSDTimer();
		if (!(h245params->H245Flags[VS_H323ParserInfo::e_msd] & VS_H245Data::h245_req_send))
				return SendMSDA(ctx, decision);
	} else
	if (state == MSDIncomingAwaitingResponse)
{
		dprint3("VS_H323Parser::RecvMSD: MSDError (C) inappropriate MSD message (MSDAck expected)\n");
		ctx->StopMSDTimer();
		ctx->SetMsdState(MSDIdle);
		TerminateSession(ctx);
		return false;
	} else
	if (state == MSDOutgoingAwaitingResponse)
	{
		h245params->m_their_msd_type = msd->terminalType.value;
		h245params->m_their_msd_num = msd->statusDeterminationNumber.value;

		int decision = DetermineMSDStatus(ctx);
		if (decision == VS_H245MasterSlaveDeterminationAck_Decision::e_indeterminate)
	{
			if (ctx->GenNewMSDNums())
		{
				ctx->StartMSDTimer();
				return SendMSD(ctx);
			} else
			{
				dprint3("VS_H323Parser::RecvMSD: MSDError (F) max number of retries exceeded\n");
				ctx->SetMsdState(MSDIdle);
				TerminateSession(ctx);
		}
		} else
		{
			h245params->H245Flags[VS_H323ParserInfo::e_msd] |= VS_H245Data::h245_req_recv;
			ctx->StopWaitingMSD();
			ctx->SetMsdState(MSDIncomingAwaitingResponse);
			ctx->StartMSDTimer();
			h245params->m_msd_type = (decision == VS_H245MasterSlaveDeterminationAck_Decision::e_master) ? 1 : 0;
			return SendMSDA(ctx, decision);
		}
	}

	return true;
}

bool VS_H323Parser::SendMSDA(const std::shared_ptr<VS_H323ParserInfo>& ctx, int our_decision) {
	VS_H245Data* h245params = ctx->GetH245Params();

		VS_H245MultimediaSystemControlMessage   mscm;
		VS_H245ResponseMessage   *rm = new VS_H245ResponseMessage;
		VS_H245MasterSlaveDeterminationAck   *msda = new VS_H245MasterSlaveDeterminationAck;

	// toggle value, because H.245 Recomendation says RECEIVEING terminal status
	msda->decision.tag = (our_decision == VS_H245MasterSlaveDeterminationAck_Decision::e_master) ?
		VS_H245MasterSlaveDeterminationAck_Decision::e_slave :
		VS_H245MasterSlaveDeterminationAck_Decision::e_master;

	msda->decision.filled = true;	msda->filled = true;
	VS_AsnNull * a = new VS_AsnNull;
	a->filled = true;
	msda->decision.choice = a;
		*rm = msda;		mscm = rm;
	if (PutH245Message(mscm, ctx)) {
			h245params->H245Flags[VS_H323ParserInfo::e_msd] |= VS_H245Data::h245_rsp_send;
			return true;
		}
	return false;
	}

bool VS_H323Parser::SendMSDR(const std::shared_ptr<VS_H323ParserInfo>& ctx, int cause)
{
	VS_H245Data* h245params = ctx->GetH245Params();

	VS_H245MultimediaSystemControlMessage   mscm;
	VS_H245ResponseMessage   *rm = new VS_H245ResponseMessage;
	VS_H245MasterSlaveDeterminationReject *msdr = new VS_H245MasterSlaveDeterminationReject;
	msdr->cause.tag = cause;
	msdr->cause.filled = true;
	msdr->filled = true;

	msdr->cause.choice = new VS_AsnNull;
	msdr->cause.choice->filled = true;

	*rm = msdr;		mscm = rm;
	if (PutH245Message(mscm, ctx)) {
		h245params->H245Flags[VS_H323ParserInfo::e_msd] &= ~VS_H245Data::h245_req_send;

		if (ctx->GenNewMSDNums()) {
			ctx->StartWaitingMSD();

			return true;
		}
	}
	return false;
}

bool VS_H323Parser::SendMSDRel(const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245Data* h245params = ctx->GetH245Params();

	VS_H245MultimediaSystemControlMessage   mscm;
	VS_H245IndicationMessage   *im = new VS_H245IndicationMessage;
	VS_H245MasterSlaveDeterminationRelease *msdrel = new VS_H245MasterSlaveDeterminationRelease;
	msdrel->filled = true;

	*im = msdrel;		mscm = im;
	return PutH245Message(mscm, ctx);
}

bool VS_H323Parser::RecvTCS(VS_H245TerminalCapabilitySet * tcs, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (!tcs)
	{
		return false;
	}
	const auto& tcs_oid = tcs->protocolIdentifier;
	if (tcs_oid.size < 6)
	{
		dprint2("VS_H323Parser: TCS received, short OID (length=%zu)\n", tcs_oid.size);
		return false;
	}
	if (tcs_oid.value[0] != 0 || tcs_oid.value[1] != 0 || tcs_oid.value[2] != 8 || tcs_oid.value[3] != 245 || tcs_oid.value[4] != 0)
	{
		dprint2("VS_H323Parser: TCS received, wrong OID: { %u, %u, %u, %u, %u, %u }\n", tcs_oid.value[0], tcs_oid.value[1], tcs_oid.value[2], tcs_oid.value[3], tcs_oid.value[4], tcs_oid.value[5]);
		return false;
	}
	unsigned h245_version = tcs_oid.value[5];
	ctx->SetPeerH245Version(h245_version);
	dprint2("VS_H323Parser: TCS received, H.245 version %u\n", h245_version);

	// If h245 version is lower than 5, than genericAudioCapability/genericVideoCapability are not supported
	// and we need to resend our TCS without codecs that use them (H.264, G.722.1, G.722.1C)
	if (h245_version < 5)
		SendTCS(ctx);

	//#ifdef _LOG_DEBUG_
	//	ShowCD(	tcs->capabilityDescriptors );
	//#endif

	for (auto& cte: tcs->capabilityTable)
	{
		if (!cte.capabilityTableEntryNumber.filled
		 || !cte.capability.filled
		 || cte.capability.tag != VS_H245Capability::e_genericControlCapability)
		{
			continue;
		}
		VS_H245GenericCapability* cc = cte.capability;
		if (!cc
		 || !cc->capabilityIdentifier.filled
		 || !CheckH245CapabilityIdentifier(cc->capabilityIdentifier, oid_h239_cc))
		{
			continue;
		}
		ctx->SetH239CapabilityPresent(true);
		break;
	}

	VS_H245Data* h245params = ctx->GetH245Params();

	h245params->H245Flags[VS_H323ParserInfo::e_tcs] |= VS_H245Data::h245_req_recv;
	ctx->SetTCSWaitingTick(NULL_TICK);

	// если канал уже был открыть, но пришли новые TCS
	if ((h245params->H245Flags[VS_H323ParserInfo::e_olc_video] & VS_H245Data::h245_req_send))
	{
		MakeCloseLogicalChannel(h245params->m_videoNumberLCSender, ctx);
		h245params->H245Flags[VS_H323ParserInfo::e_olc_video] &= ~VS_H245Data::h245_req_send;

		ctx->CloseH245LogicalChannel(h245params->m_videoNumberLCSender);
	}
	if ((h245params->H245Flags[VS_H323ParserInfo::e_olc_audio] & VS_H245Data::h245_req_send))
	{
		MakeCloseLogicalChannel(h245params->m_audioNumberLCSender, ctx);
		h245params->H245Flags[VS_H323ParserInfo::e_olc_audio] &= ~VS_H245Data::h245_req_send;

		ctx->CloseH245LogicalChannel(h245params->m_audioNumberLCSender);
	}
	if ((h245params->H245Flags[VS_H323ParserInfo::e_olc_slides] & VS_H245Data::h245_req_send))
	{
		MakeCloseLogicalChannel(h245params->m_slidesNumberLCSender, ctx);
		h245params->H245Flags[VS_H323ParserInfo::e_olc_slides] &= ~VS_H245Data::h245_req_send;

		ctx->CloseH245LogicalChannel(h245params->m_slidesNumberLCSender);
	}
	if ((h245params->H245Flags[VS_H323ParserInfo::e_olc_data] & VS_H245Data::h245_req_send))
	{
		MakeCloseLogicalChannel(h245params->m_dataNumberLCSender, ctx);
		h245params->H245Flags[VS_H323ParserInfo::e_olc_data] &= ~VS_H245Data::h245_req_send;

		ctx->CloseH245LogicalChannel(h245params->m_dataNumberLCSender);
	}

	auto& h235_auth = ctx->h235_auth;

	VS_H245AudioCapability** ac = ctx->GetAudioCapability();
	auto codec_prec_list = ctx->GetAudioCodecPrecedenceList();
	for (auto &codec : codec_prec_list)
	{
		VS_GatewayAudioMode mode;
		auto &&i = ctx->GetAudioIndex(codec);

		if (i >= ac_number)
			continue;
		{
			VS_H245GenericCapability *result = dynamic_cast<VS_H245GenericCapability*>(ac[i]->choice);
			unsigned int maxBr = (result && result->maxBitRate.filled) ? result->maxBitRate.value : 0;

			unsigned capabilityTableEntryNumber;
			if ((capabilityTableEntryNumber = FindAudioCapability(ac[i]->tag, maxBr, tcs, mode, ctx)) != 0)
			{
				//ctx->SetACDefault(i); //ac_default = i;
				dstream3 << "Search for generic encryption mode for audio channel...\n";
				mode.sec_cap.m = h235_auth.FindH235EncryptionMode(tcs, capabilityTableEntryNumber);
				ctx->SetACDefault(ctx->GetAudioIndex(mode.CodecType));
				ctx->audio_channel.snd_mode_audio = mode;
				break;
			}
		}

	}

	VS_H245VideoCapability** vc = ctx->GetVideoCapability();
	for (int i = vc_number-1; i>=0; --i)
	{
		VS_GatewayVideoMode mode;
		unsigned capabilityTableEntryNumber;
		if ((capabilityTableEntryNumber = FindVideoCapability(vc[i], tcs, mode, ctx)) != 0)
		{
			//vc_default = i;
			size_t maxBr = ctx->GetH245Params()->m_videoMaxBitrate * 1000;
			mode.Bitrate = std::min<decltype(mode.Bitrate)>(mode.Bitrate, maxBr);
			if (mode.Bitrate < 256000)
				ctx->SetH264Level(13);

			dstream3 << "Search for generic encryption mode for video channel...\n";
			mode.sec_cap.m = h235_auth.FindH235EncryptionMode(tcs, capabilityTableEntryNumber);
			ctx->video_channel.snd_mode_video = mode;
			break;
		}
	}

	for (int i = vc_number-1; i>=0; --i)
	{
		VS_GatewayVideoMode mode;
		unsigned capabilityTableEntryNumber;
		if ((capabilityTableEntryNumber = FindSlidesVideoCapability(vc[i], tcs, mode, ctx)) != 0)
		{
			size_t maxBr = ctx->GetH245Params()->m_videoMaxBitrate * 1000;
			mode.Bitrate = std::min<decltype(mode.Bitrate)>(mode.Bitrate, maxBr);
			if (mode.Bitrate < 256000)
				ctx->SetH264Level(13);

			dstream3 << "Search for generic encryption mode for slides channel...\n";
			mode.sec_cap.m = h235_auth.FindH235EncryptionMode(tcs, capabilityTableEntryNumber);
			ctx->slides_channel.snd_mode_video = mode;
			break;
		}
	}

	VS_H245DataApplicationCapability** dc = ctx->GetDataCapability();
	for (int i = dc_number - 1; i >= 0; --i)
	{
		VS_GatewayDataMode mode;
		unsigned capabilityTableEntryNumber;
		if ((capabilityTableEntryNumber = FindDataCapability(dc[i], tcs, mode, ctx)) != 0)
		{
			size_t maxBr = 5000; // TODO_FECC what here should be?
			mode.BitRate = std::min<decltype(mode.BitRate)>(mode.BitRate, maxBr);
			dstream3 << "Search for generic encryption mode for data application channel...\n";
			mode.sec_cap.m = h235_auth.FindH235EncryptionMode(tcs, capabilityTableEntryNumber);
			ctx->SetH224CapabilityPresent(true);
			ctx->data_channel.snd_mode_data = mode;
		}
	}
	return true;
}

void VS_H323Parser::MakeCloseLogicalChannelAck(const unsigned int num, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245CloseLogicalChannelAck* clca = new VS_H245CloseLogicalChannelAck;
	clca->forwardLogicalChannelNumber.value = num;
	clca->forwardLogicalChannelNumber.filled = true;
	clca->filled = true;

	VS_H245ResponseMessage* resp = new VS_H245ResponseMessage;
	resp->filled = true;
	resp->choice = clca;
	resp->tag = VS_H245ResponseMessage::e_closeLogicalChannelAck;

	VS_H245MultimediaSystemControlMessage mscm;
	mscm.choice = resp;
	mscm.filled = true;
	mscm.tag = VS_H245MultimediaSystemControlMessage::e_response;

	PutH245Message(mscm, ctx);
}

bool VS_H323Parser::SendTCSA(std::uint32_t sequenceNumber, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245MultimediaSystemControlMessage   mscm;
	VS_H245ResponseMessage   *rm = new VS_H245ResponseMessage;
	VS_H245TerminalCapabilitySetAck   *tcsa = new VS_H245TerminalCapabilitySetAck;
	tcsa->sequenceNumber.value = sequenceNumber;
	tcsa->sequenceNumber.filled = true;
	tcsa->filled = true;

	*rm = tcsa;
	mscm = rm;
	if (PutH245Message(mscm, ctx))
	{
		VS_H245Data* h245params = ctx->GetH245Params();
		h245params->H245Flags[VS_H323ParserInfo::e_tcs] |= VS_H245Data::h245_rsp_send;

		return true;
	}
	return false;
}

void VS_H323Parser::SendRejectOLC(VS_H245OpenLogicalChannel* olc, const std::shared_ptr<VS_H323ParserInfo>& ctx, const unsigned int cause)
{
	VS_H245MultimediaSystemControlMessage mscm;
	VS_H245ResponseMessage* rm = new VS_H245ResponseMessage;

	VS_H245OpenLogicalChannelReject* olcr = new VS_H245OpenLogicalChannelReject;
	olcr->forwardLogicalChannelNumber = olc->forwardLogicalChannelNumber;
	olcr->cause.choice = new VS_AsnNull;
	olcr->cause.tag = cause;
	olcr->cause.filled = true;
	olcr->filled = true;

	rm->choice = olcr;
	rm->tag = VS_H245ResponseMessage::e_openLogicalChannelReject;
	rm->filled = true;

	mscm.choice = rm;
	mscm.tag = VS_H245MultimediaSystemControlMessage::e_response;
	mscm.filled = true;

	PutH245Message(mscm, ctx);
}

void UpdateDynamicPayloadTypeAndH235Caps(const int dynamic_pt, const VS_H235SecurityCapability& sec_cap, const unsigned channel_number, VS_GatewayMediaMode& OUTmode, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	auto &out_sc = OUTmode.sec_cap;
	if (dynamic_pt){
		OUTmode.PayloadType = dynamic_pt;
		out_sc = sec_cap;
		if (out_sc.syncFlag != dynamic_pt){
			dstream3 << "ERROR\tH235 syncFlag = '" << out_sc.syncFlag << "' and channel PayloadType = '" << OUTmode.PayloadType << "' must be equal.\n";
		}
	}
	else{
		std::tie(out_sc.h235_sessionKey, out_sc.m) = std::make_tuple(sec_cap.h235_sessionKey, sec_cap.m);
	}

	bool master = ctx->GetH245Params()->m_msd_type == 1;
	if (master) ctx->channels_h235caps.emplace(channel_number, out_sc);
}

bool VS_H323Parser::RecvOLC(VS_H245OpenLogicalChannel* olc, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (!olc || !ctx)
		return false;

	dstream2 << "VS_H323Parser::RecvOLC with number ='" << olc->forwardLogicalChannelNumber.value << "'\n";

	unsigned dynamic_pt(0);
	if (olc->forwardLogicalChannelParameters.multiplexParameters.tag == VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters::e_h2250LogicalChannelParameters)
	{
		auto p = dynamic_cast<const VS_H245H2250LogicalChannelParameters*>(olc->forwardLogicalChannelParameters.multiplexParameters.choice);
		if (p && p->dynamicRTPPayloadType.filled)
			dynamic_pt = p->dynamicRTPPayloadType.value;
	}

	VS_AsnChoice* channel_dataTypeParams = &olc->forwardLogicalChannelParameters.dataType;
	const VS_H235Authenticator &h235_auth = ctx->h235_auth;
	VS_H245LogicalChannelInfoDataType data_type;

	try{

	VS_H235SecurityCapability sec_cap(encryption_mode::no_encryption, dynamic_pt);
	if (channel_dataTypeParams->tag == VS_H245DataType::e_h235Media && channel_dataTypeParams->choice->filled){
		VS_H245H235Media * m = static_cast<VS_H245H235Media *>(channel_dataTypeParams->choice);
		unsigned real_mediaTag(0);
		bool res(false);

		dstream3 << "Negotiate receive encryption for channel no='" << olc->forwardLogicalChannelNumber.value << "'...\n";
		std::tie(res, sec_cap.m, real_mediaTag) = h235_auth.ReceiveH235Media(m);
		if (!res){
			throw (unsigned int)VS_H245OpenLogicalChannelReject_Cause::e_securityDenied;
		}
		else{
			channel_dataTypeParams = &m->mediaType;
			channel_dataTypeParams->tag = real_mediaTag;
		}

		if (sec_cap.m != encryption_mode::no_encryption){
			bool master = ctx->GetH245Params()->m_msd_type == 1;
			if (!master){
				// we are slave and must save h235key and SynchFlag from master
				bool decrypted = h235_auth.ReadEncryptionSync(olc->encryptionSync, sec_cap);
				if (!decrypted)	throw (unsigned int)VS_H245OpenLogicalChannelReject_Cause::e_securityDenied;
				else{
					dstream3 << "H235 Session key was successfully decrypted and saved for logcal channel no='" << olc->forwardLogicalChannelNumber.value << "'.\n";
				}
			}
			else {
				sec_cap.h235_sessionKey = h235_auth.GenerateSessionKey(sec_cap.m);
				assert(!sec_cap.h235_sessionKey.empty());
			}
		}
	}

	if (channel_dataTypeParams->tag == VS_H245DataType::e_videoData
		&& static_cast<VS_H245VideoCapability*>(channel_dataTypeParams->choice)->tag != VS_H245VideoCapability::e_extendedVideoCapability)
	{
		data_type = e_video;

		auto cap = static_cast<const VS_H245VideoCapability*>(channel_dataTypeParams->choice);
		if (cap)
		{
			auto codec = GetVideoCapabilityCodec(cap);
			if (codec != e_videoNone)
			{
				auto mode = ctx->GetRecvVideoMode(codec);
				if (mode){
					// update dynamic (96-127) PayloadType for our rcv video codec
					UpdateDynamicPayloadTypeAndH235Caps(dynamic_pt, sec_cap, olc->forwardLogicalChannelNumber.value, *mode, ctx);
				}
			}
		}
	}
	else if (channel_dataTypeParams->tag == VS_H245DataType::e_audioData)
	{
		data_type = e_audio;

		{
			VS_H245AudioCapability* acap = static_cast<VS_H245AudioCapability*>(channel_dataTypeParams->choice);

			switch (acap->tag)
			{
			case VS_H245AudioCapability::e_genericAudioCapability:
			{
				VS_H245GenericCapability *cap = static_cast<VS_H245GenericCapability*>(acap->choice);
				VS_GatewayAudioMode* a_mode = nullptr;

				if (cap->maxBitRate.filled)
				{
					auto bitrate = cap->maxBitRate.value;
					VS_AsnObjectId *id = static_cast<VS_AsnObjectId*>(cap->capabilityIdentifier.choice);

					// G7221
					if (memcmp(id->value, id_G7221, sizeof(id_G7221)) == 0)
					{
						switch (bitrate)
						{
						case 32000:
							a_mode = ctx->GetRecvAudioMode(e_rcvG722132);
							break;
						case 24000:
							a_mode = ctx->GetRecvAudioMode(e_rcvG722124);
							break;
						}
					}
					else if (memcmp(id->value, id_SIREN14, sizeof(id_SIREN14)) == 0) // SIREN14
					{
						switch (bitrate)
						{
						case 240:
							a_mode = ctx->GetRecvAudioMode(e_rcvSIREN14_24);
							break;
						case 320:
							a_mode = ctx->GetRecvAudioMode(e_rcvSIREN14_32);
							break;
						case 480:
							a_mode = ctx->GetRecvAudioMode(e_rcvSIREN14_48);
							break;
						}
					}
				}

				// set proper payload type
				if (a_mode != NULL)
				{
					// update dynamic (96-127) PayloadType for our rcv audio codec
					UpdateDynamicPayloadTypeAndH235Caps(dynamic_pt, sec_cap, olc->forwardLogicalChannelNumber.value, *a_mode, ctx);
				}

			} break;
			}
		}
	}
	else if (channel_dataTypeParams->tag == VS_H245DataType::e_videoData
		&& static_cast<VS_H245VideoCapability*>(channel_dataTypeParams->choice)->tag == VS_H245VideoCapability::e_extendedVideoCapability
		&& ctx->IsH239Enabled())
	{
		data_type = e_slides;


		{
			auto cap = static_cast<const VS_H245VideoCapability*>(channel_dataTypeParams->choice);
			if (cap)
			{
				auto codec = GetVideoCapabilityCodec(cap);
				if (codec != e_videoNone)
				{
					auto mode = ctx->GetRecvSlidesVideoMode(codec);
					if (mode){
						// update dynamic (96-127) PayloadType for our rcv slides video codec
						UpdateDynamicPayloadTypeAndH235Caps(dynamic_pt, sec_cap, olc->forwardLogicalChannelNumber.value, *mode, ctx);
					}
				}
			}
		}
	}
	else if (channel_dataTypeParams->tag == VS_H245DataType::e_data
		&& ctx->IsH224Enabled())
	{
		data_type = e_data;

		if (dynamic_pt)
		{
			auto cap = static_cast<const VS_H245DataApplicationCapability*>(channel_dataTypeParams->choice);
			if (cap)
			{
				auto codec = GetDataCapabilityCodec(cap);
				if (codec != VS_H323DataCodec::dataNone)
				{
					auto mode = ctx->GetRecvDataMode(codec);
					if (mode)
						mode->PayloadType = dynamic_pt;
				}
			}
		}

		auto number = olc->forwardLogicalChannelNumber.value;

		if (!ctx->CreateH245LogicalChannel(data_type, number, false))
			return false;

		VS_H245LogicalChannelInfo info;

		if (!ctx->GetH245LogicalChannel(number, info))
			return false;
		info.m_sessionID = (static_cast<VS_H245H2250LogicalChannelParameters *>(olc->forwardLogicalChannelParameters.multiplexParameters.choice))->sessionID.value;
		net::address addr;
		net::port port;
		if (!(static_cast<VS_GwH245OpenLogicalChannel &>(*olc)).GetReverseRtcpIpPort(addr, port))
			return false;

		info.m_remote_rtcp_address = { addr, port };

		if (!ctx->SetH245LogicalChannel(number, info))
			return false;

		ctx->SetLCPending(number, true);

		VS_H245Data* h245params = ctx->GetH245Params();
		h245params->H245Flags[VS_H323ParserInfo::e_olc_data] |= h245params->h245_req_recv;
		h245params->m_dataNumberLCReciver = number;

		return true;
	}
	else
	{
		throw (unsigned int)VS_H245OpenLogicalChannelReject_Cause::e_dataTypeNotSupported;
	}
	}
	catch (const unsigned int reject_cause){
		SendRejectOLC(olc, ctx, reject_cause);
		return false;
	}

	//if (olc->forwardLogicalChannelParameters.dataType.tag ==
	//	olc->forwardLogicalChannelParameters.dataType.e_audioData )
	//{
	//	type = (VS_H245LogicalChannelInfoDataTypes)e_audio;
	//	VS_H245AudioCapability * aucap = (VS_H245AudioCapability*)
	//		(olc->forwardLogicalChannelParameters.dataType.choice);

	//	logprint2("\n\t AUCAP: %x Tag: %d",aucap, aucap->tag);

	//	m_modes.rcvAudioCodecType = TestAudioCapability( aucap );
	//
	//} else
	//if (olc->forwardLogicalChannelParameters.dataType.tag ==
	//	olc->forwardLogicalChannelParameters.dataType.e_videoData )
	//{
	//	type = /*VS_H245LogicalChannelInfoDataTypes::*/e_video;

	//	VS_H245VideoCapability * vcap = (VS_H245VideoCapability*)
	//		olc->forwardLogicalChannelParameters.dataType.choice;
	//	switch(vcap->tag)
	//
	//	{
	//	case vcap->e_genericVideoCapability:
	//		{
	//	        VS_H245GenericCapability * cap =
	//	            static_cast<VS_H245GenericCapability*>
	//		        (vcap->choice);

	//			m_modes.rcvVideoCodecType = e_videoH264;
	//			m_modes.rcvVideoBitrate = cap->maxBitRate.value * 100;
	//		}break;
	//	case vcap->e_h263VideoCapability:
	//		{
	//			VS_H245H263VideoCapability * vc2 = (VS_H245H263VideoCapability *)vcap->choice;
	//			m_modes.rcvVideoMode = (vc2->sqcifMPI.value!=0)* 0x01 |
	//				(vc2->qcifMPI.value!=0)			* 0x02 |
	//				(vc2->cifMPI.value!=0)			* 0x04 |
	//				(vc2->cif4MPI.value!=0)			* 0x08 |
	//				(vc2->cif16MPI.value!=0)		* 0x10 ;
	//			m_modes.rcvVideoBitrate =  vc2->maxBitRate.value * 100;
	//               m_modes.rcvVideoCodecType = e_videoH263;
	//			m_modes.rcvVideoCodecAnexInfo = 0;

	//			VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters* mparams =
	//								(VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters*)
	//														&olc->forwardLogicalChannelParameters.multiplexParameters;
	//			if (mparams->tag == mparams->e_h2250LogicalChannelParameters)
	//			{
	//				VS_H245H2250LogicalChannelParameters* h2250params = (VS_H245H2250LogicalChannelParameters*) mparams->choice;
	//				if (h2250params->dynamicRTPPayloadType.filled)
	//					m_modes.rcvVideoPayloadType = h2250params->dynamicRTPPayloadType.value;
	//			}

	//			if (vc2->advancedPrediction.value)
	//				m_modes.rcvVideoCodecAnexInfo |= VS_H323BeginH245Modes::e_anexF;

	//               if (vc2->h263Options.filled)
	//               {
	//                   m_modes.rcvVideoCodecType = e_videoH263plus;
	//				if (vc2->h263Options.advancedIntraCodingMode.value)
	//					m_modes.rcvVideoCodecAnexInfo |= VS_H323BeginH245Modes::e_anexI;
	//				if (vc2->h263Options.deblockingFilterMode.value)
	//					m_modes.rcvVideoCodecAnexInfo |= VS_H323BeginH245Modes::e_anexJ;
	//				if (vc2->h263Options.modifiedQuantizationMode.value)
	//					m_modes.rcvVideoCodecAnexInfo |= VS_H323BeginH245Modes::e_anexT;
	//				if (vc2->h263Options.refPictureSelection.filled)
	//					m_modes.rcvVideoCodecAnexInfo |= VS_H323BeginH245Modes::e_anexN;
	//                   if (vc2->h263Options.h263Version3Options.filled)
	//                       m_modes.rcvVideoCodecType = e_videoH263plus2;
	//               }

	//			// ktrushnikov fix: RosSelHozBank: Tandberg 770 MXP: detect if H.263 or H.263+
	//			//					due to payloadDescriptor(rfc_num) rather than existent of h263Options
	//			if (olc->forwardLogicalChannelParameters.multiplexParameters.tag == olc->forwardLogicalChannelParameters.multiplexParameters.e_h2250LogicalChannelParameters)
	//			{
	//				VS_H245H2250LogicalChannelParameters* p = (VS_H245H2250LogicalChannelParameters*) olc->forwardLogicalChannelParameters.multiplexParameters.choice;
	//				if (p)
	//				{
	//					if (p->mediaPacketization.tag == p->mediaPacketization.e_rtpPayloadType)
	//					{
	//						VS_H245RTPPayloadType* pt = (VS_H245RTPPayloadType*) p->mediaPacketization.choice;
	//						if (pt)
	//						{
	//							if (pt->payloadDescriptor.tag == pt->payloadDescriptor.e_rfc_number)
	//							{
	//								VS_AsnInteger* i = (VS_AsnInteger*) pt->payloadDescriptor.choice;
	//								if (i)
	//								{
	//									if (i->value == 2429)	// H.263+ (RFC2429 H263-1998)
	//										m_modes.rcvVideoCodecType = e_videoH263plus;
	//								}
	//							}
	//						}
	//					}
	//				}
	//			}

	//			logprint1("\n\t Recieve video mode: %d", m_modes.rcvVideoMode);
	//			logprint1("\n\t Recieve video Anexes: %d", m_modes.rcvVideoCodecAnexInfo);

	//		}break;
	//	case vcap->e_h261VideoCapability:
	//		{
	//			VS_H245H261VideoCapability * h261v =
	//			(VS_H245H261VideoCapability *)(vcap->choice);
	//			m_modes.rcvVideoMode =
	//				(h261v->qcifMPI.value!=0)			* 0x02 |
	//				(h261v->cifMPI.value!=0)			* 0x04 ;
	//               m_modes.rcvVideoBitrate = h261v->maxBitRate.value * 100;
	//               m_modes.rcvVideoCodecType = e_videoH261;

	//		}break;
	//	default:
	//		{
	//		}break;
	//	}
	//}else
	//{
	//	return false;
	//}

	// Check if channel is already exist
	//if ( m_mgr->GetCallInfo()->GetH323CallInfoInterface()->GetH245LogicalChannel( type, true, info) )
	//{
	//	if ( !CreateOLCReject(olc) )
	//		return false;
	//}

	auto number = olc->forwardLogicalChannelNumber.value;

	if (!ctx->CreateH245LogicalChannel(data_type, number, false))
		return false;

	VS_H245LogicalChannelInfo info;

	if (!ctx->GetH245LogicalChannel(number, info))
		return false;
	info.m_sessionID = (static_cast<VS_H245H2250LogicalChannelParameters *>(olc->forwardLogicalChannelParameters.multiplexParameters.choice))->sessionID.value;
	net::address addr;
	net::port port;
	if (!(static_cast<VS_GwH245OpenLogicalChannel &>(*olc)).GetReverseRtcpIpPort(addr, port))
		return false;
	info.m_remote_rtcp_address = { addr, port };

	if (!ctx->SetH245LogicalChannel(number, info))
		return false;

	ctx->SetLCPending(number, true);
	SendSetMediaChannels(ctx);

	VS_H245Data* h245params = ctx->GetH245Params();
	switch (data_type)
	{
	case e_audio:
		h245params->H245Flags[VS_H323ParserInfo::e_olc_audio] |= VS_H245Data::h245_req_recv;
		h245params->m_audioNumberLCReciver = number;
		break;
	case e_video:
		h245params->H245Flags[VS_H323ParserInfo::e_olc_video] |= VS_H245Data::h245_req_recv;
		h245params->m_videoNumberLCReciver = number;
		break;
	case e_slides:
		h245params->H245Flags[VS_H323ParserInfo::e_olc_slides] |= VS_H245Data::h245_req_recv;
		h245params->m_slidesNumberLCReciver = number;
		break;
	}

	return true;
}

bool VS_H323Parser::RecvOLCA(VS_H245OpenLogicalChannelAck * ack, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (!ack)
		return false;

	VS_H245LogicalChannelInfo info;
	unsigned long number = ack->forwardLogicalChannelNumber.value;

	if (!ctx->GetH245LogicalChannel(number, info))
		return false;

	VS_H235SecurityCapability *h235_sec_cap = nullptr;
	VS_H245Data* h245params = ctx->GetH245Params();
	switch (info.m_dataType)
	{
	case e_audio:
		h245params->H245Flags[VS_H323ParserInfo::e_olc_audio] |= VS_H245Data::h245_rsp_recv;
		h235_sec_cap = &ctx->audio_channel.snd_mode_audio.sec_cap;
		break;
	case e_video:
		h245params->H245Flags[VS_H323ParserInfo::e_olc_video] |= VS_H245Data::h245_rsp_recv;
		h235_sec_cap = &ctx->video_channel.snd_mode_video.sec_cap;
		break;
	case e_slides:
		h245params->H245Flags[VS_H323ParserInfo::e_olc_slides] |= VS_H245Data::h245_rsp_recv;
		h235_sec_cap = &ctx->slides_channel.snd_mode_video.sec_cap;
		break;
	case e_data:
		h245params->H245Flags[VS_H323ParserInfo::e_olc_data] |= VS_H245Data::h245_rsp_recv;
		h235_sec_cap = &ctx->data_channel.snd_mode_data.sec_cap;
		break;
	default:
		return false;
	}

	encryption_mode emode = encryption_mode::no_encryption;
	auto h235_sec_caps_it = ctx->channels_h235caps.find(number);
	if (h235_sec_caps_it != ctx->channels_h235caps.end() && (emode = h235_sec_caps_it->second.m) != encryption_mode::no_encryption){
		assert(h235_sec_cap != nullptr);

		bool master = ctx->GetH245Params()->m_msd_type == 1;
		if (!master){
			if (h235_sec_cap) {
				auto &sec_cap = *h235_sec_cap;
				bool decrypted = ctx->h235_auth.ReadEncryptionSync(ack->encryptionSync, sec_cap);
				if (!decrypted)
					return false;

					dstream3 << "H235 Session key was successfully decrypted and saved for logcal channel no = '" << number << "'.\n";
					sec_cap.m = emode;

				}
			}
		else{
			if (h235_sec_cap) *h235_sec_cap = h235_sec_caps_it->second;
		}
	}

	auto channel = ctx->GetMediaChannel(info.m_dataType);
	if (!channel)
		return false;

	net::address rtp_addr;
	net::port rtp_port;

	net::address rtcp_addr;
	net::port rtcp_port;

	if (!(static_cast<VS_GwH245OpenLogicalChannelAck *>(ack))->GetRtpRtcpIpPort(rtp_addr, rtp_port, rtcp_addr, rtcp_port))
		return false;

	info.m_remote_rtp_address = { rtp_addr, rtp_port };
	info.m_remote_rtcp_address = { rtcp_addr, rtcp_port };

	channel->remote_rtp_address.address(rtp_addr);
	channel->remote_rtp_address.port(rtp_port);
	channel->remote_rtcp_address.address(rtcp_addr);
	channel->remote_rtcp_address.port(rtcp_port);

	//if (m_context->SetH245LogicalChannel( number , info ))
	//	return false;

	SendSetMediaChannels(ctx);

	return true;
}

unsigned VS_H323Parser::FindAudioCapability(unsigned tag, std::uint32_t maxBr, VS_H245TerminalCapabilitySet* tcs, VS_GatewayAudioMode& mode, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245AudioCapability** ac = ctx->GetAudioCapability();
	for (auto& cte: tcs->capabilityTable)
	{
		if (!cte.capabilityTableEntryNumber.filled || !cte.capability.filled)
		{
			continue;
		}

		if (cte.capability.tag!=VS_H245Capability::e_receiveAudioCapability &&
			cte.capability.tag!=VS_H245Capability::e_receiveAndTransmitAudioCapability)
		{
			continue;
		}

		VS_H245AudioCapability* ac1 = cte.capability;
		if (!ac1)
		{
			continue;
		}

		if (ac1->tag != tag)
			continue;

		VS_GatewayAudioMode m;
		if (ac1->tag==VS_H245AudioCapability::e_g711Ulaw64k) {
			m.CodecType = e_rcvG711Ulaw64k;
			m.PayloadType = SDP_PT_G711U;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG711Ulaw64k);
		} else if (ac1->tag==VS_H245AudioCapability::e_g711Alaw64k) {
			m.CodecType = e_rcvG711Alaw64k;
			m.PayloadType = SDP_PT_G711A;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG711Alaw64k);
		} else if (ac1->tag==VS_H245AudioCapability::e_g729AnnexA) {
			m.CodecType = e_rcvG729a;
			m.PayloadType = SDP_PT_G729A;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG729a);
		} else if (ac1->tag==VS_H245AudioCapability::e_g7231) {
			m.CodecType = e_rcvG723;
			m.PayloadType = SDP_PT_G723;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG723);
			{	// set maxAl-sduAudioFrames
				VS_H245AudioCapability_G7231* g7231_his = static_cast<VS_H245AudioCapability_G7231*> (ac1->choice);
				unsigned long his_value = g7231_his->maxAl_sduAudioFrames.value;

				const unsigned int g723_number = GetAudioIndex(e_rcvG723);
				if (g723_number < ac_number )
				{
					if (ac[g723_number] && ac[g723_number]->choice)
						(dynamic_cast<VS_H245AudioCapability_G7231*>(ac[g723_number]->choice))->maxAl_sduAudioFrames.value = his_value;
				}
			}
		} else if (ac1->tag==VS_H245AudioCapability::e_g728) {
			m.CodecType = e_rcvG728;
			m.PayloadType = SDP_PT_G728;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG728);
		} else if (ac1->tag==VS_H245AudioCapability::e_genericAudioCapability) {
			VS_H245GenericCapability *caps = dynamic_cast<VS_H245GenericCapability*>(ac1->choice);
			if (caps && caps->maxBitRate.filled)
			{
				/*
				By Artem Boldarev (23.09.2015):
				We should choose right version of the SIREN14 codec
				using second codec parameter from the Terminal Cabaility Set -
				it is boolean array. Unfortunately, I am unable to get this data (but it is obviously here),
				so I forced to use bitrate value.

				It is still worthwhile solution though.
				*/

				auto bitrate = caps->maxBitRate.value; // common minimal bitrate

				// function to check codec availability
				auto is_codec_available = [ctx](const int codec) -> bool {
					return ctx->GetRecvAudioMode(codec) != nullptr;
				};

				VS_AsnObjectId *id = dynamic_cast<VS_AsnObjectId*>(caps->capabilityIdentifier.choice);
				if (id == nullptr)
					continue;

				bitrate = maxBr <= caps->maxBitRate.value ? maxBr : caps->maxBitRate.value;
				// Siren 14 (G722.1C)
				if (memcmp(&id->value[0], id_SIREN14, sizeof(id_SIREN14)) == 0)
				{
					/*
					//TODO: Try to get supported codec bitrate from second (boolean array) parameter.
					VS_ConstrainedArrayAsn *arr = dynamic_cast<VS_ConstrainedArrayAsn*>(&gcap->collapsing);
					VS_H245GenericParameter *param;
					//VS_Asn *param;
					if (arr == NULL && arr->length < 2)
					continue;

					//param = (VS_H245GenericParameter *)&arr->asns[0];
					param = (VS_H245GenericParameter*)(&arr->asns[1]);

					if (param == NULL)
					{
					continue;
					}
					puts("Test");*/

					//bitrate = maxBr <= caps->maxBitRate.value ? maxBr : caps->maxBitRate.value;
					switch (bitrate)
					{
					case 480:
						if (is_codec_available(e_rcvSIREN14_48))
						{
							m.CodecType = e_rcvSIREN14_48;
							m.PayloadType = SDP_PT_DYNAMIC_SIREN14_48k;
							m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvSIREN14_48);

						}
						else
						{
							continue;
						}
						break;
					case 320:
						if (is_codec_available(e_rcvSIREN14_32))
						{
							m.CodecType = e_rcvSIREN14_32;
							m.PayloadType = SDP_PT_DYNAMIC_SIREN14_32k;
							m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvSIREN14_32);
						}
						else
						{
							continue;
						}
						break;
					case 240:
						if (is_codec_available(e_rcvSIREN14_24))
						{
							m.CodecType = e_rcvSIREN14_24;
							m.PayloadType = SDP_PT_DYNAMIC_SIREN14_24k;
							m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvSIREN14_24);
						}
						else
						{
							continue;
						}
						break;
					default:
						continue;
						break;
					}
				}
				// G722.1
				else if (memcmp(&id->value[0], id_G7221, sizeof(id_G7221)) == 0)
				{
					switch (bitrate)
					{
					case 32000:
						if (is_codec_available(e_rcvG722132))
						{
							m.CodecType = e_rcvG722132;
							m.PayloadType = SDP_PT_DYNAMIC_G722132;
							m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG722132);
						}
						else
						{
							continue;
						}
						break;
					case 24000:
						if (is_codec_available(e_rcvG722124))
						{
							m.CodecType = e_rcvG722124;
							m.PayloadType = SDP_PT_DYNAMIC_G722124;
							m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG722124);
						}
						else
						{
							continue;
						}
						break;
					default:
						continue;
						break;
					}
				}
				else
				{
					continue;
				}

				/*
				// Old codec selection code
				unsigned int value = caps->maxBitRate.value;
				if (maxBr && maxBr != value)
					continue;

				if (value == 480)
				{
					m.CodecType = e_rcvSIREN14_48;
					m.PayloadType = SDP_PT_DYNAMIC_SIREN14_48k;
				}
				else if (value == 320)
				{
					m.CodecType = e_rcvSIREN14_32;
					m.PayloadType = SDP_PT_DYNAMIC_SIREN14_32k;
				}
				else if (value == 240)
				{
					m.CodecType = e_rcvSIREN14_24;
					m.PayloadType = SDP_PT_DYNAMIC_SIREN14_24k;
				}
				else if (value == 32000)
				{
					m.CodecType = e_rcvG722132;
					m.PayloadType = SDP_PT_DYNAMIC_G722132;
				}
				else
				{
					m.CodecType = e_rcvG722124;
					m.PayloadType = SDP_PT_DYNAMIC_G722124;
				}*/
			}
			else
			{
				continue;
			}
		} else if (ac1->tag==VS_H245AudioCapability::e_g722_64k) {
			m.CodecType = e_rcvG722_64k;
			m.PayloadType = SDP_PT_G722_64k;
			m.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, e_rcvG722_64k);
		} else {
			continue;
		}

		if (m.CodecType == e_rcvNone || m.PayloadType == SDP_PT_INVALID)
			continue;

		dprint2("recv TCS audio_cap: codec=%d, pt=%d, initNAT=%d\n",m.CodecType,m.PayloadType,m.InitializeNAT);
		mode = m;
		return cte.capabilityTableEntryNumber.value;
	}

	return 0;
}

unsigned VS_H323Parser::FindVideoCapability(VS_H245VideoCapability* our_cap, VS_H245TerminalCapabilitySet* tcs, VS_GatewayVideoMode& mode,
									   const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (!our_cap)
		return 0;

	for (auto& cte: tcs->capabilityTable)
	{
		if (!cte.capabilityTableEntryNumber.filled || !cte.capability.filled)
		{
			continue;
		}

		if (cte.capability.tag != VS_H245Capability::e_receiveVideoCapability &&
		    cte.capability.tag != VS_H245Capability::e_receiveAndTransmitVideoCapability)
		{
			continue;
		}

		VS_H245VideoCapability* peer_cap = cte.capability;
		if (!peer_cap || peer_cap->tag != our_cap->tag)
		{
			continue;
		}
		if (!FillVideoMode(our_cap, peer_cap, mode, ctx))
			continue;

		dprint2("recv TCS video_cap: codec=%d, pt=%d, mode=%d\n", mode.CodecType, mode.PayloadType, mode.Mode);
		ctx->SetVCDefault(*peer_cap);
		return cte.capabilityTableEntryNumber.value;
	}
	return 0;
}

unsigned VS_H323Parser::FindSlidesVideoCapability(VS_H245VideoCapability* our_cap, VS_H245TerminalCapabilitySet* tcs, VS_GatewayVideoMode& mode, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (!our_cap)
		return 0;

	for (auto& cte: tcs->capabilityTable)
	{
		if (!cte.capabilityTableEntryNumber.filled || !cte.capability.filled)
			continue;
		if (cte.capability.tag != VS_H245Capability::e_receiveVideoCapability && cte.capability.tag != VS_H245Capability::e_receiveAndTransmitVideoCapability)
			continue;
		VS_H245VideoCapability* peer_cap = cte.capability;
		if (!peer_cap || peer_cap->tag != VS_H245VideoCapability::e_extendedVideoCapability)
			continue;
		VS_H245ExtendedVideoCapability* evc = *peer_cap;
		if (!evc || evc->videoCapability.empty() || evc->videoCapabilityExtension.empty())
			continue;
		if (evc->videoCapability[0].tag != our_cap->tag)
			continue;
		if (!CheckH245CapabilityIdentifier(evc->videoCapabilityExtension[0].capabilityIdentifier, oid_h239_evc) || evc->videoCapabilityExtension[0].collapsing.empty())
			continue;
		auto role_param = GetH245ParameterValue_booleanArray(evc->videoCapabilityExtension[0].collapsing[0], 1);
		if (!role_param || (*role_param & 0x1) == 0)
			continue;

		if (!FillVideoMode(our_cap, &evc->videoCapability[0], mode, ctx))
			continue;

		dprint2("recv TCS slides_cap: codec=%d, pt=%d, mode=%d\n", mode.CodecType, mode.PayloadType, mode.Mode);
		ctx->SetVSCDefault(evc->videoCapability[0]);
		return cte.capabilityTableEntryNumber.value;
	}
	return 0;
}

unsigned VS_H323Parser::FindDataCapability(VS_H245DataApplicationCapability* our_cap, VS_H245TerminalCapabilitySet* tcs, VS_GatewayDataMode& mode, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (!our_cap)
		return 0;

	for (auto& cte : tcs->capabilityTable)
	{
		if (!cte.capabilityTableEntryNumber.filled || !cte.capability.filled)
			continue;
		if (cte.capability.tag != VS_H245Capability::e_receiveAndTransmitDataApplicationCapability &&
			cte.capability.tag != VS_H245Capability::e_receiveDataApplicationCapability)
			continue;
		VS_H245DataApplicationCapability* peer_cap = cte.capability;
		if (!peer_cap || peer_cap->application.tag != VS_H245DataApplicationCapability_Application::e_h224)
			continue;

		VS_H245DataProtocolCapability *our_dpc = dynamic_cast<VS_H245DataProtocolCapability*>(our_cap->application.choice);
		VS_H245DataProtocolCapability *peer_dpc = dynamic_cast<VS_H245DataProtocolCapability*>(peer_cap->application.choice);
		if (!our_dpc || !peer_dpc)
			continue;

		if (our_dpc->tag != peer_dpc->tag)
			continue;

		mode.CodecType = VS_H323DataCodec::FECC;
		mode.ExtendedCodec = true;
		mode.PayloadType = SDP_PT_DYNAMIC_H224;
		mode.BitRate = peer_cap->maxBitRate.value * 100;
		mode.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::application_fecc, static_cast<int>(VS_H323DataCodec::FECC));


		dprint2("recv TCS data_cap: codec=%d, pt=%d\n", (int)mode.CodecType, mode.PayloadType);
		ctx->SetDCDefault(*peer_cap);
		return cte.capabilityTableEntryNumber.value;
	}

	return 0;
}

bool VS_H323Parser::FillVideoMode(VS_H245VideoCapability* ourCap, VS_H245VideoCapability* peerCap, VS_GatewayVideoMode& mode, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (peerCap->tag == VS_H245VideoCapability::e_h261VideoCapability)
	{
			//VS_H245H261VideoCapability * h261_me =
			//    (VS_H245H261VideoCapability*)vc[i]->choice;
			//

			//VS_H245H261VideoCapability * h261_other =
			//    (VS_H245H261VideoCapability*)vcap->choice;
			//if (h261_me && h261_other)
			//{
			//    TestBandwith( h261_me->maxBitRate.value,
			//                h261_other->maxBitRate.value);
			//}
		VS_H245H261VideoCapability* h261 = static_cast<VS_H245H261VideoCapability*>(peerCap->choice);
		mode.CodecType = e_videoH261;
		mode.PayloadType = SDP_PT_H261;
		mode.Mode
			= (h261->qcifMPI.value != 0) * 0x02
			| (h261->cifMPI.value != 0)  * 0x04;
		mode.Bitrate = h261->maxBitRate.value * 100;
		mode.CodecAnexInfo = 0;
		mode.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::video, e_videoH261);
	}
	else if (peerCap->tag == VS_H245VideoCapability::e_h263VideoCapability)
	{
		VS_H245H263VideoCapability* our_h263 = dynamic_cast<VS_H245H263VideoCapability*>(ourCap->choice);
		VS_H245H263VideoCapability* h263 = static_cast<VS_H245H263VideoCapability*>(peerCap->choice);
		if (!our_h263)
			return false;
		// skip if our h263++ (his not)
		if (our_h263->h263Options.h263Version3Options.filled && !h263->h263Options.h263Version3Options.filled)
			return false;
		// skip if our h263+ (his not)
		if (our_h263->h263Options.filled && !h263->h263Options.filled)
			return false;

		if (h263->h263Options.h263Version3Options.filled)
		{
			mode.CodecType = e_videoH263plus2;
			mode.PayloadType = SDP_PT_DYNAMIC_H263plus2;
			mode.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::video, e_videoH263plus2);
		}
		else if (h263->h263Options.filled)
		{
			mode.CodecType = e_videoH263plus;
			mode.PayloadType = SDP_PT_DYNAMIC_H263plus;
			mode.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::video, e_videoH263plus);
		}
		else
		{
			mode.CodecType = e_videoH263;
			mode.PayloadType = SDP_PT_H263;
			mode.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::video, e_videoH263);
		}
		mode.Mode
			= (h263->sqcifMPI.value != 0) * 0x01
			| (h263->qcifMPI.value != 0)  * 0x02
			| (h263->cifMPI.value != 0)   * 0x04
			| (h263->cif4MPI.value != 0)  * 0x08
			| (h263->cif16MPI.value != 0) * 0x10;
		mode.Bitrate = h263->maxBitRate.value * 100;
		mode.CodecAnexInfo = 0;
	}
	else if (peerCap->tag == VS_H245VideoCapability::e_genericVideoCapability)
	{
		VS_H245GenericCapability* h264 = static_cast<VS_H245GenericCapability*>(peerCap->choice);

		if (!CheckH245CapabilityIdentifier(h264->capabilityIdentifier, oid_h264_gc))
			return false;

		mode.CodecType = e_videoH264;
		mode.PayloadType = SDP_PT_DYNAMIC_H264;	// todo(kt): fill it at OLC
		mode.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::video, e_videoH264);

		for (VS_H245GenericParameter& p: h264->collapsing)
		{
			if (p.parameterIdentifier.tag != VS_H245ParameterIdentifier::e_standard)
				continue;
			VS_AsnInteger* id = dynamic_cast<VS_AsnInteger*>(p.parameterIdentifier.choice);
			VS_AsnInteger* v = dynamic_cast<VS_AsnInteger*>(p.parameterValue.choice);
			if (!id || !v)
				continue;

			if (id->value == H264_CODEC_PARAMETER_LEVEL)
			{
				int level = 12; // default level = 1.2
				switch (v->value)
				{
				case 15: level = 10; break;
				case 22: level = 11; break;
				case 29: level = 12; break;
				case 36: level = 13; break;
				case 43: level = 20; break;
				case 50: level = 21; break;
				case 57: level = 22; break;
				case 64: level = 30; break;
				case 71: level = 31; break;
				case 78: level = 32; break;
				case 85: level = 40; break;
				case 92: level = 41; break;
				case 99: level = 42; break;
				case 106: level = 50; break;
				case 113: level = 51; break;
				}
				mode.Mode = level;
			}
			else if (id->value == H264_CODEC_PARAMETER_CUSTOMMBPS)
			{
				mode.MaxMbps = v->value * 500; // kt: e maximum macroblock processing rate, in 500 units of macroblocks per seconds
			}
			else if (id->value == H264_CODEC_PARAMETER_CUSTOMFS)
			{
				mode.MaxFs = v->value * 256;	 // kt: maximum frame size, in 256 luma macroblocks
			}
		}

			// AdoptH264Capability
			//unsigned int h264_index = GetVideoIndex(e_videoH264);
			//vc[h264_index]->
			//h264->maxBitRate.value

		VS_H245Data* h245params = ctx->GetH245Params();
		mode.Bitrate = (h264->maxBitRate.value) ? h264->maxBitRate.value * 100 : h245params->m_videoMaxBitrate * 1000;
		mode.CodecAnexInfo = 0;
	}
	else
		return false;
	return true;
}

void VS_H323Parser::MakeCloseLogicalChannel(const std::uint32_t num, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245CloseLogicalChannel* clc = new VS_H245CloseLogicalChannel;
	clc->forwardLogicalChannelNumber.value = num;
	clc->forwardLogicalChannelNumber.filled = true;
	clc->filled = true;

	VS_AsnNull* null1 = new VS_AsnNull;
	null1->filled = true;
	clc->source.tag = VS_H245CloseLogicalChannel_Source::e_user;
	clc->source.choice = null1;
	clc->source.filled = true;

	VS_AsnNull* null2 = new VS_AsnNull;
	null2->filled = true;
	clc->reason.tag = VS_H245CloseLogicalChannel_Reason::e_unknown;
	clc->reason.choice = null2;
	clc->reason.filled = true;

	VS_H245RequestMessage* req = new VS_H245RequestMessage;
	req->filled = true;
	req->choice = clc;
	req->tag = VS_H245RequestMessage::e_closeLogicalChannel;

	VS_H245MultimediaSystemControlMessage mscm;
	mscm.choice = req;
	mscm.filled = true;
	mscm.tag = VS_H245MultimediaSystemControlMessage::e_request;

	PutH245Message(mscm, ctx);
}


bool VS_H323Parser::CreateOLCA(std::uint32_t number, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245LogicalChannelInfo info;
	if (!ctx->GetH245LogicalChannel(number, info))
		return false;

	VS_H245UnicastAddress *mc_ua = new VS_H245UnicastAddress();
	set_ip_address(*mc_ua, info.m_our_rtp_address.address(), info.m_our_rtp_address.port());

	VS_H245UnicastAddress *mcc_ua = new VS_H245UnicastAddress();
	set_ip_address(*mcc_ua, info.m_our_rtcp_address.address(), info.m_our_rtcp_address.port());

	VS_GwH245OpenLogicalChannelAck *olca = new VS_GwH245OpenLogicalChannelAck();
	olca->forwardLogicalChannelNumber.value = number;
	olca->forwardLogicalChannelNumber.filled = true;


	const bool master = ctx->GetH245Params()->m_msd_type == 1;
	if (master) {
		auto h235_caps_it = ctx->channels_h235caps.find(number);
		if (h235_caps_it != ctx->channels_h235caps.cend() && h235_caps_it->second.m != encryption_mode::no_encryption)
		{
			if (ctx->h235_auth.BuildEncryptionSync(olca->encryptionSync, h235_caps_it->second))
			{
				dstream3 << "H235 Session key was successfully encrypted and sended for channel no='" << olca->forwardLogicalChannelNumber.value << "'.\n";
			}
		}
	}

	VS_H245H2250LogicalChannelAckParameters *h2250_lcap = new VS_H245H2250LogicalChannelAckParameters;
	h2250_lcap->sessionID.value = info.m_sessionID;
	h2250_lcap->sessionID.filled = true;
	h2250_lcap->mediaChannel = mc_ua;

	h2250_lcap->mediaControlChannel = mcc_ua;
	h2250_lcap->filled = true;

	olca->forwardMultiplexAckParameters = h2250_lcap;
	olca->filled = true;

	VS_H245ResponseMessage *rm = new VS_H245ResponseMessage();
	*rm = static_cast<VS_H245OpenLogicalChannelAck *>(olca);

	VS_H245MultimediaSystemControlMessage mscm_olca;
	mscm_olca = rm;

	if (PutH245Message(mscm_olca, ctx))
	{
		if (!ctx->SetH245LogicalChannel(number, info))
			return false;

		VS_H245Data* h245params = ctx->GetH245Params();
		switch (info.m_dataType)
		{
		case e_audio:
			ctx->SetRecvAudioReady(true);
			h245params->H245Flags[VS_H323ParserInfo::e_olc_audio] |= VS_H245Data::h245_rsp_send;
			break;
		case e_video:
			ctx->SetRecvVideoReady(true);
			h245params->H245Flags[VS_H323ParserInfo::e_olc_video] |= VS_H245Data::h245_rsp_send;
			break;
		case e_slides:
			//ctx->SetRecvSlidesReady(true);
			h245params->H245Flags[VS_H323ParserInfo::e_olc_slides] |= VS_H245Data::h245_rsp_send;
			break;
		case e_data:
			h245params->H245Flags[VS_H323ParserInfo::e_olc_data] |= VS_H245Data::h245_rsp_send;
			break;
		}
		return true;
	}
	return false;
}

bool VS_H323Parser::UpdateCallState(const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (!ctx)
		return false;
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return false;

	if (ctx->GetCallDirection() == e_in)
	{
		{
			//VS_AutoLock lock(ctx.get()); //TODO:FIXME
			if (ctx->IsInDialog())
				return true;
			ctx->SetInDialog(true);
		}
		auto &dialog_id = ctx->GetDialogID();
		// If this is call through gatekeeper, required early by ARQ, we should use
		// h323-id as TC-user call_id. Otherwise - this is h323 call to default destination.
		std::shared_ptr<VS_H323GatekeeperStorage::Info> info =
			VS_H323GatekeeperStorage::Instance().GetTerminalInfo(ctx->GetCallIdentifier());
		VS_H323GatekeeperStorage::Instance().RemoveTerminalInfo(ctx->GetCallIdentifier());
		string_view call_to = ctx->GetDstAlias();
		call_to = call_to.substr(0, call_to.find('@'));
		string_view call_to_e164 = ctx->GetDstDigit();

		if ((!call_to.empty() && CheckUserNameInCallConfigStorage(VS_CallConfig::H225RAS, call_to)) ||
			(!call_to_e164.empty() && CheckUserNameInCallConfigStorage(VS_CallConfig::H225RAS, call_to_e164)))
		{
			call_to = DEFAULT_DESTINATION_CALLID_H323;
		}
		if(call_to.empty())
			call_to = ctx->GetDstDigit();
		if(call_to.empty())
			call_to = DEFAULT_DESTINATION_CALLID_H323;
		if(info)
		{
			// Call through gatekeeper.
			if(call_to.empty())
				return false;
			std::shared_ptr<VS_H225RASParserInfo> reg_ctx = info->context.lock();
			if(reg_ctx)
			{
				confMethods->PutSharedTranscoder(dialog_id, reg_ctx->GetTranscoder().lock());
			}
		}

		if (VS_IsRTPCallID(call_to)) {
			const auto& config = ctx->GetConfig();
			bool isAuthorizedCall = config.isAuthorized.get_value_or(false);
			if (!isAuthorizedCall) {
				dstream2 << "Call from " << ctx->GetAliasRemote() << " to " << call_to << " suppressed. Call from sip to third-party protocol (sip/h323/rtsp)\n";
				MakeReleaseComplete(ctx);
				return false;
			}
		}

		bool res = confMethods->InviteMethod(dialog_id, ctx->GetAliasRemote(), call_to, VS_ConferenceInfo(false, false), ctx->GetDisplayNamePeer());
		if (!res)
		{
			dprint3("VS_H323Parser::UpdateCallState(): call to m_confMethods->InviteMethod() failed! sending releaseComplete!\n");
			MakeReleaseComplete(ctx);
		}
		return res;
	}

		if (ctx->IsReady())
		{
			{
			//TODO:FIXME
			//VS_AutoLock lock(ctx.get());
				if (ctx->IsInDialog())
					return true;
				ctx->SetInDialog(true);
				SendDTMF(ctx);
			}
			return confMethods->InviteReplay(ctx->GetDialogID(), e_call_ok, false, {}, {});
		}

	return true;
}

bool VS_H323Parser::TryStartDialog(const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
// 	if (!ctx)
// 		return false;
//
// 	if (ctx->IsInDialog())
// 		return false;
//
// 	if (ctx->IsParamsReady() && ctx->IsOLCTimeout(GetTickCount()))
// 	{
// 		VS_GatewayMediaModes* modes = ctx->GetMediaModes();
// 		VS_SIPMediaChannelInfo* a_info = ctx->GetAudioInfo();
// 		VS_SIPMediaChannelInfo* v_info = ctx->GetVideoInfo();
// 		if (m_confMethods->SetMediaChannelInfo(ctx->GetDialogID(), a_info, v_info, modes))
// 		{
// 			ctx->SetInDialog(true);
//
// 			if (ctx->GetCallDirection() == e_in)
// 			{
// 				char* from_id = ctx->GetQ931DisplayName();
// 				char* dialog_id = ctx->GetDialogID();
// 				return m_confMethods->InviteMethod(dialog_id, from_id, "test@192.168.0.102", false);
// 			}
// 			else
// 			{
// 				return m_confMethods->InviteReplay(ctx->GetDialogID(), e_call_ok, false);
// 			}
// 		}
// 	}
	return false;
}

// void VS_H323Parser::TestDialogsReady()
// {
// 	VS_AutoLock lock(&m_ctx_lock);
// 	std::map<VS_SimpleStr, std::shared_ptr<VS_H323ParserInfo>>::iterator it;
// 	for (it = m_ctx.begin(); it != m_ctx.end(); it++)
// 	{
// 		TryStartDialog(it->second);
// 	}
// }

unsigned int VS_H323Parser::GetAudioIndex( const int value )
{
	return GetCodecIndex( value );
}

unsigned int VS_H323Parser::GetVideoIndex( const int value )
{
	return GetCodecIndex( value );
}

unsigned int VS_H323Parser::GetCodecIndex( const int value ) const
{
	int index = value;
	int i = 0;
	if (index==0) return 10000;
	while(!(index & 0x01 ))
	{
		i++;
		index = index>>1;
	}
	return i;
}

bool VS_H323Parser::RecvMSDA(VS_H245MasterSlaveDeterminationAck * msda, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	if (!msda)
		return false;

	VS_H245Data* h245params = ctx->GetH245Params();
	h245params->H245Flags[VS_H323ParserInfo::e_msd] |= VS_H245Data::h245_rsp_recv;
	MSDState state = ctx->MsdState();
	if (state == MSDOutgoingAwaitingResponse)
	{
		ctx->StopMSDTimer();
		ctx->SetMsdState(MSDIdle);
		// A remote side sends us a reverse of its decision. So, basically, it sends us our decision.
		int our_decision = msda->decision.tag;
		h245params->m_msd_type = (our_decision == VS_H245MasterSlaveDeterminationAck_Decision::e_master) ? 1 : 0;
		return SendMSDA(ctx, our_decision);
	} else if (state == MSDIncomingAwaitingResponse)
	{
		ctx->StopMSDTimer();
		ctx->SetMsdState(MSDIdle);

		int recv_decision = msda->decision.tag;
		int our_decision = DetermineMSDStatus(ctx);
		if (recv_decision != our_decision)
		{
			dprint3("VS_H323Parser::RecvMSDA: MSDError (E) inconsistent field value (decisions does not match)\n");
			TerminateSession(ctx);
		}
		h245params->m_msd_type = (our_decision == VS_H245MasterSlaveDeterminationAck_Decision::e_master) ? 1 : 0;
	}

	//if (h245params->H245Flags[VS_H323ParserInfo::e_msd] & h245params->h245_req_send)
	//	return SendMSDA(ctx);

	return true;
}

bool VS_H323Parser::SendVFUP(std::uint32_t number, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245MultimediaSystemControlMessage mscm;
	VS_H245CommandMessage* m = new VS_H245CommandMessage;

	VS_H245MiscellaneousCommand* misc = new VS_H245MiscellaneousCommand;
	misc->type.tag = VS_H245MiscellaneousCommand_Type::e_videoFastUpdatePicture;
	misc->type.choice = new VS_AsnNull;
	misc->type.filled = true;
	misc->logicalChannelNumber.value = number;
	misc->logicalChannelNumber.filled = true;
	misc->filled = true;

	m->tag = VS_H245CommandMessage::e_miscellaneousCommand;
	m->choice = misc;
	m->filled = true;

	mscm.tag = VS_H245MultimediaSystemControlMessage::e_command;
	mscm.choice = m;
	mscm.filled = true;

	return PutH245Message(mscm, ctx);
}

void InitLogicalChannelGenericDataParams(VS_H245DataType &dataType, const VS_Asn * data_capability, const encryption_mode m, const unsigned media_type, const std::shared_ptr<VS_H323ParserInfo> &ctx){
	if (m != encryption_mode::no_encryption){
		dataType.choice = ctx->h235_auth.InitGenericH235Media(data_capability, media_type, m);
		dataType.tag = VS_H245DataType::e_h235Media;
	}
	else{
		dataType.choice = const_cast<VS_Asn *>(data_capability);
		dataType.tag = media_type;
	}
	dataType.filled = true;
}

bool VS_H323Parser::SendOLC(std::uint32_t number, const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245Data* h245params = ctx->GetH245Params();

	VS_H245LogicalChannelInfo info;
	if (!ctx->GetH245LogicalChannel(number, info))
	{
		TerminateSession(ctx);
		return false;
	}

	VS_H323ParserInfo::VS_CapabilityExchangeStages stage;
	switch (info.m_dataType)
	{
	case e_audio:
		stage = VS_H323ParserInfo::e_olc_audio;
		break;
	case e_video:
		stage = VS_H323ParserInfo::e_olc_video;
		break;
	case e_slides:
		stage = VS_H323ParserInfo::e_olc_slides;
		break;
	case e_data:
		stage = VS_H323ParserInfo::e_olc_data;
		break;
	default:
		return false;
	}

	if ((h245params->H245Flags[stage] & VS_H245Data::h245_req_send))
		return true;

	if (info.m_our_rtp_address.port() == 0 || info.m_our_rtcp_address.port() == 0)
	{
		if (ctx->GetLCPending(number))
			return true;
		ctx->SetLCPending(number, true);
		SendSetMediaChannels(ctx);
		return true;
	}

	VS_H245UnicastAddress *ua = new VS_H245UnicastAddress();
	set_ip_address(*ua, info.m_our_rtcp_address.address(), info.m_our_rtcp_address.port());

	std::unique_ptr<VS_H245OpenLogicalChannel> olc = vs::make_unique<VS_H245OpenLogicalChannel>();

	olc->forwardLogicalChannelNumber.value = number;
	olc->forwardLogicalChannelNumber.filled = true;

	VS_H245H2250LogicalChannelParameters* h2250_lcp = new VS_H245H2250LogicalChannelParameters;

	h2250_lcp->sessionID.value = info.m_sessionID;
	h2250_lcp->sessionID.filled = true;

	h2250_lcp->mediaControlChannel.choice = ua;
	h2250_lcp->mediaControlChannel.tag = VS_H245TransportAddress::e_unicastAddress;
	h2250_lcp->mediaControlChannel.filled = true;
	h2250_lcp->filled = true;

	olc->forwardLogicalChannelParameters.multiplexParameters.choice = h2250_lcp;
	olc->forwardLogicalChannelParameters.multiplexParameters.tag = VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters::e_h2250LogicalChannelParameters;
	olc->forwardLogicalChannelParameters.multiplexParameters.filled = true;

	encryption_mode encryption = encryption_mode::no_encryption;
	char pt = SDP_PT_INVALID;
	switch (info.m_dataType)
	{
	case e_audio:
	{
		VS_H245AudioCapability** ac = ctx->GetAudioCapability();
		unsigned ac_index = ctx->GetACDefault();
		auto codec = static_cast<VS_H323AudioCodec>(VS_H323ParserInfo::GetCodecID(ac_index));
		pt = VS_H323ParserInfo::GetACPayloadType(codec);
		if (pt == SDP_PT_INVALID)
			pt = VS_H323ParserInfo::GetACDynamicPayloadType(codec);

		VS_H245AudioCapability* ac_temp = new VS_H245AudioCapability;
		*ac_temp = *ac[ac_index];

		// Siren 14
		{
			const VS_AsnObjectId oid_SIREN14(id_SIREN14, sizeof(id_SIREN14) / sizeof(id_SIREN14[0]));
			VS_H323_GenericCapabilityGenerator gen(oid_SIREN14);
			unsigned siren14_32_index = ctx->GetAudioIndex(e_rcvSIREN14_32),
				siren14_24_index = ctx->GetAudioIndex(e_rcvSIREN14_24),
				siren14_48_index = ctx->GetAudioIndex(e_rcvSIREN14_48);

			auto generate_siren_audio_capability = [&]() {
				ac_temp->FreeChoice();
				ac_temp->choice = gen.Generate();
				if (ac_temp->choice)
				{
					ac_temp->filled = true;
					ac_temp->tag = VS_H245AudioCapability::e_genericAudioCapability;
				}
			};

			if (siren14_24_index <= ac_number && siren14_24_index == ac_index)
			{
				gen.SetMaxBitRate(240);
				gen.AddStandartIntParametr(true,
					VS_H245ParameterValue::e_unsignedMin, 1);
				gen.AddStandartIntParametr(true,
					VS_H245ParameterValue::e_booleanArray, e_SIREN14_24K);
				generate_siren_audio_capability();
			}
			else if (siren14_32_index <= ac_number && siren14_32_index == ac_index)
			{
				gen.SetMaxBitRate(320);
				gen.AddStandartIntParametr(true,
					VS_H245ParameterValue::e_unsignedMin, 1);
				gen.AddStandartIntParametr(true,
					VS_H245ParameterValue::e_booleanArray, e_SIREN14_32K);
				generate_siren_audio_capability();
			}
			else if (siren14_48_index <= ac_number && siren14_48_index == ac_index)
			{
				gen.SetMaxBitRate(480);
				gen.AddStandartIntParametr(true,
					VS_H245ParameterValue::e_unsignedMin, 1);
				gen.AddStandartIntParametr(true,
					VS_H245ParameterValue::e_booleanArray, e_SIREN14_48K);
				generate_siren_audio_capability();
			}
		}
		encryption = ctx->audio_channel.snd_mode_audio.sec_cap.m;
		InitLogicalChannelGenericDataParams(olc->forwardLogicalChannelParameters.dataType, ac_temp, encryption, VS_H245DataType::e_audioData, ctx);
		FillACMediaPacketization(codec, pt, &h2250_lcp->mediaPacketization);
	}
		break;
	case e_video:
	{
		if (!ctx->HasVCDefault())
			return false;

		VS_H245VideoCapability* vc_default = ctx->GetVCDefault();
		auto codec = GetVideoCapabilityCodec(vc_default);
		pt = GetVCPayloadType(codec);

		VS_H245VideoCapability* vc_temp = new VS_H245VideoCapability;
		*vc_temp = *vc_default;

		encryption = ctx->video_channel.snd_mode_video.sec_cap.m;
		InitLogicalChannelGenericDataParams(olc->forwardLogicalChannelParameters.dataType, vc_temp, encryption, VS_H245DataType::e_videoData, ctx);
		FillVCMediaPacketization(codec, pt, &h2250_lcp->mediaPacketization);
	}
		break;
	case e_slides:
	{
		if (!ctx->HasVSCDefault())
			return false;

		VS_H245VideoCapability* vsc_default = ctx->GetVSCDefault();
		auto codec = GetVideoCapabilityCodec(vsc_default);
		pt = GetVCPayloadType(codec);

		VS_H245VideoCapability* vc_temp = new VS_H245VideoCapability;
		VS_H245ExtendedVideoCapability* evc = new VS_H245ExtendedVideoCapability;

		evc->videoCapability.reset(new VS_H245VideoCapability[1], 1);
		evc->videoCapability[0] = *vsc_default;
		evc->videoCapability.filled = true;

		evc->videoCapabilityExtension.reset(new VS_H245GenericCapability[1], 1); // h239ExtendedVideoCapability
		SetH245CapabilityIdentifier(evc->videoCapabilityExtension[0].capabilityIdentifier, oid_h239_evc);

		evc->videoCapabilityExtension[0].collapsing.reset(new VS_H245GenericParameter[1], 1); // roleLabel
		SetH245ParameterValue_booleanArray(evc->videoCapabilityExtension[0].collapsing[0], 1, 1); // Presentation role
		evc->videoCapabilityExtension[0].collapsing.filled = true;

		evc->videoCapabilityExtension[0].filled = true;
		evc->videoCapabilityExtension.filled = true;
		evc->filled = true;

		vc_temp->choice = evc;
		vc_temp->tag = VS_H245VideoCapability::e_extendedVideoCapability;
		vc_temp->filled = true;

		encryption = ctx->slides_channel.snd_mode_video.sec_cap.m;
		InitLogicalChannelGenericDataParams(olc->forwardLogicalChannelParameters.dataType, vc_temp, encryption, VS_H245DataType::e_videoData, ctx);
		FillVCMediaPacketization(codec, pt, &h2250_lcp->mediaPacketization);
	}
		break;
	case e_data:
	{
		if (!ctx->HasDCDefault())
			return false;

		VS_H245DataApplicationCapability* dc_default = ctx->GetDCDefault();
		auto codec = GetDataCapabilityCodec(dc_default);
		pt = GetDCPayloadType(codec);

		VS_H245DataApplicationCapability* dc_temp = new VS_H245DataApplicationCapability;
		VS_H245DataProtocolCapability* dpc = new VS_H245DataProtocolCapability;

		dpc->choice = new VS_AsnNull;
		dpc->tag = VS_H245DataProtocolCapability::e_hdlcFrameTunnelling;
		dpc->filled = true;

		dc_temp->application.choice = dpc;
		dc_temp->application.tag = VS_H245DataApplicationCapability_Application::e_h224;
		dc_temp->application.filled = true;
		dc_temp->maxBitRate.value = 50;
		dc_temp->maxBitRate.filled = true;
		dc_temp->filled = true;

		olc->forwardLogicalChannelParameters.dataType.choice = dc_temp;
		olc->forwardLogicalChannelParameters.dataType.tag = VS_H245DataType::e_data;
		olc->forwardLogicalChannelParameters.dataType.filled = true;
		encryption = ctx->data_channel.snd_mode_data.sec_cap.m;
		InitLogicalChannelGenericDataParams(olc->forwardLogicalChannelParameters.dataType, dc_temp, encryption, VS_H245DataType::e_data, ctx);
		// FillDCMediaPacketization(codec, pt, &h2250lcp->mediaPacketization);
	}
	}

	if (pt >= 96)
	{
		h2250_lcp->dynamicRTPPayloadType.value = pt;
		h2250_lcp->dynamicRTPPayloadType.filled = true;
	}

	VS_H235SecurityCapability sec_cap(encryption, pt);
	bool master = ctx->GetH245Params()->m_msd_type == 1;
	if (master && encryption != encryption_mode::no_encryption){
		if (ctx->h235_auth.BuildEncryptionSync(olc->encryptionSync, sec_cap)){
			dstream3 << "H235 Session key was successfully encrypted for channel no='" << olc->forwardLogicalChannelNumber.value << "'.\n";
		}
	}
	ctx->channels_h235caps.emplace(olc->forwardLogicalChannelNumber.value, sec_cap);

	olc->forwardLogicalChannelParameters.filled = true;
	olc->filled = true;

	VS_H245RequestMessage* rm = new VS_H245RequestMessage;
	rm->choice = olc.release();
	rm->tag = VS_H245RequestMessage::e_openLogicalChannel;
	rm->filled = true;

	VS_H245MultimediaSystemControlMessage mscm;
	mscm.choice = rm;
	mscm.tag = VS_H245MultimediaSystemControlMessage::e_request;
	mscm.filled = true;

	if (PutH245Message(mscm, ctx))
	{
		if (!ctx->SetH245LogicalChannel(number, info))
			return false;

		h245params->H245Flags[stage] |= VS_H245Data::h245_req_send;
		dstream3 << "Sent OLC with number ='" << number << "'\n";
		return true;
	}

	dstream3 << "OLC with number ='" << number << "' wasn't send!\n";
	return false;
}

bool VS_H323Parser::MakeEndSessionCommand(const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	VS_H245MultimediaSystemControlMessage mscm;
	mscm.tag = VS_H245MultimediaSystemControlMessage::e_command;

	VS_H245CommandMessage* cmd = new VS_H245CommandMessage;
	cmd->tag = VS_H245CommandMessage::e_endSessionCommand;

	VS_H245EndSessionCommand* esc = new VS_H245EndSessionCommand;
	esc->tag = VS_H245EndSessionCommand::e_disconnect;

	VS_AsnNull* nuller = new VS_AsnNull;
	nuller->filled = true;

	esc->choice = nuller;
	esc->filled = true;

	cmd->choice = esc;
	cmd->filled = true;

	mscm.choice = cmd;
	mscm.filled = true;

	return PutH245Message(mscm, ctx);
}

bool VS_H323Parser::MakeReleaseComplete(const std::shared_ptr<VS_H323ParserInfo>& ctx, int term_reason)
{
	//if ( !isValid() )
	//	return false;

	const bool isVisicronToH323 = !!ctx->GetCallDirection();

	VS_CsReleaseCompleteUuie* rc = new VS_CsReleaseCompleteUuie;
	rc->protocolIdentifier = h225ProtocolIdentifier;
	rc->filled = true;
	rc->reason.filled = false;

	if (term_reason != -1)
		rc->reason.tag = term_reason;

	VS_CsH323UserInformation ui;
	ui.h323UuPdu.filled = true;
	ui.h323UuPdu.h323MessageBody = rc;
	ui.filled = true;

	VS_PerBuffer ui_buf;
	if ( !ui.Encode(ui_buf) )
		return false;

	VS_Q931 q931;
	q931.fromDestination = !isVisicronToH323;	// Мы не destination
	q931.callReference = ctx->GetCRV();
	q931.messageType = VS_Q931::e_releaseCompleteMsg;

	VS_PerBuffer q931_buf;
	if ( !q931.EncodeMHeader(q931_buf) )
		return false;

	if ( !VS_Q931::SetUserUserIE(q931_buf, ui_buf) )
		return false;

	void* data = q931_buf.GetData();
	const auto sz = q931_buf.ByteSize();

	auto odata = vs::make_unique_default_init<unsigned char[]>(sz);
	memcpy(odata.get(), data, sz);

	return PutOutputMessage(std::move(odata), sz, e_H225);
}

bool VS_H323Parser::MakeStatus(const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	//if ( !isValid() )
	//	return false;

	const bool isVisicronToH323 = !!ctx->GetCallDirection();

	VS_H225Status_UUIE* status = new VS_H225Status_UUIE;
	status->protocolIdentifier = h225ProtocolIdentifier;
	status->filled = true;
	unsigned char raw_call_id[CONFERENCEID_LENGTH];
	DecodeDialogID(ctx->GetCallIdentifier().c_str(), raw_call_id);
	VS_BitBuffer callIdentifier(raw_call_id, CONFERENCEID_LENGTH * 8);
	status->callIdentifier.guid.value = callIdentifier;
	status->callIdentifier.guid.filled = true;
	status->callIdentifier.filled = true;

	VS_CsH323UserInformation ui;
	ui.h323UuPdu.filled = true;
	ui.h323UuPdu.h323MessageBody = status;
	ui.filled = true;

	VS_PerBuffer ui_buf;
	if (!ui.Encode(ui_buf))
		return false;

	VS_Q931 q931;
	q931.fromDestination = !isVisicronToH323;	// Мы не destination
	q931.callReference = ctx->GetCRV();
	q931.messageType = VS_Q931::e_statusMsg;

	VS_PerBuffer q931_buf;
	if (!q931.EncodeMHeader(q931_buf))
		return false;

	if (!VS_Q931::SetUserUserIE(q931_buf, ui_buf))
		return false;

	void* data = q931_buf.GetData();
	const auto sz = q931_buf.ByteSize();

	auto odata = vs::make_unique_default_init<unsigned char[]>(sz);
	memcpy(odata.get(), data, sz);

	return PutOutputMessage(std::move(odata), sz, e_H225);
}

void VS_H323Parser::OnContextDestructor(string_view dialogId)
{
	std::lock_guard<decltype(m_ctx_garbage_lock)> _{ m_ctx_garbage_lock };
	m_ctx_garbage.emplace_back(dialogId);
}

void VS_H323Parser::HangupCall(string_view dialogId)
{
	std::shared_ptr<VS_H323ParserInfo> ctx = GetParserContext(dialogId);
	if(!ctx)
		return;
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;

	if(ctx->IsGatekeeperCall())
		confMethods->HangupOutcomingCall(dialogId);
	else
		confMethods->Hangup(dialogId);

}

int VS_H323Parser::DetermineMSDStatus(const std::shared_ptr<VS_H323ParserInfo>& ctx)
{
	auto &&h245params = ctx->GetH245Params();

	if (h245params->m_my_msd_type > h245params->m_their_msd_type)
		return VS_H245MasterSlaveDeterminationAck_Decision::e_master;

	if (h245params->m_my_msd_type < h245params->m_their_msd_type)
		return VS_H245MasterSlaveDeterminationAck_Decision::e_slave;

	if (h245params->m_my_msd_type == h245params->m_their_msd_type)
	{
		static const auto module = 1 << 24;
		static const auto bound = 1 << 23;

		const auto result = (h245params->m_their_msd_num - h245params->m_my_msd_num + module) % module;

		if (0 == result || bound == result)
			return VS_H245MasterSlaveDeterminationAck_Decision::e_indeterminate;

		return result < bound ? VS_H245MasterSlaveDeterminationAck_Decision::e_master : VS_H245MasterSlaveDeterminationAck_Decision::e_slave;
	}
	return VS_H245MasterSlaveDeterminationAck_Decision::e_indeterminate;
}

void VS_H323Parser::EncodeDialogID(const unsigned char rawDialogId[CONFERENCEID_LENGTH], char *dialogId)
{
	for (unsigned i = 0; i < CONFERENCEID_LENGTH; i++)
	{
		sprintf(dialogId + i * 2, "%02X", static_cast<unsigned int>(rawDialogId[i]));
	}
}

void VS_H323Parser::DecodeDialogID(const char* dialog_id, unsigned char raw_dialog_id[CONFERENCEID_LENGTH])
{
	auto ch = 0u;
	for(unsigned i = 0; i < CONFERENCEID_LENGTH; i++)
	{
		sscanf(dialog_id + i * 2, "%2X", &ch);
		raw_dialog_id[i] = (char)ch;
	}
}

std::shared_ptr<VS_H323ParserInfo> VS_H323Parser::FindParserInfoByRemoteTarget(string_view remote) {

	if (remote.empty())
		return {};

	std::lock_guard<decltype(m_ctx_lock)> lock(m_ctx_lock);

	for (auto it = m_ctx.cbegin(); it != m_ctx.cend(); ++it)
	{
		std::shared_ptr<VS_H323ParserInfo> ctx = it->second;
		if (ctx && ctx->GetSIPRemoteTarget() == remote)
		{
			return ctx;
		}
	}
	return nullptr;
}

void VS_H323Parser::FastUpdatePicture(string_view dialogId)
{
	dprint3("FastUpdatePicture from SingleGW\n");
	std::shared_ptr<VS_H323ParserInfo> ctx = GetParserContext(dialogId, false);
	if (!ctx)
		return;

	auto h245params = ctx->GetH245Params();
	if (h245params->H245Flags[VS_H323ParserInfo::e_olc_video] & VS_H245Data::h245_rsp_send)
		SendVFUP(h245params->m_videoNumberLCReciver, ctx);
	if (h245params->H245Flags[VS_H323ParserInfo::e_olc_slides] & VS_H245Data::h245_rsp_send)
		SendVFUP(h245params->m_slidesNumberLCReciver, ctx);
}

bool VS_H323Parser::ResolveOnExternalGatekeeper(string_view /*myName*/, string_view /*callId*/, net::address &/*addr*/, net::port &/*port*/)
{
	return false;
}

void VS_H323Parser::Chat(string_view dialogId, const std::string &from, const std::string &to, const std::string &dn, const char *mess)
{
	dstream3 << "VS_H323Parser::Chat: from=" << from << ", to=" << to << ", dn=" << dn << ", mess=" << mess;
}

void VS_H323Parser::Command(string_view dialogId, string_view from, string_view command)
{
	dstream3 << "VS_H323Parser::Command: from=" << from << "cmd=" << command;

	auto ctx = GetParserContext(dialogId);
	if (!ctx)
		return;

	static string_view dtmf_prefix = "#dtmf:";
	if (boost::starts_with(command, dtmf_prefix) && command.length() > dtmf_prefix.length()) {
		command.remove_prefix(dtmf_prefix.length());
		char digit = command[0];
		dprint3("DTMF:%c\n", digit);

		VS_AsnGeneralString* str = new VS_AsnGeneralString;
		str->filled = true;
		str->value.AddBits(digit, 8);

		VS_H245UserInputIndication* input = new VS_H245UserInputIndication;
		input->filled = true;
		input->tag = VS_H245UserInputIndication::e_alphanumeric;
		input->choice = str;

		VS_H245IndicationMessage* ind = new VS_H245IndicationMessage;
		ind->filled = true;
		ind->tag = VS_H245IndicationMessage::e_userInput;
		ind->choice = input;

		VS_H245MultimediaSystemControlMessage   mscm;
		mscm = ind;
		PutH245Message(mscm, ctx);
	} else if (boost::starts_with(command, string_view(SHOW_SLIDE_COMMAND)))
		UpdateSlideshowState(ctx, true);
	else if (boost::starts_with(command, string_view(END_SLIDESHOW_COMMAND)))
		UpdateSlideshowState(ctx, false);
	else if (boost::equals(command, string_view(CONTENTFORWARD_PULL)))
		UpdateSlideshowState(ctx, true);
	else if (boost::equals(command, string_view(CONTENTFORWARD_PUSH)) || boost::equals(command, string_view(CONTENTFORWARD_STOP)))
		UpdateSlideshowState(ctx, false);
}

bool VS_H323Parser::UpdateSlideshowState(const std::shared_ptr<VS_H323ParserInfo>& ctx, bool active)
{
	ctx->SetSlideshowState(active);

	auto* h245params = ctx->GetH245Params();
	if (!(h245params->H245Flags[VS_H323ParserInfo::e_olc_slides] & VS_H245Data::h245_rsp_recv)) // if outgoing channel isn't open
		return false;

	auto token = ctx->GetH239PresentationToken();
	if (token->owned == active)
		return true;

	if (active)
		SendH239PresentationTokenRequest(ctx);
	else
	{
		SendH245LogicalChannelActiveIndication(ctx->GetH245Params()->m_slidesNumberLCSender, false, ctx);
		SendH239PresentationTokenRelease(ctx);
	}
	return true;
}

bool VS_H323Parser::GetAudioMode(string_view dialogId, VS_GatewayAudioMode &res)
{
	auto ctx = GetParserContext(dialogId, false);
	if (ctx == nullptr)
		return false;
	res = ctx->audio_channel.snd_mode_audio;
	return true;
}

bool VS_H323Parser::GetVideoMode(string_view dialogId, VS_GatewayVideoMode &res)
{
	auto ctx = GetParserContext(dialogId, false);
	if (ctx == nullptr)
		return false;
	res = ctx->video_channel.snd_mode_video;
	return true;
}

bool VS_H323Parser::GetDataMode(string_view dialogId, VS_GatewayDataMode &res)
{
	auto ctx = GetParserContext(dialogId, false);
	if (ctx == nullptr)
		return false;
	res = ctx->data_channel.snd_mode_data;
	return true;
}


bool VS_H323Parser::OnFacilityArrived(VS_PerBuffer &aInBuffer, VS_Q931 &aQ931_In, const net::address &fromAddr, net::port fromPort)
{
{
		unsigned char dn[82 + 1] = { 0 };
		unsigned char e164[50] = { 0 };
		// decode message
		if (!VS_Q931::GetUserUserIE(aInBuffer, dn, e164))
			return false;
}

	//VS_CsH323UserInformation ui;
	auto ui = vs::make_unique<VS_CsH323UserInformation>();
	if (!ui->Decode(aInBuffer))
		return false;

	VS_H225Facility_UUIE *facility = static_cast<VS_H225Facility_UUIE *>(ui->h323UuPdu.h323MessageBody.choice);
	auto ctx = GetDefaultParserContext(fromAddr, fromPort);
	if (!ctx)
		return 0;


	// route call to gatekeeper
	if (facility->reason.filled && facility->reason.tag == VS_H225FacilityReason::e_routeCallToGatekeeper)
	{
		VS_H225TransportAddress_IpAddress *ip_addr;

		if (ctx->IsInDialog())
		{
			return true;
		}

		// extract ip
		if (!facility->alternativeAddress.filled ||
			(ip_addr = static_cast<VS_H225TransportAddress_IpAddress *>(facility->alternativeAddress.choice)) == nullptr ||
			!ip_addr->filled)
		{
			HangupCall(ctx->GetDialogID());
			return true;
		}

		if (ip_addr == nullptr)
		{
			HangupCall(ctx->GetDialogID());
			return true;
		}

		// get old peer address
		const auto& old_peer_addr = ctx->GetPeerCsAddress();
		const auto& old_peer_port = ctx->GetPeerCsPort();

		tcp_endpoint_t new_peer_addr(boost::asio::ip::address_v4(*static_cast<uint32_t *>(ip_addr->ip.value.GetData())), static_cast<net::port>(ip_addr->port.value));

		dstream4 << "H225 facility message was received. Forwarding call from:" << old_peer_addr << ":" << old_peer_port << " to gatekeeper at:" << new_peer_addr;

		// connect to new peer
		if (!MakeNewConnection(new_peer_addr, e_H225))
		{
			HangupCall(ctx->GetDialogID());
			return true;
		}

		// close old connection
		CloseConnection(old_peer_addr, old_peer_port, net::protocol::TCP);

		// update peer addr
		//SetPeerCSAddress(ctx->GetDialogID(), new_peer_addr);

		// generate setup
		MakeH225Setup(ctx);

		return true;
	}
	else // wrong/unsupported facility reason
	{
		if (facility->reason.filled)
		{
			dprint4("H225 facility message was received. Unsupported facility reason: %d.\n", facility->reason.tag);
		}

		return true;
	}

	return true;
}

void VS_H323Parser::UseACL(bool use)
{
	m_use_acl = use;
}

bool VS_H323Parser::IsACLUsed(void) const
{
	return m_use_acl && m_acl.GetMode() != VS_NetworkConnectionACL::ACL_NONE;
}

void VS_H323Parser::SendDTMF(const std::shared_ptr<VS_H323ParserInfo> &ctx)
{
	if (!ctx)
	{
		return;
	}
	auto &dtmf = ctx->GetDTMF();
	if (dtmf.empty())
	{
		return;
	}
	dprint3("DTMF:%s\n", dtmf.c_str());
	for (const auto &digit : dtmf)
	{
		VS_AsnGeneralString* str = new VS_AsnGeneralString;
		str->filled = true;
		str->value.AddBits(digit, 8);

		VS_H245UserInputIndication* input = new VS_H245UserInputIndication;
		input->filled = true;
		input->tag = VS_H245UserInputIndication::e_alphanumeric;
		input->choice = str;

		VS_H245IndicationMessage* ind = new VS_H245IndicationMessage;
		ind->filled = true;
		ind->tag = VS_H245IndicationMessage::e_userInput;
		ind->choice = input;

		VS_H245MultimediaSystemControlMessage   mscm;
		mscm = ind;
		PutH245Message(mscm, ctx);
	}
}

#undef NULL_TICK
#undef DEBUG_CURRENT_MODULE