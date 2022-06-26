#include "VS_FFLSourceCollection.h"
#include "../FrameFilterLib/Endpoints/SourceMediaPeer.h"
#include "../FrameFilterLib/Endpoints/SourceStaticImage.h"
#include "../FrameFilterLib/Utility/FilterNOP.h"
#include "../tools/SingleGatewayLib/VS_SlideShowDownloader.h"
#include "../std/cpplib/VS_Protocol.h"
#include "std-generic/cpplib/scope_exit.h"
#include "../std/debuglog/VS_Debug.h"

#include <cassert>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

VS_FFLSourceCollection::VS_FFLSourceCollection(
	boost::asio::io_service& ios,
	const boost::shared_ptr<VS_MediaSourceCollection>& media_source_collection
)
	: m_ios(ios)
	, m_media_source_collection(media_source_collection)
{
}

VS_FFLSourceCollection::~VS_FFLSourceCollection()
{
	// Destroy any stale content chains.
	for (auto& kv : m_slideshow_cache)
		kv.second.src->Detach();
}

void VS_FFLSourceCollection::CleanConference(const char* conf_name)
{
	auto s_it = m_slideshow_cache.find(string_view(conf_name));
	if (s_it != m_slideshow_cache.end())
	{
		s_it->second.src->Detach();
		m_slideshow_cache.erase(s_it);
	}
}

void VS_FFLSourceCollection::SetSlide(string_view conf_name, const char* url_)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto& si = GetSlideshowInfo(conf_name);

	if (url_)
	{
		if (!si.slide_src)
			si.slide_src = ffl::SourceStaticImage::Create();

		if (si.current_slide == url_)
		{
			// Reconnect slide_src to src in case src was switched to pushed_src.
			si.slide_src->Resume();
			si.slide_src->RegisterSinkOrGetCompatible(si.src);
			return;
		}

		if (si.requsted_slide == url_)
			return;

		std::string url(url_);
		si.requsted_slide = url;
		auto res = vs::download_slide(url , [this, conf_name = std::string(conf_name), url, self = shared_from_this()](std::vector<uint8_t> &image, const size_t w, const size_t h) {
			std::lock_guard<std::mutex> lock(m_mutex);
			auto& si = GetSlideshowInfo(conf_name);

			auto ds = dstream4;
			ds << "Slide download (conf=" << conf_name << ", url=\"" << url << "\"): ";

			assert(si.requsted_slide == url);
			if (si.requsted_slide != url)
			{
				ds << "out of order\n";
				return;
			}
			VS_SCOPE_EXIT { si.requsted_slide.clear(); };

			if (image.empty())
			{
				ds << "failure\n";
				return;
			}
			else
			{
				assert(w != 0);
				assert(h != 0);
			}
			ds << "success (width = " << w << ", height = " << h << "): ";

			if (!si.slide_src->SetImage(&image[0], image.size(), w, h))
			{
				ds << " image source update failed\n";
				return;
			}
			ds << "image source updated\n";

			si.current_slide = url;
			si.slide_src->Resume();
			si.slide_src->RegisterSinkOrGetCompatible(si.src);
		});

		if (!res)
		{
			si.requsted_slide.clear();
		}
	}
	else
	{
		si.current_slide.clear();
		si.requsted_slide.clear();
		// Reconnect to pushed source if possible.
		if (si.pushed_src)
			si.pushed_src->RegisterSinkOrGetCompatible(si.src);
		else
			si.src->Detach();
	}
}

void VS_FFLSourceCollection::SetContentSource(string_view conf_name, const char* part_id, std::shared_ptr<ffl::AbstractSource> src)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto& si = GetSlideshowInfo(conf_name);

	if (src)
	{
		si.pushed_src = std::move(src);
		si.pushed_from = part_id;
		si.pushed_src->RegisterSinkOrGetCompatible(si.src);
	}
	else if (si.pushed_from == part_id)
	{
		si.pushed_src = nullptr;
		si.pushed_from.clear();
		// Reconnect to slide source if possible.
		if (si.slide_src)
			si.slide_src->RegisterSinkOrGetCompatible(si.src);
		else
			si.src->Detach();
	}
}

std::shared_ptr<ffl::AbstractSource> VS_FFLSourceCollection::GetSlideshowSource(string_view conf_name, const char* recv_part_id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return GetSlideshowInfo(conf_name).src;
}

auto VS_FFLSourceCollection::GetSlideshowInfo(string_view conf_name) -> slideshow_info&
{
	// Assuming that everybody in the conference must see the same slide

	auto src_it = m_slideshow_cache.find(conf_name);
	if (src_it == m_slideshow_cache.end())
	{
		src_it = m_slideshow_cache.emplace(conf_name, slideshow_info()).first;
		src_it->second.src = ffl::FilterNOP::Create();
		src_it->second.src->Persistent(true);
	}
	return src_it->second;
}