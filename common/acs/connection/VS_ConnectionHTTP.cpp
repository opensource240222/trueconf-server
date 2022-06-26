/**
 **************************************************************************
 * \file VS_ConnectionHTTP.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief HTTP function realizations.
 *
 *
 *
 * \b Project HTTP tunneling.
 * \author SlavetskyA
 * \date 01.10.02
 *
 * $Revision: 1 $
 *
 * $History: VS_ConnectionHTTP.cpp $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/acs/connection
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/acs/connection
 *
 * *****************  Version 9  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 20:07
 * Updated in $/VS/acs/connection
 *
 ****************************************************************************/

#include <stdio.h>

#include "VS_ConnectionHTTP.h"
#include "VS_ConnectionTypes.h"
#include "../Lib/VS_AcsHttpLib.h"
#include "../Lib/VS_MarkCryptLib.h"

#define   READING_PORTIONS   8196

const char   tmp_crypt[] = "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW";	// It's tempory

const char   start_html_head[] = "<HTML><HEAD>",
			end_html_head[] = "</HEAD></HTML>";
#define   SIZE_START_HTML_HEAD   (sizeof(start_html_head) - 1)
#define   SIZE_END_HTML_HEAD   (sizeof(end_html_head) - 1)
#define   ADD_SIZE_BODY   (SIZE_START_HTML_HEAD + SIZE_END_HTML_HEAD)

struct VS_ConnectionHTTP_Implementation
{
	VS_ConnectionHTTP_Implementation( VS_ConnectionSock_Implementation *imp ) :
		imp(imp), http_type(vs_connection_http_type_uninstalled),
		host_port(0), http_host_port_resource(0),
		canWrite(false), canRead(false), isReaderOrWriter(true),
//		writeMessage(zeroMessage), readMessage(zeroMessage),
		readBuffer(0), readBytes(0), stateRead(0), nWriteIter(0)
	{}
	// end VS_ConnectionHTTP_Implementation

	~VS_ConnectionHTTP_Implementation( void )
	{
		if (http_host_port_resource)	delete http_host_port_resource;
		if (host_port)					delete host_port;
		if (writeMessage.buf)			imp->Free( (void *)writeMessage.buf );
		if (readMessage.buf)			imp->Free( (void *)readMessage.buf );
	}
	// end ~VS_ConnectionHTTP_Implementation

	VS_ConnectionSock_Implementation   *imp;
	VS_ConnectionHTTP_EncapsulationType   http_type;
	char   *host_port, *http_host_port_resource;
	bool   canWrite, canRead, isReaderOrWriter;
	VS_HttpMessage   writeMessage, readMessage;
	void   *readBuffer;
	unsigned long   readBytes, stateRead;
	unsigned long   nWriteIter;

	inline bool GenerateMessage( const char *start_line[], const char **headers[],
					VS_HttpMessage &message,
					const VS_Buffer *bodies = 0, const unsigned long n_bodies = 0,
					char *strBodyLen = 0 )
	{
		if (!message.size_buf)
		{
			unsigned long   size_buf = 0;
			if (bodies && n_bodies)
			{
				size_buf = VS_BuffersLength( bodies, n_bodies );
				if (http_type == vs_connection_http_type_base64)
					size_buf = VS_BASE64_Size_Encode(size_buf);
				size_buf += ADD_SIZE_BODY;
				if (strBodyLen)		_ultoa( size_buf, strBodyLen, 10 );
			}
			message.size_buf = VS_SizeHeader( start_line, headers ) + size_buf;
		}
		if (!message.buf && !imp->Alloc( (void *&)message.buf, message.size_buf ))
			return false;
		message.size_mes = VS_GenerateHeader( message.buf, start_line, headers );
		if (bodies && n_bodies)
		{
			memcpy( (void *)&message.buf[message.size_mes], start_html_head, SIZE_START_HTML_HEAD );
			message.size_mes += SIZE_START_HTML_HEAD;
			if (http_type == vs_connection_http_type_base64)
				message.size_mes += VS_BASE64_Encode( bodies, n_bodies, (unsigned char *)&message.buf[message.size_mes] );
			else
				message.size_mes += VS_BuffersCopy( (void *)&message.buf[message.size_mes], bodies, n_bodies );
			memcpy( (void *)&message.buf[message.size_mes], end_html_head, SIZE_END_HTML_HEAD );
			message.size_mes += SIZE_END_HTML_HEAD;
		}
		message.index = 0;
		return true;
	}
	// end GenerateMessage

