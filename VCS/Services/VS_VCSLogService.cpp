/**
 ****************************************************************************
 * Project: Base Server
 *
 * \file VS_LogService.cpp
 *
 * Log Service definition file
 *
 ****************************************************************************/

#include "VS_VCSLogService.h"
#include "../../BaseServer/Services/storage/VS_DBStorageInterface.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "../../common/std/cpplib/VS_Replace.h"
#include "../../common/std/cpplib/VS_MemoryLeak.h"
#include "../../common/transport/Router/VS_RouterMessage.h"

#ifdef HAVE_CPPDB
#include "std-generic/cpplib/VS_CppDBIncludes.h"
#include "std-generic/cpplib/VS_CppDBLibs.h"
#endif

#define DEBUG_CURRENT_MODULE VS_DM_LOGSERVICE

class VS_VCSLogService::EventLogger
{
public:
	static std::unique_ptr<EventLogger> Create(const std::string& config) try
	{
		return std::unique_ptr<EventLogger>(new EventLogger(config));
	}
	catch (const std::exception& e)
	{
		dstream3 << "VS_VCSLogService::EventLogger: Init failed: " << e.what();
		return nullptr;
	}

	void LogEvent(VS_Container&& cnt) const;

private:
	EventLogger(const std::string& config);

#if defined(HAVE_CPPDB)
	cppdb::pool::pointer m_pool;
#endif
};


#if defined(HAVE_CPPDB)

VS_VCSLogService::EventLogger::EventLogger(const std::string& config)
	: m_pool(cppdb::pool::create(config))
{
}

void VS_VCSLogService::EventLogger::LogEvent(VS_Container&& cnt) const try
{
	auto object_type = cnt.GetStrValueRef(OBJECT_TYPE_PARAM);
	auto object_name = cnt.GetStrValueRef(OBJECT_NAME_PARAM);
	auto event_type = cnt.GetStrValueRef(EVENT_TYPE_PARAM);
	auto payload = cnt.GetStrValueRef(PAYLOAD_PARAM);

	if (!object_type || !*object_type ||
		!object_name || !*object_name ||
		!event_type  || !*event_type  ||
		!payload     || !*payload)
		return;

	cppdb::session session(m_pool->open());
	if (!session.is_open())
		return;

	auto statement = session.create_statement(
		"select * from log.add_event("
		"\"_object_type\" => ?, "
		"\"_object_name\" => ?, "
		"\"_event_type\" => ?, "
		"\"_payload\" => ?);"
	);

	statement.bind(1, object_type);
	statement.bind(2, object_name);
	statement.bind(3, event_type);
	statement.bind(4, payload);

	auto res = statement.query();
}
catch (const cppdb::cppdb_error& e)
{
	dstream3 << "VS_VCSLogService::EventLogger: LogEvent failed: " << e.what();
	return;
}

#else

VS_VCSLogService::EventLogger::EventLogger(const std::string&)
{
}

void VS_VCSLogService::EventLogger::LogEvent(VS_Container&& cnt) const
{
}

#endif

////////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
////////////////////////////////////////////////////////////////////////////////
VS_VCSLogService::VS_VCSLogService(void)
{

}

VS_VCSLogService::~VS_VCSLogService()
{

}

bool VS_VCSLogService::Init(const char *our_endpoint, const char *our_service, const bool permittedAll)
{
	VS_Container log_cnt;
	log_cnt.AddValue(OBJECT_TYPE_PARAM, SERVER_OBJECT_TYPE);
	log_cnt.AddValue(OBJECT_NAME_PARAM, our_endpoint);
	log_cnt.AddValue(EVENT_TYPE_PARAM, START_SERVER_EVENT_TYPE);
	log_cnt.AddValue(PAYLOAD_PARAM, "{}");
	if (m_ev_log)
		m_ev_log->LogEvent(std::move(log_cnt));
	return VS_LogServiceBase::Init(our_endpoint, our_service, permittedAll);
}

