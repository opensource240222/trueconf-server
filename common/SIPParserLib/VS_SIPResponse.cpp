#include "VS_SIPResponse.h"
#include "VS_SIPRequest.h"
#include "VS_SIPMetaField.h"
#include "VS_SDPMetaField.h"
#include "VS_SIPAuthDigest.h"
#include "VS_SIPField_Contact.h"
#include "VS_SIPField_CSeq.h"
#include "VS_SIPField_StartLine.h"
#include "VS_SIPField_Auth.h"
#include "VS_SIPField_Expires.h"
#include "VS_SIPField_SessionExpires.h"
#include "VS_SIPAuthGSS.h"
#include "../std/cpplib/VS_Utils.h"

//////////
#include "VS_SIPURI.h"
#include "VS_SIPField_To.h"
////////
#include <chrono>
#include "VS_TimerExtention.h"

VS_SIPResponse::VS_SIPResponse()
{

}

VS_SIPResponse::~VS_SIPResponse()
{

}

bool VS_SIPResponse::MakeOnUpdateResponseOK(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo) {
	return MakeOnInviteResponseOK(req, getInfo, updateInfo);
}

bool VS_SIPResponse::MakeOnInviteResponseOK(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo)
{
	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	const auto req_sip_meta = req->GetSIPMetaField();
	if ( !req_sip_meta )
		return false;

	updateInfo.IsRequest(false);
	updateInfo.SetMessageType( req_sip_meta->iCSeq->GetType() );
	updateInfo.SetSipSequenceNumber( req_sip_meta->iCSeq->Value() );
	updateInfo.SetResponseCode( 200 );
	updateInfo.SetResponseStr( "OK" );
	SetContentType(CONTENTTYPE_SDP);			// For VS_SIPField_ContentType
	updateInfo.SetContentType(CONTENTTYPE_SDP);		// For VS_SIPMessage::Encode()

	if (!getInfo.IsAnswered())
	{
		// First INVITE (and no tag at 180 Ringing)
		if (getInfo.GetTagMy().empty())
		{
			char key[32 + 1] = { 0 };
			VS_GenKeyByMD5(key);
			if (!updateInfo.SetTagMy(key))
				return false;
		}
	}
	else
	{
		// ReINVITE
		updateInfo.IncreaseSdpSessionVersion();
	}

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) )
		return false;

// Via
	const auto meta = req->GetSIPMetaField();
	if ( !meta || (meta->iVia.empty()) )
		return false;

	updateInfo.ResetIndexSipVia();
	updateInfo.ClearSipVia();

	for(std::size_t i=0; i < meta->iVia.size(); i++)
	{
		updateInfo.SetSipVia( meta->iVia[i] );
		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo) )
			return false;
	}

	const size_t routeSetSize = getInfo.GetSipRouteSetSize();
	if (routeSetSize)
	{
		updateInfo.ResetSipRouteIndex();
		for (size_t i = 0; i < routeSetSize; ++i)
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_RecordRoute, getInfo))
			return false;
	}

	const bool timerExt = getInfo.IsSessionTimerEnabled() && getInfo.GetTimerExtention().refresher != REFRESHER::REFRESHER_INVALID;
	const bool requireHeader = getInfo.IsSessionTimerInRequire();

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Allow, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) ||
		 (requireHeader && timerExt && !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Require, getInfo)) ||
		 (timerExt && !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Supported, getInfo)) ||
		 (timerExt && !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_SessionExpires, getInfo)) ||
		 (timerExt && !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MinSE, getInfo)) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentType, getInfo) /*||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_ContentLength, info)*/ )
		return false;

	if ( !InsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_Version, getInfo) ||
		 !InsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_Origin, getInfo) ||
		 !InsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_SessionName, getInfo) ||
		 //!InsertSDPField(VS_SDPObjectFactory::SDPHeader_Connection, info) ||
		 !InsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_Bandwidth, getInfo) ||
		 !InsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_Time, getInfo) ||
		 !InsertMediaStreams(getInfo) )
		return false;

	return true;
}

bool VS_SIPResponse::MakeOnByeResponseOK(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo)
{
	if ( !req )
		return false;

	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	const auto req_sip_meta = req->GetSIPMetaField();
	if ( !req_sip_meta )
		return false;

	updateInfo.IsRequest(false);
	updateInfo.SetMessageType( TYPE_BYE );
	updateInfo.SetSipSequenceNumber( req_sip_meta->iCSeq->Value() );
	updateInfo.SetResponseCode( 200 );
	updateInfo.SetResponseStr( "OK" );

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) )
		return false;

