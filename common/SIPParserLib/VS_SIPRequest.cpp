#include "VS_SIPRequest.h"
#include "VS_SIPMetaField.h"
#include "VS_SDPField_Attribute.h"
#include "VS_SDPMetaField.h"
#include "VS_SIPAuthScheme.h"
#include "VS_SIPAuthGSS.h"
#include "VS_SIPField_Via.h"
#include "VS_SIPField_CSeq.h"
#include "VS_SIPInstantMessage.h"
#include "std/cpplib/VS_Utils.h"
#include "VS_TimerExtention.h"


bool VS_SIPRequest::MakeRefreshINVITE(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo)
{
	const bool is_req = getInfo.IsRequest();
	const bool res = MakeINVITE(getInfo, updateInfo);
	updateInfo.IsRequest(is_req);
	return res;
}


bool VS_SIPRequest::MakeINVITE(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo)
{

	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	updateInfo.IsRequest(true);
	updateInfo.IncreaseMySequenceNumber();
	updateInfo.AlterMyBranch();
	updateInfo.SetMessageType(TYPE_INVITE);
	SetContentType(CONTENTTYPE_SDP);			// For VS_SIPField_ContentType
	updateInfo.SetContentType(CONTENTTYPE_SDP);		// For VS_SIPMessage::Encode()

/////////////////////////
// Calc Response
/////////////////////////
	bool IsAuth = false;

	auto auth_scheme = getInfo.GetAuthScheme();
	if ( auth_scheme )
	{
		VS_SIPObjectFactory* factory = VS_SIPObjectFactory::Instance();
		assert(factory);

		auth_scheme->method(VS_SIPObjectFactory::GetMethod(TYPE_INVITE));
		if (std::dynamic_pointer_cast<VS_SIPAuthGSS>(auth_scheme)) {
			IsAuth = true;
		} else {
			IsAuth = factory->CalcDigestResponse(auth_scheme.get());
		}
	}

/////////////////////////

/////////////////////////
// Insert Field
/////////////////////////

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) /*||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Via, info)*/ ) // will be filled in sip::TransportLayer
		 return false;

	if (IsAuth)
	{
		VS_SIPAuthInfo::AuthType auth_type = auth_scheme->auth_type();
		switch (auth_type)
		{
		case VS_SIPAuthInfo::TYPE_AUTH_USER_TO_USER:
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Authorization, getInfo))
				return false;
			break;
		case VS_SIPAuthInfo::TYPE_AUTH_PROXY_TO_USER:
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ProxyAuthorization, getInfo))
				return false;
			break;
		default:
			return false;
		}

		updateInfo.AuthInInvite(true);
	}
	else
		updateInfo.AuthInInvite(false);

	const bool timerExt = (getInfo.IsSessionTimerEnabled() || getInfo.IsSessionTimerUsed()) && getInfo.GetTimerExtention().refresher != REFRESHER::REFRESHER_INVALID;

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		(timerExt && !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Supported, getInfo)) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Allow, getInfo) ||
		(timerExt && !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_SessionExpires, getInfo)) ||
		(timerExt && !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MinSE, getInfo)) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) || // will be filled in sip::TransportLayer
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Supported, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentType, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo))
		return false;

	if (!InsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_Version, getInfo) ||
		!InsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_SessionName, getInfo) ||
		!InsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_Bandwidth, getInfo) ||
		!InsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_Time, getInfo))
		return false;

	if (getInfo.SrtpEnabled()) {
		VS_BaseField *f1 = new VS_SDPField_Attribute(VS_SDPField_Attribute::XMediaBW),
					 *f2 = new VS_SDPField_Attribute(VS_SDPField_Attribute::XDeviceCaps);
		if (f1->Init(getInfo) != TSIPErrorCodes::e_ok || f2->Init(getInfo) != TSIPErrorCodes::e_ok) return false;
		m_sdp_meta_field->AddField(std::unique_ptr<VS_BaseField>(f1));
		m_sdp_meta_field->AddField(std::unique_ptr<VS_BaseField>(f2));
	}

	if (!InsertMediaStreams(getInfo))
		return false;

	return true;
}

