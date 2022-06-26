/**
 **************************************************************************
 * \file VS_StreamsSVCStatistics.cpp
 * (c) 2012 TrueConf LLC
 * \brief Implementation of SVC Server processing statistic
 *
 * \b Project Server
 * \author SAnufriev
 * \date 10.02.2012
 ****************************************************************************/

#include "SVCBuffer.h"
#include "ConferencesConditions.h"
#include "StatisticsInterface.h"
#include "Types.h"
#include "../VS_StreamsDefinitions.h"
#include "../VS_StreamsSVCTypes.h"
#include "../Statistics.h"
#include <math.h>
#include <boost/filesystem.hpp>
#include <sys/stat.h>
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"
#include "../../std/cpplib/VS_MemoryLeak.h"
#include "../../std/cpplib/VS_RcvFunc.h"
#include "std/cpplib/VS_Utils.h"
#include "std/Globals.h"
#include "std-generic/clib/vs_time.h"
#include "std/debuglog/VS_Debug.h"

#ifdef TEST_SPEED
#include "../../std/VS_ProfileTools.h"
#endif

#define DEBUG_CURRENT_MODULE VS_DM_STREAMS

#define UNDEF_BANDWIDTH (unsigned short)(-1)

static const auto STAT_BITRATECALC_TM = std::chrono::seconds(2);
static const auto STAT_BITRATEFREEZE_TM = std::chrono::seconds(6);
static const auto STAT_BITRATEUP_TM	= std::chrono::seconds(12);
static const auto STAT_REDUCEUPD_WAIT_TM = std::chrono::seconds(30);
static const auto STAT_REDUCEUPD_TM = std::chrono::seconds(60);
static const auto STAT_TRASH_TM = std::chrono::milliseconds(50);
static const auto STAT_SYSLOAD_TM = std::chrono::seconds(12);

typedef mapStatBuffers::iterator						itStatBuffers;
typedef mapSender::iterator								itSender;
typedef mapParticipant::iterator						itParticipant;
typedef std::vector<VS_StreamSVCStatistics*>::iterator	itBuffer;
typedef std::pair<char*, VS_StreamSVCStatistics*>		pairStatBuffers;
typedef std::pair<char*, VS_SenderState*>				pairSender;
typedef std::pair<char*, VS_ParticipantState*>			pairParticipant;
typedef std::pair<itSender, bool>						pairSenderIns;
typedef std::pair<itParticipant, bool>					pairParticipantIns;
typedef std::pair<itStatBuffers, bool>					pairStatBuffersIns;
typedef mapMainStat::iterator							itMainStat;
typedef std::pair<const char*, VS_MainSVCStatistics*>	pairMainStat;
typedef std::pair<itMainStat, bool>						pairMainStatIns;

mapMainStat VS_MainSVCStatistics::m_mapStatInstance;
tc_VideoLevelCaps VS_MainSVCStatistics::m_LevelCaps;
int VS_MainSVCStatistics::m_iBitrateSndMediaMax = 640;
int VS_MainSVCStatistics::m_iBitrateSndMax = 2048;
int VS_MainSVCStatistics::m_iBitratePartMin = 48;
int VS_MainSVCStatistics::m_iBitrateDataPartMin = 100;
const char SVC_LOGS_DIRECTORY_TAG[] = "svc_logs";

VS_SenderState::VS_SenderState(unsigned int aTypeSVC, int aBitrateSndMax, int aBitrateMediaSndMax, int maxMBps, int maxFrameSizeMB)
{
	m_maxMBps = maxMBps;
	m_maxFrameSizeMB = maxFrameSizeMB;
	uTypeSVC = aTypeSVC;
	bEmulateSVC = (aTypeSVC == SVC_NONE);
	iLastSetBitrate = aBitrateMediaSndMax;
	iLastDataBitrate = aBitrateSndMax - aBitrateMediaSndMax;
	m_uTimeLayerStat = std::chrono::steady_clock::time_point{};
	m_numSlayers = 1;
	if (uTypeSVC & SVC_2S_MODE) m_numSlayers = 2;
	if (uTypeSVC & SVC_3S_MODE) {
		m_numSlayers = 3;
		int thrMB = 3600;
		if (uTypeSVC & SVC_SMOD_MODE) {
			thrMB = 1600;
		}
		if (maxFrameSizeMB < thrMB) {
			m_numSlayers = 2;
			if (maxFrameSizeMB < 880) {
				m_numSlayers = 1;
			}
		}
	}
	for (int s = 0; s < MAX_NUM_LAYERS; s++) {
		for (int t = 0; t < MAX_NUM_LAYERS; t++) {
			m_iReduceCoefs[s*MAX_NUM_LAYERS+t] = 100;
			m_fMBpsRatio[s*MAX_NUM_LAYERS+t] = 0xffff;
			m_iLayerBytes[s][t] = 0;
		}
		m_fFrameSizeMBRatio[MBSIZE_WINDOW][s] = ((1ull << (2 * s)) * 100.0) / 200.0;
		m_fFrameSizeMBRatio[MBSIZE_LEVEL][s] = maxFrameSizeMB / (1ull << (2 * s));
	}
	if (!bEmulateSVC) m_iReduceCoefs[0] = 55;
	for (int l = 0; l < SVC_MAX; l++) {
		m_iLayersStatNum[l] = 0;
		m_iLayersNum[l] = 1;
	}
	for (int l = 0; l < MAX_NUM_REDUCE; l++) {
		m_iStateLayers[l][0] = 0;
		m_iStateLayers[l][1] = 2;
		m_iStateLayers[l][2] = l;
	}
	m_iReduceCoefNum = 1;
	memset(m_iBytesPerSec, 0, TRACK_MAX * sizeof(int));
	m_bDataSend = false;
	m_bVideoSend = false;
	m_pStatBuffer = 0;
	m_iNumIterates = 0;
	memset(m_iBaseBitrateTmp, 0, MAX_WND_SIZE * sizeof(int));
	m_iBaseBitrate = 0;
	m_iTypeDbgStat = 0;
}

void VS_SenderState::SetStatBuffer(VS_StreamSVCStatistics *pBuffer)
{
	m_pStatBuffer = pBuffer;
	if (m_pStatBuffer) {
		m_fStat = VS_MainSVCStatistics::GetStatisticsFile(m_pStatBuffer->ConferenceName().c_str());
		if (m_fStat) {
			fprintf(m_fStat, "\n %s : CREATE sender %s : svc = 0x%08x, maxMBps = %d, maxFrameSizeMB = %d",
								m_pStatBuffer->ConferenceName().c_str(), m_pStatBuffer->ParticipantName().c_str(), uTypeSVC, m_maxMBps, m_maxFrameSizeMB);
			m_iTypeDbgStat = 1;
		}
	}
}

void VS_SenderState::CalculateLayerStat(int size, int sl, int tl, int msl) {
	m_iLayersStatNum[SVC_SPATIAL] = msl;
	if (sl == 0 && m_iLayersStatNum[SVC_TEMPORAL] <= tl) m_iLayersStatNum[SVC_TEMPORAL]++;
	m_iLayerBytes[sl][tl] += size;
}

void VS_SenderState::ResetLayerStat(std::chrono::steady_clock::time_point ctime, int dLayers, int maxSLayers)
{
	m_uTimeLayerStat = ctime;
	for (int s = 0; s < MAX_NUM_LAYERS; s++)
		for (int t = 0; t < MAX_NUM_LAYERS; t++)
			m_iLayerBytes[s][t] = 0;
	m_numSlayers = maxSLayers;
}

int VS_SenderState::ResetFrameLayerCoef(int sndFrameSizeMB, int sndFrameWidth, int sndFrameHeight, int partFrameSizeMB) {
	if (m_maxFrameSizeMB != sndFrameSizeMB) {
		for (int s = 0; s < MAX_NUM_LAYERS; s++) {
			m_fFrameSizeMBRatio[MBSIZE_LEVEL][s] = sndFrameSizeMB / (1ull << (2 * s));
		}
		m_maxMBps = m_maxMBps / m_maxFrameSizeMB * sndFrameSizeMB;
		m_maxFrameSizeMB = sndFrameSizeMB;
		if (m_iTypeDbgStat > 0 && m_fStat) {
			fprintf(m_fStat, "\n UPDATE sender base layer %s : maxMBps = %d, maxFrameSizeMB = %d (%4d x %4d), numSL = %d",
								m_pStatBuffer->ParticipantName().c_str(), m_maxMBps, m_maxFrameSizeMB, sndFrameWidth, sndFrameHeight, m_numSlayers);
		}
	}
	return FrameSizeMB2SLayer(partFrameSizeMB);
}

bool VS_SenderState::UpdateLayerStat(std::chrono::steady_clock::time_point ctime, FILE *fStat) {
	if (m_uTimeLayerStat == std::chrono::steady_clock::time_point{})
		m_uTimeLayerStat = ctime;
	auto dt = ctime - m_uTimeLayerStat;
	if (dt < STAT_REDUCEUPD_WAIT_TM || dt > STAT_REDUCEUPD_TM) {
		int s = 0, t = 0, i = 0;
		int64_t iBytes[MAX_NUM_LAYERS][MAX_NUM_LAYERS] = {0};
		int nSLayers = m_iLayersStatNum[SVC_SPATIAL];
		int nTLayers = 0;
		if (nSLayers > 0) {
			nTLayers = m_iLayersStatNum[SVC_TEMPORAL];
			m_iLayersNum[SVC_SPATIAL] = nSLayers;
			m_iLayersNum[SVC_TEMPORAL] = nTLayers;
			m_iReduceCoefNum = nSLayers * nTLayers;
		}
		double fInitTReduce[5] = {0.55, 0.75, 1.0,  1.0,   1.0};
		double fInitSReduce[5] = {1.0,  0.4,  0.16, 0.064, 0.064};
		int iLastIndex = 100;
		for (s = 0; s < nSLayers; s++) {
			for (t = 0; t < nTLayers; t++) {
				iBytes[s][t] = m_iLayerBytes[s][t];
				if (t > 0) iBytes[s][t] += iBytes[s][t-1];
			}
			double ks = fInitSReduce[s];
			if (iBytes[0][nTLayers-1] != 0 && iBytes[s][nTLayers-1] != 0) ks = (double)iBytes[s][nTLayers-1] / (double)iBytes[0][nTLayers-1];
			for (t = nTLayers - 1; t >= 0; t--) {
				double kt = fInitTReduce[t];
				if (iBytes[s][t] != 0) kt = (double)iBytes[s][t] / (double)iBytes[s][nTLayers-1];
				int iReduce = (int)(kt * ks * 100 + 0.5);
				if (iReduce > 100) iReduce = 100;
				int index = s * nTLayers + (nTLayers - 1) - t;
				m_iReduceCoefs[index] = (m_iReduceCoefs[index] + iReduce + 1) / 2;
				for (i = m_iReduceCoefs[index]; i <= iLastIndex; i++) {
					m_iStateLayers[i][0] = s;
					m_iStateLayers[i][1] = t;
					m_iStateLayers[i][2] = m_iReduceCoefs[index];
				}
				if (m_iReduceCoefs[index] > iLastIndex) m_iReduceCoefs[index] = iLastIndex;
				else iLastIndex = m_iReduceCoefs[index] - 1;
			}
		}
		for (i = 0; i <= iLastIndex; i++) {
			m_iStateLayers[i][0] = nSLayers - 1;
			m_iStateLayers[i][1] = -1;
			m_iStateLayers[i][2] = 0;
		}
		if (dt > STAT_REDUCEUPD_TM)
			m_uTimeLayerStat = ctime - STAT_REDUCEUPD_WAIT_TM;
		if (uTypeSVC == 0x00000200) { /// clients with first svc revision: 3 temporal
			m_fMBpsRatio[0] = 1.0;
			m_fMBpsRatio[1] = 1.5;
			m_fMBpsRatio[2] = 3.0;
		} else if (uTypeSVC == 0x00010100 || uTypeSVC == 0x00020100) { /// clients  with second svc revision: 2 spatial, 2 temporal
			for (s = 0; s < nSLayers; s++) {
				m_fMBpsRatio[s*2+0] = 1 << 2 * s;
				m_fMBpsRatio[s*2+1] = 1 << (2 * s + 1);
			}
		} else if (uTypeSVC != 0x00000000) {
			for (s = 0; s < nSLayers; s++) {
				int ratio = 1 << (2 * s);
				for (t = 0; t < nTLayers; t++) {
					m_fMBpsRatio[s*nTLayers+t] = ratio + t * 2.5 / (double)nTLayers;
				}
			}
		}
		return true;
	}
	return false;
}

