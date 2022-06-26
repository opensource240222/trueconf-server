
#ifndef VS_TRANSPORT_LIB_H
#define VS_TRANSPORT_LIB_H

#include "../../net/Handshake.h"
#include "../../acs/VS_AcsDefinitions.h"

extern const char VS_Transport_PrimaryField[net::HandshakeHeader::primary_field_size];

#define VS_MIN_TRANSPORT_VERSION 8
#define VS_MAX_TRANSPORT_VERSION 0x0f

#define VS_CURRENT_TRANSPORT_VERSION 9

#if VS_CURRENT_TRANSPORT_VERSION < VS_MIN_TRANSPORT_VERSION
#error VISICRON: WRONG TRANSPORT VERSION DEFINED
#endif

#if VS_CURRENT_TRANSPORT_VERSION > VS_MAX_TRANSPORT_VERSION
#error VISICRON: WRONG TRANSPORT VERSION DEFINED
#endif

#define VS_NEWARCH_TRANSPORT_VERSION 8
#if VS_NEWARCH_TRANSPORT_VERSION != 8
#error VISICRON: NEW ARCITECTURE STARTED FROM v.8
#endif

enum hs_errors {
	hserr_ok		= 0,
	hserr_antikyou	= 1,
	hserr_unused_01	= 2, // lost in the space
	hserr_antikme	= 3,
	hserr_alienserver = 4, // lost in the space
	hserr_busy		= 5,
	hserr_verify_failed = 6, // sign verification failed;
	hserr_ssl_required  = 7,  // A client tried to connect without SSL encription
	hserr_oldarch	= 0xff // use only to answer to old arch.
};

extern const unsigned char VS_SSL_SUPPORT_BITMASK;

extern const char *VS_TRANSPORT_MANAGING_TYPE_NAME_VALUE;
extern const char *VS_TRANSPORT_MANAGING_TYPE_NAME_GATES;
extern const char *VS_TRANSPORT_MANAGING_VERSION_NAME_VALUE;

extern const char *VS_VersionNameValue;	// = VS_TRANSPORT_MANAGING_VERSION_NAME_VALUE
extern const char *VS_TypeNameValue;	// = VS_TRANSPORT_MANAGING_TYPE_NAME_VALUE
extern const char *VS_GatesNameValue;	// = VS_TRANSPORT_MANAGING_TYPE_NAME_GATES

// declaration this in header prevents error C2065 in switch/case statement
const char VS_TRANSPORT_MANAGING_PING		= 'p';
const char VS_TRANSPORT_MANAGING_CONNECT	= 'c';
const char VS_TRANSPORT_MANAGING_DISCONNECT	= 'd';

extern const unsigned long VS_TRANSPORT_TIMELIFE_PING;
extern const unsigned long VS_TRANSPORT_TIMELIFE_CONNECT;
extern const unsigned long VS_TRANSPORT_TIMELIFE_DISCONNECT;

#define VS_RND_DATA_FOR_SIGN_SZ	16

net::HandshakeHeader* VS_FormTransportHandshake(const char* cid,
								const char *sid,
								const unsigned char hops,
									bool  IsSSLSupport = false,
									bool  tcpKeepAliveSupport = true);

bool VS_TransformTransportHandshake(net::HandshakeHeader* hs, char *&cid,
									char *&sid,
									unsigned char &hops, unsigned long &rnd_data_ln,
									const unsigned char *&rnd_data, unsigned long &sign_sz,const unsigned char *&sign,
									bool &tcpKeepAliveSupport);

net::HandshakeHeader* VS_FormTransportReplyHandshake(const char *cid,
								const unsigned char resultCode,
								const unsigned short maxConnSilenceMs,
								const unsigned char fatalSilenceCoef, const unsigned char hops,
								const char *server_id,
								bool IsSSLSupport = false,
								bool tcpKeepAliveSupport = true);

bool VS_TransformTransportReplyHandshake(net::HandshakeHeader* hs,
											unsigned char &resultCode,
											unsigned short &maxConnSilenceMs,
											unsigned char &fatalSilenceCoef,
											unsigned char &hops,
											char *&server_id,
											char *&cid,
											bool &tcpKeepAliveSupport
											);

net::HandshakeHeader *VS_FormTransportReplyHandshake___OLDARCH(); // use for answer to old architecture only

#pragma pack(   )

#endif  // VS_TRANSPORT_LIB_H
