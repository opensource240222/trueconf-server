#pragma once

#include <memory>
#include <vector>

namespace jpeg
{
	bool read_RGB24_file(const char *filename, std::unique_ptr<uint8_t[]> &out, unsigned &width, unsigned &height) noexcept;
	bool write_RGB24_mem(const uint8_t *in, std::vector<uint8_t> &out, unsigned width, unsigned height, int quality) noexcept;

	bool write_I420_mem(const uint8_t *in, std::vector<uint8_t> &out, unsigned width, unsigned height, int quality) noexcept;
} //namespace jpeg