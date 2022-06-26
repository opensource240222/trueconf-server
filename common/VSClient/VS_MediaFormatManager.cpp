
#include "VS_MediaFormatManager.h"
#include "VSTrClientProc.h"
#include "VSVideoCaptureList.h"
#include "VS_Dmodule.h"
#include "Transcoder/GetTypeHardwareCodec.h"

VS_MediaFormatManager::VS_MediaFormatManager()
{
	m_cVideoDeviceName[0] = 0;

	m_bUseHQAuto = true;
	m_bUseHDInputSource = false;
	m_bUseStereo = false;
	m_bFixedRcvFormat = false;
	m_bHQAutoCurrent = false;
	m_bRcvDynamicFmt = true;
	m_bCanRcvHWEncoding = false;
	m_bCanMul8 = true;
	m_bScreenCapturer = false;
	m_iSetBitrate = 256;
	m_uRcvLevel = 0;
	m_iRcvScreenWidth = 0;
	m_iRcvScreenHeight = 0;
	m_iConfType = 0;
	m_confFmt.SetZero();

	m_pVCaptureList = 0;
	m_pSysBench = 0;
	m_pProtocol = 0;
	m_pBandCtrl = 0;

	m_eTypeVideoEncoder = GetTypeHardwareCodec();
	m_bUseHWEncoding = m_eTypeVideoEncoder != ENCODER_SOFTWARE;

	m_pAutoLevelCaps = new tc_AutoLevelCaps();
	m_uLevelBitrate = m_pAutoLevelCaps->GetStartBitrateLevel();
	m_iLastBitrate = 128;
}

VS_MediaFormatManager::~VS_MediaFormatManager()
{
	delete m_pAutoLevelCaps;
}

void VS_MediaFormatManager::SetControlExternal(eControlExt ctrl, int iVal)
{
	VS_AutoLock lock(this);

	switch (ctrl)
	{
	case CTRL_EXT_HQAUTO:
		m_bUseHQAuto = (iVal > 0);
		break;
	case CTRL_EXT_HDINPUTSOURCE:
		m_bUseHDInputSource = (iVal > 0);
		break;
	case CTRL_EXT_STEREO:
		m_bUseStereo = (iVal > 0);
		break;
	case CTRL_EXT_BITRATE:
		m_iSetBitrate = iVal;
		break;
	default : return;
	}
}

void VS_MediaFormatManager::SetControlExternal(eControlExt ctrl, void *pVal)
{
	VS_AutoLock lock(this);

	switch (ctrl)
	{
	case CTRL_EXT_VIDEOCAPTURELIST:
		m_pVCaptureList = reinterpret_cast <CVideoCaptureList*> (pVal);
		break;
	case CTRL_EXT_SYSBENCH:
		m_pSysBench = reinterpret_cast <VS_SysBenchmarkBase*> (pVal);
		break;
	case CTRL_EXT_VIDEODEVICENAME:
		{
			wchar_t *pDevName = reinterpret_cast <wchar_t*> (pVal);
			if (pDevName) wcscpy(m_cVideoDeviceName, pDevName);
			m_bScreenCapturer = (wcsstr(m_cVideoDeviceName, VS_CaptureDeviceScreen::_nameScreenCapture) != 0);
			break;
		}
	case CTRL_EXT_PROTOCOL:
		m_pProtocol = reinterpret_cast <CVSTrClientProc*> (pVal);
		break;
	case CTRL_EXT_LEVELCAPS:
		m_pLevelCaps = reinterpret_cast <tc_VideoLevelCaps*> (pVal);
		break;
	case CTRL_EXT_CTRLBANDWIDTH:
		m_pBandCtrl = reinterpret_cast <VS_ControlBandBase*> (pVal);
		break;
	default : return;
	}
}

