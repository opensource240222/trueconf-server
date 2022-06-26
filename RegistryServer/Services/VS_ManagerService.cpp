#include "VS_ManagerService.h"
#include "../../common/std/cpplib/VS_Protocol.h"
#include <string.h>

#define DEBUG_CURRENT_MODULE VS_DM_REGS

VS_ManagerService::VS_ManagerService()
{

}

VS_ManagerService::~VS_ManagerService()
{

}

bool VS_ManagerService::Init(const char *our_endpoint, const char *our_service,const bool permittedAll)
{
	return true;
}

bool VS_ManagerService::Processing(std::unique_ptr<VS_RouterMessage>&&recvMess)
{
    if (recvMess == 0)  return true;
    switch (recvMess->Type())
    {
	case transport::MessageType::Invalid:
        break;
	case transport::MessageType::Request:
        m_recvMess = recvMess.get();
        {
            VS_Container cnt;
            if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
                const char* method = 0;
                if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
                    if (_stricmp(method, LOGSTATS_METHOD) == 0)
					{
						// HACK: VCS <= 3.0.7 send to MANAGER_SRV instead of REGISTRATION_SRV
						if (m_recvMess->SrcServer())
						{
							cnt.AddValue(SERVER_PARAM, (const char*) m_recvMess->SrcServer());
							PostRequest(OurEndpoint(), 0, cnt, 0, REGISTRATION_SRV);
						}
					}
                }
            }
        }
        break;
	case transport::MessageType::Reply:
        break;
	case transport::MessageType::Notify:
        break;
    }
    m_recvMess = nullptr;
    return true;
}