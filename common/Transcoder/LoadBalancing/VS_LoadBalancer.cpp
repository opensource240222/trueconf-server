
#include "VS_LoadBalancer.h"
#include "BalancingModule.h"
#include "Transcoder/VideoCodec.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_Cpu.h"
#include "std/cpplib/VS_VideoLevelCaps.h"
#include "std/cpplib/VS_MediaFormat.h"
#include "std-generic/cpplib/scope_exit.h"
#include "Transcoder/nvidia/nvenc/NvEncoderCuda.h"
#include "Transcoder/nvidia/nvdec/cuviddec.h"
#include "Transcoder/VS_RetriveVideoCodec.h"

#ifdef _WIN32
#include "NvApiDeviceMonitoring.h"
#endif

#if !(defined(_WIN32) && !defined(_WIN64))
#include "NvmlDeviceMonitoring.h"
#endif

#include <algorithm>
#include <memory>
#include <thread>
#include <chrono>
#include <cmath>
#include <iomanip>

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

LoadBalancingHardware LoadBalancingHardware::m_loadBalancing;

namespace load_balancing
{

	enum class NvencAlgo : int32_t
	{
		highPerformance = 0,
		highQuality = 1,
		lowLatencyHP,
		lowLatencyHQ,
	};

	inline int32_t idAlgoNvenc(NvencAlgo algo)
	{
		return static_cast<int32_t>(algo);
	}

	struct NvidiaDeviceInfo
	{
		std::string board;
		std::string family;
		std::string chip;
		uint32_t baseCoreClock;
		int32_t numChips = 0;
		int32_t numNvencOnChip = 0;
		int32_t numStreamsNvenc = 0;
		int32_t baseFrameMBNvenc = 0;
		int32_t h264OnNvenc[4] = { 0 };
		int32_t h265OnNvenc[4] = { 0 };
		int32_t numNvdecOnChip = 0;
		int32_t numStreamsNvdec = 0;
		int32_t baseFrameMBNvdec = 0;
		int32_t h264OnNvdec = 0;
		int32_t h265OnNvdec = 0;
		int32_t vp8OnNvdec = 0;
		int32_t vp9OnNvdec = 0;
	};

	/// nvenc performance
#define             NVENC_NOTSUPPORT {   0,   0,   0,   0 }

#define  NVENC_H264_KEPLER_1GEN_PERF { 217,  76, 134,  77 }
#define  NVENC_H264_KEPLER_2GEN_PERF { 217,  76, 134,  77 }
#define NVENC_H264_MAXWELL_1GEN_PERF { 360, 216, 244, 230 }
#define NVENC_H264_MAXWELL_2GEN_PERF { 479, 278, 335, 280 }
#define  NVENC_H264_PASCAL_1GEN_PERF { 663, 370, 557, 367 }
#define   NVENC_H264_VOLTA_1GEN_PERF { 663, 370, 557, 367 }
#define  NVENC_H264_TURING_1GEN_PERF { 719, 423, 695, 418 }

#define  NVENC_H265_KEPLER_1GEN_PERF {   0,   0,   0,   0 }
#define  NVENC_H265_KEPLER_2GEN_PERF {   0,   0,   0,   0 }
#define NVENC_H265_MAXWELL_1GEN_PERF {   0,   0,   0,   0 }
#define NVENC_H265_MAXWELL_2GEN_PERF { 223, 157, 223, 217 }
#define  NVENC_H265_PASCAL_1GEN_PERF { 401, 250, 401, 401 }
#define   NVENC_H265_VOLTA_1GEN_PERF { 401, 250, 401, 401 }
#define  NVENC_H265_TURING_1GEN_PERF { 810, 159, 496, 328 }

	/// nvdec performance
#define       NVDEC_KEPLER_1GEN_PERF 161,    0,   0,   0
#define       NVDEC_KEPLER_2GEN_PERF 161,    0,   0,   0
#define      NVDEC_MAXWELL_1GEN_PERF 417,    0,   0,   0
#define      NVDEC_MAXWELL_2GEN_PERF 426,  464, 396, 396
#define       NVDEC_PASCAL_1GEN_PERF 648,  789, 820, 820
#define        NVDEC_VOLTA_1GEN_PERF 648,  789, 820, 820
#define       NVDEC_TURING_1GEN_PERF 690, 1261, 856, 856