bool VS_SIPRequest::MakeACK(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo,
	const bool newTransaction)
{
	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	updateInfo.IsRequest(true);
	//	info->IncreaseMySequenceNumber();
	if (newTransaction)
		updateInfo.AlterMyBranch();
	updateInfo.SetMessageType(TYPE_ACK);

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) /*||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Via, info)*/ )
		 return false;

	size_t routeSetSize = getInfo.GetSipRouteSetSize();
	if (routeSetSize)
	{
		updateInfo.ResetSipRouteIndex();
		for (size_t i = 0; i < routeSetSize; ++i)
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Route, getInfo))
			return false;
	}

	if (getInfo.IsAuthInInvite() && getInfo.GetAuthScheme())
	{
		VS_SIPAuthInfo::AuthType auth_type = getInfo.GetAuthScheme()->auth_type();
		switch (auth_type)
		{
		case VS_SIPAuthInfo::TYPE_AUTH_USER_TO_USER:
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Authorization, getInfo))
				return false;
			break;
		case VS_SIPAuthInfo::TYPE_AUTH_PROXY_TO_USER:
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ProxyAuthorization, getInfo))
				return false;
			break;
		default:
			return false;
		}
	}

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo))
		return false;

	return true;
}

bool VS_SIPRequest::MakeBYE(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo)
{
	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	updateInfo.IsRequest(true);
	updateInfo.IncreaseMySequenceNumber();
	updateInfo.AlterMyBranch();
	updateInfo.SetMessageType(TYPE_BYE);

	auto auth_scheme = std::dynamic_pointer_cast<VS_SIPAuthGSS>(getInfo.GetAuthScheme());
	if (auth_scheme) {
		VS_SIPAuthInfo::AuthType auth_type = auth_scheme->auth_type();
		switch (auth_type) {
		case VS_SIPAuthInfo::TYPE_AUTH_USER_TO_USER:
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Authorization, getInfo))
				return false;
			break;
		case VS_SIPAuthInfo::TYPE_AUTH_PROXY_TO_USER:
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ProxyAuthorization, getInfo))
				return false;
			break;
		default:
			return false;
		}
	}

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) /*||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Via, info)*/ )
		 return false;

	size_t routeSetSize = getInfo.GetSipRouteSetSize();
	if (routeSetSize)
	{
		updateInfo.ResetSipRouteIndex();
		for (size_t i = 0; i < routeSetSize; ++i)
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Route, getInfo))
			return false;
	}

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo))
		return false;

	return true;
}

bool VS_SIPRequest::MakeCANCEL(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo)
{
	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	updateInfo.IsRequest(true);
	//info->IncreaseMySequenceNumber();
	updateInfo.SetMessageType(TYPE_CANCEL);

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) /*||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Via, info)*/)
		 return false;

	size_t routeSetSize = getInfo.GetSipRouteSetSize();
	if (routeSetSize)
	{
		updateInfo.ResetSipRouteIndex();
		for (size_t i = 0; i < routeSetSize; ++i)
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Route, getInfo))
			return false;
	}

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		//!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		//!InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo))
		return false;

	return true;
}

bool VS_SIPRequest::MakeINFO_FastUpdatePicture(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo)
{

	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	updateInfo.IsRequest(true);
	updateInfo.IncreaseMySequenceNumber();
	updateInfo.AlterMyBranch();
	updateInfo.SetMessageType(TYPE_INFO);
	SetContentType(CONTENTTYPE_MEDIACONTROL_XML);			// For VS_SIPField_ContentType
	updateInfo.SetContentType(CONTENTTYPE_MEDIACONTROL_XML);		// For VS_SIPMessage::Encode()

	auto auth_scheme = std::dynamic_pointer_cast<VS_SIPAuthGSS>(getInfo.GetAuthScheme());
	if (auth_scheme) {
		VS_SIPAuthInfo::AuthType auth_type = auth_scheme->auth_type();
		switch (auth_type) {
		case VS_SIPAuthInfo::TYPE_AUTH_USER_TO_USER:
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Authorization, getInfo))
				return false;
			break;
		case VS_SIPAuthInfo::TYPE_AUTH_PROXY_TO_USER:
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ProxyAuthorization, getInfo))
				return false;
			break;
		default:
			return false;
		}
	}

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) /*||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Via, info)*/ )
		return false;

	size_t routeSetSize = getInfo.GetSipRouteSetSize();
	if (routeSetSize)
	{
		updateInfo.ResetSipRouteIndex();
		for (size_t i = 0; i < routeSetSize; ++i)
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Route, getInfo))
			return false;
	}

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		//!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		//!InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentType, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo))
		return false;

	if (!InsertField_MediaControl_FastUpdatePicture(getInfo))
		return false;

	return true;
}

