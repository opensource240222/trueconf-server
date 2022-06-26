
#pragma once

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"

#include "std-generic/compat/map.h"
#include "std-generic/cpplib/StrCompare.h"
#include <memory>

namespace tc3_test { class VS_DSControlServiceTest; }

class VS_DSControlService : public VS_TransportRouterServiceReplyHelper
{
	struct ToPart {
		std::string conf;
		std::string server;
		vs::map< std::string, std::string, vs::str_less>	from_map; // from, server
	};
	vs::map< std::string, ToPart, vs::str_less>	m_ds_map;
public:
	friend class tc3_test::VS_DSControlServiceTest;

	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	// Protocol Methods
private:
	void VideoSourceType_Method(VS_Container &cnt);
	void DSControlRequest_Method(VS_Container &cnt);
	void DSControlResponse_Method(VS_Container &cnt);
	void DSControlFinish_Method(VS_Container &cnt);
	bool DSControlFinished(string_view to, string_view from);
	void DSCommand_Method(VS_Container &cnt);
	void DeleteParticipant_Method(VS_Container &cnt);
	const char* PairControlled(string_view to, string_view from);
};
