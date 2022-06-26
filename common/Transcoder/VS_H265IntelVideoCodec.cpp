#ifdef _WIN32
#include "VS_H265IntelVideoCodec.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "MfxUtils.h"
#include "intel_mdk/mfxplugin.h"

// TODO: get rid
#include "VSClient/VS_ApplicationInfo.h"
#include "ipp.h"
#include "IppLib2/libinit.h"
#include <Windows.h>

#include <algorithm>

VS_H265IntelVideoCodec::VS_H265IntelVideoCodec(int CodecId, bool IsCoder) : VideoCodec(CodecId, IsCoder)
{
	m_valid = false;
	m_pmfxEncoder = 0;
	m_pmfxDecoder = 0;
	m_pmfxVPP = 0;
	m_pSurfaces = 0;
	m_bForceSoftware = false;
	m_numFramesQueue = 0;
	m_mfxBS.Data = 0;
	memset(&m_mfxParams, 0, sizeof(m_mfxParams));
	memset(&m_mfxVppParams, 0, sizeof(m_mfxVppParams));
	m_bitrate = 1000;
	m_bInitFromBs = false;
	m_bNextKey = false;
	m_bWaitKey = false;
	m_uLastKeyRequest = 0;
	memset(&m_mfxBS, 0, sizeof(m_mfxBS));
	memset(&m_CodingExtOption, 0, sizeof(m_CodingExtOption));
	ResetExtOptions();
	m_mfxVersion.Major = 1;
	m_mfxVersion.Minor = 9;

#ifdef TEST_MDK_VCODEC
	char name[128];
	sprintf(name, "stat_encoder_%x.txt", this);
	m_fStat = fopen(name, "w");
	m_numPuchFrames = 0;
#endif

}

VS_H265IntelVideoCodec::~VS_H265IntelVideoCodec()
{

#ifdef TEST_MDK_VCODEC
	fclose(m_fStat);
#endif

	Release();
}

void VS_H265IntelVideoCodec::ResetExtOptions()
{
	memset(&m_CodingExtOption, 0, sizeof(m_CodingExtOption));
	m_CodingExtOption.Header.BufferSz = sizeof(m_CodingExtOption);
	m_CodingExtOption.Header.BufferId = MFX_EXTBUFF_CODING_OPTION;
	m_CodingExtOption.NalHrdConformance = MFX_CODINGOPTION_OFF;
	m_CodingExtOption.MaxDecFrameBuffering = 1;
	m_pCodingExtBuf = (mfxExtBuffer*)&m_CodingExtOption;
}

mfxStatus VS_H265IntelVideoCodec::AllocSurfaces(bool bCoder)
{
	mfxStatus sts = MFX_ERR_NONE;

	mfxFrameAllocRequest request;
	mfxU16 nEncSurfNum = 0; // number of surfaces for encoder
	memset(&request, 0, sizeof(request));
	request.Type = MFX_MEMTYPE_SYSTEM_MEMORY;
	if (bCoder) {
		request.Type |= MFX_MEMTYPE_EXTERNAL_FRAME | MFX_MEMTYPE_FROM_ENCODE;
		sts = m_pmfxEncoder->QueryIOSurf(&m_mfxParams, &request);
	}
	else {
		request.Type |= MFX_MEMTYPE_EXTERNAL_FRAME | MFX_MEMTYPE_FROM_DECODE;
		sts = m_pmfxDecoder->QueryIOSurf(&m_mfxParams, &request);
	}
	if (sts == MFX_ERR_NONE) {
		m_numFramesQueue += request.NumFrameSuggested;
		m_numFramesQueue = std::max<decltype(m_numFramesQueue)>(1, m_numFramesQueue);
		m_pSurfaces = new mfxFrameSurface1[m_numFramesQueue];
		for (int i = 0; i < m_numFramesQueue; i++) {
			memset(&(m_pSurfaces[i]), 0, sizeof(mfxFrameSurface1));
			memcpy(&(m_pSurfaces[i].Info), &(m_mfxParams.mfx.FrameInfo), sizeof(mfxFrameInfo));
			int w = m_mfxParams.mfx.FrameInfo.Width;
			int h = m_mfxParams.mfx.FrameInfo.Height;
			m_pSurfaces[i].Data.Y = new unsigned char[w * h * 3 / 2];
			m_pSurfaces[i].Data.U = m_pSurfaces[i].Data.Y + w * h;
			m_pSurfaces[i].Data.V = m_pSurfaces[i].Data.U + 1;
			m_pSurfaces[i].Data.Pitch = w;
		}
	}

	return sts;
}

