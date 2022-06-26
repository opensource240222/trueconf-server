/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 20.05.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
/**
 *****************************************************************************************
 * \file VS_SmtpMailerService.h
 * \brief (c) 2002 Visicron Inc.  http://www.visicron.net/ \n
 * \author A.Slavetsky
 * \b Project
 * \version
 * \date 06.06.03
 *
 ******************************************************************************************/

#ifndef VS_SMTP_MAILER_BASE_H
#define VS_SMTP_MAILER_BASE_H

#include <stdlib.h>
#include <sys/stat.h>


#include "transport/Router/VS_TransportRouterServiceBase.h"
#include "transport/Router/VS_PoolThreadsService.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/debuglog/VS_Debug.h"

#define   VS_SMTP_MAILER_SERVICE_NAME   "SMTP_MAILER"
#define		VS_SMTP_UNICODE_FLAG					"UTF8"

//////////////////////////////////////////////////////////////////////////////////////////

class VS_SmtpMailerBase : public virtual VS_TransportRouterServiceBase
{
public:
		VS_SmtpMailerBase( void ) {}
	virtual ~VS_SmtpMailerBase( void ) {}

 	bool   SendSmtpMail( 	const char *to,
							const char *from,
							const char *subject,					// Can be equal to Zero
							const char *text,						// Can be equal to Zero
							const bool isTextHtml = false,
							const char *replyTo = 0,
							const char *cc = 0,
							const char *attachmentFile = 0		);
	VS_RouterMessage* MakeMailMessage( const char *to,
										const char *from,
										const char *subject,
										const char *text,
										const char *replyTo = 0,
										const char *cc=0,
										const char *attachmentFile=0,
										const char* file_ics_utf8 = 0);
	VS_RouterMessage* MakeMailMessage( const wchar_t *to,
										const wchar_t *from,
										const wchar_t *subject,
										const wchar_t *text,
										const wchar_t *replyTo = 0,
										const wchar_t *cc=0,
										const wchar_t *attachmentFile=0,
										const char* file_ics_utf8=0);

	bool   SendSmtpMail(	VS_Container &args	);
};
// end VS_SmtpMailerBase::VS_SmtpMailerBase

//////////////////////////////////////////////////////////////////////////////////////////

#define   VS_MIN_SMTP_MAX_POOL_THREADS			1
#define   VS_MAX_SMTP_MAX_POOL_THREADS			10
#define   VS_MIN_SMTP_CHECK_PERIOD				20
#define   VS_MAX_SMTP_CHECK_PERIOD				3000
#define   VS_MIN_SMTP_EXTERNAL_CHECK_PERIOD		1
#define   VS_MAX_SMTP_EXTERNAL_CHECK_PERIOD		3000
#define   VS_MIN_SMTP_ATTEMPTS_PERIOD			20
#define   VS_MAX_SMTP_ATTEMPTS_PERIOD			3600
#define   VS_MIN_SMTP_LIFETIME					( 60 * 60 )
#define   VS_MAX_SMTP_LIFETIME					( 1440 * 60 )

class VS_SmtpMailerService
	: public VS_TransportRouterServiceBase
{
public:
		VS_SmtpMailerService( const unsigned maxPoolThreads = 3,
								const char *mailedDir = "mlr_lts",
								const unsigned checkPeriodSec = 60,
								const unsigned checkExternalSec = 6,
								const unsigned attemptsPeriodSec = 300,
								const unsigned lettersLifetimeSec = 3600);
	virtual ~VS_SmtpMailerService( void );
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;

private:
	inline void     TestDir(void);
	bool			Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;
	bool			Timer(unsigned long tickcount) override;
	bool			IsChangedDir(time_t cur_time);
	bool			FillPaths(char * out_path);
	bool			isValid;		unsigned short   mark;
	char			*dir;	unsigned   checkPeriodSec, attemptsPeriodSec, lettersLifetimeSec,checkExternalSec;
	char			*server;		int   port;		unsigned   maxThreads, nThreads, nTimers;
	std::string		login, password, from_;
	bool			from_is_valid;
	bool			useSSL;
	int				authType;
	bool			is_dir_changed;
	time_t			change_dir_time;
	class	VS_SmtpMailer	        * FillSmtpValid(class VS_Container * );
	inline void		Write( class VS_SmtpMailer * in, VS_Container &cnt);
	inline void		Write( class VS_SmtpMailer * in, const VS_RouterMessage *mess);
	inline VS_SmtpMailer	*ReWrite( const char *);
};
// end VS_SmtpMailerService::VS_SmtpMailerService

//////////////////////////////////////////////////////////////////////////////////////////
#endif
