
#ifndef VS_ACS_BWT_H
#define VS_ACS_BWT_H

struct VS_BwtResult
{
	unsigned long   out_response_ms, in_response_ms;
	double   out_bytes, in_bytes;
	float   out_bps, in_bps;
	long   out_jitter_min_ms, in_jitter_min_ms,
			out_jitter_max_ms, in_jitter_max_ms;
};
// end VS_BwtResult struct

class VS_BwtIntermediate
{
public:
	virtual bool Result( const unsigned status, const void *inf, const unsigned mark ) = 0;
	virtual void Out( const unsigned long /*loc_offset_ms*/, const long /*jitter_ms*/ ) {}
	virtual void In( const unsigned long /*loc_offset_ms*/, const long /*jitter_ms*/ ) {}
};
// end VS_BwtIntermediate class

#define   VS_ACS_BWT_MIN_HEX_BUFFER_SIZE   32
#define   VS_ACS_BWT_MAX_HEX_BUFFER_SIZE   32768
#define   VS_ACS_BWT_HEX_BUFFER_SIZE   4096

#define   VS_ACS_BWT_MAX_PERIOD   1000

#define   VS_BWT_MODE_OUT			1
#define   VS_BWT_MODE_IN			2
#define   VS_BWT_MODE_DUPLEX		7
#define   VS_BWT_MODE_HALFDUPLEX	11

#define   VS_BWT_MAX_TEST_SECONDS   120
#define   VS_BWT_MIN_TEST_SECONDS   4

bool VS_BwtAsync( unsigned *ret_code,		//	May bee equal NULL
						const char *server_endpoint_name, VS_BwtResult *result,
						VS_BwtIntermediate *callbacks = 0,
						const unsigned mode = VS_BWT_MODE_HALFDUPLEX,
						const unsigned seconds = 5,
						const unsigned long frames_size = VS_ACS_BWT_HEX_BUFFER_SIZE,
						const unsigned long period = 0 );

unsigned   VS_Bwt( const char *server_endpoint_name, VS_BwtResult *result,
						VS_BwtIntermediate *callbacks = 0,
						const unsigned mode = VS_BWT_MODE_HALFDUPLEX,
						const unsigned seconds = 5,
						const unsigned long frames_size = VS_ACS_BWT_HEX_BUFFER_SIZE,
						const unsigned long period = 0 );

// Values of status and arguments for VS_BwtIntermediate::Result( status, inf, mark )
#define   VS_BWT_ST_INTER_RESULT		0	// inf = &VS_BwtResult, mark - unused
#define   VS_BWT_ST_START_CONNECT		1	// inf - unused, mark = 0-outbound/1-inbound
#define   VS_BWT_ST_CONNECT_ATTEMPT		2	// inf = &net::endpoint::ConnectTCP, mark = number
#define   VS_BWT_ST_CONNECT_OK			3	// inf - unused, mark = 0-outbound/1-inbound
#define   VS_BWT_ST_CONNECT_ERROR		4	// inf - unused, mark = 0-outbound/1-inbound
#define   VS_BWT_ST_START_HANDSHAKE		5	// inf - unused, mark = 0-outbound/1-inbound
#define   VS_BWT_ST_HANDSHAKE_OK		6	// inf - unused, mark = 0-outbound/1-inbound
#define   VS_BWT_ST_HANDSHAKE_ERROR		7	// inf - unused, mark = 0-нет соединения,
											//						1-неверен endpoint,
											//						2-нет ресурсов
#define   VS_BWT_ST_NO_RESOURCES		8	// inf - unused, mark = 0-outbound/1-inbound
#define   VS_BWT_ST_START_TEST			9	// inf - unused, mark = unused
#define   VS_BWT_ST_FINISH_TEST			10	// inf - unused, mark = unused
#define   VS_BWT_ST_CONNECTION_DIED		11	// inf - unused, mark = 0-outbound/1-inbound
#define   VS_BWT_ST_CONNECTION_ERROR	12	// inf - unused, mark = 0-outbound/1-inbound

//  Values of return of VS_Bwt
#define   VS_BWT_RET_OK					0
#define   VS_BWT_RET_ERR_ARGS			1
#define   VS_BWT_RET_ERR_START_ASYNC	2
#define   VS_BWT_RET_ERR_INIT			3
#define   VS_BWT_RET_ERR_CONNECT		4
#define   VS_BWT_RET_ERR_HANDSHAKE		5
#define   VS_BWT_RET_ERR_CONNECTION		6
#define   VS_BWT_RET_TEST_STOP			7

#endif  // VS_ACS_BWT_H
