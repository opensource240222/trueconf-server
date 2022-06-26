#pragma once

#include "VS_OpenH264VideoCodec.h"

class VS_OpenH264SlidesVideoCodec : public VS_OpenH264VideoCodec
{
protected:
	void FillDefaultParam(SEncParamExt& param) override;

public:
	VS_OpenH264SlidesVideoCodec(int CodecId, bool IsCoder);
};