void VS_SenderState::SetCurBaseBitrate(int baseBitrate)
{
	int idx = m_iNumIterates % MAX_WND_SENDER;
	int nCount = (m_iNumIterates > MAX_WND_SENDER) ? MAX_WND_SENDER : m_iNumIterates;
	m_iBaseBitrateTmp[idx] = baseBitrate;
	m_iBaseBitrate = 0;
	for (int i = 0; i < nCount; i++) {
		m_iBaseBitrate += m_iBaseBitrateTmp[i];
	}
	m_iBaseBitrate /= MAX_WND_SENDER;
	m_iNumIterates++;
}

int VS_SenderState::WindowMB2LayerMB(int wndMB)
{
	double ratio = (double)m_maxFrameSizeMB / (double)wndMB;
	int i = 0;
	for (i = m_numSlayers - 1; i >= 0; i--) {
		if (ratio >= m_fFrameSizeMBRatio[MBSIZE_WINDOW][i]) return (int)m_fFrameSizeMBRatio[MBSIZE_LEVEL][i];
	}
	return (int)m_fFrameSizeMBRatio[MBSIZE_LEVEL][0];
}

int VS_SenderState::LevelMB2LayerMB(int lvlMB)
{
	int layerMB = 0;
	for (int i = 0; i < m_numSlayers; i++) {
		layerMB = (int)m_fFrameSizeMBRatio[MBSIZE_LEVEL][i];
		if (lvlMB >= layerMB) break;
	}
	return layerMB;
}

int VS_SenderState::FrameSizeMB2SLayer(int frameSizeMB)
{
	for (int i = 0; i < m_numSlayers; i++) {
		if (frameSizeMB >= m_fFrameSizeMBRatio[MBSIZE_LEVEL][i]) return i;
	}
	return m_numSlayers - 1;
}

int VS_SenderState::MBps2ReduceCoef(int maxMBps, std::chrono::steady_clock::time_point ctime)
{
	if (ctime - m_uTimeLayerStat > STAT_REDUCEUPD_WAIT_TM) {
		double ratio = (double)m_maxMBps / (double)maxMBps;
		for (int i = 0; i < m_iReduceCoefNum; i++) {
			if (ratio <= m_fMBpsRatio[i]) return m_iReduceCoefs[i];
		}
		return 100;
	}
	return -1;
}

int VS_SenderState::ChangeLoadCoef(int iReduce, bool bCutting)
{
	int nTL = GetNumLayers(SVC_TEMPORAL);
	int nSL = GetNumLayers(SVC_SPATIAL);
	int cTL = GetTLayerId(iReduce);
	int cSL = GetSLayerId(iReduce);
	if (bCutting) {
		cTL--;
		if (cTL < 0) {
			cTL = 0;
			cSL++;
			if (cSL >= (nSL - 1)) cSL = nSL - 1;
		}
	} else {
		cSL--;
		if (cSL < 0) {
			cSL = 0;
			cTL++;
			if (cTL >= (nTL - 1)) cTL = nTL - 1;
		}
	}
	int idx = cSL * nTL + (nTL - 1) - cTL;
	return GetReduceCoef(idx);
}

void VS_SenderState::UpdateStatistics()
{
	if (!m_pStatBuffer) return;
	m_pStatBuffer->UpdateSndStatistics();

#ifdef TEST_SPEED
	AUTO_PROF
#endif

	m_iBytesPerSec[TRACK_ALL]  = m_pStatBuffer->GetBytes();
	m_iBytesPerSec[id(stream::Track::video)] = m_pStatBuffer->GetVideoBytes();
	m_iBytesPerSec[id(stream::Track::data)] = m_pStatBuffer->GetDataBytes();
	m_iBytesPerSec[id(stream::Track::audio)] = m_pStatBuffer->GetAudioBytes();
	int btrHiSpatial = m_iBytesPerSec[TRACK_ALL];
	for (int i = 1; i < m_iLayersNum[SVC_SPATIAL]; i++) {
		btrHiSpatial -= (m_iReduceCoefs[i*m_iLayersNum[SVC_TEMPORAL]] * m_iBytesPerSec[TRACK_ALL]) / 100;
	}
	if (btrHiSpatial < 0) btrHiSpatial = 0;
	m_bDataSend = ((btrHiSpatial * 5) < (m_iBytesPerSec[id(stream::Track::data)] * 100));
	m_bVideoSend = (m_iBytesPerSec[id(stream::Track::video)] != 0);
	m_bAudioSend = (m_iBytesPerSec[id(stream::Track::audio)] != 0);
}

int VS_SenderState::GetLayerBytes(int iSLayer, int iTLayer)
{
	return 0;
}

struct VS_ParticipantState
{
	std::vector<VS_StreamSVCStatistics*> vStatBuffer;
	std::vector<VS_SenderState*>		 vSnds;
	unsigned short					 uPhBandwidth;
	unsigned short					 uHighPhBandwidth, uLowPhBandwidth;
	unsigned short					 uCalcBandwidth;
	int								 maxMBps;
	int								 maxLevelRestrictMB;
	int								 sysLoad;
	std::chrono::steady_clock::time_point iOverflowTime;
	std::chrono::steady_clock::time_point iPhBandwidthChangeTime;
	std::chrono::steady_clock::time_point iSysLoadTime;
	bool							 bFirstOverBitrate;
	int								 iCoefLoad;
	const char						 *namePart;
	/// avg temporary stat
	unsigned short					 uBufferBitrateVideo;
	unsigned short					 uBufferBitrateData;
	unsigned short					 uBufferBitrate;
	unsigned short					 uMaxBufferBitrateVideo;
	int								 iNumActiveSnds;	/// all active senders
	int								 iNumDataSnds;		/// data and/or video senders
	int								 iNumDisableSnds;	/// disabled senders
	int								 iAccBufferBitrate;
	unsigned int					 uAvgQueueBytes;
	unsigned int					 uAvgQueueLen;
	unsigned int					 uAvgRcvBytes;
	unsigned int					 uAvgDataRcvBytes;
	unsigned int					 uAvgAudioRcvBytes;
	unsigned int					 eState;
	int								 iCoefUpper;

public :

	void UpdateSysLoad(std::chrono::steady_clock::time_point ctime, FILE *fStat, int iLimitReduce);

};

void VS_ParticipantState::UpdateSysLoad(std::chrono::steady_clock::time_point ctime, FILE *fStat, int iLimitReduce)
{
	int i = 0;
	if (iNumActiveSnds > 0) {
		int iReduce = 0, newReduce = 0;;
		int numSnds = vStatBuffer.size();
		if (iLimitReduce == -1) {
			int MBpsOnSnd = maxMBps / iNumActiveSnds;
			for (i = 0; i < numSnds; i++) {
				unsigned int uState = vStatBuffer[i]->GetState();
				if (!(uState & SYSLOAD_INIT) && (uState & BUFFER_READY)) {
					if (!(vStatBuffer[i]->GetState() & (DISABLE_VIDEO | DATA_FROM_SND | NOT_VIDEO_FROM_SND))) {
						iReduce = vSnds[i]->MBps2ReduceCoef(MBpsOnSnd, ctime);
						if (iReduce >= 0) {
							vStatBuffer[i]->SetLoadReduceCoef(iReduce);
							vStatBuffer[i]->SetState(SYSLOAD_INIT);
							fprintf(fStat, "\n SET LOAD COEFS %s: %d", vStatBuffer[i]->ParticipantName().c_str(), iReduce);
							if ((sysLoad > DETECT_SYSLOAD_HIGH) && (maxMBps / MBpsOnSnd > 5)) {
								vStatBuffer[i]->SetState(DISABLE_VIDEO);
								fprintf(fStat, ", DISABLE BUFFER");
							}
						}
					}
				}
			}
			if (ctime - iSysLoadTime >= STAT_SYSLOAD_TM) {
				if (sysLoad > DETECT_SYSLOAD_HIGH || sysLoad < DETECT_SYSLOAD_LOW) {
					for (i = 0; i < numSnds; i++) {
						if (vStatBuffer[i]->GetState() & SYSLOAD_INIT) {
							iReduce = vStatBuffer[i]->GetLoadReduceCoef();
							newReduce = vSnds[i]->ChangeLoadCoef(iReduce, (sysLoad > DETECT_SYSLOAD_HIGH));
							if (newReduce != iReduce) {
								vStatBuffer[i]->SetLoadReduceCoef(newReduce);
								fprintf(fStat, "\n CHANGE LOAD COEFS %s: %d -> %d", vStatBuffer[i]->ParticipantName().c_str(), iReduce, newReduce);
							}
						}
					}
				}
				iSysLoadTime = ctime;
			}
		} else {
			for (i = 0; i < numSnds; i++) {
				int setLimitReduce = iLimitReduce;
				int numReduce = vSnds[i]->GetNumReduceCoef();
				if (setLimitReduce >= numReduce) setLimitReduce = numReduce - 1;
				int iReduce = vSnds[i]->GetReduceCoef(setLimitReduce);
				vStatBuffer[i]->SetLoadReduceCoef(iReduce);
			}
		}
	}
}

