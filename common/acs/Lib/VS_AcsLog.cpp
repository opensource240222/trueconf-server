
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <fstream>
#include <sys/stat.h>
#include <windows.h>
#include "../../std/cpplib/VS_MemoryLeak.h"
#include "VS_AcsLog.h"
#include "../../std/cpplib/VS_Utils.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/clib/vs_time.h"

const char   VS_AcsTransportExtLog[] = VS_ACS_LOG_TRANSPORT_EXTENSION;


struct VS_AcsLog_Implementation
{
	VS_AcsLog_Implementation( const char *endpoint,
							const unsigned long maxSize, const unsigned long hysSize,
							const char *directory, const char *extension, const bool to_stdout = false ) :
		maxSize(maxSize), hysSize(hysSize), extension(0), fileName(0), file(0),
		last_errno(0), last_error(0),log_to_stdout(to_stdout)
	{
		InitializeCriticalSection( &sect );
		if(log_to_stdout)
		{
			file = stdout;
			return;
		}
		if (hysSize >= maxSize || !endpoint || !extension)		return;
		char   *tempDir = (char *)malloc( 512 + 1024 ), *fileName,
				*endp = _strdup( endpoint ), *extn = _strdup( extension );
		VS_SCOPE_EXIT {
			free(tempDir);
			free(endp);
			free(extn);
		};
		if (!tempDir || !endp || !extn)		return;
		fileName = &tempDir[512];	*tempDir = *fileName = 0;
		if (!directory) {	DWORD   res = GetTempPath( 512, tempDir );
							if (!res || (int)res >= 512 || !*tempDir)	return;	}
		else {				if (strlen( directory ) >= 512)		return;
							strcpy( tempDir, directory );		}
		const unsigned   ind = (const unsigned)strlen(tempDir) - 1;
		const char   *ch = (tempDir[ind] == '/' || tempDir[ind] == '\\') ? "" : "\\";
		sprintf(fileName, "%s%svs_%s.%s", tempDir, ch, VS_FilterPath(endp), VS_FilterPath(extn));
		VS_AcsLog_Implementation::extension = _strdup( extension );
		VS_AcsLog_Implementation::fileName = _strdup( fileName );
		if (!VS_AcsLog_Implementation::fileName)	return;
		const int   fd = _open( VS_AcsLog_Implementation::fileName, _O_APPEND | _O_BINARY | _O_TEXT | _O_CREAT | _O_RDWR, _S_IREAD | _S_IWRITE );
		if (fd == -1) {		SetErrorsClose();	return;	}
		file = _fdopen( fd, "a+" );
		if (!file || fseek( file, 0, SEEK_END )) {		SetErrorsClose();	return;	}
		Resize();
	}
	// end VS_AcsLog_Implementation::VS_AcsLog_Implementation

	~VS_AcsLog_Implementation( void )
	{
		if (file&&!log_to_stdout)
		{
			const bool   fileZero = !_filelength( _fileno( file ));
			fclose( file );
			if (fileZero && fileName)	remove( fileName );
		}
		if (extension)	free( (void *)extension );
		if (fileName)	free( (void *)fileName );
		DeleteCriticalSection( &sect );
	}
	// end VS_AcsLog_Implementation::~VS_AcsLog_Implementation

	const unsigned long   maxSize, hysSize;
	char   *extension, *fileName;	FILE   *file;
	int   last_errno;	DWORD   last_error;
	bool log_to_stdout;
	CRITICAL_SECTION   sect;

	inline void SetErrorsClose( void )
	{
		last_errno = errno;		last_error = GetLastError();
		if (file&&!log_to_stdout) {		fclose( file );		file = 0;	}
	}
	// end VS_AcsLog_Implementation::SetErrorsClose

