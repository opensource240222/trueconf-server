#include "VS_SIPMessage.h"
#include "VS_SIPMetaField.h"
#include "VS_SDPMetaField.h"
#include "VS_MetaField_MediaControl_XML.h"
#include "VS_MetaField_DTMF_Relay.h"
#include "VS_MetaField_PIDF_XML.h"
#include "VS_SIPURI.h"
#include "VS_SIPField_To.h"
#include "VS_SIPField_From.h"
#include "VS_SIPField_Via.h"
#include "VS_SIPField_Contact.h"
#include "VS_SIPField_CSeq.h"
#include "VS_SIPField_CallID.h"
#include "VS_SIPField_ContentType.h"
#include "VS_SIPField_ContentLength.h"
#include "VS_SIPField_Auth.h"
#include "VS_SIPField_UserAgent.h"
#include "VS_SIPField_SessionExpires.h"
#include "VS_SIPField_StartLine.h"
#include "VS_SDPField_Bandwidth.h"
#include "VS_SDPField_MediaStream.h"
#include "VS_SIPInstantMessage.h"
#include "VS_SIPField_RetryAfter.h"
#include "VS_SIPField_Server.h"
#include "std-generic/compat/memory.h"


VS_SIPMessage::VS_SIPMessage() :m_content_type(CONTENTTYPE_INVALID)
{

}

VS_SIPMessage::VS_SIPMessage(const char* aInput, const std::size_t aSize): m_content_type(CONTENTTYPE_INVALID)
{
	this->Decode(aInput, aSize);
}

VS_SIPMessage::~VS_SIPMessage()
{}

