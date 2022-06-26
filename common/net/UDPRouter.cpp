#include "net/UDPRouter.h"
#include "net/UDPRouter_impl.h"

namespace net { namespace ur {
template class Router<boost::asio::ip::udp>;
template class Connection<boost::asio::ip::udp>;
}}