void VS_MainSVCStatistics::RegisterStreamBuffer(stream::Buffer* pBuffer)
{
	auto pSVCBuffer = dynamic_cast<stream::SVCBuffer*>(pBuffer);
	VS_StreamSVCStatistics *pStatBuffer = dynamic_cast <VS_StreamSVCStatistics*> (pBuffer);
	if (pSVCBuffer && pStatBuffer) {
		itMainStat it = m_mapStatInstance.find(pSVCBuffer->ConferenceName().c_str());
		if (it == m_mapStatInstance.end()) {
			VS_MainSVCStatistics *pStatInstance = new VS_MainSVCStatistics(pSVCBuffer->ConferenceName().c_str());
			char *name = new char [VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			strcpy((char*)name, pSVCBuffer->ConferenceName().c_str());
			pairMainStatIns ins = m_mapStatInstance.emplace(name, pStatInstance);
			it = ins.first;
		}
		tc_levelVideo_t descLvl;
		unsigned char lvl = pSVCBuffer->GetLevel();
		if (lvl > 0) {
			lvl = m_LevelCaps.CheckLevel(lvl);
			m_LevelCaps.GetLevelDesc(lvl, &descLvl);
		} else {
			m_LevelCaps.GetLevelDesc(m_LevelCaps.GetMaxLevel(), &descLvl);
		}
		it->second->AddStreamBuffer(pSVCBuffer->ParticipantName().c_str(),
									pSVCBuffer,
									pSVCBuffer->GetTypeSVC(),
									descLvl.maxMBps,
									pSVCBuffer->GetSndMBps(),
									pSVCBuffer->GetSndFrameSizeMB());
	}
}

void VS_MainSVCStatistics::UnRegisterStreamBuffer(stream::Buffer* pBuffer)
{
	stream::SVCBuffer *pSVCBuffer = dynamic_cast <stream::SVCBuffer*> (pBuffer);
	VS_StreamSVCStatistics *pStatBuffer = dynamic_cast <VS_StreamSVCStatistics*> (pBuffer);
	if (pSVCBuffer && pStatBuffer) {
		itMainStat it = m_mapStatInstance.find(pSVCBuffer->ConferenceName().c_str());
		if (it != m_mapStatInstance.end()) {
			if (it->second->RemoveStreamBuffer(pSVCBuffer->ParticipantName().c_str(), pStatBuffer)) {
				delete [] it->first;
				delete it->second; it->second = 0;
				m_mapStatInstance.erase(it);
			}
		}
	}
}

void VS_MainSVCStatistics::UpdateStatistics(stream::ConferencesConditions* pCallback)
{
	for (itMainStat it = m_mapStatInstance.begin(), et = m_mapStatInstance.end(); it != et; it++) {
		it->second->BitrateControl(it->first, pCallback);
	}
}

void VS_MainSVCStatistics::UpdateTrashData(const char *conferenceName, std::chrono::steady_clock::time_point ctime)
{
	itMainStat it = m_mapStatInstance.find(conferenceName);
	if (it == m_mapStatInstance.end()) return;
	it->second->TrashDataProcess(ctime);
}

FILE* VS_MainSVCStatistics::GetStatisticsFile(const char *conferenceName)
{
	itMainStat it = m_mapStatInstance.find(conferenceName);
	if (it == m_mapStatInstance.end()) return 0;
	return it->second->GetStatFile();
}

void VS_MainSVCStatistics::UpdateSystemLoad(const char *conferenceName, const char *participantName, long sysLoad)
{
	itMainStat it = m_mapStatInstance.find(conferenceName);
	if (it == m_mapStatInstance.end()) return;
	it->second->SetSystemLoadParticipants(participantName, sysLoad);
}

void VS_MainSVCStatistics::UpdateFrameSizeMB(const char *conferenceName, const char *participantNameTo, const std::vector<stream::ParticipantFrameSizeInfo> &mb)
{
	if (!conferenceName || !participantNameTo)
		return;
	itMainStat it = m_mapStatInstance.find(conferenceName);
	if (it == m_mapStatInstance.end())
		return;
	for (const auto &pit : mb) {
		it->second->SetFrameSizeMBParticipant(participantNameTo, pit.participant_name_from.c_str(), pit.mb);
	}
}

void VS_MainSVCStatistics::CleanStatDirectory()
{
	const auto log_dir = vs::GetLogDirectory() + SVC_LOGS_DIRECTORY_TAG;
	boost::system::error_code ec;
	boost::filesystem::create_directories(log_dir, ec);
	if (ec)
	{
		dstream1 << "Can't create directory '" << log_dir << "': " << ec.message();
		return;
	}

	time_t now;
	time(&now);

	uintmax_t limitStatSize;
	time_t limitStatTime;
	{
		uint32_t value;
		VS_RegistryKey key(true, CONFIGURATION_KEY);
		if (key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "maxStatFileSizeMB") > 0)
			limitStatSize = static_cast<uintmax_t>(value) * 1024 * 1024;
		else
			limitStatSize = 1024 * 1024 * 1024;
		if (key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "maxStatFileTimeDay") > 0)
			limitStatTime = static_cast<time_t>(value) * 24 * 3600;
		else
			limitStatTime = 100 * 24 * 3600;
	}

	boost::filesystem::recursive_directory_iterator it(log_dir, ec);
	if (ec)
	{
		dstream1 << "Can't iterate through directory '" << log_dir << "': " << ec.message();
		return;
	}

	struct FileInfo
	{
		FileInfo(const boost::filesystem::path& path_, uintmax_t size_, time_t mtime_) : path(path_), size(size_), mtime(mtime_) {}
		boost::filesystem::path path;
		uintmax_t size;
		time_t mtime;
	};
	std::vector<FileInfo> svc_files;
	uintmax_t totalSize = 0;
	for ( ; it != boost::filesystem::recursive_directory_iterator{}; ++it)
	{
		const auto& path = it->path();
		if (path.extension() != ".txt")
			continue;

		const auto mtime = boost::filesystem::last_write_time(path, ec);
		if (ec)
		{
			dstream1 << "Can't get mtime of file '" << path << "': " << ec.message();
			// We prefer to not delete a file rather that delete a recent one.
			// So we ignore this file as we don't know its modification time.
			continue;
		}

		// Delete files older than the limit immediately.
		if (mtime <= now && now - mtime >= limitStatTime)
		{
			boost::filesystem::remove(path, ec);
			if (ec)
			{
				dstream1 << "Can't remove file '" << path << "': " << ec.message();
				// If the deletion fails we still need to get the file size to calculate the total size, so no "continue" here.
			}
			else
				continue;
		}

		const auto size = boost::filesystem::file_size(path, ec);
		if (ec)
		{
			dstream1 << "Can't get size of file '" << path << "': " << ec.message();
			continue;
		}
		totalSize += size;
		svc_files.emplace_back(path, size, mtime);
	}

	if (totalSize >= limitStatSize)
	{
		std::sort(svc_files.begin(), svc_files.end(), [](const FileInfo& l, const FileInfo& r) { return l.mtime < r.mtime; });
		for (const auto& x : svc_files)
		{
			if (x.mtime >= now - 24 * 3600)
				break; // Don't delete files less that one day old.

			boost::filesystem::remove(x.path, ec);
			if (ec)
			{
				dstream1 << "Can't remove file '" << x.path << "': " << ec.message();
				continue;
			}
			assert(x.size <= totalSize);
			totalSize -= x.size;
			if (totalSize < limitStatSize)
				break;
		}
	}
}

VS_MainSVCStatistics::VS_MainSVCStatistics(const char *conferenceName)
{
	m_mapSVCBuffers.clear();
	m_mapStreamSndLogical.clear();
	m_mapStreamPart.clear();
	m_mapStreamSnd.clear();
	m_iBitrateTime = std::chrono::steady_clock::time_point{};
	m_uTrashTime = std::chrono::steady_clock::now();
	m_changeRenderMB = false;

	const auto log_dir = vs::GetLogDirectory() + SVC_LOGS_DIRECTORY_TAG;
	boost::system::error_code ec;
	boost::filesystem::create_directories(log_dir, ec);
	if (!ec) {
		char name[512] = {0};
		sprintf(name, "%s/vs_stat_svc_%s.txt", log_dir.c_str(), conferenceName);
		m_fStat = fopen(name, "a");
		if (m_fStat) {
			char chtime[32];
			time_t curt;
			time(&curt);
			tm curt_tm;
			strftime(chtime, 30, "%d/%m/%Y %H:%M:%S", localtime_r(&curt, &curt_tm));
			fprintf(m_fStat, "\n %20s  CREATE : conference %s", chtime, name);
		}
	}
}

VS_MainSVCStatistics::~VS_MainSVCStatistics()
{
	if (m_fStat) {
		char chtime[32];
		time_t curt;
		time(&curt);
		tm curt_tm;
		strftime(chtime, 30, "%d/%m/%Y %H:%M:%S", localtime_r(&curt, &curt_tm));
		fprintf(m_fStat, "\n %20s  KILL : conference", chtime);
		fclose(m_fStat);
		m_fStat = 0;
	}
	Release();
}

void VS_MainSVCStatistics::Release()
{
	/// clean svc buffers map
	for (itStatBuffers it = m_mapSVCBuffers.begin(), et = m_mapSVCBuffers.end(); it != et; ) {
		delete [] it->first;
		it = m_mapSVCBuffers.erase(it);
	}
	m_mapSVCBuffers.clear();
	/// clean logical senders map
	for (itStatBuffers it = m_mapStreamSndLogical.begin(), et = m_mapStreamSndLogical.end(); it != et; ) {
		delete [] it->first;
		it = m_mapStreamSndLogical.erase(it);
	}
	m_mapStreamSndLogical.clear();
	/// clean senders map
	for (itSender it = m_mapStreamSnd.begin(), et = m_mapStreamSnd.end(); it != et; ) {
		delete [] it->first;
		delete it->second;
		it = m_mapStreamSnd.erase(it);
	}
	m_mapStreamSnd.clear();
	/// clean parts map
	for (itParticipant it = m_mapStreamPart.begin(), et = m_mapStreamPart.end(); it != et; ) {
		delete [] it->first;
		delete it->second;
		it = m_mapStreamPart.erase(it);
	}
	m_mapStreamPart.clear();
}

