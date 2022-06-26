#include "std-generic/compat/memory.h"
#include "VS_VideoCodecState.h"
#include "Transcoder/VS_RetriveVideoCodec.h"

VS_VideoCodecState::VS_VideoCodecState(const VS_MediaFormat & mf)
{
	cdc.reset(VS_RetriveVideoCodec(mf, true));
	encodeFrame.push_back(vs::make_unique_default_init<uint8_t[]>(mf.dwVideoWidht * mf.dwVideoHeight * 3 / 2));
	encodeSize.push_back(0);
	state = STATE_WAIT_IDLE;
	format = mf;
}

VS_VideoCodecState::~VS_VideoCodecState()
{
	if (thread.joinable()) {
		shutdown = true;
		eventStartEncode.set();
		thread.join();
	}
}

void VS_VideoCodecState::PushEncode(uint8_t * frame, bool key)
{
	srcFrame = frame;
	keyFrame = key;
	if (key) {
		if (state == STATE_WAIT_IDLE) {
			state = STATE_IDLE;
		}
		if (state == STATE_WAIT_SKIP) {
			state = STATE_SKIP;
		}
	}
	eventStartEncode.set();
}

void VS_VideoCodecState::EncodeRoutine(uint8_t * src, int32_t idx)
{
	VS_VideoCodecParam prm;
	prm.cmp.Quality = 0;
	prm.cmp.KeyFrame = keyFrame ? 1 : 0;
	encodeSize[idx] = cdc->Convert(src, encodeFrame[idx].get(), &prm);
	keyFrame = (prm.cmp.IsKeyFrame > 0);
	svc = prm.cmp.Quality;
}