	inline unsigned long Curtail( const unsigned long current, const unsigned long necessary )
	{
		if (current <= necessary)	return current;
		if (!necessary)
		{	if (fseek( file, 0, SEEK_SET ) || _chsize( _fileno( file ), 0 ))	SetErrorsClose();
			return 0;	}
		if (fseek( file, -(long)necessary, SEEK_END ))
		{	SetErrorsClose();	return 0;	}
		void   *buff = malloc( (size_t)necessary );
		if (!buff)
		{	if (fseek( file, 0, SEEK_SET ) || _chsize( _fileno( file ), 0 ))	SetErrorsClose();
			return 0;	}
		else
		{	unsigned long   ret = necessary;
			if (fread( buff, (size_t)necessary, 1, file ) != 1
					|| fseek( file, 0, SEEK_SET )
					|| _chsize( _fileno( file ), 0 )
					|| fwrite( buff, (size_t)necessary, 1, file ) != 1)
			{	SetErrorsClose();	ret = 0;	}
			free( (void *)buff );	return ret;
	}	}
	// end VS_AcsLog_Implementation::Curtail

	inline unsigned long Resize( void )
	{
		if(log_to_stdout)
			return 0;
		if (!file)	return 0;
		if (fseek( file, 0, SEEK_END )) {	SetErrorsClose();	return 0;	}
		const unsigned long   current = (const unsigned long)ftell( file );
		if (current == (const unsigned long)-1L) {	SetErrorsClose();	return 0;	}
		return current > maxSize ? Curtail( current, hysSize ) : current;
	}
	// end VS_AcsLog_Implementation::Resize

	inline int Printf(const char *format, ...)
	{
		va_list arg;
		va_start(arg, format);
		auto ret = VPrintf(format, arg);
		va_end(arg);
		return ret;
	}
	inline int VPrintf(const char *format, va_list arg)
	{
		int   ret = EOF;
		EnterCriticalSection( &sect );
		if (file) {
			const size_t sizet = 2000;
			char t[sizet] = {0};
			vsprintf_s(t, sizet, format, arg);
			ret = fputs(t, file);
			if (ret < 0)	SetErrorsClose();
			else			Resize();
		}
		LeaveCriticalSection( &sect );
		return ret;
	}
	inline int TPrintf(const char *format, ...)
	{
		va_list arg;
		va_start(arg, format);
		auto ret = VTPrintf(format, arg);
		va_end(arg);
		return ret;
	}
	inline int VTPrintf(const char *format, va_list arg)
	{
		int   ret = EOF;
		EnterCriticalSection( &sect );
		if (file) {
			const size_t sizet = 2000;
			char t[sizet] = {0};
			char chtime[32];
			vsprintf_s(t, sizet, format, arg);
			time_t curt;
			time(&curt);
			tm curt_tm;
			strftime(chtime, 30, "\n%d/%m/%Y %H:%M:%S", localtime_r(&curt, &curt_tm));
			ret = fprintf(file, "%s - [%5ld] %s", chtime, GetCurrentThreadId(), t);
			if (ret < 0)	SetErrorsClose();
			else			Resize();
		}
		LeaveCriticalSection( &sect );
		return ret;
	}
	inline int CPrintf(const char *format, ...)
	{
		va_list arg;
		va_start(arg, format);
		auto ret = VCPrintf(format, arg);
		va_end(arg);
		return ret;
	}
	inline int VCPrintf(const char *format, va_list arg)
	{
		int   ret = EOF;
		EnterCriticalSection( &sect );
		if (file) {
			const size_t sizet = 2000;
			char t[sizet] = {0};
			vsprintf_s(t, sizet, format, arg);
			puts(t);
			ret = fputs(t, file);
			if (ret < 0)	SetErrorsClose();
			else			Resize();
		}
		LeaveCriticalSection( &sect );
		return ret;
	}

	inline int Puts( const char *string )
	{
		if (!string)	return 0;
		int   ret = EOF;
		EnterCriticalSection( &sect );
		if (file) {
			ret = fputs( string, file );
			if (ret < 0)	SetErrorsClose();
			else			Resize();
		}
		LeaveCriticalSection( &sect );
		return ret;
	}
	// end VS_AcsLog_Implementation::Puts