void VS_MainSVCStatistics::AddStreamBuffer(const char *name, stream::SVCBuffer* pBuffer, unsigned int uTypeSVC, int maxMBps, int sndMBps, int sndFrameSizeMB)
{
	char *pdst = strstr((char*)name, "-<%%>-");
	char *pname_buffer = new char [VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME+1];

	pairStatBuffersIns it_insert_buffer;

	if (pdst == 0) {
		strcpy(pname_buffer, name);
		it_insert_buffer = m_mapStreamSndLogical.emplace(pname_buffer, pBuffer);
		if (!it_insert_buffer.second) delete [] pname_buffer;
		{
			char *pname_snd = new char[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			strcpy(pname_snd, name);
			auto pSender = new VS_SenderState(uTypeSVC, m_iBitrateSndMax, m_iBitrateSndMediaMax, sndMBps, sndFrameSizeMB);
			auto it_insert_snd = m_mapStreamSnd.emplace(pname_snd, pSender);
			auto pSenderIn = it_insert_snd.first->second;
			if (!it_insert_snd.second) {
				delete pSender;
				delete[] pname_snd;
			}
			else {
				pSenderIn->SetStatBuffer(it_insert_buffer.first->second);
			}
		}
		return;
	}

	char *pname_snd = new char [VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME+1];

	int num_snd_name = pdst - name;
	pdst += 6;

	char *pname_part = new char [VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME+1];

	strcpy(pname_buffer, name);
	strncpy(pname_part, name, num_snd_name);
	pname_part[num_snd_name] = 0;
	strcpy(pname_snd, pdst);

	it_insert_buffer = m_mapSVCBuffers.emplace(pname_buffer, pBuffer);
	if (!it_insert_buffer.second) delete [] pname_buffer;

	VS_SenderState *pSenderIn = 0;
	VS_ParticipantState *pPartIn = 0;
	VS_SenderState *pSender = new VS_SenderState(uTypeSVC, m_iBitrateSndMax, m_iBitrateSndMediaMax, sndMBps, sndFrameSizeMB);

	VS_ParticipantState *pPart = new VS_ParticipantState();
	pairSenderIns it_insert_snd;
	pairParticipantIns it_insert_part;

	it_insert_snd = m_mapStreamSnd.emplace(pname_snd, pSender);
	pSenderIn = it_insert_snd.first->second;
	pSenderIn->vStatBuffer.push_back(pBuffer);

	if (!it_insert_snd.second) {
		delete pSender;
		delete [] pname_snd;
	} else {
		itStatBuffers itl = m_mapStreamSndLogical.find(it_insert_snd.first->first);
		if (itl != m_mapStreamSndLogical.end()) pSenderIn->SetStatBuffer(itl->second);
	}

	pBuffer->SetCallbackSender(pSenderIn);

	int maxLevelFrameMB = 0;
	if (sndMBps > 0 && sndFrameSizeMB > 0) {
		maxLevelFrameMB = maxMBps / (sndMBps / sndFrameSizeMB);
	}
	pBuffer->SetLevelRestrictMB(3600);
	pBuffer->SetWindowRestrictMB(900);
	pBuffer->SetSLayerId(pSenderIn->FrameSizeMB2SLayer(900));
	pBuffer->SetSLayerNum(pSenderIn->GetMaxNumSLayers());

	it_insert_part = m_mapStreamPart.emplace(pname_part, pPart);
	pPartIn = it_insert_part.first->second;
	pPartIn->vStatBuffer.push_back(pBuffer);
	if (it_insert_part.second) {
		pPartIn->uPhBandwidth = UNDEF_BANDWIDTH;
		pPartIn->uHighPhBandwidth = UNDEF_BANDWIDTH;
		pPartIn->uLowPhBandwidth = UNDEF_BANDWIDTH;
		pPartIn->uCalcBandwidth = UNDEF_BANDWIDTH;
		pPartIn->uBufferBitrateVideo = UNDEF_BANDWIDTH;
		pPartIn->uBufferBitrateData = UNDEF_BANDWIDTH;
		pPartIn->uBufferBitrate = UNDEF_BANDWIDTH;
		pPartIn->uMaxBufferBitrateVideo = UNDEF_BANDWIDTH;
		pPartIn->maxMBps = maxMBps;
		pPartIn->maxLevelRestrictMB = maxLevelFrameMB;
		pPartIn->sysLoad = 70;
		pPartIn->iNumActiveSnds = 0;
		pPartIn->iNumDataSnds = 0;
		pPartIn->iNumDisableSnds = 0;
		pPartIn->iAccBufferBitrate = 0;
		pPartIn->iOverflowTime = std::chrono::steady_clock::time_point{};
		pPartIn->iSysLoadTime = std::chrono::steady_clock::time_point{};
		pPartIn->iCoefUpper = 0;
		pPartIn->namePart = pname_part;
	} else {
		delete pPart;
		delete [] pname_part;
	}

	pSenderIn->vParts.push_back(pPartIn);
	pPartIn->vSnds.push_back(pSenderIn);
}

bool VS_MainSVCStatistics::RemoveStreamBuffer(const char *name, VS_StreamSVCStatistics *pBuffer)
{
	char *pdst = strstr((char*)name, "-<%%>-");
	char name_snd[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME+1] = {0};

	if (pdst == 0) {
		itStatBuffers itl_snd = m_mapStreamSndLogical.find((char*)name);
		if (itl_snd != m_mapStreamSndLogical.end()) {
			itSender it_snd = m_mapStreamSnd.find((char*)name);
			if (it_snd != m_mapStreamSnd.end()) {
				it_snd->second->SetStatBuffer(0);
				if (it_snd->second->vStatBuffer.empty()) {
					delete[] it_snd->first;
					delete it_snd->second;
					m_mapStreamSnd.erase(it_snd);
				}
			}
			delete [] itl_snd->first;
			m_mapStreamSndLogical.erase(itl_snd);
		}
		return m_mapStreamSnd.empty() && m_mapStreamPart.empty() && m_mapStreamSndLogical.empty();
	}

	itStatBuffers it_buffer = m_mapSVCBuffers.find((char*)name);
	if (it_buffer != m_mapSVCBuffers.end()) {
		delete [] it_buffer->first;
		m_mapSVCBuffers.erase(it_buffer);
	}

	int num_snd_name = pdst - name;
	pdst += 6;

	char name_part[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME+1] = {0};
	strncpy(name_part, name, num_snd_name);
	name_part[num_snd_name] = 0;
	strcpy(name_snd, pdst);

	int num = 0, i = 0;
	itSender it_snd = m_mapStreamSnd.find(name_snd);
	if (it_snd != m_mapStreamSnd.end()) {
		VS_SenderState *pSender = it_snd->second;
		num = pSender->vStatBuffer.size();
		itBuffer it_buffer = pSender->vStatBuffer.begin();
		std::vector<VS_ParticipantState*>::iterator it = pSender->vParts.begin();
		for (i = 0; i < num; i++) {
			if (pSender->vStatBuffer[i] == pBuffer) break;
			it_buffer++;
			it++;
		}
		pSender->vStatBuffer.erase(it_buffer);
		pSender->vParts.erase(it);
		if (pSender->vStatBuffer.empty()) {
			auto ls = m_mapStreamSndLogical.find(it_snd->first);
			if (ls == m_mapStreamSndLogical.end()) {
				delete[] it_snd->first;
				delete it_snd->second;
				m_mapStreamSnd.erase(it_snd);
			}
		}
	}
	itParticipant it_part = m_mapStreamPart.find((char*)name_part);
	if (it_part != m_mapStreamPart.end()) {
		VS_ParticipantState *pPart = it_part->second;
		num = pPart->vStatBuffer.size();
		itBuffer it_buffer = pPart->vStatBuffer.begin();
		std::vector<VS_SenderState*>::iterator it = pPart->vSnds.begin();
		for (i = 0; i < num; i++) {
			if (pPart->vStatBuffer[i] == pBuffer) break;
			it_buffer++;
			it++;
		}
		pPart->vStatBuffer.erase(it_buffer);
		pPart->vSnds.erase(it);
		if (pPart->vStatBuffer.empty()) {
			delete [] it_part->first;
			delete it_part->second;
			m_mapStreamPart.erase(it_part);
		}
	}
	return m_mapStreamSnd.empty() && m_mapStreamPart.empty() && m_mapStreamSndLogical.empty();
}

void VS_MainSVCStatistics::TrashDataProcess(std::chrono::steady_clock::time_point ctime)
{

#ifdef TEST_SPEED
	AUTO_PROF
#endif

	auto dt = ctime - m_uTrashTime;
	if (dt >= STAT_TRASH_TM) {
		for (itSender is = m_mapStreamSnd.begin(), es = m_mapStreamSnd.end(); is != es; is++) {
			int num_part = is->second->vStatBuffer.size();
			for (int i = 0; i < num_part; i++) {
				is->second->vStatBuffer[i]->AnalyseTrash();
			}
		}
		m_uTrashTime = ctime;
	}
}

void VS_MainSVCStatistics::PredictionBitrateBuffer(VS_ParticipantState *pPart, int iActiveSnds, int iDataSnds, int iDisableSnds, bool bHalfVideo, unsigned short btrAudioOnPart)
{

#ifdef TEST_SPEED
	AUTO_PROF
#endif

	pPart->iNumActiveSnds = iActiveSnds;
	pPart->iNumDataSnds = iDataSnds;
	pPart->iNumDisableSnds = iDisableSnds;
	int btrOnPart = m_iBitrateSndMax * iActiveSnds;
	if (pPart->uCalcBandwidth != UNDEF_BANDWIDTH) {
		btrOnPart = pPart->uCalcBandwidth;
		btrOnPart -= btrAudioOnPart;
		if (btrOnPart < BANDWIDTH_MIN) btrOnPart = BANDWIDTH_MIN;
	}
	unsigned short	btrOnBuffer = static_cast<uint16_t>(std::min<int32_t>(btrOnPart, UNDEF_BANDWIDTH)),
					btrOnBufferData = m_iBitrateSndMax - m_iBitrateSndMediaMax,
					btrOnBufferVideo = static_cast<uint16_t>(std::min<int32_t>(btrOnPart, pPart->uMaxBufferBitrateVideo));
	if (iActiveSnds > 0) {
		btrOnBuffer = btrOnPart / iActiveSnds;
		if (iDataSnds > 0) {
			btrOnBufferVideo = btrOnBuffer;
			if (bHalfVideo) btrOnBufferData = btrOnBuffer / 2; /// video & data : 50 / 50
			else btrOnBufferData = btrOnBuffer;
			int btrOnVideo = btrOnBuffer - btrOnBufferData;
			if (btrOnBufferData < m_iBitrateDataPartMin) {
				btrOnVideo = std::max(0, btrOnBuffer - m_iBitrateDataPartMin);
				btrOnBufferData = m_iBitrateDataPartMin;
				if (btrOnVideo < m_iBitratePartMin) {
					btrOnBufferData += btrOnVideo;
					btrOnBuffer = btrOnBufferData;
				}
			} else if (btrOnVideo < m_iBitratePartMin) {
				btrOnBufferData = btrOnBuffer;
			} else if (btrOnVideo > pPart->uMaxBufferBitrateVideo) {
				btrOnBufferData = btrOnBuffer - pPart->uMaxBufferBitrateVideo;
			}
		} else {
			btrOnBufferVideo = std::min(btrOnBuffer, pPart->uMaxBufferBitrateVideo);
		}
	}
	pPart->uBufferBitrate = btrOnBuffer;
	pPart->uBufferBitrateVideo = btrOnBufferVideo;
	pPart->uBufferBitrateData = btrOnBufferData;
}

int VS_MainSVCStatistics::CorrectBitrateBuffer(VS_ParticipantState *pPart, unsigned short minBufferData, int iActiveSnds, int iDataSnds)
{

#ifdef TEST_SPEED
	AUTO_PROF
#endif

	int iTryDisable = 0, iTrySnds = 0;
	int iOnlyVideoSnds = iActiveSnds;
	if (iActiveSnds > 0) {
		int btrOnPart = iActiveSnds * pPart->uBufferBitrateVideo;
		int btrOnPartVideo = btrOnPart;
		if (iDataSnds > 0) {
			if (pPart->uBufferBitrate == pPart->uBufferBitrateData) {
				btrOnPartVideo = btrOnPart - iDataSnds * minBufferData;
				if (btrOnPartVideo < 0) btrOnPartVideo = 0;
				pPart->uBufferBitrate = minBufferData;
				pPart->uBufferBitrateData = minBufferData;
			}
			iOnlyVideoSnds = iActiveSnds - iDataSnds;
		}
		iTrySnds = btrOnPartVideo / m_iBitratePartMin;
		if (iTrySnds > iOnlyVideoSnds) iTrySnds = iOnlyVideoSnds;
		iTryDisable = iOnlyVideoSnds - iTrySnds;
		pPart->uBufferBitrateVideo = btrOnPartVideo / std::max(iTrySnds, 1);
		if (iDataSnds == 0) pPart->uBufferBitrate = btrOnPartVideo;
	}
	pPart->uBufferBitrateVideo = std::min(pPart->uBufferBitrateVideo, pPart->uMaxBufferBitrateVideo);
	pPart->uBufferBitrate = std::min<decltype(pPart->uBufferBitrate)>(pPart->uBufferBitrate, m_iBitrateSndMax);
	return iTryDisable;
}

void VS_MainSVCStatistics::CorrectRatingLayersBuffers(VS_ParticipantState * pPart)
{
	int32_t dMB(0);
	int32_t maxFrameSizeWndMB(0);
	for (const auto& x : pPart->vStatBuffer) {
		maxFrameSizeWndMB += x->GetWindowRestrictMB();
	}
	for (int32_t i = 0; i < pPart->vStatBuffer.size(); i++) {
		auto buffer = pPart->vStatBuffer[i];
		auto sender = pPart->vSnds[i];
		int32_t layerMB(0),
				wndMB(buffer->GetWindowRestrictMB());
		if (pPart->maxLevelRestrictMB < maxFrameSizeWndMB) {
			int32_t lvlMB = (pPart->maxLevelRestrictMB * wndMB) / maxFrameSizeWndMB + dMB;
			layerMB = sender->LevelMB2LayerMB(lvlMB);
			dMB = (lvlMB - layerMB);
		}
		else {
			layerMB = wndMB;
		}
		buffer->SetLevelRestrictMB(layerMB);
		int32_t restrictMB = buffer->GetRestrictFrameMB();
		int32_t SLayer = sender->FrameSizeMB2SLayer(restrictMB);
		buffer->SetSLayerId(SLayer);
	}
}

void VS_MainSVCStatistics::AnalysisRcvRating(int maxMBps)
{
	int ratingSnds[32][4] = {};

	int i = 0, k = 0, N = 0;

	int sndLayer[4] = {0, 0, 0, 0};
	int maxSndLayer = 0, dMBps = 0;
	int MBpsLayer[4] = {13200, 6600, 3300, 1650};

	for (k = 0; k < 4; k++) {
		if (maxSndLayer == 0) {
			maxSndLayer = maxMBps / MBpsLayer[k];
			dMBps = maxMBps % MBpsLayer[k];
			if (k == 3) sndLayer[k] = std::max(1, maxSndLayer);
			else sndLayer[k] = maxSndLayer;
		} else {
			sndLayer[k] = dMBps / MBpsLayer[k];
			dMBps %= MBpsLayer[k];
		}
	}

	for (N = 0, k = 0; k < 4; k++)  N += sndLayer[k];

	for (i = 0; i < 32; i++) {
		if ((N - sndLayer[3]) != 0) {
			while (N < (i + 1)) {
				for (k = 0; k < 4; k++) if (sndLayer[k] != 0) break;
				if (k == 3) break;
				sndLayer[k+1] += 2;
				sndLayer[k]--;
				for (N = 0, k = 0; k < 4; k++)  N += sndLayer[k];
			}
		}
		for (k = 0; k < 4; k++) ratingSnds[i][k] = sndLayer[k];
	}
}

void VS_MainSVCStatistics::SetSystemLoadParticipants(const char *participantName, long sysLoad)
{
	itParticipant it = m_mapStreamPart.find((char*)participantName);
	if (it == m_mapStreamPart.end()) return;
	if (sysLoad < 0) sysLoad = 70;
	it->second->sysLoad = sysLoad;
}

void VS_MainSVCStatistics::SetFrameSizeMBParticipant(const char *participantNameTo, const char *participantNameFrom, long frameSizeMB)
{
	char partName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME+1];
	VS_RcvFunc::SetName(partName, participantNameTo, participantNameFrom);

	itSender it_s = m_mapStreamSnd.find((char*)participantNameFrom);
	if (it_s == m_mapStreamSnd.end()) {
		return;
	}
	itStatBuffers it_b = m_mapSVCBuffers.find(partName);
	if (it_b == m_mapSVCBuffers.end()) {
		return;
	}
	auto buffer = it_b->second;
	int layerMB(it_s->second->WindowMB2LayerMB(frameSizeMB));
	if (buffer->GetWindowRestrictMB() == layerMB) {
		if (m_fStat) {
			fprintf(m_fStat, "\n CHANGE frame size MB %s : mb_wnd = %ld", partName, frameSizeMB);
		}
	}
	else {
		buffer->SetWindowRestrictMB(layerMB);
		m_changeRenderMB = true;
		if (m_fStat) {
			fprintf(m_fStat, "\n CHANGE frame size MB %s : mb_wnd = %ld, mb_layer = %d", partName, frameSizeMB, layerMB);
		}
	}
}

