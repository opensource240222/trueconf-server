#pragma once

#include <cstdint>
#include <cstddef>

/* secure */
namespace sec {
	uint32_t crc32(const void *buf, size_t size);
}