#include "std-generic/cpplib/ThreadUtils.h"
#if defined(_WIN32) // Not ported yet

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <thread>
#include <chrono>
#include <atomic>

#include "H323RASParserTestBase.h"
#include "std/cpplib/ThreadUtils.h"

namespace h323_ras_parser_test {
	static const size_t BUFSZ = 64 * 1024; // temporary buffer size

#include "h225ras_term_to_srv.h"
#include "h225ras_srv_to_term.h"
#include "h225raw.h"

	static const char *from = "artem@artm001.trueconf.name";
	static const char *to = "192.168.90.14";
	static const char *h323id = "asrv";
	static const char *dialed_digits = "777";
	static const char *password = "123";

	class H323RASParserTest_TermToServer : public H323RASParserTestBase
	{
	public:
		H323RASParserTest_TermToServer()
		: H323RASParserTestBase(net::address_v4::from_string("192.168.41.156"), 1719, net::address_v4::from_string("192.168.90.14"), 1719)
		{}
	};

	TEST_F(H323RASParserTest_TermToServer, term_to_srv)
	{
		std::vector<uint8_t> buf(BUFSZ);
		std::size_t sz = buf.size();
		bool reg_result = false;
		VS_PerBuffer setup_msg(test_data1::term_to_server_setup, sizeof(test_data1::term_to_server_setup) * 8);
		VS_CsH323UserInformation ui;

		auto dialog_id = ras->NewDialogID(string_view{ to }, string_view{ from }, {});
		auto base_ctx = ras->GetParserContextBase(dialog_id, false);
		auto ctx = dynamic_cast<VS_H225RASParserInfo *>(base_ctx.get());

		auto reset_buf = [&]() -> void {
			sz = buf.size();
			memset(&buf[0], 0, sz);
		};

		ctx->SetH323ID(h323id);
		ctx->SetDialedDigits(dialed_digits);
		ctx->SetPassword(password);

		// set SeqNo accroding to the sniff data
		ctx->SetCurrentRequestSequenceNumber(20317);
		ctx->SetLastRRQSequenceNumber(20317);

		// generate and get RRQ
		ASSERT_TRUE(ras->DoRegister(dialog_id));
		ASSERT_TRUE(GetNextOutputMessage(&buf[0], sz)) << "No data to send (sz=" << sz << ")";
		// receive RCF
		ASSERT_TRUE(ras->SetRecvBuf(test_data1::peer1_0, sizeof(test_data1::peer1_0), e_RAS, terminal_addr.addr,
			terminal_addr.port, srv_addr.addr, srv_addr.port) > 0) << "Can't receive RCF" << std::endl;
		// check registration succes
		ASSERT_TRUE(ctx->IsRegistred()) << "Registration Failed" << std::endl;

		// Output queue should be empty.
		ASSERT_FALSE(GetNextOutputMessage(&buf[0], sz)) << "Output queue is not empty!";
		reset_buf();
		// Decode Setup message
		ASSERT_TRUE(DecodeUserInfo(setup_msg, ui)) << "Can't decode Setup message." << std::endl;
		VS_CsSetupUuie * setup = ui.h323UuPdu.h323MessageBody;
		ASSERT_NE(setup, nullptr);

		{
			ras->ARQForIncomingCall(setup, [&](bool res, bool timeout, net::address addr, net::port port) -> bool {
				reg_result = res;
				return res;
			});
		}

		// receive ACF
		ASSERT_TRUE(ras->SetRecvBuf(test_data1::peer1_1, sizeof(test_data1::peer1_1), e_RAS, terminal_addr.addr,
			terminal_addr.port, srv_addr.addr, srv_addr.port) > 0) << "Can't receive ACF" << std::endl;

		// check registration result
		ASSERT_TRUE(reg_result) << "Registration failed." << std::endl;
	}
	TEST_F(H323RASParserTest_TermToServer, reg_terminal)
	{
		using ::testing::_;
		using ::testing::SaveArg;

		std::vector<uint8_t> buf(BUFSZ);
		std::vector<std::string> aliases = {"#h323:asrv","#h323:\\e\\777" };
		std::vector<std::string> rez_aliases;
		std::size_t sz = buf.size();
		VS_CallConfig config;

		config.Address = { net::address_v4(1), net::port(1), net::protocol::UDP };
		config.HostName = "213.133.168.206";
		config.IsValid = true;
		config.Login = "test";
		config.Password = "test";

		std::string dialog_id = ras->NewDialogID(to, from, config);
		auto base_ctx = ras->GetParserContextBase(dialog_id, false);
		auto ctx = static_cast<VS_H225RASParserInfo *>(base_ctx.get());

		ctx->SetAliasMy("Login");
		ctx->SetH323ID(h323id);
		ctx->SetDialedDigits(dialed_digits);
		ctx->SetPassword(password);

		ctx->SetCurrentRequestSequenceNumber(20317);
		ctx->SetLastRRQSequenceNumber(20317);
		EXPECT_CALL(*conf_protocol, LoginUser(_, _, _, _, _, _, _, _)).WillOnce(SaveArg<7>(&rez_aliases));
		ASSERT_TRUE(ras->DoRegister(dialog_id));
		ASSERT_TRUE(ras->GetBufForSend(&buf[0], sz, e_RAS, terminal_addr.addr, terminal_addr.port, srv_addr.addr, srv_addr.port) > 0);
		ClearParserCtx();
		SetParserMode(VS_H225RASParser::PM_GATEKEEPER);
		ASSERT_TRUE(ras->SetRecvBuf(&buf[0], sz, e_RAS, terminal_addr.addr, terminal_addr.port, srv_addr.addr, srv_addr.port) > 0);

		ASSERT_STREQ(aliases[0].c_str(), rez_aliases[0].c_str());
		ASSERT_STREQ(aliases[1].c_str(), rez_aliases[1].c_str());
	}

