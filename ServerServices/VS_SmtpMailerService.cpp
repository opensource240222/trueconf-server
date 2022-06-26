/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 06.06.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
/**
 **************************************************************
 * \file VS_SmtpMailerService.cpp
 * \brief (c) 2002 Visicron Inc.  http://www.visicron.net/ \n
 * \author A.Slavetsky
 * \version
 * \date 06.06.03
 *
 ****************************************************************/
#ifdef _WIN32 // not ported
#include <string>
#include <Windows.h>
#include <comutil.h>
#include <stdio.h>
#include <io.h>
#include <direct.h>
#include <time.h>
#include <errno.h>

#include <sys/types.h>
#include "../common/transport/Router/VS_RouterMessage.h"
#include "../common/std/cpplib/VS_MemoryLeak.h"
#include "../common/SmtpMail/SmtpMailer/VS_SmtpMailer.h"
#include "../common/std/cpplib/VS_RegistryKey.h"
#include "../common/std/cpplib/VS_RegistryConst.h"
#include "../common/std/cpplib/VS_WideStr.h"
#include "std-generic/cpplib/macro_utils.h"
#include "VS_SmtpMailerService.h"


#define   VS_SMTP_MAILER_SEND		"DNES_RELIAM_PTMS_SV"
#define   VS_SMTP_MAILER_END_TASK	"KSAT_DNE_RELIAM_PTMS_SV"

#define   max_path_length  2048
//////////////////////////////////////////////////////////////////////////////////////////
#define DEBUG_CURRENT_MODULE VS_DM_SMTP

class VS_SmtpMailerTask : public VS_PoolThreadsTask
{
	//////////////////////////////////////////////////////////////////////////////////////////

public:
	VS_SmtpMailerTask(VS_SmtpMailer *args, const char *fileName)
		: isValid(true)
		, res(std::make_unique<Result>(args, fileName, nullptr, false))
	{
	}
	// end VS_SmtpMailerTask::VS_SmtpMailerTask


	VS_SmtpMailerTask( VS_SmtpMailer *args, std::unique_ptr<VS_RouterMessage> &&recvMess )
		: isValid(true)
		, res(std::make_unique<Result>(args, "", std::move(recvMess), false))
	{
	}
	// end VS_SmtpMailerTask::VS_SmtpMailerTask
	bool   isValid;
	struct Result
	{
		VS_FORWARDING_CTOR4(Result, args, fileName, recvMess, result) {}
		VS_SmtpMailer* args;
		std::string fileName;
		std::unique_ptr<VS_RouterMessage> recvMess;
		bool result;
	};	// end Result struct
	std::unique_ptr<Result> res;
	//////////////////////////////////////////////////////////////////////////////////////////

	void Run( void )
	{
	if (isValid)
	{


		char buf_to[0xffff] = {0};
		char buf_from[0xffff] = {0};
		char *buf_to_ptr = buf_to;
		char *buf_from_ptr = buf_from;
		res->args->GetFrom(&buf_from_ptr, 0xffff - 1);
		res->args->GetTo(&buf_to_ptr, 0xffff - 1);
		dprint3("SmtpMailerTask: Sending msg: from: %s; to: %s;\n",buf_from,buf_to);

		res->result = res->args->Send();


	}
		const auto res_ptr = res.get();
		VS_RouterMessage   *mes = new VS_RouterMessage( 0, VS_SMTP_MAILER_SERVICE_NAME,
							VS_SMTP_MAILER_END_TASK, 0, VS_SMTP_MAILER_SERVICE_NAME,
							~0, &res_ptr, sizeof(res_ptr), 0, 0, OurEndpoint(), OurEndpoint() );
		if (mes && mes->IsValid())
		{	if (SendMes( mes ))
			{
				mes = 0;
				(void)res.release(); // We have succesfully sent the message, now the message receiver is responsible for deleting the Result.
			}
		}
		else {	/* Here it will be necessary to throw off in TRACE */	}
		if (mes) { delete mes; mes = 0; }
	}
	// end VS_SmtpMailerTask::Run
};
// end VS_SmtpMailerTask class

//////////////////////////////////////////////////////////////////////////////////////////

