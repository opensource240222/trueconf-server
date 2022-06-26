

#ifdef _WIN32 // not ported
#include "VS_PingService.h"
//#include "VS_EndpointRegistrationService.h"
#include "transport/Router/VS_RouterMessage.h"
#include "std-generic/cpplib/VS_Container.h"
#include "Common.h"
#include "../common/std/cpplib/VS_MemoryLeak.h"

/*#include <malloc.h>
#include <string.h>*/

bool VS_PingService::Init(const char* /*our_endpoint*/, const char* /*our_service*/, const bool /*permittedAll*/)
{
	return true;
}

// VS_TransportRouterCallService implementation
bool VS_PingService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)	// Skip
		return true;
	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		// Write to log !!! Error !!!
		break;
		// Store request
	case transport::MessageType::Request:
		m_recvMess = recvMess.get();
		{
			VS_Container cnt;
			if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
				// Search method name
				const char* method = 0;
				if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
					// Process methods
					if (_stricmp(method, PING_METHOD) == 0) {
						Ping_Method();
					} // end if
				} else {
					// Write to log !!! Error !!! No method Name
				} // end if
			} else {
				// Write to log !!! Error !!! Invalid Body
			} // end if
		}
		break;
	case transport::MessageType::Reply:
		// Write to log !!! Error !!!
		break;
	case transport::MessageType::Notify: // Later... may be...
		break;
	} // end switch

	m_recvMess = nullptr;
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// PING_METHOD()
////////////////////////////////////////////////////////////////////////////////
void VS_PingService::Ping_Method()
{
	// Make Body
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, PING_METHOD);
	void* body;
	size_t bodySize;
	rCnt.SerializeAlloc(body, bodySize);
	VS_RouterMessage *reply = new VS_RouterMessage(m_recvMess, 20000, body, bodySize);
	if (!PostMes(reply))
		delete reply;
	free(body);
}
#endif