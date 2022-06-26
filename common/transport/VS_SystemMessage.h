#ifndef _VS_SYSTEM_MESSAGE_H_
#define _VS_SYSTEM_MESSAGE_H_

#include "transport/Router/VS_TransportRouterServiceHelper.h"

class VS_SystemMessage: public virtual VS_TransportRouterServiceHelper
{
public:
	void SendCommandMessage(const char* message, const char * user, const char* server=0);
	void SendSystemMessage(const char* message, const char * user, const char* server=0);
	void SendSystemMessageByCid(const wchar_t* message, const char * cid);
};

#endif /*_VS_SYSTEM_MESSAGE_H_*/