VS_SmtpMailerService::VS_SmtpMailerService( const unsigned maxPoolThreads,
												const char *mailedDir,
												const unsigned checkPeriodSec,
												const unsigned checkExternalSec,
												const unsigned attemptsPeriodSec,
												const unsigned lettersLifetimeSec )
	:
		isValid(false), mark(0), dir(0), checkPeriodSec(0), attemptsPeriodSec(0),
		lettersLifetimeSec(0), server(0), port(0), maxThreads(maxPoolThreads),
		nThreads(0), nTimers(0), from_is_valid(false), useSSL(false), authType(0)
{
	m_TimeInterval = std::chrono::seconds(3);
	int   i;	const int   max = max_path_length;
	char   *bf = (char *)malloc( max );		if (!bf)	return;
	VS_RegistryKey cfg_root(false, CONFIGURATION_KEY);
	if (!cfg_root.IsValid())
		goto go_ret;
	if (cfg_root.GetValue(bf, max, VS_REG_STRING_VT, SMTP_DIRECTORY_TAG) > 1)
	{	dir = _strdup( bf );		}
	else {	if (!mailedDir)	goto go_ret;	dir = _strdup( mailedDir );	}
	if (!dir)	goto go_ret;
	i = _access( dir, 06 );
	if (i == -1 && errno == ENOENT) {	i = _mkdir( dir );	if (i)	goto go_ret;	}
	VS_SmtpMailerService::checkPeriodSec = checkPeriodSec;
	VS_SmtpMailerService::checkExternalSec = checkExternalSec;
	VS_SmtpMailerService::attemptsPeriodSec = attemptsPeriodSec;
	VS_SmtpMailerService::lettersLifetimeSec = lettersLifetimeSec;

	if (cfg_root.GetValue(bf, max, VS_REG_STRING_VT, SMTP_SERVER_TAG) <= 1)
		goto go_ret;
	server = _strdup( bf );		if (!server)	goto go_ret;
	if (cfg_root.GetValue(bf, max, VS_REG_INTEGER_VT, SMTP_PORT_TAG) <= 0)
		goto go_ret;
	port = *reinterpret_cast<int*>( bf );		if (port < 1 || port > 65535)	goto go_ret;
	if (cfg_root.GetValue(bf, max, VS_REG_STRING_VT, SMTP_LOGIN_TAG) > 1)
		login = bf;
	if (cfg_root.GetValue(bf, max, VS_REG_STRING_VT, SMTP_PASSWORD_TAG) > 1)
		password = bf;
	if (cfg_root.GetValue(bf, max, VS_REG_STRING_VT, SMTP_FROM_TAG) > 1)
		from_ = bf;
	else
	{
		from_ = login;
		char *p = strchr(server, '.'); // escape "smtp.", "mail." or alike
		if (p)
		{
			from_ += "@";
			from_ += p + 1;
		}
	}
	if (from_.find('@') != std::string::npos)
		from_is_valid = true;
	if (cfg_root.GetValue(bf, max, VS_REG_INTEGER_VT, SMTP_USE_SSL_TAG) > 0)
	{
		i = *reinterpret_cast<int*>(bf);
		useSSL = i != 0;
	}
	if (cfg_root.GetValue(bf, max, VS_REG_INTEGER_VT, SMTP_AUTH_TYPE_TAG) > 0)
	{
		i = *reinterpret_cast<int*>(bf);
		if (i == VS_SimplePass || i == VS_NTLM)
			authType = i;
		else
			authType = VS_NoAuth;
	}
	if (cfg_root.GetValue(bf, max, VS_REG_INTEGER_VT, SMTP_MAX_POOL_THREADS_TAG) > 0)
	{	i = *reinterpret_cast<int*>( bf );
		if (i >= VS_MIN_SMTP_MAX_POOL_THREADS && i <= VS_MAX_SMTP_MAX_POOL_THREADS)
			maxThreads = (unsigned)i;
	}
	if (cfg_root.GetValue(bf, max, VS_REG_INTEGER_VT, SMTP_CHECK_PERIOD_TAG) > 0)
	{	i = *reinterpret_cast<int*>( bf );
		if (i >= VS_MIN_SMTP_CHECK_PERIOD && i <= VS_MAX_SMTP_CHECK_PERIOD)
			VS_SmtpMailerService::checkPeriodSec = (unsigned)i;
	}
	if (cfg_root.GetValue(bf, max, VS_REG_INTEGER_VT, SMTP_EXTERNAL_CHECK_PERIOD_TAG) > 0)
	{	i = *reinterpret_cast<int*>( bf );
		if (i >= VS_MIN_SMTP_EXTERNAL_CHECK_PERIOD && i <= VS_MAX_SMTP_EXTERNAL_CHECK_PERIOD)
			VS_SmtpMailerService::checkExternalSec = (unsigned)i;
	}
	if (cfg_root.GetValue(bf, max, VS_REG_INTEGER_VT, SMTP_ATTEMPTS_PERIOD_TAG) > 0)
	{	i = *reinterpret_cast<int*>( bf );
		if (i >= VS_MIN_SMTP_ATTEMPTS_PERIOD && i <= VS_MAX_SMTP_ATTEMPTS_PERIOD)
			VS_SmtpMailerService::attemptsPeriodSec = (unsigned)i;
	}
	if (cfg_root.GetValue(bf, max, VS_REG_INTEGER_VT, SMTP_LIFETIME_TAG) > 0)
	{	i = *reinterpret_cast<int*>( bf );
		if (i >= VS_MIN_SMTP_LIFETIME && i <= VS_MAX_SMTP_LIFETIME)
			VS_SmtpMailerService::lettersLifetimeSec = (unsigned)i;
	}
	isValid = true;
go_ret:
	if (bf) { free( (void *)bf ); bf = 0; }
}
// end VS_SmtpMailerService::VS_SmtpMailerService

