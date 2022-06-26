
#include <string.h>
#include "VS_AcsHttpLib.h"
#include "acs/connection/VS_ConnectionTypes.h"


unsigned long VS_SizeHeader( const char *start_line[], const char **headers[] )
{
	unsigned long i, j, size = 0;
	if (start_line)
	{	for (j = 0; start_line[j]; ++j)
			size += (unsigned long)strlen( start_line[j] ) + 1;
		++size;
	}
	if (headers)
	{	for (i = 0; headers[i]; ++i)
		{	for (j = 0; headers[i][j]; ++j)
				size += (unsigned long)strlen( headers[i][j] ) + 1;
			++size;
	}	}
	return size ? size + 2 : 4;
}
// end VS_SizeHeader


unsigned long VS_GenerateHeader( char *message, const char *start_line[],
										const char **headers[] )
{
	unsigned long   i, j, k, size = 0;
	if (start_line)
	{	for (j = 0; start_line[j]; ++j)
		{	for (k = 0; start_line[j][k]; ++k)
				message[size++] = start_line[j][k];
			message[size++] = ' ';
		}
		--size;		message[size++] = '\r';		message[size++] = '\n';
	}
	if (headers)
	{	for (i = 0; headers[i]; ++i)
		{	for (j = 0; headers[i][j]; ++j)
			{	for (k = 0; headers[i][j][k]; ++k)
					message[size++] = headers[i][j][k];
				message[size++] = ' ';
			}
			--size;		message[size++] = '\r';		message[size++] = '\n';
	}	}
	if (!size) {	message[size++] = '\r';		message[size++] = '\n';		}
	message[size++] = '\r';		message[size++] = '\n';
	return size;
}
// end VS_GenerateHeader


unsigned long VS_UniteStrings( char *message, const char *strings[] )
{
	unsigned long   j, k, size = 0;
	for (j = 0; strings[j]; ++j)
		for (k = 0; strings[j][k]; ++k)		message[size++] = strings[j][k];
	return size;
}
// end VS_UniteStrings


bool VS_FindEndHeader( VS_HttpMessage *message )
{
	unsigned long   i = message->index;
	for (; i < message->size_mes; ++i)
		if (*( message->buf + i ) == '\r')
		{
			if (( i + 4 ) > message->size_mes)	break;
			if (*( message->buf + i + 1 ) == '\n'
				&& *( message->buf + i + 2 ) == '\r'
				&& *( message->buf + i + 3 ) == '\n')
			{
				if (!i) {	i += 4;	continue;	}
				message->index = i + 4;
				return true;
		}	}
	message->index = i;
	return false;
}
// end VS_FindEndHeader

/// 1 - next word
/// 2 - new line
/// 3 - end header
/// 0 - end header, and it's not complete

int VS_PartHeader( VS_HttpWord *word, VS_HttpMessage *message )
{
	unsigned long   i = message->index, j = i;
	bool   f = false;
	for (; i < message->size_mes; ++i)
	{
		switch (*( message->buf + i ))
		{
		case '\r' :
			if ((i + 4) > message->size_mes)
			{
				message->index = i;
				return 0;
			}
			if (*( message->buf + i + 1 ) == '\n')
			{
				if (f)
				{
					message->index = i;
					word->size = i - j;
					word->word = ( message->buf + j );
					return 1;
				}
				if (*( message->buf + i + 2 ) != '\r'
					|| *( message->buf + i + 3 ) != '\n')
				{
					if (!i) {	i += 4;	break;	}
					message->index = i + 2;
					return 2;
				}
				message->index = i + 4;
				return 3;
			}
			break;
		case ' ' :
		case '\t' :
			if (!f)
			{
				j = i + 1;
				break;
			}
			message->index = i;
			word->size = i - j;
			word->word = (message->buf + j);
			return 1;
		default :
			f = true;
	}	}
	message->index = i;
	return 0;
}
// end VS_PartHeader