	inline bool GenerateMessageStartHtml( const char *start_line[],
											const char **headers[], VS_HttpMessage &message )
	{
		message.size_buf = VS_SizeHeader( start_line, headers ) + SIZE_START_HTML_HEAD;
		if (!GenerateMessage( start_line, headers, message ))	return false;
		memcpy( (void *)&message.buf[message.size_mes], start_html_head, SIZE_START_HTML_HEAD );
		message.size_mes += SIZE_START_HTML_HEAD;
		return true;
	}
	// end GenerateMessageStartHtml

	inline bool ReadHeader( VS_HttpMessage &message, unsigned long &mills )
	{
		if (!message.buf && !imp->Alloc( (void *&)message.buf, message.size_buf ))
			return false;
		message.size_mes = message.index = 0;
		while (1)
		{
			unsigned long   bytes = message.size_buf - message.size_mes;
			if (bytes > 10)		bytes = 10;
			if (!imp->ReadStream( (void *)(message.buf + message.size_mes), bytes )	)
				return false;
			int   ret = imp->GetReadStreamResult( mills, 0, true );
			if (ret < 0)	return false;
			message.size_mes += ret;
			if (VS_FindEndHeader( &message ))	return true;
	}	}
	// end ReadHeader

	inline bool WriteGetRequest( unsigned long &mills )
	{
		sprintf( http_host_port_resource, "http://%s/%s_%ld.html", host_port, tmp_crypt, ++nWriteIter );
		const char   *start_line[] = { "GET", http_host_port_resource, "HTTP/1.0", 0 },
					*header_1[] = { "Accept: */*", 0 },
					*header_2[] = { "Accept-Language: en-us", 0 },
					*header_3[] = { "Connection: Keep-Alive", 0 },
					*header_4[] = { "Proxy-Connection: Keep-Alive", 0 },
					*header_5[] = { "Host:", host_port, 0 },
					*header_6[] = { "Cache-Control: no-cache", 0 },
					*header_7[] = { "Pragma: no-cache", 0 },
					**headers[] = { header_1, header_2, header_3, header_4,
									header_5, header_6, 0 };
		memset( (void *)&writeMessage, 0, sizeof(writeMessage) );
		if (!GenerateMessage( start_line, headers, writeMessage ))		return false;
WriteTxtTrace( "\n\n -- Connect: We will attempt to write --\n\n", writeMessage.buf, writeMessage.size_mes );
		if (!imp->WriteStream( (const void *)writeMessage.buf, writeMessage.size_mes ))	return false;
		if (imp->GetWriteStreamResult( mills ) < 0)		return false;
		imp->Free( writeMessage.buf );		memset( (void *)&writeMessage, 0, sizeof(writeMessage) );
		return true;
	}
	// end WriteGetRequest

	inline bool ReadGetRequest( unsigned long &mills )
	{
		memset( (void *)&readMessage, 0, sizeof(readMessage) );
		readMessage.size_buf = 8096;
		if (!ReadHeader( readMessage, mills ))		return false;
WriteTxtTrace( "\n\n -- Accept: We read --\n\n", readMessage.buf, readMessage.size_mes );
		readMessage.index = 0;
		if (VS_RequestLine( 0, &readMessage ) != vs_request_method_get)		return false;
		imp->Free( readMessage.buf );
		memset( (void *)&readMessage, 0, sizeof(readMessage) );
		return true;
	}
	// end ReadGetRequest