VS_SmtpMailerService::~VS_SmtpMailerService( void )
{
	if (server) { free( (void *)server ); server = 0; }
	if (dir) { free( (void *)dir ); dir = 0; }
}
// end VS_SmtpMailerService::~VS_SmtpMailerService

static char   str_from[] = "From:", str_to[] = "To:", str_cc[] = "Cc:",
				str_subject[] = "Subject:", str_attachment[] = "Attachment:",
				str_ishtml[] = "IsHTML:", str_date[] = "Date:",
				str_ishtml_true[] = "TRUE", str_ishtml_false[] = "FALSE";
const unsigned max_name_size  = 128;
const unsigned max_value_size = 256;
const unsigned max_buff_size  = 10000;
const unsigned max_file_size  = 256;
const unsigned check_dir_period = 1;


/////////////////////////////////////////////////////////////////////////////////////

inline void VS_SmtpMailerService::Write( VS_SmtpMailer *args,  VS_Container &cnt)
{
	if (args->IsSendErrorCritical()) return;
	char * name=0;
    char * to = 0;
	if (!(name =new char[max_path_length])) return;
	if (!(to=new char[100]))
	{
		if (name) { delete [] name; name = 0; }
		return;
	}
	if (!(args->GetTo(&to,99)))
	{
		if (to) { delete [] to; to = 0; }
		if (name) { delete [] name; name = 0; }
		return;
	}
	to[99]='\0';
		/// для получения информации о изменениях директории извне
	if (!++mark)	++mark;
	char seps[]   = {'"','@','.','<','>'};
	char *token;
	token = strtok( to, seps );
	if(token) token = strtok( NULL, seps );
	// 2-// 4-mark 10 - запас
	if ((!token) || (2+4+strlen(dir)+strlen(token)+10 > max_path_length) || !cnt.IsValid())
	{
		if (to) { delete [] to; to = 0; }
		if (name) { delete [] name; name = 0; }
		return;
	}
	///////////////////////
	sprintf( name, "%s\\%s_%04X.txt", dir,token,  mark );
	is_dir_changed |= IsChangedDir( change_dir_time );
	///args->SaveToFile(name);

	void *buf(0);
	size_t sz(0);
	cnt.SerializeAlloc(buf,sz);
	FILE * fd = fopen(name,"wb");
	if(sz && fd )
	{
		fwrite(buf,1,sz,fd);
		fflush(fd);
	}
	if(fd)
		fclose(fd);
	if(buf)
	{
		free(buf);
		sz = 0;
	}


	change_dir_time = time( 0 );
	if (to) { delete [] to; to = 0; }
	if (name) { delete [] name; name = 0; }
	return;
}
inline void VS_SmtpMailerService::Write(VS_SmtpMailer *args,  const VS_RouterMessage *mess)
{
	if (args->IsSendErrorCritical()) return;
	char * name=0;
    char * to = 0;
	if (!(name =new char[max_path_length])) return;
	if (!(to=new char[100]))
	{
		if (name) { delete [] name; name = 0; }
		return;
	}
	if (!(args->GetTo(&to,99)))
	{
		if (to) { delete [] to; to = 0; }
		if (name) { delete [] name; name = 0; }
		return;
	}
	to[99]='\0';
		/// для получения информации о изменениях директории извне
	if (!++mark)	++mark;
	char seps[]   = {'"','@','.','<','>'};
	char *token;
	token = strtok( to, seps );
	if(token) token = strtok( NULL, seps );
	// 2-// 4-mark 10 - запас
	if ((!token) || (2+4+strlen(dir)+strlen(token)+10 > max_path_length) || !mess->IsValid())
	{
		if (to) { delete [] to; to = 0; }
		if (name) { delete [] name; name = 0; }
		return;
	}
	///////////////////////
	sprintf( name, "%s\\%s_%04X.txt", dir,token,  mark );
	is_dir_changed |= IsChangedDir( change_dir_time );
	///args->SaveToFile(name);

	const auto buf = mess->Body();
	const auto sz = mess->BodySize();

	FILE * fd = fopen(name,"wb");
	if(sz && fd )
	{
		fwrite(buf,1,sz,fd);
		fflush(fd);
	}
	if(fd)
		fclose(fd);

	change_dir_time = time( 0 );
	if (to) { delete [] to; to = 0; }
	if (name) { delete [] name; name = 0; }
	return;
}
// end VS_SmtpMailerService::Write
/////////////////////////////////////////////////////////////////////////////////////
inline VS_SmtpMailer *VS_SmtpMailerService::ReWrite( const char *file)
{
	HANDLE   h = INVALID_HANDLE_VALUE;
	FILETIME   acTm, lwTm , crTm;
	VS_SmtpMailer * args =0;
	/*if(!(args=new VS_SmtpMailer(server,port)))
		goto go_ret;*/
	is_dir_changed |= IsChangedDir( change_dir_time );

/**
	Прочитать контейнер и заполнить args
*/

	VS_Container	cnt;

	FILE * fd = fopen(file,"rb");
	long f_size(0);
	if(fd && !fseek(fd,0,SEEK_END) &&
		(-1 != (f_size = ftell(fd))) && !fseek(fd,0,SEEK_SET))
	{
		void *buf = new char[f_size];
		fread(buf,f_size,1,fd);
		fclose(fd);
		cnt.Clear();
		cnt.Deserialize(buf,f_size);
		delete [] buf;
		if(!(args = FillSmtpValid(&cnt)))
			goto go_ret;
	}


	//////args->LoadFromFile(file);
	/////////////////time
	h = CreateFile( file, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0 );
	if (h == INVALID_HANDLE_VALUE || !GetFileTime( h, &crTm, &acTm, &lwTm ))
		goto go_ret;
	acTm = lwTm;
	GetSystemTimeAsFileTime( &lwTm );
	acTm.dwHighDateTime=0x019E8ac0;////After 1970 year
	acTm.dwLowDateTime=0x8f418000;////  !!!
	if (!SetFileTime( h, &crTm , &acTm, &lwTm ))
		goto go_ret;
	CloseHandle( h );
	h = INVALID_HANDLE_VALUE;
	/////////////////time end
go_ret:
	if (h != INVALID_HANDLE_VALUE)
		CloseHandle( h );
	change_dir_time = time( 0 );/// для получения информации о изменениях директории извне
	return args;
}
// end VS_SmtpMailerService::ReWrite
/////////////////////////////////////////////////////////////////////////////////////
VS_SmtpMailer *VS_SmtpMailerService::FillSmtpValid(VS_Container * ar)
{
	VS_SmtpMailer * ret=0;
	if (!(ret = new VS_SmtpMailer( server , port, static_cast<VS_SmtpAuthType>(authType), login.c_str(), password.c_str(), useSSL )))
		return NULL;//Утечка, если письмо не отправили сразу (если все задачи заняты)
    const char * buff=0;
	const char * buff1=0;

	bool unicode=false;
	ar->GetValue(VS_SMTP_UNICODE_FLAG,unicode);
	VS_WideStr wbuff;
	//////////////////////////////////main fields////////////////////////

	if (!(buff=ar->GetStrValueRef("To")))
	{
		if (ret) { delete ret; ret = 0; }
		return NULL;
	}
	if ( !buff || !*buff || !strstr(buff, "@"))
	{
		if (ret) { delete ret; ret = 0; }
		return NULL;
	}

	if(unicode)
	{ wbuff.AssignUTF8(buff);
		ret->To( wbuff );	}
	else
		ret->To( buff );
	buff=0;

	if (from_is_valid)
	{
		if (unicode)
		{
			wbuff.AssignUTF8(from_.c_str());
			ret->From(wbuff);
		}
		else
			ret->From(from_.c_str());
	}
	else
	{
		if (!(buff = ar->GetStrValueRef("From")))
		{
			if (ret) { delete ret; ret = 0; }
			return NULL;
		}
		if (!buff || !*buff || !strstr(buff, "@"))
		{
			if (ret) { delete ret; ret = 0; }
			return NULL;
		}

		if (unicode)
		{
			wbuff.AssignUTF8(buff);
			ret->From(wbuff);
		}
		else
			ret->From(buff);
		buff = 0;
	}

	if (buff = ar->GetStrValueRef("Reply-To"))
	{
		if (*buff && strstr(buff, "@"))
		{
			if (unicode)
			{
				wbuff.AssignUTF8(buff);
				ret->ReplyTo(wbuff);
			}
			else
				ret->ReplyTo(buff);
		}
	}
	buff = 0;

	if (buff=ar->GetStrValueRef("Subject"))
	{
		if(unicode)
		{ wbuff.AssignUTF8(buff);
			ret->Subject( wbuff );	}
		else
			ret->Subject( buff );
	}
	buff=0;

	if (buff=ar->GetStrValueRef("Body"))
	{
		if(unicode)
		{ wbuff.AssignUTF8(buff);
			ret->SetTextBody( wbuff );	}
		else
			ret->SetTextBody( buff );
	}
	buff=0;

	if (buff=ar->GetStrValueRef("AttachmentManual"))
	{
		ret->AddAttachmentManual(buff);
	}
	buff=0;
	/////////////////////////////////////////////////////////////////////

	ar->Reset();
	while(ar->Next())
	{
		const char* name=ar->GetName();
		if(
			(_stricmp(name,"To")  !=0) &&
			(_stricmp(name,"From")!=0) &&
			(_stricmp(name,"Subject")!=0) &&
			(_stricmp(name,"Body")!=0)	&&
			(_stricmp(name,"Reply-To")!=0) &&
			(_stricmp(name,VS_SMTP_UNICODE_FLAG)!=0) &&
			(_stricmp(name,"AttachmentManual")!=0)
		)
		{
			buff = ar->GetStrValueRef(name);
			if(unicode)
			{ wbuff.AssignUTF8(buff);
				ret->AddField( name,(wchar_t*)wbuff ); }
			else
			ret->AddField( name,buff );
	}	}
	return ret;
}
//	end VS_SmtpMailerService::FillSmtpValid

