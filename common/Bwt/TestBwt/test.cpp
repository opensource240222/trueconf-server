
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <direct.h>

#include "../VS_Bwt.h"
#include "../../std/cpplib/VS_CmdLine.h"
#include "../../std/cpplib/VS_MemoryLeak.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../acs/Lib/VS_AcsLib.h"
#include "../../acs/Lib/VS_DifferentBosh.h"
#include "../../acs/ConnectionManager/VS_ConnectionManager.h"
#include "../../VSClient/VS_ApplicationInfo.h"

int   argc;		char   **argv;

inline void PrintUsage( void );
inline bool SetRootRegistry( const char * );
inline char *GetCurrentEndpoint( void );
inline char *StrOutIn( const unsigned type );

char   default_registry[] = "Online", default_registry2[] = "Client",
		default_registry3[] = "OnlineTwin", default_registry4[] = "ClientTwin";
char   in_type[] = "IN", out_type[] = "OUT", dup_type[] = "DUPLEX", hld_type[] = "HALFDUPLEX";
const size_t   in_t_sz = sizeof(in_type) - 1, out_t_sz = sizeof(out_type) - 1,
				dup_t_sz = sizeof(dup_type) - 1, hld_t_sz = sizeof(hld_type) - 1;
const unsigned  default_duration = 5;

