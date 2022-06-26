#if defined(_WIN32) // Not ported yet

#include "InstantMessageTest.h"
#include "../TrueGateway/sip/VS_SIPGetInfoImpl.h"
#include "../TrueGateway/sip/VS_SIPUpdateInfoImpl.h"
#include "../SIPParserLib/VS_SIPAuthGSS.h"

using testing::_;
using testing::SaveArg;

namespace
{
	constexpr std::chrono::seconds REFRESH_PERIOD(100);
}

boost::shared_ptr<VS_SIPParserInfo> SIPChat::MakeInfo(string_view dialogId, string_view from, string_view to, string_view displayName, boost::asio::io_service::strand & strand){
	boost::shared_ptr<VS_SIPParserInfo> pCallInfo = boost::make_shared<VS_SIPParserInfo>("serverVendor");

	if (!to.empty() && to.front() == '#')
	{
		const auto pos = to.find(':');
		if(pos != string_view::npos)
		{
			to = to.substr(pos + 1);
		}
	}

	// settings for start line
	pCallInfo->IsRequest(true);
	pCallInfo->SetMessageType(TYPE_MESSAGE);
	pCallInfo->SetSIPRemoteTarget(std::string(to));

	// settings for call_id
	pCallInfo->SIPDialogID(std::string(dialogId));

	// settings for from
	pCallInfo->SetAliasMy(std::string(from));
	pCallInfo->SetTagMy({});			// not sure what is it
	pCallInfo->SetDisplayNameMy(std::string(displayName));

	// settings for to
	pCallInfo->SetAliasRemote(std::string(to));
	pCallInfo->SetTagSip({});			// not sure what is it
	pCallInfo->SetDisplayNameSip(std::string(to));

	// settings for content type
	pCallInfo->SetContentType(CONTENTTYPE_TEXT_PLAIN);

	pCallInfo->SetMyCsAddress({ net::address::from_string("127.0.0.1"), 5060, net::protocol::TCP });
	pCallInfo->EnableSessionTimer();
	pCallInfo->GetTimerExtention().refreshPeriod = REFRESH_PERIOD;

	return pCallInfo;
}

std::shared_ptr<VS_SIPParserInfo> SIPChat::GetInternalCtx(VS_SIPParser *parser, string_view dialogId)
{
	return parser->GetParserContext(dialogId, false);
}

TEST_F(SIPChat, TestEncodeDecode){
	const int SIZE = 1000;
	char request_buff[SIZE];
	size_t sz(SIZE);
	VS_SIPBuffer message_buff;

	boost::shared_ptr<VS_SIPParserInfo> pCallInfo = SIPChat::MakeInfo(dialog_id, from, to, display_name, strand);

	const VS_SIPGetInfoImpl get_info(*pCallInfo);
	VS_SIPUpdateInfoImpl update_info(*pCallInfo);

	VS_SIPInstantMessage inst_message;
	ASSERT_EQ(TSIPErrorCodes::e_ok, inst_message.Init(get_info, message));
	ASSERT_EQ(TSIPErrorCodes::e_ok, inst_message.Encode(message_buff));
	ASSERT_EQ(TSIPErrorCodes::e_ok, inst_message.Decode(message_buff));

	boost::shared_ptr<VS_SIPRequest> req = boost::make_shared<VS_SIPRequest>();
	ASSERT_TRUE(req->MakeMESSAGE(get_info, update_info, message));

	// should be filled in transport layer, do it here for testing
	ASSERT_TRUE(req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_To, get_info));
	ASSERT_TRUE(req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, get_info));
	ASSERT_TRUE(req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, get_info));

	ASSERT_TRUE(req->GetSIPInstantMessage() != nullptr);

	ASSERT_EQ(TSIPErrorCodes::e_ok, req->Encode(request_buff, sz));

	boost::shared_ptr<VS_SIPRequest> new_req = boost::make_shared<VS_SIPRequest>();
	ASSERT_EQ(TSIPErrorCodes::e_ok, new_req->Decode(request_buff, sz));
	ASSERT_TRUE(new_req->GetSIPInstantMessage() != nullptr);

	std::string orig_message = message;
	std::string decoded_message = new_req->GetSIPInstantMessage()->GetMessageText();
	boost::algorithm::trim(orig_message);
	boost::algorithm::trim(decoded_message);
	ASSERT_EQ(0, orig_message.compare(decoded_message));
}

