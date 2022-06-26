#if defined(_WIN32) // Not ported yet

#include "common.h"
#include "../../../tools/H323Gateway/Lib/src/VS_Q931.h"
#include "../../../FakeClient/VS_ConferenceInfo.h"

#include <windows.h>
#include <cstring>
#include <fstream>
#include <iostream>

namespace terminals_tests
{
	static const char *TESTS_DATA_DIRECTRY_NAME = "sniffs_data";

	static bool ProbeFile(const char *path)
	{
		bool status = false;
		std::ifstream file(path, std::ios::binary);
		status = file.good();
		file.close();

		return status;
	}

	std::string GetTestDataFilePath(const char *test_data_filename)
	{
		std::string executable_directory;
		std::string test_data_file_path;
		char exec_path[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, exec_path, sizeof(exec_path));

		{
			char* p = strrchr(exec_path, '\\');
			if (p == NULL)
				return "";

			executable_directory.assign(exec_path, p + 1);
		}

		{
			test_data_file_path = executable_directory + "../tests/UnitTestCommon/" + TESTS_DATA_DIRECTRY_NAME + '/' + test_data_filename;
			if (ProbeFile(test_data_file_path.c_str()))
			{
				return test_data_file_path;
			}

			test_data_file_path = executable_directory + "../common/tests/UnitTestCommon/" + TESTS_DATA_DIRECTRY_NAME + '/' + test_data_filename;
			if (ProbeFile(test_data_file_path.c_str()))
			{
				return test_data_file_path;
			}
		}

		return  "";
	}

	int GetH225MessageType(const void *data, const size_t size)
	{
		VS_TearMessageQueue queue;

		unsigned char* in_buff = new unsigned char[size];
		memcpy(in_buff, data, size);
		queue.PutTearMessage((unsigned char *)in_buff, size);
		if (!queue.IsExistEntireMessage())
		{
			return -1;
		}

		std::unique_ptr<unsigned char[]> ent_mess;
		uint32_t ent_mess_sz = 0;
		bool got_message = false;

		while (!got_message)
		{
			got_message = queue.GetEntireMessage(ent_mess.get(), ent_mess_sz);
			if (!got_message && ent_mess == nullptr)
			{
				ent_mess.reset(new unsigned char[ent_mess_sz]);
			}
			else if (ent_mess != nullptr && got_message != true)
			{
				return -1;
			}
		}

		VS_PerBuffer in_per_buff(ent_mess.get(), ent_mess_sz * 8);
		VS_Q931 theQ931_In;

		if (!theQ931_In.DecodeMHeader(*static_cast<VS_BitBuffer*>(&in_per_buff)))
		{
			return -1;
		}

		return theQ931_In.messageType;
	}

	void GetH264VideoResolution(const VS_GatewayVideoMode &video, std::uint32_t &width, std::uint32_t &height)
	{
		if (video.MaxFs != 0)
		{
			VS_H323BeginH245Modes::GetH264ResolutionByMaxFs(video.MaxFs, width, height);
		}
		else
		{
			VS_H323BeginH245Modes::GetH264ResolutionByLevel(video.Mode, width, height);
		}
	}

	// HD Video is of high priority
	bool IsH264HDVideo(const VS_GatewayVideoMode &video)
	{
		std::uint32_t width = 0, height = 0;
		if (video.CodecType != e_videoH264)
		{
			return false;
		}

		// get resolution
		GetH264VideoResolution(video, width, height);
		if (width < 1280 || height < 720)
		{
			return false;
		}
		return true;
	}

	// Siren 14 is of high priority
	bool IsSiren14Audio(const VS_GatewayAudioMode &audio)
	{
		if (audio.CodecType != e_rcvSIREN14_24
			&& audio.CodecType != e_rcvSIREN14_32
			&& audio.CodecType != e_rcvSIREN14_48)
		{
			return false;
		}

		return true;
	}


	MediaModesValidator MakeDefaultMediaModesValidator(void)
	{
		return [](const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video) -> bool {
			return IsH264HDVideo(video) && IsSiren14Audio(audio);
		};
	}

	// SkipPacketMixin
	SkipPacketMixin::SkipPacketMixin(void)
	{}

