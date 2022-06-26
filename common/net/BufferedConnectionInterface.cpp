#include "BufferedConnectionInterface.h"
#include "BufferedConnectionInterface_impl.h"
#include "tls/socket.h"

namespace net
{
template class BufferedConnectionInterface_impl<>;
template class BufferedConnectionInterface_impl<tls::socket>;
}
