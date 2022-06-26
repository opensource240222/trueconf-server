#ifdef _WIN32 // not ported
#include "VS_BSLogService.h"
#include "../../common/std/cpplib/curl_deleters.h"

#include <curl/curl.h>
#include <boost/make_shared.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_LOGSERVICE

VS_BSLogService::VS_BSLogService()
{
	m_thread.reset(new VS_SimpleWorkThread);
	m_thread->Start("BSLogService");
}

VS_BSLogService::~VS_BSLogService()
{

}

void VS_BSLogService::SendMail_Method(VS_Container &cnt)
{
	std::string do_mailer;
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return;
	dbStorage->GetServerProperty("suppress_missed_call_mail", do_mailer);
	if(!do_mailer.empty() && do_mailer == "1")
		return ;

	int32_t event_type = -1;
	cnt.GetValue(TYPE_PARAM, event_type);
	boost::shared_ptr<VS_MessageData> mess;

	dprint3("NotifyViaWeb: event=%d id1=%s id2=%s conf_name=%s\n", event_type, cnt.GetStrValueRef(CALLID_PARAM), cnt.GetStrValueRef(CALLID2_PARAM), cnt.GetStrValueRef(CONFERENCE_PARAM));
	switch (event_type){
	case e_notifyviaweb_missed_chat_msg:
		mess = boost::make_shared<VS_MessageData>(e_notifyviaweb_missed_chat_msg, nullptr, 0);
		break;
	case e_notifyviaweb_user_waits_for_letter:
		mess = boost::make_shared<VS_MessageData>(e_notifyviaweb_user_waits_for_letter, nullptr, 0);
		break;
	default:
		mess = boost::make_shared<NotifyViaWeb_MissedCall_Mess>(cnt);
		break;
	}

	boost::shared_ptr<VS_MessageHandler> handler = m_this.lock();
	if(!handler)
		return ;
	m_thread->Post(handler, mess);
}

void VS_BSLogService::HandleMessage(const boost::shared_ptr<VS_MessageData> &message)
{
	if(!message)
		return;
	int event_type(0);
	unsigned long sz(0);
	unsigned long type(0);
	message->GetMessPointer(type,sz);
	const char *url = nullptr;

	switch(type)
	{
	case e_notifyviaweb_missed_call:
		{
			NotifyViaWeb_MissedCall_Mess *mess = reinterpret_cast<NotifyViaWeb_MissedCall_Mess*>(message.get());
			g_dbStorage->NotifyWeb_MissedCall(mess->m_cnt);

			if (!mess->m_cnt.GetStrValueView(CONFERENCE_PARAM).empty()) { // use URL for gconf
				if (!m_notify_conf_invite_url.empty())	url = m_notify_conf_invite_url.c_str();
			} else { // use URL for p2p conf
				if (!m_notify_calls_url.empty())	url = m_notify_calls_url.c_str();
			}
		}
		break;
	case e_notifyviaweb_missed_chat_msg:
		if (!m_notify_chat_url.empty())	url = m_notify_chat_url.c_str();
		break;
	case e_notifyviaweb_user_waits_for_letter:
		if (!m_notify_wait_for_letter_url.empty())	url = m_notify_wait_for_letter_url.c_str();

	}

	// libcURL
	if (url){
		std::unique_ptr<CURL, CURL_deleter> pCurl(::curl_easy_init());
		CURL *curl = pCurl.get();

		CURLcode res;
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_NOBODY, 1);						// to use HEAD instead of GET (not to get contents)
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);					// timeout in seconds
			res = curl_easy_perform(curl);
		}
	}
}
#endif