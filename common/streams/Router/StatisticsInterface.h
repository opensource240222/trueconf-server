#pragma once

#include "../fwd.h"

#include <cstddef>

namespace stream {

class ParticipantStatisticsInterface
{
public:
	virtual size_t FormSndStatistics(StreamStatistics* s, size_t s_size, bool* video = nullptr) = 0;
	virtual size_t FormRcvStatistics(StreamStatistics* s, size_t s_size, bool* video = nullptr) = 0;
	virtual size_t GetSndStatisticsSize() = 0;
	virtual size_t GetRcvStatisticsSize() = 0;
};

}
