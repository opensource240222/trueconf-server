#pragma once

#include "FrameFilterLib/fwd.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/asio_fwd.h"

#include <boost/shared_ptr.hpp>

#include "std-generic/compat/map.h"
#include "std-generic/compat/memory.h"
#include <mutex>
#include <string>
#include <tuple>

class VS_MediaSourceCollection;
class VS_MediaFormat;

class VS_FFLSourceCollection : public vs::enable_shared_from_this<VS_FFLSourceCollection>
{
public:
	~VS_FFLSourceCollection();

	VS_MediaSourceCollection& MediaSourceCollection()
	{
		return *m_media_source_collection;
	}

	void CleanConference(const char* conf_name);
	void SetSlide(string_view conf_name, const char* url);
	void SetContentSource(string_view conf_name, const char* part_id, std::shared_ptr<ffl::AbstractSource> src);

	std::shared_ptr<ffl::AbstractSource> GetSlideshowSource(string_view conf_name, const char* recv_part_id);

protected:
	VS_FFLSourceCollection(
		boost::asio::io_service &ios,
		const boost::shared_ptr<VS_MediaSourceCollection> &media_source_collection
	);

private:
	boost::asio::io_service& m_ios;
	std::mutex m_mutex;
	boost::shared_ptr<VS_MediaSourceCollection> m_media_source_collection;
	struct slideshow_info
	{
		std::shared_ptr<ffl::FilterNOP> src;
		std::shared_ptr<ffl::SourceStaticImage> slide_src;
		std::shared_ptr<ffl::AbstractSource> pushed_src;
		std::string current_slide;
		std::string requsted_slide;
		std::string pushed_from;
	};
	vs::map<std::string/*conf_name*/, slideshow_info, vs::str_less> m_slideshow_cache;
	slideshow_info& GetSlideshowInfo(string_view conf_name);
};
