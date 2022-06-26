#pragma once
#include "VS_WorkThread.h"
#include "std-generic/cpplib/scope_exit.h"
#include "acs/connection/VS_IOHandler.h"

#if defined(_WIN32) && !defined(_TRY_PORTED_)
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#endif

struct VS_Overlapped;
class VS_WorkThreadIOCP : public VS_WorkThread
{
	void*	m_iocp;
	VS_Overlapped	*m_processing_messageOv;
	VS_Overlapped	*m_stopOv;

protected:
	void Thread(boost::shared_ptr<VS_WorkThread>&& keepAlive) override;
	void Notify() override;
public:

	void Stop() override;
	VS_WorkThreadIOCP(const bool freeByThreadTerminate = false);
	~VS_WorkThreadIOCP();
	bool SetHandledConnection(VS_Connection *conn) override;

	void Handle(const unsigned long sz, const struct VS_Overlapped *ov) override;
	void HandleError(const unsigned long err, const struct VS_Overlapped *ov) override;

	virtual void Handle(VS_IOHandler *handle, const unsigned long sz, const struct VS_Overlapped *ov);
	virtual void HandleError(VS_IOHandler *handle, const unsigned long err, const struct VS_Overlapped *ov);
};


extern void SetCreatorWorkThread(std::function<boost::shared_ptr<VS_WorkThread>(const char *)> creator);
extern boost::shared_ptr<VS_WorkThread> MakeWorkThread(const char *nameThread);

#if defined(_WIN32) && !defined(_TRY_PORTED_) // not ported
class VS_WorkThreadASIOStrand : public VS_WorkThreadIOCP
{
public:

	template<typename ...Args>
	static boost::shared_ptr<VS_WorkThreadASIOStrand> Create(Args&& ...args)
	{
		struct EnableMakeShared final : public VS_WorkThreadASIOStrand
		{
			EnableMakeShared(Args&& ...args) : VS_WorkThreadASIOStrand(std::forward<Args>(args)...) {}
		};
		return boost::make_shared<EnableMakeShared>(std::forward<Args>(args)...);
	}

	bool IsCurrent() const override
	{
		return m_strand.running_in_this_thread();
	}

	void Send(const boost::shared_ptr<VS_MessageHandler>& h, const boost::shared_ptr<VS_MessageData>& data,
		const boost::shared_ptr<VS_MessResult>& outRes) override
	{
		vs::event ev {false};
		m_strand.dispatch([&]()
		{
			VS_SCOPE_EXIT{ ev.set(); };
			if (outRes)
				h->HandleMessageWithResult(data, outRes);
			else
				h->HandleMessage(data);
		});
		ev.wait();
	}

	void Post(const boost::shared_ptr<VS_MessageHandler>& h, const boost::shared_ptr<VS_MessageData>& data) override
	{
		m_strand.post([h, data]()
		{
			h->HandleMessage(data);
		});
	}

	boost::asio::io_service::strand &GetStrand() const
	{
		return m_strand;
	}

	void Handle(VS_IOHandler *handle, const unsigned long sz, const struct VS_Overlapped *ov) override
	{
		m_strand.dispatch([=]()
		{
			handle->Handle(sz, ov);
		});
	}

	void HandleError(VS_IOHandler *handle, const unsigned long err, const struct VS_Overlapped *ov) override
	{
		m_strand.dispatch([=]()
		{
			handle->HandleError(err, ov);
		});
	}


	void Post(task_type task) override
	{
		m_strand.post(std::move(task));
	}

	void Dispatch(task_type task) override
	{
		m_strand.dispatch(std::move(task));
	}

protected:
	VS_WorkThreadASIOStrand(boost::asio::io_service::strand &strand) : m_strand(strand) {}

private:
	boost::asio::io_service::strand &m_strand;
};
#endif