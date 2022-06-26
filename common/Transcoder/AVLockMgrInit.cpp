#include <iostream>
#include <mutex>

#include "AVLockMgrInit.h"

static int my_lockmgr_cb(void **mutex, enum AVLockOp op)
{
	if (NULL == mutex)
		return -1;

	switch (op)
	{
	case AV_LOCK_CREATE:
	{
		*mutex = NULL;
		std::mutex * m = new std::mutex();
		*mutex = static_cast<void*>(m);
		break;
	}
	case AV_LOCK_OBTAIN:
	{
		std::mutex * m = static_cast<std::mutex*>(*mutex);
		m->lock();
		break;
	}
	case AV_LOCK_RELEASE:
	{
		std::mutex * m = static_cast<std::mutex*>(*mutex);
		m->unlock();
		break;
	}
	case AV_LOCK_DESTROY:
	{
		std::mutex * m = static_cast<std::mutex*>(*mutex);
		delete m;
		break;
	}
	default:
		break;
	}

	return 0;
}

void AVLockMgrInit()
{
	static bool isInit = false;
	static std::mutex initMut;

	initMut.lock();

	if (!isInit)
	{
		int res = av_lockmgr_register(my_lockmgr_cb);

		if (res < 0)
			std::cout << "av_lockmgr_register failed" << std::endl;
		else
			isInit = true;
	}

	initMut.unlock();
}
