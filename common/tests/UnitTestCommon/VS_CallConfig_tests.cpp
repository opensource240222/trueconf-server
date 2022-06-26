#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include <boost/lexical_cast.hpp>
#include "RawSIPMessages.h"
#include <gmock/gmock.h>
#include "h225raw.h"
#include <tests/common/TestHelpers.h>
#include "tools/H323Gateway/Lib/src/VS_RasMessages.h"
#include "tests/common/ASIOEnvironment.h"
#include "tests/common/GMockOverride.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/ThreadUtils.h"

#include <boost/make_shared.hpp>
#include <gtest/gtest.h>

#include <fstream>
#include <string>
#include <streambuf>

#include "TrueGateway/CallConfig/VS_CallConfig.h"
#include "TrueGateway/CallConfig/VS_CallConfigCorrector.h"
#include "TrueGateway/VS_SignalConnectionsMgr.h"
#include "TrueGateway/CallConfig/VS_IndentifierH225RAS.h"
#include "TrueGateway/VS_TransportConnection.h"
#include "TrueGateway/sip/VS_SIPCallResolver.h"
#include "../SIPParserBase/VS_Const.h"

#include "TrueGateway/CallConfig/VS_CallConfigStorage.h"
#include "VS_IdentifierSIPTestImpl.h"
#include "SIPParserTestBase.h"
#include "TransceiversPoolFake.h"
#include "std/cpplib/MakeShared.h"

namespace call_config_tests {

	class ValueCheckerInterface {
	public:
		virtual void CheckBool(const char *name, const bool &val) = 0;
		virtual void CheckInteger(const char *name, const int32_t &val) = 0;
		virtual void CheckString(const char *name, const std::string &val, bool mustBeMerged = true) = 0;
		virtual void CheckProtocol(const char *name, const net::protocol &val) = 0;
	};

	class TestValueChecker : public ValueCheckerInterface
	{
	public:
		void CheckBool(const char *name, const bool &val)
		{
			ASSERT_EQ(val, true) << "Test for boolean value " << name << " failed." << std::endl;
		}

		void CheckInteger(const char *name, const int32_t &val)
		{
			ASSERT_EQ(val, 0xCAFE) << "Test for integer value " << name << " failed." << std::endl;
		}

		void CheckString(const char *name, const std::string &val, bool mustBeMerged = true)
		{
			if (!mustBeMerged) return;
			ASSERT_EQ(val, "TestString") << "Test for string value " << name << " failed." << std::endl;
		}

		void CheckProtocol(const char *name, const net::protocol &val)
		{
			ASSERT_EQ(val, net::protocol::UDP) << "Test for protocol value " << name << " failed." << std::endl;
		}
	};

	void CheckValues(const VS_CallConfig &cfg, ValueCheckerInterface &checker)
	{
		//VS_CallConfig::CallConfigValue val;
		bool val_boolean;
		std::string val_str;
		int32_t val_integer;
		net::protocol val_conntype;

		val_str = cfg.HostName;
		checker.CheckString("Hostname", val_str);

		val_str = cfg.Login;
		checker.CheckString("Login", val_str);

		val_str = cfg.Password;
		checker.CheckString("Password", val_str);

		val_str = cfg.TelephonePrefixReplace;
		checker.CheckString("TelephonePrefixReplace", val_str);

		val_str = cfg.sip.FromUser;
		checker.CheckString("fromuser", val_str);

		val_str = cfg.sip.FromDomain;
		checker.CheckString("fromdomain", val_str);

		val_conntype = cfg.Address.protocol;
		checker.CheckProtocol("Protocol", val_conntype);

		ASSERT_FALSE(cfg.ConnectionTypeSeq.empty());
		ASSERT_EQ(cfg.ConnectionTypeSeq[0], net::protocol::UDP);

		val_str = cfg.h323.DialedDigit;
		checker.CheckString("DialedDigit", val_str);

		ASSERT_TRUE(cfg.h323.EnabledH235.is_initialized());
		val_boolean = cfg.h323.EnabledH235.get();
		checker.CheckBool("H235 Enabled", val_boolean);

		val_str = cfg.Codecs;
		checker.CheckString("Enabled Codecs", val_str);

		ASSERT_TRUE(cfg.sip.DefaultBFCPProto.is_initialized());
		val_conntype = cfg.sip.DefaultBFCPProto.get();
		checker.CheckProtocol("BFCP Protocol", val_conntype);

		ASSERT_TRUE(cfg.h323.ConventionalSirenTCE.is_initialized());
		val_boolean = cfg.h323.ConventionalSirenTCE.get();
		checker.CheckBool("Conventional Siren TCE", val_boolean);

		ASSERT_TRUE(cfg.h323.EnableH263plus2.is_initialized());
		val_boolean = cfg.h323.EnableH263plus2.get();
		checker.CheckBool("Enable H263plus2", val_boolean);

		val_integer = cfg.Address.port;
		checker.CheckInteger("Port", val_integer);

		ASSERT_TRUE(cfg.sip.RegistrationBehavior.is_initialized());
		val_integer = cfg.sip.RegistrationBehavior.get();
		checker.CheckInteger("RegisterStrategy", val_integer);

		val_boolean = cfg.UseAsTel;
		checker.CheckBool("IsVoIPServer", val_boolean);

		ASSERT_TRUE(cfg.sip.IsKeepAliveSendEnabled.is_initialized());
		val_boolean = cfg.sip.IsKeepAliveSendEnabled.get();
		checker.CheckBool("SendKeepAlive", val_boolean);

		ASSERT_TRUE(cfg.sip.BFCPEnabled.is_initialized());
		val_boolean = cfg.sip.BFCPEnabled.get();
		checker.CheckBool("BFCP Enabled", val_boolean);

		ASSERT_TRUE(cfg.sip.BFCPRoles.is_initialized());
		val_integer = cfg.sip.BFCPRoles.get();
		checker.CheckInteger("BFCP Roles", val_integer);

		ASSERT_TRUE(cfg.Bandwidth.is_initialized());
		val_integer = cfg.Bandwidth.get();
		checker.CheckInteger("Gateway Bandwidth", val_integer);

		ASSERT_TRUE(cfg.sip.SessionTimers.Enabled.is_initialized());
		val_boolean = cfg.sip.SessionTimers.Enabled.get();
		checker.CheckBool("Session Timers Enabled", val_boolean);

		ASSERT_TRUE(cfg.sip.CompactHeader.is_initialized());
		val_boolean = cfg.sip.CompactHeader.get();
		checker.CheckBool("CompactHeader", val_boolean);

		ASSERT_TRUE(cfg.sip.UseSingleBestCodec.is_initialized());
		val_boolean = cfg.sip.UseSingleBestCodec.get();
		checker.CheckBool("UseSingleBestCodec", val_boolean);

		ASSERT_TRUE(cfg.sip.NoRtpmapForAudioStaticPT.is_initialized());
		val_boolean = cfg.sip.NoRtpmapForAudioStaticPT.get();
		checker.CheckBool("NoRtpmapForAudioStaticPayload", val_boolean);

		ASSERT_TRUE(cfg.sip.NoRtpmapForVideoStaticPT.is_initialized());
		val_boolean = cfg.sip.NoRtpmapForVideoStaticPT.get();
		checker.CheckBool("NoRtpmapForVideoStaticPayload", val_boolean);

		ASSERT_TRUE(cfg.h323.H239Enabled.is_initialized());
		val_boolean = cfg.h323.H239Enabled.get();
		checker.CheckBool("H239 Enabled", val_boolean);

		ASSERT_TRUE(cfg.codecParams.h264_payload_type.is_initialized());
		val_integer = cfg.codecParams.h264_payload_type.get();
		checker.CheckInteger("H264 Payload Type", val_integer);

		ASSERT_TRUE(cfg.codecParams.h264_snd_preferred_width.is_initialized());
		val_integer = cfg.codecParams.h264_snd_preferred_width.get();
		checker.CheckInteger("H264 To Terminal Video Width", val_integer);

		ASSERT_TRUE(cfg.codecParams.h264_snd_preferred_height.is_initialized());
		val_integer = cfg.codecParams.h264_snd_preferred_height.get();
		checker.CheckInteger("H264 To Terminal Video Height", val_integer);

		ASSERT_TRUE(cfg.codecParams.h264_force_cif_mixer.is_initialized());
		val_boolean = cfg.codecParams.h264_force_cif_mixer.get();
		checker.CheckBool("H264 Mixer Mode CIF", val_boolean);

		ASSERT_TRUE(cfg.codecParams.siren_swap_bytes.is_initialized());
		val_boolean = cfg.codecParams.siren_swap_bytes.get();
		checker.CheckBool("Siren Swap Bytes", val_boolean);

		val_str = cfg.sip.AuthName;
		checker.CheckString("Authorization Name", val_str);

		ASSERT_TRUE(cfg.codecParams.gconf_to_term_height.is_initialized());
		val_integer = cfg.codecParams.gconf_to_term_height.get();
		checker.CheckInteger("GConf To Term Video Height", val_integer);

		ASSERT_TRUE(cfg.codecParams.gconf_to_term_width.is_initialized());
		val_integer = cfg.codecParams.gconf_to_term_width.get();
		checker.CheckInteger("GConf To Term Video Width", val_integer);

		val_str = cfg.sip.ContactDomain;
		checker.CheckString("contactdomain", val_str);

		ASSERT_TRUE(cfg.sip.RequestOPTIONS.is_initialized());
		val_boolean = cfg.sip.RequestOPTIONS.get();
		checker.CheckBool("Request OPTIONS", val_boolean);

		ASSERT_TRUE(cfg.sip.ICEEnabled.is_initialized());
		val_boolean = cfg.sip.ICEEnabled.get();
		checker.CheckBool("ICE Enabled", val_boolean);

		ASSERT_TRUE(cfg.sip.SRTPEnabled.is_initialized());
		val_boolean = cfg.sip.SRTPEnabled.get();
		checker.CheckBool("SRTP Enabled", val_boolean);

		val_str = cfg.sip.AuthDomain;
		checker.CheckString("Authorization domain", val_str);

		val_str = cfg.DefaultProxyConfigurationName;
		checker.CheckString("Default Proxy", val_str, false);

		ASSERT_TRUE(cfg.isAuthorized.is_initialized());
		val_boolean = cfg.isAuthorized.get();
		checker.CheckBool("Authorized", val_boolean);

		val_str = cfg.sip.OutboundProxy;
		checker.CheckString("Outbound Proxy", val_str);
	}

