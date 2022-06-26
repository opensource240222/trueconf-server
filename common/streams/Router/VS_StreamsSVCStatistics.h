#pragma once

#include "../fwd.h"
#include "../Protocol.h"
#include "../../std/cpplib/VS_VideoLevelCaps.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <map>
#include <vector>

/// criterion detect bound
#define DETECT_BITRATESET_PERCENT	(95)
#define DETECT_BAND_PERCENT			(20)
#define FREEZE_BITRATE_PERCENT 		(10)
#define DETECT_LAYER_PERCENT		(50)
#define DETECT_COEF_REDUCE			(100)
#define DETECT_KEYFRAME_PERCENT		(300)
#define DETECT_QCLEAR_PERCENT		(700)
#define DETECT_LAYERUP_PERCENT		(10)
#define DETECT_LAYERUP_QLEN			(2)
#define DETECT_MAX_QLEN				(500)
#define DETECT_COEF_DATA			(90)
#define DETECT_SYSLOAD_HIGH			(85)
#define DETECT_SYSLOAD_LOW			(70)
#define DETECT_COEF_LIMIT			(5)
/// criterion timeout
///
#define BANDWIDTH_MIN			(10)
#define QBYTES_MIN				(8192) /// 64 kbps
///
#define MAX_NUM_LAYERS			(20)
#define MAX_NUM_REDUCE			(100+1)
///
#define MAX_WND_SIZE			(5)
#define MAX_WND_SENDER			(20)

class VS_StreamBuffer;
class VS_StreamSVCBuffer;

static int avg_coef_stat[4][5] =
{
	{ 0,  5, 10, 15, 20}, /// for undef bandwidth
	{10, 10, 10, 10, 10}, /// for update bitrate up
	{ 6,  6,  6,  6, 26}, /// for freeze bitrate up
	{ 8,  8,  8,  9, 17}, /// for reduce buffer
};

enum eSVCMethods
{
	SVC_TEMPORAL = 0,
	SVC_SPATIAL = 1,
	SVC_QUALITY = 2,
	SVC_MAX,
};

enum eBufferState
{
	BUFFER_CONNECT		= 0x00000001,
	BUFFER_READY		= 0x00000002,
	DISABLE_VIDEO		= 0x00000004,
	DISABLE_DATA		= 0x00000008,
	NOT_VIDEO_FROM_SND	= 0x00000010,
	DATA_FROM_SND		= 0x00000020,
	BUFFER_WORST		= 0x00000040,
	DATA_TRASH			= 0x00000080,
	SYSLOAD_INIT		= 0x00000100,
};

static const size_t TRACK_ALL = 256;
static const size_t TRACK_MAX = 257;

enum eParticipantState
{
	PART_UNDEF			= 0x00000000,
	PART_IDLE			= 0x00000001,
	PART_RESTRICT		= 0x00000002,
	PART_FIRSTUPPER		= 0x00000004,
	PART_SECONDUPPER	= 0x00000008,
	PART_THIRDUPPER		= 0x00000010,
	PART_FREEZE			= 0x00000020,
};

enum eTypeMBsize
{
	MBSIZE_WINDOW	= 0,
	MBSIZE_LEVEL	= 1,
	MBSIZE_MAX
};

