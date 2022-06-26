#pragma once

#include "../fwd.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/attributes.h"

#include <memory>
#include <string>

class VS_StreamCrypter;

namespace stream {

class Buffer
{
public:
	enum class Status
	{
		non_fatal = 0,
		success = 1,
		fatal = -1,
	};

	virtual ~Buffer() {};

	virtual bool Init(string_view conf_name, string_view part_name) = 0;
	virtual void Destroy(string_view conf_name, string_view part_name) = 0;
	VS_NODISCARD virtual Status PutFrame(uint32_t tick_count, Track track, std::unique_ptr<char[]>&& buffer, size_t size) = 0;
	VS_NODISCARD virtual Status GetFrame(uint32_t& tick_count, Track& track, std::unique_ptr<char[]>& buffer, size_t& size) = 0;
	VS_NODISCARD virtual unsigned GetFrameCount() const = 0;
	VS_NODISCARD virtual unsigned GetFrameCount(Track track) const = 0;

	virtual void SetParticipantStatisticsInterface(ParticipantStatisticsInterface*) {};
	virtual void SetStreamCrypter(VS_StreamCrypter*) {};
	virtual bool Ready() const { return true; }
	virtual uint32_t GetQueueBytes() const { return 0; }

	const std::string& ConferenceName() const
	{
		return m_conf_name;
	}
	const std::string& ParticipantName() const
	{
		return m_part_name;
	}

protected:
	std::string m_conf_name;
	std::string m_part_name;
};

}
