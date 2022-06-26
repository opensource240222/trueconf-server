#ifdef _WIN32
#include "GetTypeHardwareCodec.h"
#include "MfxUtils.h"
#include <cstring>
#include <algorithm>
#include "VS_NvidiaVideoCodec.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "VSClient/VS_ApplicationInfo.h"
#include "Transcoder/VS_RetriveVideoCodec.h"

mfxStatus TestEncodeIntelH264(MFXVideoSession *mfxSession, mfxVideoParam par1, mfxVideoParam par2)
{
	mfxStatus sts = MFX_ERR_UNKNOWN;

	MFXVideoENCODE *pmfxEncoder = new MFXVideoENCODE(*mfxSession);
	sts = MFX_ERR_NOT_INITIALIZED;
	if (pmfxEncoder) {
		sts = pmfxEncoder->Close();
		if (sts == MFX_ERR_NOT_INITIALIZED) sts = MFX_ERR_NONE;
		if (sts == MFX_ERR_NONE) {
			sts = pmfxEncoder->Query(&par1, &par2);
			if (sts >= MFX_ERR_NONE) {
				mfxFrameAllocRequest request;
				mfxU16 nEncSurfNum = 0; // number of surfaces for encoder
				memset(&request, 0, sizeof(request));
				request.Type = MFX_MEMTYPE_SYSTEM_MEMORY | MFX_MEMTYPE_EXTERNAL_FRAME | MFX_MEMTYPE_FROM_ENCODE;
				sts = pmfxEncoder->QueryIOSurf(&par2, &request);
				if (sts == MFX_ERR_NONE) {
					unsigned short numFramesQueue = request.NumFrameSuggested + 1;
					numFramesQueue = std::max<decltype(numFramesQueue)>(1, numFramesQueue);
					mfxFrameSurface1 *pSurfaces = new mfxFrameSurface1[numFramesQueue]; ///
					for (int i = 0; i < numFramesQueue; i++) {
						memset(&(pSurfaces[i]), 0, sizeof(mfxFrameSurface1));
						memcpy(&(pSurfaces[i].Info), &(par2.mfx.FrameInfo), sizeof(mfxFrameInfo));
						int w = par2.mfx.FrameInfo.Width;
						int h = par2.mfx.FrameInfo.Width;
						pSurfaces[i].Data.Y = new unsigned char[w * h * 3 / 2];
						pSurfaces[i].Data.U = pSurfaces[i].Data.Y + w * h;
						pSurfaces[i].Data.V = pSurfaces[i].Data.U + 1;
						pSurfaces[i].Data.Pitch = w;
					}
					sts = pmfxEncoder->Init(&par2);
					sts = pmfxEncoder->GetVideoParam(&par1);
					if (sts >= MFX_ERR_NONE) {
						mfxBitstream mfxBS;
						memset(&mfxBS, 0, sizeof(mfxBS));
						mfxBS.Data = new unsigned char[0x800000];
						mfxBS.MaxLength = 0x800000;
						for (int idx = 0; idx < numFramesQueue; idx++) {
							sts = MFX_ERR_UNKNOWN;
							unsigned short nEncSurfIdx = GetFreeSurface(pSurfaces, numFramesQueue);
							if (nEncSurfIdx != 0xffff) {
								mfxFrameSurface1 *pSurf = &pSurfaces[nEncSurfIdx];
								mfxSyncPoint EncSyncP;
								mfxEncodeCtrl ctrlEnc;
								memset(&ctrlEnc, 0, sizeof(ctrlEnc));
								ctrlEnc.FrameType = MFX_FRAMETYPE_UNKNOWN;
								sts = pmfxEncoder->EncodeFrameAsync(&ctrlEnc, pSurf, &mfxBS, &EncSyncP);
								if (sts == MFX_ERR_MORE_DATA) continue;
								if (MFX_ERR_NONE <= sts && EncSyncP) {
									sts = mfxSession->SyncOperation(EncSyncP, INFINITE);
									break;
								}
							}
						}
						delete[] mfxBS.Data;
					}
					if (sts > MFX_ERR_NONE) sts = MFX_ERR_NONE;
					for (int i = 0; i < numFramesQueue; i++) delete[] pSurfaces[i].Data.Y;
					delete[] pSurfaces;
					pSurfaces = 0;
				}
			}
		}
		delete pmfxEncoder;
	}

	return sts;
}

void FillMFXTestParams(mfxVideoParam *pParam)
{
	pParam->mfx.CodecId = MFX_CODEC_AVC;
	pParam->mfx.NumThread = 0;
	pParam->IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
	///
	pParam->mfx.TargetUsage = MFX_TARGETUSAGE_BEST_QUALITY;
	pParam->mfx.EncodedOrder = 0;
	pParam->mfx.NumThread = 1;
	pParam->mfx.CodecProfile = MFX_PROFILE_AVC_BASELINE;
	pParam->mfx.CodecLevel = 31;
	/// bitrate
	pParam->mfx.RateControlMethod = MFX_RATECONTROL_VBR;
	pParam->mfx.TargetKbps = (unsigned short)1024;
	pParam->mfx.MaxKbps = (unsigned short)(3 * 1024 / 2);
	/// other
	pParam->mfx.GopPicSize = 1000;
	pParam->mfx.IdrInterval = 0;
	pParam->mfx.GopRefDist = 1; // only I, P
	pParam->mfx.NumSlice = 45;
	pParam->mfx.NumRefFrame = 1;
	/// type frame
	pParam->mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
	pParam->mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
	pParam->mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
	pParam->mfx.FrameInfo.Width = 1280;
	pParam->mfx.FrameInfo.Height = 720;
	pParam->mfx.FrameInfo.CropX = 0;
	pParam->mfx.FrameInfo.CropY = 0;
	pParam->mfx.FrameInfo.CropW = 1280;
	pParam->mfx.FrameInfo.CropH = 720;
	pParam->mfx.FrameInfo.FrameRateExtN = 30;
	pParam->mfx.FrameInfo.FrameRateExtD = 1;
	/// ext settings
	pParam->NumExtParam = 1;
}

