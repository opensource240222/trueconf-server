#pragma once

namespace net
{
enum class BufferedConnectionState: unsigned
{
	empty,
	active,
	shutdown,
	close
};
}