	inline int CPuts( const char *string )
	{
		if (!string)	return 0;
		int   ret = EOF;
		EnterCriticalSection( &sect );
		if (file) {
			puts( string );
			ret = fputs( string, file );
			if (ret < 0)	SetErrorsClose();
			else			Resize();
		}
		LeaveCriticalSection( &sect );
		return ret;
	}
	// end VS_AcsLog_Implementation::CPuts

	inline unsigned long GetSize( void )
	{
		EnterCriticalSection( &sect );
		unsigned long   ret = Resize();
		LeaveCriticalSection( &sect );
		return ret;
	}
	// end VS_AcsLog_Implementation::GetSize

	inline unsigned long GetDataAct( void *data, const unsigned long size )
	{
		const unsigned long   current = Resize();
		if (!current)	return 0;
		const unsigned long   read_size = current > size ? size : current;
		if (fseek( file, 0, SEEK_SET )
			|| fread( data, (size_t)read_size, 1, file ) != 1)
		{	SetErrorsClose();	return 0;	}
		Curtail( current, current - read_size );	return read_size;
	}
	// end VS_AcsLog_Implementation::GetDataAct

	inline unsigned long GetData( void *data, const unsigned long size )
	{
		if (!data || !size)		return 0;
		EnterCriticalSection( &sect );
		unsigned long   ret = GetDataAct( data, size );
		LeaveCriticalSection( &sect );
		return ret;
	}
	// end VS_AcsLog_Implementation::GetData

	inline unsigned long GetDataAct( VS_Container &container )
	{
		const unsigned long   read_size = Resize();
		if (!read_size)		return 0;
		void   *data = malloc( (size_t)read_size );
		if (!data)	return 0;
		unsigned long   ret = read_size;
		if (fseek( file, 0, SEEK_SET )
				|| fread( data, (size_t)read_size, 1, file ) != 1)
		{	SetErrorsClose();	ret = 0;	}
		else if (!extension || !container.AddValue(extension, data, read_size))
		{	if (fseek( file, 0, SEEK_END ))		SetErrorsClose();
			ret = 0;	}
		else	Curtail( read_size, 0 );
		free( data );	return ret;
	}
	// end VS_AcsLog_Implementation::GetDataAct

	inline unsigned long GetData( VS_Container &container )
	{
		EnterCriticalSection( &sect );
		unsigned long   ret = GetDataAct( container );
		LeaveCriticalSection( &sect );
		return ret;
	}
	// end VS_AcsLog_Implementation::GetData

	inline unsigned long PutDataAct( const void *data, const unsigned long size )
	{
		if (!file || !data || !size)	return 0;
		if (fwrite( data, size, 1, file ) != 1) {	SetErrorsClose();	return 0;	}
		return !Resize() ? 0 : size;
	}
	// end VS_AcsLog_Implementation::PutDataAct

	inline unsigned long PutData( const void *data, const unsigned long size )
	{
		EnterCriticalSection( &sect );
		unsigned long   ret = PutDataAct( data, size );
		LeaveCriticalSection( &sect );
		return ret;
	}
	// end VS_AcsLog_Implementation::PutData

	inline unsigned long PutDataAct( VS_Container &container )
	{
		if (!file)	return 0;
		size_t size = 0;
		const void   *data = container.GetBinValueRef( extension, size );
		if (!data || !size)		return 0;
		if (fwrite( data, size, 1, file ) != 1) {	SetErrorsClose();	return 0;	}
		return !Resize() ? 0 : size;
	}
	// end VS_AcsLog_Implementation::PutDataAct

	inline unsigned long PutData( VS_Container &container )
	{
		EnterCriticalSection( &sect );
		unsigned long   ret = PutDataAct( container );
		LeaveCriticalSection( &sect );
		return ret;
	}
	// end VS_AcsLog_Implementation::PutData

