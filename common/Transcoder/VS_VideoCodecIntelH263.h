#pragma once

#include "VS_VfwVideoCodec.h"

enum VS_IntelH263Option {
	IH263_NONE = 0x000,
	IH263_QUICK_CMP = 0x002,
	IH263_STRICT_COMPL = 0x004,
	IH263_UNRESTR_MV = 0x008,
	IH263_ADV_PRED = 0x010,
	IH263_PB_FRAMES = 0x020,
	IH263_DEF_OPT = 0x040,//?
	IH263_DEBL_FILTR = 0x080,
	IH263_PREFILTR = 0x200,
	IH263_SYNC_GOB = 0x400, //?
	IH263_ONLY_INTRA = 0x800 //?
};

class VS_VideoCoderIntelH263 : public VS_VfwVideoCodec
{
protected:
	bool UpdateBitrate();

public:
	VS_VideoCoderIntelH263();
	~VS_VideoCoderIntelH263();

	bool SetCoderOption(uint32_t param);
};