void VS_VCSLogService::AsyncDestroy()
{
	VS_Container log_cnt;
	log_cnt.AddValue(OBJECT_TYPE_PARAM, SERVER_OBJECT_TYPE);
	log_cnt.AddValue(OBJECT_NAME_PARAM, OurEndpoint());
	log_cnt.AddValue(EVENT_TYPE_PARAM, STOP_SERVER_EVENT_TYPE);
	log_cnt.AddValue(PAYLOAD_PARAM, "{}");
	if (m_ev_log)
		m_ev_log->LogEvent(std::move(log_cnt));
	return VS_LogServiceBase::AsyncDestroy();
}

bool VS_VCSLogService::InitEventStorage(const std::string& config)
{
#if defined(HAVE_CPPDB)
	m_ev_log = EventLogger::Create(config);
	return m_ev_log != nullptr;
#else
	return true;
#endif
}

bool VS_VCSLogService::Processing(std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (m_ev_log && recvMess)
	{
		const auto bodySize = recvMess->BodySize();
		const auto body = recvMess->Body();
		VS_Container cnt;
		if (bodySize && body && cnt.Deserialize(body, bodySize))
			m_ev_log->LogEvent(std::move(cnt));
	}
	return VS_LogServiceBase::Processing(std::move(recvMess));
}

#ifdef _WIN32		// no emails at linux server
void FillMailTemplate(std::string &OUTtoFill, const char* fromDN, const char* fromId, const char* missed_call_time_str, const char* toDn, const char* toId, const char* app_name,const char* to_email) {
	VS_ReplaceAll(OUTtoFill, "%caller_display_name", fromDN != nullptr ? fromDN : "");
	VS_ReplaceAll(OUTtoFill, "%caller_call_id", fromId != nullptr ? fromId : "");
	VS_ReplaceAll(OUTtoFill, "%missed_call_time", missed_call_time_str != nullptr? missed_call_time_str: "");
	VS_ReplaceAll(OUTtoFill, "%recipient_display_name", toDn != nullptr ? toDn : "");
	VS_ReplaceAll(OUTtoFill, "%recipient_call_id", toId != nullptr ? toId : "");
	VS_ReplaceAll(OUTtoFill, "%application", app_name != nullptr ? app_name : "");
	VS_ReplaceAll(OUTtoFill, "%subscribed_email", to_email != nullptr ? to_email : "");
}

void VS_VCSLogService::MissedCallMail(const std::chrono::system_clock::time_point  missed_call_time, const char *app_name, const char* fromId, const char* fromDn, const char* toId, const char* toDn)
{

	/**
		%key md5 email+secretS
	*/
	VS_SimpleStr	to_email;
	VS_SimpleStr	from_email;
	std::string		subj;
	std::string		body;
	std::string		fromDN;
	std::string		toDN;
	dprint3("MissedCallMail fromEm = %s; toEm = %s;\n",fromId, toId);

	auto dbStorage = g_dbStorage;
	if (fromDn) fromDN = fromDn;
	if (toDn) toDN = toDn;

	if(!!dbStorage && dbStorage->GetMissedCallMailTemplate(missed_call_time,fromId, fromDN,toId, toDN,from_email,to_email,subj,body))
	{
		/**
caller_display_name - имя звонившего
2) caller_call_id - e-mail звонившего
3) missed_call_time - дата и время пропущенного звонка
4) recipient_display_name - имя получателя
5) recipient_call_id - e-mail получателя
6) application
		*/

		char missed_call_time_str[0xff] = { 0 };
		if (tu::TimeToGStr(missed_call_time, missed_call_time_str, 0xff) == 0) return;

		FillMailTemplate(body, fromDN.c_str(), fromId, missed_call_time_str, toDN.c_str(), toId, app_name, to_email);
		FillMailTemplate(subj, fromDN.c_str(), fromId, missed_call_time_str, toDN.c_str(), toId, app_name, to_email);

		ReplaceGuestUrl(body);

		if(!from_email)
		{
			if(fromId)
				from_email = VS_RealUserLogin(fromId).GetID();
			else
				from_email = VS_RealUserLogin("administrator").GetID();
		}

		///application
		dstream3 << "MissedCallMail: MakeMailMessage To:" << to_email.m_str << " From:" << from_email.m_str << " Subj:" << subj << "\n";
		VS_RouterMessage	*mess = MakeMailMessage(to_email, from_email, subj.c_str(), body.c_str(), from_email);
		if(mess)
		{
			if(!PostMes(mess))
				delete mess;
		}
	}
}