class VS_StreamSVCStatistics
{
private :
	stream::StreamStatistics* m_pStat;
	int											m_iNumIterates, m_iCountStat;
	int											m_iQueueLenAvg, m_iQueueBytesAvg;
	int											m_iBytesPerSec[TRACK_MAX], m_iBytesPerSecTmp[MAX_WND_SIZE][TRACK_MAX], m_iBytesPerSecLast;
	int											m_qQueueLen[MAX_WND_SIZE], m_qQueueBytes[MAX_WND_SIZE];

protected :
	stream::ParticipantStatisticsInterface* m_pRouterStat;

public :
	VS_StreamSVCStatistics();
	virtual ~VS_StreamSVCStatistics();
	int				Init();
	virtual void SetState(unsigned int iState) = 0;
	virtual unsigned int GetState() = 0;
	virtual void SetReduceCoef(int iReduce, int iBitrateSet) = 0;
	virtual void SetLoadReduceCoef(int iReduce) = 0;
	virtual int GetLoadReduceCoef() = 0;
	virtual int GetReduceCoef() = 0;
	virtual void SetTLayerId(int iLayer) = 0;
	virtual void SetSLayerId(int iLayer) = 0;
	virtual void SetSLayerNum(int iLayerNum) = 0;
	virtual int GetSLayerMaxId() = 0;
	virtual int GetTLayerMaxId() = 0;
	virtual int GetSLayerId() = 0;
	virtual int GetTLayerId() = 0;
	virtual bool IsWaitKeyFrame(std::chrono::steady_clock::time_point ct) = 0;
	virtual const std::string& ConferenceName() const = 0;
	virtual const std::string& ParticipantName() const = 0;
	virtual int GetInstantQueueBytes() = 0;
	virtual int GetInstantQueueLen() = 0;
	virtual void SetTypeStat(int m_iTypeDbgStat) = 0;
	virtual void AnalyseTrash() = 0;
	virtual void SetLevelRestrictMB(int maxLevelRestrictMB) = 0;
	virtual void SetWindowRestrictMB(int maxWndRestrictMB) = 0;
	virtual int GetRestrictFrameMB() = 0;
	virtual int GetWindowRestrictMB() = 0;
	virtual void CheckVideoFromSnd(std::chrono::steady_clock::time_point ctime) = 0;
	int				GetInstantBytes()							{ return m_iBytesPerSecLast; }
	int				GetQueueBytesAvg()							{ return m_iQueueBytesAvg; }
	int				GetQueueLenAvg()							{ return m_iQueueLenAvg; }
	int				GetBytes()									{ return m_iBytesPerSec[TRACK_ALL]; }
	int				GetVideoBytes()								{ return m_iBytesPerSec[id(stream::Track::video)]; }
	int				GetAudioBytes()								{ return m_iBytesPerSec[id(stream::Track::audio)]; }
	int				GetDataBytes()								{ return m_iBytesPerSec[id(stream::Track::data)]; }
	void			UpdateRcvStatistics();
	void			UpdateSndStatistics();
};

struct VS_SenderState;
struct VS_ParticipantState;
class  VS_MainSVCStatistics;

struct fnCompare
{
	bool operator() (const char *name0, const char *name1) const { return strcmp(name0, name1) < 0; }
};

typedef std::map<char*, VS_StreamSVCStatistics*, fnCompare>			mapStatBuffers;
typedef std::map<char*, VS_SenderState*, fnCompare>					mapSender;
typedef std::map<char*, VS_ParticipantState*, fnCompare>			mapParticipant;
typedef std::map<const char*, VS_MainSVCStatistics*, fnCompare>		mapMainStat;

class VS_MainSVCStatistics
{
private :
	static mapMainStat							m_mapStatInstance;
	static int									m_iBitrateSndMediaMax;
	static int									m_iBitrateSndMax;
	static int									m_iBitratePartMin;
	static int									m_iBitrateDataPartMin;
	static tc_VideoLevelCaps					m_LevelCaps;
	mapStatBuffers								m_mapStreamSndLogical;
	mapStatBuffers								m_mapSVCBuffers;
	mapSender									m_mapStreamSnd;
	mapParticipant								m_mapStreamPart;
	std::chrono::steady_clock::time_point		m_iBitrateTime;
	std::chrono::steady_clock::time_point		m_uTrashTime;
	bool										m_changeRenderMB;
	FILE										*m_fStat;
public :
	static void  RegisterStreamBuffer(stream::Buffer* pBuffer);
	static void  UnRegisterStreamBuffer(stream::Buffer* pBuffer);
	static void  UpdateStatistics(stream::ConferencesConditions* pCallback);
	static void	 UpdateTrashData(const char *conferenceName, std::chrono::steady_clock::time_point ctime);
	static void  UpdateSystemLoad(const char *conferenceName, const char *participantName, long sysLoad);
	static void  UpdateFrameSizeMB(const char *conferenceName, const char *participantNameTo, const std::vector<stream::ParticipantFrameSizeInfo> &mb);
	static FILE* GetStatisticsFile(const char *conferenceName);
	static void	 CleanStatDirectory();
private :
	VS_MainSVCStatistics(const char *conferenceName);
	~VS_MainSVCStatistics();
	void Release();
	void AddStreamBuffer(const char* name, stream::SVCBuffer* pBuffer, unsigned int uTypeSVC, int maxMBps, int sndMBps, int sndFrameSizeMB);
	bool RemoveStreamBuffer(const char* name, VS_StreamSVCStatistics *pBuffer);
	void BitrateControl(const char *conferenceName, stream::ConferencesConditions* pCallback);
	void TrashDataProcess(std::chrono::steady_clock::time_point ctime);
	FILE* GetStatFile() { return m_fStat; }
	void PredictionBitrateBuffer(VS_ParticipantState *pPart, int iActiveSnds, int iDataSnds, int iDisableSnds, bool bHalfVideo, unsigned short btrAudioOnPart);
	int CorrectBitrateBuffer(VS_ParticipantState *pPart, unsigned short minBufferData, int iActiveSnds, int iDataSnds);
	void CorrectRatingLayersBuffers(VS_ParticipantState *pPart);
	void AnalysisRcvRating(int maxMBps);
	void SetSystemLoadParticipants(const char *participantName, long sysLoad);
	void SetFrameSizeMBParticipant(const char *participantNameTo, const char *participantNameFrom, long frameSizeMB);
};

