#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "FrameFilterLib/Base/Debug.h"
#include "std-generic/cpplib/string_view.h"

#include <atomic>
#include <fstream>
#include <string>

namespace ffl {

class FilterDumpAudio : public AbstractFilter<AbstractSingleSourceSink>
{
public:
	static std::shared_ptr<AbstractSource> Create(const std::shared_ptr<AbstractSource>& src, string_view name);

#if FFL_DUMP_AUDIO
	FilterDumpAudio(string_view name);

	bool IsCompatibleWith(const AbstractSink* sink) override;

protected:
	e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;

private:
	std::ofstream m_file;
	std::string m_name;
	unsigned m_id;

	static std::atomic<unsigned> s_cnt;
#endif
};

#if !FFL_DUMP_AUDIO
inline std::shared_ptr<AbstractSource> FilterDumpAudio::Create(const std::shared_ptr<AbstractSource>& src, string_view /*name*/)
{
	return src;
}
#endif

}