VS_Request VS_RequestLine( VS_HttpWord *parts, VS_HttpMessage *message )
{
	VS_HttpWord   wrd[4] = { 0 };
	unsigned int i = 0;
	for (; i < 3; ++i)
		if (VS_PartHeader( wrd + i, message ) != 1)
			return vs_request_line_invalid;
	if (VS_PartHeader( wrd + i, message ) != 2)
		return vs_request_line_invalid;
	if (wrd->size < 3)	return vs_request_method_understand;
	VS_Request   ret = vs_request_method_understand;
	switch (*(unsigned long *)(wrd->word))
	{
	case (('I'<<24)+('T'<<16)+('P'<<8)+'O') :	ret = wrd->size!=7?ret:(*(wrd->word+4)!='O'?ret:(*(wrd->word+5)!='N'?ret:(*(wrd->word+6)!='S'?ret:(*(wrd->word+7)!=' '?ret:vs_request_method_options))));	break;
	case ((' '<<24)+('T'<<16)+('E'<<8)+'G') :	ret = wrd->size!=3?ret:vs_request_method_get;	break;
	case (('D'<<24)+('A'<<16)+('E'<<8)+'H') :	ret = wrd->size!=4?ret:(*(wrd->word+4)!=' '?ret:vs_request_method_head);	break;
	case (('T'<<24)+('S'<<16)+('O'<<8)+'P') :	ret = wrd->size!=4?ret:(*(wrd->word+4)!=' '?ret:vs_request_method_post);	break;
	case ((' '<<24)+('T'<<16)+('U'<<8)+'P') :	ret = wrd->size!=3?ret:vs_request_method_put;	break;
	case (('E'<<24)+('L'<<16)+('E'<<8)+'D') :	ret = wrd->size!=6?ret:(*(wrd->word+4)!='T'?ret:(*(wrd->word+5)!='E'?ret:(*(wrd->word+6)!=' '?ret:vs_request_method_delete)));	break;
	case (('C'<<24)+('A'<<16)+('R'<<8)+'T') :	ret = wrd->size!=5?ret:(*(wrd->word+4)!='E'?ret:(*(wrd->word+5)!=' '?ret:vs_request_method_trace));		break;
	}
	if (ret == vs_request_method_understand)	return ret;
	if (parts)
	{	*parts = *wrd;	*(parts + 1) = *(wrd + 1);	*(parts + 2) = *(wrd + 2);	}
	return ret;
}
// end VS_RequestLine


