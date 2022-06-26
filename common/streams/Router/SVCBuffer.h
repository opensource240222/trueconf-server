#pragma once

#include "../fwd.h"
#include "Buffer.h"
#include "VS_StreamsSVCStatistics.h"
#include "../Protocol.h"

#include <algorithm>
#include <cstdint>
#include <list>
#include <memory>

class CAviFile;

namespace stream {

struct SVCPacket;

class SVCBuffer : public Buffer, public VS_StreamSVCStatistics
{
private:
	VS_StreamCrypter		*m_pCrypter;
	VS_SenderState			*m_pSenderCallback;
	std::list<std::unique_ptr<SVCPacket>> m_lQueuePacket;
	unsigned int			m_uTypeSVC;
	bool					m_bKeyFrame, m_bWaitKey, m_bNeedKey,
							m_bLogicalBuffer, m_bSendTrashData,
							m_bDynChangeFmt,
							m_bChangeSLRestrict;
	unsigned char			m_uLevel;
	int						m_iSndMBps, m_iSndFrameSizeMB;
	int						m_iTLayer, m_iTLayerMax;
	int						m_iSLayer, m_iSLayerMax, m_iSLayerRestrict, m_iSLayerNum;
	int						m_iQLayer, m_iQLayerMax;
	int						m_iReduce, m_iReduceSet, m_iReduceLoad;
	int						m_maxLevelRestrictMB, m_maxWindowRestrictMB;
	unsigned int			m_iState;
	int						m_iBitrateSet;
	int						m_iBytesTrash;
	unsigned				m_iQueueLengthMax;
	int						m_iQueueBytes;
	int						m_iPacketsToConnect;
	unsigned short			m_nTrackFrames[TRACK_ALL];
	unsigned char			m_uFrameId, m_uFrameCount;
	unsigned int			m_uSectionCount;
	int64_t					m_iTimestamp, m_iTimestampPrev;
	std::chrono::steady_clock::time_point m_iVideoTimeout;
	std::chrono::steady_clock::time_point m_uAnalyseTimeRcv;
	std::chrono::steady_clock::time_point m_uAnalyseTimeSnd;
	std::chrono::steady_clock::time_point m_uTimeoutLayer;
	std::chrono::steady_clock::time_point m_uKeyFrameTimeout;
	int						m_iTypeDbgStat;
	FILE*					m_fStat;
	/// func
	void	ReduceQueue(int iLevel);
	void	ReduceTrash();
	void	AnalyseQueuePacket(std::chrono::steady_clock::time_point ctime, bool bFromSnd = false);
	void	UpdateReduceCoefs();
	int		DetectChangeSpatialLayers(std::chrono::steady_clock::time_point ctime, int nsl);
	void	DetectChangeSpatialLayerSize(unsigned char *p, int size);
	int		GetFrameSize(unsigned char *p, int size, int &width, int &height);
	bool	ReduceQueueByTrack(stream::Track track, int bytes_rcv, bool bQueueBtrFull);
	void	EraseQueue(int bytes_rcv, bool bQueueBtrFull);

#ifdef TEST_CRASH

	static const unsigned NUM_SLAYERS = 3;
	FILE			*out_stat[2][NUM_SLAYERS];
	FILE			*out_cmp[2][NUM_SLAYERS];
	std::unique_ptr<CAviFile> m_avifile[2][NUM_SLAYERS];
	unsigned char	m_avi_frame[2][NUM_SLAYERS][640*480*4];
	int				m_size_avi_frame[2][NUM_SLAYERS];
	int64_t			m_tmstp_prev[2][NUM_SLAYERS];
	unsigned char	m_avi_frame_id[2][NUM_SLAYERS];
	bool			m_bKey_Avi[2][NUM_SLAYERS];
	int				m_svc_layer[2][NUM_SLAYERS];
	int				m_num_pckt[2][NUM_SLAYERS];
	int				m_num_frames[2][NUM_SLAYERS];

	void	CouplingFrame(SVCPacket* p, int idx);

#endif

public:
	SVCBuffer(unsigned int modeSVC, bool bDynChangeFmt, unsigned char uLevel, int sndMBps, int sndFrameSizeMB);
	SVCBuffer(unsigned int modeSVC, bool bDynChangeFmt, unsigned char uLevel, int sndMBps, int sndFrameSizeMB, unsigned maxFrames);
	~SVCBuffer();
	/// Buffer interface
	bool Init(string_view conf_name, string_view part_name) override;
	void Destroy(string_view conf_name, string_view part_name) override;
	Status PutFrame(uint32_t tick_count, Track track, std::unique_ptr<char[]>&& buffer, size_t size) override;
	Status GetFrame(uint32_t& tick_count, Track& track, std::unique_ptr<char[]>& buffer, size_t& size) override;
	unsigned GetFrameCount() const override;
	unsigned GetFrameCount(Track track) const override;
	void SetParticipantStatisticsInterface(ParticipantStatisticsInterface* p) override;
	void SetStreamCrypter(VS_StreamCrypter* p) override;

	unsigned int	GetTypeSVC()									 { return m_uTypeSVC; }
	int				GetLevel()										 { return m_uLevel; }
	int				GetSndMBps()									 { return m_iSndMBps; }
	int				GetSndFrameSizeMB()								 { return m_iSndFrameSizeMB; }
	void SetCallbackSender(VS_SenderState* pSender) { m_pSenderCallback = pSender; }
	///	Statictics Interface
	const std::string& ConferenceName() const override;
	const std::string& ParticipantName() const override;
	void	AnalyseTrash();
	void	SetState(unsigned int iState);
	unsigned int GetState()								{ return m_iState; }
	void	SetReduceCoef(int iReduce, int iBitrateSet) { m_iReduceSet = iReduce; m_iBitrateSet = iBitrateSet; }
	void	SetLoadReduceCoef(int iReduce)				{ m_iReduceLoad = iReduce; }
	int		GetLoadReduceCoef()							{ return m_iReduceLoad; }
	int		GetReduceCoef()								{ return m_iReduceSet; }
	void	SetTLayerId(int iLayer);
	void	SetSLayerId(int iLayer);
	void	SetSLayerNum(int iLayerNum);
	int		GetSLayerMaxId()							{ return m_iSLayerMax; }
	int		GetTLayerMaxId()							{ return m_iTLayerMax; }
	int		GetSLayerId()								{ return m_iSLayer; }
	int		GetTLayerId()								{ return m_iTLayer; }
	int		GetInstantQueueBytes()						{ return m_iQueueBytes; }
	int		GetInstantQueueLen()						{ return m_lQueuePacket.size(); }
	bool	IsWaitKeyFrame(std::chrono::steady_clock::time_point ct) override;
	void	SetTypeStat(int iTypeDbgStat)				{ m_iTypeDbgStat = (m_fStat) ? iTypeDbgStat : 0; }
	void	SetLevelRestrictMB(int maxLevelRestrictMB)	{ m_maxLevelRestrictMB = maxLevelRestrictMB; }
	void	SetWindowRestrictMB(int maxWndRestrictMB)	{ m_maxWindowRestrictMB = maxWndRestrictMB; }
	int		GetRestrictFrameMB()						{ return std::min(m_maxLevelRestrictMB, m_maxWindowRestrictMB); }
	int		GetWindowRestrictMB()						{ return m_maxWindowRestrictMB; }
	void	CheckVideoFromSnd(std::chrono::steady_clock::time_point ctime);
};

}
