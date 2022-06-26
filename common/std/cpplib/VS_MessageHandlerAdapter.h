#pragma once
#include <boost/signals2.hpp>
#include "VS_MessageHandler.h"

class VS_MessageHandlerAdapter : public VS_MessageHandler
{
public:
	virtual ~VS_MessageHandlerAdapter()
	{
	}
	boost::signals2::connection	ConnectToHandleMessage(const boost::signals2::signal<void (const boost::shared_ptr<VS_MessageData>& )>::slot_type &slot)
	{
		return m_fireHandleMessage.connect(slot);
	}
	boost::signals2::connection	ConnectToHandleMessageWithResult(const 	boost::signals2::signal<void (const boost::shared_ptr<VS_MessageData>&,
																	const boost::shared_ptr<VS_MessResult>&)>::slot_type &slot)
	{
		return m_fireHandleMessageWithResult.connect(slot);
	}
private:
	friend boost::signals2::deconstruct_access;
	template<typename T> friend
		void adl_postconstruct(const boost::shared_ptr<VS_MessageHandlerAdapter> &p, T *instance)
	{
	}

	VS_MessageHandlerAdapter(){}
	virtual void HandleMessage(const boost::shared_ptr<VS_MessageData> &message)
	{
		m_fireHandleMessage(message);
	}
	virtual void HandleMessageWithResult(const boost::shared_ptr<VS_MessageData> &message, const boost::shared_ptr<VS_MessResult> &res)
	{
		m_fireHandleMessageWithResult(message,res);
	}

	boost::signals2::signal<void (const boost::shared_ptr<VS_MessageData>& )>	m_fireHandleMessage;
	boost::signals2::signal<void (const boost::shared_ptr<VS_MessageData>&,
		const boost::shared_ptr<VS_MessResult>&)>								m_fireHandleMessageWithResult;
};