	TEST_F(H323RASParserTest_TermToServer, reg_terminal_with_2_h323id)
	{
		using ::testing::_;
		using ::testing::SaveArg;

		std::vector<uint8_t> buf(BUFSZ);
		std::size_t sz = buf.size();
		std::vector<std::string> aliases = { "#h323:yealink.kamzal","#h323:yealink.kamzal" };
		std::vector<std::string> rez_aliases;

		ClearParserCtx();
		EXPECT_CALL(*conf_protocol, LoginUser(_, _, _, _, _, _, _, _)).WillOnce(SaveArg<7>(&rez_aliases));
		ASSERT_GT(ras->SetRecvBuf(h225raw::rrq_with_2_h323id, sizeof(h225raw::rrq_with_2_h323id), e_RAS, terminal_addr.
			addr, terminal_addr.port, srv_addr.addr, srv_addr.port),0);
		auto ctx = GetFirstCtx();
		SendRCF(ctx->GetDialogID());
		ASSERT_TRUE(ras->GetBufForSend(&buf[0], sz, e_RAS, terminal_addr.addr, terminal_addr.port, srv_addr.addr, srv_addr.port) > 0);
		VS_PerBuffer in_per_buff(&buf[0], sz * 8);
		VS_RasMessage mess;
		ASSERT_TRUE(mess.Decode(in_per_buff));
		ASSERT_EQ(mess.tag, mess.e_registrationConfirm);

		ASSERT_STREQ(aliases[0].c_str(), rez_aliases[0].c_str());
		ASSERT_STREQ(aliases[1].c_str(), rez_aliases[1].c_str());
	}

	class H323RASParserTest_ServerToTerm : public H323RASParserTestBase
	{
	public:
		H323RASParserTest_ServerToTerm()
			: H323RASParserTestBase(net::address_v4::from_string("192.168.41.156"), 1719, net::address_v4::from_string("192.168.90.14"), 1719)
		{
		}
		~H323RASParserTest_ServerToTerm()
		{
			if (resolve_thread.joinable())
			{
				resolve_thread.join();
			}
		}
	protected:
		std::thread resolve_thread;
	};

	TEST_F(H323RASParserTest_ServerToTerm, srv_to_term)
	{
		std::vector<uint8_t> buf(BUFSZ);
		std::size_t sz = buf.size();
		bool reg_result = false;
		VS_PerBuffer setup_msg(test_data2::srv_to_term, sizeof(test_data2::srv_to_term) * 8);
		VS_CsH323UserInformation ui;
		VS_CsSetupUuie* setup = nullptr;

		std::string dialog_id = ras->NewDialogID(string_view{ to }, string_view{ from }, {});
		auto base_ctx = ras->GetParserContextBase(dialog_id, false);
		auto ctx = dynamic_cast<VS_H225RASParserInfo *>(base_ctx.get());

		auto reset_buf = [&]() -> void {
			sz = buf.size();
			memset(&buf[0], 0, sz);
		};

		ctx->SetH323ID(h323id);
		ctx->SetDialedDigits(dialed_digits);
		ctx->SetPassword(password);

		// set SeqNo accroding to the sniff data
		ctx->SetCurrentRequestSequenceNumber(19210);
		ctx->SetLastRRQSequenceNumber(19210);

		// generate and get RRQ
		ASSERT_TRUE(ras->DoRegister(dialog_id));

		ASSERT_TRUE(GetNextOutputMessage(&buf[0], sz)) << "No data to send (sz=" << sz << ")";
		reset_buf();

		// receive RCF
		ASSERT_TRUE(ras->SetRecvBuf(test_data2::peer1_0, sizeof(test_data2::peer1_0), e_RAS, terminal_addr.addr,
			terminal_addr.port, srv_addr.addr, srv_addr.port) > 0) << "Can't receive RCF" << std::endl;
		// check registration succes
		ASSERT_TRUE(ctx->IsRegistred()) << "Registration Failed" << std::endl;

		ASSERT_FALSE(GetNextOutputMessage(&buf[0], sz)) << "Output queue is not empty!";
		reset_buf();

		// Decode Setup message
		//artem@artm001.trueconf.name
		//#h323:vc400

		// resolve address on external
		std::thread t([&]()
		{
			vs::SetThreadName("T:Resolve");
			net::address addr;
			net::port port;
			reg_result = ras->ResolveOnExternalGatekeeper("artem@artm001.trueconf.name", "#h323:vc400", addr, port);
		});


		vs::SleepFor(std::chrono::milliseconds(200));
		resolve_thread.swap(t);

		ASSERT_TRUE(GetNextOutputMessage(&buf[0], sz)) << "No data to send (sz=" << sz << ")";
		reset_buf();

		// receive ACF
		ASSERT_TRUE(ras->SetRecvBuf(test_data2::peer1_1, sizeof(test_data2::peer1_1), e_RAS, terminal_addr.addr,
			terminal_addr.port, srv_addr.addr, srv_addr.port) > 0) << "Can't receive ACF" << std::endl;

		// clean output buffer (send ARQ)
		/*ASSERT_TRUE(ras->GetBufForSend(&buf[0], sz, e_RAS, terminal_addr, srv_addr) > 0) << "No data to send (sz=" << sz << ")";
		reset_buf();*/
		resolve_thread.join();

		// check registration result
		ASSERT_TRUE(reg_result) << "Registration failed." << std::endl;
	}
};


#endif