TSIPErrorCodes VS_SIPMessage::Decode(const char* aInput, std::size_t aSize)
{
	if ( !aInput || (aSize < 1) )
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	std::string __buffer;
	if ( CorrectNewLine(aInput, aSize, __buffer) )
	{
		aInput = __buffer.c_str();
		aSize = __buffer.size();
	}

/*** SIP ***/
	VS_SIPBuffer theSIPBuffer;

	//char* content = 0;
	std::size_t content_len = 0;
	const char *content = strstr(aInput, "\r\n\r\n");

	if ( static_cast<std::size_t>(content - aInput) >= aSize )
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	if ( content )
	{
		content += 4;	// Skip new line
		const std::size_t sip_len = (content - aInput);
		content_len = aSize - sip_len;

		theSIPBuffer.AddData(aInput, sip_len);
	}else{
		theSIPBuffer.AddData(aInput, aSize);
	}

	m_sip_meta_field = vs::make_unique<VS_SIPMetaField>();

	TSIPErrorCodes err = m_sip_meta_field->Decode(theSIPBuffer);
	if ( err != TSIPErrorCodes::e_ok )
	{
		m_sip_meta_field.reset();
		SetError(err);
		return err;
	}

/*** Content ***/
	if ( m_sip_meta_field->iContentType && (m_sip_meta_field->iContentType->GetContentType() != CONTENTTYPE_INVALID)
		 && content && content_len )
	{
		const int theContentType = m_sip_meta_field->iContentType->GetContentType();

		VS_SIPBuffer theContentBuffer;

		err = theContentBuffer.AddData(content, content_len);
		if (err != TSIPErrorCodes::e_ok)
		{
			m_sip_meta_field.reset();
			SetError(TSIPErrorCodes::e_Content);
			return TSIPErrorCodes::e_Content;
		}

		theContentBuffer.AddData("\r\n");

		switch ( theContentType )
		{
		case CONTENTTYPE_SDP:
				m_sdp_meta_field = vs::make_unique<VS_SDPMetaField>();

				err = m_sdp_meta_field->Decode(theContentBuffer);
				if (err != TSIPErrorCodes::e_ok)
				{
					m_sip_meta_field.reset();
					m_sdp_meta_field.reset();
					SetError(err);
					return err;
				}

				m_content_type = CONTENTTYPE_SDP;
			break;

		case CONTENTTYPE_MEDIACONTROL_XML:
				m_mc_meta_field = vs::make_unique<VS_MetaField_MediaControl_XML>();

				err = m_mc_meta_field->Decode(theContentBuffer);
				if (err != TSIPErrorCodes::e_ok)
				{
					m_sip_meta_field.reset();
					m_mc_meta_field.reset();
					SetError(err);
					return err;
				}

				m_content_type = CONTENTTYPE_MEDIACONTROL_XML;
			break;

		case CONTENTTYPE_BFCP:
				m_content_type = CONTENTTYPE_BFCP;
				// TODO: Decode from base64 and save content
			break;

		case CONTENTTYPE_UNKNOWN:
				m_content_type = CONTENTTYPE_UNKNOWN;
			break;
		case CONTENTTYPE_PIDF_XML:
				m_content_type = CONTENTTYPE_PIDF_XML;
			break;
		case CONTENTTYPE_TEXT_PLAIN:
			m_sip_message = vs::make_unique<VS_SIPInstantMessage>();

			err = m_sip_message->Decode(theContentBuffer);
			if (err != TSIPErrorCodes::e_ok)
			{
				m_sip_message.reset();
				SetError(err);
				return err;
			}
			m_content_type = CONTENTTYPE_TEXT_PLAIN;
			break;
		case CONTENTTYPE_MULTIPART: {
			m_sdp_meta_field = vs::make_unique<VS_SDPMetaField>();

			const char *first_sdp = strstr(content, "\r\n\r\n");
			if (first_sdp) {
				first_sdp += 4;
				theContentBuffer.Clean();
				theContentBuffer.AddData(first_sdp, content_len - (first_sdp - content));
				theContentBuffer.AddData("\r\n");
			}

			err = m_sdp_meta_field->Decode(theContentBuffer);
			if (err != TSIPErrorCodes::e_ok) {
				m_sip_meta_field.reset();
				m_sdp_meta_field.reset();
				SetError(err);
				return err;
			}

			m_content_type = CONTENTTYPE_SDP;
		} break;
		default:
				m_content_type = CONTENTTYPE_INVALID;

				SetError(TSIPErrorCodes::e_UnknownContent);
				return TSIPErrorCodes::e_UnknownContent;
			break;
		}
	}else{
		if ( m_sip_meta_field->iContentType && (m_sip_meta_field->iContentType->GetContentType() != CONTENTTYPE_INVALID) )
			return TSIPErrorCodes::e_Content;
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPMessage::Encode(char* aOutput, std::size_t &aSize) const
{
	if ( !IsValid() || !m_sip_meta_field )
		return GetLastClassError();

	if ( !m_sip_meta_field->IsValid() )
		return m_sip_meta_field->GetLastClassError();

	VS_SIPBuffer content;
	switch (m_content_type)
	{
	case CONTENTTYPE_SDP:
			if ( m_sdp_meta_field && m_sdp_meta_field->IsValid() )
			{
				TSIPErrorCodes err = m_sdp_meta_field->Encode(content);
				if (err != TSIPErrorCodes::e_ok)
				{
					return err;
				}
			}
		break;
	case CONTENTTYPE_MEDIACONTROL_XML:
			if ( m_mc_meta_field && m_mc_meta_field->IsValid() )
			{
				TSIPErrorCodes err = m_mc_meta_field->Encode(content);
				if (err != TSIPErrorCodes::e_ok)
				{
					return err;
				}
			}
		break;
	case CONTENTTYPE_PIDF_XML:
			if ( m_pidf_meta_field && m_pidf_meta_field->IsValid() )
			{
				TSIPErrorCodes err = m_pidf_meta_field->Encode(content);
				if (err != TSIPErrorCodes::e_ok)
				{
					return err;
				}
			}
		break;
	case CONTENTTYPE_DTMF_RELAY:
			if ( m_dtmf_relay && m_dtmf_relay->IsValid() )
			{
				TSIPErrorCodes err = m_dtmf_relay->Encode(content);
				if (err != TSIPErrorCodes::e_ok)
				{
					return err;
				}
			}
		break;
	case CONTENTTYPE_TEXT_PLAIN:
	case CONTENTTYPE_TEXT_RTF:
		if (m_sip_message && m_sip_message->IsValid())
		{
			TSIPErrorCodes err = m_sip_message->Encode(content);
			if (err != TSIPErrorCodes::e_ok)
			{
				return err;
			}
		}
		break;
	}

// Count length of content
	const auto length_of_content = content.GetWriteIndex();
	if ( length_of_content && m_sip_meta_field->iContentLength && m_sip_meta_field->iContentLength->IsValid() )
		m_sip_meta_field->iContentLength->Value(length_of_content);

	VS_SIPBuffer sip_msg;

	TSIPErrorCodes err = m_sip_meta_field->Encode(sip_msg);
	if (err != TSIPErrorCodes::e_ok)
	{
		return err;
	}

	err = sip_msg.AddData("\r\n");
	if ( err != TSIPErrorCodes::e_ok )
	{
		return err;
	}

	if ( length_of_content )
	{
		err = sip_msg.AddData(content);
		if ( err != TSIPErrorCodes::e_ok )
		{
			return err;
		}
	}

	const auto buf_len = sip_msg.GetWriteIndex();

	if (aSize < buf_len)
	{
		aSize = buf_len;

		return TSIPErrorCodes::e_buffer;
	}

	aSize = buf_len;

	err = sip_msg.GetDataConst(aOutput, aSize);
	return err;
}

TSIPErrorCodes VS_SIPMessage::Encode(std::string &aOutput) const
{
	char* out = 0;
	std::size_t out_sz = 0;

	TSIPErrorCodes ret = Encode(out, out_sz);
	if (ret == TSIPErrorCodes::e_buffer && out_sz > 0)
	{
		out = new char[out_sz + 1];
		ret = Encode(out, out_sz);
		if (ret == TSIPErrorCodes::e_ok)
			aOutput = out;
	}

	delete [] out;

	return ret;
}

eContentType VS_SIPMessage::GetContentType() const
{
	return m_content_type;
}

void VS_SIPMessage::SetContentType(const eContentType type)
{
	m_content_type = type;
}

eMessageType VS_SIPMessage::GetMessageType() const
{
	int type = MESSAGE_TYPE_INVALID;

	if (m_sip_meta_field)
		m_sip_meta_field->GetType(type);

	return static_cast<eMessageType>(type);
}

bool VS_SIPMessage::InsertSIPField(const VS_SIPObjectFactory::SIPHeader header, const VS_SIPGetInfoInterface& info)
{

	VS_SIPObjectFactory* factory = VS_SIPObjectFactory::Instance();
	assert(factory);

	VS_ObjectFactory::CreateFieldResult result = factory->CreateField(header);
	if (TSIPErrorCodes::e_ok != result.error_code)
		return false;

	if ( !result.p_field)
		return false;

	if (TSIPErrorCodes::e_ok != result.p_field->Init(info) )
		return false;

	if ( !m_sip_meta_field ) m_sip_meta_field = vs::make_unique<VS_SIPMetaField>();

	m_sip_meta_field->AddField(std::move(result.p_field));

	m_sip_meta_field->SetError(TSIPErrorCodes::e_ok);
	m_sip_meta_field->SetValid(true);

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return true;
}

bool VS_SIPMessage::InsertSDPField(const VS_SDPObjectFactory::SDPHeader header, const VS_SIPGetInfoInterface& info)
{
	VS_SDPObjectFactory* factory = VS_SDPObjectFactory::Instance();
	assert(factory);

	VS_ObjectFactory::CreateFieldResult result(factory->CreateField(header));
	if (TSIPErrorCodes::e_ok != result.error_code)
		return false;

	if ( !result.p_field)
		return false;

	if (TSIPErrorCodes::e_ok != result.p_field->Init(info) )
		return false;

	if ( !m_sdp_meta_field ) m_sdp_meta_field = vs::make_unique<VS_SDPMetaField>();
	if ( !m_sdp_meta_field ) return false;

	m_sdp_meta_field->AddField(std::move(result.p_field));

	m_sdp_meta_field->SetError(TSIPErrorCodes::e_ok);
	m_sdp_meta_field->SetValid(true);

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return true;
}

// Only for Incom SIP-message
bool VS_SIPMessage::FillInfoByInviteMessage(VS_SipUpdateInfoInterface& updateInfo)
{
	if ( !m_sip_meta_field || !m_sip_meta_field->IsValid() )
		return false;

// timer extention
	if (m_sip_meta_field->iSessionExpires)
	{
		if (m_sip_meta_field->iSessionExpires->GetRefresher() == REFRESHER::REFRESHER_UAS)
			updateInfo.SetRefresher(REFRESHER::REFRESHER_UAS);
		else
			if (m_sip_meta_field->iSessionExpires->GetRefresher() == REFRESHER::REFRESHER_UAC)
				updateInfo.SetRefresher(REFRESHER::REFRESHER_UAC);
			else // default value
				updateInfo.SetRefresher(REFRESHER::REFRESHER_UAC);

		updateInfo.SetRefreshPeriod(m_sip_meta_field->iSessionExpires->GetRefreshInterval());
	}

// cseq
	// INVTE and UPDATE are possible
	if (m_sip_meta_field->iCSeq)
	{
		updateInfo.SetMessageType(m_sip_meta_field->iCSeq->GetType());
		updateInfo.SetSipSequenceNumber(m_sip_meta_field->iCSeq->Value());
	}

	updateInfo.ResetIndexSipVia();
	updateInfo.ClearSipVia();

	for (std::size_t i = 0; i < m_sip_meta_field->iVia.size(); i++)
	{
		updateInfo.SetSipVia(m_sip_meta_field->iVia[i]);
	}

// To
	if ( m_sip_meta_field->iTo && m_sip_meta_field->iTo->IsValid() )
	{
		VS_SIPURI* uri = m_sip_meta_field->iTo->GetURI();
		if ( uri && uri->IsValid() )
		{
			char alias[256] = {0};
			if ( uri->GetAlias_UserHost(alias) )
			{
				updateInfo.SetAliasMy(alias);
			}
			{
				updateInfo.SetTagMy(uri->Tag());
			}
			{
				updateInfo.SetEpidMy(uri->Epid());
			}
			{
				updateInfo.SetDisplayNameMy(uri->Name());
			}
		}
	}

// From
	if ( m_sip_meta_field->iFrom && m_sip_meta_field->iFrom->IsValid() )
	{
		VS_SIPURI* uri = m_sip_meta_field->iFrom->GetURI();
		if ( uri && uri->IsValid() )
		{
			char alias[256] = {0};
			if ( uri->GetAlias_UserHost(alias) )
			{
				updateInfo.SetAliasRemote(alias);
			}
			{
				updateInfo.SetTagSip(uri->Tag());
			}
			{
				updateInfo.SetEpidSip(uri->Epid());
			}
			{
				updateInfo.SetDisplayNameSip(uri->Name());
			}
		}
	}

// Contact and route set
	if (m_sip_meta_field->iContact && m_sip_meta_field->iContact->IsValid())
	{
		updateInfo.FillUriSetForEstablishedDialog(m_sip_meta_field->iContact->GetLastURI(), m_sip_meta_field->iRouteSet, false);
	}

// Call-ID
	if ( m_sip_meta_field->iCallID && m_sip_meta_field->iCallID->IsValid() )
	{
		updateInfo.SetSipDialogId(m_sip_meta_field->iCallID->Value());
	}

// Via
	if (!m_sip_meta_field->iVia.empty()){
		updateInfo.ResetIndexSipVia();
		updateInfo.ClearSipVia();

		for (unsigned int i = 0; i < m_sip_meta_field->iVia.size(); ++i)
		{
			updateInfo.SetSipVia(m_sip_meta_field->iVia[i]);
		}
	}

// User Agent
	FillInfoUserAgent(updateInfo);

	return true;
}

bool VS_SIPMessage::FillInfoFromSDP(const VS_SIPGetInfoInterface& getInfo, VS_SipUpdateInfoInterface& updateInfo,
	const bool is_offer)
{
	if (!m_sdp_meta_field || !m_sdp_meta_field->IsValid())
		return false;

// Bandwidth
	if (m_sdp_meta_field->iBandwidth && m_sdp_meta_field->iBandwidth->IsValid())
	{
		updateInfo.SetRemoteBandwidth(m_sdp_meta_field->iBandwidth->GetBandwidth());
	}

	unsigned int min_bw = 1536;
	const auto local_bw = getInfo.GetLocalBandwidth() / 1024;
	if (local_bw > 0)
		min_bw = std::min(min_bw, local_bw);
	const unsigned int remote_bw = getInfo.GetRemoteBandwidth() / 1024;
	if (remote_bw > 0)
		min_bw = std::min(min_bw, remote_bw);
	for (const auto& ms: m_sdp_meta_field->iMediaStreams)
	{
		if (ms->GetMediaType() != SDPMediaType::video)
			continue;
		unsigned int ms_bw = ms->GetBandwidth() / 1000;
		if (ms_bw > 0)
			min_bw = std::min(min_bw, ms_bw);
	}
	if (min_bw < 256)
	{
		updateInfo.LimitH264Level(13);
	}

// Update media streams
	for (std::size_t index = 0; index < m_sdp_meta_field->iMediaStreams.size(); ++index)
	{
		VS_SDPField_MediaStream* const ms = m_sdp_meta_field->iMediaStreams[index];
		VS_SDPField_MediaStream* const info_ms = updateInfo.MediaStream(index, true);
		if (!ms || !ms->IsValid() || !info_ms)
			return false;

		bool ignore = false;

		const SDPMediaType media_type = ms->GetMediaType();

		eSDP_MediaChannelDirection direction = ms->GetMediaDirection();
		if (direction == SDP_MEDIACHANNELDIRECTION_INVALID)
			direction = SDP_MEDIACHANNELDIRECTION_SENDRECV;
		if (direction == SDP_MEDIACHANNELDIRECTION_SEND)
			direction = SDP_MEDIACHANNELDIRECTION_RECV;
		else if (direction == SDP_MEDIACHANNELDIRECTION_RECV)
			direction = SDP_MEDIACHANNELDIRECTION_SEND;

		const net::port port = ms->GetPort();
		if (port == 0)
			ignore = true;

		net::port local_port = info_ms->GetLocalPort(); //NB: Taking local_port from info_ms, not from ms (where it always set to 9)
		if (local_port == 0)
			local_port = DISCARD_PROTOCOL_PORT;

		const eSDP_RTPPROTO proto = ms->GetProto();

		eSDP_ContentType content = ms->GetContent();
		if (content == eSDP_ContentType::SDP_CONTENT_INVALID)
			content = SDP_CONTENT_MAIN;

		std::string label = ms->GetLabel();

		eSDP_Setup setup = ms->GetSetup();
		if (setup == SDP_SETUP_INVALID)
			setup = is_offer ? SDP_SETUP_ACTIVE : SDP_SETUP_PASSIVE;
		if (setup == SDP_SETUP_ACTIVE)
			setup = SDP_SETUP_PASSIVE;
		else if (setup == SDP_SETUP_PASSIVE)
			setup = SDP_SETUP_ACTIVE;

		eSDP_Connection connection_attr = ms->GetConnectionAttr();
		if (connection_attr == SDP_CONNECTION_INVALID)
			connection_attr = SDP_CONNECTION_NEW;

		unsigned int bfcp_floor_ctrl = ms->GetBFCPFloorCtrl();
		if (bfcp_floor_ctrl == SDP_FLOORCTRL_ROLE_INVALID)
			bfcp_floor_ctrl = is_offer ? SDP_FLOORCTRL_ROLE_C_ONLY : SDP_FLOORCTRL_ROLE_S_ONLY;
		if (!!(bfcp_floor_ctrl & SDP_FLOORCTRL_ROLE_C_ONLY) != !!(bfcp_floor_ctrl & SDP_FLOORCTRL_ROLE_S_ONLY)) // If only one of 'c-only', 's-only' is set...
			bfcp_floor_ctrl ^= SDP_FLOORCTRL_ROLE_C_ONLY | SDP_FLOORCTRL_ROLE_S_ONLY; // ...flip this roles on our end

		unsigned int bfcp_conf_id = ms->GetBFCPConfID();
		unsigned int bfcp_user_id = ms->GetBFCPUserID();
		unsigned int bfcp_floor_id = ms->GetBFCPFloorID();
		std::vector<std::string> bfcp_floor_labels = ms->BFCPFloorLabels();

		// Validation & fixes
		if (media_type != SDPMediaType::audio && media_type != SDPMediaType::video && media_type != SDPMediaType::application_bfcp && media_type != SDPMediaType::application_fecc)
			ignore = true;
		if (media_type == SDPMediaType::audio && content != SDP_CONTENT_MAIN)
			ignore = true;
		if (media_type == SDPMediaType::video && content != SDP_CONTENT_MAIN && content != SDP_CONTENT_SLIDES)
			ignore = true;

		if (media_type == SDPMediaType::audio || media_type == SDPMediaType::video)
		{
			if (proto != SDP_RTPPROTO_RTP_AVP && proto != SDP_RTPPROTO_RTP_SAVP)
				ignore = true;
		}

		if (media_type == SDPMediaType::video && content == SDP_CONTENT_SLIDES)
		{
			if (!getInfo.IsBfcpEnabled())
				ignore = true;

			if (label.empty())
				label = "3";
		}

		if (media_type == SDPMediaType::application_bfcp)
		{
			if (!getInfo.IsBfcpEnabled())
				ignore = true;

			if (proto != SDP_PROTO_TCP_BFCP && proto != SDP_PROTO_UDP_BFCP)
				ignore = true;

			local_port = getInfo.GetBfcpLocalPort();

			auto supported_roles = getInfo.GetBfcpSupportedRoles();
			if (supported_roles & SDP_FLOORCTRL_ROLE_C_S)
				supported_roles = SDP_FLOORCTRL_ROLE_C_ONLY | SDP_FLOORCTRL_ROLE_S_ONLY;
			if (bfcp_floor_ctrl & SDP_FLOORCTRL_ROLE_C_S)
				bfcp_floor_ctrl = SDP_FLOORCTRL_ROLE_C_ONLY | SDP_FLOORCTRL_ROLE_S_ONLY;
			bfcp_floor_ctrl &= supported_roles;
			if (bfcp_floor_ctrl & SDP_FLOORCTRL_ROLE_C_ONLY && bfcp_floor_ctrl & SDP_FLOORCTRL_ROLE_S_ONLY)
				bfcp_floor_ctrl = SDP_FLOORCTRL_ROLE_C_ONLY;
			if (bfcp_floor_ctrl == SDP_FLOORCTRL_ROLE_INVALID)
				ignore = true;

			if (setup == SDP_SETUP_ACTPASS)
				setup = (bfcp_floor_ctrl & SDP_FLOORCTRL_ROLE_C_ONLY) ? SDP_SETUP_ACTIVE : SDP_SETUP_PASSIVE;

			if (connection_attr == SDP_CONNECTION_EXISTING)
				connection_attr = SDP_CONNECTION_NEW;

			if (bfcp_floor_ctrl & SDP_FLOORCTRL_ROLE_S_ONLY)
			{
				bfcp_conf_id = 1;
				bfcp_user_id = 2;
				bfcp_floor_id = 1;
				bfcp_floor_labels.emplace_back("3");
			}
		}

		if (media_type == SDPMediaType::application_fecc)
		{
			if (!getInfo.IsH224Enabled())
				ignore = true;

			if (proto != SDP_RTPPROTO_RTP_AVP)
				ignore = true;
		}

		info_ms->SetMediaType(media_type);
		info_ms->SetMediaDirection(direction);
		info_ms->SetLocalPort(ignore ? 0 : local_port);
		info_ms->SetPort(port);
		info_ms->SetPortRange(ms->GetPortRange());
		info_ms->SetProto(proto);
		info_ms->SetConnection(ms->GetConnection());
		info_ms->SetBandwidth(ms->GetBandwidth());
		info_ms->SetContent(content);
		info_ms->SetLabel(std::move(label));
		info_ms->ClearLocalCodecs();
		info_ms->CopyLocalCodecsFrom(getInfo);
		info_ms->ClearRemoteCodecs();
		info_ms->CopyRemoteCodecsFrom(ms);
		info_ms->SetFIRSupport(ms->GetFIRSupport());

		if (getInfo.UseSingleBestCodec())
		{
			if (auto best_codec = info_ms->GetBestCodec(false, true))
			{
				info_ms->ClearLocalCodecs();
				info_ms->AddLocalCodec(std::move(best_codec));
			}
		}

		info_ms->SetRemoteIceUfrag(ms->GetRemoteIceUfrag());
		info_ms->SetRemoteIcePwd(ms->GetRemoteIcePwd());
		info_ms->SetRemoteCryptoKey(ms->GetRemoteCryptoKey());

		// Not copying control and raw_fmtp because they are not used in SIP

		if (getInfo.IceEnabled()) {
			InsertMediaStreamICE(getInfo, info_ms);
		}

		if (getInfo.SrtpEnabled()) {
			InsertMediaStreamSRTP(getInfo, info_ms);
		}

		info_ms->SetSetup(setup);
		info_ms->SetConnectionAttr(connection_attr);
		info_ms->SetBFCPFloorCtrl(bfcp_floor_ctrl);
		info_ms->SetBFCPConfID(bfcp_conf_id);
		info_ms->SetBFCPUserID(bfcp_user_id);
		info_ms->SetBFCPFloorID(bfcp_floor_id);
		info_ms->BFCPFloorLabels() = std::move(bfcp_floor_labels);
	}
	for (unsigned int index = m_sdp_meta_field->iMediaStreams.size(); index < getInfo.MediaStreams().size(); ++index)
	{
		// Incoming sdp has less media streams than before.
		// While this is serious violation of offer/answer negotiation protocol by remote part (rfc3264 section 6), for now we just disable missing streams.
		VS_SDPField_MediaStream* const info_ms = updateInfo.MediaStream(index);
		if (!info_ms)
			continue;

		info_ms->SetLocalPort(0);
	}

	return true;
}

bool VS_SIPMessage::FillInfoUserAgent(VS_SipUpdateInfoInterface& updateInfo)
{
	if (GetSIPMetaField()->iUserAgent && GetSIPMetaField()->iUserAgent->IsValid() && !GetSIPMetaField()->iUserAgent->GetUserAgent().empty())
	{
		updateInfo.SetUserAgent(GetSIPMetaField()->iUserAgent->GetUserAgent());
		return true;
	}
	return false;
}

bool VS_SIPMessage::InsertMediaStreams(const VS_SIPGetInfoInterface& getInfo)
{

	VS_SDPObjectFactory* factory = VS_SDPObjectFactory::Instance();
	assert(factory);

	if (!m_sdp_meta_field)
		m_sdp_meta_field = vs::make_unique<VS_SDPMetaField>();
	if (!m_sdp_meta_field)
		return false;

	for (auto& info_ms: getInfo.MediaStreams())
	{
		auto result(factory->CreateField(VS_SDPObjectFactory::SDPHeader::SDPHeader_MediaStream));
		if (TSIPErrorCodes::e_ok != result.error_code)
			return false;

		auto ms = dynamic_cast<VS_SDPField_MediaStream*>(result.p_field.get());
		if (!ms)
			return false;

		ms->SetMediaType(info_ms->GetMediaType());
		ms->SetMediaDirection(info_ms->GetMediaDirection());
		ms->SetLocalPort(info_ms->GetLocalPort());
		ms->SetPort(info_ms->GetPort());
		// Not copying connection because we use session-wide connection
		ms->SetProto(info_ms->GetProto());
		if (ms->GetMediaType() == SDPMediaType::video)
		{
			ms->SetBandwidth(getInfo.GetLocalBandwidth());
		}
		ms->SetContent(info_ms->GetContent());
		ms->SetLabel(info_ms->GetLabel());
		ms->ClearLocalCodecs();
		ms->CopyLocalCodecsFrom(getInfo);
		ms->ClearRemoteCodecs();
		ms->CopyRemoteCodecsFrom(info_ms);

		if (getInfo.UseSingleBestCodec())
		{
			if (auto best_codec = ms->GetBestCodec(false, true))
			{
				ms->ClearLocalCodecs();
				ms->AddLocalCodec(std::move(best_codec));
			}
		}

		// Not copying control and raw_fmtp because they are not used in SIP

		ms->SetSetup(info_ms->GetSetup());
		ms->SetConnectionAttr(info_ms->GetConnectionAttr());
		ms->SetBFCPFloorCtrl(info_ms->GetBFCPFloorCtrl());
		ms->SetBFCPConfID(info_ms->GetBFCPConfID());
		ms->SetBFCPUserID(info_ms->GetBFCPUserID());
		ms->SetBFCPFloorID(info_ms->GetBFCPFloorID());
		ms->BFCPFloorLabels() = info_ms->BFCPFloorLabels();

		if (getInfo.IceEnabled()) {
			InsertMediaStreamICE(getInfo, ms);
		}

		if (getInfo.SrtpEnabled()) {
			InsertMediaStreamSRTP(getInfo, ms);
		}

		ms->SetMessageURL(info_ms->GetMessageURL());
		ms->AcceptTypes() = info_ms->AcceptTypes();

		result.p_field->SetError(TSIPErrorCodes::e_ok);
		result.p_field->SetValid(true);
		m_sdp_meta_field->AddField(std::move(result.p_field));
		m_sdp_meta_field->SetError(TSIPErrorCodes::e_ok);
		m_sdp_meta_field->SetValid(true);
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return true;
}

const VS_SIPMetaField* VS_SIPMessage::GetSIPMetaField() const
{
	return m_sip_meta_field.get();
}

const VS_SDPMetaField* VS_SIPMessage::GetSDPMetaField() const
{
	return m_sdp_meta_field.get();
}

bool VS_SIPMessage::CheckAuth(const VS_SIPAuthInfo* info)
{
	// ¬з€ть из VS_SIPMetaField VS_SIPAuthInfo и сверить их
//	m_sip_meta_field->

	return false;
}

bool VS_SIPMessage::IsFastUpdatePicture() const
{
	if (m_mc_meta_field)
		return m_mc_meta_field->IsFastUpdatePicture();

	return false;
}

bool VS_SIPMessage::InsertField_MediaControl_FastUpdatePicture(const VS_SIPGetInfoInterface& info)
{
	m_mc_meta_field = vs::make_unique<VS_MetaField_MediaControl_XML>();

	const TSIPErrorCodes err = m_mc_meta_field->Init(info);
	if (TSIPErrorCodes::e_ok != err )
	{
		SetError(err);
		SetValid(false);
		return false;
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return true;
}

bool VS_SIPMessage::InsertField_DTMF_Relay(const VS_SIPGetInfoInterface& info, const char dtmf_digit)
{
	m_dtmf_relay = vs::make_unique<VS_MetaField_DTMF_Relay>(dtmf_digit);

	const TSIPErrorCodes err = m_dtmf_relay->Init(info);
	if (TSIPErrorCodes::e_ok != err )
	{
		SetError(err);
		SetValid(false);
		return false;
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return true;
}

bool VS_SIPMessage::InsertField_PIDF(const VS_SIPGetInfoInterface& info)
{
	m_pidf_meta_field = vs::make_unique<VS_MetaField_PIDF_XML>();

	TSIPErrorCodes err = m_pidf_meta_field->Init(info);
	if (TSIPErrorCodes::e_ok != err )
	{
		SetError(err);
		SetValid(false);
		return false;
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return true;
}

void VS_SIPMessage::EraseSIPField(const VS_SIPObjectFactory::SIPHeader header) const
{
	if (m_sip_meta_field) {
		m_sip_meta_field->EraseFields(header);
	}
}

void VS_SIPMessage::EraseSDPField(const VS_SDPObjectFactory::SDPHeader header) const
{
	if (m_sdp_meta_field) {
		m_sdp_meta_field->EraseFields(header);
	}
}

bool VS_SIPMessage::HasSIPField(const VS_SIPObjectFactory::SIPHeader header) const
{
	if (m_sip_meta_field) {
		return m_sip_meta_field->HasField(header);
	}
	return false;
}

bool VS_SIPMessage::HasSDPField(const VS_SDPObjectFactory::SDPHeader header) const
{
	if (m_sdp_meta_field) {
		return m_sdp_meta_field->HasField(header);
	}
	return false;
}

bool VS_SIPMessage::UpdateOrIgnoreSIPField(const VS_SIPObjectFactory::SIPHeader header, const VS_SIPGetInfoInterface& info)
{
	if (m_sip_meta_field && HasSIPField(header)) {
		EraseSIPField(header);
		return InsertSIPField(header, info);
	}
	return true;
}

bool VS_SIPMessage::UpdateOrInsertSIPField(const VS_SIPObjectFactory::SIPHeader header, const VS_SIPGetInfoInterface& info)
{
	if (m_sip_meta_field) {
		EraseSIPField(header);
		return InsertSIPField(header, info);
	}
	return false;
}

bool VS_SIPMessage::UpdateOrIgnoreSDPField(const VS_SDPObjectFactory::SDPHeader header, const VS_SIPGetInfoInterface& info)
{
	if (m_sdp_meta_field && HasSDPField(header)) {
		EraseSDPField(header);
		return InsertSDPField(header, info);
	}
	return true;
}

bool VS_SIPMessage::UpdateOrInsertSDPField(const VS_SDPObjectFactory::SDPHeader header, const VS_SIPGetInfoInterface& info)
{
	if (m_sdp_meta_field) {
		EraseSDPField(header);
		return InsertSDPField(header, info);
	}
	return false;
}

std::shared_ptr<VS_SIPAuthInfo> VS_SIPMessage::GetAuthInfo() const
{
	if ( !m_sip_meta_field || m_sip_meta_field->iAuthHeader.empty() || !m_sip_meta_field->iAuthHeader[0]->IsValid() )
		return nullptr;

	auto info = m_sip_meta_field->iAuthHeader[0]->GetAuthInfo();
	if ( !info )
		return nullptr;

	return info;
}

string_view VS_SIPMessage::CallID() const
{
	return (m_sip_meta_field &&
			m_sip_meta_field->iCallID &&
			m_sip_meta_field->iCallID->IsValid()) ? m_sip_meta_field->iCallID->Value() : string_view{};
}

string_view VS_SIPMessage::Branch() const
{
	return (m_sip_meta_field &&
			!m_sip_meta_field->iVia.empty() &&
			m_sip_meta_field->iVia[0]->IsValid()) ? m_sip_meta_field->iVia[0]->Branch() : string_view{};
}


eStartLineType VS_SIPMessage::GetMethod() const
{
	return (m_sip_meta_field &&
			m_sip_meta_field->iCSeq &&
			m_sip_meta_field->iCSeq->IsValid()) ? m_sip_meta_field->iCSeq->GetType() : TYPE_INVALID;
}

std::int32_t VS_SIPMessage::GetCSeq() const
{
	return (m_sip_meta_field &&
			m_sip_meta_field->iCSeq &&
			m_sip_meta_field->iCSeq->IsValid()) ? m_sip_meta_field->iCSeq->Value() : -1;
}

net::protocol VS_SIPMessage::GetConnectionType() const
{
	return (m_sip_meta_field &&
			!m_sip_meta_field->iVia.empty() &&
			m_sip_meta_field->iVia[0]->IsValid()) ? m_sip_meta_field->iVia[0]->ConnectionType() : net::protocol::none;
}

string_view VS_SIPMessage::UserAgent() const
{
	return (m_sip_meta_field &&
			m_sip_meta_field->iUserAgent &&
			m_sip_meta_field->iUserAgent->IsValid()) ? m_sip_meta_field->iUserAgent->GetUserAgent() : string_view{};
}

int VS_SIPMessage::GetResponseCode() const
{
	return (m_sip_meta_field &&
			m_sip_meta_field->iStartLine &&
			m_sip_meta_field->iStartLine->IsValid()) ? m_sip_meta_field->iStartLine->GetResponseCode() : 0;
}

int VS_SIPMessage::GetResponseCodeClass() const
{
	return (m_sip_meta_field &&
			m_sip_meta_field->iStartLine &&
			m_sip_meta_field->iStartLine->IsValid()) ? m_sip_meta_field->iStartLine->GetResponseCodeClass() : 0;
}

string_view VS_SIPMessage::Server() const
{
	return (m_sip_meta_field &&
			m_sip_meta_field->iServer &&
			m_sip_meta_field->iServer->IsValid()) ? m_sip_meta_field->iServer->GetServer() : string_view{};
}

bool VS_SIPMessage::InsertInstantMessage(const VS_SIPGetInfoInterface& info, string_view message) {
	m_sip_message = vs::make_unique<VS_SIPInstantMessage>();

	const TSIPErrorCodes err = m_sip_message->Init(info, message);
	if (TSIPErrorCodes::e_ok != err)
	{
		SetError(err);
		SetValid(false);
		return false;
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return true;
}

const VS_SIPInstantMessage* VS_SIPMessage::GetSIPInstantMessage() const
{
	return m_sip_message.get();
}

std::vector<std::string> VS_SIPMessage::GetAcceptedTextFormats() const
{
	if (!m_sdp_meta_field || m_sdp_meta_field->iMediaStreams.empty()) return {};

	const auto &streams = m_sdp_meta_field->iMediaStreams;
	const auto it = std::find_if(streams.begin(), streams.end(),[](VS_SDPField_MediaStream* s) { return s && s->GetMediaType() == SDPMediaType::message; });
	if (it == streams.end()) return {};

	return (*it)->AcceptTypes();
}

std::string VS_SIPMessage::GetFrom() const{
	if (!m_sip_meta_field ||
		!m_sip_meta_field->iFrom ||
		!m_sip_meta_field->iFrom->IsValid())
		return "";

	char user_host[256];
	if (m_sip_meta_field->iFrom->GetURI()->GetAlias_UserHost(user_host)){
		std::string res(user_host);
		return res;
	}
	return "";
}
std::string VS_SIPMessage::GetTo() const{
	if (!m_sip_meta_field ||
		!m_sip_meta_field->iTo ||
		!m_sip_meta_field->iTo->IsValid())
		return "";

	char user_host[256];
	if (m_sip_meta_field->iTo->GetURI()->GetAlias_UserHost(user_host)){
		std::string res(user_host);
		return res;
	}
	return "";
}

string_view VS_SIPMessage::DisplayNameMy() const{
	return (m_sip_meta_field &&
		m_sip_meta_field->iFrom &&
		m_sip_meta_field->iFrom->IsValid()) ? m_sip_meta_field->iFrom->GetURI()->Name() : string_view{};
}

void VS_SIPMessage::InsertMediaStreamICE(const VS_SIPGetInfoInterface& getInfo, VS_SDPField_MediaStream* ms)
{
	if (ms->GetMediaType() == SDPMediaType::audio) {
		ms->SetOurSsrcRange(getInfo.GetSsrcRangeAudio());
		ms->SetOurIceUfrag(std::string(getInfo.GetIceUfrag().data(), 4));
		ms->SetOurIcePwd(std::string(getInfo.GetIcePwd().data(), 22));
		ms->SetOurIcePwd(std::string(getInfo.GetIcePwd().data(), 22));
	}
	else if (ms->GetMediaType() == SDPMediaType::video)
	{
		ms->SetOurSsrcRange(getInfo.GetSsrcRangeVideo());
		ms->SetOurIceUfrag(std::string(getInfo.GetIceUfrag().data() + 4, 4));
		ms->SetOurIcePwd(std::string(getInfo.GetIcePwd().data() + 24, 22));
	}
}

void VS_SIPMessage::InsertMediaStreamSRTP(const VS_SIPGetInfoInterface& getInfo, VS_SDPField_MediaStream* ms)
{
	if (!getInfo.HaveAuthenticatedTLSConnection()) return;

	string_view key = getInfo.GetSrtpKey();
	const size_t len = key.length();
	assert(len > 1);

	if (ms->GetMediaType() == SDPMediaType::audio) {
		ms->SetOurCryptoKey(std::string(key.substr(0, len / 2)));
	} else if (ms->GetMediaType() == SDPMediaType::video) {
		ms->SetOurCryptoKey(std::string(key.substr(len / 2, len / 2)));
	}
}

std::chrono::steady_clock::duration VS_SIPMessage::GetRetryAfterInterval() const{
	return (m_sip_meta_field &&
		m_sip_meta_field->iRetryAfterField &&
		m_sip_meta_field->iRetryAfterField->IsValid()) ? m_sip_meta_field->iRetryAfterField->Value() : std::chrono::steady_clock::duration();
}

void VS_SIPMessage::CopyAllContacts(std::vector<std::string>& OUT_contacts) const{
	if (m_sip_meta_field && m_sip_meta_field->iContact && m_sip_meta_field->iContact->IsValid()){
		m_sip_meta_field->iContact->GetURIs(OUT_contacts);
	}
}