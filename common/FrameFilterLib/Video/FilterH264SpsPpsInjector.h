#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"

#include <vector>

namespace ffl {
	class FilterH264SpsPpsInjector : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterH264SpsPpsInjector> Create(const std::shared_ptr<AbstractSource>& src);

		FilterH264SpsPpsInjector();

		void SetSPS(std::vector<unsigned char>&& sps);
		template <class InputIterator>
		void SetSPS(InputIterator begin, InputIterator end)
		{
			SetSPS(std::vector<unsigned char>(begin, end));
		}
		void SetPPS(std::vector<unsigned char>&& pps);
		template <class InputIterator>
		void SetPPS(InputIterator begin, InputIterator end)
		{
			SetPPS(std::vector<unsigned char>(begin, end));
		}

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;

	private:
		void FixH264KeyFrame(vs::SharedBuffer& buffer);

		std::vector<unsigned char> m_saved_sps;
		std::vector<unsigned char> m_saved_pps;
	};
}