/**
 **************************************************************************
 * \file VS_StreamSVCBuffer.cpp
 * (c) 2012 TrueConf LLC
 * \brief SVC Server Implementation
 *
 * \b Project Server
 * \author SAnufriev
 * \date 10.02.2012
 ****************************************************************************/

#include "SVCBuffer.h"
#include "../VS_StreamsDefinitions.h"
#include "../VS_StreamsSVCTypes.h"
#include "SecureLib/VS_StreamCrypter.h"
#include "../../MediaParserLib/VS_VPXParser.h"
#include "../../MediaParserLib/VS_H264Parser.h"
#include "../../std/cpplib/VS_MemoryLeak.h"

#ifdef TEST_SPEED
#include "../../std/VS_ProfileTools.h"
#endif

#ifdef TEST_CRASH
#include "../../Video/WinApi/CAviFile.h"
#endif

#include "std-generic/compat/memory.h"

static const auto STAT_READY_TM = std::chrono::seconds(5);
static const auto STAT_TRASH_TM = std::chrono::milliseconds(50);
static const auto STAT_KEYFRAME_TM = std::chrono::seconds(2);
static const auto DETECT_LAYERDOWN_TM = std::chrono::seconds(3);
static const auto DETECT_TRASHDOWN_TM = std::chrono::seconds(5);
static const auto DETECT_LAYERUP_TM = std::chrono::seconds(6);