void VS_VCSLogService::InviteMail(const std::chrono::system_clock::time_point missed_call_time,const char *app_name,const char *fromId, const char *fromDn, const char *toId)
{
	VS_SimpleStr	email_to;
	VS_SimpleStr	email_from;
	std::string		subj;
	std::string		body;
	std::string		fromDN;
	dprint3("InviteMail fromEm = %s; toEm = %s;\n",fromId, toId);
	auto dbStorage = g_dbStorage;
	if (fromDn) fromDN = fromDn;
	if(!!dbStorage && dbStorage->GetInviteCallMailTemplate(missed_call_time,fromId, fromDN,toId, email_from,email_to,subj,body))
	{
		char missed_call_time_str[0xff];
		if (tu::TimeToGStr(missed_call_time, missed_call_time_str, 0xff) == 0) return;

		FillMailTemplate(body, fromDN.c_str(), fromId, missed_call_time_str, nullptr, toId, app_name, email_to);
		FillMailTemplate(subj, fromDN.c_str(), fromId, missed_call_time_str, nullptr, toId, app_name, email_to);

		ReplaceGuestUrl(body);

		///application
		dstream3 << "InviteMail: MakeMailMessage To:" << email_to.m_str << " From:" << email_from.m_str << " Subj:" << subj << "\n";
		VS_RouterMessage	*mess = MakeMailMessage(email_to, email_from, subj.c_str(), body.c_str(), email_from);
		if(mess)
		{
			if(!PostMes(mess))
				delete mess;
		}
	}
}

void VS_VCSLogService::MultiInviteMail(const std::chrono::system_clock::time_point missed_call_time, const char*app_name,const char* fromId, const char* fromDn, const char* toId, const char* toDn)
{
	VS_SimpleStr	to_email;
	VS_SimpleStr	from_email;
	std::string		subj;
	std::string		body;
	std::string		fromDN;
	std::string		toDN;
	dprint3("MultiInviteMail fromEm = %s; toEm = %s;\n",fromId, toId);

	if (fromDn) fromDN = fromDn;
	if (toDn) toDN = toDn;
	auto dbStorage = g_dbStorage;
	if(!!dbStorage && dbStorage->GetMissedCallMailTemplate(missed_call_time,fromId, fromDN,toId, toDN,from_email,to_email,subj,body))
	{
		char missed_call_time_str[0xff];
		if (tu::TimeToGStr(missed_call_time, missed_call_time_str, 0xff) == 0) return;

		FillMailTemplate(body, fromDN.c_str(), fromId, missed_call_time_str, toDN.c_str(), toId, app_name, to_email);
		FillMailTemplate(subj, fromDN.c_str(), fromId, missed_call_time_str, toDN.c_str(), toId, app_name, to_email);

		ReplaceGuestUrl(body);

		///application
		dstream3 << "MultiInviteMail: MakeMailMessage To:" << to_email.m_str << " From:" << from_email.m_str << " Subj:" << subj << "\n";
		VS_RouterMessage	*mess = MakeMailMessage(to_email, from_email, subj.c_str(), body.c_str(), from_email);
		if(mess)
		{
			if(!PostMes(mess))
				delete mess;
		}
	}

}