	inline bool Connect( const char *host, const unsigned short port,
					unsigned long &milliseconds, const VS_ConnectionHTTP_EncapsulationType type,
					const char *http_proxy_host, const unsigned short http_proxy_port,
					const char *http_proxy_user, const char *http_proxy_passwd )
	{
		char   *hst = (char *)host;
		unsigned short   prt = port;
		if (http_proxy_host) {	hst = (char *)http_proxy_host;	prt = http_proxy_port;	}
		switch (type)
		{
		case vs_connection_http_type_raw :
		case vs_connection_http_type_base64 :
		case vs_connection_http_type_tunneling :	break;
		default :	return false;
		}
		if (imp->state < vs_sock_state_connected
			&& !imp->Connect( hst, prt, vs_connection_type_stream, milliseconds ))
			return false;
		host_port = new char[strlen(host) + 32];
		http_host_port_resource = new char[strlen(host) + 64];
		if (!host_port || !http_host_port_resource)		return false;
		sprintf( host_port, "%s:%u", host, port );
		if (!WriteGetRequest( milliseconds ))	return false;
		char   crypt[VS_MARK_CRYPT_LEN] = { 0 };
		canRead = true;
		canWrite = false;
		if (!Read( (void *)crypt, sizeof(crypt) ))	return false;
		if (GetReadResult( milliseconds ) < 0)		return false;
		http_type = type;
		return true;
	}
	// end Connect

	inline bool AcceptHTTP( unsigned long &milliseconds )
	{
		if (!ReadGetRequest( milliseconds ))	return false;
		canRead = false;
		canWrite = true;
		char   contentLength[16] = { 0 };
		if (!Write( (const void *)tmp_crypt, VS_MARK_CRYPT_LEN ))	return false;
		if (GetWriteResult( milliseconds ) < 0)		return false;
		http_type = vs_connection_http_type_raw;
		return true;
	}
	// end AcceptHTTP

	inline bool Accept( const char *host, const unsigned short port, unsigned long &milliseconds )
	{
		if (imp->state < vs_sock_state_connected
				&& !imp->Accept( host, port, vs_connection_type_stream, milliseconds,false,false,false,nullptr))
			return false;
		return AcceptHTTP( milliseconds );
	}
	// end Accept

	inline bool Accept( VS_ConnectionTCP *listener, unsigned long &milliseconds )
	{
		if (imp->state < vs_sock_state_connected
				&& !imp->Accept( listener->imp, milliseconds ))
			return false;
		return AcceptHTTP( milliseconds );
	}
	// end Accept

	inline bool SetAsWriterConnect( unsigned long &milliseconds )
	{
		if (!canWrite)	return false;
		const char   *start_line[] = { "POST", http_host_port_resource, "HTTP/1.0", 0 },
					*header_1[] = { "Connection: Keep-Alive", 0 },
					*header_2[] = { "Proxy-Connection: Keep-Alive", 0 },
					*header_3[] = { "Content-Type: text/html; charset=windows-1251", 0 },
//					*header_4[] = { "Content-Length: 2147483646", 0 },
					*header_4[] = { "Content-Length: 1048576", 0 },
					*header_5[] = { "Host:", host_port, 0 },
					*header_6[] = { "Cache-Control: no-cache", 0 },
					*header_7[] = { "Pragma: no-cache", 0 },
					**headers[] = { header_1, header_2, header_3, header_4,
									header_5, header_6, header_7, 0 };
		memset( (void *)&writeMessage, 0, sizeof(writeMessage) );
		if (!GenerateMessageStartHtml( start_line, headers, writeMessage ))
			return false;
WriteTxtTrace( "\n\n -- Connect (SetAsWriter): We will attempt to write --\n\n", writeMessage.buf, writeMessage.size_mes );
		if (!imp->WriteStream( (const void *)writeMessage.buf, writeMessage.size_mes ))	return false;
		if (imp->GetWriteStreamResult( milliseconds ) < 0)	return false;
		imp->Free( writeMessage.buf );
		isReaderOrWriter = canRead = false;
		return canWrite = true;
	}
	// end SetAsWriterConnect