	class TestValuesReader :
		public VS_CallConfig::ValueReaderInterface
	{
	public:
		TestValuesReader(bool flag)
			: m_flag(flag)
		{}

		bool ReadBool(const char *name, bool &val)
		{
			if (CheckFlag())
			{
				val = true;
				return true;
			}
			return false;
		}

		bool ReadInteger(const char *name, int32_t &val)
		{
			if (CheckFlag())
			{
				val = 0xCAFE;
				return true;
			}

			return false;
		}

		bool ReadString(const char *name, std::string &val, bool canBeEmpty = false)
		{
			if (CheckFlag())
			{
				val = "TestString";
				return true;
			}
			return false;
		}

		bool ReadProtocolSeq(const char *name, std::vector<net::protocol> &seq)
		{
			if (CheckFlag())
			{
				seq.push_back(net::protocol::UDP);
				return true;
			}

			return false;
		}
	private:
		bool CheckFlag(void)
		{
			bool res = m_flag;
			m_flag = !m_flag;
			return res;
		}
	private:
		bool m_flag;
	};


	class CallConfigMerging : public ::testing::Test
	{
	protected:
		CallConfigMerging()
		{}

		virtual void SetUp()
		{}

		virtual void TearDown()
		{}
	};

	TEST_F(CallConfigMerging, MergingTest)
	{
		VS_CallConfig res;
		VS_CallConfig cfg2;

		// we should set the same signalling protocol
		cfg2.SignalingProtocol = res.SignalingProtocol = VS_CallConfig::H323;

		// Fill structures with some well known initial parameters
		cfg2.Codecs = res.Codecs = "TestString";
		cfg2.sip.SessionTimers.Enabled = res.sip.SessionTimers.Enabled = true;
		cfg2.sip.BFCPRoles = 0xCAFE & (SDP_FLOORCTRL_ROLE_C_ONLY | SDP_FLOORCTRL_ROLE_S_ONLY | SDP_FLOORCTRL_ROLE_C_S);
		cfg2.sip.DefaultBFCPProto = net::protocol::UDP;

		auto &&call_manager_res = create_call_config_manager(res);
		// The half of the prameters in each CallConfig structure will be set to the some well known values.
		// Then we will merge two structures and check every parameter.
		{
			TestValuesReader reader(false);
			call_manager_res.LoadValues(reader);
		}

		{
			auto &&call_manager_cfg2 = create_call_config_manager(cfg2);
			TestValuesReader reader(true);
			call_manager_cfg2.LoadValues(reader);
		}

		call_manager_res.MergeWith(cfg2);
		// handle some "special parameters"
		//ASSERT_EQ(res.sip.BFCPRoles.get(), 0xCAFE & (SDP_FLOORCTRL_ROLE_C_ONLY | SDP_FLOORCTRL_ROLE_S_ONLY | SDP_FLOORCTRL_ROLE_C_S));
		EXPECT_TRUE(res.sip.DefaultBFCPProto.is_initialized());
		EXPECT_EQ(res.sip.DefaultBFCPProto.get(), cfg2.sip.DefaultBFCPProto.get());

		EXPECT_TRUE(res.sip.BFCPRoles.is_initialized());
		EXPECT_EQ(res.sip.BFCPRoles.get(), cfg2.sip.BFCPRoles.get());
		EXPECT_FALSE(res.sip.RegistrationBehavior.is_initialized());

		res.sip.BFCPRoles = 0xCAFE;
		res.Address.port = 0xCAFE;
		res.Address.protocol = net::protocol::UDP;
		res.Password = "TestString";
		res.sip.RegistrationBehavior = (VS_CallConfig::eRegistrationBehavior)0xCAFE;

		TestValueChecker checker;
		CheckValues(res, checker);
	}

	class CallConfigCorrection : public ::testing::Test
	{
	protected:
		CallConfigCorrection()
			: m_corrector(VS_CallConfigCorrector::GetInstance())
		{}

		virtual void SetUp()
		{}

		virtual void TearDown()
		{}

		VS_CallConfigCorrector &m_corrector;
	};

