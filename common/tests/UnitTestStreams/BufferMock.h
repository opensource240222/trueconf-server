#include "tests/common/GMockOverride.h"
#include "streams/Router/Buffer.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace streams_test {

class BufferMock : public stream::Buffer
{
public:
	BufferMock()
	{
		using ::testing::_;
		using ::testing::DoAll;
		using ::testing::Invoke;

		ON_CALL(*this, Init(_,_)).WillByDefault(Invoke([this](string_view conf_name, string_view part_name) {
			m_conf_name = std::string(conf_name);
			m_part_name = std::string(part_name);
			return true;
		}));
	}

	void DelegateTo(Buffer* impl)
	{
		using ::testing::_;
		using ::testing::Invoke;

		ON_CALL(*this, Init(_,_)).WillByDefault(Invoke(impl, &stream::Buffer::Init));
		ON_CALL(*this, Destroy(_,_)).WillByDefault(Invoke(impl, &stream::Buffer::Destroy));
		ON_CALL(*this, PutFrame_mocked(_,_,_,_)).WillByDefault(Invoke([impl](uint32_t tick_count, stream::Track track, std::unique_ptr<char[]>& buffer, size_t size) {
			return impl->PutFrame(tick_count, track, std::move(buffer), size);
		}));
		ON_CALL(*this, GetFrame(_,_,_,_)).WillByDefault(Invoke(impl, &stream::Buffer::GetFrame));
		ON_CALL(*this, GetFrameCount()).WillByDefault(Invoke(impl, static_cast<unsigned (stream::Buffer::*)() const>(&stream::Buffer::GetFrameCount)));
		ON_CALL(*this, GetFrameCount(_)).WillByDefault(Invoke(impl, static_cast<unsigned (stream::Buffer::*)(stream::Track track) const>(&stream::Buffer::GetFrameCount)));
		ON_CALL(*this, SetParticipantStatisticsInterface(_)).WillByDefault(Invoke(impl, &stream::Buffer::SetParticipantStatisticsInterface));
		ON_CALL(*this, SetStreamCrypter(_)).WillByDefault(Invoke(impl, &stream::Buffer::SetStreamCrypter));
	}

	MOCK_METHOD2_OVERRIDE(Init, bool(string_view conf_name, string_view part_name));
	MOCK_METHOD2_OVERRIDE(Destroy, void(string_view conf_name, string_view part_name));
	MOCK_METHOD4(PutFrame_mocked, Status(uint32_t tick_count, stream::Track track, std::unique_ptr<char[]>& buffer, size_t size));
	Status PutFrame(uint32_t tick_count, stream::Track track, std::unique_ptr<char[]>&& buffer, size_t size) override
	{
		return PutFrame_mocked(tick_count, track, buffer, size);
	}
	MOCK_METHOD4_OVERRIDE(GetFrame, Status(uint32_t& tick_count, stream::Track& track, std::unique_ptr<char[]>& buffer, size_t& size));
	MOCK_CONST_METHOD0_OVERRIDE(GetFrameCount, unsigned());
	MOCK_CONST_METHOD1_OVERRIDE(GetFrameCount, unsigned(stream::Track track));
	MOCK_METHOD1_OVERRIDE(SetParticipantStatisticsInterface, void(stream::ParticipantStatisticsInterface*));
	MOCK_METHOD1_OVERRIDE(SetStreamCrypter, void(VS_StreamCrypter*));
};

}