bool VS_SIPRequest::MakeINFO_DTMF(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo, const char digit)
{

	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	updateInfo.IsRequest(true);
	updateInfo.IncreaseMySequenceNumber();
	updateInfo.AlterMyBranch();
	updateInfo.SetMessageType(TYPE_INFO);
	SetContentType(CONTENTTYPE_DTMF_RELAY);				// For VS_SIPField_ContentType
	updateInfo.SetContentType(CONTENTTYPE_DTMF_RELAY);		// For VS_SIPMessage::Encode()

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) /*||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Via, info)*/ )
		 return false;

	size_t routeSetSize = getInfo.GetSipRouteSetSize();
	if (routeSetSize)
	{
		updateInfo.ResetSipRouteIndex();
		for (size_t i = 0; i < routeSetSize; ++i)
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Route, getInfo))
			return false;
	}

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentType, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo))
		return false;

	if (!InsertField_DTMF_Relay(getInfo, digit))
		return false;

	return true;
}

bool VS_SIPRequest::MakeREGISTER(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo,
                                 const VS_SIPObjectFactory::SIPHeader authHeader)
{
	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	updateInfo.IsRequest(true);
	updateInfo.IncreaseMySequenceNumber();
	updateInfo.AlterMyBranch();
	updateInfo.SetMessageType(TYPE_REGISTER);

	bool IsAuth = false;

	auto auth_scheme = getInfo.GetAuthScheme();
	if ( auth_scheme )
	{
		VS_SIPObjectFactory* factory = VS_SIPObjectFactory::Instance();
		assert(factory);

		auth_scheme->method(VS_SIPObjectFactory::GetMethod(TYPE_REGISTER));
		if (std::dynamic_pointer_cast<VS_SIPAuthGSS>(auth_scheme)) {
			IsAuth = true;
		} else {
			IsAuth = factory->CalcDigestResponse(auth_scheme.get());
		}
	}

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo)) return false;

	if (IsAuth && authHeader != VS_SIPObjectFactory::SIPHeader::SIPHeader_Invalid)
	{
		if (/*!InsertSIPField(VS_SIPObjectFactory::SIPHeader_Via, info) || */
			!InsertSIPField(authHeader, getInfo))
			return false;
	} else {
		if (!GenerateMyInfo(getInfo, updateInfo)) return false;
		//if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader_Via, info)) return false;
	}

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo)||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactorySIPHeader::::SIPHeader_To, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Expires, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactorySIPHeader::::SIPHeader_Contact, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Supported, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo))
		return false;

	return true;
}

bool VS_SIPRequest::GenerateMyInfo(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo)
{

	char key[32 + 1] = {0};

	//VS_GenKeyByMD5(key);
	//if ( !info->SetSIPDialogID(key) )
	//	return false;

	const auto auth = getInfo.GetAuthScheme();
	const bool gen_tag = (!auth || auth->scheme() != SIP_AUTHSCHEME_NTLM || getInfo.GetTagMy().empty());

	if (gen_tag) {
		VS_GenKeyByMD5(key);
		if (!updateInfo.SetTagMy(key))
			return false;
	}

	VS_GenKeyByMD5(key);
	updateInfo.SetMyBranch(key);
	return true;
}

bool VS_SIPRequest::MakeNOTIFY(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo)
{
	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	updateInfo.IsRequest(true);
	updateInfo.IncreaseMySequenceNumber();
	updateInfo.AlterMyBranch();
	updateInfo.SetMessageType(TYPE_NOTIFY);
	SetContentType(CONTENTTYPE_PIDF_XML);					// For VS_SIPField_ContentType
	updateInfo.SetContentType(CONTENTTYPE_PIDF_XML);				// For VS_SIPMessage::Encode()

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) /*||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Via, info)*/ )
		 return false;

	size_t routeSetSize = getInfo.GetSipRouteSetSize();
	if (routeSetSize)
	{
		updateInfo.ResetSipRouteIndex();
		for (size_t i = 0; i < routeSetSize; ++i)
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Route, getInfo))
			return false;
	}

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_To, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Contact, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_SubscriptionState, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentType, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo))
		return false;

	if ( !InsertField_PIDF(getInfo))
		return false;

	return true;
}

