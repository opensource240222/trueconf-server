//////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 12.11.02     by  A.Slavetsky
//
//////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_TransportRouterServiceBase.h
/// \brief
/// \note
///

#ifndef VS_TRANSPORT_ROUTER_SERVICE_BASE_H
#define VS_TRANSPORT_ROUTER_SERVICE_BASE_H

#include "../typedefs.h"
#include "VS_MessageFloodBlock.h"

#include <memory>
#include <string>
#include "std-generic/cpplib/synchronized.h"
#include <functional>
#include "std-generic/attributes.h"

class VS_SimpleStr;
class VS_RouterMessage;
class VS_PoolThreadsTask;

class VS_PointParams
{
public:
	enum Point_Type {
		PT_UNKNOWN = -1,
		PT_CLIENT = 0,
		PT_SERVER = 1
	};
	enum Condition_Reason {
		CR_HSERROR = -2,
		CR_TIMEOUT = -1,
		CR_UNKNOWN = 0,
		CR_REQUESTED = 1,
		CR_INCOMING = 2,
	};
	int		type;	// Point_Type
	int		ver;	// transport protocol version
	int		reazon;	// Condition_Reason
	const char*	cid;	// connection id
	const char*	uid;	// user/server name (if present)

	VS_PointParams() : type(0), ver(0), reazon(0), cid(0), uid(0) {}
	~VS_PointParams() {}
};

class VS_TransportRouterServiceBase
{
protected:
	virtual void AsyncDestroy() {};
public:
	class Impl
	{
	public:
		virtual ~Impl() {};

		virtual bool SendMes(VS_RouterMessage* mes) = 0;
		virtual bool PostMes(VS_RouterMessage* mes) = 0;
		virtual bool RequestResponse(VS_RouterMessage* mes, ResponseCallBackT&& cb, RequestLifeTimeT&& life_time) = 0;
		virtual ResponseFutureT RequestResponse(VS_RouterMessage* mes, RequestLifeTimeT&& life_time) = 0;
		virtual const char* OurEndpoint() const = 0;
		virtual const char* OurService() const = 0;
		virtual bool GetStatistics(struct VS_TransportRouterStatistics* stat) = 0;
		virtual bool IsThereEndpoint(const char* endpoint) = 0;
		virtual void DisconnectEndpoint(const char* endpoint) = 0;
		virtual void FullDisconnectEndpoint(const char* endpoint) = 0;
		virtual void FullDisconnectAllEndpoints() = 0;
		virtual bool AuthorizeClient(const char* uid, const char* new_uid) = 0;
		virtual bool UnauthorizeClient(const char* uid) = 0;
		virtual bool IsAuthorized(const char* uid) = 0;
		virtual std::string GetCIDByUID(const char* uid) = 0;
		virtual bool GetIPByCID(const char* cid, std::string& ip) = 0;
		virtual bool PutTask(VS_PoolThreadsTask* task, const char* nameExtension, unsigned lifetimeSec, unsigned priority) = 0;