// Via
	const auto meta = req->GetSIPMetaField();
	if ( !meta || (meta->iVia.empty()) )
		return false;

	updateInfo.ResetIndexSipVia();
	updateInfo.ClearSipVia();

	for(std::size_t i=0; i < meta->iVia.size(); i++)
	{
		updateInfo.SetSipVia( meta->iVia[i] );
		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo) )
			return false;
	}

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Contact, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo) )
		return false;

	return true;
}

bool VS_SIPResponse::MakeOnByeResponseOK(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo)
{
	updateInfo.IsRequest(false);
	updateInfo.SetMessageType(TYPE_BYE);
	updateInfo.SetResponseCode(200);
	updateInfo.SetResponseStr("OK");
	return MakeSimpleResponse(getInfo, updateInfo);
//	if ( !info/* || !req */)
//		return false;
//
//	if ( m_sip_meta_field )
//		m_sip_meta_field->Clear();
//
//	if ( m_sdp_meta_field )
//		m_sdp_meta_field->Clear();
//
//	//VS_SIPMetaField* req_sip_meta = req->GetSIPMetaField();
//	//if ( !req_sip_meta )
//	//	return false;
//
//	info->IsRequest(false);
//	info->SetMessageType( TYPE_BYE );
////	info->SetSIPSequenceNumber( req_sip_meta->iCSeq->Value() );
//	info->SetResponseCode( 200 );
//	info->SetResponseStr( "OK" );
//
//	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader_StartLine, info) )
//		return false;
//
//// Via
//	//VS_SIPMetaField* meta = req->GetSIPMetaField();
//	//if ( !meta || (meta->iVia.size() < 1) )
//	//	return false;
//
//	info->ResetIndexSIPVia();
////	info->ClearSIPVia();
//
//	for(unsigned int i=0; i < info->GetSIPViaSize()/*meta->iVia.size()*/; i++)
//	{
////		info->SetSIPVia( meta->iVia[i] );
//		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Via, info) )
//			return false;
//	}
//
//	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader_MaxForwards, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_From, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_CallID, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_CSeq, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_ContentLength, info) )
//		return false;
//
//	return true;
}

bool VS_SIPResponse::MakeOnInfoResponseOK(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo)
{
	if (!req )
		return false;

	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	auto req_sip_meta = req->GetSIPMetaField();
	if ( !req_sip_meta )
		return false;

	updateInfo.IsRequest(false);
	updateInfo.SetMessageType( TYPE_INFO );
	updateInfo.SetSipSequenceNumber( req_sip_meta->iCSeq->Value() );
	updateInfo.SetResponseCode( 200 );
	updateInfo.SetResponseStr( "OK" );
	SetContentType(CONTENTTYPE_MEDIACONTROL_XML);			// For VS_SIPField_ContentType
	updateInfo.SetContentType(CONTENTTYPE_MEDIACONTROL_XML);		// For VS_SIPMessage::Encode()

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) )
		return false;

// Via
	auto meta =req->GetSIPMetaField();
	if ( !meta || meta->iVia.empty() )
		return false;

	updateInfo.ResetIndexSipVia();
	updateInfo.ClearSipVia();

	for(std::size_t i=0; i < meta->iVia.size(); i++)
	{
		updateInfo.SetSipVia( meta->iVia[i] );
		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo) )
			return false;
	}

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Contact, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo) )
		return false;

	return true;
}

bool VS_SIPResponse::MakeResponseUnsupported(const VS_SIPRequest *req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo)
{
	if (!MakeOnInfoResponseUnsupported(req, getInfo, updateInfo)) return false;
	m_sip_meta_field->iStartLine->SetResponseStr("Not Implemented");
	m_sip_meta_field->iStartLine->SetResponseCode(501);
	return true;
}

