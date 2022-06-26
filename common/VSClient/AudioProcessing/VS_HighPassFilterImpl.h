/*
*  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#ifndef WEBRTC_MODULES_AUDIO_PROCESSING_HIGH_PASS_FILTER_IMPL_H_
#define WEBRTC_MODULES_AUDIO_PROCESSING_HIGH_PASS_FILTER_IMPL_H_

#include <memory>
#include <vector>

#include "rtc_base/constructormagic.h"
#include "rtc_base/criticalsection.h"
#include "modules/audio_processing/include/audio_processing.h"

namespace webrtc
{
	class AudioBuffer;

	class VS_HighPassFilterImpl : public HighPassFilter
	{
	public:
		explicit VS_HighPassFilterImpl(rtc::CriticalSection* crit);
		~VS_HighPassFilterImpl() override;

		// TODO(peah): Fold into ctor, once public API is removed.
		void Initialize(size_t channels, int sample_rate_hz);
		void ProcessCaptureAudio(AudioBuffer* audio);

		// HighPassFilter implementation.
		int Enable(bool enable) override;
		bool is_enabled() const override;

	private:
		class BiquadFilter;
		rtc::CriticalSection* const crit_ = nullptr;
		bool enabled_ RTC_GUARDED_BY(crit_) = false;
		std::vector<std::unique_ptr<BiquadFilter>> filters_ RTC_GUARDED_BY(crit_);
		RTC_DISALLOW_IMPLICIT_CONSTRUCTORS(VS_HighPassFilterImpl);
	};
}  // namespace webrtc

#endif  // WEBRTC_MODULES_AUDIO_PROCESSING_HIGH_PASS_FILTER_IMPL_H_
