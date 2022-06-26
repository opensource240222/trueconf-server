
#ifndef VS_ACS_LOG_H
#define VS_ACS_LOG_H

#include "std-generic/cpplib/VS_Container.h"
#include <iosfwd>

#define   VS_ACS_LOG_TRANSPORT_EXTENSION   "log"
extern const char   VS_AcsTransportExtLog[];  // = VS_ACS_LOG_TRANSPORT_EXTENSION

#define   VS_ACS_LOG_MAX_PRINT_ARGS   16  // This parameter to not modify !!!

class VS_AcsLog
{
protected:
	VS_AcsLog(){imp = 0;}
public:
	VS_AcsLog( const char *endpoint, const unsigned long maxSize = 20000,
					const unsigned long hysSize = 10000, const char *directory = 0,
					const char *extension = VS_AcsTransportExtLog );
	virtual ~VS_AcsLog( void );
	struct VS_AcsLog_Implementation   *imp;

	virtual bool			IsValid( void ) const {		return imp != 0;	}
	virtual int				Printf( const char *format, ... );
	virtual int				TPrintf(const char *format, ... );
	virtual int				CPrintf( const char *format, ... );
	virtual int				Puts( const char *string );
	virtual int				CPuts( const char *string );
	virtual unsigned long	GetSize( void );
	virtual unsigned long	GetData( void *data, const unsigned long size );
	virtual unsigned long	GetData( VS_Container &container );
	virtual unsigned long	PutData( const void *data, const unsigned long size );
	virtual unsigned long	PutData( VS_Container &container );
	virtual unsigned long	ReadLastData( void *data, const unsigned long size );
	virtual int				PrintHex( const void *data, const size_t size );
};
// end VS_AcsLog class
class VS_AcsEmptyLog : public VS_AcsLog
{
public:
	VS_AcsEmptyLog(){imp = 0;}
	virtual bool IsValid( void ) const { return imp == 0;}
};

class VS_AcsSTDOUTLog : public VS_AcsLog
{
public:
	VS_AcsSTDOUTLog();
	virtual ~VS_AcsSTDOUTLog(){};


};

std::streamoff VS_AcsLog_HandleFileSize(const char *fileName, const char *backupFileName, size_t maxSize);

#endif  // VS_ACS_CLIENT_LOG_H