void FillNamedConfMailTemplate(std::string &OUTtoFill, const char* fromDN, const char* fromId, const char* missed_call_time_str, const char* toDn, const char* toId, const char* app_name, const char* to_email) {
	VS_ReplaceAll(OUTtoFill, "%caller_display_name", fromDN != nullptr ? fromDN : "");
	VS_ReplaceAll(OUTtoFill, "%caller_call_id", fromId != nullptr ? fromId : "");
	VS_ReplaceAll(OUTtoFill, "%start_time", missed_call_time_str != nullptr ? missed_call_time_str : "");
	VS_ReplaceAll(OUTtoFill, "%user_display_name", toDn != nullptr ? toDn : "");
	VS_ReplaceAll(OUTtoFill, "%recipient_call_id", toId != nullptr ? toId : "");
	VS_ReplaceAll(OUTtoFill, "%application", app_name != nullptr ? app_name : "");
	VS_ReplaceAll(OUTtoFill, "%subscribed_email", to_email != nullptr ? to_email : "");
}

void VS_VCSLogService::MissedNamedConfMail(const std::chrono::system_clock::time_point missed_call_time, const char *app_name, const char* fromId, const char* fromDn, const char* toId, const char* toDn, const char* conf_id, const std::string& topic, const char* file_ics_utf8)
{

	/**
		%key md5 email+secretS
	*/
	VS_SimpleStr	to_email;
	VS_SimpleStr	from_email;
	std::string		subj;
	std::string		body;
	std::string		fromDN;
	std::string		toDN;
	dprint3("MissedCallMail fromEm = %s; toEm = %s;\n",fromId, toId);

	if (fromDn) fromDN = fromDn;
	if (toDn) toDN = toDn;
	auto dbStorage = g_dbStorage;
	if(!!dbStorage && dbStorage->GetMissedNamedConfMailTemplate(fromId, fromDN,toId, toDN,from_email,to_email,subj,body))
	{
		/**
caller_display_name - имя звонившего
2) caller_call_id - e-mail звонившего
3) missed_call_time - дата и время пропущенного звонка
4) recipient_display_name - имя получателя
5) recipient_call_id - e-mail получателя
6) application
		*/

		char missed_call_time_str[0xff];
		if (tu::TimeToGStr(missed_call_time, missed_call_time_str, 0xff) == 0) return;

		FillNamedConfMailTemplate(body, fromDN.c_str(), fromId, missed_call_time_str, toDN.c_str(), toId, app_name, to_email);
		FillNamedConfMailTemplate(subj, fromDN.c_str(), fromId, missed_call_time_str, toDN.c_str(), toId, app_name, to_email);

		ReplaceGuestUrl(body, conf_id);

		const char* fromName = fromDn ? fromDn : fromId;
		if (fromName)
			VS_ReplaceAll(body, "%owner_name", fromName);

		if(conf_id) VS_ReplaceAll(body, "%conf_id", conf_id);
		VS_ReplaceAll(body, "%conf_name", topic);

		if (conf_id) VS_ReplaceAll(subj, "%conf_id", conf_id);
		VS_ReplaceAll(subj, "%conf_name", topic);

		///application
		dstream3 << "MissedCallMail: MakeMailMessage To:" << to_email.m_str << " From:" << from_email.m_str << " Subj:" << subj << "\n";
		VS_RouterMessage	*mess = MakeMailMessage(to_email, from_email, subj.c_str(), body.c_str(), from_email,0,0,file_ics_utf8);
		if(mess)
		{
			if(!PostMes(mess))
				delete mess;
		}
	}
}