	TEST_F(CallConfigCorrection, BasicTest)
	{
		VS_CallConfig cfg;
		ASSERT_FALSE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "dummy terminal")); // corrector is not initialized
		ASSERT_FALSE(m_corrector.UpdateCorrectorData("!!B@dScr1pt")); // syntactically invalid chunk
		ASSERT_FALSE(VS_CallConfigCorrector::IsValidData("!!B@dScr1pt"));
		ASSERT_TRUE(m_corrector.UpdateCorrectorData("")); // empty Lua chunk

		ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "dummy terminal"));
		ASSERT_TRUE(m_corrector.UpdateCorrectorData("function get_call_config_data(protocol, terminal_id) error(\"Simple error message.\"); end"));
		ASSERT_FALSE(VS_CallConfigCorrector::IsValidData("!!B@dScr1pt"));

		// check lua error recovery
		ASSERT_FALSE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "dummy terminal"));
		ASSERT_TRUE(m_corrector.UpdateCorrectorData("_G[\"get_call_config_data\"] = nil;")); // delete entry function
		ASSERT_FALSE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "dummy terminal"));

		// check all parameter types handling
		auto chunk =
			"function get_call_config_data(proto, terminal_id)\n"
			"  local t = {}\n"
			"  t[\"Enabled Codecs\"] = \"Codecs\"\n" // string parameter
			"  t[\"BFCP Protocol\"] = CONNECTIONTYPE_UDP\n" // protocol paramater
			"  t[\"Enable H263plus2\"] = true\n" // boolean prameter
			"  t.Port = 8980\n" // numeric parameter
			"  t[\"Gateway Bandwidth\"] = 9000.0\n" // numeric (float) parameter
			"  return t\n"
			"end\n";

		ASSERT_TRUE(m_corrector.UpdateCorrectorData(chunk));
		ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "dummy terminal"));

		// verify parameters
		ASSERT_TRUE(cfg.Codecs == "Codecs");
		ASSERT_TRUE(cfg.sip.DefaultBFCPProto.is_initialized() && cfg.sip.DefaultBFCPProto.get() == net::protocol::UDP);
		ASSERT_TRUE(cfg.h323.EnableH263plus2.is_initialized() && cfg.h323.EnableH263plus2.get() == true);
		ASSERT_TRUE(cfg.Address.port == 8980);
		ASSERT_TRUE(cfg.Bandwidth.is_initialized() && cfg.Bandwidth.get() == 9000);

		// check protocol parameters type handling
		{
			VS_CallConfig cfg2;
			auto chunk_wrong_proto =
				"function get_call_config_data(proto, terminal_id)\n"
				"  local t = {}\n"
				"  t[\"BFCP Protocol\"] = 99999\n" // wrong protocol paramater
				"  return t\n"
				"end\n";
			ASSERT_TRUE(m_corrector.UpdateCorrectorData(chunk_wrong_proto));
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg2, VS_CallConfig::H323, "dummy terminal"));
			ASSERT_FALSE(cfg2.sip.DefaultBFCPProto.is_initialized());
		}

		{
			VS_CallConfig cfg2;
			auto chunk_proto_seq =
				"function get_call_config_data(proto, terminal_id)\n"
				"  local t = {}\n"
				"  t[\"Protocol\"] = \"TLS,TCP,UDP\"\n" // wrong protocol paramater
				"  return t\n"
				"end\n";
			ASSERT_TRUE(m_corrector.UpdateCorrectorData(chunk_proto_seq));
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg2, VS_CallConfig::H323, "dummy terminal"));
			ASSERT_EQ(cfg2.Address.protocol, net::protocol::TLS);

			ASSERT_EQ(cfg2.ConnectionTypeSeq.size(), 3);
			ASSERT_EQ(cfg2.ConnectionTypeSeq[0], net::protocol::TLS);
			ASSERT_EQ(cfg2.ConnectionTypeSeq[1], net::protocol::TCP);
			ASSERT_EQ(cfg2.ConnectionTypeSeq[2], net::protocol::UDP);
		}

		// in the following test some of the values are non unique
		{
			VS_CallConfig cfg2;
			auto chunk_proto_seq =
				"function get_call_config_data(proto, terminal_id)\n"
				"  local t = {}\n"
				"  t[\"Protocol\"] = \"TLS, TLS, TCP, TCP ,UDP, TCP, TLS\"\n" // wrong protocol paramater
				"  return t\n"
				"end\n";
			ASSERT_TRUE(m_corrector.UpdateCorrectorData(chunk_proto_seq));
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg2, VS_CallConfig::H323, "dummy terminal"));
			ASSERT_EQ(cfg2.Address.protocol, net::protocol::TLS);

			ASSERT_EQ(cfg2.ConnectionTypeSeq.size(), 3);
			ASSERT_EQ(cfg2.ConnectionTypeSeq[0], net::protocol::TLS);
			ASSERT_EQ(cfg2.ConnectionTypeSeq[1], net::protocol::TCP);
			ASSERT_EQ(cfg2.ConnectionTypeSeq[2], net::protocol::UDP);
		}
	}

	class LuaChunkGenerator :
		public VS_CallConfig::ValueReaderInterface
	{
	public:
		LuaChunkGenerator()
			: m_generated(false)
		{
			m_chunk = "function get_call_config_data(proto, terminal_id)\n";
			m_chunk += "  local t = {}\n";
		}

		bool ReadBool(const char *name, bool &val)
		{
			if (m_generated)
				return false;

			m_chunk += "  t[\"" + std::string(name) + "\"] = true\n";
			return false;
		}

		bool ReadInteger(const char *name, int32_t &val)
		{
			if (m_generated)
				return false;

			m_chunk += "  t[\"" + std::string(name) + "\"] = 0xCAFE\n";
			return false;
		}

		bool ReadString(const char *name, std::string &val, bool canBeEmpty = false)
		{
			if (m_generated)
				return false;

			m_chunk += "  t[\"" + std::string(name) + "\"] = \"TestString\"\n";
			return false;
		}

		bool ReadProtocolSeq(const char *name, std::vector<net::protocol> &seq)
		{
			if (m_generated)
				return false;

			m_chunk += "  t[\"" + std::string(name) + "\"] = CONNECTIONTYPE_UDP\n";
			return false;
		}

		const std::string &GetLuaChunk(void)
		{
			if (!m_generated)
			{
				m_generated = true;
			}
			else
			{
				return m_chunk;
			}

			m_chunk += "  return t\n";
			m_chunk += "end\n";

			return m_chunk;
		}

	private:
		std::string m_chunk;
		bool m_generated;
	};

	TEST_F(CallConfigCorrection, LuaValuesRetrieval)
	{
		VS_CallConfig cfg;
		// generate test chunk which initialized all known parameters to the well known parameters
		LuaChunkGenerator chunk_generator;
		auto &&call_manager = create_call_config_manager(cfg);
		call_manager.LoadValues(chunk_generator);
		// generated chunk
		std::cout << chunk_generator.GetLuaChunk() << std::endl;

		ASSERT_TRUE(m_corrector.UpdateCorrectorData(chunk_generator.GetLuaChunk().c_str())) << "Chunk loading failed!" << std::endl; // load chunk
		ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "")) << "Parameters correction failed!" << std::endl;

		// handle some "special" paramters
		ASSERT_TRUE(cfg.sip.SessionTimers.Enabled.is_initialized() && cfg.sip.SessionTimers.Enabled.get() == true);
		ASSERT_TRUE(cfg.sip.BFCPRoles.is_initialized());
		ASSERT_EQ(cfg.sip.BFCPRoles.get(), 0xCAFE & (SDP_FLOORCTRL_ROLE_C_ONLY | SDP_FLOORCTRL_ROLE_S_ONLY | SDP_FLOORCTRL_ROLE_C_S));
		cfg.sip.BFCPRoles = 0xCAFE;
		cfg.sip.RegistrationBehavior = (VS_CallConfig::eRegistrationBehavior)0xCAFE;

		// validate parameters
		TestValueChecker checker;
		CheckValues(cfg, checker);
	}

	TEST_F(CallConfigCorrection, DumbTerminals)
	{
		// read dumb terminals database
		const auto exe_dir = VS_GetExecutableDirectory();
		ASSERT_FALSE(exe_dir.empty());

		std::ifstream ifs;
		auto try_open = [&](const std::string& path) {
#if defined(_WIN32)
			const auto w_path = vs::UTF8ToWideCharConvert(path);
			if (w_path.empty())
				return false;
			ifs.open(w_path, std::ios::binary | std::ios::in);
#else
			ifs.open(path, std::ios::binary | std::ios::in);
#endif
			return ifs.good();
		};

		if (!try_open(exe_dir + "../tests/UnitTestCommon/Lua/terminals.lua")
		 && !try_open(exe_dir + "../common/tests/UnitTestCommon/Lua/terminals.lua")
		 && !try_open(exe_dir + "terminals.lua")
		)
		{
			FAIL() << "Can't open 'terminals.lua'! Did you move this executable?";
		}

		const auto str = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
		ASSERT_FALSE(str.empty()) << "Can't read Lua script!";
		ifs.close();

		ASSERT_TRUE(m_corrector.UpdateCorrectorData(str.c_str())) << "Can't read Lua script!" << std::endl;

		// SIP: Polycom VVX-1500
		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::SIP, "PolycomVVX-VVX_1500-UA/3.2.2.0481"));
			ASSERT_TRUE(cfg.codecParams.h264_snd_preferred_height.is_initialized());
			ASSERT_TRUE(cfg.codecParams.h264_snd_preferred_width.is_initialized());
			ASSERT_EQ(cfg.codecParams.h264_snd_preferred_height.get(), 288);
			ASSERT_EQ(cfg.codecParams.h264_snd_preferred_width.get(), 352);
		}

		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::SIP, "PolycomVVX-VVX_1500-UA/3.3.3.0072"));
			ASSERT_TRUE(cfg.codecParams.h264_snd_preferred_height.is_initialized());
			ASSERT_TRUE(cfg.codecParams.h264_snd_preferred_width.is_initialized());
			ASSERT_EQ(cfg.codecParams.h264_snd_preferred_height.get(), 288);
			ASSERT_EQ(cfg.codecParams.h264_snd_preferred_width.get(), 352);
		}

		// H323: Polycom VVX-1500
		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "VVX 1500::3.3.3.0072"));
			ASSERT_TRUE(cfg.codecParams.h264_snd_preferred_height.is_initialized());
			ASSERT_TRUE(cfg.codecParams.h264_snd_preferred_width.is_initialized());
			ASSERT_EQ(cfg.codecParams.h264_snd_preferred_height.get(), 288);
			ASSERT_EQ(cfg.codecParams.h264_snd_preferred_width.get(), 352);
		}

		// Grandstream GVC3200
		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::SIP, "Grandstream GVC3200 1.0.1.58"));
			ASSERT_TRUE(cfg.sip.DefaultBFCPProto == net::protocol::UDP);
		}
		{
			VS_CallConfig cfg;
			EXPECT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::SIP, "Grandstream GVC3200 1.0.3.39"));
			EXPECT_TRUE(cfg.sip.DefaultBFCPProto == net::protocol::UDP);
		}

	    // H323: LifeSize Express 220
		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "LifeSize Express 220::4.7.11.4"));
			ASSERT_TRUE(cfg.codecParams.siren_swap_bytes.get_value_or(false));
		}

		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "LifeSize Express 220::4.11.13.1"));
			ASSERT_TRUE(cfg.codecParams.siren_swap_bytes.get_value_or(false));
		}

		// H323: LifeSize Room 220 4.10.0.49
		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "LifeSize Room 220::4.10.0.49"));
			ASSERT_TRUE(cfg.codecParams.siren_swap_bytes.get_value_or(false));
		}

		// H323: LifeSize Team 220 4.12.3.4
		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "LifeSize Team 220::4.12.3.4"));
			ASSERT_TRUE(cfg.codecParams.siren_swap_bytes.get_value_or(false));
		}

		// H323: LifeSize Passport 2 4.11.14.2
		{
			VS_CallConfig cfg;
			EXPECT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "LifeSize Passport 2::4.11.14.2"));
			EXPECT_TRUE(cfg.codecParams.siren_swap_bytes.get_value_or(false));
		}

		// SIP: LifeSize Express 220
		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::SIP, "LifeSize Express_220/LS_EX2_4.7.11 (4)"));
			ASSERT_TRUE(cfg.Codecs.size() > 0 && cfg.Codecs.find("G729a") == std::string::npos);
		}

		// SIP: Huawei-MC850/V100R001C02B120
		{
			VS_CallConfig cfg;
			EXPECT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::SIP, "Huawei-MC850/V100R001C02B120"));
			EXPECT_FALSE(cfg.Codecs.empty());
			EXPECT_EQ(cfg.Codecs.find("SIREN14_32"), std::string::npos);
			EXPECT_EQ(cfg.Codecs.find("SIREN14_48"), std::string::npos);
			EXPECT_EQ(cfg.Codecs.find("SIREN14_24"), std::string::npos);
		}

		// Sony PCS-TL30
		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "SONY PCS-101 00::PCS-G70 2009-11-30 17:00 Ver 02.70      \r\n"));
			ASSERT_TRUE(cfg.codecParams.h264_snd_preferred_height.is_initialized());
			ASSERT_TRUE(cfg.codecParams.h264_snd_preferred_width.is_initialized());
			ASSERT_EQ(cfg.codecParams.h264_snd_preferred_height.get(), 288);
			ASSERT_EQ(cfg.codecParams.h264_snd_preferred_width.get(), 352);
		}
		{
			VS_CallConfig cfg;
			EXPECT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "SONY PCS-101 00::PCS-G70 2005-06-10 09:20 Ver 02.10      \r\n"));
			ASSERT_TRUE(cfg.codecParams.h264_snd_preferred_height.is_initialized());
			ASSERT_TRUE(cfg.codecParams.h264_snd_preferred_width.is_initialized());
			ASSERT_EQ(cfg.codecParams.h264_snd_preferred_height.get(), 288);
			ASSERT_EQ(cfg.codecParams.h264_snd_preferred_width.get(), 352);
		}

		// SIP: Linphone
		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::SIP, "Linphone/3.9.1 (belle-sip/1.4.2)"));
			ASSERT_TRUE(cfg.sip.BFCPEnabled.is_initialized());
			ASSERT_FALSE(cfg.sip.BFCPEnabled.get());
		}

		// SIP: Panasonic KX-VC1300_HDVC-MPCS 4.52 (Profile=5)
		{
			VS_CallConfig cfg;
			EXPECT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::SIP, "KX-VC1300_HDVC-MPCS 4.52 (Profile=5)"));
			EXPECT_EQ(cfg.Codecs.find("XH264UC"), std::string::npos);
			EXPECT_TRUE(cfg.sip.DefaultBFCPProto == net::protocol::UDP);

		}

		// H323: LifeSize Bridge 2200::LS_BR1_2.1.0 (14)
		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::H323, "LifeSize Bridge 2200::LS_BR1_2.1.0 (14)"));
			ASSERT_TRUE(cfg.codecParams.siren_swap_bytes.is_initialized());
			ASSERT_TRUE(cfg.codecParams.siren_swap_bytes.get());
		}

		// SIP: Yealink VCDesktop
		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::SIP, "Yealink VCDesktop 1.28.0.11"));
			ASSERT_TRUE(cfg.codecParams.siren_swap_bytes.is_initialized());
			ASSERT_TRUE(cfg.codecParams.siren_swap_bytes.get());
		}

		// SIP: Ericsson-LG Enterprise iPECS-eMG eMG800
		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::SIP, "Ericsson-LG Enterprise iPECS-eMG eMG800 3.1.10"));
			ASSERT_TRUE(cfg.codecParams.h264_payload_type.is_initialized());
			ASSERT_TRUE(cfg.codecParams.h264_payload_type.get() == 98);
		}

		// SIP: Lync/Skype for business
		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::SIP, "UCCAPI/16.0.4417.1000 OC/16.0.4417.1000 (Skype for Business)"));
			ASSERT_TRUE(cfg.sip.ICEEnabled.is_initialized());
			ASSERT_TRUE(cfg.sip.ICEEnabled.get());
			ASSERT_TRUE(cfg.sip.SRTPEnabled.is_initialized());
			ASSERT_TRUE(cfg.sip.SRTPEnabled.get());
			ASSERT_TRUE(cfg.sip.BFCPEnabled.is_initialized());
			ASSERT_FALSE(cfg.sip.BFCPEnabled.get());
			ASSERT_TRUE(cfg.H224Enabled.is_initialized());
			ASSERT_FALSE(cfg.H224Enabled.get());
			ASSERT_EQ(cfg.Address.protocol, net::protocol::TLS);
		}

		{
			VS_CallConfig cfg;
			ASSERT_TRUE(m_corrector.CorrectCallConfig(cfg, VS_CallConfig::SIP, "RTC/6.0"));
			ASSERT_TRUE(cfg.sip.ICEEnabled.is_initialized());
			ASSERT_TRUE(cfg.sip.ICEEnabled.get());
			ASSERT_TRUE(cfg.sip.SRTPEnabled.is_initialized());
			ASSERT_TRUE(cfg.sip.SRTPEnabled.get());
			ASSERT_TRUE(cfg.sip.BFCPEnabled.is_initialized());
			ASSERT_FALSE(cfg.sip.BFCPEnabled.get());
			ASSERT_TRUE(cfg.H224Enabled.is_initialized());
			ASSERT_FALSE(cfg.H224Enabled.get());
			ASSERT_EQ(cfg.Address.protocol, net::protocol::TLS);
		}
	}

