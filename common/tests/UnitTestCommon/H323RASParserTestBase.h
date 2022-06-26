#include <gtest/gtest.h>

#include "VS_ConferenceProtocolMock.h"
#include "VS_ConferenceProtocolFake.h"

#include "TrueGateway/VS_GatewayStarter.h"
#include "tools/H323Gateway/Lib/VS_H323Lib.h"
#include "tools/H323Gateway/Lib/src/VS_Q931.h"
#include "tools/Server/vs_messageQueue.h"
#include "TrueGateway/h323/VS_H225RASParserInfo.h"
#include "TrueGateway/h323/VS_H225RASParser.h"
#include "TrueGateway/CallConfig/VS_Indentifier.h"
#include "TrueGateway/CallConfig/VS_CallConfigStorage.h"

#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/MakeShared.h"
#include <tests/common/ASIOEnvironment.h>

class H323RASParserTestBase : public ::testing::Test {
protected:
	H323RASParserTestBase(net::address srvAddr, net::port srvPort, net::address terminalAddr, net::port terminalPort)
		: srv_addr{ srvAddr, srvPort, net::protocol::UDP }
		, terminal_addr{ terminalAddr, terminalPort, net::protocol::UDP }
		, strand(g_asio_environment->IOService())
		, ras(vs::MakeShared<VS_H225RASParser>(strand, nullptr))
		, conf_protocol(std::make_shared<VS_ConferenceProtocolMock>())
	{
		auto call_config = vs::MakeShared<VS_CallConfigStorage>();
		call_config->RegisterProtocol(VS_Indentifier::GetCommonIndentifierChain(g_asio_environment->IOService(), "serverVendor"));
		ras->SetCallConfigStorage(call_config);
		ras->SetMyCsAddress(srv_addr);
		ras->SetPeerCSAddress({}, terminal_addr);
		ras->SetConfCallBack(conf_protocol);
		ras->UnregisterAll();

		conf_protocol_fake.parser = ras.get();
		conf_protocol->DelegateTo(&conf_protocol_fake);
	}

	void SetUp() override
	{
		//H323RASParserTestBase::SetUp();
		ras->ClearLastGK();
	}

	void TearDown() override
	{
		//H323RASParserTestBase::TearDown();
	}

public:
	static void SetUpTestCase()
	{
		if (!VS_GatewayStarter::IsVCS())
		{
			VS_GatewayStarter::SetIsVCS(true);
		}
	}
	void SetParserMode(VS_H225RASParser::ParserMode mode) //friend with H225RASParser for test reg_terminal
	{
		ras->m_parser_mode = mode;
	}
	void ClearParserCtx() noexcept //friend with H225RASParser for test reg_terminal
	{
		ras->m_ctx.clear();
		ras->m_is_shutdown = false;
	}
	void SendRCF(string_view dialogId)
	{
		ras->MakeAndSendRCF(dialogId);
	}
	std::shared_ptr<VS_H225RASParserInfo> GetFirstCtx() const
	{
		return ras->FindFirstParserInfo();
	}

	static bool DecodeUserInfo(VS_PerBuffer &setup_msg, VS_CsH323UserInformation &ui)
	{
		uint8_t tpkt_header[4] = { 0 };
		VS_Q931 Q931_header;
		uint8_t dn[82 + 1] = { 0 };
		std::size_t dn_sz = 0;
		uint8_t e164[50] = { 0 };

		// Skip TPKT header
		setup_msg.GetBits(tpkt_header, 4 * 8);

		if (!Q931_header.DecodeMHeader(setup_msg) ||
			!(Q931_header.messageType == VS_Q931::e_setupMsg) ||
			!VS_Q931::GetUserUserIE(setup_msg, dn, e164) ||
			!ui.Decode(setup_msg))
		{
			return false;
		}


		return true;
	}

	bool GetNextOutputMessage(unsigned char *buf, std::size_t &size)
	{
		auto queue = ras->GetOutputQueue(e_RAS);

		if (queue == nullptr)
		{
			size = 0;
			return false;
		}

		{
			uint32_t sz = 0;

			auto data = queue->GetChannelMessage(sz, e_RAS);
			if (!data)
			{
				size = 0;
				return false;
			}

			if (size < sz)
			{
				size = 0;
				return false;
			}
			else
			{
				size = sz;
				if (buf != nullptr)
					memcpy(buf, data.get(), sz);
			}
		}

		return true;
	}
protected:
	net::Endpoint srv_addr, terminal_addr;
	boost::asio::io_service::strand strand;
	std::shared_ptr<VS_H225RASParser> ras;
	std::shared_ptr<VS_ConferenceProtocolMock> conf_protocol;
	VS_ConferenceProtocolFake conf_protocol_fake;
};