	inline bool SetAsWriterAccept( unsigned long &milliseconds )
	{
		if (!canRead)	return false;
		if (!ReadGetRequest( milliseconds ))	return false;
		const char   *start_line[] = { "HTTP/1.0 200 OK", 0 },
					*header_1[] = { "Connection: Keep-Alive", 0 },
					*header_2[] = { "Proxy-Connection: Keep-Alive", 0 },
					*header_3[] = { "Content-Type: text/html; charset=windows-1251", 0 },
					*header_4[] = { "Cache-Control: no-cache", 0 },
					*header_5[] = { "Pragma: no-cache", 0 },
					**headers[] = { header_1, header_2, header_3, header_4,
									header_5, 0 };
		memset( (void *)&writeMessage, 0, sizeof(writeMessage) );
		if (!GenerateMessageStartHtml( start_line, headers, writeMessage ))
			return false;
WriteTxtTrace( "\n\n -- Accept (SetAsWriter): We will attempt to write --\n\n", writeMessage.buf, writeMessage.size_mes );
		if (!imp->WriteStream( (const void *)writeMessage.buf, writeMessage.size_mes ))	return false;
		if (imp->GetWriteStreamResult( milliseconds ) < 0)	return false;
		imp->Free( writeMessage.buf );
		isReaderOrWriter = canRead = false;
		return canWrite = true;
	}
	// end SetAsWriterAccept

	inline bool SetAsWriter( unsigned long &milliseconds )
	{
		if (!isReaderOrWriter)		return false;
		switch (imp->connectType)
		{
		case vs_sock_type_connect :		return SetAsWriterConnect( milliseconds );
		case vs_sock_type_accept :		return SetAsWriterAccept( milliseconds );
		default :	return false;
	}	}
	// end SetAsWriter

	inline bool SetAsReaderConnect( unsigned long &milliseconds )
	{
		if (!canWrite)	return false;
		if (!WriteGetRequest( milliseconds ))	return false;
		memset( (void *)&readMessage, 0, sizeof(readMessage) );
		readMessage.size_buf = 8096;
		if (!ReadHeader( readMessage, milliseconds ))	return false;
WriteTxtTrace( "\n\n -- Connect (SetAsReader): We read --\n\n", readMessage.buf, readMessage.size_mes );
		unsigned long   index = readMessage.index;
		readMessage.index = 0;
		if (VS_ResponseLine( 0, &readMessage ) != vs_response_code_200)		return false;
		readMessage.index = index;
		index = readMessage.size_mes - index;
		if (index < SIZE_START_HTML_HEAD)
		{
			if (!imp->ReadStream( (void *)&readMessage.buf[readMessage.size_mes], SIZE_START_HTML_HEAD - index ))
				return false;
			if (imp->GetReadStreamResult( milliseconds ) < 0)	return false;
		}
		if (strncmp( (const char *)&readMessage.buf[readMessage.index], start_html_head, SIZE_START_HTML_HEAD ))
			return false;
		imp->Free( readMessage.buf );
		memset( (void *)&readMessage, 0, sizeof(readMessage) );
		isReaderOrWriter = canWrite = false;
		return canRead = true;
	}
	// end SetAsReaderConnect

	inline bool SetAsReaderAccept( unsigned long &milliseconds )
	{
		if (!canRead)	return false;
		memset( (void *)&readMessage, 0, sizeof(readMessage) );
		readMessage.size_buf = 8096;
		if (!ReadHeader( readMessage, milliseconds ))		return false;
WriteTxtTrace( "\n\n -- Connect (SetAsReader): We read --\n\n", readMessage.buf, readMessage.size_mes );
		unsigned long   index = readMessage.index;
		readMessage.index = 0;
		if (VS_RequestLine( 0, &readMessage ) != vs_request_method_post)	return false;
		readMessage.index = index;
		index = readMessage.size_mes - index;
		if (index < SIZE_START_HTML_HEAD)
		{
			if (!imp->ReadStream( (void *)&readMessage.buf[readMessage.size_mes], SIZE_START_HTML_HEAD - index ))
				return false;
			if (imp->GetReadStreamResult( milliseconds ) < 0)	return false;
		}
		if (strncmp( (const char *)&readMessage.buf[readMessage.index], start_html_head, SIZE_START_HTML_HEAD ))
			return false;
		imp->Free( readMessage.buf );
		memset( (void *)&readMessage, 0, sizeof(readMessage) );
		isReaderOrWriter = canWrite = false;
		return canRead = true;
	}
	// end SetAsReaderAccept