void VS_H265IntelVideoCodec::DeleteSurfaces()
{
	for (int i = 0; i < m_numFramesQueue; i++) delete[] m_pSurfaces[i].Data.Y;
	delete[] m_pSurfaces;
	m_numFramesQueue = 0;
	m_pSurfaces = 0;
}

int VS_H265IntelVideoCodec::Init(int w, int h, uint32_t ColorMode, unsigned char sndLvl, int numThreads, unsigned int framerate)
{
	mfxStatus sts = MFX_ERR_NONE;

	mfxVersion version;
	version.Major = 1;
	version.Minor = 9;

	mfxIMPL impl = (m_bForceSoftware) ? MFX_IMPL_SOFTWARE : MFX_IMPL_HARDWARE_ANY;
	sts = m_mfxSession.Init(impl, &version);
	if (sts == MFX_ERR_NONE) {
		sts = m_mfxSession.QueryVersion(&m_mfxVersion);
		if (sts == MFX_ERR_NONE) {
			m_mfxParams.mfx.CodecId = MFX_CODEC_HEVC;
			m_mfxParams.mfx.NumThread = 0;
			if (IsCoder()) {
				sts = MFXVideoUSER_Load(m_mfxSession, &MFX_PLUGINID_HEVCE_HW, 1);
				if (sts == MFX_ERR_NONE) {
					m_bNextKey = false;
					m_bWaitKey = false;
					m_mfxParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
					///
					int level_idc = 31;
					VS_RegistryKey key(true, REG_CurrentConfiguratuon);
					key.GetValue(&level_idc, 4, VS_REG_INTEGER_VT, "H.265 Level IDC");
					m_mfxParams.mfx.TargetUsage = (impl == MFX_IMPL_SOFTWARE) ? MFX_TARGETUSAGE_BALANCED : MFX_TARGETUSAGE_BEST_QUALITY;
					m_mfxParams.mfx.EncodedOrder = 0;
					m_mfxParams.mfx.NumThread = 0;
					m_mfxParams.mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN;
					m_mfxParams.mfx.CodecLevel = level_idc;
					/// bitrate
					m_bitrate = 20000;
					m_mfxParams.mfx.RateControlMethod = MFX_RATECONTROL_VBR;
					m_mfxParams.mfx.TargetKbps = (unsigned short)m_bitrate;
					m_mfxParams.mfx.MaxKbps = (unsigned short)(3 * m_bitrate / 2);
					/// other
					m_mfxParams.mfx.GopPicSize = 1000;
					m_mfxParams.mfx.IdrInterval = 0;
					m_mfxParams.mfx.GopRefDist = 1; // only I, P
					m_mfxParams.mfx.NumSlice = 0;// (impl == MFX_IMPL_SOFTWARE) ? 0 : h / 16;
					m_mfxParams.mfx.NumRefFrame = 1;
					/// type frame
					m_mfxParams.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
					m_mfxParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
					m_mfxParams.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
					m_mfxParams.mfx.FrameInfo.Width = (w + 31) & ~31;
					m_mfxParams.mfx.FrameInfo.Height = (h + 31) & ~31;
					m_mfxParams.mfx.FrameInfo.CropX = 0;
					m_mfxParams.mfx.FrameInfo.CropY = 0;
					m_mfxParams.mfx.FrameInfo.CropW = w;
					m_mfxParams.mfx.FrameInfo.CropH = h;
					m_mfxParams.mfx.FrameInfo.FrameRateExtN = framerate;
					m_mfxParams.mfx.FrameInfo.FrameRateExtD = 1;
					/// ext settings
					m_mfxParams.NumExtParam = 1;
					m_mfxParams.ExtParam = &m_pCodingExtBuf;
					m_CodingExtOption.RateDistortionOpt = (impl == MFX_IMPL_SOFTWARE) ? MFX_CODINGOPTION_OFF : MFX_CODINGOPTION_ON;
					///
					mfxVideoParam inPar;
					memcpy(&inPar, &m_mfxParams, sizeof(m_mfxParams));
					/// init
					m_pmfxEncoder = new MFXVideoENCODE(m_mfxSession);
					sts = MFX_ERR_NOT_INITIALIZED;
					if (m_pmfxEncoder) {
						sts = m_pmfxEncoder->Close();
						if (sts == MFX_ERR_NOT_INITIALIZED) sts = MFX_ERR_NONE;
						if (sts == MFX_ERR_NONE) {
							sts = m_pmfxEncoder->Query(&inPar, &m_mfxParams);
							if (sts >= MFX_ERR_NONE) {
								DeleteSurfaces();
								sts = AllocSurfaces(true);
								if (sts == MFX_ERR_NONE) {
									sts = m_pmfxEncoder->Init(&m_mfxParams);
									sts = m_pmfxEncoder->GetVideoParam(&inPar);
									if (sts > MFX_ERR_NONE) sts = MFX_ERR_NONE;
								}
							}
						}
					}
				}
			}
			else {
				sts = MFXVideoUSER_Load(m_mfxSession, &MFX_PLUGINID_HEVCD_HW, 1);
				if (sts == MFX_ERR_NONE) {
					m_mfxParams.AsyncDepth = 1;
					m_mfxParams.IOPattern = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
					m_mfxParams.mfx.DecodedOrder = 0;
					m_mfxParams.mfx.NumThread = 1;
					/// init
					m_pmfxDecoder = new MFXVideoDECODE(m_mfxSession);
					sts = MFX_ERR_NOT_INITIALIZED;
					if (m_pmfxDecoder) {
						sts = m_pmfxDecoder->Close();
						if (sts == MFX_ERR_NOT_INITIALIZED) sts = MFX_ERR_NONE;
					}
				}
			}
		}
	}

#ifdef TEST_MDK_VCODEC
	fprintf(m_fStat, "\n %d.%d : %d : %d", version.Major, version.Minor, sts, m_mfxParams.mfx.NumRefFrame);
#endif

	m_valid = true;

	if (sts != MFX_ERR_NONE) {
		Release();
	}
	else {
		m_mfxBS.Data = new unsigned char[0x800000];
		m_mfxBS.MaxLength = 0x800000;
		m_bInitFromBs = false;
		IppLibInit();
	}

	return sts;
}

