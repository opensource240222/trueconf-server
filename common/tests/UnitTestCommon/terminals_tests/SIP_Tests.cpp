#ifdef _WIN32
#include "common.h"

namespace terminals_tests
{
	// HDX8000 from VCS
	class HDX8000_SIP_From_VCS_Test : public SIPSniffBasedTestBase
	{
	public:
		HDX8000_SIP_From_VCS_Test()
			: SIPSniffBasedTestBase(
			{ net::address_v4::from_string("192.168.41.140"), 51306, net::protocol::TCP }, // VCS
			{ net::address_v4::from_string("192.168.62.42"), 5060, net::protocol::TCP },   // Terminal
			"hdx8000_from_tc_sip.h")
		{}
	};

	TEST_F(HDX8000_SIP_From_VCS_Test, RunTestBody)
	{
		SetCallInfo(
			"#sip:hdx8000@192.168.62.42",
			"b@brchk000.trueconf.ua",
			"C14C8AC5A9072A9F4DC9314BA290F4D4",
			Direction::VCS_TO_TERMINAL);
		SetBranchAndTag(
			//"z9hG4bK608D9FA57249669BF7439F99F4C653BC",
			"608D9FA57249669BF7439F99F4C653BC",
			"0AD9F983BC4EB02E40C5E6303B1995E4");
		SetPacketToSkip(SIP, 0, 0); /* 184 - [TCP segment of a reassembled PDU] */
		SetPacketToSkip(SIP, 1, 1); /* 196 - [TCP segment of a reassembled PDU] */
		SniffBasedTestBody();
	}

	// HDX8000 from VCS
	class HDX8000_SIP_To_VCS_Test : public SIPSniffBasedTestBase
	{
	public:
		HDX8000_SIP_To_VCS_Test()
			: SIPSniffBasedTestBase(
			{net::address_v4::from_string("192.168.41.156"), 5060, net::protocol::TCP },  // VCS
			{net::address_v4::from_string("192.168.62.42"), 5060, net::protocol::TCP },   // Terminal
			"hdx8000_to_tc_sip.h")
		{}
	};

	TEST_F(HDX8000_SIP_To_VCS_Test, RunTestBody)
	{
		SetCallInfo(
			"admin@artm001.trueconf.name",
			"#sip:hdx8000@192.168.62.42",
			"3247377008-287312526",
			Direction::TERMINAL_TO_VCS);
		SetBranchAndTag(
			"3247377008-287312527",
			"7694B490E02D25058733BF1943F609A1");
		SetPacketToSkip(SIP, 0, 0); /* 150 - [TCP segment of a reassembled PDU] */
		SetPacketToSkip(SIP, 1, 1); /* 176 - [TCP segment of a reassembled PDU] */
		SniffBasedTestBody();
	}

	// AVer_EVC130 to VCS
	class AVer_EVC130_SIP_To_VCS_Test : public SIPSniffBasedTestBase
	{
	public:
		AVer_EVC130_SIP_To_VCS_Test()
			: SIPSniffBasedTestBase(
			{net::address_v4::from_string("192.168.41.79"), 5060, net::protocol::UDP},   // VCS
			{net::address_v4::from_string("192.168.62.89"), 5060, net::protocol::UDP},   // Terminal
			"AVer_EVC130_to_tc_sip.h")
		{}
	};

	TEST_F(AVer_EVC130_SIP_To_VCS_Test, RunTestBody)
	{
		SetCallInfo(
			"a@192.168.62.79",
			"#sip:AVer@192.168.62.89",
			"1230108781",
			Direction::TERMINAL_TO_VCS);
		SetBranchAndTag(
			"234138762",
			"1156946410");
		SniffBasedTestBody();
	}


	// from VCS to AVer_EVC130
	class AVer_EVC130_SIP_from_VCS_Test : public SIPSniffBasedTestBase
	{
	public:
		AVer_EVC130_SIP_from_VCS_Test()
			: SIPSniffBasedTestBase(
			{net::address_v4::from_string("192.168.41.79"), 5060, net::protocol::UDP},   // VCS
			{net::address_v4::from_string("192.168.62.89"), 5060, net::protocol::UDP},   // Terminal
			"AVer_EVC130_from_tc_sip.h")
		{}
	};

