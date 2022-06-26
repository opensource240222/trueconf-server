#pragma once

#include "intel_mdk/mfxvideo++.h"

//#define MSDK_DEC_WAIT_INTERVAL 60000
#define MSDK_DEC_WAIT_INTERVAL 10000
#define MSDK_ENC_WAIT_INTERVAL 10000
#define MSDK_VPP_WAIT_INTERVAL 60000
//#define MSDK_WAIT_INTERVAL MSDK_DEC_WAIT_INTERVAL+3*MSDK_VPP_WAIT_INTERVAL+MSDK_ENC_WAIT_INTERVAL // an estimate for the longest pipeline we have in samples
#define MSDK_WAIT_INTERVAL MSDK_DEC_WAIT_INTERVAL+MSDK_ENC_WAIT_INTERVAL // an estimate for the longest pipeline we have in samples

unsigned short GetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, unsigned short nPoolSize);

unsigned short GetFreeSurface(mfxFrameSurface1* pSurfacesPool, unsigned short nPoolSize);
