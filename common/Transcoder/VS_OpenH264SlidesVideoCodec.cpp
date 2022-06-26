#include "VS_OpenH264SlidesVideoCodec.h"

void VS_OpenH264SlidesVideoCodec::FillDefaultParam(SEncParamExt& param)
{
	param.iUsageType = EUsageType::SCREEN_CONTENT_REAL_TIME;
	param.bEnableDenoise = false;
	param.iSpatialLayerNum = 1;
	param.bEnableFrameSkip = false;
	param.iEntropyCodingModeFlag = 0;
	param.bEnableSceneChangeDetect = true;
	param.iMinQp = 25;
	param.iMaxQp = 45;
}

VS_OpenH264SlidesVideoCodec::VS_OpenH264SlidesVideoCodec(int CodecId, bool IsCoder)
	: VS_OpenH264VideoCodec(CodecId, IsCoder)
{
}
