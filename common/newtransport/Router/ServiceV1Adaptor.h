#pragma once

#include "transport/Router/VS_PoolThreadsService.h"
#include "transport/Router/VS_TransportRouterServiceBase.h"
#include "std-generic/cpplib/string_view.h"

#include <type_traits>

namespace transport {

class Router;
bool InstallV1Service(VS_TransportRouterServiceBase* srv, string_view name, bool withOwnThread, const std::shared_ptr<Router>& router);
bool UninstallV1Service(VS_TransportRouterServiceBase* srv);

}