	TEST_F(AVer_EVC130_SIP_from_VCS_Test, RunTestBody)
	{
		SetCallInfo(
			"#sip:@192.168.62.89",
			"a@matvey.trueconf.loc",
			"0E3A498B1075989209348A23339D7395",
			Direction::VCS_TO_TERMINAL);
		SetBranchAndTag(
			"4E4AB57F918D6CCEFA9D23305D3D560C",
			"600ADAD95520389F6740B2D24EE65946");

		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {
			if (!IsH264HDVideo(video))
			{
				return false;
			}

			if (audio.CodecType != e_rcvG722_64k)
			{
				return false;
			}

			return true;
		});
		SniffBasedTestBody();
	}

	// !!!! Tandberg GVC3200 !!!!
	class Tandberg_GVC3200_SIP_to_VCS_Test : public SIPSniffBasedTestBase
	{
	public:
		Tandberg_GVC3200_SIP_to_VCS_Test()
			: SIPSniffBasedTestBase(
			{net::address_v4::from_string("192.168.41.79"), 5060, net::protocol::UDP},   // VCS
			{net::address_v4::from_string("192.168.62.77"), 5060, net::protocol::UDP},   // Terminal
			"GVC3200_to_tc_sip.h")
		{}
	};

	TEST_F(Tandberg_GVC3200_SIP_to_VCS_Test, RunTestBody)
	{
		SetCallInfo(
			"#sip:@192.168.41.79",
			"#sip:@192.168.62.77",
			"1980986749-5060-3@BJC.BGI.GC.HH",
			Direction::TERMINAL_TO_VCS);
		SetBranchAndTag(
			"8711726",
			"1135828586");

		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {
			if (!IsH264HDVideo(video))
			{
				return false;
			}

			if (audio.CodecType != e_rcvG722132)
			{
				return false;
			}

			return true;
		});
		SniffBasedTestBody();
	}

	class Tandberg_GVC3200_SIP_from_VCS_Test : public SIPSniffBasedTestBase
	{
	public:
		Tandberg_GVC3200_SIP_from_VCS_Test()
			: SIPSniffBasedTestBase(
			{net::address_v4::from_string("192.168.41.77"), 5060, net::protocol::UDP},   // VCS
			{net::address_v4::from_string("192.168.62.79"), 5060, net::protocol::UDP},   // Terminal
			"GVC3200_from_tc_sip.h")
		{}
	};

	TEST_F(Tandberg_GVC3200_SIP_from_VCS_Test, RunTestBody)
	{
		SetCallInfo(
			"#sip:@192.168.41.77",
			"a@matv001.trueconf.name",
			"97A20F03B8FF332D7C3DCCFD497F6F20",
			Direction::VCS_TO_TERMINAL);
		SetBranchAndTag(
			"F1D6FCABB3E9B70B4211B80EC0B21D93",
			"CA2F13FE4023A3CBAAE95AA1C0A30C85");
		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {
			if (!IsH264HDVideo(video))
			{
				return false;
			}

			if (audio.CodecType != e_rcvG722132)
			{
				return false;
			}

			return true;
		});
		SniffBasedTestBody();
	}

	/// !!!! Sony XG77 !!!!
	class Sony_XG77_SIP_to_VCS_Test : public SIPSniffBasedTestBase
	{
	public:
		Sony_XG77_SIP_to_VCS_Test()
			: SIPSniffBasedTestBase(
			{net::address_v4::from_string("192.168.41.79"), 5060, net::protocol::TCP },  // VCS
			{net::address_v4::from_string("192.168.62.153"), 44301, net::protocol::TCP }, // Terminal
			"sony-xg77_to_tc_sip.h")
		{}
	};

	TEST_F(Sony_XG77_SIP_to_VCS_Test, RunTestBody)
	{
		SetCallInfo(
			"xg77@matvey.trueconf.loc",
			"a@matvey.trueconf.loc",
			"cc4c785e-e014-11d3-8141-421b47e33a7f",
			Direction::TERMINAL_TO_VCS);
		SetBranchAndTag(
			"cc4caf86",
			"cc4c6896");

		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {
			if (!IsH264HDVideo(video))
			{
				return false;
			}

			if (audio.CodecType != e_rcvG722_64k)
			{
				return false;
			}

			return true;
		});
		SniffBasedTestBody();
	}

	class Sony_XG77_SIP_from_VCS_Test : public SIPSniffBasedTestBase
	{
	public:
		Sony_XG77_SIP_from_VCS_Test()
			: SIPSniffBasedTestBase(
			{net::address_v4::from_string("192.168.41.153"), 50471, net::protocol::TCP },   // VCS
			{net::address_v4::from_string("192.168.62.79"),  5060, net::protocol::TCP },    // Terminal
			"sony-xg77_from_tc_sip.h")
		{}
	};

	TEST_F(Sony_XG77_SIP_from_VCS_Test, RunTestBody)
	{
		SetCallInfo(
			"a@matvey.trueconf.loc",
			"xg77@matvey.trueconf.loc",
			"B05D1C8DA349822F6D0C7FFF31B620BD",
			Direction::VCS_TO_TERMINAL);
		SetBranchAndTag(
			"634F630FC6621FA8E6FB06E9350EE7E1",
			"0494A128B0EADC1543F06FD2B13D17EF");
		SetVCSPeerNo(1);
		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {
			if (!IsH264HDVideo(video))
			{
				return false;
			}

			if (audio.CodecType != e_rcvG722_64k)
			{
				return false;
			}

			return true;
		});

		SetPacketToSkip(SIP, 1, 2); /* 148 - [TCP segment of a reassembled PDU] */
		SetPacketToSkip(SIP, 0, 3); /* part of the packet */
		SetPacketToSkip(SIP, 0, 4); /* part of the packet */
		SniffBasedTestBody();
	}

	// Yealink VC400
	class Yealink_VC400_SIP_from_VCS_Test : public SIPSniffBasedTestBase
	{
	public:
		Yealink_VC400_SIP_from_VCS_Test()
			: SIPSniffBasedTestBase(
			{net::address_v4::from_string("192.168.62.79"), 23747, net::protocol::TCP },   // VCS
			{net::address_v4::from_string("192.168.62.48"), 5060, net::protocol::TCP },     // Terminal
			"Yealink_VC400_from_tc_sip.h")
		{}
	};

	TEST_F(Yealink_VC400_SIP_from_VCS_Test, RunTestBody)
	{
		SetCallInfo(
			"a@matvey.trueconf.loc",
			"#sip:@192.168.62.48",
			"2FB2368FA1A7725A572701C76E8998F7",
			Direction::VCS_TO_TERMINAL);
		SetBranchAndTag(
			"A2227C2FE281F7B612D98EDB441881B8",
			"E10CDA670EE11E71897591A07360EFB8");

		SetPacketToSkip(SIP, 0, 0);   /* 175 - [TCP segment of a reassembled PDU] */
		SetPacketToSkip(SIP, 1, 1);   // part of the packet
		SetPacketToSkip(SIP, 1, 2);   // part of the packet
		SetPacketToSkip(SIP, 1, 4);   /* 297 - [TCP segment of a reassembled PDU] */
		SetPacketToSkip(SIP, 0, 3);   /* 829 - [TCP segment of a reassembled PDU] */

		SniffBasedTestBody();
	}

	class Yealink_VC400_SIP_to_VCS_Test : public SIPSniffBasedTestBase
	{
	public:
		Yealink_VC400_SIP_to_VCS_Test()
			: SIPSniffBasedTestBase(
			{net::address_v4::from_string("192.168.62.48"), 5060, net::protocol::TCP },     // VCS
			{net::address_v4::from_string("192.168.62.79"), 50007, net::protocol::TCP },    // Terminal
			"Yealink_VC400_to_tc_sip.h")
		{}
	};

	TEST_F(Yealink_VC400_SIP_to_VCS_Test, RunTestBody)
	{
		SetCallInfo(
			"#sip:vc400@192.168.62.48",
			"a@matvey.trueconf.loc",
			"0_3884661032@192.168.62.48",
			Direction::TERMINAL_TO_VCS);
		SetBranchAndTag(
			"1308202258",
			"3015585994");

		SetPacketToSkip(SIP, 0, 0);   /* 195 - [TCP segment of a reassembled PDU] */
		SetPacketToSkip(SIP, 1, 1);   /* 288 - [TCP segment of a reassembled PDU] */
		SetPacketToSkip(SIP, 0, 3);   /* 293 - [TCP segment of a reassembled PDU] */
		SetPacketToSkip(SIP, 1, 3);   /* 313 - [TCP segment of a reassembled PDU] */

		SniffBasedTestBody();
	}

	// Cisco E20
	class Cisco_E20_SIP_from_VCS_Test : public SIPSniffBasedTestBase
	{
	public:
		Cisco_E20_SIP_from_VCS_Test()
			: SIPSniffBasedTestBase(
			{net::address_v4::from_string("192.168.41.140"), 54357, net::protocol::TCP },   // VCS
			{net::address_v4::from_string("192.168.62.43"), 5060, net::protocol::TCP },     // Terminal
			"Cisco_E20_from_tc_sip.h")
		{}
	};

	TEST_F(Cisco_E20_SIP_from_VCS_Test, RunTestBody)
	{
		SetCallInfo(
			"b@brchk000.trueconf.ua",
			"#sip:@192.168.62.48",
			"A221DA51526868EA72FE9BFFF6E43A42",
			Direction::VCS_TO_TERMINAL);
		SetBranchAndTag(
			"95F196B1054D7B21C4343556DCDFD3D0",
			"4D7EFC6DA94A48D9E5D5D592094BB369");

		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {

			std::uint32_t width = 0, height = 0;
			if (video.CodecType != e_videoH264)
			{
				return false;
			}

			GetH264VideoResolution(video, width, height);
			if (width != 768 || height != 448)
			{
				return false;
			}

			if (audio.CodecType != e_rcvG722132)
				return false;

			return true;
		});

		SetPacketToSkip(SIP, 0, 0);   /* 38 - [TCP segment of a reassembled PDU] */
		SetPacketToSkip(SIP, 1, 1);   /* 49 - [TCP segment of a reassembled PDU] */

		SniffBasedTestBody();
	}

	// !!!! Polycom VSX7000 !!!!
	class Polycom_VSX7000_SIP_From_TC : public SIPSniffBasedTestBase
	{
	public:
		Polycom_VSX7000_SIP_From_TC()
			: SIPSniffBasedTestBase(
			{net::address_v4::from_string("192.168.41.140"), 52922, net::protocol::TCP },   // VCS
			{net::address_v4::from_string("192.168.62.41"), 5060, net::protocol::TCP },     // Terminal
			"Polycom_VSX7000_from_tc_sip.h")
		{}
	};

	TEST_F(Polycom_VSX7000_SIP_From_TC, RunTestBody)
	{
		SetCallInfo(
			"#sip:vsx7000@192.168.62.41",
			"b@brchk000.trueconf.ua",
			"C9D4605756D9CD371E41DBA00253F78B",
			Direction::VCS_TO_TERMINAL);
		SetBranchAndTag(
			"E33318A458145F231D1AB2926221BD94",
			"C4285400FEB840D2CDC4BFBA87906F5B");

		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {

			std::uint32_t width = 0, height = 0;
			if (video.CodecType != e_videoH263plus)
			{
				return false;
			}

			GetH264VideoResolution(video, width, height);
			if (width != 352 || height != 288) // CIF
			{
				return false;
			}

			if (!IsSiren14Audio(audio))
				return false;

			return true;
		});


		SetPacketToSkip(SIP, 0, 0);   /* 155 - [TCP segment of a reassembled PDU] */
		SetPacketToSkip(SIP, 1, 1);   /* 171 - [TCP segment of a reassembled PDU] */

		SniffBasedTestBody();
	}

	// !!!! Huawei MC850 !!!!
	class Huawei_MC850_SIP_From_TC : public SIPSniffBasedTestBase
	{
	public:
		Huawei_MC850_SIP_From_TC()
			: SIPSniffBasedTestBase(
			{net::address_v4::from_string("192.168.41.140"), 5060, net::protocol::UDP},    // VCS
			{net::address_v4::from_string("192.168.62.44"),  5060, net::protocol::UDP},    // Terminal
			"Huawei_MC850_from_tc_sip.h")
		{}
	};

	TEST_F(Huawei_MC850_SIP_From_TC, RunTestBody)
	{
		SetCallInfo(
			"#sip:@192.168.62.44",
			"b@brchk000.trueconf.ua",
			"C8CE75BBAF6CA96CB72242767EB2A24F",
			Direction::VCS_TO_TERMINAL);
		SetBranchAndTag(
			"A28C88AB852CE948D7E572CD64071189",
			"677F80C1108734C399C3BD28049D90C0");

		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {

			std::uint32_t width = 0, height = 0;
			if (video.CodecType != e_videoH264)
			{
				return false;
			}

			GetH264VideoResolution(video, width, height);
			if (width != 640 || height != 360)
			{
				return false;
			}

			// there is no audio mode was negotiated according to sniff
			if (audio.CodecType != e_rcvNone)
				return false;

			return true;
		});

		SetPacketToSkip(SIP, 0, 3); // 323 - resend of frame 155
		SetPacketToSkip(SIP, 1, 4); // 494 - resend of frame 493
		SetPacketToSkip(SIP, 0, 5); // 5276 - resend of frame 5267
		SetPacketToSkip(SIP, 1, 6); // 5282 - resend of frame 5278

		SniffBasedTestBody();
	}

	/// !!!! ViewPoint 9000 (Huawei HD9030) !!!!
	class ViewPoint_9000_SIP_to_VCS_Test : public SIPSniffBasedTestBase
	{
	public:
		ViewPoint_9000_SIP_to_VCS_Test()
			: SIPSniffBasedTestBase(
			{net::address_v4::from_string("192.168.41.140"), 5060, net::protocol::UDP},   // VCS
			{net::address_v4::from_string("192.168.62.46"),  5060, net::protocol::UDP},   // Terminal
			"ViewPoint_HD9000_to_tc_sip.h")
		{}
	};

	TEST_F(ViewPoint_9000_SIP_to_VCS_Test, RunTestBody)
	{
		SetCallInfo(
			"b@brchk000.trueconf.ua",
			"#sip:@192.168.62.46",
			"baegfhfpae4qfeglafb4ahhqc0pb00ap@127.0.0.1",
			Direction::TERMINAL_TO_VCS);
		SetBranchAndTag(
			"jq00dgej5ld04l5jhje0fq2f2",
			"4pla0lcc");

		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {

			if (!IsH264HDVideo(video))
				return false;

			if (audio.CodecType != e_rcvG722_64k)
				return false;

			return true;
		});
		SniffBasedTestBody();
	}

	class ViewPoint_9000_SIP_from_VCS_Test : public SIPSniffBasedTestBase
	{
	public:
		ViewPoint_9000_SIP_from_VCS_Test()
			: SIPSniffBasedTestBase(
			{net::address_v4::from_string("192.168.41.46"), 5060, net::protocol::UDP},   // VCS
			{net::address_v4::from_string("192.168.62.140"), 5060, net::protocol::UDP},  // Terminal
			"ViewPoint_HD9000_from_tc_sip.h")
		{}
	};

	TEST_F(ViewPoint_9000_SIP_from_VCS_Test, RunTestBody)
	{
		SetCallInfo(
			"#sip:@192.168.62.46",
			"b@brchk000.trueconf.ua",
			"D922EA14BE503AD582D1FA2E4F00467A",
			Direction::VCS_TO_TERMINAL);
		SetBranchAndTag(
			"813FA0D66E78E9AF9EE01B7BA9F21B9A",
			"A9DECAD0529778E25409C1E2BA50EC33");

		SetMediaModesValidator([](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {

			if (!IsH264HDVideo(video))
				return false;

			if (audio.CodecType != e_rcvG722_64k)
				return false;

			return true;
		});
		SniffBasedTestBody();
	}

}
#endif