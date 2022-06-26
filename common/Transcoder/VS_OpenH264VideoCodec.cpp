#include "VS_OpenH264VideoCodec.h"
#include <thread>
#include <algorithm>

bool VS_OpenH264VideoCodec::UpdateBitrate()
{
	if (m_bitrate_prev == m_bitrate)
		return true;

	m_bitrate_prev = m_bitrate;

	SBitrateInfo targetBitrateInfo;
	SBitrateInfo targetMaxBitrateInfo;
	SBitrateInfo currBitrateInfo;
	int ret = 0;

	targetBitrateInfo.iBitrate = m_bitrate * 1000;
	targetMaxBitrateInfo.iBitrate = (m_bitrate * 6 / 5) * 1000; // +20% of target bitrate

	currBitrateInfo.iLayer = SPATIAL_LAYER_ALL;

	m_Encoder->GetOption(ENCODER_OPTION_BITRATE, &currBitrateInfo);

	if (targetMaxBitrateInfo.iBitrate < currBitrateInfo.iBitrate)
	{
		targetBitrateInfo.iLayer = SPATIAL_LAYER_ALL;
		ret = m_Encoder->SetOption(ENCODER_OPTION_BITRATE, &targetBitrateInfo);
		targetBitrateInfo.iLayer = SPATIAL_LAYER_0;
		ret = m_Encoder->SetOption(ENCODER_OPTION_BITRATE, &targetBitrateInfo);

		targetMaxBitrateInfo.iLayer = SPATIAL_LAYER_ALL;
		ret = m_Encoder->SetOption(ENCODER_OPTION_MAX_BITRATE, &targetMaxBitrateInfo);
		targetMaxBitrateInfo.iLayer = SPATIAL_LAYER_0;
		ret = m_Encoder->SetOption(ENCODER_OPTION_MAX_BITRATE, &targetMaxBitrateInfo);
	}
	else
	{
		targetMaxBitrateInfo.iLayer = SPATIAL_LAYER_ALL;
		ret = m_Encoder->SetOption(ENCODER_OPTION_MAX_BITRATE, &targetMaxBitrateInfo);
		targetMaxBitrateInfo.iLayer = SPATIAL_LAYER_0;
		ret = m_Encoder->SetOption(ENCODER_OPTION_MAX_BITRATE, &targetMaxBitrateInfo);

		targetBitrateInfo.iLayer = SPATIAL_LAYER_ALL;
		ret = m_Encoder->SetOption(ENCODER_OPTION_BITRATE, &targetBitrateInfo);
		targetBitrateInfo.iLayer = SPATIAL_LAYER_0;
		ret = m_Encoder->SetOption(ENCODER_OPTION_BITRATE, &targetBitrateInfo);
	}

	if (ret != 0)
		return false;

	return true;
}

void VS_OpenH264VideoCodec::FillDefaultParam(SEncParamExt& param)
{
	param.iUsageType = EUsageType::CAMERA_VIDEO_REAL_TIME;
	param.bEnableDenoise = false;
	param.iSpatialLayerNum = 1;
	param.bEnableFrameSkip = false;
	param.iEntropyCodingModeFlag = 0;
	param.bEnableSceneChangeDetect = false;
	param.iMinQp = 1;
	param.iMaxQp = 45;

	param.bEnableBackgroundDetection = false;
	param.bEnableAdaptiveQuant = false;
}

VS_OpenH264VideoCodec::VS_OpenH264VideoCodec(int CodecId, bool IsCoder)
	: VideoCodec(CodecId, IsCoder)
{
	m_Encoder = nullptr;
}

VS_OpenH264VideoCodec::~VS_OpenH264VideoCodec()
{
	Release();
}