// test with root, host and user configurations
#define TestValueRHU(name,root_config_val,host_config_val,user_config_val, expected_val) \
	root_config = MakeRootConfig(); \
	host_config = MakeHostConfig(); \
	user_config = MakeUserConfig(); \
	root_config.name = root_config_val; \
	host_config.name = host_config_val; \
	user_config.name = user_config_val; \
	MakeMerge(); \
	ASSERT_TRUE(base_config.name.is_initialized()); \
	ASSERT_TRUE(base_config.name == expected_val); \

// test with root and host configurations
#define TestValueRH(name,root_config_val,host_config_val, expected_val) \
	root_config = MakeRootConfig(); \
	host_config = MakeHostConfig(); \
	user_config = {}; /*reset*/ \
	root_config.name = root_config_val; \
	host_config.name = host_config_val; \
	MakeMerge(); \
	ASSERT_TRUE(base_config.name.is_initialized()); \
	ASSERT_TRUE(base_config.name == expected_val); \

#define TestValueRHU_NoOptional(name,root_config_val,host_config_val,user_config_val, expected_val) \
	root_config = MakeRootConfig(); \
	host_config = MakeHostConfig(); \
	user_config = MakeUserConfig(); \
	root_config.name = root_config_val; \
	host_config.name = host_config_val; \
	user_config.name = user_config_val; \
	MakeMerge(); \
	ASSERT_TRUE(base_config.name == expected_val); \