void VS_H265IntelVideoCodec::Release()
{
	m_valid = false;
	m_bitrate = 1000;
	m_bitrate_prev = m_bitrate;
	m_bInitFromBs = false;

	if (IsCoder()) {
		MFXVideoUSER_UnLoad(m_mfxSession, &MFX_PLUGINID_HEVCE_HW);
	}
	else {
		MFXVideoUSER_UnLoad(m_mfxSession, &MFX_PLUGINID_HEVCD_HW);
	}

	delete m_pmfxEncoder; m_pmfxEncoder = 0;
	delete m_pmfxDecoder; m_pmfxDecoder = 0;
	delete m_pmfxVPP; m_pmfxVPP = 0;

	delete[] m_mfxBS.Data;
	memset(&m_mfxBS, 0, sizeof(m_mfxBS));

	ResetExtOptions();
	DeleteSurfaces();

	m_mfxSession.Close();
}

bool VS_H265IntelVideoCodec::UpdateBitrate()
{
	if (m_bitrate_prev == m_bitrate) return true;
	m_bitrate_prev = m_bitrate;
	mfxVideoParam inPar;
	memset(&inPar, 0, sizeof(inPar));
	mfxStatus sts = m_pmfxEncoder->GetVideoParam(&inPar);
	inPar.NumExtParam = 1;
	inPar.ExtParam = &m_pCodingExtBuf;
	inPar.mfx.TargetKbps = (m_bitrate_prev == 1000) ? 1001 : (mfxU16)m_bitrate_prev;
	sts = m_pmfxEncoder->Query(&inPar, &m_mfxParams);
	if (sts >= MFX_ERR_NONE) {
		sts = m_pmfxEncoder->Reset(&m_mfxParams);
		sts = m_pmfxEncoder->GetVideoParam(&m_mfxParams);
		if (sts >= MFX_ERR_NONE) return true;
	}
	return false;
}