namespace stream {

/// SVC Packet Interface

struct SVCPacket
{
	stream::SVCHeader* svch;
	stream::SliceHeader* sh;
	std::unique_ptr<char[]> pBuffer;
	size_t					size;
	unsigned long			timestamp;
	stream::Track			track;
	stream::SVCHeader		emulated_svc_hdr;
	VS_StreamCrypter		*pCrypter;
	SVCPacket(std::unique_ptr<char[]>&& buffer, size_t len, unsigned long tickcount, stream::Track type, bool emulate_svc, VS_StreamCrypter *crypter)
	{
		pCrypter = crypter;
		emulated_svc_hdr = { 0, 0, 0, 0 };
		track = type;
		timestamp = tickcount;
		pBuffer = std::move(buffer);
		size = len;
		if (track == stream::Track::video) {
			if (pCrypter) {
				uint32_t decr_size = 0;
				pCrypter->Decrypt(reinterpret_cast<unsigned char*>(pBuffer.get()), len, nullptr, &decr_size);
				if (decr_size > 0) {
					auto decr_buffer = vs::make_unique<char[]>(decr_size);
					if (pCrypter->Decrypt(reinterpret_cast<unsigned char*>(pBuffer.get()), len, reinterpret_cast<unsigned char*>(decr_buffer.get()), &decr_size)) {
						pBuffer = std::move(decr_buffer);
						size = decr_size;
					}
				}
			}
			if (emulate_svc) {
				svch = &emulated_svc_hdr;
			} else {
				size -= sizeof(stream::SVCHeader);
				svch = reinterpret_cast<stream::SVCHeader*>(pBuffer.get() + size);
			}
			sh = reinterpret_cast<stream::SliceHeader*>(pBuffer.get() + size - sizeof(stream::SliceHeader));
		}
	}
	~SVCPacket() {};
	std::unique_ptr<char[]> GetBuffer() {
		if (pCrypter && (track == stream::Track::video || track == stream::Track::garbage)) {
			uint32_t len = size * 2 + 128;
			auto buff = vs::make_unique<char[]>(size * 2 + 16);
			if (pCrypter->Encrypt(reinterpret_cast<unsigned char*>(pBuffer.get()), size, reinterpret_cast<unsigned char*>(buff.get()), &len)) {
				pBuffer = std::move(buff);
				size = len;
			}
		}
		return std::move(pBuffer);
	}
};

/// SVC Buffers

SVCBuffer::SVCBuffer(unsigned int modeSVC, bool bDynChangeFmt, unsigned char uLevel, int sndMBps, int sndFrameSizeMB)
	: SVCBuffer(modeSVC, bDynChangeFmt, uLevel, sndMBps, sndFrameSizeMB, DETECT_MAX_QLEN)
{
}

SVCBuffer::SVCBuffer(unsigned int modeSVC, bool bDynChangeFmt, unsigned char uLevel, int sndMBps, int sndFrameSizeMB, unsigned maxFrames)
	: m_pCrypter(nullptr)
	, m_pSenderCallback(nullptr)
	, m_uTypeSVC(modeSVC)
	, m_bDynChangeFmt(bDynChangeFmt)
	, m_bChangeSLRestrict(false)
	, m_uLevel(uLevel)
	, m_iSndMBps(sndMBps)
	, m_iSndFrameSizeMB(sndFrameSizeMB)
	, m_iQueueLengthMax(maxFrames)
	, m_iTypeDbgStat(0)
{
}

SVCBuffer::~SVCBuffer()
{
	Destroy(m_conf_name, m_part_name);
}

bool SVCBuffer::Init(string_view conf_name, string_view part_name)
{
	if (conf_name.empty() || part_name.empty())
		return false;
	m_conf_name = std::string(conf_name);
	m_part_name = std::string(part_name);

	m_pCrypter = nullptr;
	m_bLogicalBuffer = m_part_name.find("-<%%>-") == m_part_name.npos;
	m_bWaitKey = true;
	m_bNeedKey = false;
	m_iSLayerNum = 1;
	if (m_uTypeSVC & SVC_2S_MODE) m_iSLayerNum = 2;
	if (m_uTypeSVC & SVC_3S_MODE) m_iSLayerNum = 3;
	m_iTLayer = m_iTLayerMax = 2;
	m_iSLayer = m_iSLayerMax = m_iSLayerRestrict = 0;
	m_iQLayer = m_iQLayerMax = 0;
	m_iReduce = m_iReduceSet = m_iReduceLoad = 100;
	m_iState = 0x00000000;
	m_iBitrateSet = m_iBytesTrash = 0;
	m_bSendTrashData = false;
	m_uFrameId = 0;
	m_uFrameCount = 0;
	m_uSectionCount = 0;
	m_iTimestamp = 0;
	m_iTimestampPrev = 0;
	m_iQueueBytes = 0;
	m_iPacketsToConnect = 0;
	m_uAnalyseTimeRcv = std::chrono::steady_clock::time_point{};
	m_uAnalyseTimeSnd = std::chrono::steady_clock::time_point{};
	m_uTimeoutLayer = std::chrono::steady_clock::time_point{};
	m_uKeyFrameTimeout = std::chrono::steady_clock::time_point{};
	m_iTypeDbgStat = 0;
	m_iVideoTimeout = {};
	VS_StreamSVCStatistics::Init();
	memset(m_nTrackFrames, 0, sizeof(m_nTrackFrames));
	VS_MainSVCStatistics::RegisterStreamBuffer(this);
	m_fStat = VS_MainSVCStatistics::GetStatisticsFile(m_conf_name.c_str());
	if (m_fStat) {
		fprintf(m_fStat, "\n %s : CREATE participant %s", m_conf_name.c_str(), m_part_name.c_str());
		m_iTypeDbgStat = 1;
	}

#ifdef TEST_CRASH

	if (!m_bLogicalBuffer) {
		std::string tmp_name = m_part_name;
		tmp_name.replace("-<%%>-", "_");
		for (int i = 0; i < NUM_SLAYERS; i++) {
			/// in
			sprintf(m_fname[0], "E:\\work\\test_server\\test_in_%s_%d_sl.txt", tmp_name.c_str(), i);
			out_stat[0][i] = fopen(m_fname[0], "w");
			sprintf(m_fname[0], "E:\\work\\test_server\\_test_in_%s_%d_sl.cmp", tmp_name.c_str(), i);
			out_cmp[0][i] = fopen(m_fname[0], "wb");
			sprintf(m_vname[0], "test_in_%s_%d_sl.avi", tmp_name.c_str(), i);
			m_avifile[0][i] = vs::make_unique<CAviFile>();
			m_avifile[0][i]->Init(m_vname[0], true);
			/// out
			sprintf(m_fname[1], "E:\\work\\test_server\\test_out_%s_%d_sl.txt", tmp_name.c_str(), i);
			out_stat[1][i] = fopen(m_fname[1], "w");
			sprintf(m_fname[1], "E:\\work\\test_server\\_test_out_%s_%d_sl.cmp", tmp_name.c_str(), i);
			out_cmp[1][i] = fopen(m_fname[1], "wb");
			sprintf(m_vname[1], "test_out_%s_%d_sl.avi", tmp_name.c_str(), i);
			m_avifile[0][i] = vs::make_unique<CAviFile>();
			m_avifile[1][i]->Init(m_vname[1], true);
			///
			int w = 1280, h = 720;
			if (i == 1) {
				w = 640;
				h = 356;
			} else if (i == 2) {
				w = 320;
				h = 176;
			}
			BITMAPINFOHEADER bm;
			memset(&bm, 0, sizeof(BITMAPINFOHEADER));
			bm.biSize = sizeof(BITMAPINFOHEADER);
			bm.biWidth = w;
			bm.biHeight = h;
			bm.biPlanes = 1;
			bm.biCompression = '08PV';
			m_avifile[0][i]->m_fps = 15.0;
			m_avifile[1][i]->m_fps = 15.0;
			m_avifile[0][i]->SetFormat(&bm);
			m_avifile[1][i]->SetFormat(&bm);
			m_size_avi_frame[0][i] = 0;
			m_avi_frame_id[0][i] = -1;
			m_bKey_Avi[0][i] = false;
			m_tmstp_prev[0][i] = -1;
			m_num_frames[0][i] = 0;
			m_size_avi_frame[1][i] = 0;
			m_avi_frame_id[1][i] = -1;
			m_bKey_Avi[1][i] = false;
			m_tmstp_prev[1][i] = -1;
			m_num_frames[1][i] = 0;
		}
	}

#endif

	return true;
}

void SVCBuffer::Destroy(string_view conf_name, string_view part_name)
{
	if (m_conf_name.empty() || m_part_name.empty())
		return;
	if (conf_name.empty() || part_name.empty())
		return;
	if (m_conf_name == conf_name && m_part_name == part_name) {
		m_fStat = VS_MainSVCStatistics::GetStatisticsFile(m_conf_name.c_str());
		if (m_fStat) {
			fprintf(m_fStat, "\n %s : KILL participant %s", m_conf_name.c_str(), m_part_name.c_str());
			fflush(m_fStat);
		}
		VS_MainSVCStatistics::UnRegisterStreamBuffer(this);
		m_lQueuePacket.clear();
		m_conf_name.clear();
		m_part_name.clear();
		m_fStat = 0;

#ifdef TEST_CRASH

		if (!m_bLogicalBuffer) {
			for (int i = 0; i < NUM_SLAYERS; i++) {
				fclose(out_stat[0][i]);
				fclose(out_cmp[0][i]);
				fclose(out_stat[1][i]);
				fclose(out_cmp[1][i]);
			}
		}

#endif

	}
}

void SVCBuffer::SetSLayerNum(int iLayerNum)
{
	m_iSLayerNum = iLayerNum;
}

void SVCBuffer::SetParticipantStatisticsInterface(stream::ParticipantStatisticsInterface* p)
{
	m_pRouterStat = p;
}

void SVCBuffer::SetStreamCrypter(VS_StreamCrypter* p)
{
	m_pCrypter = p;
}

const std::string& SVCBuffer::ConferenceName() const
{
	return m_conf_name;
}

const std::string& SVCBuffer::ParticipantName() const
{
	return m_part_name;
}

auto SVCBuffer::PutFrame(uint32_t tickCount, Track track, std::unique_ptr<char[]>&& buffer, size_t s_buffer) -> Status
{
	if (m_bLogicalBuffer)
		return Status::non_fatal;

	if (!(m_iState & BUFFER_CONNECT) && m_lQueuePacket.empty()) {
		/// set trash packets to queue for check connect
		m_bSendTrashData = true;
		m_iBytesTrash = 100;
		for (int i = 0; i < 2; i++) AnalyseTrash();
		m_bSendTrashData = false;
		m_iBytesTrash = 0;
	}

#ifdef TEST_SPEED
	AUTO_PROF
#endif

	auto ctime = std::chrono::steady_clock::now();
	auto p = vs::make_unique<SVCPacket>(std::move(buffer), s_buffer, tickCount, track, (m_uTypeSVC == 0x00000000), m_pCrypter);
	int iSLayerNum = 0, iSLayerDelta = 0;
	if (p->track == stream::Track::video) {
		iSLayerNum = p->svch->maxspatial + 1;
		iSLayerDelta = DetectChangeSpatialLayers(ctime, iSLayerNum);
		if (p->sh->id == p->sh->first_id) {
			if (p->svch->spatial == 0 && p->pBuffer[0] != 0) { /// parsing key frame for base sl
				DetectChangeSpatialLayerSize(reinterpret_cast<unsigned char*>(p->pBuffer.get()) + 5, p->size - 5);
			}
		}
	}
	UpdateReduceCoefs();
	if (m_iState & BUFFER_CONNECT) AnalyseQueuePacket(ctime, true);
	if (p->track == stream::Track::video) {

#ifdef TEST_CRASH
		CouplingFrame(p, 0);
#endif

		bool bNewFrame = false;
		m_pSenderCallback->CalculateLayerStat(s_buffer, p->svch->spatial, p->svch->temporal, iSLayerNum);
		if (p->sh->id == p->sh->first_id) {
			bNewFrame = true;
			if (iSLayerDelta == 0) {
				m_bKeyFrame = p->pBuffer[0] != 0;
				if (m_bKeyFrame && m_iSLayer != m_iSLayerMax) {
					if (m_iSLayerMax == p->svch->spatial) { /// change spatial layer
						if (m_iTypeDbgStat > 0) {
							fprintf(m_fStat, "\n %s : %s : SL   change : (% d, % d) -> (% d, % d)",
											 m_conf_name.c_str(), m_part_name.c_str(), m_iSLayer, m_iTLayer, m_iSLayerMax, m_iTLayerMax);
						}
						m_iSLayer = m_iSLayerMax;
						m_iTLayer = m_iTLayerMax;
						m_uTimeoutLayer = ctime;
						m_bWaitKey = false;
						m_bNeedKey = false;
					} else if (m_iSLayer < m_iSLayerMax) {
						bNewFrame = false;
						m_bKeyFrame = false;
						m_bWaitKey = true;
					}
				}
			} else {
				m_iSLayerMax += iSLayerDelta;
				m_iSLayer += iSLayerDelta;
				if (m_bDynChangeFmt) {
					if (m_iSLayerMax < 0) m_iSLayerMax = 0;
					if (m_iSLayer < 0) m_iSLayer = 0;
				}
			}
		}

		if (m_iSLayer == p->svch->spatial && m_iQLayer == p->svch->quality) {
			if (bNewFrame) {
				m_uFrameCount++;
				m_bKeyFrame = p->pBuffer[0] != 0;
				if (m_bKeyFrame) m_bWaitKey = false;
				else if (m_uFrameCount != p->sh->frame_counter)
					m_bWaitKey = true;
				m_uFrameCount = p->sh->frame_counter;
				m_uSectionCount = p->sh->id;
				m_iTimestampPrev += *(unsigned int*)(p->pBuffer.get() + 1);
				if (m_bWaitKey) m_iTimestamp = m_iTimestampPrev;
			} else {
				if (m_uFrameCount != p->sh->frame_counter)
					m_bWaitKey = true;
				else if (m_uSectionCount != p->sh->id)
					m_bWaitKey = true;
			}
			if (!m_bWaitKey) {
				m_uSectionCount--;
				if (bNewFrame) {
					bool bLayerUp = (GetQueueBytesAvg() * 100 < GetBytes() * DETECT_LAYERUP_PERCENT) &&
									(ctime - m_uTimeoutLayer) >= DETECT_LAYERUP_TM && (m_iTLayer < m_iTLayerMax) && (m_iSLayer >= m_iSLayerMax);
					if ((m_bKeyFrame && m_iTLayer < 0 && m_iTLayerMax >= 0) || (p->svch->temporal == 0 && m_iTLayer >= 0 && bLayerUp)) {
						/// change temporal layer
						m_iTLayer++;
						if (m_iTLayer > m_iTLayerMax) m_iTLayer = m_iTLayerMax;
						m_uTimeoutLayer = ctime;
						if (m_iTypeDbgStat > 0) {
							fprintf(m_fStat, "\n %s : %s : TL   up : (% d, % d) -> (% d, % d)",
											 m_conf_name.c_str(), m_part_name.c_str(), m_iSLayer, m_iTLayer - 1, m_iSLayer, m_iTLayer);
						}
					}
				}
				if ((p->svch->spatial == m_iSLayer && p->svch->temporal <= m_iTLayer) || m_bKeyFrame) {
					if (bNewFrame) {
						*(unsigned int*)(p->pBuffer.get() + 1) = (unsigned int)(m_iTimestampPrev - m_iTimestamp);
						m_iTimestamp = m_iTimestampPrev;
						m_uFrameId++;
					}
					p->sh->frame_counter = (unsigned char)(m_uFrameId - 1);
					m_iQueueBytes += p->size;
					m_nTrackFrames[id(p->track)]++;
					m_lQueuePacket.push_back(std::move(p));
				}
			}
		}
		m_iVideoTimeout = ctime;
	} else {
		m_iQueueBytes += p->size;
		m_nTrackFrames[id(p->track)]++;
		m_lQueuePacket.push_back(std::move(p));
	}

	VS_MainSVCStatistics::UpdateTrashData(m_conf_name.c_str(), ctime);

	return Status::success;
}

bool SVCBuffer::IsWaitKeyFrame(std::chrono::steady_clock::time_point ct)
{
	UpdateReduceCoefs();
	bool forceKey(false);
	auto dt = ct - m_uKeyFrameTimeout;
	if (m_bNeedKey) {
		forceKey = true;
	}
	else if (m_bWaitKey) {	
		forceKey = (dt >= STAT_KEYFRAME_TM);
	}
	if (forceKey) {
		m_bNeedKey = false;
		m_uKeyFrameTimeout = ct;
	}
	return forceKey;
}

void SVCBuffer::CheckVideoFromSnd(std::chrono::steady_clock::time_point ctime)
{
	auto dt = ctime - m_iVideoTimeout;
	if (m_iVideoTimeout == std::chrono::steady_clock::time_point{} || dt >= STAT_READY_TM)
		m_iState |= NOT_VIDEO_FROM_SND;
	else m_iState &= ~NOT_VIDEO_FROM_SND;
}

int SVCBuffer::DetectChangeSpatialLayers(std::chrono::steady_clock::time_point ctime, int nsl)
{
	if (m_iSLayerNum == - 1) m_iSLayerNum = nsl;
	int dsl = nsl - m_iSLayerNum;
	m_iSLayerNum = nsl;
	if (dsl != 0) {
		m_pSenderCallback->ResetLayerStat(ctime, dsl, m_iSLayerNum);
	}
	return dsl;
}

void SVCBuffer::DetectChangeSpatialLayerSize(unsigned char *p, int size)
{
	int width(0), height(0);
	int sndFrameSize = GetFrameSize(p, size, width, height);
	if (sndFrameSize != 0) {
		int restrictSLayer = m_pSenderCallback->ResetFrameLayerCoef(sndFrameSize, width, height, GetRestrictFrameMB());
		SetSLayerId(restrictSLayer);
	}
}

int SVCBuffer::GetFrameSize(unsigned char *p, int size, int &width, int &height)
{
	int w = 0, h = 0, nth = 0, mb = 0;
	if (ResolutionFromBitstream_VPX(p, size, w, h, nth) == 0) {
		mb = w * h / 256;
	}
	else if (ResolutionFromBitstream_H264(p, size, w, h) == 0) {
		mb = w * h / 256;
	}
	width = w;
	height = h;
	return mb;
}

void SVCBuffer::AnalyseTrash()
{
	if (m_bSendTrashData) {
		auto pBuffer = vs::make_unique<char[]>(m_iBytesTrash);
		pBuffer[0] = 0; /// stream::Command::Type::Empty
		pBuffer[1] = 4; /// stream::Command::Info
		pBuffer[2] = 0; /// RetCode
		pBuffer[3] = 0; /// DataLen
		m_iQueueBytes += m_iBytesTrash;
		m_nTrackFrames[id(stream::Track::garbage)]++;
		m_lQueuePacket.push_back(vs::make_unique<SVCPacket>(std::move(pBuffer), m_iBytesTrash, 0, stream::Track::garbage, (m_uTypeSVC == 0x00000000), m_pCrypter));
	}
}

void SVCBuffer::UpdateReduceCoefs()
{

#ifdef TEST_SPEED
	AUTO_PROF
#endif

	bool bDataFromSnd = !!(m_iState & DATA_FROM_SND);
	bool bTrashData = false;
	bool bChangeReduce = false;
	bool bSysLoadLimit = (m_iReduceLoad < m_iReduceSet);
	int iSLayer = std::max(m_iSLayerMax, m_iSLayerRestrict) , iTLayer = m_iTLayerMax;
	int iTLayerMax = m_pSenderCallback->GetNumLayers(SVC_TEMPORAL) - 1;
	if (m_iState & DISABLE_VIDEO) {
		iTLayer = -1;
	} else if (m_iState & NOT_VIDEO_FROM_SND) {
		iTLayer = m_iTLayerMax;
		if (m_bChangeSLRestrict) {
			iSLayer = m_pSenderCallback->GetSLayerId(m_iReduce);
			iSLayer = std::max(iSLayer, m_iSLayerRestrict);
		}
	} else {
		bTrashData = (m_iState & DATA_TRASH) && !bDataFromSnd;
		int newReduce = std::min(m_iReduceSet, m_iReduceLoad);
		bChangeReduce = m_iReduce != newReduce;
		m_iReduce = newReduce;
		iSLayer = m_pSenderCallback->GetSLayerId(m_iReduce);
		iTLayer = m_pSenderCallback->GetTLayerId(m_iReduce);
		if (m_iSLayerRestrict >= iSLayer) {
			if (m_iSLayerRestrict > iSLayer) iTLayer = iTLayerMax;
			iSLayer = m_iSLayerRestrict;
			m_iBytesTrash = 0;
		} else if (iTLayer == iTLayerMax && iSLayer > 0) {
			if (bChangeReduce || m_iBytesTrash == 0) {
				//m_iBytesTrash = 0;
				int dr = m_iReduce - m_pSenderCallback->GetBaseReduce(m_iReduce);
				double db = dr * m_iBitrateSet * 128.0 / 100.0;
				if (db > 0) {
					m_iBytesTrash = (int)(db * std::chrono::duration_cast<std::chrono::seconds>(STAT_TRASH_TM).count());
					if (m_iBytesTrash < 4) m_iBytesTrash = 4;
				}
			}
		}/* else {
			m_iBytesTrash = 0;
		}*/
		if (!(m_iState & BUFFER_WORST) && !bDataFromSnd && !bSysLoadLimit) iTLayer = iTLayerMax;
	}
	bool bMaxTLayer = m_iTLayer == iTLayerMax && m_iSLayer > 0;
	if (bTrashData && m_iBytesTrash > 0 && bMaxTLayer) {
		if (!m_bSendTrashData) {
			if (m_iTypeDbgStat > 0) {
				fprintf(m_fStat, "\n %s : %s : start Trash Data : (% d, % d), %5d x %4d",
						m_conf_name.c_str(), m_part_name.c_str(), iSLayer, iTLayer, m_iBytesTrash, std::chrono::duration_cast<std::chrono::duration<int, std::milli>>(STAT_TRASH_TM).count());
			}
			m_bSendTrashData = true;
		} else if (m_bSendTrashData && bChangeReduce) {
			if (m_iTypeDbgStat > 0) {
				fprintf(m_fStat, "\n %s : %s : change Trash Data : (% d, % d), %5d x %4d",
						m_conf_name.c_str(), m_part_name.c_str(), iSLayer, iTLayer, m_iBytesTrash, std::chrono::duration_cast<std::chrono::duration<int, std::milli>>(STAT_TRASH_TM).count());
			}
		}
	} else {
		if (m_bSendTrashData) {
			if (m_iTypeDbgStat > 0) {
				fprintf(m_fStat, "\n %s : %s : end Trash Data (%d, %d, %d)", m_conf_name.c_str(), m_part_name.c_str(), bTrashData, bMaxTLayer, m_iBytesTrash);
			}
			m_uTimeoutLayer = std::chrono::steady_clock::now();
			m_bSendTrashData = false;
		}
		if (!bMaxTLayer) m_iBytesTrash = 0;
	}
	if (iSLayer != m_iSLayerMax && m_bDynChangeFmt) {
		if (iSLayer < m_iSLayerMax) {
			m_iTLayerMax = iTLayer;
		} else {
			if (iTLayer == -1) SetTLayerId(iTLayer);
			else m_iTLayerMax = iTLayer;
		}
		m_bNeedKey = true;
		m_iSLayerMax = iSLayer;
	} else if (iTLayer != m_iTLayerMax) {
		SetTLayerId(iTLayer);
	}
	m_bChangeSLRestrict = false;
}

#ifdef TEST_CRASH

void SVCBuffer::CouplingFrame(SVCPacket *p, int idx)
{
	if (idx == 1) return;
	int sl = p->svch->spatial;
	if (p->sh->frame_counter != m_avi_frame_id[idx][sl]) {
		if (m_tmstp_prev[idx][sl] != -1) {
			fprintf(out_stat[idx][sl], "!!!frame write!!! : svc = %d cnt = %6d/%6d id = %3d npckt = %2d s = %6d key = %2d dt = %8d\n",
										m_svc_layer[idx],
										m_num_frames[idx][sl], m_num_frames[0][sl],
										m_avi_frame_id[idx][sl], m_num_pckt[idx][sl], m_size_avi_frame[idx][sl],
										(int)m_bKey_Avi[idx][sl], (int)m_tmstp_prev[idx][sl]);
			//m_avifile.WriteVideo(m_avi_frame, m_size_avi_frame, m_bKey_Avi, (unsigned long)m_tmstp_prev);
			m_avifile[idx][sl]->WriteVideo(m_avi_frame[idx][sl], m_size_avi_frame[idx][sl], m_bKey_Avi[idx][sl]);
		}
		m_tmstp_prev[idx][sl] = *(unsigned int*)(p->pBuffer + 1);
		m_avi_frame_id[idx][sl] = p->sh->frame_counter;
		m_bKey_Avi[idx][sl] = p->pBuffer[0] != 0;
		m_size_avi_frame[idx][sl] = p->size - 5 - sizeof(stream::SliceHeader);
		m_svc_layer[idx][sl] = p->svch->temporal;
		m_num_pckt[idx][sl] = 1;
		m_num_frames[idx][sl]++;
		memcpy(m_avi_frame[idx][sl], p->pBuffer + 5, m_size_avi_frame[idx][sl]);
		//fprintf(out_stat, "svc = %d clc_cnt = %3d num_sct = %2d all_sct = %2d s = %4d\n", p->svch->temporal, p->sh->frame_counter, p->sh->id, p->sh->first_id, m_size_avi_frame);
	} else {
		memcpy(m_avi_frame[idx][sl] + m_size_avi_frame[idx][sl], p->pBuffer, p->size - sizeof(stream::SliceHeader));
		m_size_avi_frame[idx][sl] += p->size - sizeof(stream::SliceHeader);
		m_num_pckt[idx][sl]++;
		//fprintf(out_stat, "svc = %d clc_cnt = %3d num_sct = %2d all_sct = %2d s = %4d\n", p->svch->temporal, p->sh->frame_counter, p->sh->id, p->sh->first_id, p->size - sizeof(stream::SliceHeader));
	}

	//if (p->sh->id == p->sh->first_id) m_bKey_Avi[idx] = p->pBuffer[0] != 0;
	//fprintf(out_stat[idx], "l = (%d, %d), clc_cnt = %3d, sct = (%2d, %2d), key = %d, s = %4d\n",
	//						p->svch->spatial, p->svch->temporal, p->sh->frame_counter,
	//						p->sh->id, p->sh->first_id, m_bKey_Avi[idx], p->size - sizeof(stream::SliceHeader));
}

#endif

auto SVCBuffer::GetFrame(uint32_t& tick_count, Track& track, std::unique_ptr<char[]>& buffer, size_t& size) -> Status
{
	if (m_bLogicalBuffer)
		return Status::non_fatal;

#ifdef TEST_SPEED
	AUTO_PROF
#endif

	Status ret = Status::non_fatal;
	tick_count = 0;
	track = {};
	buffer.reset();
	size = 0;
	auto ctime = std::chrono::steady_clock::now();
	if (!m_lQueuePacket.empty()) {
		auto it = m_lQueuePacket.begin();
		SVCPacket* p = it->get();

#ifdef TEST_CRASH
		//if (p->track == 2) CouplingFrame(p, 1);
#endif

		if (p->track == stream::Track::data && m_nTrackFrames[id(stream::Track::audio)] > 0 && m_nTrackFrames[id(stream::Track::video)] == 0) {
			auto et = m_lQueuePacket.end();
			while (it != et) {
				if ((*it)->track != stream::Track::data) break;
				++it;
			}
			if (it == et) it = m_lQueuePacket.begin();
			p = it->get();
		}
		m_iQueueBytes -= p->size; /// decrease queue size before GetBuffer() !!! may be crypted
		tick_count = p->timestamp;
		track = p->track;
		buffer = p->GetBuffer();
		size = p->size; /// return size only after GetBuffer() !!!! may be crypted
		m_lQueuePacket.erase(it);
		m_nTrackFrames[id(track)]--;
		if (track == stream::Track::garbage)
			track = stream::Track::command; /// trash data
		ret = Status::success;
	}
	if (m_iState & BUFFER_CONNECT) {
		AnalyseQueuePacket(ctime);
	} else {
		m_iPacketsToConnect++;
		if (m_iTypeDbgStat > 0) fprintf(m_fStat, "\n NOT CONNECT buffer %s", m_part_name.c_str());
		if (m_iPacketsToConnect >= 2) {
			m_iState |= BUFFER_CONNECT;
			if (m_iTypeDbgStat > 0) fprintf(m_fStat, "\n CONNECT buffer %s", m_part_name.c_str());
		}
	}
	return ret;
}

unsigned SVCBuffer::GetFrameCount() const
{
	return m_lQueuePacket.size();
}

unsigned SVCBuffer::GetFrameCount(Track track) const
{
	return m_nTrackFrames[id(track)];
}

void SVCBuffer::ReduceTrash()
{

#ifdef TEST_SPEED
	AUTO_PROF
#endif

	for (auto it = m_lQueuePacket.begin(), et = m_lQueuePacket.end(); it != et; ) {
		SVCPacket* p = it->get();
		if (p->track == stream::Track::garbage) {
			m_iQueueBytes -= p->size;
			m_nTrackFrames[id(stream::Track::garbage)]--;
			it = m_lQueuePacket.erase(it);
		} else {
			++it;
		}
	}
}

void stream::SVCBuffer::ReduceQueue(int iLevel)
{

#ifdef TEST_SPEED
	AUTO_PROF
#endif

	int bytes_remove = 0;
	unsigned char uNumRemove = 0;
	unsigned int uDeltaTime = 0;
	bool bStartRemove = false;
	bool bKeyFrame = false;
	for (auto it = m_lQueuePacket.begin(), et = m_lQueuePacket.end(); it != et; ) {
		SVCPacket* p = it->get();
		if (p->track != stream::Track::video) {
			++it;
			continue;
		}
		bool bNewFrame = p->sh->id == p->sh->first_id;
		if (bNewFrame) bKeyFrame = p->pBuffer[0] != 0;
		if (!bStartRemove) bStartRemove = bNewFrame;
		bool bRemove = (bStartRemove && p->svch->temporal > iLevel && !bKeyFrame);
		if (bRemove) {
			if (bNewFrame) {
				uNumRemove++;
				uDeltaTime += *(unsigned int*)(p->pBuffer.get() + 1);
			}
			bytes_remove += p->size;
			it = m_lQueuePacket.erase(it);
			m_nTrackFrames[id(stream::Track::video)]--;
		} else {
			p->sh->frame_counter -= uNumRemove;
			*(unsigned int*)(p->pBuffer.get() + 1) += uDeltaTime;
			uDeltaTime = 0;
			++it;
		}
	}
	m_uFrameId -= uNumRemove;
	m_iTimestamp -= uDeltaTime;
	m_iQueueBytes -= bytes_remove;
	m_iTLayer = iLevel;
}

void SVCBuffer::SetState(unsigned int iState)
{
	unsigned int nState = (~iState) & 0xffffffff;
	if (nState == DISABLE_VIDEO || nState == DISABLE_DATA || nState == NOT_VIDEO_FROM_SND ||
		nState == DATA_FROM_SND || nState == BUFFER_WORST || nState == DATA_TRASH || nState == SYSLOAD_INIT) {
		m_iState &= iState;
	} else {
		m_iState |= iState;
		if (iState == DISABLE_VIDEO || iState == NOT_VIDEO_FROM_SND) m_iState &= (~DATA_TRASH);
	}
}

void SVCBuffer::SetTLayerId(int iLayer) {
	if (iLayer < m_iTLayer) {
		ReduceQueue(iLayer);
		m_uTimeoutLayer = std::chrono::steady_clock::now();
	}
	m_iTLayerMax = iLayer;
}

void SVCBuffer::SetSLayerId(int iLayer)
{
	if (iLayer != m_iSLayerRestrict) {
		m_bChangeSLRestrict = true;
		if (m_iTypeDbgStat > 0) {
			fprintf(m_fStat, "\n RESTRICT MB %s : %s : %d -> %d (wsl = %d, lsl = %d)",
					m_conf_name.c_str(), m_part_name.c_str(), m_iSLayerRestrict, iLayer, m_maxWindowRestrictMB, m_maxLevelRestrictMB);
		}
	}
	m_iSLayerRestrict = iLayer;
}

bool SVCBuffer::ReduceQueueByTrack(stream::Track track, int bytes_rcv, bool bQueueBtrFull)
{

#ifdef TEST_SPEED
	AUTO_PROF
#endif

	if (m_nTrackFrames[id(track)] > 0 && ((int)m_lQueuePacket.size() > m_iQueueLengthMax || bQueueBtrFull)) {
		if (m_iTypeDbgStat > 0) {
			fprintf(m_fStat, "\n %s : %s : erase %d track : load = (%4d, %6d, %3zu)",
					m_conf_name.c_str(), m_part_name.c_str(), static_cast<int>(id(track)), (bytes_rcv > 0) ? (m_iQueueBytes * 100) / bytes_rcv : 0, m_iQueueBytes, m_lQueuePacket.size());
		}
		for (auto it = m_lQueuePacket.begin(), et = m_lQueuePacket.end(); it != et; ) {
			SVCPacket* p = it->get();
			if (p->track == track) {
				m_iQueueBytes -= p->size;
				m_nTrackFrames[id(p->track)]--;
				it = m_lQueuePacket.erase(it);
			} else {
				++it;
			}
		}
		bQueueBtrFull = (bytes_rcv > 0 && m_iQueueBytes > QBYTES_MIN) ? (m_iQueueBytes * 100 > bytes_rcv * DETECT_QCLEAR_PERCENT) : false;
	}
	return bQueueBtrFull;
}

void SVCBuffer::EraseQueue(int bytes_rcv, bool bQueueBtrFull)
{

#ifdef TEST_SPEED
	AUTO_PROF
#endif

	if ((int)m_lQueuePacket.size() > m_iQueueLengthMax || bQueueBtrFull) {
		if (m_iTypeDbgStat > 0) {
			fprintf(m_fStat, "\n %s : %s : erase all track : load = (%4d, %6d, %3zu)",
					m_conf_name.c_str(), m_part_name.c_str(), (bytes_rcv > 0) ? (m_iQueueBytes * 100) / bytes_rcv : 0, m_iQueueBytes, m_lQueuePacket.size());
		}
		while (!m_lQueuePacket.empty()) {
			SVCPacket* p = m_lQueuePacket.front().get();
			m_nTrackFrames[id(p->track)]--;
			m_lQueuePacket.pop_front();
		}
		m_iQueueBytes = 0;
	}
}

void SVCBuffer::AnalyseQueuePacket(std::chrono::steady_clock::time_point ctime, bool bFromSnd)
{
	if (m_uAnalyseTimeSnd == std::chrono::steady_clock::time_point{} && bFromSnd)
		m_uAnalyseTimeSnd = ctime + STAT_READY_TM;
	if (m_uAnalyseTimeRcv == std::chrono::steady_clock::time_point{} && !bFromSnd)
		m_uAnalyseTimeRcv = ctime + STAT_READY_TM;
	auto dt = bFromSnd ? ctime - m_uAnalyseTimeSnd : ctime - m_uAnalyseTimeRcv;
	if (dt >= std::chrono::seconds(1)) {

#ifdef TEST_SPEED
		AUTO_PROF
#endif

		m_iState |= BUFFER_READY;
		int bytes_rcv = GetBytes();
		if (!bFromSnd) {
			if (bytes_rcv > 0) {
				int avg_qbytes = GetQueueBytesAvg();
				if (avg_qbytes > QBYTES_MIN) {
					bool bLevelChange = (ctime - m_uTimeoutLayer) >= DETECT_LAYERDOWN_TM &&
										((avg_qbytes * 100 > bytes_rcv * DETECT_LAYER_PERCENT && m_iTLayer > 0) ||
										(avg_qbytes * 100 > bytes_rcv * DETECT_KEYFRAME_PERCENT && m_iTLayer == 0));
					if (bLevelChange) {
						m_uTimeoutLayer = ctime;
						if (m_nTrackFrames[id(stream::Track::garbage)] > 0) {
							if (m_iTypeDbgStat > 0) {
								fprintf(m_fStat, "\n %s : %s : reduce Trash : (% d, % d), load = (%4d, %6d/%6d, %6d, %3d/%3d)", m_conf_name.c_str(),
										m_part_name.c_str(), m_iSLayer, m_iTLayer,
										(avg_qbytes * 100) / bytes_rcv, avg_qbytes, GetInstantQueueBytes(),
										GetInstantBytes(), GetQueueLenAvg(), GetInstantQueueLen());
							}
							ReduceTrash();
							m_uTimeoutLayer += (DETECT_TRASHDOWN_TM - DETECT_LAYERDOWN_TM);
						} else {
							if (m_iTypeDbgStat > 0) {
								fprintf(m_fStat, "\n %s : %s : TL down : (% d, % d) -> (% d, % d), load = (%4d, %6d/%6d, %6d, %3d/%3d)", m_conf_name.c_str(),
										m_part_name.c_str(), m_iSLayer, m_iTLayer, m_iSLayer, m_iTLayer - 1,
										(avg_qbytes * 100) / bytes_rcv, avg_qbytes, GetInstantQueueBytes(),
										GetInstantBytes(), GetQueueLenAvg(), GetInstantQueueLen());
							}
							ReduceQueue(m_iTLayer-1);
						}
						m_iState &= (~DATA_TRASH);
					}
				}
			}
			m_uAnalyseTimeRcv = ctime;
		} else {
			bool bQueueBtrFull = (bytes_rcv > 0 && m_iQueueBytes > QBYTES_MIN) ? (m_iQueueBytes * 100 > bytes_rcv * DETECT_QCLEAR_PERCENT) : false;
			bQueueBtrFull = ReduceQueueByTrack(stream::Track::garbage, bytes_rcv, bQueueBtrFull);
			bQueueBtrFull = ReduceQueueByTrack(stream::Track::video, bytes_rcv, bQueueBtrFull);
			bQueueBtrFull = ReduceQueueByTrack(stream::Track::data, bytes_rcv, bQueueBtrFull);
			EraseQueue(bytes_rcv, bQueueBtrFull);
			m_uAnalyseTimeSnd = ctime;
		}
	}
}

}