VS_Response VS_ResponseLine( VS_HttpWord *parts, VS_HttpMessage *message )
{
	VS_HttpWord   wrd = { 0 }, wrd1 = { 0 }, wrd2 = { 0 }, wrd3 = { 0 };
	if (VS_PartHeader( &wrd1, message ) != 1 || VS_PartHeader( &wrd2, message ) != 1)
		return vs_response_line_invalid;
	int i = VS_PartHeader(&wrd3, message);
	if (i == 1)
	{
		wrd = wrd3;
		do	{	i = VS_PartHeader( &wrd, message );	}	while (i == 1);
		if (!i)		return vs_response_line_invalid;
		wrd3.size = (unsigned long)(wrd.word - wrd3.word) + wrd.size;
	}
	if (wrd2.size != 3)		return vs_response_code_not_understand;
	VS_Response   ret = vs_response_code_not_understand;
	switch (*(unsigned long *)wrd2.word)
	{
	case ((' '<<24)+('0'<<16)+('0'<<8)+'1') :	ret = vs_response_code_100;	break;
	case ((' '<<24)+('1'<<16)+('0'<<8)+'1') :	ret = vs_response_code_101;	break;
	case ((' '<<24)+('0'<<16)+('0'<<8)+'2') :	ret = vs_response_code_200;	break;
	case ((' '<<24)+('1'<<16)+('0'<<8)+'2') :	ret = vs_response_code_201;	break;
	case ((' '<<24)+('2'<<16)+('0'<<8)+'2') :	ret = vs_response_code_202;	break;
	case ((' '<<24)+('3'<<16)+('0'<<8)+'2') :	ret = vs_response_code_203;	break;
	case ((' '<<24)+('4'<<16)+('0'<<8)+'2') :	ret = vs_response_code_204;	break;
	case ((' '<<24)+('5'<<16)+('0'<<8)+'2') :	ret = vs_response_code_205;	break;
	case ((' '<<24)+('6'<<16)+('0'<<8)+'2') :	ret = vs_response_code_206;	break;
	case ((' '<<24)+('0'<<16)+('0'<<8)+'3') :	ret = vs_response_code_300;	break;
	case ((' '<<24)+('1'<<16)+('0'<<8)+'3') :	ret = vs_response_code_301;	break;
	case ((' '<<24)+('2'<<16)+('0'<<8)+'3') :	ret = vs_response_code_302;	break;
	case ((' '<<24)+('3'<<16)+('0'<<8)+'3') :	ret = vs_response_code_303;	break;
	case ((' '<<24)+('4'<<16)+('0'<<8)+'3') :	ret = vs_response_code_304;	break;
	case ((' '<<24)+('5'<<16)+('0'<<8)+'3') :	ret = vs_response_code_305;	break;
	case ((' '<<24)+('7'<<16)+('0'<<8)+'3') :	ret = vs_response_code_307;	break;
	case ((' '<<24)+('0'<<16)+('0'<<8)+'4') :	ret = vs_response_code_400;	break;
	case ((' '<<24)+('1'<<16)+('0'<<8)+'4') :	ret = vs_response_code_401;	break;
	case ((' '<<24)+('2'<<16)+('0'<<8)+'4') :	ret = vs_response_code_402;	break;
	case ((' '<<24)+('3'<<16)+('0'<<8)+'4') :	ret = vs_response_code_403;	break;
	case ((' '<<24)+('4'<<16)+('0'<<8)+'4') :	ret = vs_response_code_404;	break;
	case ((' '<<24)+('5'<<16)+('0'<<8)+'4') :	ret = vs_response_code_405;	break;
	case ((' '<<24)+('6'<<16)+('0'<<8)+'4') :	ret = vs_response_code_406;	break;
	case ((' '<<24)+('7'<<16)+('0'<<8)+'4') :	ret = vs_response_code_407;	break;
	case ((' '<<24)+('8'<<16)+('0'<<8)+'4') :	ret = vs_response_code_408;	break;
	case ((' '<<24)+('9'<<16)+('0'<<8)+'4') :	ret = vs_response_code_409;	break;
	case ((' '<<24)+('0'<<16)+('1'<<8)+'4') :	ret = vs_response_code_410;	break;
	case ((' '<<24)+('1'<<16)+('1'<<8)+'4') :	ret = vs_response_code_411;	break;
	case ((' '<<24)+('2'<<16)+('1'<<8)+'4') :	ret = vs_response_code_412;	break;
	case ((' '<<24)+('3'<<16)+('1'<<8)+'4') :	ret = vs_response_code_413;	break;
	case ((' '<<24)+('4'<<16)+('1'<<8)+'4') :	ret = vs_response_code_414;	break;
	case ((' '<<24)+('5'<<16)+('1'<<8)+'4') :	ret = vs_response_code_415;	break;
	case ((' '<<24)+('6'<<16)+('1'<<8)+'4') :	ret = vs_response_code_416;	break;
	case ((' '<<24)+('7'<<16)+('1'<<8)+'4') :	ret = vs_response_code_417;	break;
	case ((' '<<24)+('0'<<16)+('0'<<8)+'5') :	ret = vs_response_code_500;	break;
	case ((' '<<24)+('1'<<16)+('0'<<8)+'5') :	ret = vs_response_code_501;	break;
	case ((' '<<24)+('2'<<16)+('0'<<8)+'5') :	ret = vs_response_code_502;	break;
	case ((' '<<24)+('3'<<16)+('0'<<8)+'5') :	ret = vs_response_code_503;	break;
	case ((' '<<24)+('4'<<16)+('0'<<8)+'5') :	ret = vs_response_code_504;	break;
	case ((' '<<24)+('5'<<16)+('0'<<8)+'5') :	ret = vs_response_code_505;	break;
	default :	return vs_response_code_not_understand;
	}
	if (parts)
	{	*parts = wrd1;	*(parts + 1) = wrd2;	*(parts + 2) = wrd3;	}
	return ret;
}
// end VS_ResponseLine


