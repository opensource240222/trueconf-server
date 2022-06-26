#include "VS_VCSLocatorService.h"
#include "../../common/std/cpplib/VS_SimpleStr.h"
#include "../../common/std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_RESOLVE

bool VS_VCSLocatorService::Init(const char* /*our_endpoint*/, const char* /*our_service*/, const bool /*permittedAll*/)
{
	return true;
}

bool VS_VCSLocatorService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)  return true;

	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
	{
		VS_SimpleStr service = recvMess->AddString();
		if (!service) {
			dprint1("no service to forward (msg from %s:%s)\n", recvMess->SrcServer(), recvMess->SrcUser());
			break;
		}

		VS_SimpleStr server = OurEndpoint();

		recvMess->SetAddString("");
		recvMess->SetDstService((char*)service);
		recvMess->SetDstServer(server.m_str);
		auto m = recvMess.release();
		if (!PostMes(m))
			delete m;
		dprint3(" to server %s\n", server.m_str);
		break;
	}
	case transport::MessageType::Notify:
		break;
	}
	return true;
}