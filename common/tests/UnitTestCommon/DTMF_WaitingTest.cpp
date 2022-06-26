#include "DTMF_WaitingTest.h"
#include "TrueGateway/sip/VS_SIPGetInfoImpl.h"
#include "TrueGateway/sip/VS_SIPUpdateInfoImpl.h"
#include "SIPParserLib/VS_SIPMetaField.h"
#include "SIPParserLib/VS_SIPField_To.h"
#include "SIPParserLib/VS_SIPField_StartLine.h"
#include "SIPParserLib/VS_SIPField_CSeq.h"

std::string DTMFTest::dialog_id = "123456789";
std::string DTMFTest::from = "from@host.org";
std::string DTMFTest::to = "#sip:to@host.org";
std::string DTMFTest::to_name = "to";
std::string DTMFTest::to_host = "host.org";

std::string DTMFTest::display_name = "display_name";
std::string DTMFTest::server = "localhost";

const int DTMF_SYMBOLS = 4;

const int REFRESH_PERIOD_SEC = 100;

/*
 * In this test we verify that next dtmf symbol will not be sent
 * while answer 200 ok is not received
 */
TEST_F(DTMFTest,WaitForAnswer200OK){
	auto internal_info = SimulateIncomingInvite();
	ASSERT_TRUE(internal_info);

	std::string dtmf_symbols = "1234";
	for (unsigned i = 0; i < dtmf_symbols.length(); ++i){
		internal_info->AddDTMF(dtmf_symbols[i]);
	}

	for (unsigned i = 0; i < dtmf_symbols.length(); ++i)
	{
		pParser->Timeout();									// put dtmf request to output queue
		auto dtmf_info_message = SendDTMF();
		ASSERT_TRUE(dtmf_info_message.first.lock());
		ASSERT_TRUE(dtmf_info_message.second);

		// try send next dtmf before 200 ok answer arrived
		pParser->Timeout();									// put dtmf request to output queue
		auto false_send_res = SendDTMF();					// must haven't the result

		// no sending while haven't answer 200 ok
		ASSERT_FALSE(false_send_res.first.lock());
		ASSERT_FALSE(false_send_res.second);

		ASSERT_TRUE(ReceiveResponseOnDTMFRequest(dtmf_info_message));	// receive 200 ok
	}
}

/*
* In this test we verify that next dtmf symbol will not be sent
* while answer dtmf answer timeout not passed
*/
TEST_F(DTMFTest, RequestTimeout){
	auto pCallInfo = makeInfo(dialog_id, from, to, display_name, strand);
	auto internal_info = SimulateIncomingInvite();
	ASSERT_TRUE(internal_info);

	std::string dtmf_symbols = "1234";
	for (unsigned i = 0; i < dtmf_symbols.length(); ++i){
		internal_info->AddDTMF(dtmf_symbols[i]);
	}

	for (unsigned i = 0; i < dtmf_symbols.length(); ++i)
	{
		pParser->Timeout();										// put dtmf request to output queue
		auto dtmf_info_message = SendDTMF();
		auto ctx = dtmf_info_message.first.lock();
		ASSERT_TRUE(ctx);
		ASSERT_TRUE(dtmf_info_message.second);

		// try send next dtmf before 200 ok answer arrived
		pParser->Timeout();										// put dtmf request to output queue
		auto false_send_res = SendDTMF();

		// no sending while haven't answer 200 ok
		ASSERT_FALSE(false_send_res.first.lock());
		ASSERT_FALSE(false_send_res.second);

		// don't send 200 OK but make last request time long time ago
		ctx->SetDTMFRequestTime(std::chrono::steady_clock::now() - std::chrono::seconds(50));
	}
}

/*
* In this test we verify that dtmf symbols ',' and 'p'
* really increase dtmf pause sending
*/
TEST_F(DTMFTest, PauseTest){
	auto internal_info = SimulateIncomingInvite();
	ASSERT_TRUE(internal_info);

	std::string dtmf_symbols = ",,1,,23p4pwp12,w,3";
	for (unsigned i = 0; i < dtmf_symbols.length(); ++i)
	{
		internal_info->AddDTMF(dtmf_symbols[i]);
	}

	for (unsigned i = 0; i < dtmf_symbols.length(); ++i)
	{
		pParser->Timeout();										// put dtmf request to output queue
		auto dtmf_info_message = SendDTMF();
		auto ctx = dtmf_info_message.first.lock();

		if (dtmf_symbols[i] == ',' || dtmf_symbols[i] == 'p' || dtmf_symbols[i] == 'w'){
			ASSERT_FALSE(ctx);
			ASSERT_FALSE(dtmf_info_message.second);

			ASSERT_TRUE(internal_info->GetDTMFPauseTime() > std::chrono::steady_clock::duration());		// verify that pause is really present
			internal_info->SetDTMFPauseTime(std::chrono::steady_clock::duration());						// no need for waiting in test
		}
		else{
			ASSERT_TRUE(ctx);
			ASSERT_TRUE(dtmf_info_message.second);
			ASSERT_TRUE(ReceiveResponseOnDTMFRequest(dtmf_info_message)); // receive 200 ok
		}
	}
}

bool DTMFTest::ReceiveResponseOnDTMFRequest(ctx_msg_pair dtmf_info_message){
	auto info_rsp = std::make_shared<VS_SIPResponse>();
	auto ctx = dtmf_info_message.first.lock();
	const VS_SIPGetInfoImpl get_info(*ctx);
	VS_SIPUpdateInfoImpl update_info(*ctx);

	if (!info_rsp->MakeOnInfoResponseOK((VS_SIPRequest *)dtmf_info_message.second.get(), get_info, update_info)) return false;
	info_rsp->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_To, get_info); // must be inserted in transport layer
	if (!pParser->SetRecvMsg_SIP(info_rsp, addr_local)) return false; // 200 ok for dtmf INFO
	pParser->GetMsgForSend_SIP();								// remove 200 ok from queue

	return true;
}

