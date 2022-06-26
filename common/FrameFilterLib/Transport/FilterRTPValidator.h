#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"

#include <boost/container/flat_set.hpp>

#include <cstdint>

namespace ffl {
	class FilterRTPValidator : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		typedef boost::container::flat_set<uint8_t> pt_set_t;

		static std::shared_ptr<FilterRTPValidator> Create(const std::shared_ptr<AbstractSource>& src, const pt_set_t& valid_pt, bool accept_zero_ssrc);

		explicit FilterRTPValidator(const pt_set_t& valid_pt, bool accept_zero_ssrc);

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;

	private:
		pt_set_t m_valid_pt;
		bool m_accept_zero_ssrc;
		bool m_non_zero_ssrc_seen;
	};
}