#pragma once
#include "storage/VS_DBStorage.h"
#include "../../ServerServices/VS_LogServiceBase.h"
#include "../../common/std/cpplib/VS_SimpleWorkThread.h"
#include "../../common/std/cpplib/VS_MessageHandler.h"


enum {
	e_notifyviaweb_missed_call,
	e_notifyviaweb_missed_chat_msg,
	e_notifyviaweb_user_waits_for_letter
};

class NotifyViaWeb_MissedCall_Mess: public VS_MessageData
{
public:
	NotifyViaWeb_MissedCall_Mess(VS_Container &cnt):VS_MessageData(e_notifyviaweb_missed_call,0,0),
		m_cnt(cnt)
	{
	}
	VS_Container	m_cnt;
};


class VS_BSLogService: public VS_LogServiceBase, public VS_MessageHandler
{
	VS_BSLogService();
	boost::weak_ptr<VS_BSLogService>	m_this;
	friend class boost::signals2::deconstruct_access;
	template<typename T> friend
		void adl_postconstruct(const boost::shared_ptr<VS_BSLogService> &p, T *instance)
	{
		p->m_this = p;
		//p->Init();
	}
	boost::shared_ptr<VS_SimpleWorkThread> m_thread;
public:
	virtual ~VS_BSLogService();

	virtual void SendMail_Method(VS_Container &cnt);

	void HandleMessage(const boost::shared_ptr<VS_MessageData> &message);
};