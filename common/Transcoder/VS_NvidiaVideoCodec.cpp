#ifdef _WIN32
#include "VS_NvidiaVideoCodec.h"
#include "MediaParserLib/VS_H264Parser.h"
#include "nvidia/nvenc/NvEncoderCuda.h"
#include "nvidia/include/cuda_runtime.h"
#include "nvidia/include/nppi_color_conversion.h"

// TODO: get rid
#include "ipp.h"
#include "IppLib2/libinit.h"

#include "std-generic/compat/memory.h"

VS_NvidiaVideoCodec::VS_NvidiaVideoCodec(int CodecId, bool IsCoder) : VideoCodec(CodecId, IsCoder)
{
	m_externalContext = true;
	m_cuContext = nullptr;
	if (CheckCuda()) {
		if (cuInit(0) == CUDA_SUCCESS) {
			InitCuda(0);
		}
	}
}

VS_NvidiaVideoCodec::VS_NvidiaVideoCodec(int CodecId, bool IsCoder, int deviceId) : VideoCodec(CodecId, IsCoder)
{
	m_externalContext = true;
	m_cuContext = nullptr;
	if (CheckCuda()) {
		if (cuInit(0) == CUDA_SUCCESS) {
			InitCuda(deviceId);
		}
	}
}

VS_NvidiaVideoCodec::~VS_NvidiaVideoCodec()
{
	if (!m_externalContext && m_cuContext) {
		cuCtxDestroy(m_cuContext);
	}
	m_cuContext = nullptr;
}

bool VS_NvidiaVideoCodec::CheckCuda()
{
#ifdef _WIN32	// not ported
#ifdef UNICODE
	static LPCWSTR __cuvidLibName = L"nvcuvid.dll";
	static LPCWSTR __cudaLibName = L"nvcuda.dll";
#else
	static LPCSTR __cuvidLibName = "nvcuvid.dll";
	static LPCSTR __cudaLibName = "nvcuda.dll";
#endif

	auto hCuda = LoadLibrary(__cudaLibName);
	auto hCuvid = LoadLibrary(__cuvidLibName);
	if (!hCuda || !hCuvid) {
		return false;
	}
	auto dwSize = GetFileVersionInfoSize(__cudaLibName, NULL);
	if (dwSize == 0) {
		return false;
	}
	uint32_t drv1(0), drv2(0);
	auto cudaLibInfo = new uint8_t[dwSize];
	if (GetFileVersionInfo(__cudaLibName, 0, dwSize, cudaLibInfo)) {
		VS_FIXEDFILEINFO *fileInfo(NULL);
		UINT lenFileInfo(0);
		if (VerQueryValue(cudaLibInfo, TEXT("\\"), (LPVOID*)&fileInfo, &lenFileInfo)) {
			drv1 = HIWORD(fileInfo->dwFileVersionLS);
			drv2 = LOWORD(fileInfo->dwFileVersionLS);
		}
	}
	delete[] cudaLibInfo;
	return (drv1 > 13 || (drv1 == 13 && drv2 >= 9077));
#else
	return false;
#endif
}

bool VS_NvidiaVideoCodec::InitCuda(int32_t deviceID)
{
	CUdevice cuDevice = 0;
	int32_t deviceCount = 0;
	int32_t SMminor = 0, SMmajor = 0;
	if (deviceID < 0) {
		return false;
	}
	if (cuDeviceGetCount(&deviceCount) != CUDA_SUCCESS) {
		return false;
	}
	if (deviceID > deviceCount - 1) {
		return false;
	}
	if (cuDeviceGet(&cuDevice, deviceID) != CUDA_SUCCESS) {
		return false;
	}
	if (cuDeviceComputeCapability(&SMmajor, &SMminor, deviceID) != CUDA_SUCCESS) {
		return false;
	}
	if (((SMmajor << 4) + SMminor) < 0x30) {
		return false;
	}
	if (cuCtxGetCurrent(&m_cuContext) != CUDA_SUCCESS || m_cuContext == 0) {
		if (cuCtxCreate(&m_cuContext, 0, cuDevice) != CUDA_SUCCESS) {
			return false;
		}
		m_externalContext = false;
	}
	return true;
}

VS_NvidiaVideoEncoder::VS_NvidiaVideoEncoder(int CodecId) : VS_NvidiaVideoCodec(CodecId, true)
{

}

VS_NvidiaVideoEncoder::VS_NvidiaVideoEncoder(int CodecId, int deviceId) : VS_NvidiaVideoCodec(CodecId, true, deviceId)
{

}

VS_NvidiaVideoEncoder::~VS_NvidiaVideoEncoder()
{
	Release();
}

