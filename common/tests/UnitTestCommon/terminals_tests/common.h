#pragma once

#include "../../../WiresharkStreamParser/WiresharkStreamParser.h"
#include "../H323ParserTestBase.h"
#include "../SIPParserTestBase.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstdint>

#include <string>
#include <map>
#include <utility>
#include <functional>


namespace terminals_tests
{
	enum Direction
	{
		VCS_TO_TERMINAL,
		TERMINAL_TO_VCS
	};

	typedef std::function<bool(const VS_GatewayAudioMode &audio, const VS_GatewayVideoMode &video)> MediaModesValidator;

	// returns path to the found test data file
	extern std::string GetTestDataFilePath(const char *test_data_filename);
	// returns h225 message type
	extern int GetH225MessageType(const void *data, const size_t size);
	// HD Video is of high priority
	extern void GetH264VideoResolution(const VS_GatewayVideoMode &video, std::uint32_t &width, std::uint32_t &height);
	extern bool IsH264HDVideo(const VS_GatewayVideoMode &video);
	// Siren 14 is of high priority
	extern bool IsSiren14Audio(const VS_GatewayAudioMode &audio);

	// return function which checks if video is H264 HD video and audio is Siren 14 audio.
	MediaModesValidator MakeDefaultMediaModesValidator(void);

	class SkipPacketMixin
	{
	protected:
		SkipPacketMixin(void);
	public:
		void SetPacketToSkip(const int protocol_id, const size_t peer_no, const size_t packet_no);
		bool IsPacketToSkip(const int protocol_id, const size_t peer_no, const size_t packet_no) const;
		void ClearPacketsToSkip(const int protocol_id);
		void ClearAll(void);
	private:
		std::map<int, std::vector<std::pair<size_t, size_t>>> m_data;
	};

	class SniffBasedTestCommon
	{
	public:
		void SetCallInfo(const std::string &to, const std::string &from, const std::string &dialog_id, const Direction call_direction = Direction::VCS_TO_TERMINAL);
		void SetMediaModesValidator(const MediaModesValidator &func);
	protected:
		SniffBasedTestCommon();

		virtual void SniffBasedTestBody() = 0;
	protected:
		std::string m_to;
		std::string m_from;
		std::string m_dialog_id;
		Direction m_direction;
		MediaModesValidator m_media_modes_validator;
	};

	class H323SniffBasedTestBase :
		public SniffBasedTestCommon,
		public H323ParserTestBase,
		public SkipPacketMixin
	{
	protected:
		enum Protocols {
			H225,
			H245
		};
	protected:
		H323SniffBasedTestBase(
			net::address vcsAddr, net::port vcsPort,
			net::address terminalAddr, net::port terminalPort,
			const char *h225DataFileName,
			const char *h245DataFileName
			);

		void SetUp() override;
		void TearDown() override;
		void SniffBasedTestBody() override;
		void SetVCSH245PeerNo(const size_t no);
		void EnableOldMSDBehaviour(const bool enable); // always wai MSD response

		size_t m_vcs_h245_peer_no;

		std::string m_h225_data_file_name;
		std::string m_h245_data_file_name;
		WiresharkStreamParser::StreamPtr m_h225_stream;
		WiresharkStreamParser::StreamPtr m_h245_stream;
		bool m_old_msd_behaviour;
	};

	class SIPSniffBasedTestBase :
		public SniffBasedTestCommon,
		public SIPParserTestBase< ::testing::Test>,
		public SkipPacketMixin
	{
	protected:
		enum Protocols {
			SIP
		};
	protected:
		SIPSniffBasedTestBase(
			net::Endpoint vcs,
			net::Endpoint terminal,
			const char *sip_data_file_name);

		void SetUp() override;
		void TearDown() override;
		void SniffBasedTestBody() override;

		void SetBranchAndTag(const char *branch, const char *tag = "");
		void SetVCSPeerNo(const std::uint32_t no);

		std::uint32_t m_vcs_peer_no;
		std::string m_sip_data_file_name;
		WiresharkStreamParser::StreamPtr m_sip_stream;
		std::string m_branch;
		std::string m_tag;
	};
}