int main( int _argc, char *_argv[] )
{
#ifdef _DEBUG
	//VS_MemoryLeak   ml;
#endif
	argc = _argc;	argv = _argv;
	char   *endpoint = 0, *registry = default_registry;
	unsigned long   frames_size = VS_ACS_BWT_HEX_BUFFER_SIZE, period = 0;
	bool   generate_files = false, request_finish = true;
	unsigned   type = VS_BWT_MODE_HALFDUPLEX, duration = default_duration, res = 0;
	class VS_Auth : public VS_Authentication
	{
		unsigned Request( const char *protocol, const char *proxy, const unsigned short port,
								char *user, const unsigned user_size,
								char *passwd, const unsigned passwd_size )
		{
			char   *usr = (char *)malloc( (size_t)user_size ), *psw = (char *)malloc( (size_t)passwd_size );
			unsigned   ret = 0, i;		int   ch;
			if (usr && psw)
			{	memset( (void *)usr, 0, (size_t)user_size );	memset( (void *)psw, 0, (size_t)passwd_size );
				printf( "Identification Request. Protocol: \"%s\", Proxy: \"%s\", Port: %u.\n", protocol, proxy, port );
				printf( "Press 'Y' for continue: " );
				ch = _getch();	puts( "" );
				if (ch =='y' || ch == 'Y' || ch == 'í' || ch == 'Í')
				{	printf( "User Name: " );	gets( usr );
					printf( "Password: " );
					for (i = 0, ch = _getch(); ch != '\r' && i < ( passwd_size - 1 ); ch = _getch())
					{	if (ch != '\b') {	psw[i++] = ch;	_putch( '*' );	}
						else if (i) {	--i;	_putch( '\b' );	_putch( ' ' );	_putch( '\b' );	}}
					psw[i] = 0;		puts( "" );
					printf( "Press 'Y' to remember User Name and Password for this proxy: " );
					ch = _getch();	puts( "" );		ret = 1;
					if (ch =='y' || ch == 'Y' || ch == 'í' || ch == 'Í')
					{	printf( "Press 'Y' if necessary to write to Registry: " );
						ch = _getch();	puts( "" );		ret = 2;
						if (ch =='y' || ch == 'Y' || ch == 'í' || ch == 'Í')	ret = 3;
			}	}	}
			if (ret) {	strncpy( user, usr, (size_t)user_size );	user[user_size - 1] = 0;
						strncpy( passwd, psw, (size_t)passwd_size );	user[passwd_size - 1] = 0;	}
			if (usr)	free( (void *)usr );		if (psw)	free( (void *)psw );
			return ret;
		}
		// end VS_Auth::Request
	} auth;		// end VS_Auth class
	{
		CmdlineParam   params[64], *param;	memset( (void *)params, 0, sizeof(params) );
		ParseCmdLine( GetCommandLine(), params );
		param = FindParam( params, "N" );	request_finish = (param->m_name == 0);
		param = FindParam( params, "?" );
		if (param->m_name) {	PrintUsage();	goto go_return;		}
		param = FindParam( params, "E" );
		if (param->m_value)		endpoint = _strdup( param->m_value );
		param = FindParam( params, "T" );
		if (param->m_value)
		{
			const char   *value = param->m_value;	const size_t   sz = strlen( value );
			if (!_strnicmp( value, in_type, ( in_t_sz < sz ? in_t_sz : sz )))			type = VS_BWT_MODE_IN;
			else if (!_strnicmp( value, out_type, ( out_t_sz < sz ? out_t_sz : sz )))	type = VS_BWT_MODE_OUT;
			else if (!_strnicmp( value, dup_type, ( dup_t_sz < sz ? dup_t_sz : sz )))	type = VS_BWT_MODE_DUPLEX;
			else if (!_strnicmp( value, hld_type, ( hld_t_sz < sz ? hld_t_sz : sz )))	type = VS_BWT_MODE_HALFDUPLEX;
			else
			{	puts( "The type of the test specified by you is not clear." );
				res = VS_BWT_RET_ERR_ARGS;		goto go_return;
		}	}
		param = FindParam( params, "R" );
		if (param->m_value)		registry = _strdup( param->m_value );
		param = FindParam( params, "D" );
		if (param->m_value)
		{	duration = (unsigned)atoi( param->m_value );
			if (duration < VS_BWT_MIN_TEST_SECONDS || duration > VS_BWT_MAX_TEST_SECONDS)
			{	printf( "Value of duration for testing can be Min: %u, Max: %u.\n", VS_BWT_MIN_TEST_SECONDS, VS_BWT_MAX_TEST_SECONDS );
				res = VS_BWT_RET_ERR_ARGS;		goto go_return;
		}	}
		param = FindParam( params, "S" );
		if (param->m_value)
		{	frames_size = (unsigned long)atoi( param->m_value );
			if (frames_size < VS_ACS_BWT_MIN_HEX_BUFFER_SIZE || frames_size > VS_ACS_BWT_MAX_HEX_BUFFER_SIZE)
			{	printf( "Value of frames size for testing can be Min: %u, Max: %u.\n", VS_ACS_BWT_MIN_HEX_BUFFER_SIZE, VS_ACS_BWT_MAX_HEX_BUFFER_SIZE );
				res = VS_BWT_RET_ERR_ARGS;		goto go_return;
		}	}
		param = FindParam( params, "P" );
		if (param->m_value)
		{	period = (unsigned long)atoi( param->m_value );
			if (period > VS_ACS_BWT_MAX_PERIOD)
			{	printf( "Maximum value of period is %u.\n", VS_ACS_BWT_MAX_PERIOD );
				res = VS_BWT_RET_ERR_ARGS;		goto go_return;
		}	}
		param = FindParam( params, "F" );	generate_files = (param->m_name != 0);
	}
again_set_registry:
	if (!SetRootRegistry( registry ))
	{	if (registry == default_registry)
		{	registry = default_registry2;	goto again_set_registry;	}
		if (registry == default_registry2)
		{	registry = default_registry3;	goto again_set_registry;	}
		if (registry == default_registry3)
		{	registry = default_registry4;	goto again_set_registry;	}
		res = VS_BWT_RET_ERR_ARGS;		goto go_return;
	}
	if (!endpoint)	endpoint = GetCurrentEndpoint();
	if (!endpoint)
	{	if (registry == default_registry)
		{	registry = default_registry2;	goto again_set_registry;	}
		if (registry == default_registry2)
		{	registry = default_registry3;	goto again_set_registry;	}
		if (registry == default_registry3)
		{	registry = default_registry4;	goto again_set_registry;	}
		res = VS_BWT_RET_ERR_ARGS;		goto go_return;
	}
	if (!VS_AcsLibInitial() || !VS_InstallConnectionManager( "TemporyEndpoint" )
			|| !VS_SetAuthenticationInterface( &auth ))
	{	res = VS_BWT_RET_ERR_INIT;		goto go_return;		}
	{
		VS_BwtResult   result;		memset( (void *)&result, 0, sizeof(result) );
		char   *file_out_name = 0, *file_in_name = 0;
		FILE   *file_out = 0, *file_in = 0;		long   pos_out = 0, pos_in = 0;
		if (generate_files)
		{	const char   *dir_name = VS_ModifiedPath( _strdup( endpoint ));
			_mkdir( dir_name );
			char   *file_name = (char *)malloc( strlen( dir_name ) + 64 );
			const unsigned long   add_seq = (unsigned long)time( 0 );
			if (type == VS_BWT_MODE_OUT || type == VS_BWT_MODE_DUPLEX || type == VS_BWT_MODE_HALFDUPLEX)
			{	sprintf( file_name, "%s\\BWT%08X_OUT.CSV", dir_name, add_seq );
				file_out = fopen( file_name, "w" );
				if (file_out)
				{	file_out_name = _strdup( file_name );
					fputs( "Offset.Ms,Send.Ms,Bytes.Seconds\n", file_out );
					pos_out = ftell( file_out );
			}	}
			if (type == VS_BWT_MODE_IN || type == VS_BWT_MODE_DUPLEX || type == VS_BWT_MODE_HALFDUPLEX)
			{	sprintf( file_name, "%s\\BWT%08X_IN.CSV", dir_name, add_seq );
				file_in = fopen( file_name, "w" );
				if (file_in)
				{	file_in_name = _strdup( file_name );
					fputs( "Offset.Ms,Jitter.Ms,Bytes.Seconds\n", file_in );
					pos_in = ftell( file_in );
			}	}
			free( (void *)dir_name );	free( (void *)file_name );
		}
		class Intermediate : public VS_BwtIntermediate
		{	public: Intermediate( const VS_BwtResult &result, FILE *file_out, FILE *file_in )
				: result(result), file_out(file_out), file_in(file_in) {}
			bool Result( const unsigned status, const void *inf, const unsigned mark )
			{	switch (status)
				{
				case VS_BWT_ST_INTER_RESULT :
					{	const VS_BwtResult   &result = *(VS_BwtResult *)inf;
						printf( "Out. Bytes: %.0f, Bps: %.0f.   In. Bytes: %.0f, Bps: %.0f.   \r", result.out_bytes, result.out_bps, result.in_bytes, result.in_bps );	}
					return true;
				case VS_BWT_ST_START_CONNECT :
					printf( "Connection with type %s is starting.\n", StrOutIn( mark ));
					return true;
				case VS_BWT_ST_CONNECT_ATTEMPT :
					{
						auto rc = static_cast<const net::endpoint::ConnectTCP*>(inf);
						printf("Attempt of connection number %u.\n\tHost: %s, Port: %u, Protocol: %s.\n", mark, rc->host.c_str(), rc->port, rc->protocol.c_str());
					}
					return true;
				case VS_BWT_ST_CONNECT_OK :
					printf( "Connection with type %s was established.\n", StrOutIn( mark ));
					return true;
				case VS_BWT_ST_CONNECT_ERROR :
					printf( "Connection with type %s was not established.\n", StrOutIn( mark ));
					return true;
				case VS_BWT_ST_START_HANDSHAKE :
					printf( "Handshake for %s connection is starting.\n", StrOutIn( mark ));
					return true;
				case VS_BWT_ST_HANDSHAKE_OK :
					printf( "Handshake for %s connection was successful.\n", StrOutIn( mark ));
					return true;
				case VS_BWT_ST_HANDSHAKE_ERROR :
					puts( "Handshake for connection was unsuccessful." );
					if (mark == 1)		puts( "\tThe name of a server is incorrectly specified." );
					return true;
				case VS_BWT_ST_NO_RESOURCES :
					printf( "Refused in access to resources for %s connection.\n", StrOutIn( mark ));
					return true;
				case VS_BWT_ST_START_TEST :
					puts( "Test for connection is starting.\n" );
					return true;
				case VS_BWT_ST_FINISH_TEST :
					puts( "\n       Response(ms),  Jitters(min/max)(ms)" );
					printf( "Out.     %7u,        %7d/%-7d\n", result.out_response_ms, result.out_jitter_min_ms, result.out_jitter_max_ms );
					printf( "In.      %7u,        %7d/%-7d\n", result.in_response_ms, result.in_jitter_min_ms, result.in_jitter_max_ms );
					puts( "\nTest for connection was finished." );
					return true;
				case VS_BWT_ST_CONNECTION_DIED :
					printf( "Connection with type %s died.\n", StrOutIn( mark ));
					return true;
				case VS_BWT_ST_CONNECTION_ERROR :
					printf( "Connection with type %s has broken.\n", StrOutIn( mark ));
					return true;
				default :
					printf( "Unknown status: %u\n", status );
					return true;
			}	}
			// end Intermediate::Result
			void Out( const unsigned long loc_offset_ms, const long jitter_ms )
			{	if (file_out)	fprintf( file_out, "%u,%d,%.0f\n", loc_offset_ms, jitter_ms, result.out_bps );	}
			// end Intermediate::Out
			void In( const unsigned long loc_offset_ms, const long jitter_ms )
			{	if (file_in)	fprintf( file_in, "%u,%d,%.0f\n", loc_offset_ms, jitter_ms, result.in_bps );	}
			// end Intermediate::In
			FILE   *file_out, *file_in;		const VS_BwtResult   &result;
		};	// end Intermediate class
		Intermediate   callback( result, file_out, file_in );
		res = VS_Bwt( endpoint, &result, &callback, type, duration, frames_size, period );
		VS_UninstallConnectionManager();
		if (file_out)
		{	const bool   unUse = (pos_out == ftell( file_out ));	fclose( file_out );
			if (unUse && file_out_name)		remove( file_out_name );
			if (file_out_name)	free( (void *)file_out_name );
		}
		if (file_in)
		{	const bool   unUse = (pos_in == ftell( file_in ));		fclose( file_in );
			if (unUse && file_in_name)		remove( file_in_name );
			if (file_in_name)	free( (void *)file_in_name );
	}	}
	if (endpoint)	free( (void *)endpoint );
	if (registry && registry != default_registry && registry != default_registry2
			&& registry != default_registry3 && registry != default_registry4)
		free( (void *)registry );
go_return:
	if (res == VS_BWT_RET_ERR_ARGS)		PrintUsage();
	if (request_finish) {	puts( "Press any key for end" );	_getch();	}
	return res;
}
// end main