bool VS_SmtpMailerService::Init(const char* /*our_endpoint*/, const char* /*our_service*/, const bool /*permittedAll*/)
{
	return true;
}

bool VS_SmtpMailerService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess )
{
	const char   *topic = recvMess->AddString();
	is_dir_changed |= IsChangedDir( change_dir_time );
	if (!topic) {	/* Here it will be necessary to throw off in TRACE */	}
	else if (!strcmp( VS_SMTP_MAILER_SEND, topic ))
	{
		dprint3("method VS_SMTP_MAILER_SEND;\n");
		VS_Container args;

		args.Clear();
		VS_SmtpMailer * smtp=0;
		args.Deserialize(recvMess->Body(), recvMess->BodySize());
		///////////////
		if (recvMess->BodySize() && (smtp = FillSmtpValid(&args)))
		{
			char buf_to[0xffff] = {0};
			char buf_from[0xffff] = {0};
			char *buf_to_ptr = buf_to;
			char *buf_from_ptr = buf_from;
			smtp->GetFrom(&buf_from_ptr,0xffff - 1);
			smtp->GetTo(&buf_to_ptr,0xffff - 1);
			dprint3("smtp_mailer prepared, from: %s; to: %s;\n",buf_from, buf_to);
			if  (nThreads < maxThreads)
			{
				///контейнер в месаге
				VS_SmtpMailerTask   *task = new VS_SmtpMailerTask( smtp, std::move(recvMess) );
				if (task && task->IsValid())
				{
					if (PutTask(task, VS_SMTP_MAILER_SERVICE_NAME))
					{
						recvMess = 0;
						++nThreads;
					}
					else
						recvMess = std::move(task->res->recvMess);
				} else
				{	/* Here it will be necessary to throw off in TRACE */	}
				if (recvMess)
				{
					if (task)
					{
						delete task;
						task = 0;
					}
					Write( smtp, recvMess.get() );
					if(smtp)
						delete smtp;
				}
			}
			else
			{
				Write( smtp, recvMess.get() );
				if(smtp)
					delete smtp;
			}
		} else {	/* Here it will be necessary to throw off in TRACE */	}


	}else if (!strcmp( VS_SMTP_MAILER_END_TASK, topic ))
	{
		dprint3("method VS_SMTP_MAILER_END_TASK;\n");
		const void* body = recvMess->Body();
		const auto sz = recvMess->BodySize();
		if (sz == sizeof(VS_SmtpMailerTask::Result*))
		{
			std::unique_ptr<VS_SmtpMailerTask::Result> res(*static_cast<VS_SmtpMailerTask::Result* const *>(body));
			if (res->result)
			{
				if (!res->fileName.empty())
				{
					is_dir_changed |= IsChangedDir( change_dir_time );
					/// для получения информации о изменениях директории извне
					remove(res->fileName.c_str());
					change_dir_time = time( 0 );
					/// для получения информации о изменениях директории извне
				}
			}
			else
			{
				if (res->recvMess)
					Write(res->args, res->recvMess.get());
			}
			if (res->args)
			{
				delete res->args;
				res->args = 0;
			}
			if (nThreads)
				--nThreads;
		} else
		{	/* Here it will be necessary to throw off in TRACE */	}
	} else
	{	/* Here it will be necessary to throw off in TRACE */	}
	if (is_dir_changed)
		TestDir();
	return true;
}
// end VS_SmtpMailerService::Processing
/////////////////////////////////////////////////////////////////////////////////////////
void VS_SmtpMailerService::TestDir()
{
	//директория успела измениться - принять меры
	char * next_path = new char[max_path_length];
	bool flag;
	while((flag = FillPaths(next_path)) && (maxThreads - nThreads))
	{
		VS_SmtpMailer   *args = ReWrite( next_path );
		if (!args)	{	remove( next_path );	change_dir_time = time( 0 );	}
		else
		{

			VS_SmtpMailerTask   *task = new VS_SmtpMailerTask( args, next_path );
			if (task && task->IsValid())
			{
				if (PutTask( task, VS_SMTP_MAILER_SERVICE_NAME ))
				{	args = 0;	task = 0;		}
			}
			if (task) { delete task; task = 0; }
			if (args) { delete args; args = 0; }
		}
		memset(next_path,0,max_path_length);
		nThreads++;
	}
	if (next_path) { delete [] next_path; next_path = 0; }
	is_dir_changed = flag;
	return;
}
// end VS_SmtpMailerService::TestDir()
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_SmtpMailerService::Timer( unsigned long tickcount )
{
	if ((nTimers/checkExternalSec)*checkExternalSec == nTimers)
	{
		is_dir_changed |= IsChangedDir( change_dir_time );
	}
	if (is_dir_changed)
		TestDir();
	if (++nTimers <= checkPeriodSec)
		return true;
	nTimers = 0;
	char   *dir = (char *)malloc( 512 ), *path = (char *)malloc( max_path_length ), *attemptsBuf = 0;
	if (!dir || !path)
		goto go_ret;

	{
		char   **attemptPaths = 0;
		unsigned   indexThreads = 0;
		const unsigned   possibleThreads = maxThreads - nThreads;
		if (possibleThreads)
		{
			const size_t   points_sz = sizeof(char *) * possibleThreads,
							arrays_sz = max_path_length * possibleThreads, sz = points_sz + arrays_sz;
			attemptsBuf = (char *)malloc( sz );
			if (!attemptsBuf)
				goto go_ret;
			memset( (void *)attemptsBuf, 0, sz );	attemptPaths = (char **)attemptsBuf;
			attemptPaths[0] = attemptsBuf + points_sz;
			for (unsigned i = 1; i < possibleThreads; ++i)
				attemptPaths[i] = attemptPaths[i - 1] + max_path_length;
		}
		WIN32_FIND_DATA   fileData;		HANDLE   hSearch;
		if(VS_SmtpMailerService::dir)
			sprintf( dir, "%s\\", VS_SmtpMailerService::dir );
		sprintf( path, "%s*.txt", dir );
		hSearch = FindFirstFile( path, &fileData );
		if (hSearch != INVALID_HANDLE_VALUE)
		{
			const time_t   tm = time( 0 );
			struct _stat   st;
			do
			{
				if(!(strlen(fileData.cFileName)+strlen(dir)+10 > max_path_length))
				{
					sprintf( path, "%s%s", dir, fileData.cFileName );
					if (_stat( path, &st ))
					{	/* Here it will be necessary to throw off in TRACE */	break;	}
					if ((unsigned)(tm - st.st_ctime) > lettersLifetimeSec)
					{
						remove( path );
						change_dir_time = time( 0 );
						continue;
					}
					if (possibleThreads && (unsigned)(tm - st.st_mtime) > attemptsPeriodSec)
					{
						strcpy( attemptPaths[indexThreads], path );
						if (++indexThreads >= possibleThreads)
							indexThreads = 0;
					}
				}
			}
			while (FindNextFile( hSearch, &fileData ));
			FindClose( hSearch );
			if (possibleThreads)
			{
				for (unsigned i = 0; i < possibleThreads; ++i)
				{
					if (attemptPaths[i] && *(attemptPaths[i]))
					{
						VS_SmtpMailer	*args = ReWrite( attemptPaths[i] );
						if (!args)
						{
							remove( attemptPaths[i] );
							change_dir_time = time( 0 );
						}
						else
						{
							VS_SmtpMailerTask   *task = new VS_SmtpMailerTask( args, attemptPaths[i] );
							if (task && task->IsValid())
							{
								if (PutTask( task, VS_SMTP_MAILER_SERVICE_NAME ))
								{
									args = 0;	task = 0;	++nThreads;
								}
							}
							if (task)
							{
								delete task;
								task = 0;
							}
							if (args)
							{
								delete args;
								args = 0;
							}
						}
					}
				}
			}
		}
	}
go_ret:
	if (attemptsBuf)
	{
		free( (void *)attemptsBuf );
		attemptsBuf = 0;
	}
	if (dir)
	{
		free( (void *)dir );
		dir = 0;
	}
	if (path)
	{
		free( (void *)path );
		path = 0;
	}
	return true;
}
// end VS_SmtpMailerService::Timer
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_SmtpMailerService::IsChangedDir(time_t cur_time)
{
	struct _stat   st;
	if(!(VS_SmtpMailerService::dir))  return false;
	_stat(VS_SmtpMailerService::dir,&st);
	time_t l = st.st_mtime;
	return (l >= cur_time )?true:false;
}
// end VS_SmtpMailerService::Timer
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_SmtpMailerService::FillPaths(char * out_path)
{
	if (!out_path || (!VS_SmtpMailerService::dir)) return false;
	char * dir;
	char *path;
	dir = new char[max_path_length];
	path = new char[max_path_length];
	bool isFilled=false;
	if (dir && path)
	{
		WIN32_FIND_DATA   fileData;		HANDLE   hSearch;
		if(VS_SmtpMailerService::dir) sprintf( dir, "%s\\", VS_SmtpMailerService::dir );
		sprintf( path, "%s*.txt", dir );
		hSearch = FindFirstFile( path, &fileData );
		if (hSearch != INVALID_HANDLE_VALUE)
		{	const time_t   tm = time( 0 );		struct _stat   st;
			do
			{	if(!(strlen(fileData.cFileName)+strlen(dir)+10 > max_path_length))
				{
					sprintf( path, "%s%s", dir, fileData.cFileName );
					if (_stat( path, &st ))
					{	/* Here it will be necessary to throw off in TRACE */	break;	}
					if(st.st_atime!=-1)
					{
						if ((unsigned)(tm - st.st_ctime) > lettersLifetimeSec)
						{	remove( path );		change_dir_time = time( 0 );	continue;	}
						if ((unsigned)(tm - st.st_mtime) > attemptsPeriodSec)
						{
							strcpy( out_path , path );
							isFilled = true;
				}	}	}	}
			while ((!isFilled) && FindNextFile( hSearch, &fileData ));		FindClose( hSearch );
		}
	}
	if(dir) { delete [] dir; dir = 0; }
	if(path) { delete [] path; path = 0; }
	return (isFilled ? true : false);
}
// end VS_SmtpMailerService::Timer