bool VS_FindHeader( const char *header, VS_HttpWord *parts, VS_HttpMessage *message )
{
	VS_HttpWord   wrd = { 0 }, wrd1 = { 0 }, wrd2 = { 0 };
	VS_HttpMessage   msg = *message;
	//msg.index = 0;
	bool   flag = true;
	while (1)
	{
		switch (VS_PartHeader( &wrd1, &msg ))
		{
		case 1 :	if (!flag)		break;
					if (!strncmp( header, wrd1.word, (size_t)wrd1.size ))
					{
						int i = VS_PartHeader(&wrd2, &msg);
						if (i == 1)
						{
							wrd = wrd2;
							do	{	i = VS_PartHeader( &wrd, &msg );	}	while (i == 1);
							if (!i)		return false;
							wrd2.size = (unsigned long)(wrd.word - wrd2.word) + wrd.size;
						}
						if (parts)	{	*parts = wrd1;	*(parts + 1) = wrd2;	}
						return true;
					}
					flag = false;	break;
		case 2 :	flag = true;	break;
		default :	return false;
}	}	}
// end VS_FindHeader


const char	vs_invalid_line[] = "Status Line Invalid",
		vs_code_not_understand[] = "Status Code Understand",
		vs_not_supported[] = "For this HTTP version it is not supported",
		*vs_reason_phrases_10[] = { vs_invalid_line, vs_code_not_understand,
	vs_not_supported, vs_not_supported, "OK", "Created", "Accepted", vs_not_supported,
	"No Content", vs_not_supported, vs_not_supported, vs_not_supported,
	"Moved Permanently", "Moved Temporarily", vs_not_supported, "Not Modified",
	vs_not_supported, vs_not_supported, "Bad Request", "Unauthorized",
	vs_not_supported, "Forbidden", "Not Found", vs_not_supported, vs_not_supported,
	vs_not_supported, vs_not_supported, vs_not_supported, vs_not_supported,
	vs_not_supported, vs_not_supported, vs_not_supported, vs_not_supported,
	vs_not_supported, vs_not_supported, vs_not_supported, "Internal Server Error",
	"Not Implemented", "Bad Gateway", "Service Unavailable", vs_not_supported,
	vs_not_supported },
		*vs_reason_phrases_11[] = { vs_invalid_line, vs_code_not_understand,
	"Continue", "Switching Protocols", "OK", "Created", "Accepted",
	"Non-Authoritative Information", "No Content", "Reset Content", "Partial Content",
	"Multiple Choices", "Moved Permanently", "Found", "See Other", "Not Modified",
	"Use Proxy", "Temporary Redirect", "Bad Request", "Unauthorized",
	"Payment Required", "Forbidden", "Not Found", "Method Not Allowed",
	"Not Acceptable", "Proxy Authentication Required", "Request Time-out", "Conflict",
	"Gone", "Length Required", "Precondition Failed", "Request Entity Too Large",
	"Request-URI Too Large", "Unsupported Media Type",
	"Requested range not satisfiable", "Expectation Failed", "Internal Server Error",
	"Not Implemented", "Bad Gateway", "Service Unavailable", "Gateway Time-out",
	"HTTP Version not supported" };

//////////////////////////////////////////////////////////////////////////////////////////

bool VS_ReadHttpHead( VS_HttpMessage &message, VS_ConnectionSock_Implementation *imp, unsigned long &mills )
{
	if (message.size_buf < 4 || !message.buf)	return false;
	message.size_mes = message.index = 0;
	int   res = imp->Receive( (void *)message.buf, 4, mills );
	if (res != 4) {		if (res > 0)	message.index = (unsigned long)res;
						return false;		}
	message.index = 4;
	while (1)
	{	unsigned long   bytes = 4;
		if (message.buf[message.index - 1] == 0x0A)
		{	if (message.buf[message.index - 2] == 0x0D)
			{	bytes = 2;
				if (message.buf[message.index - 3] == 0x0A)
				{	if (message.buf[message.index - 4] == 0x0D)
					{	message.size_mes = message.index;
						return true;
		}	}	}	}
		else if (message.buf[message.index - 1] == 0x0D)
		{	bytes = 3;
			if (message.buf[message.index - 2] == 0x0A)
			{	if (message.buf[message.index - 3] == 0x0D)
				{	bytes = 1;
		}	}	}
		if (( message.index + bytes ) > message.size_buf)	return false;
		res = imp->Receive( (void *)&message.buf[message.index], bytes, mills );
		if (res != (int)bytes) {	if (res > 0)	message.index += res;
									return false;		}
		message.index += bytes;
}	}
// end VS_ReadHeader

