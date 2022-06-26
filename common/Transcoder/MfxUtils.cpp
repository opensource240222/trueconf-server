#ifdef _WIN32
#include "MfxUtils.h"
#include <Windows.h>

unsigned short GetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, unsigned short nPoolSize)
{
	if (pSurfacesPool) {
		for (unsigned short i = 0; i < nPoolSize; i++) {
			if (0 == pSurfacesPool[i].Data.Locked) return i;
		}
	}
	return 0xffff;
}

unsigned short GetFreeSurface(mfxFrameSurface1* pSurfacesPool, unsigned short nPoolSize)
{
	unsigned int SleepInterval = 5; // milliseconds
	unsigned short idx = 0xffff;

	for (mfxU32 i = 0; i < MSDK_WAIT_INTERVAL; i += SleepInterval) {
		idx = GetFreeSurfaceIndex(pSurfacesPool, nPoolSize);
		if (0xffff != idx) break;
		Sleep(SleepInterval);
	}

	return idx;
}
#endif