int VS_OpenH264VideoCodec::Init(int w, int h, uint32_t ColorMode, unsigned char sndLvl, int numThreads, unsigned int framerate)
{
	Release();

	int result = WelsCreateSVCEncoder(&m_Encoder);

	if (result != 0 || !m_Encoder)
		return -1;

	memset(&m_EncodeParam, 0, sizeof(SEncParamExt));
	m_Encoder->GetDefaultParams(&m_EncodeParam);

	FillDefaultParam(m_EncodeParam);

	m_EncodeParam.fMaxFrameRate = framerate;
	m_EncodeParam.iPicWidth = w;
	m_EncodeParam.iPicHeight = h;
	m_EncodeParam.iTargetBitrate = m_bitrate * 1000;
	m_EncodeParam.iMaxBitrate = m_EncodeParam.iTargetBitrate * 1.2f;

	int num_treads = numThreads;

	if (num_treads == 1) {
		if (w >= 1280 && h >= 720) {
			num_treads = std::min(std::thread::hardware_concurrency(), 4u);
		}
	}
	m_EncodeParam.iMultipleThreadIdc = num_treads;
	m_num_threads = num_treads;

	m_EncodeParam.sSpatialLayers[0].iVideoWidth = m_EncodeParam.iPicWidth;
	m_EncodeParam.sSpatialLayers[0].iVideoHeight = m_EncodeParam.iPicHeight;
	m_EncodeParam.sSpatialLayers[0].fFrameRate = m_EncodeParam.fMaxFrameRate;
	m_EncodeParam.sSpatialLayers[0].iSpatialBitrate = m_EncodeParam.iTargetBitrate;
	m_EncodeParam.sSpatialLayers[0].iMaxSpatialBitrate = m_EncodeParam.iMaxBitrate;
	m_EncodeParam.sSpatialLayers[0].uiProfileIdc = PRO_BASELINE;
	m_EncodeParam.sSpatialLayers[0].uiLevelIdc = LEVEL_3_1;

	m_EncodeParam.sSpatialLayers[0].sSliceArgument.uiSliceMode = SliceModeEnum::SM_SIZELIMITED_SLICE;
	m_EncodeParam.sSpatialLayers[0].sSliceArgument.uiSliceSizeConstraint = 1400;
	m_EncodeParam.uiMaxNalSize = 1500;

	m_EncodeParam.iRCMode = RC_BITRATE_MODE;
	m_EncodeParam.iComplexityMode = LOW_COMPLEXITY;
	m_EncodeParam.iNumRefFrame = 1;

	result = m_Encoder->InitializeExt(&m_EncodeParam);

	if (result != 0)
		return -1;

	int videoFormat = videoFormatI420;
	int IDRInterval = 1000;

	result = m_Encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);

	if (result != 0)
		return -1;

	result = m_Encoder->SetOption(ENCODER_OPTION_IDR_INTERVAL, &IDRInterval);

	if (result != 0)
		return -1;

	m_valid = true;

	return 0;
}

void VS_OpenH264VideoCodec::Release()
{
	if (m_Encoder)
	{
		m_Encoder->Uninitialize();
		WelsDestroySVCEncoder(m_Encoder);
	}

	m_Encoder = nullptr;
	m_bitrate = 128;
	m_num_threads = 1;

	m_valid = false;
}

int	VS_OpenH264VideoCodec::Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param)
{
	int frameSize = m_EncodeParam.iPicWidth * m_EncodeParam.iPicHeight * 3 / 2;
	int ret = -1;

	if (!UpdateBitrate())
		return -1;

	SFrameBSInfo info;
	SSourcePicture pic;

	memset(&info, 0, sizeof(SFrameBSInfo));
	memset(&pic, 0, sizeof(SSourcePicture));

	pic.iPicWidth = m_EncodeParam.iPicWidth;
	pic.iPicHeight = m_EncodeParam.iPicHeight;
	pic.iColorFormat = videoFormatI420;
	pic.iStride[0] = pic.iPicWidth;
	pic.iStride[1] = pic.iStride[2] = pic.iPicWidth >> 1;
	pic.pData[0] = in;
	pic.pData[1] = pic.pData[0] + pic.iPicWidth * pic.iPicHeight;
	pic.pData[2] = pic.pData[1] + (pic.iPicWidth * pic.iPicHeight >> 2);

	if (param->cmp.KeyFrame)
		m_Encoder->ForceIntraFrame(true);

	ret = m_Encoder->EncodeFrame(&pic, &info);

	if (ret != cmResultSuccess)
		return -1;

	if (info.eFrameType == videoFrameTypeSkip)
		return 0;

	size_t shift = 0;

	for (int layer = 0; layer < info.iLayerNum; layer++)
	{
		size_t layerSize = 0;

		for (int nal = 0; nal < info.sLayerInfo[layer].iNalCount; nal++)
			layerSize += info.sLayerInfo[layer].pNalLengthInByte[nal];

		memcpy(out + shift, info.sLayerInfo[layer].pBsBuf, layerSize);

		shift += layerSize;
	}

	if (info.eFrameType == videoFrameTypeI || info.eFrameType == videoFrameTypeIDR)
		param->cmp.IsKeyFrame = true;
	else
		param->cmp.IsKeyFrame = false;

	return info.iFrameSizeInBytes;
}

bool VS_OpenH264VideoCodec::SetCoderOption(void *param)
{
	h264_Param* inParam = (h264_Param*)param;
	int complexity = -1;

	switch (inParam->me_quality)
	{
	case 0:
	{
		complexity = LOW_COMPLEXITY;
		break;
	}

	case 1:
	{
		complexity = MEDIUM_COMPLEXITY;
		break;
	}

	case 2:
	{
		complexity = HIGH_COMPLEXITY;
		break;
	}
	}

	if (complexity < 0)
		return false;

	if (m_Encoder->SetOption(ENCODER_OPTION_COMPLEXITY, &complexity) != 0)
		return false;

	return true;
}