//////////////////////////////////////////////////////////////////////////////////////////

bool VS_FindReadHttpHead( VS_HttpMessage &message, VS_ConnectionSock_Implementation *imp, unsigned long &mills )
{
	if (message.size_buf < 4 || !message.buf)	return false;
	message.size_mes = 0;
	if (message.index < 4)
	{	const unsigned long   bts = 4 - message.index;
		int   res = imp->Receive( (void *)&message.buf[message.index], bts, mills );
		if (res != (int)bts) {		if (res > 0)	message.index += (unsigned long)res;
									return false;		}
		message.index = 4;
	} else
	{	for (unsigned long i = 4; i < message.index; ++i)
		{
			if (message.buf[i - 1] == 0x0A && message.buf[i - 2] == 0x0D
					&& message.buf[i - 3] == 0x0A && message.buf[i - 4] == 0x0D)
			{	message.size_mes = i;	return true;	}
	}	}
	while (1)
	{	unsigned long   bytes = 4;
		if (message.buf[message.index - 1] == 0x0A)
		{	if (message.buf[message.index - 2] == 0x0D)
			{	bytes = 2;
				if (message.buf[message.index - 3] == 0x0A)
				{	if (message.buf[message.index - 4] == 0x0D)
					{	message.size_mes = message.index;
						return true;
		}	}	}	}
		else if (message.buf[message.index - 1] == 0x0D)
		{	bytes = 3;
			if (message.buf[message.index - 2] == 0x0A)
			{	if (message.buf[message.index - 3] == 0x0D)
				{	bytes = 1;
		}	}	}
		if (( message.index + bytes ) > message.size_buf)	return false;
		int   res = imp->Receive( (void *)&message.buf[message.index], bytes, mills );
		if (res != (int)bytes) {	if (res > 0)	message.index += res;
									return false;		}
		message.index += bytes;
}	}
// end VS_ReadHttpHead

//////////////////////////////////////////////////////////////////////////////////////////

unsigned char VS_BASE64_Encoding[256] = { 0xFF };

bool VS_BASE64_Encoding_Init( void )
{
	unsigned long   j = 0;
	unsigned char   i;
	for (i = (unsigned char)'A'; i <= (unsigned char)'Z'; ++i, ++j)
		*( VS_BASE64_Encoding + j ) = i;
	for (i = (unsigned char)'a'; i <= (unsigned char)'z'; ++i, ++j)
		*( VS_BASE64_Encoding + j ) = i;
	for (i = (unsigned char)'0'; i <= (unsigned char)'9'; ++i, ++j)
		*( VS_BASE64_Encoding + j ) = i;
	*( VS_BASE64_Encoding + j ) = (unsigned char)'+';	++j;
	*( VS_BASE64_Encoding + j ) = (unsigned char)'/';	++j;
	if (j != 64)	return false;
	return true;
}
// end VS_BASE64_Coding_Init


unsigned long VS_BASE64_Encode( const unsigned char *src,
									const unsigned long src_len,
									unsigned char *dst )
{
	if (!src_len || !src || !dst)	return 0;
	unsigned char   b0, b1, b2, c0, c1, c2, c3;
	unsigned long   i, j;
	for (i = j = 0; i < src_len;)
	{
		b0 = *( src + i );									++i;
		c0 = (b0 >> 2);
		*( dst + j ) = *( VS_BASE64_Encoding + c0 );		++j;
		if (i >= src_len)
		{
			c1 = ( b0 & 0x3 ) << 4;
			*( dst + j ) = *( VS_BASE64_Encoding + c1 );	++j;
			break;
		}
		b1 = *( src + i );									++i;
		c1 = (( b0 & 0x3 ) << 4 ) | ( b1 >> 4 );
		*( dst + j ) = *( VS_BASE64_Encoding + c1 );		++j;
		if (i >= src_len)
		{
			c2 = ( b1 & 0xf ) << 2;
			*( dst + j ) = *( VS_BASE64_Encoding + c2 );	++j;
			break;
		}
		b2 = *( src + i );									++i;
		c2 = (( b1 & 0xf ) << 2 ) | ( b2 >> 6 );
		c3 = b2 & 0x3f;
		*( dst + j ) = *( VS_BASE64_Encoding + c2 );		++j;
		*( dst + j ) = *( VS_BASE64_Encoding + c3 );		++j;
	}

	if(src_len % 3 == 1 || src_len % 3 == 2) *( dst + j++ ) = '=';
	if(src_len % 3 == 1) *( dst + j++ ) = '=';

	return j;
}
// end VS_BASE64_Encode


