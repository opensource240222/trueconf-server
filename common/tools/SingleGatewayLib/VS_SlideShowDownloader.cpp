#include "VS_SlideShowDownloader.h"
#include "std/cpplib/ASIOThreadPool.h"

#include "std-generic/cpplib/scope_exit.h"
#include "../../std/debuglog/VS_Debug.h"
#include "../../IppLib2/VSVideoProcessingIpp.h"

#include "extlibs/stb/stb_image.h"

#include <memory>
#include <mutex>
#include <thread>

#include <cassert>
#include <cstdint>

#include <curl/curl.h>


#define DEBUG_CURRENT_MODULE VS_DM_OTHER

static std::unique_ptr<vs::ASIOThreadPool> g_executor;
static std::mutex g_executor_lock;

static vs::ASIOThreadPool& GetExecutor()
{
	std::lock_guard<std::mutex> l(g_executor_lock);

	if (g_executor == nullptr)
	{
		g_executor = std::make_unique<vs::ASIOThreadPool>(std::thread::hardware_concurrency(), "SlideDL");
		g_executor->Start();
	}
	return *g_executor;
}

static size_t download_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	::std::vector<uint8_t> *data = reinterpret_cast<::std::vector<uint8_t> *>(userdata);
	data->insert(data->end(), ptr, ptr + size * nmemb);
	return size * nmemb;
}

static bool download_slide_data(const ::std::string &url, ::std::vector<uint8_t> &data)
{
	CURL *curl;
	CURLcode res;

	bool ok = true;

	curl = curl_easy_init();
	VS_SCOPE_EXIT {
		if (curl)
		{
			curl_easy_cleanup(curl);
		}
	};

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
		{
			dprint4("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			ok = false;
		}
	}

	return ok;
}

static bool convert_image(std::vector<uint8_t> &&in, std::vector<uint8_t> &out, size_t &out_w, size_t &out_h)
{
	size_t w = 0, h = 0;
	::std::vector<uint8_t> preparedData;

	// load RGB image data from downloaded image file
	{
		int lw = 0, lh = 0;
		int loaded_channels = 0;

		auto image = stbi_load_from_memory(&in[0],
			static_cast<int>(in.size()),
			&lw, &lh, &loaded_channels,
			STBI_rgb); // 24 bits per channel (3 bytes)
		w = static_cast<size_t>(lw);
		h = static_cast<size_t>(lh);
		in.clear();

		if (image == nullptr)
		{
			return false;
		}

		VS_SCOPE_EXIT {
			if (image)
			{
				stbi_image_free(image);
			}
		};

		// flip image vertically and normalise image size (crop a bit) to make it suitable for conversion to YUV 4:2:0
		{
			const size_t bytes_per_line = w * 3; // 3 bytes per channel (24 bits)
			const size_t normalised_bytes_per_line = (w & ~7) * 3; // 3 bytes per channel (24 bits)
			const size_t image_size = h * bytes_per_line;
			const size_t normalised_image_size = (h & ~7) * normalised_bytes_per_line;
			preparedData.resize(normalised_image_size);
			for (size_t i = 0; i < (h & ~7); i++)
			{
				auto *to = &preparedData[i * normalised_bytes_per_line];
				auto from = &image[image_size - ((i + 1) * bytes_per_line)];
				memcpy(to, from, normalised_bytes_per_line);
			}

			w &= ~7;
			h &= ~7;
		}
	}

	// convert image to YUV 4:2:0
	out.resize(w * h / 2 * 3);
	//VS_VideoProc prc;
	VSVideoProcessingIpp prc;
	uint8_t *Y = &out[0];
	uint8_t *U = Y + w * h;
	uint8_t *V = U + w * h / 4;
	auto res = prc.ConvertRGB24ToI420(&preparedData[0], Y, U, V, w * 3, h, w);
	if (res)
	{
		out_w = w;
		out_h = h;
	}
	else
	{
		out.clear();
	}

	return res;
}

bool vs::download_slide(const ::std::string &url, ::std::function<void(::std::vector<uint8_t> &slide_image_data, const size_t w, const size_t h)> on_complete)
{
	if (url.empty() || on_complete == nullptr)
	{
		return false;
	}

	std::string surl = url;
	GetExecutor().get_io_service().post([surl, on_complete](void) -> void {
		::std::vector<uint8_t> slide_data;
		::std::vector<uint8_t> slide_image;
		size_t w = 0, h = 0;

		if (!download_slide_data(surl, slide_data))
		{
			on_complete(slide_image, w, h); // empty image - failure
			return;
		}

		if (!convert_image(std::move(slide_data), slide_image, w, h))
		{
			on_complete(slide_image, w, h); // empty image - failure
			return;
		}

		on_complete(slide_image, w, h);
	});
	return true;
}