int VS_H265IntelVideoCodec::Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param)
{
	int ret = 0;

	mfxStatus sts = MFX_ERR_NONE;
	IppStatus st = ippStsNoErr;
	mfxFrameSurface1* pSurf = NULL;
	unsigned short nEncSurfIdx = 0xffff;

	if (IsCoder()) {
		UpdateBitrate();
		nEncSurfIdx = GetFreeSurface(m_pSurfaces, m_numFramesQueue);
		if (nEncSurfIdx != 0xffff) {
			pSurf = &m_pSurfaces[nEncSurfIdx];
			mfxFrameData *pData = &pSurf->Data;
			mfxFrameInfo *pInfo = &pSurf->Info;
			pInfo->FrameId.ViewId = 0;
			/// convert i420 -> nv12
			IppiSize roi = { pInfo->CropW, pInfo->CropH };
			const unsigned char *pSrc[3] = { in, in + pInfo->CropW * pInfo->CropH, in + 5 * pInfo->CropW * pInfo->CropH / 4 };
			int srcStep[3] = { pInfo->CropW, pInfo->CropW >> 1, pInfo->CropW >> 1 };
			st = ippiYCbCr420_8u_P3P2R(pSrc, srcStep, pData->Y, pData->Pitch, pData->UV, pData->Pitch, roi);
			///
			if (st == ippStsNoErr) {
				m_mfxBS.DataOffset = 0;
				m_mfxBS.DataLength = 0;
				m_mfxBS.FrameType = 0;
				mfxSyncPoint EncSyncP;
				mfxEncodeCtrl ctrlEnc;
				memset(&ctrlEnc, 0, sizeof(ctrlEnc));
				unsigned int ct = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
				if (m_uLastKeyRequest == 0) m_uLastKeyRequest = ct;
				if (param->cmp.KeyFrame == 1 || m_bNextKey || (m_bWaitKey && (ct - m_uLastKeyRequest >= 1000))) {
					ctrlEnc.FrameType = MFX_FRAMETYPE_I | MFX_FRAMETYPE_REF | MFX_FRAMETYPE_IDR;
					m_bWaitKey = true;
					m_bNextKey = false;
					m_uLastKeyRequest = ct;
				}
				else {
					ctrlEnc.FrameType = MFX_FRAMETYPE_UNKNOWN;
				}
				for (;;) {
					param->cmp.IsKeyFrame = 0;
					sts = m_pmfxEncoder->EncodeFrameAsync(&ctrlEnc, pSurf, &m_mfxBS, &EncSyncP);
					if (MFX_ERR_MORE_DATA == sts) {
						if (!m_bWaitKey) m_bNextKey = true;
						break;
					}
					else if (MFX_ERR_NONE <= sts && !EncSyncP) {
						if (MFX_WRN_DEVICE_BUSY == sts) Sleep(1);
					}
					else {
						ret = -1;
						if (MFX_ERR_NONE <= sts && EncSyncP) {
							sts = m_mfxSession.SyncOperation(EncSyncP, INFINITE);
							if (sts >= MFX_ERR_NONE) {
								memcpy(out, m_mfxBS.Data + m_mfxBS.DataOffset, m_mfxBS.DataLength);
								ret = m_mfxBS.DataLength;
								if (m_mfxBS.FrameType & MFX_FRAMETYPE_I) {
									param->cmp.IsKeyFrame = 1;
									m_bWaitKey = false;
								}
								else {
									if (m_bWaitKey) ret = 0;
								}
							}
						}
						break;
					}
				}
			}
		}

#ifdef TEST_MDK_VCODEC
		m_numPuchFrames++;
		mfxEncodeStat stat;
		sts = MFXVideoENCODE_GetEncodeStat(m_mfxSession, &stat);
		fprintf(m_fStat, "\n %2d, %3d : %5d %5d %5d", nEncSurfIdx, sts, m_numPuchFrames, stat.NumFrame, stat.NumCachedFrame);
#endif

	}
	else {

		ret = -1;

		m_mfxBS.DataOffset = 0;
		m_mfxBS.DataLength = param->dec.FrameSize;
		m_mfxBS.DataFlag = MFX_BITSTREAM_COMPLETE_FRAME;
		memcpy(m_mfxBS.Data, in, param->dec.FrameSize);

		if (!m_bInitFromBs) {
			sts = m_pmfxDecoder->DecodeHeader(&m_mfxBS, &m_mfxParams);
		}

		if (sts == MFX_WRN_VIDEO_PARAM_CHANGED || (sts == MFX_ERR_NONE && !m_bInitFromBs)) {
			DeleteSurfaces();
			sts = m_pmfxDecoder->Query(&m_mfxParams, &m_mfxParams);
			if (sts >= MFX_ERR_NONE) {
				sts = AllocSurfaces(false);
				if (sts == MFX_ERR_NONE) {
					sts = m_pmfxDecoder->Init(&m_mfxParams);
					if (sts >= MFX_ERR_NONE) {
						m_bInitFromBs = true;
						sts = MFX_ERR_NONE;
					}
				}
			}
		}

		if (m_bInitFromBs) {
			ret = -1;
			mfxFrameSurface1 *pSurfDec = 0;
			nEncSurfIdx = GetFreeSurface(m_pSurfaces, m_numFramesQueue);
			if (nEncSurfIdx != 0xffff) {
				pSurf = &m_pSurfaces[nEncSurfIdx];
				mfxSyncPoint DecSyncP;
				bool tryEncode = true;
				for (;;) {
					sts = m_pmfxDecoder->DecodeFrameAsync(&m_mfxBS, pSurf, &pSurfDec, &DecSyncP);
					if (sts == MFX_WRN_VIDEO_PARAM_CHANGED) {
						sts = m_pmfxDecoder->DecodeFrameAsync(&m_mfxBS, pSurf, &pSurfDec, &DecSyncP);
					}
					if (sts >= MFX_ERR_NONE && DecSyncP) {
						sts = m_mfxSession.SyncOperation(DecSyncP, INFINITE);
						if (sts >= MFX_ERR_NONE) {
							mfxFrameData *pData = &pSurfDec->Data;
							mfxFrameInfo *pInfo = &pSurfDec->Info;
							/// convert nv12 -> i420
							IppiSize roi = { pInfo->CropW, pInfo->CropH };
							int dst_step[3] = { pInfo->CropW, pInfo->CropW >> 1, pInfo->CropW >> 1 };
							Ipp8u *pDst[3] = { out, out + pInfo->CropW * pInfo->CropH, out + 5 * pInfo->CropW * pInfo->CropH / 4 };
							st = ippiYCbCr420_8u_P2P3R(pData->Y, pData->Pitch, pData->UV, pData->Pitch, pDst, dst_step, roi);
							///
							if (st == ippStsNoErr) ret = 3 * pInfo->CropW * pInfo->CropH / 2;
							break;
						}
					}
					else {
						if (sts == MFX_WRN_DEVICE_BUSY && tryEncode) {
							tryEncode = false;
							Sleep(5);
						}
						else {
							break;
						}
					}
				}
			}
		}

	}
	return ret;
}