bool VS_SIPRequest::MakeOPTIONS(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo,
	const bool outside_of_dialog) {
    if (m_sip_meta_field)
        m_sip_meta_field->Clear();

	updateInfo.IsRequest(true);
	updateInfo.IncreaseMySequenceNumber();
	updateInfo.AlterMyBranch();
	updateInfo.SetMessageType(TYPE_OPTIONS);
    SetContentType(CONTENTTYPE_PIDF_XML);					// For VS_SIPField_ContentType
	updateInfo.SetContentType(CONTENTTYPE_PIDF_XML);				// For VS_SIPMessage::Encode()

	std::string tag_before(getInfo.GetTagSip());
    if (outside_of_dialog)
	{
		updateInfo.SetTagSip({});
    }

	bool result = InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) &&
                    //InsertSIPField(VS_SIPObjectFactory::SIPHeader_Via, info) &&
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) &&
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) &&
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) &&
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_To, getInfo) &&
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) &&
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) &&
                    //InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) &&
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Accept, getInfo) &&
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo);

    if (outside_of_dialog) {
		updateInfo.SetTagSip(std::move(tag_before));
    }

    return result;
}

void VS_SIPRequest::FillInfoByRequest(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo) const
{
// Via
	const auto meta(this->GetSIPMetaField());
	if ( !meta || (meta->iVia.empty()) )
		return ;

	updateInfo.ResetIndexSipVia();
	updateInfo.ClearSipVia();

	for(std::size_t i=0; i < meta->iVia.size(); i++)
	{
		updateInfo.SetSipVia(meta->iVia[i]);
	}

	if (meta->iVia[0]->ConnectionType() == net::protocol::TCP && meta->iVia[0]->IsKeepAlive())
		updateInfo.EnableKeepAlive();

// CSeq value
	if (meta->iCSeq)
		updateInfo.SetSipSequenceNumber(meta->iCSeq->Value());
}

bool VS_SIPRequest::MakeMESSAGE(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo,
	string_view message, eContentType ct) {

	if (m_sip_message)
		m_sip_message->Clear();

	updateInfo.IsRequest(true);
	updateInfo.IncreaseMySequenceNumber();

	updateInfo.SetMessageType(TYPE_MESSAGE);
	SetContentType(ct);			// For VS_SIPField_ContentType
	updateInfo.SetContentType(ct);		// For VS_SIPMessage::Encode()

	size_t routeSetSize = getInfo.GetSipRouteSetSize();
	if (routeSetSize)
	{
		updateInfo.ResetSipRouteIndex();
		for (size_t i = 0; i < routeSetSize; ++i)
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Route, getInfo))
				return false;
	}

	auto auth_scheme = getInfo.GetAuthScheme();
	if (auth_scheme) {
		VS_SIPObjectFactory* factory = VS_SIPObjectFactory::Instance();
		assert(factory);

		auth_scheme->method(VS_SIPObjectFactory::GetMethod(TYPE_INVITE));
		if (std::dynamic_pointer_cast<VS_SIPAuthGSS>(auth_scheme) &&
			!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Authorization, getInfo)) {
			return false;
		}
	}

	updateInfo.MsgAliveTick(std::chrono::steady_clock::now());
	//bool timerExt = info->IsSessionTimerEnabled() && info->GetTimerExtention().refresher != VS_SIPParserInfo::TimerExtention::REFRESHER_INVALID;
	const bool timerExt = getInfo.IsSessionTimerEnabled();	// refresher will be inserted in SIPHeader_SessionExpires (VS_SIPField_SesssionExpires.cpp, line 108)

	/* the field choice based on table 1,2 from rfc3428: http://www.rfc-editor.org/rfc/rfc3428.txt */
	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Accept, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||

		//!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info)||
		!timerExt ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_SessionExpires, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Date, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Supported, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentType, getInfo) /*||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader_ContentLength, info) ||
		/*!InsertSIPField(VS_SIPObjectFactory::SIPHeader_Via, info)*/)
		return false;

	if (!InsertInstantMessage(getInfo, message))
		return false;

	return true;
}