void VS_MainSVCStatistics::BitrateControl(const char *conferenceName, stream::ConferencesConditions* pCallback)
{
	for (itParticipant it = m_mapStreamPart.begin(), e = m_mapStreamPart.end(); it != e; it++) {
		for (const auto& x : it->second->vStatBuffer) {
			x->UpdateRcvStatistics();
		}
		if (!m_changeRenderMB) {
			continue;
		}
		CorrectRatingLayersBuffers(it->second);
	}
	for (itSender it = m_mapStreamSnd.begin(), e = m_mapStreamSnd.end(); it != e; it++) {
		it->second->UpdateStatistics();
	}
	if (!pCallback || m_mapStreamSnd.empty()) {
		return;
	}
	auto ctime = std::chrono::steady_clock::now();
	if ((ctime - m_iBitrateTime < STAT_BITRATECALC_TM) && !m_changeRenderMB) {
		return;
	}
	m_changeRenderMB = false;

#ifdef TEST_SPEED
	AUTO_PROF
#endif

	int i, j, l;
	VS_RegistryKey key(true, CONFIGURATION_KEY);
	key.GetValue(&m_iBitrateSndMax, sizeof(m_iBitrateSndMax), VS_REG_INTEGER_VT, "Max Client All Bitrate");
	if (key.GetValue(&m_iBitrateSndMediaMax, sizeof(m_iBitrateSndMediaMax), VS_REG_INTEGER_VT, "Max Client Bitrate")) {
		if (m_iBitrateSndMediaMax < 48) m_iBitrateSndMediaMax = 48;
	}
	key.GetValue(&m_iBitratePartMin, sizeof(m_iBitratePartMin), VS_REG_INTEGER_VT, "Min Client Bitrate");
	if (m_iBitratePartMin <= 0) m_iBitratePartMin = 10;
	int iTypeDbgStat = 1;
	if (m_fStat != 0) key.GetValue(&iTypeDbgStat, sizeof(iTypeDbgStat), VS_REG_INTEGER_VT, "Debug Bitrate File");
	else iTypeDbgStat = 0;
	int iDetectBandPercent = DETECT_BAND_PERCENT;
	key.GetValue(&iDetectBandPercent, sizeof(iDetectBandPercent), VS_REG_INTEGER_VT, "Load Threshold");
	int iVarianceBtr = 70;
	key.GetValue(&iVarianceBtr, sizeof(iVarianceBtr), VS_REG_INTEGER_VT, "Bitrate Variance Percent");
	int iBtrStep = 10;
	key.GetValue(&iBtrStep, sizeof(iBtrStep), VS_REG_INTEGER_VT, "Min Bitrate Step");
	int iLimitReduce = -1;
	key.GetValue(&iLimitReduce, sizeof(iLimitReduce), VS_REG_INTEGER_VT, "Limit Reduce");

	char chtime[32];
	if (iTypeDbgStat > 0) {
		time_t curt; time(&curt);
		tm curt_tm;
		strftime(chtime, 30, "%d/%m/%Y %H:%M:%S", localtime_r(&curt, &curt_tm));
	}

	for (itSender it = m_mapStreamSnd.begin(), e = m_mapStreamSnd.end(); it != e; it++) {
		it->second->SetTypeStat(iTypeDbgStat);
	}

	/// detect & update Participant bandwidth
	for (itParticipant it = m_mapStreamPart.begin(), e = m_mapStreamPart.end(); it != e; it++) {
		VS_ParticipantState *pPart = it->second;
		int num_snds = pPart->vStatBuffer.size(), num_ready_snds = 0;
		int iZoneTarget = -1;
		bool bReadyBuffers = false;
		int btrAudioSndBytes = 0;
		pPart->uAvgQueueLen = 0;
		pPart->uAvgQueueBytes = 0;
		pPart->uAvgRcvBytes = 0;
		pPart->uAvgDataRcvBytes = 0;
		pPart->uAvgAudioRcvBytes = 0;
		pPart->iCoefLoad = 0;
		for (i = 0; i < num_snds; i++) {
			if (!bReadyBuffers) bReadyBuffers = !!(pPart->vStatBuffer[i]->GetState() & BUFFER_READY);
			pPart->uAvgDataRcvBytes += pPart->vStatBuffer[i]->GetDataBytes();
			pPart->uAvgAudioRcvBytes += pPart->vStatBuffer[i]->GetAudioBytes();
			pPart->uAvgQueueLen += pPart->vStatBuffer[i]->GetQueueLenAvg();
			pPart->uAvgQueueBytes += pPart->vStatBuffer[i]->GetQueueBytesAvg();
			pPart->uAvgRcvBytes += pPart->vStatBuffer[i]->GetBytes();
		}
		pPart->uAvgQueueLen /= num_snds;
		if (bReadyBuffers) {
			int band = pPart->uPhBandwidth;
			unsigned int band_limit = (pPart->uPhBandwidth == UNDEF_BANDWIDTH) ? pPart->uAvgRcvBytes : pPart->uPhBandwidth * 128;
			if (pPart->uAvgQueueBytes >= QBYTES_MIN) { /// 64 kbps - limit
				pPart->iCoefLoad = pPart->uAvgQueueBytes * 100 / (band_limit + 1);
				if (pPart->iCoefLoad >= iDetectBandPercent) {
					/// detect new physical bandwidth
					if (pPart->eState & (PART_UNDEF | PART_IDLE | PART_FREEZE)) {
						pPart->uHighPhBandwidth = pPart->uPhBandwidth;
					}
					band = pPart->uAvgRcvBytes / 128;
					pPart->iOverflowTime = ctime;
				}
			}
			if (pPart->uPhBandwidth != UNDEF_BANDWIDTH) {
				/// update physical bandwidth
				auto dt = ctime - pPart->iOverflowTime;
				if (dt >= STAT_BITRATEFREEZE_TM) {
					if (pPart->iCoefLoad >= FREEZE_BITRATE_PERCENT) {
						pPart->iOverflowTime = ctime - STAT_BITRATEFREEZE_TM;
						dt = ctime - pPart->iOverflowTime;
					}
					if (pPart->uAvgRcvBytes > band_limit) {
						int k_old = 3;
						int k_new = 1;
						if (dt >= STAT_BITRATEUP_TM) {
							k_old = 9;
							k_new = 1;
						}
						band = (k_new * pPart->uAvgRcvBytes + k_old * band_limit) / (k_old + k_new) / 128 + 1;
					}
				} else if (band != pPart->uPhBandwidth) {
					band = (band + pPart->uPhBandwidth) / 2 + 1;
					if (band < pPart->uPhBandwidth) {
						int min_band = (pPart->uPhBandwidth * iVarianceBtr) / 100;
						if (band < min_band) band = min_band;
					}
				}
			}
			if (band != pPart->uPhBandwidth || pPart->iOverflowTime == ctime) {
				if (band < BANDWIDTH_MIN) band = BANDWIDTH_MIN;
				if (iTypeDbgStat == 1 && m_fStat) {
					fprintf(m_fStat, "\n UPDATE physical bandwidth %s : band = %d, load = (%3d, %6d, %3d)",
							pPart->namePart, band, pPart->iCoefLoad, pPart->uAvgQueueBytes, pPart->uAvgQueueLen);
				}
			}
			pPart->uPhBandwidth = band;
			/// detect zone bitrate
			if (pPart->iCoefLoad >= iDetectBandPercent) iZoneTarget = 2;
			else if (pPart->iCoefLoad >= FREEZE_BITRATE_PERCENT) iZoneTarget = 1;
			else if (pPart->uPhBandwidth != UNDEF_BANDWIDTH) iZoneTarget = 0;
			else iZoneTarget = -1;
			/// update avg coefficient for stat
			for (i = 0; i < num_snds; i++) {
				pPart->vStatBuffer[i]->SetState(~DATA_TRASH);
				if (iZoneTarget == -1 || iZoneTarget == 0) pPart->vStatBuffer[i]->SetState(DATA_TRASH);
			}
		} else {
			pPart->uAvgRcvBytes = 0;
			pPart->uAvgDataRcvBytes = 0;
			pPart->uAvgAudioRcvBytes = 0;
		}

		switch (iZoneTarget) {
			case -1:
				pPart->eState = PART_UNDEF;
				break;
			case 0:
				if (pPart->eState & PART_RESTRICT) {
					if (pPart->eState & PART_FIRSTUPPER) pPart->eState = PART_SECONDUPPER;
					else if (pPart->eState & PART_SECONDUPPER) pPart->eState = PART_THIRDUPPER;
					else pPart->eState = PART_FIRSTUPPER;
					pPart->iPhBandwidthChangeTime = ctime;
					pPart->iCoefUpper = 0;
				} else if ((pPart->eState & PART_FREEZE) && (pPart->iPhBandwidthChangeTime <= ctime)) {
					pPart->eState = PART_IDLE;
					pPart->iPhBandwidthChangeTime = ctime;
					pPart->uLowPhBandwidth = pPart->uCalcBandwidth;
					pPart->iCoefUpper = 0;
				} else if (pPart->eState & (PART_FIRSTUPPER | PART_SECONDUPPER | PART_THIRDUPPER)) {
					if (pPart->uCalcBandwidth >= pPart->uHighPhBandwidth) {
						pPart->eState = PART_FREEZE;
						pPart->iPhBandwidthChangeTime = ctime + 3 * STAT_BITRATEFREEZE_TM;
					}
				}
				break;
			case 1:
				if (pPart->eState & (PART_FREEZE | PART_IDLE)) {
					pPart->eState = PART_FREEZE;
					pPart->iPhBandwidthChangeTime = ctime + 3 * STAT_BITRATEFREEZE_TM;
				}
				break;
			case 2:
				{
					bool bChange = false;
					if (pPart->eState & PART_FREEZE) {
						if ((ctime - (pPart->iPhBandwidthChangeTime - 3 * STAT_BITRATEFREEZE_TM)) >= STAT_BITRATEFREEZE_TM) pPart->eState = PART_RESTRICT;
					}
					if (pPart->eState & PART_THIRDUPPER) {
						pPart->eState = PART_FREEZE;
						pPart->iPhBandwidthChangeTime = ctime + 3 * STAT_BITRATEFREEZE_TM;
						bChange = (pPart->uCalcBandwidth > pPart->uPhBandwidth);
					} else if (!(pPart->eState & PART_FREEZE)) {
						pPart->eState &= (PART_FIRSTUPPER | PART_SECONDUPPER | PART_THIRDUPPER);
						pPart->eState |= PART_RESTRICT;
						auto dt = ctime - pPart->iPhBandwidthChangeTime;
						if (dt > std::chrono::seconds(16)) pPart->eState &= ~PART_FIRSTUPPER;
						if (dt > std::chrono::seconds(30)) pPart->eState &= ~PART_SECONDUPPER;
						if (dt > std::chrono::seconds(40)) pPart->eState &= ~PART_THIRDUPPER;
						bChange = (pPart->uCalcBandwidth > pPart->uPhBandwidth);
					}
					if (bChange) {
						pPart->uCalcBandwidth = pPart->uPhBandwidth;
						pPart->uLowPhBandwidth = pPart->uPhBandwidth;
					}
				}
				break;
			default : break;
		}

		if (!(pPart->eState & PART_RESTRICT)) {
			if (pPart->eState & (PART_FIRSTUPPER | PART_SECONDUPPER | PART_THIRDUPPER | PART_IDLE)) {
				unsigned int newBitrate = pPart->uCalcBandwidth;
				unsigned short highBandwidth(pPart->uHighPhBandwidth);
				if (pPart->eState & PART_IDLE) {
					highBandwidth = static_cast<uint16_t>(std::min<uint32_t>(DETECT_COEF_LIMIT * pPart->uPhBandwidth, UNDEF_BANDWIDTH));
				}
				unsigned short doubleBandwidth = static_cast<uint16_t>(std::min<uint32_t>(DETECT_COEF_LIMIT * pPart->uLowPhBandwidth, UNDEF_BANDWIDTH));
				if (pPart->eState & PART_IDLE) {
					if (highBandwidth > UNDEF_BANDWIDTH) highBandwidth = UNDEF_BANDWIDTH;
				} else {
					if (doubleBandwidth > pPart->uHighPhBandwidth) doubleBandwidth = pPart->uHighPhBandwidth;
					if (doubleBandwidth < pPart->uLowPhBandwidth) doubleBandwidth = pPart->uLowPhBandwidth;
				}
				if (pPart->iCoefUpper == 0) {
					int nmax = 18;
					if (pPart->eState & PART_FIRSTUPPER) nmax = 6;
					else if (pPart->eState & PART_SECONDUPPER) nmax = 12;
					else if (pPart->eState & PART_THIRDUPPER) nmax = 18;
					pPart->iCoefUpper = (int) (pow(10.0, log10((double) doubleBandwidth / (double) pPart->uLowPhBandwidth) / (double) nmax) * 10000);
					if (pPart->iCoefUpper <= 10000) {
						pPart->iCoefUpper = 11000;
					}
					else if (pPart->iCoefUpper < 10100) {
						pPart->iCoefUpper = 10100;
					}
				} else {
					int k = (newBitrate <= 100 && pPart->iCoefUpper < 10500) ? 10500 : pPart->iCoefUpper;
					newBitrate = newBitrate * k / 10000;
					if (newBitrate >= doubleBandwidth) pPart->iCoefUpper = (pPart->eState & PART_IDLE) ? 10700 : 11800;
					if (newBitrate > highBandwidth) newBitrate = highBandwidth;
				}
				pPart->uCalcBandwidth = newBitrate;
			}
		}
	}

	/// state manager buffers & prediction buffers size
	unsigned short g_btrOnPartDataMin = 0;
	for (itParticipant it = m_mapStreamPart.begin(), e = m_mapStreamPart.end(); it != e; it++) {
		VS_ParticipantState *pPart = it->second;
		int maxFrameSizeWndMB = 0;
		unsigned int iState = 0x0;
		int iNumSnds = pPart->vStatBuffer.size();
		int iNumActiveSnds = 0,
			iNumDataSnds = 0,
			iNumDisableSnds = 0;
		unsigned short btrAudioOnPart = 0;
		bool bHalfVideo = false;
		for (i = 0; i < iNumSnds; i++) {
			VS_StreamSVCStatistics *pBuffer = pPart->vStatBuffer[i];
			pBuffer->SetState(~BUFFER_WORST);
			pBuffer->CheckVideoFromSnd(ctime);
			iState = (pPart->vSnds[i]->IsDataSend()) ? DATA_FROM_SND : ~DATA_FROM_SND;
			pBuffer->SetState(iState);
			iState = pBuffer->GetState();
			iNumActiveSnds++;
			if (iState & DISABLE_VIDEO) iNumDisableSnds++;
			if (iState & DATA_FROM_SND) {
				iNumDataSnds++;
				if (!(iState & NOT_VIDEO_FROM_SND)) bHalfVideo = true;
				if (iState & DISABLE_VIDEO) iNumDisableSnds--;
			} else if (iState & NOT_VIDEO_FROM_SND) {
				iNumActiveSnds--;
			}
			if (iTypeDbgStat == 1 && m_fStat) {
				if (iState & BUFFER_CONNECT) {
					if (!(iState & BUFFER_READY)) fprintf(m_fStat, "\n NOT READY buffer %s", pBuffer->ParticipantName().c_str());
					if (iState & NOT_VIDEO_FROM_SND) fprintf(m_fStat, "\n NOT VIDEO from snd %s", pBuffer->ParticipantName().c_str());
				}
			}
			btrAudioOnPart += pPart->vSnds[i]->GetAudioBytes() / 128;
			pBuffer->SetTypeStat(iTypeDbgStat);
			maxFrameSizeWndMB += pBuffer->GetWindowRestrictMB();
		}
		pPart->uMaxBufferBitrateVideo = m_iBitrateSndMediaMax - btrAudioOnPart / iNumSnds;
		PredictionBitrateBuffer(pPart, iNumActiveSnds, iNumDataSnds, iNumDisableSnds, bHalfVideo, btrAudioOnPart);
		if (iNumDataSnds > 0) {
			if (g_btrOnPartDataMin == 0) g_btrOnPartDataMin = m_iBitrateSndMax;
			if (g_btrOnPartDataMin > pPart->uBufferBitrateData) g_btrOnPartDataMin = pPart->uBufferBitrateData;
		}
		pPart->iAccBufferBitrate = 0;
	}

	/// finaly correct buffers size & buffers state
	for (itParticipant it = m_mapStreamPart.begin(), e = m_mapStreamPart.end(); it != e; it++) {
		VS_ParticipantState *pPart = it->second;
		unsigned int iState = 0x0;
		int iNumSnds = pPart->vStatBuffer.size();
		VS_StreamSVCStatistics *pBuffer;
		int iNumTryDisable = CorrectBitrateBuffer(pPart, g_btrOnPartDataMin, pPart->iNumActiveSnds, pPart->iNumDataSnds);
		pPart->iNumActiveSnds = 0;
		for (i = 0; i < iNumSnds; i++) {
			pBuffer = pPart->vStatBuffer[i];
			iState = pBuffer->GetState();
			bool bTryDisable = false;
			bool bHasVideo = !(iState & NOT_VIDEO_FROM_SND);
			bool bDisable = !!(iState & DISABLE_VIDEO);
			bool bHasData = !!(iState & DATA_FROM_SND);
			if (bHasData) {
				if (pPart->uBufferBitrate == pPart->uBufferBitrateData) bTryDisable = true;
			} else {
				if (iNumTryDisable > 0 && bHasVideo) bTryDisable = true;
			}
			if (bTryDisable) {
				if (bDisable) {
					if (iTypeDbgStat == 1 && m_fStat) fprintf(m_fStat, "\n WAS DISABLE buffer %s (DATA = %d)", pBuffer->ParticipantName().c_str(), (int)bHasData);
					if (!bHasData) iNumTryDisable--;
				} else if (bHasData || pPart->iNumDisableSnds <= 0) {
					if (iTypeDbgStat == 1 && m_fStat) fprintf(m_fStat, "\n SET DISABLE buffer %s", pBuffer->ParticipantName().c_str());
					pBuffer->SetState(DISABLE_VIDEO);
					if (!bHasData) iNumTryDisable--;
				}
			} else if (bDisable) {
				if (iTypeDbgStat == 1 && m_fStat) fprintf(m_fStat, "\n SET ENABLE buffer %s", pBuffer->ParticipantName().c_str());
				pBuffer->SetState(~DISABLE_VIDEO);
			}
			if (bDisable && !bHasData) pPart->iNumDisableSnds--;
			iState = pBuffer->GetState();
			if (!(iState & (DISABLE_VIDEO | DATA_FROM_SND | NOT_VIDEO_FROM_SND))) pPart->iNumActiveSnds++;
		}
		for (i = 0; i < iNumTryDisable; i++) {
			pBuffer = pPart->vStatBuffer[i];
			iState = pBuffer->GetState();
			if (!(iState & DISABLE_VIDEO)) {
				if (iTypeDbgStat == 1 && m_fStat) fprintf(m_fStat, "\n SET DISABLE buffer %s", pBuffer->ParticipantName().c_str());
				pBuffer->SetState(DISABLE_VIDEO);
			}
		}
		if (iNumTryDisable > 0) {
			if (iTypeDbgStat == 1 && m_fStat) fprintf(m_fStat, "\n !!! INCORRECT set disable buffers");
		}
		//pPart->UpdateSysLoad(ctime, m_fStat, iLimitReduce);
	}

	if (iTypeDbgStat == 1) {
		unsigned int bitrateFromServer = 0;
		unsigned int bitrateAudioFromServer = 0;
		unsigned int bitrateDataFromServer = 0;
		int64_t avgNumPackets = 0;
		int64_t avgNumBytes = 0;
		if (m_fStat)
			fprintf(m_fStat, "\n %32s        mbps  load        rcv  audio     data   bndph  bndes  pkt    bytes       vpart       dpart  apart", chtime);
		for (itParticipant it = m_mapStreamPart.begin(), e = m_mapStreamPart.end(); it != e; it++) {
			VS_ParticipantState *pPart = it->second;
			if (m_fStat)
				fprintf(m_fStat, "\n %32.32s || %8d,  %3d || %7d (%5d, %7d), %5d, %5d, %3d, %7d| %5d x %2d, %5d x %2d, %5d||",
					pPart->namePart, pPart->maxMBps, pPart->sysLoad, pPart->uAvgRcvBytes / 128, pPart->uAvgAudioRcvBytes / 128, pPart->uAvgDataRcvBytes / 128,
					pPart->uPhBandwidth, pPart->uCalcBandwidth, pPart->uAvgQueueLen, pPart->uAvgQueueBytes,
					pPart->uBufferBitrateVideo, pPart->iNumActiveSnds,
					pPart->uBufferBitrateData, pPart->iNumDataSnds,
					pPart->uBufferBitrate);
			bitrateFromServer += pPart->uAvgRcvBytes / 128;
			bitrateAudioFromServer += pPart->uAvgAudioRcvBytes / 128;
			bitrateDataFromServer += pPart->uAvgDataRcvBytes / 128;
			avgNumPackets += pPart->uAvgQueueLen;
			avgNumBytes += pPart->uAvgQueueBytes;
		}
		if (m_mapStreamPart.size() > 0) {
			avgNumPackets /= m_mapStreamPart.size();
			avgNumBytes /= m_mapStreamPart.size();
		}
		if (m_fStat) {
			if (m_mapStreamPart.size() > 0) {
				fprintf(m_fStat, "\n------------------------------------------------------------------------------------------------------------");
				fprintf(m_fStat, "\n                                  ||                || %7u (%5u, %7u)                %3d, %7d|",
					bitrateFromServer, bitrateAudioFromServer, bitrateDataFromServer, (int)avgNumPackets, (int)avgNumBytes);
				fprintf(m_fStat, "\n------------------------------------------------------------------------------------------------------------");
			}
			for (itSender is = m_mapStreamSnd.begin(), es = m_mapStreamSnd.end(); is != es; is++) {
				VS_SenderState *pSender = is->second;
				if (pSender->GetBytes() > 128 || !pSender->vStatBuffer.empty()) {
					fprintf(m_fStat, "\n SENDER STAT %32.32s : b = %4d, ba = %4d, bd = %4d (%d)",
						is->first, pSender->GetBytes() / 128, pSender->GetAudioBytes() / 128,
						pSender->GetDataBytes() / 128, pSender->IsDataSend() ? 1 : 0);
				}
			}
		}
	}

	if (iTypeDbgStat > 0 && m_fStat) fflush(m_fStat);

	/// detect bitrates for every sender
	for (itSender is = m_mapStreamSnd.begin(), es = m_mapStreamSnd.end(); is != es; is++) {
		VS_SenderState *pSender = is->second;
		unsigned short setBitrateOlder = 0;
		unsigned short setDataBitrate = (pSender->IsDataSend()) ? (g_btrOnPartDataMin * DETECT_COEF_DATA / 100) : (m_iBitrateSndMax - m_iBitrateSndMediaMax);
		int nTLayers = 1;
		int nSLayers = 1;
		int nNumCoefs = 1;
		if (!pSender->bEmulateSVC) {
			bool bChange = pSender->UpdateLayerStat(ctime, m_fStat);
			nSLayers = pSender->GetNumLayers(SVC_SPATIAL);
			nTLayers = pSender->GetNumLayers(SVC_TEMPORAL);
			nNumCoefs = pSender->GetNumReduceCoef();
			if (bChange && iTypeDbgStat == 1 && m_fStat) {
				fprintf(m_fStat, "\n UPDATE state sender %s : (%3d, %3d) (%3.2f", is->first, nSLayers, nTLayers, pSender->GetReduceCoef(0) / 100.0);
				for (l = 1; l < nNumCoefs; l++) {
					fprintf(m_fStat, ", %3.2f", pSender->GetReduceCoef(l) / 100.0);
				}
				fprintf(m_fStat, ")");
			}
		}
		int iNumParts = pSender->vStatBuffer.size();
		int bitrate_ctrl = pSender->iLastSetBitrate;
		unsigned short uBitratePartVideoMin = UNDEF_BANDWIDTH, uBitratePartVideoMax = 0;
		int iNumUndefPartBtr = 0;
		bool bAllDisable = true, bAllNoVideo = true;
		bool bDataSend = false;
		if (iNumParts == 1) {
			uBitratePartVideoMax = pSender->vParts[0]->uMaxBufferBitrateVideo;
		}
		for (i = 0; i < iNumParts; i++) {
			VS_StreamSVCStatistics *pBuffer = pSender->vStatBuffer[i];
			unsigned int iState = pBuffer->GetState();
			if (pSender->vParts[i]->uPhBandwidth == UNDEF_BANDWIDTH) iNumUndefPartBtr++;
			unsigned short uBitrateVideo = pSender->vParts[i]->uBufferBitrateVideo;
			if (pSender->IsDataSend()) {
				uBitrateVideo = std::min<unsigned short>(pSender->vParts[i]->uBufferBitrate - pSender->vParts[i]->uBufferBitrateData, pSender->vParts[i]->uMaxBufferBitrateVideo);
			}
			if (iState & BUFFER_READY) {
				if (!(iState & DISABLE_VIDEO) && !(iState & NOT_VIDEO_FROM_SND)) {
					if (uBitrateVideo < m_iBitratePartMin) {
						if (iTypeDbgStat == 1 && m_fStat) fprintf(m_fStat, "\n !!! INCORRECT BUFFER VALUE %s : b = %d, state = %x", pBuffer->ParticipantName().c_str(), uBitrateVideo, iState);
					} else {
						uBitratePartVideoMin = std::min(uBitratePartVideoMin, uBitrateVideo);
						uBitratePartVideoMax = std::max(uBitratePartVideoMax, uBitrateVideo);
					}
					bAllDisable = false;
					bAllNoVideo = false;
				} else {
					if (!(iState & DISABLE_VIDEO)) bAllDisable = false;
					if (!(iState & NOT_VIDEO_FROM_SND)) bAllNoVideo = false;
				}
			} else if (!(iState & NOT_VIDEO_FROM_SND)) {
				if (iState & DISABLE_VIDEO) {
					uBitrateVideo = pSender->iLastSetBitrate;
				} else {
					bAllDisable = false;
				}
				if (uBitrateVideo < m_iBitratePartMin) {
					if (iTypeDbgStat == 1 && m_fStat) fprintf(m_fStat, "\n !!! INCORRECT BUFFER VALUE %s : b = %d, state = %x", pBuffer->ParticipantName().c_str(), uBitrateVideo, iState);
				} else {
					uBitratePartVideoMin = std::min(uBitratePartVideoMin, uBitrateVideo);
					uBitratePartVideoMax = std::max(uBitratePartVideoMax, uBitrateVideo);
				}
				bAllNoVideo = false;
			}
		}
		/// calc sender real bitrate
		int k = 0;
		int bitrateSnd = pSender->GetVideoBytes() / 128;
		for (i = 0; i < nSLayers; i++) {
			k += pSender->GetReduceCoef(i*nTLayers);
		}
		int bitrateSndBase = bitrateSnd * 100 / k;
		pSender->SetCurBaseBitrate(bitrateSndBase);
		bitrateSndBase = pSender->GetBaseBitrate();
		if (bitrateSndBase == 0) bitrateSndBase = m_iBitratePartMin;
		/// calc layers for every participant
		if (iNumParts == 0) {
			bitrate_ctrl = m_iBitrateSndMediaMax - pSender->GetAudioBytes() / 128;
			setDataBitrate = m_iBitrateSndMax - m_iBitrateSndMediaMax;
		} else if (iNumUndefPartBtr != iNumParts && !bAllDisable && !bAllNoVideo) {

			if (uBitratePartVideoMin == 0 || uBitratePartVideoMax == 0) {
				if (iTypeDbgStat == 1 && m_fStat) fprintf(m_fStat, "\n !!! INCORRECT BUFFER VALUE %s : bmin = %d, bmax = %d", is->first, uBitratePartVideoMin, uBitratePartVideoMax);
				bitrate_ctrl = pSender->iLastSetBitrate;
			} else {
				double kMin = pSender->GetReduceCoef(nNumCoefs-1) / 100.0;
				bitrate_ctrl = (int)(uBitratePartVideoMin / kMin);
				if (bitrate_ctrl > uBitratePartVideoMax) bitrate_ctrl = uBitratePartVideoMax;
				int bitrate_ctrl_thr = bitrate_ctrl;
				for (j = 0; j < iNumParts; j++) {
					unsigned int iState = pSender->vStatBuffer[j]->GetState();
					bool bTroubleVideo = (iState & NOT_VIDEO_FROM_SND) || (iState & DISABLE_VIDEO);
					if (!bTroubleVideo) {
						int bitrate = pSender->vParts[j]->uBufferBitrateVideo;
						if (100 * bitrate >= DETECT_BITRATESET_PERCENT * bitrate_ctrl && bitrate < bitrate_ctrl_thr) {
							bitrate_ctrl_thr = pSender->vParts[j]->uBufferBitrateVideo;
						}
					}
				}
				bitrate_ctrl = bitrate_ctrl_thr;
				for (j = 0; j < iNumParts; j++) {
					VS_StreamSVCStatistics *pBuffer = pSender->vStatBuffer[j];
					int iReduce = 100;
					unsigned int iState = pBuffer->GetState();
					bool bTroubleVideo = (iState & NOT_VIDEO_FROM_SND) || (iState & DISABLE_VIDEO);
					if (!bTroubleVideo) {
						VS_ParticipantState *pPart = pSender->vParts[j];
						int bitrateSender = std::min<int>(pPart->uMaxBufferBitrateVideo, bitrateSndBase);
						int bitrate = 0;
						if (iState & DATA_FROM_SND) {
							bitrate = std::min<decltype(bitrate)>(std::max(pPart->uBufferBitrate - setDataBitrate, 0), pPart->uMaxBufferBitrateVideo);
						} else {
							bitrate = pPart->uBufferBitrateVideo;
						}
						iReduce = bitrate * 100 / bitrateSndBase;
						if (iReduce > 100) iReduce = 100;
					} else {
						iReduce = pBuffer->GetReduceCoef();
					}
					if (iTypeDbgStat == 1) {
						int iReduceCur = pBuffer->GetReduceCoef();
						if (iReduce != iReduceCur && m_fStat) {
							fprintf(m_fStat, "\n SET new reduce coefs STLayers %s : b = %5d, %3d -> %3d ( %3d, (% d, % d) )",
											 pBuffer->ParticipantName().c_str(),
											 bitrateSndBase, iReduceCur, iReduce,
											 pSender->GetBaseReduce(iReduce),
											 pSender->GetSLayerId(iReduce),
											 pSender->GetTLayerId(iReduce));
						}
					}
					pBuffer->SetReduceCoef(iReduce, bitrateSndBase);
				}
			}
		} else {
			int iReduce = 0;
			bitrate_ctrl = pSender->iLastSetBitrate;
			setDataBitrate = pSender->iLastDataBitrate;
			if (!bAllNoVideo || pSender->IsDataSend()) {
				if (bAllDisable) {
					bitrate_ctrl = m_iBitrateSndMediaMax - pSender->GetAudioBytes() / 128;
					setDataBitrate = m_iBitrateSndMax - m_iBitrateSndMediaMax;
				}
				else if (iNumUndefPartBtr == iNumParts) {
					bitrate_ctrl = m_iBitrateSndMediaMax - pSender->GetAudioBytes() / 128;
					setDataBitrate = m_iBitrateSndMax - m_iBitrateSndMediaMax;
					iReduce = 100;
				}
				for (j = 0; j < iNumParts; j++) {
					if (iReduce == 0) iReduce = pSender->vStatBuffer[j]->GetReduceCoef();
					pSender->vStatBuffer[j]->SetReduceCoef(iReduce, bitrateSndBase);
				}
			}
		}
		setBitrateOlder = pSender->GetAudioBytes() / 128; /// audio
		if (!pSender->IsDataSend()) {
			setBitrateOlder += bitrate_ctrl;
			if (iNumUndefPartBtr == iNumParts && !bAllNoVideo) setBitrateOlder = m_iBitrateSndMax;
			else if (setBitrateOlder > m_iBitrateSndMediaMax) setBitrateOlder = m_iBitrateSndMediaMax;
		} else {
			setBitrateOlder += setDataBitrate * 3 / 2;
		}
		if (bitrate_ctrl != pSender->iLastSetBitrate || setDataBitrate != pSender->iLastDataBitrate) {
			int dbtr = abs(pSender->iLastSetBitrate - bitrate_ctrl);
			int ddbtr = abs(pSender->iLastDataBitrate - setDataBitrate);
			if ((dbtr >= iBtrStep || (dbtr * 100 >= 7 * pSender->iLastSetBitrate)) || (ddbtr >= iBtrStep || (ddbtr * 100 >= 7 * pSender->iLastDataBitrate))) {
				pSender->iLastSetBitrate = bitrate_ctrl;
				pSender->iLastDataBitrate = setDataBitrate;
				if (pSender->bEmulateSVC) {
					bitrate_ctrl = bitrate_ctrl * 85 / 100;
					setBitrateOlder = setBitrateOlder * 85 / 100;
				}
				pCallback->RestrictBitrateSVC(conferenceName, is->first, bitrate_ctrl, bitrate_ctrl + setDataBitrate, setBitrateOlder);
				if (iTypeDbgStat == 1 && m_fStat) {
					fprintf(m_fStat, "\n SET bitrate %s : btr = %4d, vmin = %4d, dbtr = %4d (ob = %4d) (%3.2f",
						is->first, bitrate_ctrl, uBitratePartVideoMin, setDataBitrate, setBitrateOlder, pSender->GetReduceCoef(0) / 100.0);
					for (l = 1; l < nNumCoefs; l++) {
						fprintf(m_fStat, ", %3.2f", pSender->GetReduceCoef(l) / 100.0);
					}
					fprintf(m_fStat, ")");
				}
			}
		}
		int32_t numKeyRequest(0);
		for (i = 0; i < iNumParts; i++) {
			if (!pSender->vStatBuffer[i]->IsWaitKeyFrame(ctime)) {
				continue;
			}
			numKeyRequest++;
		}
		if (numKeyRequest > 0) {
			pCallback->RequestKeyFrame(conferenceName, is->first);
		}
		if (iTypeDbgStat > 0 && m_fStat) {
			fflush(m_fStat);
		}
	}

	if (iTypeDbgStat > 0 && m_fStat) {
		fprintf(m_fStat, "\n------------------------------------------------------------------------------------------------------------");
		fflush(m_fStat);
	}

	m_iBitrateTime = ctime;
}