bool VS_H265IntelVideoCodec::SetCoderOption(void *param)
{
	if (!m_pmfxEncoder) return false;
	h264_Param *inpar = (h264_Param*)param;
	mfxVideoParam inPar;
	memset(&inPar, 0, sizeof(inPar));
	mfxStatus sts = m_pmfxEncoder->GetVideoParam(&inPar);
	inPar.NumExtParam = 1;
	inPar.ExtParam = &m_pCodingExtBuf;
	inPar.mfx.TargetKbps = (inpar->bitrate == 1000) ? 1001 : (mfxU16)inpar->bitrate;
	inPar.mfx.MaxKbps = 3 * inpar->bitrate / 2;
	if (inpar->me_quality == 0) inPar.mfx.TargetUsage = MFX_TARGETUSAGE_BEST_SPEED;
	else if (inpar->me_quality == 1) inPar.mfx.TargetUsage = MFX_TARGETUSAGE_BALANCED;
	else if (inpar->me_quality >= 2) inPar.mfx.TargetUsage = MFX_TARGETUSAGE_BEST_QUALITY;
	//inPar.mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN;// (inpar->cabac_init_idc > 0) ? MFX_PROFILE_AVC_MAIN : MFX_PROFILE_AVC_BASELINE;
	sts = m_pmfxEncoder->Query(&inPar, &m_mfxParams);
	if (sts >= MFX_ERR_NONE) {
		sts = m_pmfxEncoder->Reset(&m_mfxParams);
		sts = m_pmfxEncoder->GetVideoParam(&m_mfxParams);
		if (sts >= MFX_ERR_NONE) {
			m_bitrate = m_bitrate_prev = inpar->bitrate;
			return true;
		}
	}
	return false;
}
#endif
