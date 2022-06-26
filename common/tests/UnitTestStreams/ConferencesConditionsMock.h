#include "tests/common/GMockOverride.h"
#include "streams/Router/ConferencesConditions.h"
#include "std/cpplib/VS_MediaFormat.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace streams_test {

class ConferencesConditionsMock : public stream::ConferencesConditions
{
public:
	ConferencesConditionsMock()
	{
	}

	void DelegateTo(ConferencesConditions* impl)
	{
		using ::testing::_;
		using ::testing::Invoke;

		ON_CALL(*this, CreateConference(_)).WillByDefault(Invoke(impl, &stream::ConferencesConditions::CreateConference));
		ON_CALL(*this, RemoveConference(_,_)).WillByDefault(Invoke(impl, &stream::ConferencesConditions::RemoveConference));
		ON_CALL(*this, AddParticipant(_,_)).WillByDefault(Invoke(impl, &stream::ConferencesConditions::AddParticipant));
		ON_CALL(*this, RemoveParticipant(_,_,_)).WillByDefault(Invoke(impl, &stream::ConferencesConditions::RemoveParticipant));
		ON_CALL(*this, RestrictBitrate(_,_,_)).WillByDefault(Invoke(impl, &stream::ConferencesConditions::RestrictBitrate));
		ON_CALL(*this, RestrictBitrateSVC(_,_,_,_,_)).WillByDefault(Invoke(impl, &stream::ConferencesConditions::RestrictBitrateSVC));
		ON_CALL(*this, SetParticipantCaps(_,_,_,_)).WillByDefault(Invoke(impl, &stream::ConferencesConditions::SetParticipantCaps));
	}

	MOCK_METHOD1_OVERRIDE(CreateConference, void(const char* conf_name));
	MOCK_METHOD2_OVERRIDE(RemoveConference, void(const char* conf_name, const stream::ConferenceStatistics& cs));
	MOCK_METHOD2_OVERRIDE(AddParticipant, void(const char* conf_name, const char* part_name));
	MOCK_METHOD3_OVERRIDE(RemoveParticipant, void(const char* conf_name, const char* part_name, const stream::ParticipantStatistics& ps));
	MOCK_METHOD3_OVERRIDE(RestrictBitrate, int(const char* conf_name, const char* part_name, int bitrate));
	MOCK_METHOD5_OVERRIDE(RestrictBitrateSVC, int(const char* conf_name, const char* part_name, int v_bitrate, int bitrate, int old_bitrate));
	MOCK_METHOD2_OVERRIDE(RequestKeyFrame, void(const char* conf_name, const char* part_name));
	MOCK_METHOD4_OVERRIDE(SetParticipantCaps, void(const char* conf_name, const char* part_name, const void* caps_buf, size_t buf_sz));
};

}