TEST_F(SIPChat, ChatFromSIP){
	const int SIZE = 1000;
	char request_buff[SIZE];
	size_t sz(SIZE);

	VS_CallConfig &&config = pParser->CreateCallConfig(epRemote, from, "serverVendor");
	boost::shared_ptr<VS_SIPParserInfo> pCallInfo = SIPChat::MakeInfo(dialog_id, from, to, display_name, strand);
	pCallInfo->SetConfig(config);

	ASSERT_TRUE(!!pCallInfo);

	auto req = std::make_shared<VS_SIPRequest>();
	ASSERT_TRUE(!!req);

	const VS_SIPGetInfoImpl get_info(*pCallInfo);
	VS_SIPUpdateInfoImpl update_info(*pCallInfo);

	ASSERT_TRUE(req->MakeMESSAGE(get_info, update_info, message));

	// should be filled in transport layer, do it here for testing
	ASSERT_TRUE(req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_To, get_info));
	ASSERT_TRUE(req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, get_info));
	ASSERT_TRUE(req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, get_info));

	ASSERT_EQ(TSIPErrorCodes::e_ok, req->Encode(request_buff, sz));

	EXPECT_CALL(*conf_mock, Chat(_, _, _, _, _))
		.Times(1),
		SaveArg<0>(&dialog_id),
		SaveArg<3>(&display_name),
		SaveArg<4>(&message);
	ASSERT_TRUE(pParser->SetRecvMsg_SIP(req, epRemote));

	auto msg_pair = pParser->GetMsgForSend_SIP();
	ASSERT_TRUE(!!msg_pair.second);
	ASSERT_EQ(200,msg_pair.second->GetResponseCode());
}

TEST_F(SIPChat, EmptyChatFromSIP){
	const char empty_message[] = "";

	VS_CallConfig &&config = pParser->CreateCallConfig(epRemote, from);
	boost::shared_ptr<VS_SIPParserInfo> pCallInfo = SIPChat::MakeInfo(dialog_id, from, to, display_name, strand);
	pCallInfo->SetConfig(config);

	ASSERT_TRUE(!!pCallInfo);

	auto req = std::make_shared<VS_SIPRequest>();
	ASSERT_TRUE(!!req);

	const VS_SIPGetInfoImpl get_info(*pCallInfo);
	VS_SIPUpdateInfoImpl update_info(*pCallInfo);

	ASSERT_TRUE(req->MakeMESSAGE(get_info, update_info, string_view{ empty_message, sizeof(empty_message) -1 } ));

	// should be filled in transport layer, do it here for testing
	ASSERT_TRUE(req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_To, get_info));
	ASSERT_TRUE(req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, get_info));
	ASSERT_TRUE(req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, get_info));

	ASSERT_TRUE(pParser->SetRecvMsg_SIP(req, epRemote));	// method must return true

	auto msg_pair = pParser->GetMsgForSend_SIP();
	ASSERT_FALSE(!!msg_pair.second);						// but message must not appear in queue
}

TEST_F(SIPChat, ChatToSIP){

	auto &&new_dialog = pParser->NewDialogID(to, {}, config);
	pParser->Chat(new_dialog, from, to, display_name, message);

	auto msg_pair = pParser->GetMsgForSend_SIP();
	ASSERT_TRUE(!!msg_pair.second);
	ASSERT_TRUE(!!msg_pair.second->GetSIPInstantMessage());

	std::string orig_message = message;
	std::string message_to_send = msg_pair.second->GetSIPInstantMessage()->GetMessageText();
	boost::algorithm::trim(orig_message);
	boost::algorithm::trim(message_to_send);
	ASSERT_EQ(0, orig_message.compare(message_to_send));
}

TEST_F(SIPChat, EmptyChatToSIP){
	const char empty_message[] = "";

	auto &&new_dialog = pParser->NewDialogID(to, {}, config);
	pParser->Chat(new_dialog, from, to, display_name, empty_message);

	auto msg_pair = pParser->GetMsgForSend_SIP();
	ASSERT_FALSE(!!msg_pair.second);
}

struct SIPParserTestImpl : public VS_SIPParser {