bool VS_SIPResponse::MakeOnInfoResponseUnsupported(const VS_SIPRequest *req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo)
{
	if (!req )
		return false;

	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	auto req_sip_meta = req->GetSIPMetaField();
	if ( !req_sip_meta )
		return false;

	updateInfo.IsRequest(false);
	updateInfo.SetMessageType( req->GetSIPMetaField()->iCSeq->GetType() );
	updateInfo.SetSipSequenceNumber( req_sip_meta->iCSeq->Value() );
	updateInfo.SetResponseCode( 415 );
	updateInfo.SetResponseStr( "Unsupported Media Type" );
	//SetContentType(CONTENTTYPE_MEDIACONTROL_XML);			// For VS_SIPField_ContentType
	//info->SetContentType(CONTENTTYPE_MEDIACONTROL_XML);		// For VS_SIPMessage::Encode()

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) )
		return false;

// Via
	auto meta = req->GetSIPMetaField();
	if ( !meta || (meta->iVia.empty()) )
		return false;

	updateInfo.ResetIndexSipVia();
	updateInfo.ClearSipVia();

	for(std::size_t i=0; i < meta->iVia.size(); i++)
	{
		updateInfo.SetSipVia( meta->iVia[i] );
		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo) )
			return false;
	}

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo) )
		return false;

	return true;
}

bool VS_SIPResponse::MakeOnOptionsResponseOK(const VS_SIPRequest *req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo)
{

	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	const auto req_sip_meta = req->GetSIPMetaField();
	if ( !req_sip_meta )
		return false;

	updateInfo.IsRequest(false);
	updateInfo.SetMessageType( TYPE_OPTIONS );
	updateInfo.SetSipSequenceNumber( req_sip_meta->iCSeq->Value() );
	updateInfo.SetResponseCode( 200 );
	updateInfo.SetResponseStr( "OK" );

	// the tag for the To field is constructed simply by adding "resptc" to the tag received in From
	if (getInfo.GetTagMy().empty())
	{
		if (!getInfo.GetTagSip().empty()) {
			const char resptc[] = "resptc";
			std::string tag;
			tag.reserve(getInfo.GetTagSip().length() + sizeof(resptc) - 1);
			tag += getInfo.GetTagSip();
			tag += resptc;
			if (!updateInfo.SetTagMy(std::move(tag))) return false;
		}
	}

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) )
		return false;

// Via
	const auto meta = req->GetSIPMetaField();
	if ( !meta || (meta->iVia.empty()) )
		return false;

	updateInfo.ResetIndexSipVia();
	updateInfo.ClearSipVia();

	for(std::size_t i=0; i < meta->iVia.size(); i++)
	{
		updateInfo.SetSipVia( meta->iVia[i] );
		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo) )
			return false;
	}

	const bool timerEnabled = getInfo.IsSessionTimerEnabled();

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Allow, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Accept, getInfo) ||
		 (timerEnabled && !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Supported, getInfo)) /*||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_ContentLength, info)*/ )
		return false;

	return true;
}

bool VS_SIPResponse::MakeOnInviteResponseBusyHere(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo)
{
	if ( !req )
		return false;

	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	const auto req_sip_meta = req->GetSIPMetaField();
	if ( !req_sip_meta )
		return false;

	updateInfo.IsRequest(false);
	updateInfo.SetMessageType(  req_sip_meta->iCSeq->GetType() );
	updateInfo.SetSipSequenceNumber( req_sip_meta->iCSeq->Value() );
	updateInfo.SetResponseCode( 486 );
	updateInfo.SetResponseStr( "Busy Here" );

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) )
		return false;

// Via
	auto meta = req->GetSIPMetaField();
	if ( !meta || (meta->iVia.empty()) )
		return false;

	updateInfo.ResetIndexSipVia();
	updateInfo.ClearSipVia();

	for(std::size_t i=0; i < meta->iVia.size(); i++)
	{
		updateInfo.SetSipVia( meta->iVia[i] );
		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo) )
			return false;
	}

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo) )
		return false;

	return true;
}