	inline unsigned long ReadLastDataAct( void *data, const unsigned long size )
	{
		if (!file)	return 0;
		if (fseek( file, 0, SEEK_END )) {	SetErrorsClose();	return 0;	}
		const unsigned long   current = (const unsigned long)ftell( file ),
								ret = current < size ? current : size;
		if (fseek( file, -(long)ret, SEEK_CUR ) || fread( data, (size_t)ret, 1, file ) != 1)
		{	SetErrorsClose();	return 0;	}
		fseek( file, 0, SEEK_END );		return ret;
	}
	// end VS_AcsLog_Implementation::ReadLastDataAct

	inline unsigned long ReadLastData( void *data, const unsigned long size )
	{
		EnterCriticalSection( &sect );
		unsigned long   ret = ReadLastDataAct( data, size );
		LeaveCriticalSection( &sect );
		return ret;
	}
	// end VS_AcsLog_Implementation::ReadLastData

	inline int PrintHex( const void *data, const size_t size )
	{
		EnterCriticalSection( &sect );
		const size_t len = 32;
		int   ret = EOF;
		if (!data || !size) return ret;
		size_t rows = size / len;
		size_t moda = size % len;
		const size_t strl = len * 2 + len + 4 + len;
		char str[strl] = {};
		if (EOF != (ret = Puts("\nData Hex Dump :\n"))) {
			for (size_t i = 0; i < rows; ++i) {
				const unsigned char *p = reinterpret_cast<const unsigned char *>(data) + i * len;
				_snprintf(str, strl - 1,
					"%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x "\
					"%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x "\
					"\x20\x20\x20\x20%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
					static_cast<unsigned int>(p[0]),  static_cast<unsigned int>(p[1]),  static_cast<unsigned int>(p[2]),  static_cast<unsigned int>(p[3]),  static_cast<unsigned int>(p[4]),
					static_cast<unsigned int>(p[5]),  static_cast<unsigned int>(p[6]),  static_cast<unsigned int>(p[7]),  static_cast<unsigned int>(p[8]),  static_cast<unsigned int>(p[9]),
					static_cast<unsigned int>(p[10]), static_cast<unsigned int>(p[11]), static_cast<unsigned int>(p[12]), static_cast<unsigned int>(p[13]), static_cast<unsigned int>(p[14]),
					static_cast<unsigned int>(p[15]), static_cast<unsigned int>(p[16]), static_cast<unsigned int>(p[17]), static_cast<unsigned int>(p[18]), static_cast<unsigned int>(p[19]),
					static_cast<unsigned int>(p[20]), static_cast<unsigned int>(p[21]), static_cast<unsigned int>(p[22]), static_cast<unsigned int>(p[23]), static_cast<unsigned int>(p[24]),
					static_cast<unsigned int>(p[25]), static_cast<unsigned int>(p[26]), static_cast<unsigned int>(p[27]), static_cast<unsigned int>(p[28]), static_cast<unsigned int>(p[29]),
					static_cast<unsigned int>(p[30]), static_cast<unsigned int>(p[31]),
					p[0] ? p[0] : ' ', p[1] ? p[1] : ' ', p[2] ? p[2] : ' ', p[3] ? p[3] : ' ', p[4] ? p[4] : ' ', p[5] ? p[5] : ' ', p[6] ? p[6] : ' ', p[7] ? p[7] : ' ', p[8] ? p[8] : ' ',
					p[9] ? p[9] : ' ', p[10] ? p[10] : ' ', p[11] ? p[11] : ' ', p[12] ? p[12] : ' ', p[13] ? p[13] : ' ', p[14] ? p[14] : ' ', p[15] ? p[15] : ' ', p[16] ? p[16] : ' ',
					p[17] ? p[17] : ' ', p[18] ? p[18] : ' ', p[19] ? p[19] : ' ', p[20] ? p[20] : ' ', p[21] ? p[21] : ' ', p[22] ? p[22] : ' ', p[23] ? p[23] : ' ', p[24] ? p[24] : ' ',
					p[25] ? p[25] : ' ', p[26] ? p[26] : ' ', p[27] ? p[27] : ' ', p[28] ? p[28] : ' ', p[29] ? p[29] : ' ', p[30] ? p[30] : ' ', p[31] ? p[31] : ' '
				);
				if (EOF == (ret = Puts(str))) break;
				if (EOF == (ret = Puts("\n"))) break;
			}
		}
		LeaveCriticalSection( &sect );
		return ret;
	}
	// end VS_AcsLog_Implementation::PrintHex
};
// end VS_AcsLog_Implementation struct