	std::shared_ptr<VS_SIPParserInfo> GetRegContextOnRemoteServer(const std::shared_ptr<VS_SIPParserInfo> &ctx,	const std::shared_ptr<VS_SIPRequest> &req = nullptr) override {
		auto pInfo = std::make_shared<VS_SIPParserInfo>("serverVendor");
		auto ntlm_scheme = std::make_shared<VS_SIPAuthGSS>(SIP_AUTHSCHEME_NTLM);
		pInfo->SetAuthScheme(ntlm_scheme);
		pInfo->SetUser(to_user);

		return pInfo;
	}

	std::shared_ptr<VS_SIPParserInfo> FindActiveMsgCtx(const std::shared_ptr<VS_SIPParserInfo>& ctx) override
	{
		auto &&newDialog = NewDialogID(ctx->GetUser(), {});
		auto pInfo = this->GetParserContext(newDialog);
		pInfo->SetAliasRemote(ctx->GetUser());
		pInfo->SetAliasMy("from");
		return std::static_pointer_cast<VS_SIPParserInfo>(pInfo);
	}

	std::string to_user;

protected:
	SIPParserTestImpl(boost::asio::io_service::strand& strand)
		: VS_SIPParser(strand, "serverVendor", nullptr)
	{
	}

	static void PostConstruct(std::shared_ptr<SIPParserTestImpl> &) { /*stub*/ }
};

TEST_F(SIPChat, ChatToYourself) {

	boost::asio::io_service::strand strand(g_asio_environment->IOService());
	auto parser(vs::MakeShared<SIPParserTestImpl>(strand));
	parser->SetConfCallBack(conf_mock);
	parser->SetCallConfigStorage(storage);
	parser->SetMyCsAddress(epLocal);

	for (auto& set_to_equal_from : {true,false}){
		if (set_to_equal_from) {
			parser->to_user = from; // 1. Set 'to' equal to 'from'
		}
		else {
			parser->to_user = to;	// 1.1. Do not set 'to' equal to 'from'
		}

		// 2. Send chat
		auto &&new_dialog = parser->NewDialogID(to, {}, config);
		parser->Chat(new_dialog, from, to, display_name, message);

		if (set_to_equal_from) {
			// 3. Make sure msg wasn't sended
			auto msg_pair = parser->GetMsgForSend_SIP();
			ASSERT_FALSE(!!msg_pair.second);
		}
		else {
			// 3. Make sure msg was sended
			auto msg_pair = parser->GetMsgForSend_SIP();
			ASSERT_TRUE(!!msg_pair.second);
		}
	}
}

TEST_F(SIPChat, ChatToGroupConf) {
	using ::testing::StartsWith;

	boost::asio::io_service::strand strand(g_asio_environment->IOService());
	auto parser(vs::MakeShared<SIPParserTestImpl>(strand));
	parser->SetConfCallBack(conf_mock);
	parser->SetCallConfigStorage(storage);
	parser->SetMyCsAddress(epLocal);
	parser->to_user = to;

	// 1. Message to group conference
	to = "0001@server.name#vcs";
	for (const auto use_display_name : {false,true})
	{
		auto &&new_dialog = parser->NewDialogID(to, {}, config);
	auto ctx = GetInternalCtx(parser.get(),new_dialog);
	ASSERT_NE(ctx, nullptr);
	ctx->SetGroupConf(true);

		std::string DN = use_display_name ?  std::string(display_name) : std::string{};
		parser->Chat(new_dialog, from, to, DN, message);

	auto msg_pair = parser->GetMsgForSend_SIP();
	ASSERT_NE(msg_pair.second, nullptr);

	auto pIM = msg_pair.second->GetSIPInstantMessage();
	ASSERT_NE(pIM, nullptr);

		if (!use_display_name) {
	// 2. Make sure we indicate who is owner of message. i.e. for 'hello' from "user1@server.com" we send 'user1:hello'
	std::string from_user(from, strchr(from, '@') - from); from_user += ':';
	auto chat_message = pIM->GetMessageText();
	ASSERT_THAT(chat_message.c_str(), StartsWith(from_user));
}
		else {
			auto chat_message = pIM->GetMessageText();
			ASSERT_THAT(chat_message.c_str(), StartsWith(display_name));
		}
	}
}

#endif