int VS_NvidiaVideoEncoder::Init(int w, int h, uint32_t ColorMode, unsigned char sndLvl, int numThreads, unsigned int framerate)
{
	Release();

	NV_ENC_INITIALIZE_PARAMS initializeParams = { NV_ENC_INITIALIZE_PARAMS_VER };
	NV_ENC_CONFIG encodeConfig = { NV_ENC_CONFIG_VER };

	if (m_cuContext) {
		try {
			m_pEncoder = new NvEncoderCuda(m_cuContext, w, h, NV_ENC_BUFFER_FORMAT::NV_ENC_BUFFER_FORMAT_IYUV, 0);
		}
		catch (NVENCException& e) {
		}
		if (m_pEncoder) {
			initializeParams.encodeConfig = &encodeConfig;
			GUID encGuid = (GetFcc() == VS_VCODEC_H264) ? NV_ENC_CODEC_H264_GUID : NV_ENC_CODEC_HEVC_GUID;
			GUID presetGuid = NV_ENC_PRESET_LOW_LATENCY_HQ_GUID;
			m_pEncoder->CreateDefaultEncoderParams(&initializeParams, encGuid, presetGuid);
			if (m_pEncoder->GetCapabilityValue(encGuid, NV_ENC_CAPS_SUPPORT_DYN_BITRATE_CHANGE) == 1) {
				initializeParams.encodeWidth = w;
				initializeParams.encodeHeight = h;
				initializeParams.darWidth = w;
				initializeParams.darHeight = h;
				initializeParams.maxEncodeWidth = w;
				initializeParams.maxEncodeHeight = h;
				initializeParams.frameRateNum = framerate;
				initializeParams.frameRateDen = 1;
				initializeParams.enableEncodeAsync = 1;
				initializeParams.enablePTD = 1;
				initializeParams.encodeConfig->gopLength = framerate * 20;
				if (GetFcc() == VS_VCODEC_H264) {
					auto cfg = &(initializeParams.encodeConfig->encodeCodecConfig.h264Config);
					initializeParams.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_BASELINE_GUID;
					cfg->repeatSPSPPS = 1;
					cfg->sliceMode = 1;
					cfg->sliceModeData = 1400;
					cfg->entropyCodingMode = NV_ENC_H264_ENTROPY_CODING_MODE_CAVLC;
				}
				else {
					auto cfg = &(initializeParams.encodeConfig->encodeCodecConfig.hevcConfig);
					initializeParams.encodeConfig->profileGUID = NV_ENC_HEVC_PROFILE_MAIN_GUID;
					cfg->repeatSPSPPS = 1;
					cfg->sliceMode = 1;
					cfg->sliceModeData = 1400;
				}
				auto btrCfg = &initializeParams.encodeConfig->rcParams;
				auto caps = m_pEncoder->GetCapabilityValue(encGuid, NV_ENC_CAPS_SUPPORTED_RATECONTROL_MODES);
				if (caps & NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ) {
					btrCfg->rateControlMode = NV_ENC_PARAMS_RC_CBR;// NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ;
				}
				else {
					btrCfg->rateControlMode = NV_ENC_PARAMS_RC_CBR;
				}
				if (initializeParams.enableEncodeAsync > 0) {
					initializeParams.enableEncodeAsync = static_cast<uint32_t>(m_pEncoder->GetCapabilityValue(encGuid, NV_ENC_CAPS_ASYNC_ENCODE_SUPPORT));
				}
				btrCfg->averageBitRate = 512 * 1000;
				btrCfg->maxBitRate = btrCfg->averageBitRate * 120 / 100;
				m_pEncoder->CreateEncoder(&initializeParams);
				m_bitrate_prev = m_bitrate = btrCfg->averageBitRate / 1000;
				m_valid = true;
			}
		}
	}

	if (!m_valid) {
		Release();
	}

	return (m_valid) ? 0 : -1;
}

void VS_NvidiaVideoEncoder::Release()
{
	if (m_pEncoder) {
		m_pEncoder->DestroyEncoder();
		delete m_pEncoder;
		m_pEncoder = nullptr;
	}
	m_requestKeyFrame = false;
	m_valid = false;
	m_bitrate_prev = m_bitrate = 512;
}