	inline bool SetAsReader( unsigned long &milliseconds )
	{
		if (!isReaderOrWriter)	return false;
		switch (imp->connectType)
		{
		case vs_sock_type_connect :		return SetAsReaderConnect( milliseconds );
		case vs_sock_type_accept :		return SetAsReaderAccept( milliseconds );
		default :	return false;
	}	}
	// end SetAsReader

	inline bool RWriteConnect( const VS_Buffer *buffers, const unsigned long n_buffers )
	{
		char   contentLength[16] = { 0 };
		sprintf( http_host_port_resource, "http://%s/%s_%ld.html", host_port, tmp_crypt, ++nWriteIter );
		const char   *start_line[] = { "POST", http_host_port_resource, "HTTP/1.0", 0 },
					*header_1[] = { "Connection:", "Keep-Alive", 0 },
					*header_2[] = { "Proxy-Connection: Keep-Alive", 0 },
					*header_3[] = { "Content-Type: text/html; charset=windows-1251", 0 },
					*header_4[] = { "Host:", host_port, 0 },
					*header_5[] = { "Content-Length:", contentLength, 0 },
					*header_6[] = { "Cache-Control: no-cache", 0 },
					*header_7[] = { "Pragma: no-cache", 0 },
					**headers[] = { header_1, header_2, header_3, header_4,
									header_5, header_6, header_7, 0 };
		memset( (void *)&writeMessage, 0, sizeof(writeMessage) );
		if (!GenerateMessage( start_line, headers, writeMessage, buffers, n_buffers, contentLength ))
			return false;
WriteTxtTrace( "\n\n -- Connect: We will attempt to write --\n\n", writeMessage.buf, writeMessage.size_mes );
		return imp->WriteStream( (const void *)writeMessage.buf, writeMessage.size_mes );
	}
	// end RWriteConnect

	inline bool RWriteAccept( const VS_Buffer *buffers, const unsigned long n_buffers )
	{
		char   contentLength[16] = { 0 };
		const char   *start_line[] = { "HTTP/1.0 200 OK", 0 },
					*header_1[] = { "Connection: Keep-Alive", 0 },
					*header_2[] = { "Proxy-Connection: Keep-Alive", 0 },
					*header_3[] = { "Content-Type: text/html; charset=windows-1251", 0 },
					*header_4[] = { "Content-Length:", contentLength, 0 },
					*header_5[] = { "Cache-Control: no-cache", 0 },
					*header_6[] = { "Pragma: no-cache", 0 },
					**headers[] = { header_1, header_2, header_3, header_4,
									header_5, header_6, 0 };
		memset( (void *)&writeMessage, 0, sizeof(writeMessage) );
		if (!GenerateMessage( start_line, headers, writeMessage, buffers, n_buffers, contentLength ))
			return false;
WriteTxtTrace( "\n\n -- Accept: We will attempt to write --\n\n", writeMessage.buf, writeMessage.size_mes );
		return imp->WriteStream( (const void *)writeMessage.buf, writeMessage.size_mes );
	}
	// end RWriteAccept

	inline bool RWrite( const VS_Buffer *buffers, const unsigned long n_buffers )
	{
		if (!canWrite)	return false;
		if (!isReaderOrWriter)	return imp->RWriteStream( buffers, n_buffers );
		switch (imp->connectType)
		{
		case vs_sock_type_connect :		return RWriteConnect( buffers, n_buffers );
		case vs_sock_type_accept :		return RWriteAccept( buffers, n_buffers );
		default :	return false;
	}	}
	// end RWrite

