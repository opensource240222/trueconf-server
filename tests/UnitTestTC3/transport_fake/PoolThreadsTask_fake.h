#pragma once
#include "../../common/transport/Router/VS_PoolThreadsService.h"

#include <mutex>

namespace tc3_test
{
class PoolThreadsTask_fake : public VS_PoolThreadsTask
{
	std::mutex	recursion_lock_;
	void Thread() override;
};
}