bool VS_NvidiaVideoEncoder::TryReconfigure(bool key)
{
	if (!key && (m_bitrate_prev == m_bitrate)) {
		return true;
	}

	NV_ENC_INITIALIZE_PARAMS initializeParams;
	NV_ENC_CONFIG initCodecConfig = { NV_ENC_CONFIG_VER };
	initializeParams.encodeConfig = &initCodecConfig;
	m_pEncoder->GetInitializeParams(&initializeParams);
	NV_ENC_RECONFIGURE_PARAMS reconfigureParams = { NV_ENC_RECONFIGURE_PARAMS_VER };
	memcpy(&reconfigureParams.reInitEncodeParams, &initializeParams, sizeof(initializeParams));
	NV_ENC_CONFIG reInitCodecConfig = { NV_ENC_CONFIG_VER };
	memcpy(&reInitCodecConfig, initializeParams.encodeConfig, sizeof(reInitCodecConfig));
	reconfigureParams.reInitEncodeParams.encodeConfig = &reInitCodecConfig;

	if (m_bitrate_prev != m_bitrate) {
		/// reconfigure bitrate
		{
			reInitCodecConfig.rcParams.averageBitRate = m_bitrate * 1000;
			reInitCodecConfig.rcParams.maxBitRate = m_bitrate * 1000 * 120 / 100;
		}
		m_bitrate_prev = m_bitrate;
	}
	if (key) {
		/// key frame request
		reconfigureParams.forceIDR = true;
		m_requestKeyFrame = false;
	}
	return m_pEncoder->Reconfigure(&reconfigureParams);
}

int VS_NvidiaVideoEncoder::Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param)
{
	int size(0);
	if (!IsValid()) {
		return size;
	}
	bool recfg = TryReconfigure((param->cmp.KeyFrame == 1) || m_requestKeyFrame);
	param->cmp.IsKeyFrame = 0;
	{
		/// copy to device
		const NvEncInputFrame* encoderInputFrame = m_pEncoder->GetNextInputFrame();
		NvEncoderCuda::CopyToDeviceFrame(m_cuContext, in, 0, (CUdeviceptr)encoderInputFrame->inputPtr,
			(int)encoderInputFrame->pitch,
			m_pEncoder->GetEncodeWidth(),
			m_pEncoder->GetEncodeHeight(),
			CU_MEMORYTYPE_HOST,
			encoderInputFrame->bufferFormat,
			encoderInputFrame->chromaOffsets,
			encoderInputFrame->numChromaPlanes);
	}
	{
		/// encode
		std::vector<std::vector<uint8_t>> vPacket;
		m_pEncoder->EncodeFrame(vPacket);
		for (const auto &it : vPacket) {
			m_encodeFrames.push_back(it);
		}
		if (!m_encodeFrames.empty()) {
			auto frame = m_encodeFrames.front();
			int key(1);
			TypeSliceFromBitstream_H264(frame.data(), frame.size(), key);
			if (key == 0) {
				param->cmp.IsKeyFrame = 1;
			}
			size = frame.size();
			memcpy(out, frame.data(), size);
			m_encodeFrames.pop_front();
			if (m_encodeFrames.size() > 10) {
				m_requestKeyFrame = true;
				m_encodeFrames.clear();
			}
		}
	}
	param->cmp.Quality = 0;
	return size;
}

bool VS_NvidiaVideoEncoder::SetSVCMode(uint32_t &param)
{
	if (IsValid() && IsCoder()) {
		if (param > 0) {
			param &= 0x00ff0000;
		}
		return true;
	}
	return false;
}

VS_NvidiaVideoDecoder::VS_NvidiaVideoDecoder(int CodecId) : VS_NvidiaVideoCodec(CodecId, false)
{

}

VS_NvidiaVideoDecoder::VS_NvidiaVideoDecoder(int CodecId, int deviceId) : VS_NvidiaVideoCodec(CodecId, false, deviceId)
{

}

VS_NvidiaVideoDecoder::~VS_NvidiaVideoDecoder()
{
	Release();
}

int VS_NvidiaVideoDecoder::Init(int w, int h, uint32_t ColorMode, unsigned char sndLvl, int numThreads, unsigned int framerate)
{
	return InitInternal(w, h, ColorMode, false, nullptr);
}

int VS_NvidiaVideoDecoder::InitExtended(const base_Param &settings)
{
	Dim resizeRect = { settings.out_width, settings.out_height };
	return InitInternal(settings.width, settings.height, settings.color_space, settings.device_memory, &resizeRect);
}

