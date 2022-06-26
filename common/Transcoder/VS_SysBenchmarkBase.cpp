#include "VS_SysBenchmarkBase.h"
#include "Transcoder/VideoCodec.h"
#include "VSClient/VS_ApplicationInfo.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_Utils.h"
#include <math.h>
#include "Transcoder/VS_RetriveVideoCodec.h"
#include "IppLib2/VSVideoProcessingIpp.h"
#include <thread>
#include "std-generic/cpplib/ThreadUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../std/cpplib/VS_Cpu.h"

#ifdef __cplusplus
}
#endif

#define BW						(640)
#define BH						(480)
#define CPU_GARBAGE_LOAD		(30)

VS_SysBenchmarkBase::VS_SysBenchmarkBase()
{
	m_pLevelCaps = new tc_VideoLevelCaps();
	m_bInit = false;
	m_bDecreaseRating = CheckNeedDecreaseRating();
	m_pCodec = 0;
	m_pVProc = 0;
	m_pFrameBuffer = 0;
	m_res_bench = 40000;
	m_pAutoLevelCaps = new tc_AutoLevelCaps();
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	key.GetValue(&m_res_bench, sizeof(m_res_bench), VS_REG_INTEGER_VT, "Invent");
	m_res_bench = CorrectBenchmarkResult(m_res_bench);
	int loadDec = GetLoadDecoder(CPU_GARBAGE_LOAD, 2.4);
	m_bench_snd = (m_res_bench * (100 - CPU_GARBAGE_LOAD - loadDec)) / 100;
	m_bench_rcv = (m_res_bench * loadDec * 3) / 100;
	m_bench_rcv_group = GetBenchGroupMBps(m_bench_snd, m_res_bench, CPU_GARBAGE_LOAD, 3);
	VS_GetNumCPUCores(&m_num_phcores, &m_num_lcores);
	m_num_threads = m_num_phcores + (m_num_lcores / (m_num_phcores << 1)) * (m_num_phcores >> 1);
	m_level_idc = GetLevel(VS_VCODEC_VPX, m_bench_snd);
	m_level_rcv_idc = GetLevel(VS_VCODEC_VPX, m_bench_rcv);
	m_level_rcv_group_idc = GetLevel(VS_VCODEC_VPX, m_bench_rcv_group);
	srand(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}

VS_SysBenchmarkBase::~VS_SysBenchmarkBase()
{
	delete m_pLevelCaps;
}

int VS_SysBenchmarkBase::GetLoadDecoder(int garbageLoad, double F)
{
	return (int)((100.0 - garbageLoad) / (1.0 + F));
}

int VS_SysBenchmarkBase::GetBenchGroupMBps(int sndMBps, int benchMBps, int garbageLoad, double K)
{
	int rcvGroupMBps = 0;
	unsigned char lvl = m_pLevelCaps->MBps2Level(sndMBps);
	tc_levelVideo_t descLvl;
	lvl = std::min<unsigned char>(m_pLevelCaps->MBps2Level(sndMBps), LEVEL_SNDGROUP_RESTRICT);
	m_pLevelCaps->GetLevelDesc(lvl, &descLvl);
	rcvGroupMBps = (int)(0.7 * K * (benchMBps * (100 - garbageLoad) / 100 - descLvl.maxMBps));
	return rcvGroupMBps;
}

int	VS_SysBenchmarkBase::CorrectBenchmarkResult(int res_bench)
{
	if (m_bDecreaseRating) res_bench = res_bench * 95 / 100;
	return res_bench;
}

void VS_SysBenchmarkBase::InitBenchmark()
{
	Release();
	m_pVProc = new VSVideoProcessingIpp;
	if (m_pVProc) {
		m_pCodec = VS_RetriveVideoCodec(VS_VCODEC_VPX, true);
		if (m_pCodec) {
			m_frame_size = BW * BH * 3 / 2;
			m_pFrameBuffer = new unsigned char[MAX_NUM_FRAME * m_frame_size];
			m_bInit = true;
		}
	}
}

void VS_SysBenchmarkBase::Release()
{
	m_bInit = false;
	delete m_pCodec; m_pCodec = 0;
	delete m_pVProc; m_pVProc = 0;
	delete [] m_pFrameBuffer; m_pFrameBuffer = 0;
}

void VS_SysBenchmarkBase::GenerateFrameQueue()
{
	int i = 0, j = 0;
	/// first frame
	for (i = 0; i < m_frame_size; i++) {
		m_pFrameBuffer[i] = (rand() * 256) / RAND_MAX;
	}
	int size_rsmpl = (int)((MAX_NUM_FRAME * 0.1 + 2) * (MAX_NUM_FRAME * 0.1 + 2) * BW * BH * 3 / 2 + 0.5);
	uint8_t *pResample = new uint8_t[size_rsmpl];
	double k = 1.1;
	int w = 0, h = 0, pitch_u = 0, pitch_v = 0;
	for (i = 1; i < MAX_NUM_FRAME; i++) {
		w = ((int)(k * BW + 0.5) / 16) * 16;
		h = ((int)(k * BH + 0.5) / 16) * 16;
		m_pVProc->ResampleI420(m_pFrameBuffer, BW, BH, pResample, w, h);
		unsigned char *pCrop = pResample;
		unsigned char *pDst = m_pFrameBuffer + i * m_frame_size;
		for (j = 0; j < BH; j ++) {
			memcpy(pDst, pCrop, BW);
			pCrop += w;
			pDst += BW;
		}
		for (j = 0; j < BH; j ++) {
			memcpy(pDst, pCrop, BW / 2);
			pCrop += w / 2;
			pDst += BW / 2;
		}
		k += 0.1;
	}
	delete [] pResample;
}


void VS_SysBenchmarkBase::CalcLevels()
{
	uint64_t rnd = VS_GenKeyByMD5();
	int dt = (1000 - ((rnd * 2000) >> 32));
	vs::SleepFor(std::chrono::milliseconds(5000 + dt)); /// wait 5 sec for start benchmark

	GenerateFrameQueue();
	int res = (int)RunBenchmark();
	VS_RegistryKey key(true, REG_CurrentConfiguratuon, false, true);
	m_res_bench = 0;
	key.GetValue(&m_res_bench, sizeof(m_res_bench), VS_REG_INTEGER_VT, "Invent");
	if (m_res_bench == 0) {
		m_res_bench = res;
	}
	else if (m_res_bench > res) {
		m_res_bench = (75 * m_res_bench + 25 * res) / 100;
	}
	else {
		m_res_bench = (m_res_bench + res) / 2;
	}
	key.SetValue(&m_res_bench, sizeof(m_res_bench), VS_REG_INTEGER_VT, "Invent");
	m_res_bench = CorrectBenchmarkResult(m_res_bench);
	int loadDec = GetLoadDecoder(CPU_GARBAGE_LOAD, 1.8);
	m_bench_snd = (m_res_bench * (100 - CPU_GARBAGE_LOAD - loadDec)) / 100;
	m_bench_rcv = (m_res_bench * loadDec * 3) / 100;
	m_bench_rcv_group = GetBenchGroupMBps(m_bench_snd, m_res_bench, CPU_GARBAGE_LOAD, 3);
	m_level_idc = GetLevel(VS_VCODEC_VPX, m_bench_snd);
	m_level_rcv_idc = GetLevel(VS_VCODEC_VPX, m_bench_rcv);
	m_level_rcv_group_idc = GetLevel(VS_VCODEC_VPX, m_bench_rcv_group);
}

bool VS_SysBenchmarkBase::CheckNeedDecreaseRating()
{
	return false;
}

int VS_SysBenchmarkBase::RunBenchmark()
{
	int i = 0, repeat = 9;
	double k = 3.95; /// coef for rnd frame encoding
	int maxMBps = 0;
	int fps = 0;
	int bitrate = 1024 * 10 / 30; /// to 10 fps
	std::chrono::high_resolution_clock::time_point st, ct;
	VS_VideoCodecParam param;
	memset(&param, 0, sizeof(VS_VideoCodecParam));
	unsigned char *out = new unsigned char [3*1024*1024]; /// 3MB

	m_pCodec->Init(BW, BH, FOURCC_I420, 0, m_num_threads);
	m_pCodec->SetBitrate(bitrate);

	st = std::chrono::high_resolution_clock::now();
	for (i = 0; i < MAX_NUM_FRAME * repeat; i++) {
		m_pCodec->Convert(m_pFrameBuffer + (i % MAX_NUM_FRAME) * m_frame_size, out, &param);
	}
	ct = std::chrono::high_resolution_clock::now();

	double sec = std::chrono::duration_cast<std::chrono::nanoseconds>(ct - st).count() / (double)std::chrono::nanoseconds::period::den;
	fps = (int)((repeat * MAX_NUM_FRAME * k) / (sec));
	maxMBps = (BW * BH * fps) / 256;

	//DTRACE(VSTM_VIDEO, "RUN BENCH: r = %8d, (%d x %d) @ %d, phcores = %d, lcores = %d, threads = %d",
	//					maxMBps, BW, BH, fps, m_num_phcores, m_num_lcores, m_num_threads);

	delete [] out; out = 0;
	Release();

	return maxMBps;
}

void VS_SysBenchmarkBase::GetNumCpuCores(unsigned int *phcores, unsigned int *lcores)
{
	*phcores = m_num_phcores;
	*lcores = m_num_lcores;
}

unsigned char VS_SysBenchmarkBase::GetLevel(unsigned int fourcc, int maxMBps)
{
	unsigned char level = m_pLevelCaps->MBps2Level(maxMBps);
	tc_levelVideo_t l;
	m_pLevelCaps->GetLevelDesc(level, &l);

	//DTRACE(VSTM_VIDEO, "GET LEVEL %s : r = %8d, mbps = %8d, mbframe = %5d", l.name, maxMBps, l.maxMBps, l.maxFrameSizeMB);

	return level;
}

unsigned char VS_SysBenchmarkBase::GetRcvLevel(eHardwareEncoder typeEncoder, unsigned int fourcc)
{
	return m_level_rcv_idc;
}

unsigned char VS_SysBenchmarkBase::GetRcvGroupLevel(eHardwareEncoder typeEncoder, unsigned int fourcc)
{
	return m_level_rcv_group_idc;
}

unsigned char VS_SysBenchmarkBase::GetSndLevel(eHardwareEncoder typeEncoder, unsigned int fourcc)
{
	const auto lvl = std::min<unsigned char>(m_level_idc, VIDEO_LEVEL_MAX);
	return (typeEncoder == ENCODER_SOFTWARE) ? lvl : m_pAutoLevelCaps->GetMaxLevel();
}

unsigned char VS_SysBenchmarkBase::GetBenchLevel(eHardwareEncoder typeEncoder, unsigned int fourcc)
{
	return (typeEncoder == ENCODER_SOFTWARE) ? m_level_idc : m_pAutoLevelCaps->GetMaxLevel();
}

int	VS_SysBenchmarkBase::GetRcvMBps(eHardwareEncoder typeEncoder, unsigned int fourcc)
{
	return m_bench_rcv;
}

int	VS_SysBenchmarkBase::GetRcvGroupMBps(eHardwareEncoder typeEncoder, unsigned int fourcc)
{
	return m_bench_rcv_group;
}

int	VS_SysBenchmarkBase::GetSndMBps(eHardwareEncoder typeEncoder, unsigned int fourcc)
{
	return m_bench_snd;
}

int	VS_SysBenchmarkBase::GetBenchMBps(eHardwareEncoder typeEncoder, unsigned int fourcc)
{
	return m_res_bench;
}