	NvidiaDeviceInfo nvidiaGpuPerformance[] =
	{
		/// Quadro
		{              "Quadro K420",            "Kepler",   "GM107",  540, 1, 1,     2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{              "Quadro K600",            "Kepler",   "GM107",  540, 1, 1,     2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{             "Quadro K2000",            "Kepler",   "GM107",  540, 1, 1, 65535, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{            "Quadro K2000D",            "Kepler",   "GM107",  540, 1, 1, 65535, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{             "Quadro K4000",            "Kepler",   "GM106",  540, 1, 1, 65535, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{             "Quadro K4200",            "Kepler",   "GM104",  540, 1, 1, 65535, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{             "Quadro K5000",            "Kepler",   "GM104",  540, 1, 1, 65535, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{             "Quadro K5200",  "Kepler (2nd Gen)",  "GK110B",  540, 1, 1, 65535, 8100,  NVENC_H264_KEPLER_2GEN_PERF,  NVENC_H265_KEPLER_2GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_2GEN_PERF },
		{             "Quadro K6000",  "Kepler (2nd Gen)",  "GK110B",  540, 1, 1, 65535, 8100,  NVENC_H264_KEPLER_2GEN_PERF,  NVENC_H265_KEPLER_2GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_2GEN_PERF },
		{              "Quadro K620", "Maxwell (1st Gen)",   "GM107", 1366, 1, 1,		2, 8100, NVENC_H264_MAXWELL_1GEN_PERF, NVENC_H265_MAXWELL_1GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_1GEN_PERF },
		{             "Quadro K1200", "Maxwell (1st Gen)",   "GM107", 1366, 1, 1,		2, 8100, NVENC_H264_MAXWELL_1GEN_PERF, NVENC_H265_MAXWELL_1GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_1GEN_PERF },
		{             "Quadro K2200", "Maxwell (1st Gen)",   "GM107", 1366, 1, 1,	65535, 8100, NVENC_H264_MAXWELL_1GEN_PERF, NVENC_H265_MAXWELL_1GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_1GEN_PERF },
		{             "Quadro M4000", "Maxwell (2nd Gen)",   "GM204", 1366, 1, 2,	65535, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		{             "Quadro M5000", "Maxwell (2nd Gen)",   "GM204", 1366, 1, 2,	65535, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		{             "Quadro M6000", "Maxwell (2nd Gen)",   "GM200", 1366, 1, 2,	65535, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		{             "Quadro M2000",           "Maxwell",   "GM206", 1366, 1, 1,	65535, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		{              "Quadro P400",            "Pascal",   "GP107", 1911, 1, 1,		2, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{              "Quadro P600",            "Pascal",   "GP107", 1911, 1, 1,		2, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{              "Quadro P620",            "Pascal",   "GP107", 1911, 1, 1,		2, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{             "Quadro P1000",            "Pascal",   "GP107", 1911, 1, 1,		2, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{             "Quadro P2000",            "Pascal",   "GP106", 1911, 1, 1,	65535, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{             "Quadro P4000",            "Pascal",   "GP104", 1911, 1, 1,	65535, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{             "Quadro P5000",            "Pascal",   "GP104", 1911, 1, 2,	65535, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{             "Quadro P6000",            "Pascal",   "GP102", 1911, 1, 2,	65535, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{             "Quadro GP100",            "Pascal",   "GP100", 1911, 1, 3,	65535, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{             "Quadro GV100",             "Volta",   "GV100", 1911, 1, 3,	65535, 8100,   NVENC_H264_VOLTA_1GEN_PERF,   NVENC_H265_VOLTA_1GEN_PERF, 1, 65535, 8100,   NVDEC_VOLTA_1GEN_PERF },
		{             "Quadro T1000",            "Turing",   "TU117", 1755, 1, 1,		2, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		{             "Quadro T2000",            "Turing",   "TU117", 1755, 1, 1,	65535, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		{          "Quadro RTX 3000",            "Turing",   "TU106", 1755, 1, 3,	65535, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		{          "Quadro RTX 4000",            "Turing",   "TU104", 1755, 1, 2,	65535, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		{          "Quadro RTX 5000",            "Turing",   "TU104", 1755, 1, 2,	65535, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		{          "Quadro RTX 6000",            "Turing",   "TU102", 1755, 1, 1,	65535, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		{          "Quadro RTX 8000",            "Turing",   "TU102", 1755, 1, 1,	65535, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		/// Tesla
		{                "Tesla K10",            "Kepler",   "GK104",  540, 2, 1,	65535, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{               "Tesla K20X",            "Kepler",   "GK110",  540, 1, 1,	65535, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{                "Tesla K40",            "Kepler",  "GK110B",  540, 1, 1,	65535, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{                "Tesla K80",  "Kepler (2nd Gen)",   "GK210",  540, 2, 1,	65535, 8100,  NVENC_H264_KEPLER_2GEN_PERF,  NVENC_H265_KEPLER_2GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_2GEN_PERF },
		{                "Tesla M10", "Maxwell (1st Gen)",   "GM107", 1366, 4, 1,	65535, 8100, NVENC_H264_MAXWELL_1GEN_PERF, NVENC_H265_MAXWELL_1GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_1GEN_PERF },
		{                 "Tesla M4",   "Maxwell (GM206)",   "GM206", 1366, 1, 1,	65535, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		{                "Tesla M40", "Maxwell (2nd Gen)",   "GM200", 1366, 1, 2,	65535, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		{                 "Tesla M6", "Maxwell (2nd Gen)",   "GM204", 1366, 1, 2,	65535, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		{                "Tesla M60", "Maxwell (2nd Gen)",   "GM204", 1366, 2, 2,	65535, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		{                 "Tesla P4",            "Pascal",   "GP104", 1911, 1, 2,	65535, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{                 "Tesla P6",            "Pascal",   "GP104", 1911, 1, 2,	65535, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{                "Tesla P40",            "Pascal",   "GP102", 1911, 1, 2,	65535, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{               "Tesla P100",            "Pascal",   "GP100", 1911, 1, 3,	65535, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{               "Tesla V100",             "Volta",   "GV100", 1911, 1, 3,	65535, 8100,   NVENC_H264_VOLTA_1GEN_PERF,   NVENC_H265_VOLTA_1GEN_PERF, 1, 65535, 8100,   NVDEC_VOLTA_1GEN_PERF },
		{				  "Tesla T4",            "Turing",   "TU104", 1755, 1, 1,	65535, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 2, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		/// Grid
		{                  "GRID K1",            "Kepler",	 "GK107",  540, 4, 1,	65535, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{                  "GRID K2",            "Kepler",	 "GK104",  540, 2, 1,	65535, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{                "GRID K340",            "Kepler",	 "GK107",  540, 4, 1,	65535, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{                "GRID K520",            "Kepler",	 "GK104",  540, 2, 1,	65535, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		/// GeForce
		/// GeForce GT 630-640 & GeForce GT 710-730
		{            "GeForce GT 63",            "Kepler",	 "GK208",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{            "GeForce GT 64",            "Kepler",	 "GK208",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{            "GeForce GT 71",            "Kepler",	 "GK208",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{            "GeForce GT 72",            "Kepler",	 "GK208",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{            "GeForce GT 73",            "Kepler",	 "GK208",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		/// GeForce GT 630-640 & GeForce GTX 650 & GeForce GT 740
		{            "GeForce GT 63",            "Kepler",	 "GK107",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{            "GeForce GT 64",            "Kepler",	 "GK107",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{          "GeForce GTX 650",            "Kepler",	 "GK107",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{           "GeForce GT 740",            "Kepler",	 "GK107",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		/// GeForce GTX 645-660 Ti Boost & GeForce GT 740
		{ "GeForce GTX 645 Ti Boost",            "Kepler",	 "GK106",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{ "GeForce GTX 650 Ti Boost",            "Kepler",	 "GK106",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{ "GeForce GTX 655 Ti Boost",            "Kepler",	 "GK106",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{ "GeForce GTX 660 Ti Boost",            "Kepler",	 "GK106",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{           "GeForce GT 740",            "Kepler",	 "GK106",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		/// GeForce GTX 660-690 &  GeForce GTX 760-770
		{           "GeForce GTX 66",            "Kepler",	 "GK104",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{           "GeForce GTX 67",            "Kepler",	 "GK104",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{           "GeForce GTX 68",            "Kepler",	 "GK104",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{           "GeForce GTX 69",            "Kepler",	 "GK104",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{           "GeForce GTX 76",            "Kepler",	 "GK104",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		{           "GeForce GTX 77",            "Kepler",	 "GK104",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_1GEN_PERF,  NVENC_H265_KEPLER_1GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_1GEN_PERF },
		/// GeForce GT 780-780 Ti
		{       "GeForce GTX 780 Ti",  "Kepler (2nd Gen)",	 "GK110",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_2GEN_PERF,  NVENC_H265_KEPLER_2GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_2GEN_PERF },
		{          "GeForce GTX 780",  "Kepler (2nd Gen)",	 "GK110",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_2GEN_PERF,  NVENC_H265_KEPLER_2GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_2GEN_PERF },
		/// GeForce GTX 745-750 Ti
		{       "GeForce GTX 745 Ti", "Maxwell (1st Gen)",	 "GM107", 1366, 1, 1,	    2, 8100, NVENC_H264_MAXWELL_1GEN_PERF, NVENC_H265_MAXWELL_1GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_1GEN_PERF },
		/// GeForce GTX 960 Ti-980
		{       "GeForce GTX 960 Ti", "Maxwell (2st Gen)",	 "GM204", 1366, 1, 2,	    2, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		{       "GeForce GTX 970 Ti", "Maxwell (2st Gen)",	 "GM204", 1366, 1, 2,	    2, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		{          "GeForce GTX 970", "Maxwell (2st Gen)",	 "GM204", 1366, 1, 2,	    2, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		{          "GeForce GTX 980", "Maxwell (2st Gen)",	 "GM204", 1366, 1, 2,	    2, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		/// GeForce GTX 750 & GeForce GTX 950-960
		{          "GeForce GTX 750", "Maxwell (2st Gen)",	 "GM206", 1366, 1, 1,	    2, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		{           "GeForce GTX 95", "Maxwell (2st Gen)",	 "GM206", 1366, 1, 1,	    2, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		{           "GeForce GTX 96", "Maxwell (2st Gen)",	 "GM206", 1366, 1, 1,	    2, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		/// GeForce GTX 980 Ti
		{       "GeForce GTX 980 Ti", "Maxwell (2st Gen)",	 "GM200", 1366, 1, 2,	    2, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		/// GeForce GT 1030
		{          "GeForce GT 1030",			 "Pascal",	 "GP108", 1911, 0, 0,	    0, 8100,             NVENC_NOTSUPPORT,             NVENC_NOTSUPPORT, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		/// GeForce GTX 1050 / 1050 Ti
		{      "GeForce GTX 1050 Ti",			 "Pascal",	 "GP107", 1911, 1, 1,	    2, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{         "GeForce GTX 1050",			 "Pascal",	 "GP107", 1911, 1, 1,	    2, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{      "GeForce GTX 1050 Ti",			 "Pascal",	 "GP106", 1911, 1, 1,	    2, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{         "GeForce GTX 1050",			 "Pascal",	 "GP106", 1911, 1, 1,	    2, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		/// GeForce GTX 1060
		{         "GeForce GTX 1060",			 "Pascal",	 "GP106", 1911, 1, 1,	    2, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{         "GeForce GTX 1060",			 "Pascal",	 "GP104", 1911, 1, 1,	    2, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		/// GeForce GTX 1080 Ti
		{       "GeForce GTX 1080 Ti",			 "Pascal",	 "GP102", 1911, 1, 2,	    2, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		/// GeForce GTX 1070-1080
		{         "GeForce GTX 1070",			 "Pascal",	 "GP106", 1911, 1, 1,	    2, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{         "GeForce GTX 1080",			 "Pascal",	 "GP104", 1911, 1, 2,	    2, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		/// GeForce GTX 1650
		{		  "GeForce GTX 1650",			 "Turing",	 "TU117", 1755, 1, 1,	    2, 8100,   NVENC_H264_VOLTA_1GEN_PERF,   NVENC_H265_VOLTA_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		/// GeForce GTX 1660 / 1660 Ti
		{	   "GeForce GTX 1660 Ti",			 "Turing",	 "TU116", 1755, 1, 1,	    2, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		{		  "GeForce GTX 1660",			 "Turing",	 "TU116", 1755, 1, 1,	    2, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		/// GeForce GTX 2060 / 2070
		{		  "GeForce GTX 2060",			 "Turing",	 "TU106", 1755, 1, 1,	    2, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		{		  "GeForce GTX 2070",			 "Turing",	 "TU106", 1755, 1, 1,	    2, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		/// GeForce GTX 2080 Ti
		{		  "GeForce GTX 2080 Ti",		 "Turing",	 "TU104", 1755, 1, 1,	    2, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		/// GeForce GTX 2080
		{		  "GeForce GTX 2080",			 "Turing",	 "TU102", 1755, 1, 1,	    2, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		/// Titan
		{      "GeForce GTX Titan Black",  "Kepler (2nd Gen)",	 "GK110",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_2GEN_PERF,  NVENC_H265_KEPLER_2GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_2GEN_PERF },
		{          "GeForce GTX Titan Z",  "Kepler (2nd Gen)",	 "GK110",  540, 2, 1,	    2, 8100,  NVENC_H264_KEPLER_2GEN_PERF,  NVENC_H265_KEPLER_2GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_2GEN_PERF },
		{          "GeForce GTX Titan X", "Maxwell (2nd Gen)",	 "GM200", 1366, 1, 2,	    2, 8100, NVENC_H264_MAXWELL_2GEN_PERF, NVENC_H265_MAXWELL_2GEN_PERF, 1, 65535, 8100, NVDEC_MAXWELL_2GEN_PERF },
		{ "GeForce GTX Titan X Titan Xp",            "Pascal",	 "GP102", 1911, 1, 2,	    2, 8100,  NVENC_H264_PASCAL_1GEN_PERF,  NVENC_H265_PASCAL_1GEN_PERF, 1, 65535, 8100,  NVDEC_PASCAL_1GEN_PERF },
		{                      "Titan V",			  "Volta",	 "GV100", 1911, 1, 3,	    2, 8100,   NVENC_H264_VOLTA_1GEN_PERF,   NVENC_H265_VOLTA_1GEN_PERF, 1, 65535, 8100,   NVDEC_VOLTA_1GEN_PERF },
		{					 "Titan RTX",			 "Turing",	 "TU102", 1755, 1, 1,	    2, 8100,  NVENC_H264_TURING_1GEN_PERF,  NVENC_H265_TURING_1GEN_PERF, 1, 65535, 8100,  NVDEC_TURING_1GEN_PERF },
		{            "GeForce GTX Titan",  "Kepler (2nd Gen)",	 "GK110",  540, 1, 1,	    2, 8100,  NVENC_H264_KEPLER_2GEN_PERF,  NVENC_H265_KEPLER_2GEN_PERF, 1, 65535, 8100,  NVDEC_KEPLER_2GEN_PERF },
	};

	const char* NameFromFourcc(uint32_t fourcc)
	{
		if (fourcc == VS_VCODEC_H264) {
			return "h.264";
		}
		else if (fourcc == VS_VCODEC_H265) {
			return "h.265";
		}
		else if (fourcc == VS_VCODEC_VPX) {
			return "vp8";
		}
		else if (fourcc == VS_VCODEC_VP9) {
			return "vp9";
		}
		else {
			return "unknown";
		}
	}

	uint32_t FourccFromNvencGuid(GUID guid)
	{
		auto lcmp = [](GUID g1, GUID g2) -> bool
		{
			bool cmp = g1.Data1 == g2.Data1;
			cmp = cmp && (g1.Data2 == g2.Data2);
			cmp = cmp && (g1.Data3 == g2.Data3);
			cmp = cmp && (memcmp(g1.Data4, g2.Data4, sizeof(g2.Data4)) == 0);
			return cmp;
		};
		if (lcmp(guid, NV_ENC_CODEC_H264_GUID)) {
			return VS_VCODEC_H264;
		}
		else if (lcmp(guid, NV_ENC_CODEC_HEVC_GUID)) {
			return VS_VCODEC_H265;
		}
		else {
			return 0;
		}
	}

	uint32_t FourccFromCuvidType(cudaVideoCodec type)
	{
		if (type == cudaVideoCodec_H264) {
			return VS_VCODEC_H264;
		}
		else if (type == cudaVideoCodec_HEVC) {
			return VS_VCODEC_H265;
		}
		else if (type == cudaVideoCodec_VP8) {
			return VS_VCODEC_VPX;
		}
		else if (type == cudaVideoCodec_VP9) {
			return VS_VCODEC_VP9;
		}
		else {
			return 0;
		}
	}

	/// approximate encoder memory size (cpu or gpu) for fourcc
	/// vp8
	/// - software       = [360p : 10 Mb; 720p :  50 Mb; 1080p : 110 Mb] => memory = 0.0138 * MB - 1.4286
	/// h.264
	/// - software       = [360p : 15 Mb; 720p :  40 Mb; 1080p :  65 Mb] => memory = 0.1576 * MB ^ 0.6717
	/// - nvenc quadro   = [            ; 720p :  75 Mb; 1080p :  75 Mb]
	/// - nvenc consumer = [            ; 720p : 190 Mb; 1080p : 190 Mb]

	uint32_t GetApproximateEncoderMemorySize(uint32_t mb, uint32_t fourcc, BalancingDevice device, int32_t streams)
	{
		uint32_t memoryMB(200);
		if (device == BalancingDevice::software) {
			if (fourcc == VS_VCODEC_H264) {
				memoryMB = static_cast<uint32_t>(0.1576 * std::pow(mb, 0.6717));
			}
			else {
				memoryMB = static_cast<uint32_t>(0.0138 * mb - 1.4286);
			}
			if (memoryMB < 10) {
				memoryMB = 10;
			}
		}
		else if (device == BalancingDevice::nvidia) {
			memoryMB = (streams <= 2) ? 190 : 75;
		}
		return memoryMB;
	}

	uint32_t GetApproximateDecoderMemorySize(uint32_t mb, uint32_t fourcc, BalancingDevice device, int32_t streams)
	{
		uint32_t memoryMB(200);
		if (device == BalancingDevice::software) {
			memoryMB = (mb <= 8100) ? 10 : 40;
		}
		return memoryMB;
	}

	void GetNvidiaEncoderCaps(CUcontext cuContext, const NvidiaDeviceInfo & info, CodecCapability *capability)
	{
#ifdef PORTED_CODE
		/// encoder caps
		auto algo = NvencAlgo::lowLatencyHQ;
		std::unique_ptr<NvEncoder> nvenc;
		int32_t maxStreams(info.numStreamsNvenc);
		try {
			std::vector<std::unique_ptr<NvEncoder>> nvencs;
			nvenc = std::make_unique<NvEncoderCuda>(cuContext, 1920, 1080, NV_ENC_BUFFER_FORMAT::NV_ENC_BUFFER_FORMAT_IYUV, 0, false);
			if (maxStreams <= 2) {
				for (int i = 0; i < 2; i++) {
					nvencs.push_back(std::make_unique<NvEncoderCuda>(cuContext, 1920, 1080, NV_ENC_BUFFER_FORMAT::NV_ENC_BUFFER_FORMAT_IYUV, 0, false));
				}
				maxStreams = 65535;
			}
		}
		catch (NVENCException& e) {
			// no session to encode, but caps can be checked
		}
		if (!nvenc) {
			return;
		}
		if (maxStreams <= 2) {
			return;
		}
		GUID arrayGuids[] = { NV_ENC_CODEC_H264_GUID , NV_ENC_CODEC_HEVC_GUID };
		for (const auto & guid : arrayGuids) {
			auto maxWidth = nvenc->GetCapabilityValue(guid, NV_ENC_CAPS_WIDTH_MAX);
			auto maxHeight = nvenc->GetCapabilityValue(guid, NV_ENC_CAPS_HEIGHT_MAX);
			auto maxMB = nvenc->GetCapabilityValue(guid, NV_ENC_CAPS_MB_NUM_MAX);
			//if (maxMB > 0) {
			if (maxWidth > 0 && maxHeight > 0) {
				uint32_t fourcc(FourccFromNvencGuid(guid));
				if (fourcc != 0) {
					float reduce(1.0f);
					if (fourcc == VS_VCODEC_H265) {
						reduce = static_cast<float>(info.h264OnNvenc[idAlgoNvenc(algo)]) / static_cast<float>(info.h265OnNvenc[idAlgoNvenc(algo)]);
					}
					capability->caps[fourcc] = std::make_pair(maxWidth * maxHeight / 256, reduce);
				}
			}
		}
		if (!capability->caps.empty()) {
			capability->maxStreams = maxStreams;
			capability->maxMBps = info.numNvencOnChip * info.h264OnNvenc[idAlgoNvenc(algo)] * info.baseFrameMBNvenc;
		}
#endif
	}

	void GetNvidiaDecoderCaps(CUcontext cuContext, const NvidiaDeviceInfo & info, CodecCapability *capability)
	{
#ifdef PORTED_CODE
		cudaVideoCodec arrayTypes[] = { cudaVideoCodec_H264 , cudaVideoCodec_HEVC, cudaVideoCodec_VP8, cudaVideoCodec_VP9 };
		for (const auto & type : arrayTypes) {
			CUVIDDECODECAPS decodeCaps = {};
			decodeCaps.eCodecType = type;
			decodeCaps.eChromaFormat = cudaVideoChromaFormat_420;
			decodeCaps.nBitDepthMinus8 = 0;
			if (cuvidGetDecoderCaps(&decodeCaps) == CUDA_SUCCESS) {
				if (decodeCaps.bIsSupported == 1 && decodeCaps.nMaxWidth > 0 && decodeCaps.nMaxHeight > 0) {
					uint32_t fourcc(FourccFromCuvidType(type));
					if (fourcc != 0) {
						float reduce(1.0f);
						if (fourcc == VS_VCODEC_H265) {
							reduce = static_cast<float>(info.h264OnNvdec) / static_cast<float>(info.h265OnNvdec);
						}
						else if (fourcc == VS_VCODEC_VP9) {
							reduce = static_cast<float>(info.h264OnNvdec) / static_cast<float>(info.vp9OnNvdec);
						}
						else if (fourcc == VS_VCODEC_VPX) {
							reduce = static_cast<float>(info.h264OnNvdec) / static_cast<float>(info.vp8OnNvdec);
						}
						capability->caps[fourcc] = std::make_pair(decodeCaps.nMaxWidth * decodeCaps.nMaxHeight / 256, reduce);
					}
				}
			}
		}
		if (!capability->caps.empty()) {
			capability->maxStreams = info.numStreamsNvdec;
			capability->maxMBps = info.numNvdecOnChip * info.h264OnNvdec * info.baseFrameMBNvdec;
		}
#endif
	}

	NvidiaDeviceInfo GetNvidiaDeviceInfo(const std::string & deviceName)
	{
		for (const auto& nv : nvidiaGpuPerformance) {
			auto pos = deviceName.find(nv.board);
			if (pos != std::string::npos) {
				return nv;
			}
		}
		return NvidiaDeviceInfo();
	}

	std::string ViewResourceCaps(load_balancing::BalancingDevice device, const load_balancing::CodecCapability & capability)
	{
		if (device == BalancingDevice::software) {
			return "max";
		}
		if (capability.caps.empty()) {
			return "no";
		}
		std::string view;
		for (const auto & caps : capability.caps) {
			char buff[64] = { 0 };
			sprintf(buff, "%5s:%7u|", NameFromFourcc(caps.first), caps.second.first);
			view += buff;
		}
		return view;
	}

	bool RegisterBalancingModule(std::shared_ptr<balancing_module::BalancingModule> mod)
	{
		return LoadBalancingHardware::GetLoadBalancing().RegisterModule(mod);
	}

}

using namespace load_balancing;
using namespace hardware_monitoring;

class VS_MultimediaBenchmark
{
public:
	VS_MultimediaBenchmark(int32_t w, int32_t h, int32_t br, float fr) : Width(w), Height(h), Bitrate(br), Framerate(fr)
	{
		InBuff.resize(Width * Height * 3 / 2);

		for (size_t i = 0; i < InBuff.size(); i++)
			InBuff[i] = i % 256;
	}

	void TestThread(
		std::shared_ptr<VideoCodec> encoder,
		std::shared_ptr<VideoCodec> decoder,
		std::vector<uint8_t>& inBuff,
		std::vector<uint8_t>& encBuff,
		std::vector<uint8_t>& decBuff,
		std::chrono::milliseconds testTime,
		uint32_t& result)
	{
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		int count = 0;

		while (std::chrono::steady_clock::now() - start < testTime)
		{
			VS_VideoCodecParam encParam;
			VS_VideoCodecParam decParam;

			encParam.cmp.KeyFrame = (count == 0) ? 1 : 0;

			decParam.dec.FrameSize = encoder->Convert(inBuff.data(), encBuff.data(), &encParam);
			decoder->Convert(encBuff.data(), decBuff.data(), &decParam);

			count++;
		}

		result = static_cast<uint32_t>(float(count) * 1000.f / float(testTime.count()) / Framerate);
	}

	uint32_t TestMt(uint32_t threadCount, uint32_t testTimeMs)
	{
		if (threadCount == 0)
		{
			uint32_t physicalCores = 0;
			uint32_t logicalCores = 0;

			VS_GetNumCPUCores(&physicalCores, &logicalCores);

			threadCount = logicalCores;
		}

		std::vector<std::shared_ptr<VideoCodec>> encoders;
		std::vector<std::shared_ptr<VideoCodec>> decoders;
		std::vector<std::vector<uint8_t>> encBuffers;
		std::vector<std::vector<uint8_t>> decBuffers;

		for (uint32_t i = 0; i < threadCount; i++)
		{
			encoders.emplace_back(VS_RetriveVideoCodec(VS_VCODEC_H264, true));
			decoders.emplace_back(VS_RetriveVideoCodec(VS_VCODEC_H264, false));
			encBuffers.emplace_back(InBuff.size());
			decBuffers.emplace_back(InBuff.size());

			encoders.back()->Init(Width, Height, FOURCC_I420, 0, -1);
			encoders.back()->SetBitrate(Bitrate);

			decoders.back()->Init(Width, Height, FOURCC_I420);
		}

		std::vector<std::thread> threads;
		std::vector<uint32_t> results(threadCount);

		auto f = [&](uint32_t i)
		{
			TestThread(
				encoders[i],
				decoders[i],
				InBuff,
				encBuffers[i],
				decBuffers[i],
				std::chrono::milliseconds(testTimeMs),
				results[i]
			);
		};

		for (uint32_t i = 0; i < threadCount; i++)
			threads.emplace_back(f, i);

		for (uint32_t i = 0; i < threadCount; i++)
			threads[i].join();

		uint32_t result = 0;

		for (uint32_t r : results)
			result += r;

		return result;
	}

private:
	uint32_t Width = 1280;
	uint32_t Height = 720;
	int32_t Bitrate = 1000;
	float Framerate = 15.0f;

	std::vector<uint8_t> InBuff;
};

LoadBalancingHardware& LoadBalancingHardware::GetLoadBalancing()
{
	return m_loadBalancing;
}

LoadBalancingHardware::LoadBalancingHardware() : m_monitoringEvents(MonitorEvents::cme_size, false),
												 m_balancingModules(std::make_shared<arrayModuleBalancing>())
{

}

LoadBalancingHardware::~LoadBalancingHardware()
{
	ReleaseResources();
}

uint32_t LoadBalancingHardware::HardwareFromDevice(const load_balancing::BalancingDevice &device)
{
	if (device == BalancingDevice::software) {
		return eHardwareEncoder::ENCODER_SOFTWARE;
	}
	else if (device == BalancingDevice::nvidia) {
		return eHardwareEncoder::ENCODER_H264_NVIDIA;
	}
	else if (device == BalancingDevice::intel) {
		return eHardwareEncoder::ENCODER_H264_INTEL_MSS;
	}
	else {
		return 0;
	}
}

void LoadBalancingHardware::Enable(int32_t levelHardwareBalancing, int32_t levelCpuBalancing)
{
	std::lock_guard<std::recursive_mutex> lock(m_lblock);
	if (m_levelHardwareBalancing == levelHardwareBalancing &&
		m_levelCpuBalancing == levelCpuBalancing)
	{
		dprint0("Load Balancing : initialization - skiped [hw = %d, sw = %d]", m_levelHardwareBalancing, m_levelCpuBalancing);
		return;
	}
	ReleaseResources();
	if (levelHardwareBalancing > 0 || levelCpuBalancing > 0) {
		m_levelHardwareBalancing = levelHardwareBalancing;
		m_levelCpuBalancing = levelCpuBalancing;
		if (EnumerateResources()) {
			m_monitoringEvents.set(MonitorEvents::cme_checkLogging);
		}
		else {
			ReleaseResources();
			dprint0("Load Balancing : initialization - failed [hw = %d, sw = %d]", m_levelHardwareBalancing, m_levelCpuBalancing);
		}
	}
	else {
		dprint0("Load Balancing : initialization - cleared [hw = %d, sw = %d]", m_levelHardwareBalancing, m_levelCpuBalancing);
	}
}

void LoadBalancingHardware::LoggingResources()
{
	if (m_balancingDevices.empty()) {
		return;
	}
	dprint3("Load Balancing : overall resources");
	for (const auto & it : m_balancingDevices) {
		for (const auto & desc : it.second) {
			auto monitor = *desc.monitorInfo.lock();
			if (it.first == BalancingDevice::software) {
				dprint3("%30s : #%2d, cpu = %5.2f, process = %5.2f, memory = [%8d / %8d]",
						desc.name.c_str(), desc.id, monitor.utilizationEngine, monitor.utilizationEncoder, monitor.memoryTotal - monitor.memoryFree, monitor.memoryTotal);
			}
			else {
				dprint3("%30s : #%2d, eng = %5.2f [enc = %5.2f, dec = %5.2f], memory = [%8d / %8d]",
						desc.name.c_str(), desc.id, monitor.utilizationEngine, monitor.utilizationEncoder, monitor.utilizationDecoder, monitor.memoryTotal - monitor.memoryFree, monitor.memoryTotal);
			}
			dprint3("%35s : streams = [%6d / %6d], mbps = [%8d / %8d], locked = %d, caps = %s", "encoder",
					desc.encoderDesc.activeStreams, desc.encoderDesc.maxStreams,
					desc.encoderDesc.activeMBps, desc.encoderDesc.maxMBps, monitor.lockedEncoder ? 1 : 0,
					desc.encoderDesc.capsView.c_str());
			dprint3("%35s : streams = [%6d / %6d], mbps = [%8d / %8d], locked = %d, caps = %s", "decoder",
					desc.decoderDesc.activeStreams, desc.decoderDesc.maxStreams,
					desc.decoderDesc.activeMBps, desc.decoderDesc.maxMBps, monitor.lockedDecoder ? 1 : 0,
					desc.decoderDesc.capsView.c_str());
		}
	}
	auto modules = m_balancingModules.load();
	if (!modules->empty()) {
		dprint3("Load Balancing : balancing resources");
	}
	double overallLoad(0.0);
	std::vector<std::pair<balancing_module::Type, double>> moduleLoad;
	for (uint32_t i = 0; i < balancing_module::Type::max; i++) {
		auto type(static_cast<balancing_module::Type>(i));
		balancing_module::mapThreadUtilization utilization;
		for (const auto & it : *modules) {
			auto mod = it.lock();
			if (!mod) {
				continue;
			}
			if (type != mod->GetType()) {
				continue;
			}
			auto &info = mod->GetResourceUtilization();
			for (const auto &it : info) {
				std::copy(it.second.begin(), it.second.end(), std::back_inserter(utilization[it.first]));
			}
		}
		if (utilization.empty()) {
			continue;
		}
		dstream3 << std::setw(35) << std::right << balancing_module::GetModuleName(type) << " module ----- ";
		double load(0.0);
		for (const auto & it : utilization) {
			if (it.second.empty()) {
				continue;
			}
			double overall(0.0), min_load(101.0), max_load(0.0);
			for (const auto &l : it.second) {
				overall += l;
				min_load = std::min(min_load, l);
				max_load = std::max(max_load, l);
			}
			load += overall;
			const auto threadName = balancing_module::GetThreadName(it.first);
			dprint3("%35.*s : %6.2f [num = %3u, min: %5.2f, max: %5.2f]", static_cast<int>(threadName.size()), threadName.data(), overall, static_cast<uint32_t>(it.second.size()), min_load, max_load);
		}
		overallLoad += load;
		moduleLoad.push_back(std::make_pair(type, load));
	}
	std::stringstream ss;
	for (const auto & it : moduleLoad) {
		uint32_t load(0);
		if (overallLoad) {
			load = static_cast<uint32_t>(it.second / overallLoad * 100.0);
		}
		ss << " | " << balancing_module::GetModuleName(it.first) << " = " << load;
	}
	dprint3("%35s : %6.2f %s", "Overall load", overallLoad, ss.str().c_str());
}

void LoadBalancingHardware::MonitoringWorkerThread()
{
	std::chrono::steady_clock::time_point loggingTm;
	std::chrono::steady_clock::time_point checkTm = std::chrono::steady_clock::now();
	bool exit(false);
	while (!exit) {
		uint32_t waitRes = m_monitoringEvents.wait_for(std::chrono::milliseconds(3000), false);
		switch (waitRes) {
			case vs::wait_result::time_to_die:
			{
				exit = true;
				continue;
			}
			case MonitorEvents::cme_checkLogging:
			{
				loggingTm = {};
				VS_FALLTHROUGH;
			}
			case MonitorEvents::cme_checkUtilization:
			case vs::wait_result::timeout:
			{
				for (auto & it : m_balancingDevices) {
					auto monitor = m_monitorDevices[it.first].get();
					for (auto & desc : it.second) {
						if (desc.monitor == 0 && it.first != BalancingDevice::software) {
							continue;
						}
						MonitoringInfo info;
						if (monitor->GetDeviceUtilization(desc.monitor, desc.ctx, info)) {
							desc.monitorInfo.withLock([&info] (MonitoringInfo &mi) {
								info.lockedEncoder = mi.lockedEncoder || (info.utilizationEncoder >= 85.0);
								info.lockedDecoder = mi.lockedDecoder || (info.utilizationDecoder >= 85.0);
								mi = info;
							});
						}
					}
				}
				bool expired(false);
				const auto now = std::chrono::steady_clock::now();
				auto tm = now - checkTm;
				auto modules = m_balancingModules.load();
				for (auto & it : *modules) {
					auto mod = it.lock();
					if (!mod) {
						expired = true;
						continue;
					}
					mod->CheckResourceUtilization(std::chrono::duration_cast<std::chrono::milliseconds>(tm).count());
				}
				checkTm = now;
				if (expired) {
					ClearModules();
				}
				break;
			}
			default:
			{
				break;
			}
		}
		const auto now = std::chrono::steady_clock::now();
		if (now - loggingTm > std::chrono::seconds(60)) {
			LoggingResources();
			loggingTm = now;
		}
	}
}

void LoadBalancingHardware::SoftwareBenchmark()
{
	// currently this rating is not used in load balancing
	int32_t w(1280), h(720), br(1500);
	float fr(30.0);
	VS_MultimediaBenchmark mmbench(w, h, br, fr);
	int32_t res = mmbench.TestMt(0, 1000);
	m_softwareRating = static_cast<uint32_t>((w * h * fr / 256) * res);
	dprint0("Load Balancing : software rating = %d", m_softwareRating);
}

bool LoadBalancingHardware::CheckNvidiaResources()
{

#ifdef _WIN32
	auto hCuda = LoadLibrary("nvcuda.dll");
	auto hCuvid = LoadLibrary("nvcuvid.dll");
	auto cuvidFunc = GetProcAddress(hCuvid, "cuvidGetDecoderCaps");
	return (hCuda != nullptr && hCuvid != nullptr && cuvidFunc != nullptr);
#else
	return false;
#endif

}

bool LoadBalancingHardware::EnumerateResources()
{
	if (m_bEnumerate) {
		return true;
	}
	m_balancingDevices.clear();
	/// nvidia check
	do {
		if (m_levelHardwareBalancing == 0) {
			dprint0("Load Balancing : hardware balancing - disabled");
			break;
		}
		if (!CheckNvidiaResources()) {
			dprint0("Load Balancing : missing nvcuda.dll or nvcuvid.dll");
			break;
		}
#ifdef PORTED_CODE
		if (cuInit(0) != CUDA_SUCCESS) {
			dprint0("Load Balancing : cuda cannot initialize");
			break;
		}
#endif
		auto monitor = DeviceMonitoring::CreateDeviceMonitoring(BalancingDevice::nvidia);
		if (!monitor) {
			dprint0("Load Balancing : nvidia monitoring api cannot initialize");
			break;
		}
		if (!EnumerateNvidiaDevices(monitor.get())) {
			dprint0("Load Balancing : nvidia device enumerating error");
			break;
		}
		m_monitorDevices[BalancingDevice::nvidia] = std::move(monitor);
		for (auto & desc : m_balancingDevices[BalancingDevice::nvidia]) {
			if (desc.ctx == 0) {
				dprint0("Load Balancing : %s : #%2d - no context, excluded from the balancer", desc.name.c_str(), desc.id);
			}
			if (desc.monitor == 0) {
				dprint0("Load Balancing : %s : #%2d - no monitor api, excluded from the balancer", desc.name.c_str(), desc.id);
			}
		}
		dprint0("Load Balancing : nvidia initialization - success [level = %d]", m_levelHardwareBalancing);
	} while (false);
	/// intel check: TODO
	/// amd check: TODO
	/// software check
	do {
		auto monitor = DeviceMonitoring::CreateDeviceMonitoring(BalancingDevice::software);
		if (!monitor) {
			dprint0("Load Balancing : software monitoring api cannot initialize");
			return false;
		}
		if (!EnumerateSoftwareDevices(monitor.get())) {
			dprint0("Load Balancing : software device enumerating error");
			return false;
		}
		m_monitorDevices[BalancingDevice::software] = std::move(monitor);
		dprint0("Load Balancing : software initialization - success [level = %d]", m_levelCpuBalancing);
	} while (false);
	{
		for (auto & it : m_balancingDevices) {
			for (auto & desc : it.second) {
				desc.encoderDesc.capsView = ViewResourceCaps(it.first, desc.encoderDesc);
				desc.decoderDesc.capsView = ViewResourceCaps(it.first, desc.decoderDesc);
			}
		}
		m_monitoringEvents.reset();
		m_monitorigThread = std::thread([this] () {
			vs::SetThreadName("Balance Monitor");
			MonitoringWorkerThread();
		});
	}
	m_bEnumerate = true;
	return !m_balancingDevices.empty();
}

void LoadBalancingHardware::ReleaseResources()
{
	if (m_monitorigThread.joinable()) {
		m_monitoringEvents.kill_listener();
		m_monitorigThread.join();
	}
	m_monitorDevices.clear();
	m_balancingDevices.clear();
	m_handleStorage.clear();
	m_contextDevices.clear();
	m_bEnumerate = false;
	m_levelHardwareBalancing = 0;
	m_levelCpuBalancing = 0;
}

bool LoadBalancingHardware::RegisterModule(std::weak_ptr<balancing_module::BalancingModule> mod)
{
	if (m_levelCpuBalancing == 0) {
		return false;
	}
	if (!mod.lock()) {
		return false;
	}
	auto modules = m_balancingModules.load();
	auto new_modules = std::make_shared<arrayModuleBalancing>();
	do {
		*new_modules = *modules;
		new_modules->emplace_back(std::move(mod));
	} while (!m_balancingModules.compare_exchange_weak(modules, new_modules));
	return true;
}

void LoadBalancingHardware::ClearModules()
{
	auto modules = m_balancingModules.load();
	auto new_modules = std::make_shared<arrayModuleBalancing>();
	do {
		*new_modules = *modules;
		new_modules->erase(std::remove_if(new_modules->begin(), new_modules->end(),
										  [] (std::weak_ptr<balancing_module::BalancingModule> weak) -> bool { return weak.expired(); }),
						   new_modules->end());
	} while (!m_balancingModules.compare_exchange_weak(modules, new_modules));
}

load_balancing::HandleCodecInfo LoadBalancingHardware::HoldResources(const VS_MediaFormat &mf, bool encoder)
{
	std::lock_guard<std::recursive_mutex> lock(m_lblock);
	load_balancing::HandleCodecInfo info;
	if (m_balancingDevices.empty()) {
		return info;
	}
	m_monitoringEvents.set(MonitorEvents::cme_checkUtilization);
	auto mb(mf.GetFrameSizeMB());
	auto mbps(mf.GetMBps());
	auto monitor = *m_balancingDevices[BalancingDevice::software][0].monitorInfo.lock();
	if (mf.dwHWCodec == eHardwareEncoder::ENCODER_SLIDES) {
		info = GetSoftwareResource(mf.dwVideoCodecFCC, mbps, mb, encoder);
	}
	else if (!encoder || monitor.utilizationEngine >= 90 || mb >= 900) {
		info = GetHardwareResource(mf.dwVideoCodecFCC, mbps, mb, encoder);
		if (info.error != load_balancing::ErrorMessage::ok) {
			info = GetSoftwareResource(mf.dwVideoCodecFCC, mbps, mb, encoder);
		}
	}
	else {
		info = GetSoftwareResource(mf.dwVideoCodecFCC, mbps, mb, encoder);
		if (info.error != load_balancing::ErrorMessage::ok) {
			info = GetHardwareResource(mf.dwVideoCodecFCC, mbps, mb, encoder);
		}
	}
	RegisterDeviceContext(info);
	return info;
}

void LoadBalancingHardware::UnholdResources(const load_balancing::HandleCodecInfo &info)
{
	std::lock_guard<std::recursive_mutex> lock(m_lblock);
	if (m_balancingDevices.empty()) {
		return;
	}
	if (info.error == load_balancing::ErrorMessage::ok) {
		FreeResource(info);
	}
	m_monitoringEvents.set(MonitorEvents::cme_checkUtilization);
}

bool LoadBalancingHardware::RegisterVideoCodec(const std::uintptr_t handle, const load_balancing::HandleCodecInfo &info)
{
	std::lock_guard<std::recursive_mutex> lock(m_lblock);
	if (m_balancingDevices.empty()) {
		return false;
	}
	auto it = m_handleStorage.find(handle);
	if (it != m_handleStorage.end()) {
		return false;
	}
	m_handleStorage[handle] = info;
	if (info.error != load_balancing::ErrorMessage::ok) {
		dprint0("Load Balancing : insufficient resources !!!");
		m_monitoringEvents.set(MonitorEvents::cme_checkLogging);
	} else {
		auto monitor = *m_balancingDevices[BalancingDevice::software][0].monitorInfo.lock();
		dprint3("Load Balancing : cpu = %5.2f,   register video %s [%5s, %8u, %6u] : #%2d %20s",
				monitor.utilizationEngine, info.encoder ? "encoder" : "decoder", NameFromFourcc(info.fourcc), info.mbps, info.mb, info.id, info.name.c_str());
	}
	return true;
}

bool LoadBalancingHardware::UnregisterVideoCodec(const std::uintptr_t handle)
{
	std::lock_guard<std::recursive_mutex> lock(m_lblock);
	if (m_balancingDevices.empty()) {
		return false;
	}
	auto it = m_handleStorage.find(handle);
	if (it == m_handleStorage.end()) {
		return false;
	}
	if (it->second.error != load_balancing::ErrorMessage::ok) {
		return true;
	}
	bool ret = FreeResource(it->second);
	{
		auto monitor = *m_balancingDevices[BalancingDevice::software][0].monitorInfo.lock();
		dprint3("Load Balancing : cpu = %5.2f, unregister video %s [%5s, %8u] : #%2d %20s",
				monitor.utilizationEngine, it->second.encoder ? "encoder" : "decoder", NameFromFourcc(it->second.fourcc), it->second.mbps, it->second.id, it->second.name.c_str());
	}
	m_handleStorage.erase(it);
	return ret;
}

std::uintptr_t LoadBalancingHardware::GetContextDevice(const std::uintptr_t handle)
{
	std::lock_guard<std::recursive_mutex> lock(m_lblock);
	if (m_levelHardwareBalancing < 2) {
		return 0;
	}
	if (m_balancingDevices.empty()) {
		return 0;
	}
	auto it = m_handleStorage.find(handle);
	if (it == m_handleStorage.end()) {
		return 0;
	}
	return it->second.ctx;
}

load_balancing::BalancingDevice LoadBalancingHardware::GetTypeDevice(const std::uintptr_t handle)
{
	std::lock_guard<std::recursive_mutex> lock(m_lblock);
	if (m_levelHardwareBalancing <= 0) {
		return BalancingDevice::software;
	}
	if (m_balancingDevices.empty()) {
		return BalancingDevice::software;
	}
	auto it = m_handleStorage.find(handle);
	if (it == m_handleStorage.end()) {
		return BalancingDevice::software;
	}
	return it->second.device;
}

bool LoadBalancingHardware::EnumerateSoftwareDevices(hardware_monitoring::DeviceMonitoring *monitor)
{
	load_balancing::DeviceDesc dd;
	load_balancing::CodecCapability desc;
	desc.maxStreams = 65535;
	desc.maxMBps = m_softwareRating;
	desc.caps[VS_VCODEC_H264] = std::make_pair(58982400, 1.0f);
	desc.caps[VS_VCODEC_H265] = std::make_pair(58982400, 1.0f);
	desc.caps[VS_VCODEC_VPX]  = std::make_pair(58982400, 1.0f);
	desc.caps[VS_VCODEC_VP9]  = std::make_pair(58982400, 1.0f);
	dd.name = "software";
	dd.encoderDesc = desc;
	dd.decoderDesc = desc;
	m_balancingDevices[BalancingDevice::software].push_back(std::move(dd));
	return true;
}

bool LoadBalancingHardware::EnumerateNvidiaDevices(hardware_monitoring::DeviceMonitoring *monitor)
{
	int count(0);
#ifdef PORTED_CODE
	int deviceCount(0);
	if (cuDeviceGetCount(&deviceCount) != CUDA_SUCCESS) {
		return false;
	}
	for (int i = 0; i < deviceCount; i++) {
		CUdevice cuDevice(0);
		if (cuDeviceGet(&cuDevice, i) != CUDA_SUCCESS) {
			continue;
		}
		int mj(0), mn(0);
		if (cuDeviceComputeCapability(&mj, &mn, cuDevice) != CUDA_SUCCESS) {
			continue;
		}
		if (((mj << 4) + mn) < 0x30) {
			continue;
		}
		char devName[256];
		if (cuDeviceGetName(devName, sizeof(devName), cuDevice) != CUDA_SUCCESS) {
			continue;
		}
		load_balancing::DeviceDesc desc;
		CUcontext cuContext;
		if (cuCtxCreate(&cuContext, 0, cuDevice) == CUDA_SUCCESS) {
			auto info = GetNvidiaDeviceInfo(devName);
			GetNvidiaEncoderCaps(cuContext, info, &desc.encoderDesc);
			GetNvidiaDecoderCaps(cuContext, info, &desc.decoderDesc);
			desc.baseCoreClock = info.baseCoreClock;
			desc.ctx = reinterpret_cast<std::uintptr_t>(cuContext);
			auto ContextDestroy = [] (std::uintptr_t *ctx)
			{
				if (ctx && *ctx) {
					cuCtxDestroy(reinterpret_cast<CUcontext>(*ctx));
					delete ctx;
				}
			};
			m_contextDevices.emplace_back(new std::uintptr_t(desc.ctx), ContextDestroy);
		}
		hardware_monitoring::PciInfo pci;
		if (cuDeviceGetAttribute(&pci.bus, CU_DEVICE_ATTRIBUTE_PCI_BUS_ID, cuDevice) == CUDA_SUCCESS &&
			cuDeviceGetAttribute(&pci.device, CU_DEVICE_ATTRIBUTE_PCI_DEVICE_ID, cuDevice) == CUDA_SUCCESS &&
			cuDeviceGetAttribute(&pci.domain, CU_DEVICE_ATTRIBUTE_PCI_DOMAIN_ID, cuDevice) == CUDA_SUCCESS)
		{
			desc.monitor = monitor->GetMonitor(devName, i, pci);
		}
		desc.id = i;
		desc.name = devName;
		m_balancingDevices[BalancingDevice::nvidia].push_back(std::move(desc));
		count++;
	}
#endif
	return (count > 0);
}

bool LoadBalancingHardware::GetResource(DeviceDesc & desc, BalancingDevice device, uint32_t memoryFree, uint32_t mbps, uint32_t mb, uint32_t fourcc, bool encoder, HandleCodecInfo *info)
{
	auto capability = (encoder) ? &desc.encoderDesc : &desc.decoderDesc;
	auto it = capability->caps.find(fourcc);
	if (it == capability->caps.end()) {
		return false;
	}
	if (it->second.first < mb) {
		return false;
	}
	uint32_t memoryMB = (encoder) ? GetApproximateEncoderMemorySize(mb, fourcc, device, capability->maxStreams) :
									GetApproximateDecoderMemorySize(mb, fourcc, device, capability->maxStreams);
	if ((capability->activeStreams >= capability->maxStreams) || (memoryFree <= memoryMB)) {
		info->error = load_balancing::ErrorMessage::noGpuResources;
		return false;
	}
	capability->activeStreams++;
	capability->activeMBps += mbps;
	info->name = desc.name;
	info->id = desc.id;
	info->ctx = desc.ctx;
	info->device = device;
	info->fourcc = fourcc;
	info->mb = mb;
	info->mbps = mbps;
	info->encoder = encoder;
	info->error = load_balancing::ErrorMessage::ok;
	return true;
}

load_balancing::HandleCodecInfo LoadBalancingHardware::GetHardwareResource(uint32_t fourcc, uint32_t mbps, uint32_t mb, bool encoder)
{
	load_balancing::HandleCodecInfo info;
	info.error = load_balancing::ErrorMessage::noGpuResources;
	/// devices with unrestricted concurent sessions
	for (auto &it : m_balancingDevices) {
		if (it.first == BalancingDevice::software) {
			continue;
		}
		for (auto &desc : it.second) {
			if (desc.monitor == 0) {
				continue;
			}
			auto monitor = *desc.monitorInfo.lock();
			auto locked = (encoder) ? monitor.lockedEncoder : monitor.lockedDecoder;
			auto capability = (encoder) ? &desc.encoderDesc : &desc.decoderDesc;
			if (capability->maxStreams <= 2) {
				continue;
			}
			if (locked) {
				continue;
			}
			if (GetResource(desc, it.first, monitor.memoryFree, mbps, mb, fourcc, encoder, &info)) {
				break;
			}
		}
	}
	/// other devices
	if (info.error != load_balancing::ErrorMessage::ok) {
		for (auto &it : m_balancingDevices) {
			if (it.first == BalancingDevice::software) {
				continue;
			}
			for (auto &desc : it.second) {
				if (desc.monitor == 0) {
					continue;
				}
				auto monitor = *desc.monitorInfo.lock();
				auto locked = (encoder) ? monitor.lockedEncoder : monitor.lockedDecoder;
				auto capability = (encoder) ? &desc.encoderDesc : &desc.decoderDesc;
				if (capability->maxStreams > 2 || capability->maxStreams == 0) {
					continue;
				}
				if (locked) {
					continue;
				}
				if (GetResource(desc, it.first, monitor.memoryFree, mbps, mb, fourcc, encoder, &info)) {
					break;
				}
			}
		}
	}
	return info;
}

load_balancing::HandleCodecInfo LoadBalancingHardware::GetSoftwareResource(uint32_t fourcc, uint32_t mbps, uint32_t mb, bool encoder)
{
	load_balancing::HandleCodecInfo info;
	info.error = load_balancing::ErrorMessage::noCpuResources;
	for (auto &it : m_balancingDevices) {
		if (it.first != BalancingDevice::software) {
			continue;
		}
		for (auto &desc : it.second) {
			auto monitor = *desc.monitorInfo.lock();
			if (GetResource(desc, it.first, monitor.memoryFree, mbps, mb, fourcc, encoder, &info)) {
				break;
			}
		}
	}
	return info;
}

bool LoadBalancingHardware::FreeResource(const load_balancing::HandleCodecInfo & info)
{
	auto bd = m_balancingDevices.find(info.device);
	if (bd == m_balancingDevices.end()) {
		return false;
	}
	for (auto & desc : bd->second) {
		if (desc.id != info.id || desc.name != info.name) {
			continue;
		}
		desc.monitorInfo.withLock([&info] (MonitoringInfo& mi) {
			if (info.encoder) {
				mi.lockedEncoder = false;
			}
			else {
				mi.lockedDecoder = false;
			}
		});
		auto capability = (info.encoder) ? &desc.encoderDesc : &desc.decoderDesc;
		capability->activeMBps -= info.mbps;
		capability->activeStreams--;
		UnregisterDeviceContext(info);
		return true;
	}
	return false;
}

void LoadBalancingHardware::RegisterDeviceContext(const load_balancing::HandleCodecInfo &info)
{

#ifdef PORTED_CODE
	if (info.device == BalancingDevice::nvidia && info.ctx) {
		auto res = cuCtxPushCurrent(reinterpret_cast<CUcontext>(info.ctx));
	}
#endif

}

void LoadBalancingHardware::UnregisterDeviceContext(const load_balancing::HandleCodecInfo &info)
{

#ifdef PORTED_CODE
	if (info.device == BalancingDevice::nvidia && info.ctx) {
		if (cuCtxSetCurrent(reinterpret_cast<CUcontext>(info.ctx)) == CUresult::CUDA_SUCCESS) {
			auto res = cuCtxPopCurrent(nullptr);
		}
	}
#endif

}


