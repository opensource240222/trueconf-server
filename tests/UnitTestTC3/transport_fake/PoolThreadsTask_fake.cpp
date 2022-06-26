#ifdef _WIN32
#include "PoolThreadsTask_fake.h"
namespace tc3_test
{
void PoolThreadsTask_fake::Thread()
{
	std::unique_lock<std::mutex> l(recursion_lock_, std::try_to_lock);
	if (!l.owns_lock())
		return;
	VS_PoolThreadsTask::Thread();
}
}
#endif