//////////////////////////////////////////////////////////////////////////////////////////

bool VS_SmtpMailerBase::SendSmtpMail( const char *to,
										const char *from,
										const char *subject,
										const char *text,
										const bool isTextHtml,
										const char *replyTo,
										const char *cc,
										const char *attachmentFile )
{
	VS_Container   args;
	args.Clear();
	if (to) args.AddValue("To",to);
	else return false;
	if (from) args.AddValue("From",from);
	else return false;
	if (subject) args.AddValue("Subject",subject);
	if (text) args.AddValue("Body",text);
	if (replyTo) args.AddValue("Reply-To", replyTo);
	if (cc) args.AddValue("Cc",cc);
	if (attachmentFile) args.AddValue("Attachmentfile",attachmentFile);
	return SendSmtpMail( args );
}
VS_RouterMessage* VS_SmtpMailerBase::MakeMailMessage( const wchar_t *to,
										const wchar_t* from,
										const wchar_t* subject,
										const wchar_t *text,
										const wchar_t *replyTo,
										const wchar_t *cc,
										const wchar_t *attachmentFile,
										const char* file_ics_utf8)
{
	VS_Container   args;
	args.Clear();
	args.AddValue(VS_SMTP_UNICODE_FLAG,true);
	if (to) args.AddValue("To",to);
	else return false;
	if (from) args.AddValue("From",from);
	else return false;
	if (subject) args.AddValue("Subject",subject);
	if (text) args.AddValue("Body",text);
	if (replyTo) args.AddValue("Reply-To", replyTo);
	if (cc) args.AddValue("Cc",cc);
	if (attachmentFile) args.AddValue("Attachmentfile",attachmentFile);
	if (file_ics_utf8) args.AddValue("AttachmentManual",file_ics_utf8);
	void* buffer = nullptr;
	size_t size = 0;
	if (!args.SerializeAlloc( buffer, size )) 		return false;
	/*
	VS_RouterMessage   *mail = new VS_RouterMessage( OurEndpoint(), VS_SMTP_MAILER_SERVICE_NAME, VS_SMTP_MAILER_SEND,
														OurEndpoint(), VS_SMTP_MAILER_SERVICE_NAME, ~0,
														(const void *)buffer, (const unsigned long)size );
	*/
	VS_RouterMessage   *mail = new VS_RouterMessage( 0, VS_SMTP_MAILER_SERVICE_NAME, VS_SMTP_MAILER_SEND,
														0, VS_SMTP_MAILER_SERVICE_NAME, ~0,
														(const void *)buffer, (const unsigned long)size, 0, 0, OurEndpoint(), OurEndpoint() );
	if (buffer) { free(buffer); buffer = 0; }
	return mail;
}