unsigned long VS_BASE64_Encode( const VS_Buffer *buffers,
									const unsigned long n_buffers,
									unsigned char *dst )
{
	return 0;
/*
	if (!buffers || !n_buffers || !dst)		return 0;
	unsigned char   b0, b1, b2, c0, c1, c2, c3;
	unsigned long   i, j, k;
	for (j = k = 0; k < n_buffers;)
		for (i = 0; i < buffers[k].length;)
		{
			b0 = *( (char *)buffers[k].buffer + i );			++i;
			c0 = (b0 >> 2);
			*( dst + j ) = *( VS_BASE64_Encoding + c0 );		++j;
			if (i >= src_len)
			{
				c1 = ( b0 & 0x3 ) << 4;
				*( dst + j ) = *( VS_BASE64_Encoding + c1 );	++j;
				break;
			}
			b1 = *( src + i );									++i;
			c1 = (( b0 & 0x3 ) << 4 ) | ( b1 >> 4 );
			*( dst + j ) = *( VS_BASE64_Encoding + c1 );		++j;
			if (i >= src_len)
			{
				c2 = ( b1 & 0xf ) << 2;
				*( dst + j ) = *( VS_BASE64_Encoding + c2 );	++j;
				break;
			}
			b2 = ( src + i );									++i;
			c2 = (( b1 & 0xf ) << 2 ) | ( b2 >> 6 );
			c3 = b2 & 0x3f;
			*( dst + j ) = *( VS_BASE64_Encoding + c2 );		++j;
			*( dst + j ) = *( VS_BASE64_Encoding + c3 );		++j;
		}
	return j;
*/
}
// end VS_BASE64_Encode


unsigned char VS_BASE64_Decoding[256] = { 0xFF };

bool VS_BASE64_Decoding_Init( void )
{
	unsigned char   j = 0;
	unsigned long   i;
	for (i = (unsigned long)'A'; i <= (unsigned long)'Z'; ++i, ++j)
		*( VS_BASE64_Decoding + i ) = j;
	for (i = (unsigned long)'a'; i <= (unsigned long)'z'; ++i, ++j)
		*( VS_BASE64_Decoding + i ) = j;
	for (i = (unsigned long)'0'; i <= (unsigned long)'9'; ++i, ++j)
		*( VS_BASE64_Decoding + i ) = j;
	*( VS_BASE64_Decoding + (unsigned long)'+' ) = j;	++j;
	*( VS_BASE64_Decoding + (unsigned long)'/' ) = j;	++j;
	if (j != 64)	return false;
	return true;
}
// end VS_BASE64_Decoding_Init


unsigned long VS_BASE64_Decode( const unsigned char *src,
									const unsigned long src_len,
									unsigned char *dst )
{
	if (!src_len || !src || !dst)	return 0;
	unsigned char   c0, c1, c2, c3;
	unsigned long   i, j;
	for (i = j = 0; i < src_len; ++i)
	{
		c0 = *( src + i );							++i;
		c1 = (i < src_len) ? *( src + i ) : 'A';	++i;
		c2 = (i < src_len) ? *( src + i ) : 'A';	++i;
		c3 = (i < src_len) ? *( src + i ) : 'A';	++i;
		c0 = *( VS_BASE64_Decoding + c0 );
		c1 = *( VS_BASE64_Decoding + c1 );
		c2 = *( VS_BASE64_Decoding + c2 );
		c3 = *( VS_BASE64_Decoding + c3 );
		*( dst + j ) = ( c0 << 2) | ( c1 >> 4 );				++j;
		*( dst + j ) = (( c1 & 0xf ) << 4 ) | ( c2 >> 2 );		++j;
		*( dst + j ) = (( c2 & 0x3 ) << 6 ) | c3;				++j;
	}
	return j;
}
// end VS_BASE64_Decode

//////////////////////////////////////////////////////////////////////////////////////////
