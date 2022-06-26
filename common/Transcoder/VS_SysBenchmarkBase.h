#pragma once

#include "std/cpplib/VS_VideoLevelCaps.h"

#define MAX_NUM_FRAME	(10)

class VideoCodec;
class VSVideoProcessingIpp;

#define LEVEL_SNDGROUP_RESTRICT (VS_VIDEOLEVEL_32)

/// abstract base class of system benchmark
class VS_SysBenchmarkBase
{
protected:
	tc_VideoLevelCaps	*m_pLevelCaps;
	/// calculate decoder load cpu int %
	int GetLoadDecoder(int garbageLoad, double F);
	/// calculate MBps for group conference
	int GetBenchGroupMBps(int sndMBps, int benchMBps, int garbageLoad, double K);

	bool				m_bInit;
	bool				m_bDecreaseRating;
	int					m_frame_size;
	int					m_res_bench, m_bench_snd, m_bench_rcv, m_bench_rcv_group;
	unsigned char		m_level_idc, m_level_rcv_idc, m_level_rcv_group_idc;
	unsigned int		m_num_lcores, m_num_phcores, m_num_threads;
	unsigned char		*m_pFrameBuffer;
	VideoCodec			*m_pCodec;
	VSVideoProcessingIpp *m_pVProc;
	tc_AutoLevelCaps	*m_pAutoLevelCaps;

	void			InitBenchmark();
	void			Release();
	void			GenerateFrameQueue();
	int				RunBenchmark();
	void			GetNumCpuCores(unsigned int *phcores, unsigned int *lcores);
	unsigned char	GetLevel(unsigned int fourcc, int maxMBps);
	int				CorrectBenchmarkResult(int res_bench);
	void			CalcLevels();
	virtual bool	CheckNeedDecreaseRating();

public :
	VS_SysBenchmarkBase();
	virtual ~VS_SysBenchmarkBase();
	/// get receiver level for p2p
	unsigned char	GetRcvLevel(eHardwareEncoder typeEncoder = ENCODER_SOFTWARE, unsigned int fourcc = 0);
	/// get receiver level for group conferences
	unsigned char	GetRcvGroupLevel(eHardwareEncoder typeEncoder = ENCODER_SOFTWARE, unsigned int fourcc = 0);
	/// get sender level
	unsigned char	GetSndLevel(eHardwareEncoder typeEncoder = ENCODER_SOFTWARE, unsigned int fourcc = 0);
	/// get current benchmark level
	unsigned char	GetBenchLevel(eHardwareEncoder typeEncoder = ENCODER_SOFTWARE, unsigned int fourcc = 0);
	/// get receiver benchmark result in MBps
	int				GetRcvMBps(eHardwareEncoder typeEncoder = ENCODER_SOFTWARE, unsigned int fourcc = 0);
	/// get receiver benchmark result in MBps for group conference
	int				GetRcvGroupMBps(eHardwareEncoder typeEncoder = ENCODER_SOFTWARE, unsigned int fourcc = 0);
	/// get sender benchmark result in MBps
	int				GetSndMBps(eHardwareEncoder typeEncoder = ENCODER_SOFTWARE, unsigned int fourcc = 0);
	/// get benchmark result in MBps
	int				GetBenchMBps(eHardwareEncoder typeEncoder = ENCODER_SOFTWARE, unsigned int fourcc = 0);
};