		virtual bool SetThread(VS_TransportRouterServiceBase::Impl* instance) = 0;
		virtual void ResetThread() = 0;
		virtual int ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, bool allAdopted) = 0;
		virtual int ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, std::chrono::steady_clock::duration& wait_time, bool allAdopted) = 0;
	};

	VS_TransportRouterServiceBase();
	virtual ~VS_TransportRouterServiceBase();

	virtual bool	Init		( const char *our_endpoint,
									const char *our_service,
									const bool permittedAll = false) = 0;
	virtual void Destroy(const char* /*our_endpoint*/, const char* /*our_service*/) {}

	virtual bool SendMes(VS_RouterMessage* mes)
	{
		return !imp ? false : imp->SendMes(mes);
	}
	virtual bool PostMes(VS_RouterMessage* mes)
	{
		return !imp ? false : imp->PostMes(mes);
	}
	bool ReqResp(VS_RouterMessage* mes, ResponseCallBackT&& cb, RequestLifeTimeT&& lt)
	{
		return !imp ? false : imp->RequestResponse(mes, std::move(cb), std::move(lt));
	}
	ResponseFutureT ReqResp(VS_RouterMessage* mes, RequestLifeTimeT&& lt)
	{
		return !imp ? ResponseFutureT() : imp->RequestResponse(mes, std::move(lt));
	}

	virtual const char* OurEndpoint() const
	{
		return !imp ? nullptr : imp->OurEndpoint();
	}
	virtual const char* OurService() const
	{
		return !imp ? nullptr : imp->OurService();
	}

	bool GetStatistics(struct VS_TransportRouterStatistics* stat)
	{
		return !imp ? false : imp->GetStatistics(stat);
	}

	// Присоединен ли данный Endpoint к TransportRouter непосредственно ?
	bool IsThereEndpoint(const char* endpoint)
	{
		return !imp ? false : imp->IsThereEndpoint(endpoint);
	}

	// Если данный Endpoint был непосредственно присоединен к брокеру, то после этого
	// вызова его ресурс в TransportRouter будет освобожден, соединения закрыты,
	// существующие Routes к данной точке удалены.
	void DisconnectEndpoint(const char* endpoint)
	{
		if (imp) imp->DisconnectEndpoint(endpoint);
	}

	// Тоже что и DisconnectEndpoint только с предварительным уведомлением напарника.
	void FullDisconnectEndpoint(const char* endpoint)
	{
		if (imp) imp->FullDisconnectEndpoint(endpoint);
	}

	// Тоже что и FullDisconnectEndpoint только для всех
	void FullDisconnectAllEndpoints()
	{
		if (imp) imp->FullDisconnectAllEndpoints();
	}

	bool AuthorizeClient(const char* uid, const char* new_uid)
	{
		return !imp ? false : imp->AuthorizeClient(uid, new_uid);
	}
	bool UnauthorizeClient(const char* uid)
	{
		return !imp ? false : imp->UnauthorizeClient(uid);
	}
	bool IsAuthorized(const char* uid)
	{
		return !imp ? false : imp->IsAuthorized(uid);
	}
	std::string GetCIDByUID(const char* uid)
	{
		if (imp)
			return imp->GetCIDByUID(uid);
		else
			return {};
	}
	bool GetIPByCID(const char* cid, std::string& ip)
	{
		return !imp ? false : imp->GetIPByCID(cid, ip);
	}
	bool PutTask(VS_PoolThreadsTask* task,
	             const char* nameExtension = 0, // For service name
	             const unsigned lifetimeSec = ~0, // Maximal lifetime
	             const unsigned priority = ~0) // Minimal priority
	{
		return !imp ? false : imp->PutTask(task, nameExtension, lifetimeSec, priority);
	}

	virtual bool	OnPointConnected_Event(const VS_PointParams* /*prm*/) { return true; };
	virtual bool	OnPointDeterminedIP_Event(const char* /*uid*/, const char* /*ip*/) { return true; };
	virtual bool	OnPointDisconnected_Event(const VS_PointParams* /*prm*/) { return true; };

	bool SetThread(VS_TransportRouterServiceBase* instance = nullptr)
	{
		return !imp ? false : (!instance ? imp->SetThread(nullptr) : (!instance->imp ? false : imp->SetThread(instance->imp.get())));
	}
	void ResetThread()
	{
		AsyncDestroy();
		if (imp)
			imp->ResetThread();
	}
	int ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, bool allAdopted = true)
	{
		return !imp ? -1 : imp->ReceiveMes(mes, allAdopted);
	}
	int ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, std::chrono::steady_clock::duration& wait_time, bool allAdopted = true)
	{
		return !imp ? 0 : imp->ReceiveMes(mes, wait_time, allAdopted);
	}

public:
	virtual void Thread();
	virtual bool Timer(std::chrono::milliseconds /*tickcount*/) { return true; }
	VS_DEPRECATED virtual bool Timer(unsigned long tickcount) { return Timer(std::chrono::milliseconds(tickcount)); }
	virtual bool Processing(std::unique_ptr<VS_RouterMessage>&& /*recvMess*/) { assert(false); return false; };

	int GetProcessingThreadId() const;
	bool IsInProcessingThread() const;
	template <class F>
	bool CallInProcessingThread(F&& f, bool immediately_if_possible = true)
	{
		if (immediately_if_possible && IsInProcessingThread())
		{
			f();
			return true;
		}
		else
			return CallInProcessingThread(std::function<void()>(std::forward<F>(f)));
	}

private:
	bool CallInProcessingThread(std::function<void()>&& f);
protected:
	std::chrono::steady_clock::duration m_TimeInterval;
private:
	std::chrono::steady_clock::time_point m_LastTimerTime;
	std::atomic<int> m_processingThread;
	std::unique_ptr<VS_MessageFloodBlock> m_messBlock;

	using CallFunction = std::pair<std::chrono::steady_clock::time_point, std::function<void()>>;
	vs::Synchronized<std::vector<CallFunction>> m_callFucntions;

public:
	std::unique_ptr<Impl> imp;
};
// end VS_TransportRouterServiceBase class

struct VS_TransportRouterStatistics
{
	unsigned long   endpoints;
	double   out_bytes, in_bytes;
	float   out_byterate, in_byterate;
};
// end VS_TransportRouterStatistics struct

#endif  // VS_TRANSPORT_ROUTER_SERVICE_BASE_H