	inline bool Write( const void *buffer, const unsigned long n_bytes )
	{
		if (!canWrite)	return false;
		if (!isReaderOrWriter)	return imp->WriteStream( buffer, n_bytes );
		VS_Buffer   buffers = { n_bytes, (void *)buffer };
		switch (imp->connectType)
		{
		case vs_sock_type_connect :		return RWriteConnect( &buffers, 1 );
		case vs_sock_type_accept :		return RWriteAccept( &buffers, 1 );
		default :	return false;
	}	}
	// end Write

	inline int GetWriteResult( unsigned long &milliseconds )
	{
		if (!canWrite)	return false;
		int ret = imp->GetWriteStreamResult(milliseconds);
		if (!isReaderOrWriter || ret < 0)	return ret;
		imp->Free( writeMessage.buf );
		canWrite = false;
		canRead = true;
		return ret;
	}
	// end GetWriteResult

	inline int SetWriteResult( const unsigned long b_trans, const struct VS_Overlapped *ov )
	{
		if (!canWrite)	return false;
		int ret = imp->SetWriteStreamResult(b_trans, ov);
		if (!isReaderOrWriter || ret < 0)	return ret;
		imp->Free( writeMessage.buf );
		canWrite = false;
		canRead = true;
		return ret;
	}
	// end SetWriteResult

	inline bool RReadHttp( void )
	{
		if (!imp->isRead)
		{
			if (!readMessage.buf)
			{
				if (!imp->Alloc( (void *&)readMessage.buf, READING_PORTIONS ))
					return false;
				readMessage.size_buf = READING_PORTIONS;
				stateRead = 0;
			}
			return imp->ReadStream( (void *)&readMessage.buf[readMessage.size_mes],
									readMessage.size_buf - readMessage.size_mes );
		}
		if (!imp->fRdOv)		return imp->fRdOv = true;
		return false;
	}
	// end RReadHttp

	inline bool RRead( const unsigned long n_bytes )
	{
		if (!canRead)	return false;
		if (!isReaderOrWriter)	return imp->RReadStream( n_bytes );
		readBuffer = 0;
		readBytes = n_bytes;
		return RReadHttp();
	}
	// end RRead

	inline bool Read( void *buffer, const unsigned long n_bytes )
	{
		if (!canRead)	return false;
		if (!isReaderOrWriter)	return imp->ReadStream( buffer, n_bytes );
		readBuffer = buffer;
		readBytes = n_bytes;
		return RReadHttp();
	}
	// end Read

	inline int GetReadBodyResult( unsigned long &milliseconds, void **buffer,
									const bool portion )
	{
		while (readMessage.size_mes < readMessage.size_buf)
		{
			if (!imp->ReadStream( (void *)&readMessage.buf[readMessage.size_mes],
					readMessage.size_buf - readMessage.size_mes ))
				return -1;
			int ret = imp->GetReadStreamResult(milliseconds, 0, true);
			if (ret < 0)
				return ret;
			readMessage.size_mes += ret;
		}
		readMessage.size_mes -= SIZE_END_HTML_HEAD;
		if (strncmp( &readMessage.buf[readMessage.size_mes], end_html_head, SIZE_END_HTML_HEAD ))
			return -1;
		unsigned long ret = readMessage.size_mes - readMessage.index;
		if (readBytes < ret)	ret = readBytes;
		else if (readBytes > ret && !portion)
			return -1;
		if (readBuffer)
		{
			memcpy( readBuffer, (void *)&readMessage.buf[readMessage.index], ret );
			if (buffer)		*buffer = 0;
		}
		else
		{
			if (!buffer || !imp->Alloc( *buffer, ret ))
				return -1;
			memcpy( *buffer, (void *)&readMessage.buf[readMessage.index], ret );
		}
		if ((readMessage.index += ret) < readMessage.size_mes)	imp->fRdOv = false;
		else
		{
char   tmpStr[64] = { 0 };
sprintf( tmpStr, "\n\n -- %s: We read --\n\n", imp->connectType == vs_sock_type_connect ? "Connect" : "Accept" );
WriteTxtTrace( tmpStr, readMessage.buf, readMessage.size_mes + SIZE_END_HTML_HEAD );
			imp->Free( (void *)readMessage.buf );
			memset( (void *)&readMessage, 0, sizeof(readMessage) );
			stateRead = 0;
			canWrite = true;
			canRead = false;
		}
		return (int)ret;
	}
	// end GetReadBodyResult