ctx_msg_pair DTMFTest::SendDTMF(){
	auto dtmf_info_message = pParser->GetMsgForSend_SIP();	// SIP_INFO dtmf sent
	auto ctx = dtmf_info_message.first;
	if (!ctx || !dtmf_info_message.second)
		return {};

	const VS_SIPGetInfoImpl get_info(*ctx);

	dtmf_info_message.second->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, get_info);	// must be inserted in transport layer
	if (!dtmf_info_message.second->GetSIPMetaField() ||
		!dtmf_info_message.second->GetSIPMetaField()->iStartLine ||
		!dtmf_info_message.second->GetSIPMetaField()->iCSeq)
		return {};

	dtmf_info_message.second->GetSIPMetaField()->iStartLine->SetMessageType(MESSAGE_TYPE_REQUEST);
	dtmf_info_message.second->GetSIPMetaField()->iCSeq->SetType(TYPE_INFO);
	pParser->ProcessTransaction(ctx, dtmf_info_message.second);

	return dtmf_info_message;
}

std::shared_ptr<VS_SIPParserInfo> DTMFTest::SimulateIncomingInvite(){
	boost::shared_ptr<VS_SIPParserInfo> pCallInfo = makeInfo(dialog_id, from, to, display_name, strand);
	if (!pCallInfo) return nullptr;

	auto req = std::make_shared<VS_SIPRequest>();
	MakeInviteRequest(req, pCallInfo);
	pParser->SetRecvMsg_SIP(req, addr_remote);
	auto context_message_pair = pParser->GetMsgForSend_SIP();	// SIP_INVITE sent
	auto ctx = context_message_pair.first;
	if (!ctx || !context_message_pair.second) return nullptr;
	pParser->ProcessTransaction(ctx, req);

	ctx->SetByeTick(std::chrono::steady_clock::time_point());
	ctx->SetAnswered(std::chrono::steady_clock::now() - std::chrono::seconds(1));	// was answerd 1 second ago
	ctx->IsAnswered(true);
	pParser->GetMsgForSend_SIP();								// remove 200 ok from queue
	return ctx;					// return internal parser info
}

bool MakeInviteRequest(std::shared_ptr<VS_SIPRequest> &req, boost::shared_ptr<VS_SIPParserInfo> &pCallInfo){
	if(!req)	return false;

	const VS_SIPGetInfoImpl get_info(*pCallInfo);
	VS_SIPUpdateInfoImpl update_info(*pCallInfo);

	if (!req->MakeINVITE(get_info, update_info)) return false;
	// insert fields that must be inserted in transport layer
	pCallInfo->SetViaHost(DTMFTest::server);
	if(!req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, get_info))	return false;
	if (!req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_To, get_info))	return false;

	VS_SIPURI* pUri = req->GetSIPMetaField()->iTo->GetURI();
	boost::shared_ptr<VS_SIPURI> temp = makeownURI(DTMFTest::to_name, DTMFTest::to_host);
	if (!temp) return false;
	if (!pUri)	return false;
	*pUri = *temp;	// sip to sip is not allowed, so make own uri without 'sip' addition in SIPHeader_To

	if (!req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Contact, get_info)) return false;
	if (!req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Supported, get_info)) return false;
	if (!req->InsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_Origin, get_info)) return false;
	if (!req->InsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_Connection, get_info)) return false;

	return true;
}

boost::shared_ptr<VS_SIPURI> makeownURI(const std::string &to_name, const std::string &to_host){
	if (to_name.empty() || to_host.empty())
		return nullptr;
	boost::shared_ptr<VS_SIPURI> uri = boost::make_shared<VS_SIPURI>();
	uri->User(to_name);
	if (!uri->Name(to_name)) return nullptr;
	uri->Host(to_host);
	uri->URIType(SIPURI_SIP);
	uri->AngleBracket(false);
	uri->Transport(net::protocol::TCP);
	uri->Port(5060);
	uri->SetValid(true);

	return uri;
}

boost::shared_ptr<VS_SIPParserInfo> makeInfo(const std::string& dialog_id, const std::string& from, const std::string& to, const std::string& display_name, boost::asio::io_service::strand& strand){
	auto pCallInfo = boost::make_shared<VS_SIPParserInfo>("serverVendor");

	std::string to_addr(to);
	if (to_addr[0] == '#' && to_addr.find(':') != std::string::npos)
		to_addr.erase(0, to_addr.find(':') + 1);

	// settings for start line
	pCallInfo->IsRequest(true);
	pCallInfo->SetSIPRemoteTarget(to_addr);

	// settings for call_id
	pCallInfo->SIPDialogID(dialog_id);

	// settings for from
	pCallInfo->SetAliasMy(from);
	pCallInfo->SetTagMy({});
	pCallInfo->SetDisplayNameMy(display_name);

	// settings for to
	pCallInfo->SetAliasRemote(to_addr);
	pCallInfo->SetTagSip({});
	pCallInfo->SetDisplayNameSip(std::move(to_addr));

	boost::system::error_code ec;
	net::Endpoint addr(net::address::from_string("127.0.0.1", ec), 5060, net::protocol::TCP);

	pCallInfo->SetMyCsAddress(addr);
	pCallInfo->EnableSessionTimer();
	pCallInfo->GetTimerExtention().refreshPeriod = std::chrono::seconds(REFRESH_PERIOD_SEC);

	return pCallInfo;
}