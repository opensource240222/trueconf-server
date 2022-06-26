
#undef FD_SETSIZE
#define   FD_SETSIZE   128
#include <Winsock2.h>
#include <ws2tcpip.h>

#include <cstdio>

// Default QOS
// All digit parameters are ingnored because SERVICE_NO_QOS_SIGNALING set to compability with RSVP in Windows 2000
// This working for default mode only and can be overrided by setting QoSdata structure.

#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"

signed long __enable_QoS = SERVICETYPE_GUARANTEED;
signed long __QoS_isOK = -1;  // __QoS_isOK = true when QoS Enable in registry = 1
bool __EncryptLSP = false; // Use EncryptLSP for socket

bool __UseEncryptLSP(bool use) {
	bool _rz = __EncryptLSP;
	__EncryptLSP = use;
	return _rz;
};


FLOWSPEC default_g711 =	{680000, 68000, 1360000, QOS_NOT_SPECIFIED, QOS_NOT_SPECIFIED, __enable_QoS | SERVICE_NO_QOS_SIGNALING, 340, 340};
_QualityOfService g711_qos = {default_g711, default_g711, 0};

void __apply_QoS_changes() {
	if (default_g711.ServiceType != (__enable_QoS | SERVICE_NO_QOS_SIGNALING)) {
		default_g711.ServiceType	=	(__enable_QoS | SERVICE_NO_QOS_SIGNALING);
		g711_qos.SendingFlowspec	=	default_g711;
		g711_qos.ReceivingFlowspec	=	default_g711;
	}
}

signed long GetQoSEnable() {
	signed long value(0);
	VS_RegistryKey key_server(true, CONFIGURATION_KEY);
	key_server.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "MarkQoS");
	if (!value) {
		VS_RegistryKey key_client(true, "Current configuration");
		key_client.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "MarkQoS");
	}
	return value;
}

// End QoS

int SelectIn( const void **handles, const unsigned n_handles, unsigned long &milliseconds )
{
	if (!handles || !n_handles || n_handles > FD_SETSIZE)	return -1;
	unsigned long tc = GetTickCount();
	const timeval   tmval = { milliseconds >> 10, (milliseconds & 0x3FF) << 10 };
	fd_set   ibits;		FD_ZERO(&ibits);
	for (unsigned i = 0; i < n_handles; ++i) {	FD_SET((SOCKET)handles[i],&ibits);	}
	int ret = select(0, &ibits, 0, 0, &tmval);
	tc = GetTickCount() - tc;
	milliseconds = (tc >= milliseconds) ? 0 : milliseconds - tc;
	switch (ret){	case SOCKET_ERROR :		return -1;
					case 0 :				return -2;	}
	for (unsigned i = 0; i < n_handles; ++i)
		if (FD_ISSET((SOCKET)handles[i],&ibits))	return (int)i;
	return -1;
}
// end SelectIn