bool VS_SIPResponse::MakeOnInviteResponseBusyHere(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo)
{
	updateInfo.IsRequest(false);
	updateInfo.SetMessageType(TYPE_INVITE);
	updateInfo.SetResponseCode(486);
	updateInfo.SetResponseStr("Busy Here");
	return MakeSimpleResponse(getInfo, updateInfo, true);
//	if ( !info )
//		return false;
//
//	if ( m_sip_meta_field )
//		m_sip_meta_field->Clear();
//
//	if ( m_sdp_meta_field )
//		m_sdp_meta_field->Clear();
//
//	//VS_SIPMetaField* req_sip_meta = req->GetSIPMetaField();
//	//if ( !req_sip_meta )
//	//	return false;
//
//	info->IsRequest(false);
//	info->SetMessageType( TYPE_INVITE );
////	info->SetSIPSequenceNumber( req_sip_meta->iCSeq->Value() );
////	info->IncreaseMySequenceNumber();
////	info->SetSIPSequenceNumber( info->GetMySequenceNumber() );
//	info->SetResponseCode( 486 );
//	info->SetResponseStr( "Busy Here" );
//
//	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader_StartLine, info) )
//		return false;
//
//// Via
//	//VS_SIPMetaField* meta = req->GetSIPMetaField();
//	//if ( !meta || (meta->iVia.size() < 1) )
//	//	return false;
//
//	info->ResetIndexSIPVia();
////	info->ClearSIPVia();
//
//	for(unsigned int i=0; i < info->GetSIPViaSize()/*meta->iVia.size()*/; i++)
//	{
////		info->SetSIPVia( meta->iVia[i] );
//		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Via, info) )
//			return false;
//	}
//
//	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader_MaxForwards, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_From, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_CallID, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_CSeq, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_ContentLength, info) )
//		return false;
//
//	return true;
}

bool VS_SIPResponse::MakeOnInviteResponseTrying(const VS_SIPGetInfoInterface& getInfo,
	VS_SipUpdateInfoInterface& updateInfo)
{
	updateInfo.IsRequest(false);
	updateInfo.SetMessageType(TYPE_INVITE);
	updateInfo.SetResponseCode(100);
	updateInfo.SetResponseStr("Trying");

	// 8.2.6.2 Headers and Tags: "UAS MUST add a tag to the To header field in the response"
	if (getInfo.GetTagMy().empty())
	{
		char key[32 + 1] = { 0 };
		VS_GenKeyByMD5(key);
		if (!updateInfo.SetTagMy(key))
			return false;
	}

	return MakeSimpleResponse(getInfo, updateInfo, true);
}


bool VS_SIPResponse::MakeOnInviteResponseRinging(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo)
{
	updateInfo.SetMessageType(TYPE_INVITE);
	updateInfo.SetResponseCode(180);
	updateInfo.SetResponseStr("Ringing");

	assert(!getInfo.GetTagMy().empty());

	return MakeSimpleResponse(getInfo, updateInfo, true);
//	if ( !info )
//		return false;
//
//	if ( m_sip_meta_field )
//		m_sip_meta_field->Clear();
//
//	if ( m_sdp_meta_field )
//		m_sdp_meta_field->Clear();
//
//	//VS_SIPMetaField* req_sip_meta = req->GetSIPMetaField();
//	//if ( !req_sip_meta )
//	//	return false;
//
//	info->IsRequest(false);
//	info->SetMessageType( TYPE_INVITE );
////	info->SetSIPSequenceNumber( req_sip_meta->iCSeq->Value() );
////	info->IncreaseMySequenceNumber();
////	info->SetSIPSequenceNumber( info->GetMySequenceNumber() );
//	info->SetResponseCode(180);
//	info->SetResponseStr("Ringing");
//
//	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader_StartLine, info) )
//		return false;
//
//// Via
//	//VS_SIPMetaField* meta = req->GetSIPMetaField();
//	//if ( !meta || (meta->iVia.size() < 1) )
//	//	return false;
//
//	info->ResetIndexSIPVia();
////	info->ClearSIPVia();
//
//	for(unsigned int i=0; i < info->GetSIPViaSize()/*meta->iVia.size()*/; i++)
//	{
////		info->SetSIPVia( meta->iVia[i] );
//		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Via, info) )
//			return false;
//	}
//
//	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader_MaxForwards, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_From, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_CallID, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_CSeq, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) ||
//		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_ContentLength, info) )
//		return false;
//
//	return true;
}

bool VS_SIPResponse::MakeOnInviteResponseNotFound(const VS_SIPGetInfoInterface& getInfo,
	VS_SipUpdateInfoInterface& updateInfo)
{
	updateInfo.SetMessageType(TYPE_INVITE);
	updateInfo.SetResponseCode(404);
	updateInfo.SetResponseStr("Not Found");

	return MakeSimpleResponse(getInfo, updateInfo, true);
}