struct VS_SenderState
{

private :

	int								 m_maxMBps, m_maxFrameSizeMB;
	int								 m_iLayersStatNum[SVC_MAX];
	int								 m_iLayersNum[SVC_MAX];
	int64_t							 m_iLayerBytes[MAX_NUM_LAYERS][MAX_NUM_LAYERS];
	int								 m_iReduceCoefs[MAX_NUM_LAYERS*MAX_NUM_LAYERS];
	double							 m_fMBpsRatio[MAX_NUM_LAYERS*MAX_NUM_LAYERS];
	double							 m_fFrameSizeMBRatio[MBSIZE_MAX][MAX_NUM_LAYERS];
	int								 m_numSlayers;
	int								 m_iStateLayers[MAX_NUM_REDUCE][3];
	int								 m_iReduceCoefNum;
	std::chrono::steady_clock::time_point m_uTimeLayerStat;
	int								 m_iBytesPerSec[TRACK_MAX];
	bool							 m_bDataSend;
	bool							 m_bVideoSend;
	bool							 m_bAudioSend;
	int							     m_iBaseBitrate;
	int							     m_iBaseBitrateTmp[MAX_WND_SENDER];
	int								 m_iNumIterates;
	VS_StreamSVCStatistics			 *m_pStatBuffer;
	int								 m_iTypeDbgStat;
	FILE*							 m_fStat;

public :

	std::vector<VS_StreamSVCStatistics*> vStatBuffer;
	std::vector<VS_ParticipantState*>	 vParts;
	int								 iLastSetBitrate;
	int								 iLastDataBitrate;
	unsigned int					 uTypeSVC;
	bool							 bEmulateSVC;

	VS_SenderState(unsigned int aTypeSVC, int aBitrateSndMax, int aBitrateMediaSndMax, int maxMBps, int maxFrameSizeMB);
	~VS_SenderState() {};
	void CalculateLayerStat(int size, int sl, int tl, int msl);
	bool UpdateLayerStat(std::chrono::steady_clock::time_point ctime, FILE *fStat);
	void ResetLayerStat(std::chrono::steady_clock::time_point ctime, int dLayers, int maxSLayers);
	int ResetFrameLayerCoef(int sndFrameSizeMB, int sndFrameWidth, int sndFrameHeight, int partFrameSizeMB);
	void SetStatBuffer(VS_StreamSVCStatistics *pBuffer);
	void UpdateStatistics();
	int MBps2ReduceCoef(int maxMBps, std::chrono::steady_clock::time_point ctime);
	int FrameSizeMB2SLayer(int frameSizeMB);
	int WindowMB2LayerMB(int wndMB);
	int LevelMB2LayerMB(int lvlMB);
	int ChangeLoadCoef(int iReduce, bool bCutting);
	int GetLayerBytes(int iSLayer, int iTLayer);
	void SetCurBaseBitrate(int baseBitrate);
	void SetTypeStat(int iTypeDbgStat)	{ m_iTypeDbgStat = (m_fStat) ? iTypeDbgStat : 0; }
	int GetBaseBitrate()				{ return m_iBaseBitrate; }
	int GetMaxNumSLayers()				{ return m_numSlayers; }
	int	GetNumLayers(int iType)			{ return m_iLayersNum[iType]; }
	int	GetNumReduceCoef()				{ return m_iReduceCoefNum; }
	int	GetReduceCoef(int index)		{ return m_iReduceCoefs[index]; }
	int	GetSLayerId(int iReduce)		{ return m_iStateLayers[iReduce][0]; }
	int	GetTLayerId(int iReduce)		{ return m_iStateLayers[iReduce][1]; }
	int	GetBaseReduce(int iReduce)		{ return m_iStateLayers[iReduce][2]; }
	int	GetBytes()						{ return m_iBytesPerSec[TRACK_ALL]; }
	int	GetDataBytes()					{ return m_iBytesPerSec[id(stream::Track::data)]; }
	int	GetAudioBytes()					{ return m_iBytesPerSec[id(stream::Track::audio)]; }
	int	GetVideoBytes()					{ return m_iBytesPerSec[id(stream::Track::video)]; }
	bool IsDataSend()					{ return m_bDataSend; }
	bool IsVideoSend()					{ return m_bVideoSend; }
	int GetMaxMBps()					{ return m_maxMBps; }
	int GetFrameSizeMB()				{ return m_maxFrameSizeMB; }
};