VS_StreamSVCStatistics::VS_StreamSVCStatistics()
{
	m_pStat = reinterpret_cast<stream::StreamStatistics*>(new char[0x4000]);
}

VS_StreamSVCStatistics::~VS_StreamSVCStatistics()
{
	delete[] reinterpret_cast<char*>(m_pStat);
}

int VS_StreamSVCStatistics::Init()
{
	m_pRouterStat = 0;
	m_iNumIterates = m_iCountStat = 0;
	m_iQueueBytesAvg = 0;
	m_iQueueLenAvg = 0;
	m_iBytesPerSecLast = 0;
	memset(m_qQueueLen, 0, MAX_WND_SIZE * sizeof(int));
	memset(m_qQueueBytes, 0, MAX_WND_SIZE * sizeof(int));
	memset(m_iBytesPerSec, 0, TRACK_MAX * sizeof(int));
	memset(m_iBytesPerSecTmp, 0, TRACK_MAX * sizeof(int));
	return 0;
}

void VS_StreamSVCStatistics::UpdateRcvStatistics()
{
	if (m_pRouterStat && (GetState() & BUFFER_CONNECT)) {

#ifdef TEST_SPEED
		AUTO_PROF
#endif

		if (m_iCountStat >= VS_SR_STAT_N_TICKS) {
			bool video = true;
			if (m_pRouterStat->FormSndStatistics(m_pStat, 0x4000, &video) > 0) {
				m_iBytesPerSecLast = m_pStat->allWriteBytesBand;
				int idx = m_iNumIterates % MAX_WND_SIZE;
				for (int i = 0; i < m_pStat->ntracks; i++) m_iBytesPerSecTmp[idx][id(m_pStat->tracks[i].track)] = m_pStat->tracks[i].writeBytesBand;
				m_iBytesPerSecTmp[idx][TRACK_ALL] = m_pStat->allWriteBytesBand;
				m_qQueueBytes[idx] = GetInstantQueueBytes();
				m_qQueueLen[idx] = GetInstantQueueLen();
				int nCount = (m_iNumIterates > MAX_WND_SIZE) ? MAX_WND_SIZE : m_iNumIterates;
				memset(m_iBytesPerSec, 0, TRACK_MAX * sizeof(int));
				m_iQueueBytesAvg = 0;
				m_iQueueLenAvg = 0;
				for (int i = 0; i < nCount; i++) {
					for (int j = 0; j < m_pStat->ntracks; j++) {
						auto track = m_pStat->tracks[j].track;
						m_iBytesPerSec[id(track)] += m_iBytesPerSecTmp[i][id(track)];
					}
					m_iBytesPerSec[TRACK_ALL] += m_iBytesPerSecTmp[i][TRACK_ALL];
					m_iQueueBytesAvg += m_qQueueBytes[i];
					m_iQueueLenAvg += m_qQueueLen[i];
				}
				for (int i = 0; i < m_pStat->ntracks; i++) m_iBytesPerSec[id(m_pStat->tracks[i].track)] /= MAX_WND_SIZE;
				m_iBytesPerSec[TRACK_ALL] /= MAX_WND_SIZE;
				m_iQueueBytesAvg /= MAX_WND_SIZE;
				m_iQueueLenAvg /= MAX_WND_SIZE;
				m_iNumIterates++;
			}
		}
		m_iCountStat++;
	}
}

void VS_StreamSVCStatistics::UpdateSndStatistics()
{
	if (m_pRouterStat) {

#ifdef TEST_SPEED
		AUTO_PROF
#endif

		bool video = true;
		if (m_pRouterStat->FormRcvStatistics(m_pStat, 0x4000, &video) > 0) {
			m_iBytesPerSecLast = m_pStat->allWriteBytesBand;
			for (int i = 0; i < m_pStat->ntracks; i++) m_iBytesPerSec[id(m_pStat->tracks[i].track)] = (2 * m_iBytesPerSec[id(m_pStat->tracks[i].track)] + m_pStat->tracks[i].writeBytesBand) / 3;
			m_iBytesPerSec[TRACK_ALL] = (2 * m_iBytesPerSec[TRACK_ALL] + m_pStat->allWriteBytesBand) / 3;
			m_iNumIterates++;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