	inline int GetReadStartHtmlResult( unsigned long &milliseconds, void **buffer,
											const bool portion )
	{
		while ((readMessage.size_mes - readMessage.index) < SIZE_START_HTML_HEAD)
		{
			if (!imp->ReadStream( (void *)&readMessage.buf[readMessage.size_mes],
					readMessage.size_buf - readMessage.size_mes ))
				return -1;
			int ret = imp->GetReadStreamResult(milliseconds, 0, true);
			if (ret < 0)
				return ret;
			if ((readMessage.size_mes += ret) >= readMessage.size_buf)
				return -1;
		}
		if (strncmp( &readMessage.buf[readMessage.index], start_html_head, SIZE_START_HTML_HEAD ))
			return -1;
		unsigned long   szHdr = readMessage.index;
		readMessage.index = 0;
		VS_HttpWord   parts[3] = { 0 };
		switch (imp->connectType)
		{
		case vs_sock_type_connect :
			if (VS_ResponseLine( parts, &readMessage ) != vs_response_code_200)
				return -1;
			break;
		case vs_sock_type_accept :
			if (VS_RequestLine( parts, &readMessage) != vs_request_method_post)
				return -1;
			break;
		default :
			return -1;
		}
		memset( (void *)parts, 0, sizeof(VS_HttpWord) * 3 );
		if (!VS_FindHeader( "Content-Length:", parts, &readMessage ))
			return -1;
		unsigned long   szBd = 0, szDt = readMessage.size_mes - szHdr;
		if (!(parts + 1)->word || sscanf( (parts + 1)->word, "%ld", &szBd ) != 1
			|| szBd <= ADD_SIZE_BODY)
			return -1;
		szHdr += SIZE_START_HTML_HEAD;
		szBd -= SIZE_START_HTML_HEAD;
		szDt -= SIZE_START_HTML_HEAD;
		if ((szBd + szHdr) > readMessage.size_buf)
		{
			char   *oldBuf = readMessage.buf;
			readMessage.buf = 0;
			if (!imp->Alloc( (void *&)readMessage.buf, szBd ))
				return -1;
			memcpy( (void *)&readMessage.buf[szHdr], oldBuf, (size_t)szDt );
			readMessage.size_buf = szBd;
			readMessage.size_mes = szDt;
			readMessage.index = 0;
			imp->Free( (void *)oldBuf );
		}
		else
		{
			readMessage.size_buf = szHdr + szBd;
			readMessage.index = szHdr;
		}
		stateRead = 2;
		return GetReadBodyResult( milliseconds, buffer, portion );
	}
	// end GetReadStartHtmlResult

	inline int GetReadHeaderResult( unsigned long &milliseconds, void **buffer,
										const bool portion )
	{
		while (!VS_FindEndHeader( &readMessage ))
		{
			if (!imp->ReadStream( (void *)&readMessage.buf[readMessage.size_mes],
					readMessage.size_buf - readMessage.size_mes ))
				return -1;
			int ret = imp->GetReadStreamResult(milliseconds, 0, true);
			if (ret < 0)
				return ret;
			if ((readMessage.size_mes += ret) >= readMessage.size_buf)
				return -1;
		}
		stateRead = 1;
		return GetReadStartHtmlResult( milliseconds, buffer, portion );
	}
	// end GetReadHeaderResult

	inline int GetReadResult( unsigned long &milliseconds, void **buffer = 0,
								const bool portion = false )
	{
		if (!canRead)
			return -1;
		if (!isReaderOrWriter)
			return imp->GetReadStreamResult( milliseconds, buffer, portion );
		int ret = imp->GetReadStreamResult(milliseconds, buffer, true);
		readMessage.size_mes += ret;
		switch (stateRead)
		{
		case 0 :	return GetReadHeaderResult( milliseconds, buffer, portion );
		case 1 :	return GetReadStartHtmlResult( milliseconds, buffer, portion );
		case 2 :	return GetReadBodyResult( milliseconds, buffer, portion );
		default:
			return -1;
	}	}
	// end GetReadResult