VS_AcsLog::VS_AcsLog( const char *endpoint, const unsigned long maxDataSize,
							const unsigned long maxFileSize,
							const char *directory, const char *extension )
{	imp = new VS_AcsLog_Implementation( endpoint, maxDataSize, maxFileSize, directory, extension );
	if (imp && !imp->file) {	delete imp;		imp = 0;	}
}
// end VS_AcsLog::VS_AcsLog

VS_AcsLog::~VS_AcsLog( void ) {		if (imp)	delete imp;		}
// end VS_AcsLog::~VS_AcsLog

int VS_AcsLog::Printf( const char *format, ... )
{
	if (!imp)
		return EOF;

	va_list arg;
	va_start(arg, format);
	auto ret = imp->VPrintf(format, arg);
	va_end(arg);
	return ret;
}
int VS_AcsLog::TPrintf( const char *format, ... )
{
	if (!imp)
		return EOF;

	va_list arg;
	va_start(arg, format);
	auto ret = imp->VTPrintf(format, arg);
	va_end(arg);
	return ret;
}

int VS_AcsLog::CPrintf( const char *format, ... )
{
	if (!imp)
		return EOF;

	va_list arg;
	va_start(arg, format);
	auto ret = imp->VCPrintf(format, arg);
	va_end(arg);
	return ret;
}

int VS_AcsLog::Puts( const char *string )
{	return !imp ? EOF : imp->Puts( string );	}
// end VS_AcsLog::Puts

int VS_AcsLog::CPuts( const char *string )
{	return !imp ? EOF : imp->CPuts( string );	}
// end VS_AcsLog::CPuts

unsigned long VS_AcsLog::GetSize( void ) {	return !imp ? 0 : imp->GetSize();	}
// end VS_AcsLog::GetSize

unsigned long VS_AcsLog::GetData( void *data, const unsigned long size )
{	return !imp ? 0 : imp->GetData( data, size );	}
// end VS_AcsLog::GetData

unsigned long VS_AcsLog::GetData( VS_Container &container )
{	return !imp ? 0 : imp->GetData( container );	}
// end VS_AcsLog::GetData

unsigned long VS_AcsLog::PutData( const void *data, const unsigned long size )
{	return !imp ? 0 : imp->PutData( data, size );	}
// end VS_AcsLog::PutData

unsigned long VS_AcsLog::PutData( VS_Container &container )
{	return !imp ? 0 : imp->PutData( container );	}
// end VS_AcsLog::PutData

unsigned long VS_AcsLog::ReadLastData( void *data, const unsigned long size )
{	return !imp ? 0 : imp->ReadLastData( data, size );	}
// end VS_AcsLog::ReadLastData

int VS_AcsLog::PrintHex( const void *data, const size_t size )
{
	return !imp ? EOF : imp->PrintHex( data, size );
}

VS_AcsSTDOUTLog::VS_AcsSTDOUTLog()
{
	imp = new VS_AcsLog_Implementation(0 , 0, 0, 0, 0,true);
	if (imp && !imp->file) {	delete imp;		imp = 0;	}
}

std::streamoff VS_AcsLog_HandleFileSize(const char *fileName, const char *backupFileName, size_t maxSize)
{
	std::ifstream logFile(fileName, std::ifstream::ate | std::ifstream::binary);
	if (!logFile.is_open())
		return -1;

	std::streamoff fileSize = logFile.tellg();
	logFile.close();

	if (fileSize == -1)
		return -1;

	if (fileSize < maxSize)
		return fileSize;

	remove(backupFileName);
	if (rename(fileName, backupFileName))
		return -1;

	return 0;
}
