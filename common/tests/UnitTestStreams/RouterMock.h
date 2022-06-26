#include "tests/common/GMockOverride.h"
#include "streams/Router/Router.h"
#include "streams/Router/Types.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace streams_test {

class RouterMock : public stream::Router
{
public:
	RouterMock()
	{
	}

	void DelegateTo(Router* impl)
	{
		using ::testing::_;
		using ::testing::Invoke;

		ON_CALL(*this, Stop()).WillByDefault(Invoke(impl, &stream::Router::Stop));
		ON_CALL(*this, AddConferencesCondition(_)).WillByDefault(Invoke(impl, &stream::Router::AddConferencesCondition));
		ON_CALL(*this, RemoveConferencesCondition(_)).WillByDefault(Invoke(impl, &stream::Router::RemoveConferencesCondition));
		ON_CALL(*this, GetStatistics(_)).WillByDefault(Invoke(impl, &stream::Router::GetStatistics));
		ON_CALL(*this, CreateConference(_,_,_,_,_,_)).WillByDefault(Invoke(impl, &stream::Router::CreateConference));
		ON_CALL(*this, RemoveConference(_)).WillByDefault(Invoke(impl, &stream::Router::RemoveConference));
		ON_CALL(*this, ConnectToFrameSink(_,_)).WillByDefault(Invoke(impl, &stream::Router::ConnectToFrameSink));
		ON_CALL(*this, AddParticipant(_,_,_,_,_,_,_,_)).WillByDefault(Invoke(impl, static_cast<bool (stream::Router::*)(string_view, string_view, string_view, stream::Buffer*, bool, std::chrono::steady_clock::duration, const stream::Track*, unsigned)>(&stream::Router::AddParticipant)));
		ON_CALL(*this, AddParticipant(_,_,_,_,_)).WillByDefault(Invoke(impl, static_cast<bool (stream::Router::*)(string_view, string_view, stream::Buffer*, bool, std::chrono::steady_clock::duration)>(&stream::Router::AddParticipant)));
		ON_CALL(*this, ConnectParticipant(_,_,_,_,_)).WillByDefault(Invoke(impl, &stream::Router::ConnectParticipant));
		ON_CALL(*this, ConnectParticipantSender(_,_,_,_,_)).WillByDefault(Invoke(impl, &stream::Router::ConnectParticipantSender));
		ON_CALL(*this, ConnectParticipantReceiver(_,_,_,_,_)).WillByDefault(Invoke(impl, &stream::Router::ConnectParticipantReceiver));
		ON_CALL(*this, DisconnectParticipant(_,_)).WillByDefault(Invoke(impl, static_cast<bool (stream::Router::*)(string_view, string_view)>(&stream::Router::DisconnectParticipant)));
		ON_CALL(*this, DisconnectParticipant(_,_,_)).WillByDefault(Invoke(impl, static_cast<bool (stream::Router::*)(string_view, string_view, string_view)>(&stream::Router::DisconnectParticipant)));
		ON_CALL(*this, DisconnectParticipantSender(_,_)).WillByDefault(Invoke(impl, static_cast<bool (stream::Router::*)(string_view, string_view)>(&stream::Router::DisconnectParticipantSender)));
		ON_CALL(*this, DisconnectParticipantSender(_,_,_)).WillByDefault(Invoke(impl, static_cast<bool (stream::Router::*)(string_view, string_view, string_view)>(&stream::Router::DisconnectParticipantSender)));
		ON_CALL(*this, DisconnectParticipantReceiver(_,_)).WillByDefault(Invoke(impl, static_cast<bool (stream::Router::*)(string_view, string_view)>(&stream::Router::DisconnectParticipantReceiver)));
		ON_CALL(*this, DisconnectParticipantReceiver(_,_,_)).WillByDefault(Invoke(impl, static_cast<bool (stream::Router::*)(string_view, string_view, string_view)>(&stream::Router::DisconnectParticipantReceiver)));
		ON_CALL(*this, SetParticipantTracks(_,_,_,_,_)).WillByDefault(Invoke(impl, &stream::Router::SetParticipantTracks));
		ON_CALL(*this, SetParticipantSenderTracks(_,_,_,_,_)).WillByDefault(Invoke(impl, &stream::Router::SetParticipantSenderTracks));
		ON_CALL(*this, SetParticipantReceiverTracks(_,_,_,_,_)).WillByDefault(Invoke(impl, &stream::Router::SetParticipantReceiverTracks));
		ON_CALL(*this, SetParticipantCaps(_,_,_,_)).WillByDefault(Invoke(impl, &stream::Router::SetParticipantCaps));
		ON_CALL(*this, SetParticipantSystemLoad_mocked(_)).WillByDefault(Invoke([impl](std::vector<stream::ParticipantLoadInfo>& load) {
			return impl->SetParticipantSystemLoad(std::move(load));
		}));
		ON_CALL(*this, SetParticipantFrameSizeMB_mocked(_,_,_)).WillByDefault(Invoke([impl](const char* conference_name, const char* participant_name_to, std::vector<stream::ParticipantFrameSizeInfo>& mb) {
			return impl->SetParticipantFrameSizeMB(conference_name, participant_name_to, std::move(mb));
		}));
	}

