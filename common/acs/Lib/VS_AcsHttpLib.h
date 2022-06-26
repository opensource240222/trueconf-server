
#ifndef VS_ACS_HTTP_LIB_H
#define VS_ACS_HTTP_LIB_H

#include "VS_Buffer.h"

unsigned long VS_SizeHeader( const char *start_line[], const char **headers[] );
unsigned long VS_GenerateHeader( char *message, const char *start_line[],
												const char **headers[] );
unsigned long VS_UniteStrings( char *message, const char *strings[] );

struct VS_HttpWord
{	char   *word;
	unsigned long   size;
}; // end VS_HttpWord struct

struct VS_HttpMessage
{	char   *buf;
	unsigned long   size_mes, size_buf, index;
}; // end VS_HttpMessage struct

bool VS_FindEndHeader( VS_HttpMessage *message );
int VS_PartHeader( VS_HttpWord *word, VS_HttpMessage *message );

enum VS_Request
{	vs_request_line_invalid, vs_request_method_understand,
	vs_request_method_get, vs_request_method_head, vs_request_method_post,
	vs_request_method_put, vs_request_method_delete, vs_request_method_connect,
	vs_request_method_checkin, vs_request_method_checkout,
	vs_request_method_link, vs_request_method_search,
	vs_request_method_showmethod, vs_request_method_spacejump,
	vs_request_method_textsearch, vs_request_method_unlink,
	vs_request_method_options, vs_request_method_trace
}; // end VS_Request enum
VS_Request VS_RequestLine( VS_HttpWord *parts, VS_HttpMessage *message );

enum VS_Response
{	vs_response_line_invalid, vs_response_code_not_understand,
	vs_response_code_100, vs_response_code_101, vs_response_code_200,
	vs_response_code_201, vs_response_code_202, vs_response_code_203,
	vs_response_code_204, vs_response_code_205, vs_response_code_206,
	vs_response_code_300, vs_response_code_301, vs_response_code_302,
	vs_response_code_303, vs_response_code_304, vs_response_code_305,
	vs_response_code_307, vs_response_code_400, vs_response_code_401,
	vs_response_code_402, vs_response_code_403, vs_response_code_404,
	vs_response_code_405, vs_response_code_406, vs_response_code_407,
	vs_response_code_408, vs_response_code_409, vs_response_code_410,
	vs_response_code_411, vs_response_code_412, vs_response_code_413,
	vs_response_code_414, vs_response_code_415, vs_response_code_416,
	vs_response_code_417, vs_response_code_500, vs_response_code_501,
	vs_response_code_502, vs_response_code_503, vs_response_code_504,
	vs_response_code_505
}; // end VS_Request enum
VS_Response VS_ResponseLine( VS_HttpWord *parts, VS_HttpMessage *message );

bool VS_FindHeader( const char *header, VS_HttpWord *parts, VS_HttpMessage *message );

extern const char   *vs_reason_phrases_10[], *vs_reason_phrases_11[];

bool VS_ReadHttpHead( VS_HttpMessage &message, struct VS_ConnectionSock_Implementation *imp, unsigned long &mills );
bool VS_FindReadHttpHead( VS_HttpMessage &message, struct VS_ConnectionSock_Implementation *imp, unsigned long &mills );

#define   VS_BASE64_Size_Encode(src_size)		(((src_size << 2) / 3) + ((src_size % 3) != 0))
bool VS_BASE64_Encoding_Init( void );
unsigned long VS_BASE64_Encode( const unsigned char *src,
											const unsigned long src_len,
											unsigned char *dst );
unsigned long VS_BASE64_Encode( const VS_Buffer *buffers,
											const unsigned long n_buffers,
											unsigned char *dst );
#define   VS_BASE64_Size_Decode(src_size)		((src_size * 3) >> 2)
bool VS_BASE64_Decoding_Init( void );
unsigned long VS_BASE64_Decode( const unsigned char *src,
											const unsigned long src_len,
											unsigned char *dst );
#endif // VS_ACS_HTTP_LIB_H