bool TestH264IntelHardware(mfxU32 &maxSliceSize)
{
	bool ret = false;

	maxSliceSize = 0;
	MFXVideoSession mfxSession;
	mfxVersion version;
	version.Major = 1;
	version.Minor = 9;
	mfxIMPL impl = MFX_IMPL_HARDWARE_ANY;
	mfxStatus sts = mfxSession.Init(impl, &version);
	if (sts == MFX_ERR_NONE) {
		sts = mfxSession.QueryVersion(&version);
		if (sts == MFX_ERR_NONE) {
			mfxVideoParam par1, par2;
			memset(&par1, 0, sizeof(par1));
			FillMFXTestParams(&par1);
			std::vector<mfxExtBuffer*> CodingExtBuf;
			mfxExtCodingOption  codingExtOption;
			mfxExtCodingOption2  codingExtOption2;
			memset(&codingExtOption, 0, sizeof(codingExtOption));
			codingExtOption.Header.BufferSz = sizeof(codingExtOption);
			codingExtOption.Header.BufferId = MFX_EXTBUFF_CODING_OPTION;
			codingExtOption.NalHrdConformance = MFX_CODINGOPTION_OFF;
			codingExtOption.MaxDecFrameBuffering = 1;
			codingExtOption.RateDistortionOpt = MFX_CODINGOPTION_ON;
			CodingExtBuf.push_back((mfxExtBuffer*)&codingExtOption);
			memset(&codingExtOption2, 0, sizeof(codingExtOption2));
			codingExtOption2.Header.BufferSz = sizeof(codingExtOption2);
			codingExtOption2.Header.BufferId = MFX_EXTBUFF_CODING_OPTION2;
			codingExtOption2.MaxSliceSize = 1400;
			CodingExtBuf.push_back((mfxExtBuffer*)&codingExtOption2);
			if (!CodingExtBuf.empty()) {
				par1.ExtParam = &CodingExtBuf[0];
				par1.NumExtParam = CodingExtBuf.size();
			}
			memcpy(&par2, &par1, sizeof(par1));
			sts = TestEncodeIntelH264(&mfxSession, par1, par2);
			if (sts == MFX_ERR_NONE) {
				maxSliceSize = codingExtOption2.MaxSliceSize;
				ret = true;
			}
		}
		mfxSession.Close();
	}

	return ret;
}

bool TestH264NVidiaHardware()
{
	bool ret(false);
	VS_NvidiaVideoEncoder *nvenc = new VS_NvidiaVideoEncoder(VS_VCODEC_H264, true);
	if (nvenc->Init(3840, 2160, 0, 99, 1, 30) == 0) {
		ret = true;
	}
	delete nvenc;
	return ret;
}

eHardwareEncoder GetTypeHardwareCodec()
{
	static eHardwareEncoder typeCodec = ENCODER_SOFTWARE;
	static bool bDetect = false;
	if (!bDetect) {
		VS_RegistryKey key(true, REG_CurrentConfiguratuon);
		mfxU32 maxSliceSize(0);
		int use_hw_codec = -1;
		key.GetValue(&use_hw_codec, 4, VS_REG_INTEGER_VT, "HWCodec");
		bool bIntelH264 = (use_hw_codec == -1 || use_hw_codec == 2) ? TestH264IntelHardware(maxSliceSize) : false;
		bool bNvidiaH264 = (/*use_hw_codec == -1 || */use_hw_codec == 3) ? TestH264NVidiaHardware() : false;
		if (bIntelH264) typeCodec = ENCODER_H264_INTEL;
		else if (bNvidiaH264) typeCodec = ENCODER_H264_NVIDIA;
		bDetect = true;
		if (use_hw_codec >= 0) {
			if (use_hw_codec == 1 && ((typeCodec == ENCODER_H264_LOGITECH) || (typeCodec == ENCODER_H264_INTEL))) typeCodec = ENCODER_H264_LOGITECH;
			else if (use_hw_codec == 2 && typeCodec == ENCODER_H264_INTEL) typeCodec = ENCODER_H264_INTEL;
			else if (use_hw_codec == 3 && typeCodec >= ENCODER_H264_NVIDIA) typeCodec = ENCODER_H264_NVIDIA;
			else typeCodec = ENCODER_SOFTWARE;
		}
		else {
			/// temporary disable logitech h.264
			if (typeCodec == ENCODER_H264_LOGITECH) typeCodec = ENCODER_SOFTWARE;
			/// temporary disable nvidia h.264
			if (typeCodec == ENCODER_H264_NVIDIA) typeCodec = ENCODER_SOFTWARE;
		}
		if (typeCodec == ENCODER_H264_INTEL) {
			typeCodec = (maxSliceSize > 0 && maxSliceSize <= 1400) ? ENCODER_H264_INTEL_MSS : ENCODER_H264_INTEL;
		}
	}
	return typeCodec;
}
#endif