	MOCK_METHOD0_OVERRIDE(Stop, void());
	MOCK_METHOD1_OVERRIDE(AddConferencesCondition, bool(stream::ConferencesConditions* ccs));
	MOCK_METHOD1_OVERRIDE(RemoveConferencesCondition, void(stream::ConferencesConditions* ccs));
	MOCK_METHOD1_OVERRIDE(GetStatistics, bool(stream::RouterStatistics& stat));
	MOCK_METHOD6_OVERRIDE(CreateConference, bool(string_view conference_name, VS_Conference_Type type, const char* ssl_key, unsigned max_participants, bool write_logs, std::chrono::steady_clock::duration max_silence));
	MOCK_METHOD1_OVERRIDE(RemoveConference, void(string_view conference_name));
	MOCK_METHOD2_OVERRIDE(ConnectToFrameSink, boost::signals2::connection(string_view conference_name, const stream::FrameReceivedSignalType::slot_type& slot));
	MOCK_METHOD8_OVERRIDE(AddParticipant, bool(string_view conference_name, string_view participant_name, string_view connected_participant_name, stream::Buffer* snd_buffer, bool write_logs, std::chrono::steady_clock::duration max_silence, const stream::Track* tracks, unsigned n_tracks));
	MOCK_METHOD5_OVERRIDE(AddParticipant, bool(string_view conference_name, string_view participant_name, stream::Buffer* snd_buffer, bool write_logs, std::chrono::steady_clock::duration max_silence));
	MOCK_METHOD2_OVERRIDE(RemoveParticipant, void(string_view conference_name, string_view participant_name));
	MOCK_METHOD5_OVERRIDE(ConnectParticipant, bool(string_view conference_name, string_view participant_name, string_view connected_participant_name, const stream::Track* tracks, unsigned n_tracks));
	MOCK_METHOD5_OVERRIDE(ConnectParticipantSender, bool(string_view conference_name, string_view participant_name, string_view connected_participant_name, const stream::Track* tracks, unsigned n_tracks));
	MOCK_METHOD5_OVERRIDE(ConnectParticipantReceiver, bool(string_view conference_name, string_view participant_name, string_view connected_participant_name, const stream::Track* tracks, unsigned n_tracks));
	MOCK_METHOD2_OVERRIDE(DisconnectParticipant, bool(string_view conference_name, string_view participant_name));
	MOCK_METHOD3_OVERRIDE(DisconnectParticipant, bool(string_view conference_name, string_view participant_name, string_view connected_participant_name));
	MOCK_METHOD2_OVERRIDE(DisconnectParticipantSender, bool(string_view conference_name, string_view participant_name));
	MOCK_METHOD3_OVERRIDE(DisconnectParticipantSender, bool(string_view conference_name, string_view participant_name, string_view connected_participant_name));
	MOCK_METHOD2_OVERRIDE(DisconnectParticipantReceiver, bool(string_view conference_name, string_view participant_name));
	MOCK_METHOD3_OVERRIDE(DisconnectParticipantReceiver, bool(string_view conference_name, string_view participant_name, string_view connected_participant_name));
	MOCK_METHOD5_OVERRIDE(SetParticipantTracks, bool(string_view conference_name, string_view participant_name, string_view connected_participant_name, const stream::Track* tracks, unsigned n_tracks));
	MOCK_METHOD5_OVERRIDE(SetParticipantSenderTracks, bool(string_view conference_name, string_view participant_name, string_view connected_participant_name, const stream::Track* tracks, unsigned n_tracks));
	MOCK_METHOD5_OVERRIDE(SetParticipantReceiverTracks, bool(string_view conference_name, string_view participant_name, string_view connected_participant_name, const stream::Track* tracks, unsigned n_tracks));
	MOCK_METHOD4_OVERRIDE(SetParticipantCaps, bool(string_view conference_name, string_view participant_name, const void* caps_buf, size_t buf_sz));
	void SetParticipantSystemLoad(std::vector<stream::ParticipantLoadInfo>&& load) override
	{
		SetParticipantSystemLoad_mocked(load);
	}
	MOCK_METHOD1(SetParticipantSystemLoad_mocked, void(std::vector<stream::ParticipantLoadInfo>& load));
	void SetParticipantFrameSizeMB(const char* conference_name, const char* participant_name_to, std::vector<stream::ParticipantFrameSizeInfo>&& mb) override
	{
		SetParticipantFrameSizeMB_mocked(conference_name, participant_name_to, mb);
	}
	MOCK_METHOD3(SetParticipantFrameSizeMB_mocked, void(const char* conference_name, const char* participant_name_to, std::vector<stream::ParticipantFrameSizeInfo>& mb));
};

}