int VS_NvidiaVideoDecoder::InitInternal(int w, int h, uint32_t ColorMode, bool deviceMemory, Dim *resizeRect)
{
	Release();
	m_bDeviceMemory = deviceMemory;
	if (m_cuContext) {
		cudaVideoCodec type(cudaVideoCodec_YUV420);
		if (GetFcc() == VS_VCODEC_H264) {
			type = cudaVideoCodec_H264;
		}
		else if (GetFcc() == VS_VCODEC_H265) {
			type = cudaVideoCodec_HEVC;
		}
		else if (GetFcc() == VS_VCODEC_VPX) {
			type = cudaVideoCodec_VP8;
		}
		else if (GetFcc() == VS_VCODEC_VP9) {
			type = cudaVideoCodec_VP9;
		}
		if (type != cudaVideoCodec_YUV420) {
			CUVIDDECODECAPS decodeCaps = {};
			decodeCaps.eCodecType = type;
			decodeCaps.eChromaFormat = cudaVideoChromaFormat_420;
			decodeCaps.nBitDepthMinus8 = 0;
			cuCtxPushCurrent(m_cuContext);
			if (cuvidGetDecoderCaps(&decodeCaps) == CUDA_SUCCESS) {
				if (decodeCaps.bIsSupported == 1) {
					m_pDecoder = new NvDecoder(m_cuContext, w, h, m_bDeviceMemory, type, nullptr, true, false, nullptr, resizeRect);
					if (m_pDecoder) {
						cudaStreamCreate(&m_cuvidStream);
						m_valid = true;
					}
				}
			}
			cuCtxPopCurrent(nullptr);
		}
	}
	if (m_valid) {
		IppLibInit();
	}

	return (m_valid) ? 0 : -1;
}

void VS_NvidiaVideoDecoder::Release()
{
	delete m_pDecoder; m_pDecoder = nullptr;
	if (m_cuvidStream) {
		cuCtxPushCurrent(m_cuContext);
		cudaStreamDestroy(m_cuvidStream);
		cuCtxPopCurrent(nullptr);
		m_cuvidStream = 0;
	}
	m_numDecodeFrames = 0;
	m_valid = false;
	m_decodeFrames.clear();
	m_bDeviceMemory = false;
}

int VS_NvidiaVideoDecoder::Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param)
{
	int ret(-1);
	if (!IsValid()) {
		return ret;
	}
	int32_t sizeFrame(0);
	uint8_t *decodeFrame(nullptr);
	uint8_t **ppFrame;
	int64_t *pTimestamp;
	int nFrameReturned = 0;
	if (!m_decodeFrames.empty()) {
		decodeFrame = m_decodeFrames.front().buffer;
		sizeFrame = m_decodeFrames.front().size;
	}
	if (m_pDecoder->Decode(in, param->dec.FrameSize, &ppFrame, &nFrameReturned, CUVID_PKT_ENDOFPICTURE, &pTimestamp, m_numDecodeFrames++, m_cuvidStream)) {
		if (nFrameReturned > 0 && !decodeFrame) {
			sizeFrame = m_pDecoder->GetFrameSize();
			decodeFrame = ppFrame[0];
			ppFrame++;
			nFrameReturned--;
		}
	}
	if (decodeFrame) {
		/// convert nv12 -> i420
		int dst_step[3] = { m_pDecoder->GetWidth(), m_pDecoder->GetWidth() >> 1, m_pDecoder->GetWidth() >> 1 };
		uint8_t *pDst[3] = { out, out + m_pDecoder->GetWidth() *  m_pDecoder->GetHeight(), out + m_pDecoder->GetWidth() *  m_pDecoder->GetHeight() * 5 / 4 };
		uint8_t *pSrc[2] = { decodeFrame, decodeFrame + m_pDecoder->GetWidth() * m_pDecoder->GetHeight() };
		if (!m_bDeviceMemory) {
			IppiSize roi = { m_pDecoder->GetWidth() , m_pDecoder->GetHeight() };
			IppStatus st = ippiYCbCr420_8u_P2P3R(pSrc[0], m_pDecoder->GetWidth(), pSrc[1], m_pDecoder->GetWidth(), pDst, dst_step, roi);
			if (st == ippStsNoErr) {
				ret = sizeFrame;
			}
		}
		else {
			NppStatus st(NPP_NO_ERROR);
			NppiSize roi = { m_pDecoder->GetWidth() , m_pDecoder->GetHeight() };
			cuCtxPushCurrent(m_cuContext);
			st = nppiYCbCr420_8u_P2P3R(pSrc[0], m_pDecoder->GetWidth(), pSrc[1], m_pDecoder->GetWidth(), pDst, dst_step, roi);
			cuCtxPopCurrent(NULL);
			if (st == NPP_NO_ERROR) {
				ret = sizeFrame;
			}
		}
		if (!m_decodeFrames.empty()) {
			m_decodeFrames.pop_front();
		}
	}
	for (auto i = 0; i < nFrameReturned; i++) {
		m_decodeFrames.emplace_back(ppFrame[i], sizeFrame, m_bDeviceMemory ? m_cuContext : 0, m_cuvidStream);
	}
	return ret;
}
#endif
