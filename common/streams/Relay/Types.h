#pragma once

#include "../fwd.h"

#include <boost/signals2/signal.hpp>

namespace stream {

typedef boost::signals2::signal<void(const char* /*conf_name*/, const char* /*part_name*/, const FrameHeader* /*header*/, const void* /*data*/)> FrameReceivedSignalType;

}