	bool SkipPacketMixin::IsPacketToSkip(const int protocol_id, const size_t peer_no, const size_t packet_no) const
	{
		auto protocol_data = m_data.find(protocol_id);

		if (protocol_data == m_data.end())
			return false;

		for (const auto &v : protocol_data->second)
		{
			if (v.first == peer_no && v.second == packet_no)
			{
				return true;
			}
		}

		return false;
	}

	void SkipPacketMixin::ClearPacketsToSkip(const int protocol_id)
	{
		auto protocol_data = m_data.find(protocol_id);

		if (protocol_data == m_data.end())
			return;

		protocol_data->second.clear();
	}

	void SkipPacketMixin::SetPacketToSkip(const int protocol_id, const size_t peer_no, const size_t packet_no)
	{
		if (IsPacketToSkip(protocol_id, peer_no, packet_no))
			return;

		std::pair<size_t, size_t> new_data(peer_no, packet_no);
		auto protocol_data = m_data.find(protocol_id);
		if (protocol_data == m_data.end())
		{
			std::vector<std::pair<size_t, size_t>> new_protocol_data;
			new_protocol_data.push_back(new_data);
			m_data[protocol_id] = new_protocol_data;
			return;
		}

		protocol_data->second.push_back(new_data);
	}

	void SkipPacketMixin::ClearAll(void)
	{
		m_data.clear();
	}

	// Common Sniff Based Test Class

	SniffBasedTestCommon::SniffBasedTestCommon()
		: m_direction(Direction::VCS_TO_TERMINAL),
		m_media_modes_validator(MakeDefaultMediaModesValidator())
	{
	}

	void SniffBasedTestCommon::SetCallInfo(const std::string &to, const std::string &from, const std::string &dialog_id, const Direction call_direction)
	{
		m_to = to;
		m_from = from;
		m_dialog_id = dialog_id;
		m_direction = call_direction;
	}

	void SniffBasedTestCommon::SetMediaModesValidator(const std::function<bool(const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video)> &func)
	{
		m_media_modes_validator = func;
	}


	// H323 Sniff Based test Class
	H323SniffBasedTestBase::H323SniffBasedTestBase(
		net::address vcsAddr, net::port vcsPort,
		net::address terminalAddr, net::port terminalPort,
		const char *h225DataFileName,
		const char *h245DataFileName
		)
		: SniffBasedTestCommon(),
		H323ParserTestBase(vcsAddr, vcsPort, terminalAddr, terminalPort),
		m_vcs_h245_peer_no(1),
		m_h225_data_file_name(h225DataFileName),
		m_h245_data_file_name(h245DataFileName),
		m_old_msd_behaviour(true)
	{}

	void H323SniffBasedTestBase::SetUp()
	{
		H323ParserTestBase::SetUp();
		WiresharkStreamParser stream_parser;
		// read h225 data
		{
			std::string error_message;
			std::ifstream input_data(GetTestDataFilePath(m_h225_data_file_name.c_str()), std::ios::binary);
			EXPECT_TRUE(stream_parser.Parse(input_data, error_message)) << "Can\'t load test data from " << m_h225_data_file_name << ": " << error_message << std::endl;
			m_h225_stream = stream_parser.GetStream();
			input_data.close();
		}

		// read h245 data
		{
			std::string error_message;
			std::ifstream input_data(GetTestDataFilePath(m_h245_data_file_name.c_str()), std::ios::binary);
			EXPECT_TRUE(stream_parser.Parse(input_data, error_message)) << "Can\'t load test data from " << m_h245_data_file_name << ": " << error_message << std::endl;
			m_h245_stream = stream_parser.GetStream();
			input_data.close();
		}

		stream_parser.ResetStream();
	}

	void H323SniffBasedTestBase::TearDown()
	{
		H323ParserTestBase::TearDown();
	}