#define TestValueRH_NoOptional(name,root_config_val,host_config_val, expected_val) \
	root_config = MakeRootConfig(); \
	host_config = MakeHostConfig(); \
	user_config = {}; /*reset*/ \
	root_config.name = root_config_val; \
	host_config.name = host_config_val; \
	MakeMerge(); \
	ASSERT_TRUE(base_config.name == expected_val); \

	struct CallConfigTest : public testing::Test
	{
		boost::shared_ptr<VS_IndentifierSIP_TestImpl> sip_ident;
		std::shared_ptr<VS_CallConfigStorage> storage = vs::MakeShared<VS_CallConfigStorage>();
		net::address host;
		std::string remote_user = "username";
		VS_CallConfig::eSignalingProtocol my_sign_protocol = VS_CallConfig::eSignalingProtocol::SIP;
		VS_CallConfig base_config;

		VS_CallConfig root_config, host_config, user_config;

		void SetUp() override
		{
			sip_ident = boost::make_shared<VS_IndentifierSIP_TestImpl>();
			storage->RegisterProtocol(sip_ident);
			host = net::address::from_string("192.168.41.195");

			root_config.SignalingProtocol = my_sign_protocol;

			host_config.Address.addr = host;
			host_config.SignalingProtocol = my_sign_protocol;

			user_config.Address.addr = host;
			user_config.Login = remote_user;
			user_config.SignalingProtocol = my_sign_protocol;
		}
		VS_CallConfig MakeRootConfig() const {
			VS_CallConfig root_config;
			root_config.SignalingProtocol = my_sign_protocol;
			return root_config;
		}

		VS_CallConfig MakeHostConfig() const {
			VS_CallConfig host_config;
			host_config.Address.addr = host;
			host_config.SignalingProtocol = my_sign_protocol;
			return host_config;
		}

		VS_CallConfig MakeUserConfig() const {
			VS_CallConfig user_config;
			user_config.Address.addr = host;
			user_config.Login = remote_user;
			user_config.SignalingProtocol = my_sign_protocol;
			return user_config;
		}

		void MakeMerge() {
			sip_ident->ClearConfigurations();
			sip_ident->AddRootConfiguration(std::move(root_config));
			sip_ident->AddHostConfiguration(std::move(host_config));
			sip_ident->AddUserConfiguration(std::move(user_config));
			storage->UpdateSettings();
			base_config = storage->GetConfiguration({ host, 0, net::protocol::any }, VS_CallConfig::SIP, remote_user);
		}
	};

	// Use TestValue macro.
	//	1. Specify variable name to test.
	//	2. Specify variable value for root, host and user configurations.
	//	3. Specify expected value after merging of three configurations.

	TEST_F(CallConfigTest, PriorityTest) {

		// values critical for skype for business call, make sure priority is normal
		TestValueRHU(sip.SRTPEnabled, false, false, true, true);	// Verifying VS_CallConfig::sip.SRTPEnabled value, root = false, host = false, user = true. Expect final value will be 'true'
		TestValueRHU(sip.SRTPEnabled, false, true, true, true);		// Verifying VS_CallConfig::sip.SRTPEnabled value, root = false, host = true, user = true. Expect final value will be 'true'
		TestValueRH(sip.SRTPEnabled, true, false, false);			// Verifying VS_CallConfig::sip.SRTPEnabled value, root = true, host = false. Expect final value will be 'false'
		TestValueRH(sip.SRTPEnabled, false, true, true);
		TestValueRH(sip.SRTPEnabled, true, true, true);

		TestValueRHU(sip.BFCPEnabled, false, false, true, true);
		TestValueRHU(sip.BFCPEnabled, false, true, false, false);
		TestValueRH(sip.BFCPEnabled, false, true, true);
		TestValueRH(sip.BFCPEnabled, true, false, false);

		TestValueRHU(sip.ICEEnabled, false, false, true, true);
		TestValueRHU(sip.ICEEnabled, false, true, false, false);
		TestValueRH(sip.ICEEnabled, false, true, true);
		TestValueRH(sip.ICEEnabled, true, false, false);

		TestValueRHU(H224Enabled, false, false, true, true);
		TestValueRHU(H224Enabled, false, true, false, false);
		TestValueRH(H224Enabled, false, true, true);
		TestValueRH(H224Enabled, true, false, false);

		TestValueRHU_NoOptional(Codecs, "root_codecs", "host_codecs", "user_codecs", "user_codecs");
		TestValueRH_NoOptional(Codecs, "root_codecs", "host_codecs", "host_codecs");
	}

	template<class B>
	class StatusRegistrationConfigTest : public B
	{
	public:
		template<typename ...Args>
		StatusRegistrationConfigTest(Args&& ...args) : B(std::forward<Args>(args)...){}

	public:
		static const char NAME[]; // = "test_name";
		static const char ADDR[]; // = "asterisk.trueconf.ua";
		static const char USER[]; // = "username";
		static const char PASSWORD[]; // = "password";
		//static const char REG_CONF_NAME[]; // = "555";

	protected:
		template<class T, typename ...Args>
		using list_t = std::list<T, Args...>;

		struct ConfigMetaData
		{
			std::string name;
			RegistryVT  type;
			std::string value;
		};

		struct Config
		{
			std::pair<std::string, list_t<ConfigMetaData>> baseConfigs;
			std::pair<std::string, list_t<ConfigMetaData>> configs;
		};

		void SetUp() override
		{
			B::SetUp();
			InitDefaultConfig();
		}

	private:
		static bool SetData(VS_RegistryKey &key, const ConfigMetaData &data)
		{
			bool result = false;
			switch (data.type)
			{
			case VS_REG_INTEGER_VT:
			{
				const int32_t val32 = boost::lexical_cast<int32_t>(data.value);
				result = key.SetValue(&val32, sizeof(val32), VS_REG_INTEGER_VT, data.name.c_str());
			}
			break;
			case VS_REG_INT64_VT:
			{
				const int64_t val64 = boost::lexical_cast<int64_t>(data.value);
				result = key.SetValue(&val64, sizeof(val64), VS_REG_INT64_VT, data.name.c_str());
			} break;
			case VS_REG_STRING_VT:
			{
				result = key.SetString(data.value.c_str(), data.name.c_str());
			}
			break;
			case VS_REG_BINARY_VT:
			{
				result = key.SetValue(data.value.c_str(), data.value.length(), VS_REG_BINARY_VT, data.name.c_str());
			}
			break;
			}
			return result;
		}

		static list_t<ConfigMetaData> GetData(VS_RegistryKey &key)
		{
			std::unique_ptr<void, free_deleter> value;
			RegistryVT type;
			std::string name_val;
			int32_t offset;

			list_t<ConfigMetaData> result;

			key.ResetValues();
			while ((offset = key.NextValueAndType(value, type, name_val)) > 0)
			{
				ConfigMetaData item;
				item.name = std::move(name_val);
				item.type = type;

				std::string res_value;

				switch (type)
				{
				case VS_REG_INTEGER_VT:
				{
					res_value = std::to_string((*static_cast<const int32_t*>(value.get())));
					break;
				}
				case VS_REG_INT64_VT:
				{
					res_value = std::to_string(*static_cast<const int64_t*>(value.get()));
					break;
				}
				case VS_REG_STRING_VT:
				{
					res_value = std::string(static_cast<const char*>(value.get()), offset - 1);
					break;
				}
				default:
				{
					res_value = std::string(static_cast<const char*>(value.get()), offset);
					break;
				}
				}
				item.value = std::move(res_value);
				result.push_back(std::move(item));
			}
			return result;
		}
	protected:
		static Config StoreConfig(const char *peersName)
		{
			VS_RegistryKey key{ false, peersName, true, false };
			Config config;
			if (key.IsValid())
			{
				config.baseConfigs = std::make_pair(key.GetName(), GetData(key));

				VS_RegistryKey sub_key;
				key.ResetKey();
				if(key.NextKey(sub_key))
				{
					config.configs = std::make_pair(sub_key.GetName(), GetData(sub_key));
				}
			}
			return config;
		}

		static void LoadConfig(const Config &config)
		{
			VS_RegistryKey key{ false, "", false };
			if(key.IsValid())
			{
				ASSERT_TRUE(key.RemoveKey(config.baseConfigs.first));
			}
			key = VS_RegistryKey{ false, config.baseConfigs.first, false, true };
			if (key.IsValid())
			{
				for (auto &&item : config.baseConfigs.second)
				{
					EXPECT_TRUE(SetData(key, item));
				}
			}
			auto name_cfg = config.baseConfigs.first + "\\" + config.configs.first;
			auto sub_key = VS_RegistryKey{ false, name_cfg, false, true };
			if (sub_key.IsValid())
			{
				for (auto &&item : config.configs.second)
				{
					EXPECT_TRUE(SetData(sub_key, item));
				}
			}
		}

		template<class I>
		struct VS_IndentifierExtended : public I
		{
			template<typename... Args>
			VS_IndentifierExtended(Args&& ...args) : I(std::forward<Args>(args)...) {}
		private:
			bool AsyncResolveImpl(std::function<void()>& resolve_task) const override
			{
				resolve_task();
				return true;
			}
		};

	private:
		virtual void InitDefaultConfig() = 0;
	};

	template<class B>
	const char StatusRegistrationConfigTest<B>::NAME[] = "test_name";
	template<class B>
	const char StatusRegistrationConfigTest<B>::ADDR[] = "192.168.61.183";
	template<class B>
	const char StatusRegistrationConfigTest<B>::USER[] = "username";
	template<class B>
	const char StatusRegistrationConfigTest<B>::PASSWORD[] = "password";

	class StatusRegistrationConfigSIPTest : public StatusRegistrationConfigTest<SIPParserTestBase<::testing::Test>>
	{
	public:
		static const char REG_CONF_NAME[];
	public:
		static void SetUpTestCase()
		{
			StatusRegistrationConfigTest::SetUpTestCase();
			//store data for sip
			m_sipConfig = StoreConfig(SIP_PEERS_KEY);
		}
		static void TearDownTestCase()
		{
			StatusRegistrationConfigTest::TearDownTestCase();
			//load data for sip
			LoadConfig(m_sipConfig);
		}
	protected:
		void SetUp() override
		{
			StatusRegistrationConfigTest::SetUp();

			auto storage = vs::MakeShared<VS_CallConfigStorage>();
			storage->RegisterProtocol(boost::make_shared<VS_IndentifierExtended<VS_IndentifierSIP>>(g_asio_environment->IOService(), "serverVendor"));

			//TODO:FIXME
			//m_sipCallResolver = VS_SIPCallResolverExtended::Create(g_asio_environment->IOService(), nullptr, sip, storage);

			sip->SetCallConfigStorage(storage);
		}
		void TearDown() override
		{
			//stub
		}
	protected:
		class VS_SIPCallResolverExtended : public VS_SIPCallResolver
		{
		public:
			using VS_SIPCallResolver::Timeout;
		protected:
			template<typename ...Args>
			VS_SIPCallResolverExtended(Args&& ...args) : VS_SIPCallResolver(std::forward<Args>(args)...) {}

			static void PostConstruct(std::shared_ptr<VS_SIPCallResolverExtended>&) { /*stub*/ }
		};
	public:
		boost::shared_ptr<VS_SIPCallResolverExtended> m_sipCallResolver;
	private:
		void InitDefaultConfig() override
		{
			Config init_cfg;
			//init base
			init_cfg.baseConfigs = std::make_pair(std::string(SIP_PEERS_KEY),
				list_t<ConfigMetaData>{ {"Default Proxy", VS_REG_STRING_VT, REG_CONF_NAME } });
			//init peers data
			init_cfg.configs = std::make_pair(std::string(REG_CONF_NAME),
				list_t<ConfigMetaData>{
					{ "Authorization Name", VS_REG_STRING_VT, USER },
					{ "BFCP Enabled", VS_REG_INTEGER_VT, "1" },
					{ "Hostname", VS_REG_STRING_VT, ADDR },
					{ "ICE Enabled", VS_REG_INTEGER_VT, "1" },
					{ "Login", VS_REG_STRING_VT, USER },
					{ "Marker", VS_REG_INTEGER_VT, "0" },
					{ "Name", VS_REG_STRING_VT, NAME },
					{ "OriginClientId", VS_REG_STRING_VT, "webConfig"},
					{ "OriginSubtype", VS_REG_STRING_VT, "v3"},
					{ "OriginType", VS_REG_STRING_VT, "restfulapi"},
					{ "Password", VS_REG_STRING_VT, PASSWORD },
					{ "Port", VS_REG_INTEGER_VT, "5060"},
					{ "Protocol", VS_REG_STRING_VT, ""},
					{ "RegisterStrategy", VS_REG_INTEGER_VT, "1"},
					{ "SRTP Enabled", VS_REG_INTEGER_VT, "1"}
				});
			LoadConfig(init_cfg);
			VS_RegistryKey::TEST_DumpData();
		}
		static Config m_sipConfig;
	};

	StatusRegistrationConfigSIPTest::Config StatusRegistrationConfigSIPTest::m_sipConfig;
	const char StatusRegistrationConfigSIPTest::REG_CONF_NAME[] = "444";
	//////////////////////////////////////////////////////////////////////////

	namespace
	{
		inline void set_sip_via_field(const std::shared_ptr<VS_SIPMessage> &msg, std::string &response)
		{
			const auto port = msg->GetSIPMetaField()->iVia.front()->Port();
			const std::string from = !port ? msg->GetSIPMetaField()->iVia.front()->Host() :
			msg->GetSIPMetaField()->iVia.front()->Host() + ":" + std::to_string(port);
			strreplace(response, "__from__", from);
			strreplace(response, "__branch__", msg->Branch());
		}

		inline void set_sip_from_field(const std::shared_ptr<VS_SIPMessage> &msg, std::string &response)
		{
			const auto port = msg->GetSIPMetaField()->iFrom->GetURI()->Port();
			const std::string host_from = !port ? msg->GetSIPMetaField()->iFrom->GetURI()->Host() :
				std::string(msg->GetSIPMetaField()->iFrom->GetURI()->Host()) + ":" + std::to_string(port);

			strreplace(response, "__user_from__", msg->GetSIPMetaField()->iFrom->GetURI()->User().c_str());
			strreplace(response, "__host_from__", host_from.c_str());
			strreplace(response, "__tag_from__", msg->GetSIPMetaField()->iFrom->GetURI()->Tag().c_str());
		}

		inline void set_sip_to_field(const std::shared_ptr<VS_SIPMessage> &msg, std::string &response)
		{
			const auto port = msg->GetSIPMetaField()->iTo->GetURI()->Port();
			const std::string host_to = !port ? msg->GetSIPMetaField()->iTo->GetURI()->Host() :
				std::string(msg->GetSIPMetaField()->iTo->GetURI()->Host()) + ":" + std::to_string(port);

			strreplace(response, "__user_to__", msg->GetSIPMetaField()->iTo->GetURI()->User().c_str());
			strreplace(response, "__host_to__", host_to.c_str());
		}

		inline void set_sip_callid_cseq_field(const std::shared_ptr<VS_SIPMessage> &msg, std::string &response)
		{
			strreplace(response, "__callid__", msg->CallID());
			strreplace(response, "__cseq__", std::to_string(msg->GetCSeq()).c_str());
		}

		inline void set_sip_contact_field(const std::shared_ptr<VS_SIPMessage> &msg, std::string &response)
		{
			strreplace(response, "__contact_user__", msg->GetSIPMetaField()->iContact->GetLastURI()->User().c_str());

			const auto port = msg->GetSIPMetaField()->iContact->GetLastURI()->Port();
			const std::string host = !port ? msg->GetSIPMetaField()->iContact->GetLastURI()->Host() :
				std::string(msg->GetSIPMetaField()->iContact->GetLastURI()->Host()) + ":" + std::to_string(port);
			strreplace(response, "__contact_host__", host.c_str());
		}

		inline void set_sip_expires_field(const std::shared_ptr<VS_SIPMessage> &msg, std::string &response, const int expires = -1)
		{
			strreplace(response, "__expires__", std::to_string(expires >= 0 ? expires : msg->GetSIPMetaField()->iExpires->Value().count()).c_str());
		}

		inline void init_registration_sip(StatusRegistrationConfigSIPTest &self)
		{

			VS_RegistryKey::TEST_DumpData();

			auto msg = self.GetMessageFromParser(self.sip);
			ASSERT_TRUE(msg);
			ASSERT_TRUE(msg->IsValid());
			ASSERT_EQ(MESSAGE_TYPE_REQUEST, msg->GetSIPMetaField()->iStartLine->GetMessageType());
			ASSERT_EQ(TYPE_REGISTER, msg->GetSIPMetaField()->iCSeq->GetType());
			self.sip->Timeout();

			VS_RegistryKey key{ false, SIP_PEERS_KEY + std::string{ "\\" } +StatusRegistrationConfigSIPTest::REG_CONF_NAME };
			ASSERT_TRUE(key.IsValid());
			int32_t res = -1;
			ASSERT_TRUE(key.GetValue(&res, sizeof(res), VS_REG_INTEGER_VT, "Last Authorization Result"));
			ASSERT_EQ(VS_CallConfig::e_Pending, res);

			std::string response = raw_sip_registration_unauthorized_401Unauthorized;

			set_sip_via_field(msg, response);
			set_sip_from_field(msg, response);
			set_sip_to_field(msg, response);
			set_sip_callid_cseq_field(msg, response);

			ASSERT_TRUE(self.SetRecvBuf(&response[0], response.size()));
		}

		inline void test_body_sip(StatusRegistrationConfigSIPTest &self,
			const std::function<std::string(const std::shared_ptr<VS_SIPMessage> &msg)> &getResponse, VS_CallConfig::VerificationResult resVerification)
		{
			self.sip->Timeout();
			auto &&msg = self.GetMessageFromParser(self.sip);
			ASSERT_TRUE(msg);
			ASSERT_TRUE(msg->IsValid());
			ASSERT_EQ(MESSAGE_TYPE_REQUEST, msg->GetSIPMetaField()->iStartLine->GetMessageType());
			ASSERT_EQ(TYPE_REGISTER, msg->GetSIPMetaField()->iCSeq->GetType());

			auto&& response = getResponse(msg);

			ASSERT_TRUE(self.SetRecvBuf(&response[0], response.size()));
			self.sip->Timeout();

			VS_RegistryKey::TEST_DumpData();

			VS_RegistryKey key{ false, SIP_PEERS_KEY + std::string{ "\\" } + StatusRegistrationConfigSIPTest::REG_CONF_NAME };
			ASSERT_TRUE(key.IsValid());

			int32_t res = -1;
			ASSERT_TRUE(key.GetValue(&res, sizeof(res), VS_REG_INTEGER_VT, "Last Authorization Result"));
			ASSERT_EQ(resVerification, res);

			self.sip->Shutdown();

			ASSERT_TRUE(key.GetValue(&res, sizeof(res), VS_REG_INTEGER_VT, "Last Authorization Result"));
			ASSERT_EQ(VS_CallConfig::e_Unknown, res);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	TEST_F(StatusRegistrationConfigSIPTest, DISABLED_StatusRegistrationSIPSuccess)
	{
		m_sipCallResolver->Start();
		m_sipCallResolver->Timeout(); // execute ReinitializeConfiguration

		init_registration_sip(*this);
		test_body_sip(*this, [](const auto &msg)
		{
			std::string response = raw_sip_registration_200Ok;
			set_sip_via_field(msg, response);
			set_sip_from_field(msg, response);
			set_sip_to_field(msg, response);
			set_sip_callid_cseq_field(msg, response);
			set_sip_contact_field(msg, response);
			set_sip_expires_field(msg, response);
			return response;
		}, VS_CallConfig::e_Valid);
	}

	TEST_F(StatusRegistrationConfigSIPTest, DISABLED_StatusRegistrationSIPFailed)
	{
		m_sipCallResolver->Start();
		m_sipCallResolver->Timeout(); // execute ReinitializeConfiguration
		init_registration_sip(*this);
		test_body_sip(*this, [](const auto &msg)
		{
			std::string response = raw_sip_registration_403Forbidden;
			set_sip_via_field(msg, response);
			set_sip_from_field(msg, response);
			set_sip_to_field(msg, response);
			set_sip_callid_cseq_field(msg, response);
			return response;
		}, VS_CallConfig::e_Forbidden);
	}

	TEST_F(StatusRegistrationConfigSIPTest, DISABLED_StatusRegistrationSIPServerUnreachable)
	{
		VS_RegistryKey key{ false, SIP_PEERS_KEY + std::string{ "\\" } +StatusRegistrationConfigSIPTest::REG_CONF_NAME, false };
		ASSERT_TRUE(key.IsValid());
		const char host[] = "/test/";

		ASSERT_TRUE(key.SetString(host, "Hostname"));

		m_sipCallResolver->Start();
		m_sipCallResolver->Timeout(); // execute ReinitializeConfiguration

		int32_t res = -1;
		ASSERT_TRUE(key.GetValue(&res, sizeof(res), VS_REG_INTEGER_VT, "Last Authorization Result"));
		ASSERT_EQ(VS_CallConfig::e_ServerUnreachable, res);
	}

	//////////////////////////////////////////////////////////////////////////

	class StatusRegistrationConfigH225RASTest : public StatusRegistrationConfigTest<::testing::Test>
	{
	public:
		static const char REG_CONF_NAME[];
		static const unsigned short PORT;// = 1719;
	public:

		static void SetUpTestCase()
		{
			m_225RasConfig = StoreConfig(H323_PEERS_KEY);
		}

		static void TearDownTestCase()
		{
			LoadConfig(m_225RasConfig);
		}

	protected:
		StatusRegistrationConfigH225RASTest()
			: strand(g_asio_environment->IOService())
		{
		}

		void SetUp() override
		{
			StatusRegistrationConfigTest::SetUp();

			m_fakeTransPool = std::make_shared<test::TransceiversPoolFake>();

			auto &&storage = vs::MakeShared<VS_CallConfigStorage>();
			storage->RegisterProtocol(boost::make_shared<VS_IndentifierExtended<VS_IndentifierH225RAS>>(g_asio_environment->IOService()));

			std::function<bool(const std::string&, const std::string&)> func_stub =
				[](const std::string&, const std::string&) -> bool
			{
				return true;
			};

			VS_H225RequestSeqNum seq_num;

			boost::asio::io_service ios;
			boost::asio::io_service::strand strand(ios);
			VS_SignalConnectionsMgr::InitInfo info(strand);
			info.checkDigest = func_stub;
			info.getUserStatus = [](string_view /**call_id*/, bool/**use_cache*/, bool/**do_ext_resolve*/) -> UserStatusInfo { return UserStatusInfo(); }, // get_user_status;
			info.ourEndpoint = "";
			info.ourService = "";
			info.postMes = [](VS_RouterMessage*) { return true; };
			info.peerConfig = storage;
			/*
			info.writeEvent = [this](const void *buf, std::size_t sz)
			{
				VS_PerBuffer req_buff(buf, sz * 8);
				VS_RasMessage req_mess;
				if (req_mess.Decode(req_buff) && req_mess.tag == req_mess.e_registrationRequest)
				{
					VS_RasRegistrationRequest* rrq = dynamic_cast<VS_RasRegistrationRequest*>(req_mess.choice);
					m_seq_num = boost::make_shared<VS_H225RequestSeqNum>(rrq->requestSeqNum);
				}
			};
			*/
			info.transcPool = m_fakeTransPool;
			m_h323CallResolver = vs::MakeShared<VS_SignalConnectionsMgrExtended>(std::move(info));
		}

		void TearDown() override
		{
			StatusRegistrationConfigTest::TearDown();
		}
	private:
		void InitDefaultConfig() override
		{
			Config init_cfg;
			init_cfg.baseConfigs = std::make_pair(std::string(H323_PEERS_KEY),
				list_t<ConfigMetaData>{{"Default Proxy", VS_REG_STRING_VT, REG_CONF_NAME}});

			init_cfg.configs = std::make_pair(std::string(REG_CONF_NAME),
				list_t<ConfigMetaData>
				{
					{"H224 Enabled", VS_REG_INTEGER_VT, "1"},
					{"H235 Enabled", VS_REG_INTEGER_VT, "0"},
					{"H239 Enabled", VS_REG_INTEGER_VT, "1"},
					{"Hostname", VS_REG_STRING_VT, ADDR },
					{"Login", VS_REG_STRING_VT, USER },
					{"Name", VS_REG_STRING_VT, NAME},
					{"OriginClientId", VS_REG_STRING_VT, "webConfig"},
					{"OriginSubtype", VS_REG_STRING_VT, "v3"},
					{"OriginType", VS_REG_STRING_VT, "restfulapi"},
					{"Port", VS_REG_INTEGER_VT, "1719"},
					{"Protocol", VS_REG_STRING_VT, "UDP"},
					{"RegisterStrategy", VS_REG_INTEGER_VT, "1"}
				}
			);

			m_addrConfig.protocol = net::protocol::UDP;
			m_addrConfig.port = PORT;
			m_addrConfig.addr = net::address::from_string(ADDR);

			LoadConfig(init_cfg);
			VS_RegistryKey::TEST_DumpData();
		}
	public:
		class VS_SignalConnectionsMgrExtended : public VS_SignalConnectionsMgr
		{
		public:
			using VS_SignalConnectionsMgr::Start;
			using VS_SignalConnectionsMgr::GetTransportConnectionByAddress;

		protected:
			template<typename ...Args>
			VS_SignalConnectionsMgrExtended(Args&& ...args) : VS_SignalConnectionsMgr(std::forward<Args>(args)...) {}
			static void PostConstruct(std::shared_ptr<VS_SignalConnectionsMgrExtended>&) { /*stub*/ }
		};

		boost::asio::io_service::strand strand;
		std::shared_ptr<VS_SignalConnectionsMgrExtended> m_h323CallResolver;
		std::shared_ptr<test::TransceiversPoolFake> m_fakeTransPool;
		net::Endpoint m_addrConfig;
		boost::shared_ptr<VS_H225RequestSeqNum> m_seq_num;
	private:
		static Config m_225RasConfig;
	};

	StatusRegistrationConfigH225RASTest::Config StatusRegistrationConfigH225RASTest::m_225RasConfig;
	const char StatusRegistrationConfigH225RASTest::REG_CONF_NAME[] = "555";
	const unsigned short StatusRegistrationConfigH225RASTest::PORT = 1719;

	////////////////////////////////////////////////////////////////////////

	//TODO: переделать
	TEST_F(StatusRegistrationConfigH225RASTest, DISABLED_StatusRegistrationConfigH225RASSuccess)
	{
		m_h323CallResolver->Start();
		//TODO: Use VS_SteadyClockWrapper
		// m_h323CallResolver->Timeout();
		vs::SleepFor(std::chrono::seconds(3));
		VS_RegistryKey::TEST_DumpData();

		VS_RegistryKey key{ false, H323_PEERS_KEY + std::string{ "\\" } +REG_CONF_NAME };
		EXPECT_TRUE(test::WaitFor("config_status.pending", [&key]()
		{
			int32_t res = -1;
			return key.IsValid() &&
				key.GetValue(&res, sizeof(res), VS_REG_INTEGER_VT, "Last Authorization Result") > 0 &&
				res == VS_CallConfig::e_Pending;
		}, 500 * 2, 10 * 2));

		boost::shared_ptr<VS_TransportConnection> transport_connn;
		g_asio_environment->IOService().post([&transport_connn, h323CallResolver = m_h323CallResolver, addrConfig = m_addrConfig]()
		{
			//TODO:PORTME
			//transport_connn = h323CallResolver->GetTransportConnectionByAddress(addrConfig);
		});

		EXPECT_TRUE(test::WaitFor("GetTransportConnectionByAddress", [&transport_connn]()
		{
			return transport_connn;
		}, 500 * 2, 10 * 2));

		ASSERT_TRUE(m_seq_num);

		VS_PerBuffer confirm_buff(h225raw::raw_gnugk_reg_confirm, sizeof(h225raw::raw_gnugk_reg_confirm) * 8);
		VS_RasMessage confirm_mess;
		ASSERT_TRUE(confirm_mess.Decode(confirm_buff));
		ASSERT_EQ(confirm_mess.tag, confirm_mess.e_registrationConfirm);

		VS_RasRegistrationConfirm *rcf = dynamic_cast<VS_RasRegistrationConfirm*>(confirm_mess.choice);
		rcf->requestSeqNum = *m_seq_num;

		VS_PerBuffer confirm_buff_encode;
		ASSERT_TRUE(confirm_mess.Encode(confirm_buff_encode));
		//TODO: Pass data directly to the parser
		//ASSERT_LT(0, transport_connn->SetRecvBuf(confirm_buff_encode.GetData(), confirm_buff_encode.ByteSize(), e_RAS, m_addrConfig, VS_IPPortAddress()));

		int32_t res = -1;
		ASSERT_TRUE(key.IsValid() && key.GetValue(&res, sizeof(res), VS_REG_INTEGER_VT, "Last Authorization Result") > 0);
		ASSERT_EQ(res, VS_CallConfig::e_Valid);
	}

	struct EqualityTest : public ::testing::Test {
		VS_CallConfig config1, config2;

		void SetUp() override {
			config2.SignalingProtocol = config1.SignalingProtocol = VS_CallConfig::eSignalingProtocol::SIP;
		}
	};

	TEST_F(EqualityTest, SameIP_differentHosts) {
		config1.Address.addr = net::address_v4::from_string("1.2.3.4");
		config1.HostName = "hostname1";

		config2.HostName = "hostname2";
		config2.Address = config1.Address;

		VS_CallConfigManager<VS_CallConfig> mgr(config1);
		ASSERT_TRUE(mgr.TestEndpointsEqual(config2));
	}

	TEST_F(EqualityTest, SubHosts) {
		config1.HostName = "host.loc";
		config2.HostName = ".loc";

		VS_CallConfigManager<VS_CallConfig> mgr(config1);
		ASSERT_TRUE(mgr.TestEndpointsEqual(config2));
	}

	TEST_F(EqualityTest, SameHosts) {
		config1.HostName = "host.loc";
		config2.HostName = "host.loc";

		VS_CallConfigManager<VS_CallConfig> mgr(config1);
		ASSERT_TRUE(mgr.TestEndpointsEqual(config2));
	}

	TEST_F(EqualityTest, NotSubHosts) {
		config1.HostName = ".loc";
		config2.HostName = "host.loc";

		VS_CallConfigManager<VS_CallConfig> mgr(config1);
		ASSERT_FALSE(mgr.TestEndpointsEqual(config2));
	}

	TEST_F(EqualityTest, DifferentHosts) {
		config1.HostName = "host1.loc";
		config2.HostName = "host2.loc";

		VS_CallConfigManager<VS_CallConfig> mgr(config1);
		ASSERT_FALSE(mgr.TestEndpointsEqual(config2));
	}
}