VS_MediaFormat VS_MediaFormatManager::SetConnection()
{
	VS_AutoLock lock(this);

	if (m_pProtocol) {
		char MyName[MAX_PATH];
		VS_MediaFormat mf;
		VS_ClientCaps caps;
		m_pProtocol->GetMyName(MyName);
		m_iConfType = m_pProtocol->GetMediaFormat(MyName, m_confFmt, &caps);
		m_bFixedRcvFormat = true;
		m_bCanRcvHWEncoding = false;
		m_bCanMul8 = false;
 		if (m_iConfType == 0) {
			VS_ClientCaps *pCapsRcv = &(m_pProtocol->m_Status.CurrConfInfo->ClientCaps);
			m_bAllowHD = !!(m_pProtocol->m_Status.MyInfo.Rights & 0x00100000);
			m_bRcvDynamicFmt = !!(pCapsRcv->GetVideoRcv() & VSCC_VIDEO_DYNCHANGE);
			int rating = pCapsRcv->GetRating();
			int level = pCapsRcv->GetLevel() & 0x000000ff;
			m_uRcvLevel = m_pLevelCaps->MergeRatingVsLevel(rating, (unsigned char)level);
			m_iRcvScreenWidth = pCapsRcv->GetScreenWidth();
			m_iRcvScreenHeight = pCapsRcv->GetScreenHeight();
			m_clientType = (VS_ClientType)pCapsRcv->GetClientType();
			m_bFixedRcvFormat = caps.IsFixedRcvMediaFormat(*pCapsRcv);
			if (pCapsRcv->FindVideoCodec(VS_VCODEC_H264)) m_bCanRcvHWEncoding = true;
			m_bCanMul8 = (!!(pCapsRcv->GetStreamsDC() & VSCC_STREAM_CAN_USE_SVC)) ||
						 (!!(pCapsRcv->GetVideoRcv() & VSCC_VIDEO_MULTIPLICITY8));
		}
		m_bConnect = true;
	}
	return m_confFmt;
}

void VS_MediaFormatManager::ResetConnection()
{
	VS_AutoLock lock(this);

	m_iSetBitrate = 256;
	m_iLastBitrate = 128;
	m_uLevelBitrate = m_pAutoLevelCaps->GetStartBitrateLevel();
	m_bRcvDynamicFmt = true;
	m_bFixedRcvFormat = false;
	m_bCanRcvHWEncoding = false;
	m_bAllowHD = true;
	m_bCanMul8 = true;
	m_uRcvLevel = 0;
	m_iConfType = 0;
	m_iRcvScreenWidth = 0;
	m_iRcvScreenHeight = 0;
	m_confFmt.SetZero();
	m_bConnect = false;
	m_pBandCtrl = 0;
	m_clientType = CT_SIMPLE_CLIENT;
}

unsigned char VS_MediaFormatManager::CheckBitrate(int bitrate)
{
	int db = bitrate - m_iLastBitrate;
	if (db == 0) return m_uLevelBitrate;
	m_uLevelBitrate = m_pAutoLevelCaps->CheckBitrateLevel(bitrate, (db > 0), m_uLevelBitrate);
	m_iLastBitrate = bitrate;
	return m_uLevelBitrate;
}

void VS_MediaFormatManager::RestrictRcvScreenResolution(VS_MediaFormat *outFmt, int rcv_width, int rcv_height, tc_levelVideo_t descLvl)
{
	if (rcv_width != 0 && rcv_height != 0) {
		rcv_width = rcv_width &~ 0x7;
		rcv_height = rcv_height &~0x7;
		double xk = (double)outFmt->dwVideoWidht / (double)rcv_width,
			   yk = (double)outFmt->dwVideoHeight / (double)rcv_height;
		if (xk > 1.0 || yk > 1.0) {
			if (xk >= yk) {
				outFmt->dwVideoWidht = rcv_width;
				outFmt->dwVideoHeight = ((int)((double)outFmt->dwVideoHeight / xk + 7)) &~ 0x7;
			} else if (yk >= xk) {
				outFmt->dwVideoWidht = ((int)((double)outFmt->dwVideoWidht / yk + 7)) &~ 0x7;
				outFmt->dwVideoHeight = rcv_height;
			}
			int frameMB = outFmt->dwVideoWidht * outFmt->dwVideoHeight / 256;
			tc_levelVideo_t descHwLvl;
			m_pLevelCaps->GetLevelDesc(VIDEO_HWLEVEL_MIN, &descHwLvl);
			if (frameMB < descHwLvl.maxFrameSizeMB) {
				outFmt->dwHWCodec = ENCODER_SOFTWARE;
				outFmt->dwVideoCodecFCC = VS_VCODEC_VPX;
			}
			int framerate = descLvl.maxMBps / frameMB;
			outFmt->dwFps = std::min(framerate, VIDEO_FRAMERATE_MAX_LIMIT);
		}
	}
}

