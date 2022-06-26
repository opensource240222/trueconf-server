#pragma once

#include <memory>

namespace net { namespace ur {
template <class Protocol> class Router;
template <class Protocol> using RouterPtr = std::shared_ptr<Router<Protocol>>;
template <class Protocol> class Connection;
}}
