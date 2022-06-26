#if defined(_WIN32) // Not ported yet

#include "common.h"

namespace terminals_tests
{
	/// !!!! Polycom HDX8000 !!!!

	// VCS to HDX8000
	class HDX8000_H323_To_Test : public H323SniffBasedTestBase
	{
	public:
		HDX8000_H323_To_Test()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.41.140"), 54464, // VCS
			net::address_v4::from_string("192.168.62.42"), 1720,   // Terminal
			"hdx8000_to_h225.h",
			"hdx8000_to_h245.h")
		{}
	};

	TEST_F(HDX8000_H323_To_Test, DISABLED_RunTestBody)
	{
		SetCallInfo("#h323:192.168.62.42",
			"b@brchk000.trueconf.ua",
			"4632658FED9C28DFB306F0001334204C",
			Direction::VCS_TO_TERMINAL);
		SniffBasedTestBody();
	}

	// HDX8000 to VCS
	class HDX8000_H323_From_Test : public H323SniffBasedTestBase
	{
	public:
		HDX8000_H323_From_Test()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.61.181"), 33962, // VCS
			net::address_v4::from_string("192.168.62.42"), 1720,   // Terminal
			"hdx8000_from_h225.h",
			"hdx8000_from_h245.h")
		{}
	};

	TEST_F(HDX8000_H323_From_Test, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"", // default call destination
			"#h323:hdx8000@192.168.62.42",
			"024500D2B9251C582F420C556A5BF8D1",
			Direction::TERMINAL_TO_VCS);
		SniffBasedTestBody();
	}

	/// !!!! Sony XG77 !!!!
	class Sony_XG77_H323_To_TC : public H323SniffBasedTestBase
	{
	public:
		Sony_XG77_H323_To_TC()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.62.79"), 1720,   // VCS
			net::address_v4::from_string("192.168.62.153"), 2259,  // Terminal
			"sony-xg77_to_tc_h225.h",
			"sony-xg77_to_tc_h245.h")
		{}
	};

	TEST_F(Sony_XG77_H323_To_TC, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"", // default call destination
			"#h323:PCS-XG77@192.168.62.153",
			"3679C02BCE0A001F328AFB187156D7C5",
			Direction::TERMINAL_TO_VCS);
		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {

			if (!IsH264HDVideo(video))
				return false;

			if (audio.CodecType != e_rcvG722_64k)
				return false;

			return true;
		});
		SetPacketToSkip(H245, 0, 1); // Skip: [TCP segment of a reassembled PDU]
		SniffBasedTestBody();
	}

	class Sony_XG77_H323_From_TC : public H323SniffBasedTestBase
	{
	public:
		Sony_XG77_H323_From_TC()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.62.79"), 41607,   // VCS
			net::address_v4::from_string("192.168.62.153"), 1720,  // Terminal
			"sony-xg77_from_tc_h225.h",
			"sony-xg77_from_tc_h245.h")
		{}
	};

	TEST_F(Sony_XG77_H323_From_TC, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"#h323:@192.168.62.79",
			"a@matvey.trueconf.loc",
			"C53DC26987F3D3110839E2E1443CEF3A",
			Direction::VCS_TO_TERMINAL);
		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {

			if (!IsH264HDVideo(video))
				return false;

			if (audio.CodecType != e_rcvG722_64k)
				return false;

			return true;
		});
		SetPacketToSkip(H245, 0, 0); // Skip: [TCP segment of a reassembled PDU]
		SniffBasedTestBody();
	}

	// !!!! Tandberg GVC3200 !!!!
	class Tandberg_GVC3200_To_TC : public H323SniffBasedTestBase
	{
	public:
		Tandberg_GVC3200_To_TC()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.62.79"), 1720,   // VCS
			net::address_v4::from_string("192.168.62.77"), 55433,  // Terminal
			"GVC3200_to_tc_h225.h",
			"GVC3200_to_tc_h245.h")
		{}
	};

	TEST_F(Tandberg_GVC3200_To_TC, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"", // default call destination
			"#h323:@192.168.62.77",
			"F055033757F0E51184EC000B8275540D",
			Direction::TERMINAL_TO_VCS);
		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {

			if (!IsH264HDVideo(video))
				return false;

			if (audio.CodecType != e_rcvG722_64k)
				return false;

			return true;
		});
		SniffBasedTestBody();
	}

	class Tandberg_GVC3200_From_TC : public H323SniffBasedTestBase
	{
	public:
		Tandberg_GVC3200_From_TC()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.62.79"), 6450,   // VCS
			net::address_v4::from_string("192.168.62.77"), 1720,  // Terminal
			"GVC3200_from_tc_h225.h",
			"GVC3200_from_tc_h245.h")
		{}
	};

	TEST_F(Tandberg_GVC3200_From_TC, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"#h323:@192.168.62.77",
			//"#h323:test@fd24:800c:c8d7:881f::2",
			"a@matv001.trueconf.name",
			"00F722FF43F889A0D24B7096914DF478",
			Direction::VCS_TO_TERMINAL);
		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {

			if (!IsH264HDVideo(video))
				return false;

			if (audio.CodecType != e_rcvG722_64k)
				return false;

			return true;
		});
		SniffBasedTestBody();
	}

	// !!!! Yealink VC400 !!!!
	class Yealink_VC400_To_TC : public H323SniffBasedTestBase
	{
	public:
		Yealink_VC400_To_TC()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.62.79"), 50250,   // VCS
			net::address_v4::from_string("192.168.62.48"), 1720,  // Terminal
			"Yealink_VC400_to_tc_h225.h",
			"Yealink_VC400_to_tc_h245.h")
		{}
	};

	TEST_F(Yealink_VC400_To_TC, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"#h323:yeah323@192.168.62.48",
			"a@matv001.trueconf.name",
			"62EF21AED4C2E51199D700156586FC9F",
			Direction::TERMINAL_TO_VCS);
		SniffBasedTestBody();
	}

	class Yealink_VC400_From_TC : public H323SniffBasedTestBase
	{
	public:
		Yealink_VC400_From_TC()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.62.79"), 11004,   // VCS
			net::address_v4::from_string("192.168.62.48"), 1720,  // Terminal
			"Yealink_VC400_from_tc_h225.h",
			"Yealink_VC400_from_tc_h245.h")
		{}
	};

	TEST_F(Yealink_VC400_From_TC, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"#h323:yeah323@192.168.62.48",
			"#h323:@192.168.62.79",
			"9BCC04C46D665DC280AF42F267886C3D",
			Direction::VCS_TO_TERMINAL);
		SniffBasedTestBody();
	}

	// !!!! Sony PCS-101 (PCS-TL30) !!!!
	static bool Sony_PCS_101_media_modes_validator(const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video)
	{

		std::uint32_t width = 0, height = 0;
		if (video.CodecType != e_videoH264)
		{
			return false;
		}

		VS_H323BeginH245Modes::GetH264ResolutionByMaxFs(video.MaxFs, width, height);
		if (width != 352 || height != 288) // CIF
		{
			return false;
		}

		if (audio.CodecType != e_rcvG722_64k)
			return false;

		return true;
	}

	class Sony_PCS_101_To_TC : public H323SniffBasedTestBase
	{
	public:
		Sony_PCS_101_To_TC()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.41.140"), 1720,   // VCS
			net::address_v4::from_string("192.168.62.47"), 2253,    // Terminal
			"Sony-PCS-101_to_tc_h225.h",
			"Sony-PCS-101_to_tc_h245.h")
		{}
	};

	TEST_F(Sony_PCS_101_To_TC, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"",
			"#h323:win2@192.168.62.47",
			"00AACD6D8970071F2E276714BDB20381",
			Direction::TERMINAL_TO_VCS);
		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video)
		{
			return Sony_PCS_101_media_modes_validator(audio, video);
		});
		SniffBasedTestBody();
	}

	class Sony_PCS_101_From_TC : public H323SniffBasedTestBase
	{
	public:
		Sony_PCS_101_From_TC()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.41.140"), 55892,   // VCS
			net::address_v4::from_string("192.168.62.47"), 1720,  // Terminal
			"Sony-PCS-101_from_tc_h225.h",
			"Sony-PCS-101_from_tc_h245.h")
		{}
	};

	TEST_F(Sony_PCS_101_From_TC, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"#h323:@192.168.62.47",
			"b@brchk000.trueconf.ua",
			"A836D852AF0201EE8253899A0EFD90CE",
			Direction::VCS_TO_TERMINAL);
		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video)
		{
			return Sony_PCS_101_media_modes_validator(audio, video);
		});
		SniffBasedTestBody();
	}

	// !!!! Cisco E20 !!!!
	class Cisco_E20_From_TC : public H323SniffBasedTestBase
	{
	public:
		Cisco_E20_From_TC()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.41.140"), 55892,   // VCS
			net::address_v4::from_string("192.168.62.43"), 1720,  // Terminal
			"Cisco_E20_from_tc_h225.h",
			"Cisco_E20_from_tc_h245.h")
		{}
	};

	TEST_F(Cisco_E20_From_TC, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"#h323:@192.168.62.43",
			"b@brchk000.trueconf.ua",
			"966A55701C1A723041067B637EC8B678",
			Direction::VCS_TO_TERMINAL);
		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {

			std::uint32_t width = 0, height = 0;
			if (video.CodecType != e_videoH264)
			{
				return false;
			}

			VS_H323BeginH245Modes::GetH264ResolutionByMaxFs(video.MaxFs, width, height);
			if (width != 800 || height != 448)
			{
				return false;
			}

			if (audio.CodecType != e_rcvG722132)
				return false;

			return true;
		});
		SniffBasedTestBody();
	}

	// !!!! ViewPoint HD9000 !!!!
	class ViewPoint_HD9000_To_TC : public H323SniffBasedTestBase
	{
	public:
		ViewPoint_HD9000_To_TC()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.41.140"), 1720,   // VCS
			net::address_v4::from_string("192.168.62.46"), 1027,    // Terminal
			"ViewPoint_HD9000_to_tc_h225.h",
			"ViewPoint_HD9000_to_tc_h245.h")
		{}
	};

	TEST_F(ViewPoint_HD9000_To_TC, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"#h323:kt@192.168.41.140",
			"#h323:@192.168.62.46",
			"00DFFA480000E00080051F5D10E906B8",
			Direction::TERMINAL_TO_VCS);
		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {

			if (!IsH264HDVideo(video))
				return false;

			if (audio.CodecType != e_rcvG722_64k)
				return false;

			return true;
		});
		SniffBasedTestBody();
	}

	class ViewPoint_HD9000_From_TC : public H323SniffBasedTestBase
	{
	public:
		ViewPoint_HD9000_From_TC()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.41.140"), 54836,   // VCS
			net::address_v4::from_string("192.168.62.46"), 1720,     // Terminal
			"ViewPoint_HD9000_from_tc_h225.h",
			"ViewPoint_HD9000_from_tc_h245.h")
		{}
	};

	TEST_F(ViewPoint_HD9000_From_TC, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"#h323:@192.168.62.46",
			"b@brchk000.trueconf.ua",
			"B8A34D2D5128DE6FB91086208C739829",
			Direction::VCS_TO_TERMINAL);
		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {

			if (!IsH264HDVideo(video))
				return false;

			if (audio.CodecType != e_rcvG722_64k)
				return false;

			return true;
		});
		SniffBasedTestBody();
	}

	// !!!! Polycom VSX7000 !!!!
	class Polycom_VSX7000_From_TC : public H323SniffBasedTestBase
	{
	public:
		Polycom_VSX7000_From_TC()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.41.140"), 52352,   // VCS
			net::address_v4::from_string("192.168.62.41"), 1720,     // Terminal
			"Polycom_VSX7000_from_tc_h225.h",
			"Polycom_VSX7000_from_tc_h245.h")
		{}
	};

	TEST_F(Polycom_VSX7000_From_TC, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"#h323:@192.168.62.41",
			"b@brchk000.trueconf.ua",
			"C40402113624922C1549BF75385BA2CE",
			Direction::VCS_TO_TERMINAL);
		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {
			std::uint32_t width = 0, height = 0;
			if (video.CodecType != e_videoH264)
			{
				return false;
			}

			VS_H323BeginH245Modes::GetH264ResolutionByMaxFs(video.MaxFs, width, height);
			if (width != 864 || height != 480)
			{
				return false;
			}

			if (!IsSiren14Audio(audio))
				return false;

			return true;
		});
		SetPacketToSkip(H245, 0, 0); // Skip: 106 - [TCP segment of a reassembled PDU]
		SetPacketToSkip(H245, 0, 3); // Skip: 139 - [TCP Spurious Retransmission] masterSlaveDetermination
		SniffBasedTestBody();
	}

	// !!!! AVer EVC130 !!!!
	class AVer_EVC130_To_TC : public H323SniffBasedTestBase
	{
	public:
		AVer_EVC130_To_TC()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.62.79"), 1720,   // VCS
			net::address_v4::from_string("192.168.62.65"), 30005,  // Terminal
			"AVer_EVC130_to_tc_h225.h",
			"AVer_EVC130_to_tc_h245.h")
		{}
	};

	TEST_F(AVer_EVC130_To_TC, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"#h323:@192.168.62.79",
			"#h323:AVer@192.168.62.65",
			"78B3166F820A001F244607345DD2A3A1",
			Direction::TERMINAL_TO_VCS);
		SniffBasedTestBody();
	}

	class AVer_EVC130_From_TC : public H323SniffBasedTestBase
	{
	public:
		AVer_EVC130_From_TC()
			: H323SniffBasedTestBase(
			net::address_v4::from_string("192.168.62.79"), 5503,   // VCS
			net::address_v4::from_string("192.168.62.65"), 1720,   // Terminal
			"AVer_EVC130_from_tc_h225.h",
			"AVer_EVC130_from_tc_h245.h")
		{}
	};

	TEST_F(AVer_EVC130_From_TC, DISABLED_RunTestBody)
	{
		SetCallInfo(
			"#h323:@192.168.62.65",
			"a@matvey.trueconf.loc",
			"EC628C2A89AA9CA866B9C24F597F1D93",
			Direction::VCS_TO_TERMINAL);
		SniffBasedTestBody();
	}
}

#endif