bool VS_MediaFormatManager::GetMediaFormat(VS_MediaFormat *inFmt, VS_MediaFormat *outFmt)
{
	VS_AutoLock lock(this);

	*outFmt = *inFmt;
	/// if set stereo format -> off HQ Auto & HW encoding
	if (m_bUseStereo) {
		m_bUseHQAuto = false;
		m_bUseHWEncoding = false;
	}
	/// set currrent state HQ auto
	m_bHQAutoCurrent = m_bUseHQAuto && (m_iConfType == 0) && !m_bFixedRcvFormat && (m_pSysBench != 0) && m_bRcvDynamicFmt;
	/// reset use HW encoding
	bool bSetDShowSoftware = m_bFixedRcvFormat || !m_bCanRcvHWEncoding || (m_eTypeVideoEncoder > ENCODER_H264_LOGITECH);
	m_pVCaptureList->SetUseHardwareEncoder((bSetDShowSoftware) ? false : m_bUseHWEncoding);
	/// check support hw encoding
	eHardwareEncoder typeHW = m_eTypeVideoEncoder;
	if (m_eTypeVideoEncoder < ENCODER_H264_INTEL) {
		bool bSupportHW = m_pVCaptureList->IsHWEncoderSupport(m_cVideoDeviceName);
		typeHW = (bSupportHW) ? ENCODER_H264_LOGITECH : ENCODER_SOFTWARE;
	} else if (!m_bCanRcvHWEncoding || m_bScreenCapturer) {
		typeHW = ENCODER_SOFTWARE;
	} else if (m_clientType == CT_TRANSCODER || m_clientType == CT_TRANSCODER_CLIENT) {
		if (typeHW != ENCODER_H264_INTEL_MSS) {
			typeHW = ENCODER_SOFTWARE;
		}
	}
	/// if use HQ Auto
	if (m_bHQAutoCurrent) {
		unsigned char benchLevel = 0, benchLevel_sw = 0, bitrateLevel = VS_VIDEOLEVEL_98, optimalLevel = 0;
		tc_AutoModeDesc_t modeDesc;
		tc_levelVideo_t descLvl;
		/// check video device name
		bool bVideoConnected = m_cVideoDeviceName[0] != 0;
		if (m_bConnect) { /// active conference
			/// check bench mode
			benchLevel_sw = m_pSysBench->GetSndLevel(ENCODER_SOFTWARE);
			benchLevel = m_pSysBench->GetSndLevel(typeHW);
			/// check switch bitrate
			if (!m_bScreenCapturer) bitrateLevel = CheckBitrate(m_iSetBitrate);
			/// optimal mode
			optimalLevel = std::min(std::min(bitrateLevel, m_uRcvLevel), benchLevel);
			if (!m_bAllowHD) optimalLevel = std::min<decltype(optimalLevel)>(optimalLevel, VIDEO_LEVEL_ED);
			if (!m_bScreenCapturer && benchLevel_sw >= (optimalLevel + 3)) typeHW = ENCODER_SOFTWARE;
		} else { /// without conference
			/// check bench mode
			optimalLevel = m_pSysBench->GetSndLevel(ENCODER_SOFTWARE);
			if (optimalLevel < VIDEO_HWLEVEL_MIN) m_pVCaptureList->SetUseHardwareEncoder(false);
		}
		/// check level
		optimalLevel = m_pLevelCaps->CheckLevel(optimalLevel);
		if (!m_bScreenCapturer) {
			optimalLevel = m_pAutoLevelCaps->CheckLevel(optimalLevel);
			/// set optimal mode
			m_pAutoLevelCaps->GetLevelDesc(optimalLevel, typeHW, &modeDesc);
			m_pVCaptureList->SetAutoLevel(optimalLevel, modeDesc.ar);
		} else {
			/// for screen capturer
			modeDesc.ar = VS_VIDEOAR_16_9;
			modeDesc.fps = VIDEO_FRAMERATE_MAX_LIMIT;
			modeDesc.fourcc = VS_VCODEC_VPX;
			m_pVCaptureList->SetAutoLevel(VS_VIDEOLEVEL_98, modeDesc.ar);
		}
		m_pLevelCaps->GetLevelDesc(optimalLevel, &descLvl);
		if (bVideoConnected) {
			/// state for level
			tc_LevelModeState lvlState = m_pVCaptureList->GetLevelState(m_cVideoDeviceName);
			if (typeHW >= ENCODER_H264_INTEL && modeDesc.fourcc == VS_VCODEC_H264) {
				lvlState.nTypeHWEncoder = typeHW;
			}
			if (m_bScreenCapturer) {
				modeDesc.fps = std::max<decltype(modeDesc.fps)>(VIDEO_SCREEN_FRAMERATE_MIN_LIMIT, std::min<decltype(modeDesc.fps)>(descLvl.maxMBps / lvlState.nMBFrame, modeDesc.fps));
			}
			/// set out media format
			outFmt->SetVideo(lvlState.nWidth, lvlState.nHeight, modeDesc.fourcc, modeDesc.fps, inFmt->dwStereo, inFmt->dwSVCMode, lvlState.nTypeHWEncoder);
		} else {
			outFmt->SetVideo(modeDesc.defWidth, modeDesc.defHeight, modeDesc.fourcc, modeDesc.fps, inFmt->dwStereo, inFmt->dwSVCMode);
		}

		RestrictRcvScreenResolution(outFmt, m_iRcvScreenWidth, m_iRcvScreenHeight, descLvl);

		DTRACE(VSTM_THCL, "Get Media Format (auto, connect = %d) : btr = %d, ol = %d (bl = %d, rl = %d, btrl = %d), ar = %d",
							(int)(m_bConnect), m_iSetBitrate, optimalLevel, benchLevel, m_uRcvLevel, bitrateLevel, modeDesc.ar);

	} else {
		/// disable HQ Auto
		m_pVCaptureList->SetAutoLevel(0, VS_VIDEOAR_4_3);
		/// check stereo mode
		int typeStereo = m_pVCaptureList->GetStereoMode(m_cVideoDeviceName);
		if (m_bConnect) { /// active conference
			/// disable video if change stereo mode
			if (inFmt->dwStereo > 0 && typeStereo == 0) outFmt->SetVideo(0, 0);
			if (m_bRcvDynamicFmt) {
				if (inFmt->dwVideoCodecFCC == VS_VCODEC_H264) outFmt->dwHWCodec = typeHW;
				if (m_iConfType == 0) {
					tc_levelVideo_t descLvl;
					int rcvFrameSizeMB = inFmt->dwVideoWidht * inFmt->dwVideoHeight / 256;
					int sndFrameSizeMB = rcvFrameSizeMB;
					int rcvMBps = rcvFrameSizeMB * inFmt->dwFps;
					int sndMBps = rcvMBps;
					unsigned char benchLevel = (!m_bUseHQAuto) ? VIDEO_LEVEL_4K_MAX : m_pSysBench->GetSndLevel(typeHW);
					m_pLevelCaps->GetLevelDesc(benchLevel, &descLvl);
					if (descLvl.maxMBps < rcvMBps) sndMBps = descLvl.maxMBps;
					if (descLvl.maxFrameSizeMB < rcvFrameSizeMB) sndFrameSizeMB = descLvl.maxFrameSizeMB;
					if (sndFrameSizeMB != rcvFrameSizeMB || sndMBps != rcvMBps) {
						double w = inFmt->dwVideoWidht;
						double h = inFmt->dwVideoHeight;
						if (sndFrameSizeMB != rcvFrameSizeMB) {
							double rcvAR = (double)inFmt->dwVideoWidht / (double) inFmt->dwVideoHeight;
							w = sqrt(256.0 * sndFrameSizeMB * rcvAR);
							h = w / rcvAR;
						}
						outFmt->dwFps = std::max((uint32_t)VIDEO_SCREEN_FRAMERATE_MIN_LIMIT, std::min(inFmt->dwFps, (uint32_t)((double)sndMBps / (double)sndFrameSizeMB)));
						outFmt->dwVideoWidht = ((int)w) &~ 0x7;
						outFmt->dwVideoHeight = ((int)h) &~ 0x7;
					}
				} else {
					if (m_cVideoDeviceName[0] != 0 && m_bUseHQAuto) {
						unsigned char lvl = m_pSysBench->GetSndLevel(typeHW);
						if (m_bScreenCapturer) {
							lvl = VIDEO_LEVEL_GCONF_DS;
						} else if (lvl > VIDEO_LEVEL_GCONF) {
							lvl = VIDEO_LEVEL_GCONF;
						}
						m_pVCaptureList->SetAutoLevel(lvl, VIDEO_PERFECT_AR);
						tc_LevelModeState lvlState = m_pVCaptureList->GetLevelState(m_cVideoDeviceName);
						outFmt->dwVideoWidht = lvlState.nWidth;
						outFmt->dwVideoHeight = lvlState.nHeight;
						outFmt->dwFps = std::min(inFmt->dwFps, (uint32_t)((double)lvlState.nMBps / (double)lvlState.nMBFrame));
					} else if (m_bUseStereo) {
						outFmt->dwStereo = typeStereo;
						if (outFmt->dwStereo > 0) outFmt->dwVideoCodecFCC = VS_VCODEC_VPXSTEREO;
					}
				}
			}
		} else { /// without conference
			/// check stereo mode
			if (m_bUseStereo) {
				outFmt->dwStereo = typeStereo;
				if (outFmt->dwStereo > 0) outFmt->dwVideoCodecFCC = VS_VCODEC_VPXSTEREO;
			}
			outFmt->dwHWCodec = typeHW;
		}
	}

	if (!m_bCanMul8) {
		if (typeHW == ENCODER_SOFTWARE || typeHW >= ENCODER_H264_INTEL) {
			if (m_iConfType == 0) {
				outFmt->dwVideoWidht = outFmt->dwVideoWidht &~ 0xf;
				outFmt->dwVideoHeight = outFmt->dwVideoHeight &~ 0xf;
			} else {
				outFmt->dwVideoWidht = outFmt->dwVideoWidht &~ 0x7;
				outFmt->dwVideoHeight = outFmt->dwVideoHeight &~ 0x7;
			}
		} else {
			if ((outFmt->dwVideoWidht != (outFmt->dwVideoWidht &~ 0xf)) || (outFmt->dwVideoHeight != (outFmt->dwVideoHeight &~ 0xf))) {
				outFmt->SetVideo(0, 0);
			}
		}
	}

	if (m_pBandCtrl) m_pBandCtrl->SetHardwareEncoder(outFmt->dwHWCodec == ENCODER_H264_LOGITECH);

	DTRACE(VSTM_THCL, "Get Media Format : w = %d, h = %d, fps = %d, fourcc = %u, stereo = %u, svc = %u, hwenc = %u",
						outFmt->dwVideoWidht, outFmt->dwVideoHeight, outFmt->dwFps, outFmt->dwVideoCodecFCC,
						outFmt->dwStereo, outFmt->dwSVCMode, outFmt->dwHWCodec);

	return m_bHQAutoCurrent;
}

