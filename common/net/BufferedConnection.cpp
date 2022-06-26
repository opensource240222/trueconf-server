#include "BufferedConnection.h"
#include "BufferedConnection_impl.h"
#include "tls/socket.h"

namespace net {
template class BufferedConnection<>;
template class BufferedConnection<tls::socket>;
}