bool VS_SIPResponse::MakeOnInviteResponseTemporarilyUnavailable(const VS_SIPGetInfoInterface& getInfo,
	VS_SipUpdateInfoInterface& updateInfo)
{
	updateInfo.SetMessageType(TYPE_INVITE);
	updateInfo.SetResponseCode(480);
	updateInfo.SetResponseStr("Temporarily Unavailable");

	assert(!getInfo.GetTagMy().empty());

	return MakeSimpleResponse(getInfo, updateInfo, true);
}

bool VS_SIPResponse::MakeOnInviteResponseUnauthorized(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo, const std::string& nonce) {

	if (!req)
		return false;

	if (m_sip_meta_field)
		m_sip_meta_field->Clear();

	if (m_sdp_meta_field)
		m_sdp_meta_field->Clear();

	const auto meta = req->GetSIPMetaField();
	if (!meta || (meta->iVia.empty()))
		return false;

	updateInfo.IsRequest(false);
	updateInfo.SetMessageType(TYPE_INVITE);
	updateInfo.SetSipSequenceNumber(meta->iCSeq->Value());
	updateInfo.SetResponseCode(401);
	updateInfo.SetResponseStr("Unauthorized");

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo))
		return false;

	// Via
	updateInfo.ResetIndexSipVia();
	updateInfo.ClearSipVia();

	for (std::size_t i = 0; i < meta->iVia.size(); i++)
	{
		updateInfo.SetSipVia(meta->iVia[i]);
		if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo))
			return false;
	}

	updateInfo.ClearSipContact();
	updateInfo.SetSipContact(meta->iContact);

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Allow, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Supported, getInfo))
		return false;

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Expires, getInfo)) return false;

	m_sip_meta_field->AddField(VS_SIPField_Auth_WWWAuthenticate_Instance());

	VS_SIPBuffer challenge;
#ifdef _SVKS_M_BUILD_
	challenge.AddData("WWW-Authenticate: DIGEST realm=\"svks\", nonce=\"");
#else
	challenge.AddData("WWW-Authenticate: DIGEST realm=\"trueconf\", nonce=\"");
#endif
	challenge.AddData(nonce);
	challenge.AddData("\"\r\n");

	GetSIPMetaField()->iAuthHeader[0]->Decode(challenge);
	return true;
}

bool VS_SIPResponse::MakeOnInviteResponseServiceUnavailable(const VS_SIPGetInfoInterface& getInfo,
	VS_SipUpdateInfoInterface& updateInfo)
{
	updateInfo.SetMessageType(TYPE_INVITE);
	updateInfo.SetResponseCode(503);
	updateInfo.SetResponseStr("Service Unavailable");

	return MakeSimpleResponse(getInfo, updateInfo, true)
		&& InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_RetryAfter, getInfo);
}

bool VS_SIPResponse::MakeResponseUseProxy(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo, const std::string& nonce)
{
	if ( !req )
		return false;

	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	const auto req_sip_meta = req->GetSIPMetaField();
	if ( !req_sip_meta )
		return false;

	updateInfo.IsRequest(false);
	updateInfo.SetMessageType( req_sip_meta->iCSeq->GetType() );
	updateInfo.SetSipSequenceNumber( req_sip_meta->iCSeq->Value() );
//	info->SetResponseCode( 302 );
//	info->SetResponseStr( "Moved Termporarily" );
	updateInfo.SetResponseCode( 305 );
	updateInfo.SetResponseStr( "Use Proxy" );

	//char key[32 + 1] = {0};

	//VS_GenKeyByMD5(key);
	//if ( !info->SetTag_my(key) )
	//	return false;

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) )
		return false;

// Via
	const auto meta = req->GetSIPMetaField();
	if ( !meta || (meta->iVia.empty()) )
		return false;

	updateInfo.ResetIndexSipVia();
	updateInfo.ClearSipVia();

	for(std::size_t i=0; i < meta->iVia.size(); i++)
	{
		updateInfo.SetSipVia( meta->iVia[i] );
		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo) )
			return false;
	}

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Contact, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo) )
		return false;

	return true;
}

