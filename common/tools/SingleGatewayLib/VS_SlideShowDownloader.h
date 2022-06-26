#pragma once
#include <vector>
#include <functional>

namespace vs
{
	extern bool download_slide(const ::std::string &url, ::std::function<void(::std::vector<uint8_t> &slide_image_data, const size_t w, const size_t h)> on_complete);
};