void VS_VCSLogService::ReplaceGuestUrl(std::string& body, const char* confid) const
{
	if (!body.length())
		return ;
	auto dbStorage = g_dbStorage;
	if (!dbStorage)
		return ;
   std::string guest_host;
	unsigned long guest_port(0);
	dbStorage->GetWebManagerProperty("Host",guest_host);
	dbStorage->GetWebManagerProperty("Port",guest_port);

	if (guest_host.empty())
	{
		// get any of Current Connect
		char buff[1024] = {0};
		VS_RegistryKey key(false, CONFIGURATION_KEY);
		key.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, CURRENT_CONNECT_TAG);
		if (*buff)
		{
			// take only first ip address
			char* comma = strchr(buff, ',');
			if(comma) *comma = '\0';
			// take host
			char* p = strrchr(buff, ':');
			if (p)
			{
				p[0] = 0;
				guest_host = buff;
			}else{
				guest_host = "127.0.0.1";
			}
		}
	}

	std::string web_url = guest_host;
	if (guest_port && !web_url.empty() && guest_port != 80 && guest_port !=443)
	{
    web_url += ":";
    web_url += std::to_string(guest_port);
	}
	VS_ReplaceAll(body, "%host", web_url);

	VS_SimpleStr guest_url;
	VS_SimpleStr conf_url;
	if (!dbStorage->GetAppProperty("", "site_url", conf_url)){
		conf_url = "http://";
		conf_url += web_url.c_str();
	}
	// guest url
	guest_url = conf_url;
	guest_url += "/guest";
	VS_ReplaceAll(body, "%tcs_guest_page_url", guest_url.m_str);

	// conference url
	if (confid) {
		conf_url += "/c/";
		conf_url += confid;
		VS_ReplaceAll(body, "%conf_url", conf_url.m_str);
	}

	std::string admin_fio;
	dbStorage->GetWebManagerProperty("admin_fio", admin_fio);
	if (admin_fio.empty())
          admin_fio = "";
        VS_ReplaceAll(body, "%admin_name", admin_fio);

    std::string admin_phone;
	dbStorage->GetWebManagerProperty("admin_phone", admin_phone);
	if (admin_phone.empty())
          admin_phone = "";
        VS_ReplaceAll(body, "%admin_phone", admin_phone);

    std::string admin_email;
	dbStorage->GetWebManagerProperty("admin_email", admin_email);
	if (admin_email.empty())
		admin_email = "";
	VS_ReplaceAll(body, "%admin_email", admin_email);
}

void VS_VCSLogService::SendMail_Method(VS_Container &cnt)
{
	std::string do_mailer;
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return;

	long type(0);

	const char* toId = cnt.GetStrValueRef(CALLID2_PARAM);
	if (!toId)
		return ;

	VS_UserData to_ud;

	if (dbStorage->FindUser(toId, to_ud, false) && to_ud.IsValid())
		type = 1;		// missed call
	else
		type = 2;		// invite new user

	const char* fromDn    = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
	const char* fromEm		= cnt.GetStrValueRef(CALLID_PARAM);
	const char* app_name	= cnt.GetStrValueRef("app_name");
	const char* name		  = cnt.GetStrValueRef(CONFERENCE);
	const char* topic     = cnt.GetStrValueRef(TOPIC_PARAM);
	const char* file_ics	= cnt.GetStrValueRef("file_ics");
	int32_t type_ = 0;		 cnt.GetValue(TYPE_PARAM, type_);

	std::chrono::system_clock::time_point missed_call_time;
	size_t size = 0;
	const void * time = cnt.GetBinValueRef(TIME_PARAM, size);
	if (time && (size==sizeof(int64_t)))
		missed_call_time = tu::WindowsTickToUnixSeconds(*static_cast<const int64_t*>(time));

	if (name)
		type = 3;

	if (type_ == 4)
		type = 4;

	dbStorage->GetServerProperty((type==4)?"suppress_conf_invite_mail":"suppress_missed_call_mail", do_mailer);
	if(!do_mailer.empty() && do_mailer == "1")
		return ;

	switch (type)
	{
	case 1:
		MissedCallMail(missed_call_time,app_name,fromEm,fromDn,toId,to_ud.m_displayName.c_str());
		break;
	case 2:
		InviteMail(missed_call_time,app_name,fromEm,fromDn,toId);
		break;
	case 3:
		MultiInviteMail(missed_call_time, app_name,fromEm,fromDn,toId,to_ud.m_displayName.c_str());
		break;
	case 4:
		MissedNamedConfMail(missed_call_time,app_name,fromEm,fromDn,toId,to_ud.m_displayName.c_str(),name,topic,file_ics);
		break;
	}
}
#endif