bool VS_SIPResponse::MakeMovedPermanently(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo)
{
	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	updateInfo.IsRequest(false);
	updateInfo.SetResponseCode( 301 );
	updateInfo.SetResponseStr( "Moved Permanently" );

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo ) )
		return false;

	// Via
	updateInfo.ResetIndexSipVia();
	for(unsigned int i=0; i < getInfo.GetSipViaSize(); i++)
	{
		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo) )
			return false;
	}

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		//!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Contact, getInfo) ||
		!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo) )
		return false;

	return true;
}

bool VS_SIPResponse::MakeRequestTimeout(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo)
{
	if ( !req )
		return false;

	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	const auto req_sip_meta = req->GetSIPMetaField();
	if ( !req_sip_meta )
		return false;

	updateInfo.IsRequest(false);
	updateInfo.SetMessageType(  req_sip_meta->iCSeq->GetType() );
	updateInfo.SetSipSequenceNumber(req_sip_meta->iCSeq->Value());
	updateInfo.SetResponseCode( 408 );
	updateInfo.SetResponseStr( "Request Timeout" );

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) )
		return false;

// Via
	const auto meta = req->GetSIPMetaField();
	if ( !meta || (meta->iVia.empty()) )
		return false;

	updateInfo.ResetIndexSipVia();
	updateInfo.ClearSipVia();

	for(std::size_t i=0; i < meta->iVia.size(); i++)
	{
		updateInfo.SetSipVia( meta->iVia[i] );
		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo) )
			return false;
	}

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Contact, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo) )
		return false;

	return true;
}

bool VS_SIPResponse::MakeOnRegisterResponseUnauthorized(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo, const std::string& nonce)
{
	if ( !MakeOnRegisterResponseOK(req, getInfo, updateInfo)) return false;
	GetSIPMetaField()->iStartLine->SetResponseCode( 401 );
	GetSIPMetaField()->iStartLine->SetResponseStr( "Unauthorized" );
	m_sip_meta_field->AddField( VS_SIPField_Auth_WWWAuthenticate_Instance() );

	VS_SIPBuffer challenge;
#ifdef _SVKS_M_BUILD_
	challenge.AddData("WWW-Authenticate: DIGEST realm=\"svks\", nonce=\"");
#else
	challenge.AddData("WWW-Authenticate: DIGEST realm=\"trueconf\", nonce=\"");
#endif
	challenge.AddData(nonce);
	challenge.AddData("\"\r\n");

	GetSIPMetaField()->iAuthHeader[0]->Decode(challenge);
	return true;
}

bool VS_SIPResponse::MakeOnRegisterResponseForbidden(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo)
{
	if ( !MakeOnRegisterResponseOK(req, getInfo, updateInfo)) return false;
	GetSIPMetaField()->iStartLine->SetResponseCode( 403 );
	GetSIPMetaField()->iStartLine->SetResponseStr( "Forbidden" );
	return true;
}

bool VS_SIPResponse::MakeOnRegisterResponseOK(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo)
{
	if ( !req )
		return false;

	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	auto meta = req->GetSIPMetaField();
	if ( !meta || (meta->iVia.empty()) )
		return false;

	updateInfo.IsRequest(false);
	updateInfo.SetMessageType( TYPE_REGISTER );
	updateInfo.SetSipSequenceNumber( meta->iCSeq->Value() );
	updateInfo.SetResponseCode( 200 );
	updateInfo.SetResponseStr( "OK" );

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) )
		return false;

// Via
	updateInfo.ResetIndexSipVia();
	updateInfo.ClearSipVia();

	for(unsigned int i=0; i < meta->iVia.size(); i++)
	{
		updateInfo.SetSipVia( meta->iVia[i] );
		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo) )
			return false;
	}

	if (!getInfo.GetSipContact()) {
		updateInfo.ClearSipContact();
		updateInfo.SetSipContact(meta->iContact);  // bug 32183
	}

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Allow, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Supported, getInfo) /*||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info)*/)
		 return false;

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Expires, getInfo)) return false;

/////////////////////////////////////
//// Set Contact-header from Request
/////////////////////////////////////
//	VS_SIPObjectFactory* factory = VS_SIPObjectFactory::Instance();
//	if ( !factory )
//		return false;
//
//	VS_BaseField* field = 0;
//	if ( e_ok != factory->CreateField(VS_SIPObjectFactory::SIPHeader_Contact, field) )
//		return false;
//
//	if ( !field )
//		return false;
//
//	if ( !m_sip_meta_field ) m_sip_meta_field = new VS_SIPMetaField;
//	if ( !m_sip_meta_field ) return false;
//
//	m_sip_meta_field->AddField(field);
//
//	if ( !m_sip_meta_field || !m_sip_meta_field->iContact || !meta->iContact )
//		return false;
//
//	*(m_sip_meta_field->iContact) = *(meta->iContact);
/////////////////////////////////////

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo) )
		return false;

	return true;
}