	void H323SniffBasedTestBase::SniffBasedTestBody()
	{
		using ::testing::_;
		using ::testing::AtLeast;

		m_to = m_to.empty() ? DEFAULT_DESTINATION_CALLID_H323 : m_to;

		ASSERT_NE(m_h225_stream, nullptr) << "Failed to load H225 stream data." << std::endl;
		ASSERT_NE(m_h245_stream, nullptr) << "Failed to load H245 stream data." << std::endl;
		ASSERT_LT(m_vcs_h245_peer_no, m_h245_stream->peers.size()) << "Bad VCS H245 Peer number." << std::endl;

		char send_buf[0xffff] = { 0 };
		std::size_t send_buf_sz = sizeof(send_buf);

		size_t vcs_h225_peer_no;

		size_t next_h225_packet_index = 0;
		size_t next_h245_packet_index = 0;

		VS_GatewayAudioMode audio_mode;
		VS_GatewayVideoMode video_mode;

		// set call data
		std::string dialog = h323->SetNewDialogTest(m_dialog_id, m_to, {}, VS_CallConfig{});
		ASSERT_TRUE(!dialog.empty());

		// Set base logical channel number as used by older versions of TCS
		{
			auto ctx = h323->GetParserContext(dialog);
			ctx->SetBaseLCNumber();
		}

		auto get_media_modes = [&](void) -> bool
		{
			return h323->GetAudioMode(dialog, audio_mode) && audio_mode.PayloadType != SDP_PT_INVALID && audio_mode.
				CodecType != e_rcvNone && h323->GetVideoMode(dialog, video_mode) && video_mode.PayloadType !=
				SDP_PT_INVALID && video_mode.CodecType != e_rcvNone;
		};


		h323->SetPeerCSAddress(dialog, terminal_addr);
		h323->SetMyCsAddress(vcs_addr);
		auto media_address = net::address::from_string("1.2.3.4");
		h323->SetMyMediaAddress(media_address);
		//conf_protocol_fake.SetOurAddr(vcs_addr);
		//conf_protocol_fake.SetRemoteAddr(terminal_addr);

		h245_channel_fake->remote_addr = terminal_addr.addr;
		h245_channel_fake->remote_port = 12345;
		EXPECT_CALL(*h245_channel, Open(_, _, _, _, _, _)).Times(AtLeast(1));
		EXPECT_CALL(*h245_channel, Send_mocked(_)).Times(AtLeast(1));


		if (m_direction == VCS_TO_TERMINAL)
		{
			vcs_h225_peer_no = 0;
			ASSERT_TRUE(h323->InviteMethod(dialog, m_from.c_str(), m_to.c_str(), VS_ConferenceInfo(false,false), "dn")) << "Invite method failed." << std::endl;
		}
		else
		{
			vcs_h225_peer_no = 1;
			EXPECT_CALL(*conf_protocol, InviteMethod(::testing::An<string_view>(), _, _, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(::testing::Return(true));
			ASSERT_TRUE(h323->InviteReplay(dialog, e_call_ok, false)) << "Invite reply failed." << std::endl;
		}

		// h225 processing
		for (const auto &packet : m_h225_stream->packets)
		{
			if (!IsPacketToSkip(H225, packet->peer_no, packet->peer_packet_no))
			{
				if (packet->data.size() > 4) // empty TPKT packets
				{
					int message_type = GetH225MessageType(&packet->data[0], packet->data.size());
					ASSERT_GE(message_type, 0);

					// process messages until release complete
					if (message_type == VS_Q931::e_releaseCompleteMsg) // stop h225 stream processing, go to h245 data
					{
						break;
					}
				}

				if (packet->peer_no != vcs_h225_peer_no) // packet was send by terminal
				{
					ASSERT_GT(h323->SetRecvBuf(&packet->data[0], packet->data.size(), e_H225, terminal_addr.addr,
						terminal_addr.port, vcs_addr.addr, vcs_addr.port), 0);
				}
				else // packet was send by VCS
				{
					ASSERT_GT(h323->GetBufForSend(send_buf, send_buf_sz, e_H225, terminal_addr.addr, terminal_addr.port,
						vcs_addr.addr, vcs_addr.port), 0);
					ASSERT_NE(send_buf_sz, 0) << "Can\'t send h225 data!" << std::endl;
					send_buf_sz = sizeof(send_buf);
				}
			}

			next_h225_packet_index++;
		}

		auto base_ctx = h323->GetParserContextByDialogID(h323, dialog);
		ASSERT_NE(base_ctx, nullptr);
		auto ctx = static_cast<VS_H323ParserInfo *>(base_ctx.get());
		ASSERT_NE(base_ctx, nullptr);

		// h245 processing
		// TODO (?): stop when media parameters are initialized
		bool got_media_modes = false;
		VS_H245LogicalChannelInfo video_channel_info;
		VS_H245LogicalChannelInfo audio_channel_info;
		bool got_channels_info = false;
		auto get_channels_info = [&]() -> bool {
			return ctx->GetH245LogicalChannel(e_video, false, video_channel_info) &&
				ctx->GetH245LogicalChannel(e_audio, false, audio_channel_info);
		};

		for (const auto &packet : m_h245_stream->packets)
		{
			// In the older version of TCS (the ones which do not implement Master-Slave decision properly)
			// TCS expected MSDA message. There is a specific mode for this behaviour.
			if (m_old_msd_behaviour)
			{
				auto ctx = h323->GetParserContext(dialog);
				if (ctx)
				{
					ctx->SetMsdState(MSDOutgoingAwaitingResponse);
				}
			}
			if (!IsPacketToSkip(H245, packet->peer_no, packet->peer_packet_no))
			{
				if (packet->peer_no != m_vcs_h245_peer_no) // packet was sent by terminal
				{
					h245_channel_fake->CallDataReceived(&packet->data[0], packet->data.size());
				}
				else // packet was generated by VCS
				{
					ASSERT_GT((int)h245_channel_fake->out_queue.size(), 0);
					vs::SharedBuffer send_buf2(send_buf_sz);
					std::memcpy(send_buf2.data(), send_buf, send_buf2.size());
					h245_channel_fake->Send(std::move(send_buf2));
					ASSERT_NE(send_buf_sz, 0) << "Can\'t send h245 data!" << std::endl;
					send_buf_sz = sizeof(send_buf);
				}

				// try to get media modes
				if (!got_media_modes)
				{
					got_media_modes = get_media_modes();
				}

				// try to get media channels info
				if (!got_channels_info)
				{
					got_channels_info = get_channels_info();
				}
			}

			next_h245_packet_index++;
		}

		ASSERT_TRUE(got_media_modes) << "Can\'t get media modes!" << std::endl;
		ASSERT_TRUE(got_channels_info) << "Can\'t get media channels info!" << std::endl;

		if (m_media_modes_validator)
		{
			ASSERT_TRUE(m_media_modes_validator(audio_mode, video_mode)) << "Media modes validation failed!" << std::endl;
		}

		// TODO: validate media channels data
		for (auto &chan : conf_protocol_fake.last_media_channels)
		{
			if (chan.content == SDP_CONTENT_MAIN && chan.type == SDPMediaType::video)
			{
				ASSERT_EQ(video_mode.CodecType, chan.snd_mode_video.CodecType)
					<< "Video codec value in media mode differs from the one found in a corresponding channel." << std::endl;
				ASSERT_EQ(video_mode.PayloadType, chan.snd_mode_video.PayloadType)
					<< "Video codec payload type in media mode differs from the one found in a corresponding channel." << std::endl;

				boost::asio::ip::udp::endpoint video_rtp_media_address(media_address, chan.our_rtp_address.port());
				boost::asio::ip::udp::endpoint video_rtcp_media_address(media_address, chan.our_rtcp_address.port());

				ASSERT_EQ(video_rtp_media_address, ctx->video_channel.our_rtp_address)
					<< "Video RTP address differs from the one found in a corresponding channel." << std::endl;
				ASSERT_EQ(video_rtcp_media_address, ctx->video_channel.our_rtcp_address)
					<< "Video RTCP address differs from the one found in a corresponding channel." << std::endl;

				ASSERT_EQ(video_rtp_media_address, video_channel_info.m_our_rtp_address)
					<< "Video RTP address differs from the one found in a corresponding channel info." << std::endl;
				ASSERT_EQ(video_rtcp_media_address, video_channel_info.m_our_rtcp_address)
					<< "Video RTCP address differs from the one found in a corresponding channel info." << std::endl;
			}
			else if (chan.content == SDP_CONTENT_MAIN && chan.type == SDPMediaType::audio)
			{
				ASSERT_EQ(audio_mode.CodecType, chan.snd_mode_audio.CodecType)
					<< "Audio codec value in media mode differs from the one found in a corresponding channel." << std::endl;
				ASSERT_EQ(audio_mode.PayloadType, chan.snd_mode_audio.PayloadType)
					<< "Audio codec payload type in media mode differs from the one found in a corresponding channel." << std::endl;

				boost::asio::ip::udp::endpoint audio_rtp_media_address(media_address, chan.our_rtp_address.port());
				boost::asio::ip::udp::endpoint audio_rtcp_media_address(media_address, chan.our_rtcp_address.port());

				ASSERT_EQ(audio_rtp_media_address, ctx->audio_channel.our_rtp_address)
					<< "Audio RTP address differs from the one found in a corresponding channel." << std::endl;
				ASSERT_EQ(audio_rtcp_media_address, ctx->audio_channel.our_rtcp_address)
					<< "Audio RTCP address differs from the one found in a corresponding channel." << std::endl;

				ASSERT_EQ(audio_rtp_media_address, audio_channel_info.m_our_rtp_address)
					<< "Audio RTP address differs from the one found in a corresponding channel info." << std::endl;
				ASSERT_EQ(audio_rtcp_media_address, audio_channel_info.m_our_rtcp_address)
					<< "Audio RTCP address differs from the one found in a corresponding channel info." << std::endl;
			}
			else
			{
				continue;
			}

			// RTP should be lesser than RTCP
			ASSERT_LT(chan.our_rtp_address.port(), chan.our_rtcp_address.port());

			ASSERT_EQ(chan.our_rtp_address.address(), vcs_addr.addr)
				<< "Local and given VCS RTP IP addresses are different!" << std::endl;
			ASSERT_EQ(chan.our_rtcp_address.address(), vcs_addr.addr)
				<< "Local and given VCS RTCP IP addresses are different!" << std::endl;

			ASSERT_EQ(chan.remote_rtp_address.address(), terminal_addr.addr)
				<< "Remote and given terminal RTP IP addresses are different!" << std::endl;
			ASSERT_EQ(chan.remote_rtcp_address.address(), terminal_addr.addr)
				<< "Remote and given terminal RTCP IP addresses are different!" << std::endl;
			/*VS_SimpleStr addr;
			chan.our_rtp_address.GetAddrString(addr);
			printf("Our RTP addr: %s\n", addr.m_str);
			chan.our_rtcp_address.GetAddrString(addr);
			printf("Our RTCP addr: %s\n", addr.m_str);*/
		}

		//printf("");
	}

	void H323SniffBasedTestBase::SetVCSH245PeerNo(const size_t no)
	{
		m_vcs_h245_peer_no = no;
	}

	void H323SniffBasedTestBase::EnableOldMSDBehaviour(const bool enable)
	{
		m_old_msd_behaviour = enable;
	}

	/*!!! SIP !!!*/

	SIPSniffBasedTestBase::SIPSniffBasedTestBase(
		net::Endpoint vcs,
		net::Endpoint terminal,
		const char *sip_data_file_name)
		: SniffBasedTestCommon(),
		SIPParserTestBase< ::testing::Test>(std::move(vcs), std::move(terminal)),
		m_vcs_peer_no(-1),
		m_sip_data_file_name(sip_data_file_name)
	{
	}

	void SIPSniffBasedTestBase::SetUp()
	{
		WiresharkStreamParser stream_parser;
		SIPParserTestBase<testing::Test>::SetUp();
		// read SIP data
		{
			std::string error_message;
			std::ifstream input_data(GetTestDataFilePath(m_sip_data_file_name.c_str()), std::ios::binary);
			EXPECT_TRUE(stream_parser.Parse(input_data, error_message)) << "Can\'t load test data from " << m_sip_data_file_name << ": " << error_message << std::endl;
			m_sip_stream = stream_parser.GetStream();
			input_data.close();
		}
	}

	void SIPSniffBasedTestBase::TearDown()
	{
		SIPParserTestBase<testing::Test>::TearDown();
	}

	void SIPSniffBasedTestBase::SetBranchAndTag(const char *branch, const char *tag)
	{
		if (branch != nullptr)
			m_branch = branch;

		if (tag != nullptr)
			m_tag = tag;
	}

	void SIPSniffBasedTestBase::SetVCSPeerNo(const std::uint32_t no)
	{
		m_vcs_peer_no = no;
	}

	void SIPSniffBasedTestBase::SniffBasedTestBody()
	{
		using ::testing::_;
		using ::testing::AtLeast;
		size_t vcs_peer_no;
		VS_GatewayAudioMode audio_mode;
		VS_GatewayVideoMode video_mode;

		char send_buf[0xffff] = { 0 };
		std::size_t send_buf_sz = sizeof(send_buf);

		m_to = m_to.empty() ? DEFAULT_DESTINATION_CALLID_SIP : m_to;

		EXPECT_CALL(*confProtocol, SetMediaChannels(_, _, _, _))
			.Times(AtLeast(1));

		ASSERT_NE(m_sip_stream, nullptr) << "Failed to load SIP stream data." << std::endl;

		// set call data
		auto &&dialog = sip->SetNewDialogTest(m_dialog_id, m_to, {}, VS_CallConfig{}, {});

		auto context_base = sip->GetParserContextByDialogID(sip, dialog);
		ASSERT_NE(context_base, nullptr) << "Cannot get context!" << std::endl;
		auto *ctx = static_cast<VS_SIPParserInfo *>(context_base.get());

		// set config values
		auto &&config = ctx->GetConfig();
		{
			if (terminal_addr.protocol == net::protocol::any)
				terminal_addr.protocol = net::protocol::TCP;

			config.Address = { terminal_addr.addr, terminal_addr.port, terminal_addr.protocol };
			config.HostName = terminal_addr.addr.to_string();
		}

		// set addresses
		sip->SetPeerCSAddress(dialog, terminal_addr);
		sip->SetMyCsAddress(vcs_addr);
		confProtocol_fake.SetOurAddr({ vcs_addr });
		confProtocol_fake.SetRemoteAddr({terminal_addr});

		if (m_direction == VCS_TO_TERMINAL)
		{
			ctx->SetSIPRemoteTarget(m_to);
			ctx->SetAliasMy(m_from);
			ctx->SetAliasRemote(m_to);

			vcs_peer_no = 0;
			if (!m_branch.empty())
				ctx->MyBranch(m_branch);

			if (!m_tag.empty())
				ctx->SetTagMy(m_tag);

			ASSERT_TRUE(sip->InviteMethod(dialog, m_from, m_to, VS_ConferenceInfo(false,false), "dn")) << "Invite method failed." << std::endl;
		}
		else
		{
			ctx->SetSIPRemoteTarget(m_from);
			ctx->SetAliasMy(m_to);
			ctx->SetAliasRemote(m_from);

			vcs_peer_no = 1;
		}

		if (m_vcs_peer_no != -1)
		{
			vcs_peer_no = m_vcs_peer_no;
		}

		// handle SIP traffic
		sip->Timeout();
		bool was_bye = false;
		for (const auto &packet : m_sip_stream->packets)
		{
			if (!IsPacketToSkip(SIP, packet->peer_no, packet->peer_packet_no))
			{
				if (ctx->GetMessageType() == TYPE_BYE || strncmp("BYE", &packet->data[0], 3) == 0)
				{
					was_bye = true;
				}

				//printf("=== packet %lu_%lu:\n%.*s\n===", packet->peer_no, packet->peer_packet_no, packet->data.size(), &packet->data[0]);
				if (packet->peer_no != vcs_peer_no) // packet was send by terminal
				{
					// Stop - we've got BYE message, context was destroyed.
					if (was_bye && sip->GetParserContextByDialogID(sip, dialog) == nullptr)
					{
						break;
					}
					ASSERT_TRUE(SetRecvBuf(&packet->data[0], packet->data.size()));
					//sip->Timeout();
				}
				else // packet was sent by VCS
				{
					if (m_direction == TERMINAL_TO_VCS && ctx->GetMessageType() == TYPE_INVITE)
					{
						ASSERT_TRUE(sip->InviteReplay(dialog, e_call_ok, false)) << "Invite reply failed." << std::endl;
					}

					if (m_direction == VCS_TO_TERMINAL && strncmp("BYE", &packet->data[0], 3) == 0)
					{
						sip->Hangup(dialog);
					}

					auto msg = GetMessageFromParser(sip);
					bool info_message = false;
					if (m_direction == TERMINAL_TO_VCS)
					{
						info_message = ctx->GetMessageType() == TYPE_INFO;
					}
					else
					{
						auto res = strncmp("INFO", &packet->data[0], 4);
						info_message =  res == 0;
					}

					// skip some INFO messages
					if (msg == nullptr && info_message)
					{
						continue;
					}
					ASSERT_NE(msg, nullptr) << "Can\'t send SIP data!" << std::endl;
				}
			}
		}

		ASSERT_NE(confProtocol_fake.last_media_channels.size(), 0) << "Media channels are not initialized!" << std::endl;

		auto audio_channel_it = std::find_if(confProtocol_fake.last_media_channels.begin(), confProtocol_fake.last_media_channels.end(), [](const VS_MediaChannelInfo& x) {
			return x.type == SDPMediaType::audio && x.content == SDP_CONTENT_MAIN;
		});

		//ASSERT_NE(audio_channel_it, confProtocol_fake.last_media_channels.end());
		if (audio_channel_it != confProtocol_fake.last_media_channels.end())
		{
			audio_mode = audio_channel_it->snd_mode_audio;
		}

		auto video_channel_it = std::find_if(confProtocol_fake.last_media_channels.begin(), confProtocol_fake.last_media_channels.end(), [](const VS_MediaChannelInfo& x) {
			return x.type == SDPMediaType::video && x.content == SDP_CONTENT_MAIN;
		});

		//ASSERT_NE(video_channel_it, confProtocol_fake.last_media_channels.end());
		if (video_channel_it != confProtocol_fake.last_media_channels.end())
		{
			video_mode = video_channel_it->snd_mode_video;
		}

		ASSERT_FALSE(audio_channel_it == confProtocol_fake.last_media_channels.end() &&
			video_channel_it == confProtocol_fake.last_media_channels.end())
			<< "Media modes are not initialized!" << std::endl;

		// validate media modes
		if (m_media_modes_validator)
		{
			ASSERT_TRUE(m_media_modes_validator(audio_mode, video_mode)) << "Media modes validation failed!" << std::endl;
		}

		for (auto &chan : confProtocol_fake.last_media_channels)
		{
			if (chan.content == SDP_CONTENT_MAIN && chan.type == SDPMediaType::video)
			{
				ASSERT_EQ(video_mode.CodecType, chan.snd_mode_video.CodecType)
					<< "Video codec value in media mode differs from the one found in a corresponding channel." << std::endl;
				ASSERT_EQ(video_mode.PayloadType, chan.snd_mode_video.PayloadType)
					<< "Video codec payload type in media mode differs from the one found in a corresponding channel." << std::endl;			}
			else if (chan.content == SDP_CONTENT_MAIN && chan.type == SDPMediaType::audio)
			{
				ASSERT_EQ(audio_mode.CodecType, chan.snd_mode_audio.CodecType)
					<< "Audio codec value in media mode differs from the one found in a corresponding channel." << std::endl;
				ASSERT_EQ(audio_mode.PayloadType, chan.snd_mode_audio.PayloadType)
					<< "Audio codec payload type in media mode differs from the one found in a corresponding channel." << std::endl;
			}
			else
			{
				continue;
			}

			// RTP should be lesser than RTCP
			ASSERT_LT(chan.our_rtp_address.port(), chan.our_rtcp_address.port());

			ASSERT_EQ(chan.our_rtp_address.address(), vcs_addr.addr)
				<< "Local and given VCS RTP IP addresses are different!" << std::endl;
			ASSERT_EQ(chan.our_rtcp_address.address(), vcs_addr.addr)
				<< "Local and given VCS RTCP IP addresses are different!" << std::endl;

			ASSERT_EQ(chan.remote_rtp_address.address(), terminal_addr.addr)
				<< "Remote and given terminal RTP IP addresses are different!" << std::endl;
			ASSERT_EQ(chan.remote_rtcp_address.address(), terminal_addr.addr)
				<< "Remote and given terminal RTCP IP addresses are different!" << std::endl;
		}
	}
}

#endif
