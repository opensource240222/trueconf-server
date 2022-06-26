#include "VS_DBStorageInterface.h"
#include "../../../common/statuslib/VS_ResolveServerFinder.h"
#include "../../../common/std/cpplib/VS_Utils.h"

std::string VS_VCSDBStorageInterface::VCS_ResolveHomeServer(const VS_RealUserLogin& query_r)
{
	const char* q = query_r;
	std::string server;
	VS_ResolveServerFinder	*resolve_srv = VS_ResolveServerFinder::Instance();
	if (resolve_srv)
		resolve_srv->GetServerForResolve(q, server, false);
	auto server_type = VS_GetServerType(server);
	if (server_type == ST_VCS)
		return server;
	else if (server_type == ST_BS || server_type == ST_RS)
	{
		server.clear();
		if (resolve_srv)
			resolve_srv->GetASServerForResolve(q, server, false);
	}
	return server;
}