bool VS_SIPResponse::MakeOnSubscribeResponseOK(const VS_SIPRequest* req,
	const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo)
{
	if (!req )
		return false;

	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	const auto req_sip_meta = req->GetSIPMetaField();
	if ( !req_sip_meta )
		return false;

	updateInfo.IsRequest(false);
	updateInfo.SetMessageType( TYPE_SUBSCRIBE );
	updateInfo.SetSipSequenceNumber( req_sip_meta->iCSeq->Value() );
	updateInfo.SetResponseCode( 200 );
	updateInfo.SetResponseStr( "OK" );

	if ( req_sip_meta->iExpires )
		updateInfo.SetExpires( req_sip_meta->iExpires->Value() );

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) )
		return false;

// Via
	const auto meta = req->GetSIPMetaField();
	if ( !meta || (meta->iVia.empty()) )
		return false;

	updateInfo.ResetIndexSipVia();
	updateInfo.ClearSipVia();

	for(unsigned int i=0; i < meta->iVia.size(); i++)
	{
		updateInfo.SetSipVia( meta->iVia[i] );
		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo) )
			return false;
	}

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Contact, getInfo) )
		 return false;

	if ( meta->iExpires && !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Expires, getInfo) )
		return false;

	 if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo) )
		return false;

	return true;
}

bool VS_SIPResponse::MakeSimpleResponse(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo,
	const bool responseToInvite)
{
	if ( m_sip_meta_field )
		m_sip_meta_field->Clear();

	if ( m_sdp_meta_field )
		m_sdp_meta_field->Clear();

	updateInfo.IsRequest(false);

	if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo) )
		return false;

// Via
	updateInfo.ResetIndexSipVia();
	for(std::size_t i=0; i < getInfo.GetSipViaSize()/*meta->iVia.size()*/; i++)
	{
		if ( !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo) )
			return false;
	}

	if (responseToInvite)
	{
		const size_t routeSetSize = getInfo.GetSipRouteSetSize();
		if (routeSetSize)
		{
			updateInfo.ResetSipRouteIndex();
			for (size_t i = 0; i < routeSetSize; ++i)
				if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_RecordRoute, getInfo))
					return false;
		}
	}

	if (GetResponseCode() != 100)
	{
		if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) ||
			!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo))
			return false;
	}

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_To, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) ||
		 //!InsertSIPField(VS_SIPObjectFactory::SIPHeader_Contact, info) ||
		 !InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo) )
		return false;

	return true;
}

bool VS_SIPResponse::MakeOnCancelResponseOK(const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &update_info)
{
	update_info.IsRequest(false);
	update_info.SetMessageType(TYPE_CANCEL);
	update_info.SetResponseCode(200);
	update_info.SetResponseStr("OK");
	return MakeSimpleResponse(getInfo, update_info);
}


void VS_SIPResponse::FillUriSetForEstablishedDialog(const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo) const
{
	if (m_sip_meta_field && m_sip_meta_field->iContact)
	{
		updateInfo.FillUriSetForEstablishedDialog(m_sip_meta_field->iContact->GetLastURI(), m_sip_meta_field->iRouteSet, false);
	}
}

