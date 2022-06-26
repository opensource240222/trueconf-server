#pragma once

#include "FrameFilterLib/Base/AbstractOrderedFilter.h"
#include "FrameFilterLib/Base/AbstractMultiSourceSink.h"

namespace ffl {
	class FilterAudioVideoJoiner : public AbstractOrderedFilter<AbstractMultiSourceSink>
	{
	public:
		static std::shared_ptr<FilterAudioVideoJoiner> Create();

		FilterAudioVideoJoiner();

		void SetAudio(const std::shared_ptr<AbstractSource>& src);
		void SetVideo(const std::shared_ptr<AbstractSource>& src);

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;
		bool ProcessCommand(const FilterCommand& cmd) override;

	private:
		std::shared_ptr<AbstractSource> m_audio;
		std::shared_ptr<AbstractSource> m_video;
	};
}