// end VS_SmtpMailerBase::SendSmtpMail
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_SmtpMailerBase::SendSmtpMail( VS_Container &args )
{

	void* buffer = nullptr;
	size_t size = 0;
	if (!args.SerializeAlloc( buffer, size )) 		return false;
	/*
	VS_RouterMessage   *mail = new VS_RouterMessage( OurEndpoint(), VS_SMTP_MAILER_SERVICE_NAME, VS_SMTP_MAILER_SEND,
														OurEndpoint(), VS_SMTP_MAILER_SERVICE_NAME, ~0,
														(const void *)buffer, (const unsigned long)size );
	*/
	VS_RouterMessage   *mail = new VS_RouterMessage( 0, VS_SMTP_MAILER_SERVICE_NAME, VS_SMTP_MAILER_SEND,
														0, VS_SMTP_MAILER_SERVICE_NAME, ~0,
														(const void *)buffer, (const unsigned long)size, 0, 0, OurEndpoint(), OurEndpoint() );
	if (buffer) { free(buffer); buffer = 0; }
	if(!PostMes( mail ))
	{
		if (mail) { delete mail; mail = 0; }
		return false;
	}
	return true;
}
VS_RouterMessage* VS_SmtpMailerBase::MakeMailMessage( const char *to,
										const char *from,
										const char *subject,
										const char *text,
										const char *replyTo,
										const char *cc,
										const char *attachmentFile,
										const char* file_ics_utf8)
{
	VS_Container   args;
	args.Clear();
	args.AddValue(VS_SMTP_UNICODE_FLAG, true);
	if (to) args.AddValue("To",to);
	else return false;
	if (from) args.AddValue("From",from);
	else return false;
	if (subject) args.AddValue("Subject",subject);
	if (text) args.AddValue("Body",text);
	if (replyTo) args.AddValue("Reply-To", replyTo);
	if (cc) args.AddValue("Cc",cc);
	if (attachmentFile) args.AddValue("Attachmentfile",attachmentFile);
	if (file_ics_utf8) args.AddValue("AttachmentManual", file_ics_utf8);
	void* buffer = nullptr;
	size_t size = 0;
	if (!args.SerializeAlloc( buffer, size )) 		return false;
	/*
	VS_RouterMessage   *mail = new VS_RouterMessage( OurEndpoint(), VS_SMTP_MAILER_SERVICE_NAME, VS_SMTP_MAILER_SEND,
														OurEndpoint(), VS_SMTP_MAILER_SERVICE_NAME, ~0,
														(const void *)buffer, (const unsigned long)size );
	*/
	VS_RouterMessage   *mail = new VS_RouterMessage( 0, VS_SMTP_MAILER_SERVICE_NAME, VS_SMTP_MAILER_SEND,
														0, VS_SMTP_MAILER_SERVICE_NAME, ~0,
														(const void *)buffer, (const unsigned long)size, 0, 0, OurEndpoint(), OurEndpoint() );
	if (buffer) { free(buffer); buffer = 0; }
	return mail;
}
//////////////////////////////////////////////////////////////////////////////////////////
#endif