bool VS_SIPResponse::MakeOnMessageResponseOK(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo)
{
	if (m_sip_meta_field)
		m_sip_meta_field->Clear();

	auto req_sip_meta = req->GetSIPMetaField();
	if (!req_sip_meta)
		return false;

	updateInfo.IsRequest(false);
	updateInfo.SetMessageType(TYPE_MESSAGE);
	updateInfo.SetSipSequenceNumber(req_sip_meta->iCSeq->Value());
	updateInfo.SetResponseCode(200);
	updateInfo.SetResponseStr("OK");
	SetContentType(CONTENTTYPE_TEXT_PLAIN);			// For VS_SIPField_ContentType
	updateInfo.SetContentType(CONTENTTYPE_TEXT_PLAIN);		// For VS_SIPMessage::Encode()
	updateInfo.EnableSessionTimer();
	updateInfo.MsgAliveTick(std::chrono::steady_clock::now());

	// some softphones don't set session expires field, so it will not be fill in VS_SIPMessage::FillInfoByInviteMessage
	if (std::chrono::duration_cast<std::chrono::seconds>(getInfo.GetTimerExtention().refreshPeriod).count() < 0)
		updateInfo.SetRefreshPeriod(std::chrono::seconds(90));	// min session expires

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo))
		return false;

	// Via
	updateInfo.ResetIndexSipVia();
	for (std::size_t i = 0; i < getInfo.GetSipViaSize(); ++i)
	{
		if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo))
			return false;
	}

	size_t routeSetSize = getInfo.GetSipRouteSetSize();
	if (routeSetSize)
	{
		updateInfo.ResetSipRouteIndex();
		for (size_t i = 0; i < routeSetSize; ++i)
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_RecordRoute, getInfo))
				return false;
	}

	return !(!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Accept, getInfo) || !
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) || !
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) || !
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) || !
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) || !
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) || !
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_To, getInfo) || !InsertSIPField(
			VS_SIPObjectFactory::SIPHeader::SIPHeader_SessionExpires, getInfo) || !InsertSIPField(
			VS_SIPObjectFactory::SIPHeader::SIPHeader_Supported, getInfo));
}

bool VS_SIPResponse::MakeOnMessageResponseNotFound(const VS_SIPGetInfoInterface &getInfo, VS_SipUpdateInfoInterface &updateInfo){
	updateInfo.IsRequest(false);
	updateInfo.SetMessageType(TYPE_CANCEL);
	updateInfo.SetResponseCode(404);
	updateInfo.SetResponseStr("Not Found");
	return MakeSimpleResponse(getInfo, updateInfo);
}

bool VS_SIPResponse::MakeOnMessageResponseAccepted(const VS_SIPRequest* req, const VS_SIPGetInfoInterface &getInfo,
	VS_SipUpdateInfoInterface &updateInfo)
{
	if (!req)
		return false;

	if (m_sip_meta_field)
		m_sip_meta_field->Clear();

	const auto req_sip_meta = req->GetSIPMetaField();
	if (!req_sip_meta)
		return false;


	updateInfo.IsRequest(false);
	updateInfo.SetMessageType(TYPE_MESSAGE);
	updateInfo.SetSipSequenceNumber(req_sip_meta->iCSeq->Value());
	updateInfo.SetResponseCode(202);
	updateInfo.SetResponseStr("Accepted");
	SetContentType(CONTENTTYPE_TEXT_PLAIN);			// For VS_SIPField_ContentType
	updateInfo.SetContentType(CONTENTTYPE_TEXT_PLAIN);		// For VS_SIPMessage::Encode()
	updateInfo.EnableSessionTimer();

	// some softphones don't set session expires field
	if (req_sip_meta->iSessionExpires)
		updateInfo.SetRefreshPeriod(req_sip_meta->iSessionExpires->GetRefreshInterval());
	else
		updateInfo.SetRefreshPeriod(std::chrono::seconds(90));

	if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_StartLine, getInfo))
		return false;

	// Via
	auto meta = req->GetSIPMetaField();
	if (!meta || (meta->iVia.empty()))
		return false;

	updateInfo.ResetIndexSipVia();
	updateInfo.ClearSipVia();

	for (std::size_t i = 0; i < meta->iVia.size(); i++)
	{
		updateInfo.SetSipVia(meta->iVia[i]);
		if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo))
			return false;
	}

	const size_t routeSetSize = getInfo.GetSipRouteSetSize();
	if (routeSetSize)
	{
		updateInfo.ResetSipRouteIndex();
		for (size_t i = 0; i < routeSetSize; ++i)
			if (!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_RecordRoute, getInfo))
				return false;
	}

	return !(!InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Accept, getInfo) || !
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_UserAgent, getInfo) || !
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, getInfo) || !
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CSeq, getInfo) || !
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_From, getInfo) || !
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_MaxForwards, getInfo) || !
		InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_To, getInfo) || !InsertSIPField(
			VS_SIPObjectFactory::SIPHeader::SIPHeader_SessionExpires, getInfo) || !InsertSIPField(
			VS_SIPObjectFactory::SIPHeader::SIPHeader_Supported, getInfo));
}
