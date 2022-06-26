#pragma once

#include <cstddef>
#include <cstdint>

enum VS_ACS_Response : uint32_t;

enum VS_WsRequest {
	vs_invalid,
	vs_ws,
};

namespace ws {
	VS_ACS_Response			ProtocolCheck(const void *in_buffer, size_t in_len);
}