inline char *StrOutIn( const unsigned type ) {	return !type ? "outbound" : "inbound";	}
// end StrOutIn

inline bool SetRootRegistry( const char *registry )
{
	char   str[512] = { 0 };
	sprintf( str, "TrueConf\\%s", registry );
	VS_RegistryKey::SetRoot( str );
	VS_RegistryKey   key( true, "Current configuration" );
	return key.IsValid();
}
// end SetRootRegistry

inline char *GetCurrentEndpoint( void )
{
	char   str[512] = { 0 };
	if (!VS_ReadAS( str ) || !*str)		return 0;
	return _strdup( str );
}
// end GetCurrentEndpoint

inline void PrintUsage( void )
{
	char   *pCh = strrchr( argv[0], '\\' );
	if (pCh)	++pCh;		else {		pCh = strrchr( argv[0], '/' );
										if (pCh)	++pCh;		else	pCh = argv[0];	}
	char   *pDt = strrchr( pCh, '.' );		if (pDt)	*pDt = 0;
	printf( "\nUsage: %s [/E:Target_Endpoint] [/T:Test_Type] [/D:Duration] [/R:Registry]\n", pCh );
	puts( "\t\t[/S:Frames_Size] [/P:Period] [/F] [/N]" );
	puts( "Where:\t([xxx] - optional parameter or a word)\n/E:Target_Endpoint - Tested server. By default it is current for \"/R:Registry\"" );
	printf( "/T:Test_Type - %c[%s], %c[%s], %c[%s], %c[%s]. By default it is %s\n", in_type[0], &in_type[1], out_type[0], &out_type[1], dup_type[0], &dup_type[1], hld_type[0], &hld_type[1], hld_type );
	printf( "/D:Duration - Time of duration. By default it is %u sec. Min-Max: %u - %u sec.\n", default_duration, VS_BWT_MIN_TEST_SECONDS, VS_BWT_MAX_TEST_SECONDS );
	printf( "/R:Registry - Registry Key for application. By default under the order\n\t\"%s\",\"%s\",\"%s\",\"%s\"\n", default_registry, default_registry2, default_registry3, default_registry4 );
	printf( "/S:Frames_Size - Size of frames for testing. By default it is %u\n", VS_ACS_BWT_HEX_BUFFER_SIZE );
	puts( "/P:Period - Period between frames. By default it is ZERO - Max Bandwidth" );
	puts( "/F - To generate *.CSV files with all results" );
	puts( "/N - To not request \"Press any key for end\" at finish\n" );
}
// end PrintUsage