	inline int SetReadResult( const unsigned long b_trans, const struct VS_Overlapped *ov,
								void **buffer, const bool portion )
	{
		return -1;
	}
};
// end VS_ConnectionHTTP_Implementation struct

VS_ConnectionHTTP::VS_ConnectionHTTP( void ) :
	imp(new VS_ConnectionHTTP_Implementation( VS_ConnectionSock::imp ))
{}
// end VS_ConnectionHTTP::VS_ConnectionHTTP

VS_ConnectionHTTP::~VS_ConnectionHTTP( void )
{	if (&imp != 0)		delete &imp;	}
// end VS_ConnectionHTTP::~VS_ConnectionHTTP

bool VS_ConnectionHTTP::Connect( const char *host, const unsigned short port,
					unsigned long &milliseconds, const VS_ConnectionHTTP_EncapsulationType type,
					const char *http_proxy_host, const unsigned short http_proxy_port,
					const char *http_proxy_user, const char *http_proxy_passwd )
{	return imp->Connect( host, port, milliseconds, type, http_proxy_host, http_proxy_port, http_proxy_user, http_proxy_passwd );	}
// end VS_ConnectionHTTP::Connect

bool VS_ConnectionHTTP::Accept( const char *host, const unsigned short port,
									unsigned long &milliseconds )
{	return imp->Accept( host, port, milliseconds );	}
// end VS_ConnectionHTTP::Accept

int VS_ConnectionHTTP::Accept( VS_ConnectionTCP *listener, unsigned long &milliseconds )
{	return imp->Accept( listener, milliseconds );	}
// end VS_ConnectionHTTP::Accept

bool VS_ConnectionHTTP::SetAsWriter( unsigned long &milliseconds )
{	return imp->SetAsWriter( milliseconds );	}
// end SetAsWriter

bool VS_ConnectionHTTP::SetAsReader( unsigned long &milliseconds )
{	return imp->SetAsReader( milliseconds );	}
// end SetAsReader

VS_ConnectionHTTP_EncapsulationType VS_ConnectionHTTP::HTTP_Type( void ) const
{	return imp->http_type;	}
// end VS_ConnectionHTTP::HTTP_Type

bool VS_ConnectionHTTP::RWrite( const VS_Buffer *buffers, const unsigned long n_buffers )
{	return imp->RWrite( buffers, n_buffers );		}
// end VS_ConnectionHTTP::RWrite

bool VS_ConnectionHTTP::Write( const void *buffer, const unsigned long n_bytes )
{	return imp->Write( buffer, n_bytes );	}
// end VS_ConnectionHTTP::Write

int VS_ConnectionHTTP::GetWriteResult( unsigned long &milliseconds )
{	return imp->GetWriteResult( milliseconds );		}
// end VS_ConnectionHTTP::GetWriteResult

int VS_ConnectionHTTP::SetWriteResult( const unsigned long b_trans,
											const struct VS_Overlapped *ov )
{	return imp->SetWriteResult( b_trans, ov );	}
// end VS_ConnectionHTTP::SetWriteResult

bool VS_ConnectionHTTP::RRead( const unsigned long n_bytes )
{	return imp->RRead( n_bytes );		}
// end VS_ConnectionHTTP::RRead

bool VS_ConnectionHTTP::Read( void *buffer, const unsigned long n_bytes )
{	return imp->Read( buffer, n_bytes );	}
// end VS_ConnectionHTTP::Read

int VS_ConnectionHTTP::GetReadResult( unsigned long &milliseconds,
										void **buffer, const bool portion )
{	return imp->GetReadResult( milliseconds, buffer, portion );	}
// end VS_ConnectionHTTP::GetReadResult

int VS_ConnectionHTTP::SetReadResult( const unsigned long b_trans,
										const struct VS_Overlapped *ov,
										void **buffer, const bool portion )
{	return imp->SetReadResult( b_trans, ov, buffer, portion );	}
// end VS_ConnectionHTTP::SetReadResult
