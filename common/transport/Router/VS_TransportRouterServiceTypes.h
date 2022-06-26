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
/// \file VS_TransportRouterServiceTypes.h
/// \brief
/// \note
///

#ifndef VS_TRANSPORT_ROUTER_SERVICE_TYPES_H
#define VS_TRANSPORT_ROUTER_SERVICE_TYPES_H

#include "VS_RouterMessage.h"
#include "VS_TransportRouterServiceBase.h"
#include "VS_PoolThreadsService.h"
#include "../VS_TransportDefinitions.h"
#include "../typedefs.h"
#include "../../acs/VS_AcsDefinitions.h"
#include "../../acs/Lib/VS_AcsLog.h"
#include "acs/connection/VS_ConnectionByte.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/numerical.h"
#include "std-generic/cpplib/scope_exit.h"

#include <chrono>
#include <future>

#include <Windows.h>
#include <process.h>

struct VS_TransportRouterStatistics;
class VS_SimpleStr;
class VS_TransportRouterServiceBase;


struct VS_TransportRouter_CallService
{
	VS_TransportRouter_CallService( void ) : threadId(0),tAcsLog(0) {}
	// end of VS_TransportRouter_CallService::VS_TransportRouter_CallService

	virtual ~VS_TransportRouter_CallService( void ) {}
	// end of VS_TransportRouter_CallService::~VS_TransportRouter_CallService

	DWORD   threadId;		VS_AcsLog   *tAcsLog;

	virtual bool			CallServiceSendMes( VS_RouterMessage *mes ) = 0;
	virtual bool			CallServicePostMes( VS_RouterMessage *mes ) = 0;
	virtual bool			CallServiceRequestResponse(VS_RouterMessage *mess, ResponseCallBackT&&resp_cb, RequestLifeTimeT&&req_life_time) = 0;
	virtual ResponseFutureT CallServiceRequestResponse(VS_RouterMessage*mess, RequestLifeTimeT&&req_life_time) = 0;
	//virtual bool
	virtual bool			AddService(const char *serviceName, VS_TransportRouterServiceBase *service, bool isPermittedAll = false) = 0;
	virtual bool			RemoveService(const char *serviceName) = 0;
	virtual bool			BaseServiceGetStatistics( VS_TransportRouterStatistics *stat ) = 0;

	virtual bool			BaseServiceIsThereEndpoint( const char *endpoint ) = 0;
	virtual bool			BaseServiceDisconnectEndpoint( const char *endpoint ) = 0;
	virtual bool			BaseServiceFullDisconnectEndpoint( const char *endpoint ) = 0;
	virtual bool			BaseServiceFullDisconnectAllEndpoints( const CheckEndpointPredT &pred) = 0;
	virtual bool			BaseServiceAddMessageStatistics(const char *name, unsigned long body) = 0;
	virtual bool			AuthorizeClient(const char *uid,const char *new_uid)   = 0;
	virtual bool			UnauthorizeClient(const char *uid) = 0;
	virtual bool			IsAuthorized( const char *uid ) { return true; }
	virtual std::string		GetCIDByUID(const char* uid) = 0;
	virtual bool			GetIPByCID(const char* cid, std::string& ip) = 0;

	virtual void			ShowPeriod() = 0;
};
// end of VS_TransportRouter_CallService struct

struct VS_TransportRouterServiceBase_Implementation : public VS_TransportRouterServiceBase::Impl
{
	VS_TransportRouterServiceBase_Implementation( VS_TransportRouterServiceBase *trs,
										const char *endpointName, const char *serviceName,
										VS_TransportRouter_CallService *tr ) :
			isValid(false), trs(trs), tr(tr)
	{
		memset( VS_TransportRouterServiceBase_Implementation::endpointName, 0, sizeof(VS_TransportRouterServiceBase_Implementation::endpointName) );
		memset( VS_TransportRouterServiceBase_Implementation::serviceName, 0, sizeof(VS_TransportRouterServiceBase_Implementation::serviceName) );
		if (!endpointName || !*endpointName || strlen(endpointName) > VS_ACS_MAX_SIZE_ENDPOINT_NAME
				|| !serviceName || !*serviceName || strlen(serviceName) > VS_TRANSPORT_MAX_SIZE_SERVICE_NAME
				|| !tr || !tr->threadId || tr->threadId == (DWORD)-1L)		return;
		strcpy( VS_TransportRouterServiceBase_Implementation::endpointName, endpointName );
		strcpy( VS_TransportRouterServiceBase_Implementation::serviceName, serviceName );
		isValid = true;
	}
	// end of VS_TransportRouterServiceBase_Implementation::VS_TransportRouterServiceBase_Implementation

	virtual ~VS_TransportRouterServiceBase_Implementation( void ) {}
	// end of VS_TransportRouterServiceBase_Implementation::~VS_TransportRouterServiceBase_Implementation

	bool   isValid;		VS_TransportRouterServiceBase   *trs;
	char   endpointName[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1],
			serviceName[VS_TRANSPORT_MAX_SIZE_SERVICE_NAME + 1];
	VS_TransportRouter_CallService   *tr;

	bool SendMes(VS_RouterMessage* mes) override
	{	return tr->CallServiceSendMes( mes );	}
	// end of VS_TransportRouterServiceBase_Implementation::SendMes

	bool PostMes(VS_RouterMessage* mes) override
	{	return tr->CallServicePostMes( mes );	}
	// end of VS_TransportRouterServiceBase_Implementation::PostMes
	bool RequestResponse(VS_RouterMessage* mes, ResponseCallBackT&& cb, RequestLifeTimeT&& life_time) override
	{
		return tr->CallServiceRequestResponse(mes, std::move(cb), std::move(life_time));
	}
	ResponseFutureT RequestResponse(VS_RouterMessage* mes, RequestLifeTimeT&& life_time) override
	{
		return tr->CallServiceRequestResponse(mes, std::move(life_time));
	}
	const char* OurEndpoint() const override
	{
		return endpointName;
	}
	const char* OurService() const override
	{
		return serviceName;
	}

	bool GetStatistics(VS_TransportRouterStatistics* stat) override
	{	return tr->BaseServiceGetStatistics( stat );	}
	// end of VS_TransportRouterServiceBase_Implementation::GetStatistics

	bool IsThereEndpoint(const char* endpoint) override
	{	return tr->BaseServiceIsThereEndpoint( endpoint );	}
	// end of VS_TransportRouterServiceBase_Implementation::IsThereEndpoint

	void DisconnectEndpoint(const char* endpoint) override
	{	tr->BaseServiceDisconnectEndpoint( endpoint );		}
	// end of VS_TransportRouterServiceBase_Implementation::DisconnectEndpoint

	void FullDisconnectAllEndpoints() override
	{
		tr->BaseServiceFullDisconnectAllEndpoints([]
		(string_view, string_view, unsigned char) {return true; });
	}
	// end of VS_TransportRouterServiceBase_Implementation::FullDisconnectEndpoint

	void FullDisconnectEndpoint(const char* endpoint) override
	{	tr->BaseServiceFullDisconnectEndpoint( endpoint );		}
	// end of VS_TransportRouterServiceBase_Implementation::FullDisconnectEndpoint
	bool AuthorizeClient(const char* endpointName, const char* new_uid) override
	{
		return tr->AuthorizeClient( endpointName, new_uid );
	}
	bool UnauthorizeClient(const char* endpointName) override
	{
		return tr->UnauthorizeClient( endpointName );
	}
	bool IsAuthorized(const char* uid) override
	{
		return tr->IsAuthorized( uid );
	}
	std::string GetCIDByUID(const char* uid) override
	{
		return tr->GetCIDByUID(uid);
	}
	bool GetIPByCID(const char* cid, std::string& ip) override
	{
		return tr->GetIPByCID(cid, ip);
	}
	inline bool AddMessageStatistics(const char *name, unsigned long body)
	{
		return tr->BaseServiceAddMessageStatistics(name, body);
	}
	bool PutTask(VS_PoolThreadsTask* task, const char* nameExtension, unsigned lifetimeSec, unsigned priority) override;

	bool SetThread(VS_TransportRouterServiceBase::Impl* /*instance*/) override { assert(false); return false; };
	void ResetThread() override { assert(false); };
	int ReceiveMes(std::unique_ptr<VS_RouterMessage>& /*mes*/, bool /*allAdopted*/) override { assert(false); return 0; };
	int ReceiveMes(std::unique_ptr<VS_RouterMessage>& /*mes*/, std::chrono::steady_clock::duration& /*wait_time*/, bool /*allAdopted*/) override { assert(false); return 0; };
};
// end of VS_TransportRouterServiceBase_Implementation struct

#define   MAX_ADOPTED_INSTS   (MAXIMUM_WAIT_OBJECTS - 1)

struct VS_TransportRouterService_Implementation : public VS_TransportRouterServiceBase_Implementation
{
	VS_TransportRouterService_Implementation( VS_TransportRouterServiceBase *trs,
										const char *endpointName, const char *serviceName,
										VS_TransportRouter_CallService *tr ) :
			VS_TransportRouterServiceBase_Implementation( trs, endpointName, serviceName, tr ),
			hostInst(0), pipe(0), tEnv(0), writeSize(0), readSize(sizeof(transport::MessageFixedPart)),
			readBuffer(0), stateRcv(0), skipRecv(0), skipRecvMax(5)
	{
		InitializeCriticalSection( &avoidManyPosts );
		if (!isValid)	return;		isValid = false;		if (!trs)	return;
		pipe = new VS_ConnectionByte;
		if (!pipe || !pipe->Create( vs_pipe_type_duplex )
			|| !pipe->CreateOvWriteEvent() || !pipe->CreateOvReadEvent())	return;
		isValid = true;
	}
	// end of VS_TransportRouterService_Implementation::VS_TransportRouterService_Implementation

	~VS_TransportRouterService_Implementation( void )
	{
		if (hostInst)	hostInst->DeleteAdoptedInst( this );
		isValid = false;
		delete tEnv; tEnv = 0;
		delete pipe; pipe = 0;
		DeleteCriticalSection(&avoidManyPosts);
	}
	// end of VS_TransportRouterService_Implementation::~VS_TransportRouterService_Implementation

	CRITICAL_SECTION avoidManyPosts;

	VS_TransportRouterService_Implementation   *hostInst;		VS_ConnectionByte   *pipe;
	unsigned long writeSize, readSize;
	transport::MessageFixedPart readHead;
	transport::Message writeMsg;
	unsigned char* readBuffer;
	unsigned stateRcv;
	unsigned skipRecv,skipRecvMax;
	struct ThreadEnv
	{
		ThreadEnv() :
				adoptedEvent(0), thread(0), continueThread(true), nAdoptedInsts(0)
		{	adoptedEvent = CreateEvent( 0, TRUE, TRUE, 0 );		if (!adoptedEvent)	return;
			memset( (void *)adoptedInsts, 0, sizeof(adoptedInsts) );
		};	// end of ThreadEnv::ThreadEnv

		~ThreadEnv( void )
		{	StopThread();
			if (adoptedInsts) {		for (unsigned i = 0; i < nAdoptedInsts; ++i)
										adoptedInsts[i]->hostInst = 0;		}
			if (adoptedEvent)	CloseHandle( adoptedEvent );
		};	// end of ThreadEnv::~ThreadEnv

		inline bool   IsValid( void ) const {	return adoptedEvent && adoptedInsts && thread;	}
		// end of ThreadEnv::IsValid

		HANDLE   adoptedEvent, thread;		bool   continueThread;
		VS_TransportRouterService_Implementation   *adoptedInsts[MAX_ADOPTED_INSTS];
		unsigned   nAdoptedInsts;

		inline void StartThread(VS_TransportRouterService_Implementation *trs)
		{
			thread = (HANDLE)_beginthreadex(0, 0, VS_TransportRouterService_Implementation::Thread, trs, 0, 0);
			if (!thread || thread == (HANDLE)-1L) { thread = 0;		return; }
		}

		inline void BreakThread( void )
		{	if (thread) {	continueThread = false;		SetEvent( adoptedEvent );	}}
		// end of ThreadEnv::BreakThread

		inline void StopThread( void )
		{	if (thread) {	continueThread = false;		SetEvent( adoptedEvent );
							if (WaitForSingleObject( thread, INFINITE ) == WAIT_OBJECT_0)
								CloseHandle( thread );		thread = 0;		}
		}
		// end of ThreadEnv::StopThread

		inline bool AddAdoptedInst( VS_TransportRouterService_Implementation *inst )
		{
			if (nAdoptedInsts >= MAX_ADOPTED_INSTS)
				return false;
			adoptedInsts[nAdoptedInsts] = inst;		++nAdoptedInsts;
			SetEvent( adoptedEvent );	return true;
		}
		// end of ThreadEnv::AddAdoptedInst

		inline void DeleteAdoptedInst( VS_TransportRouterService_Implementation *inst )
		{
			StopThread();
			unsigned i = 0;
			for (; i < nAdoptedInsts; ++i)
				if (adoptedInsts[i] == inst) {	--nAdoptedInsts;	break;	}
			for (; i < nAdoptedInsts; ++i)		adoptedInsts[i] = adoptedInsts[i + 1];
		}
		// end of ThreadEnv::DeleteAdoptedInst

		inline DWORD GetEvents( HANDLE hs[] )
		{
			DWORD   count = 1;		*hs = adoptedEvent;
			for (unsigned i = 0; i < nAdoptedInsts; ++i)
				hs[count++] = adoptedInsts[i]->pipe->OvReadEvent();
			return count;
		}
		// end of ThreadEnv::GetEvents
	};
	// end of VS_TransportRouterService_Implementation::ThreadEnv struct
	ThreadEnv   *tEnv;

	void Thread( void )
	{
		SetThreadName();
		if (pipe->Read((void *)&readHead, sizeof(transport::MessageFixedPart)))
			trs->Thread();
		else {	/* Here it will be necessary to throw off in TRACE */	}
	}
	// end of VS_TransportRouterService_Implementation::Thread

	static unsigned __stdcall Thread( void *arg )
	{
		((VS_TransportRouterService_Implementation *)arg)->Thread();
		_endthreadex( 0 );		return 0;
	}
	// end of VS_TransportRouterService_Implementation::Thread

	inline bool AddAdoptedInst( VS_TransportRouterService_Implementation *instance )
	{
		instance->hostInst = this;
		if (!tEnv)
		{
			tEnv = new ThreadEnv();
			if (!tEnv)
			{
				instance->hostInst = 0;
				return false;
			}
			tEnv->StartThread(this);
			if (!tEnv->IsValid())
			{
				delete tEnv;
				tEnv = 0;
				instance->hostInst = 0;
				return false;
			}
		}
		if (!tEnv->AddAdoptedInst(instance))
		{
			instance->hostInst = 0;
			return false;
		}
		return true;
	}
	// end of VS_TransportRouterService_Implementation::AddAdoptedInst

	inline void DeleteAdoptedInst( VS_TransportRouterService_Implementation *instance )
	{
		if (!instance || !tEnv)		return;
		tEnv->DeleteAdoptedInst( instance );
	}
	// end of VS_TransportRouterService_Implementation::DeleteAdoptedInst

	bool SetThread(VS_TransportRouterServiceBase::Impl* instance) override
	{
		if (!instance)	instance = this;
		if (hostInst)	return false;
		return static_cast<VS_TransportRouterService_Implementation*>(instance)->AddAdoptedInst(this);
	}
	// end of VS_TransportRouterService_Implementation::SetThread

	inline void BreakThread( void )
	{
		if (!tEnv)	return;		tEnv->BreakThread();
	}
	// end of VS_TransportRouterService_Implementation::BreakThread

	void ResetThread() override
	{
		DeleteAdoptedInst( this );
	}
	// end of VS_TransportRouterService_Implementation::ResetThread

	inline void SetThreadName()
	{
		std::string name;

		string_view srv_name(serviceName);
		if (srv_name.substr(0, 3) == "PT:")
		{
			// PutTask temporary service
			srv_name.remove_prefix(3 + 1 + 3 + 1 + 9); // Remove prefix, thread id and task id

			const auto name_end_pos = srv_name.find(" - ");
			if (name_end_pos != srv_name.npos)
				srv_name.remove_suffix(srv_name.size() - name_end_pos);

			name += "PT:";
			name += srv_name;
		}
		else
		{
			name += "S:";
			name += srv_name;
		}

		if (name.size() > 15)
			name.resize(15);
		vs::SetThreadName(name.c_str());
	}

	inline bool Write( VS_RouterMessage *mes )
	{
		const auto fpm = reinterpret_cast<const transport::MessageFixedPart*>(mes->Data());
		const unsigned long   sz = fpm->head_length + fpm->body_length + 1;
		if (!pipe->Write( (const void *)fpm, sz ))
		{
			if (tr && tr->tAcsLog)
			{
				tr->tAcsLog->CPrintf("\n\t Pointer: %X Size: %d Error: %d",
					fpm,sz,GetLastError());
			}
			return false;
		}
		writeMsg = std::move(*mes);
		writeSize = sz;
		delete mes;
		return true;
	}
	// end of VS_TransportRouterService_Implementation::Write

	inline bool GetWriteResult( void )
	{
		const unsigned long max_mills = 200000;
		const unsigned long max_lock  = 10000;
		unsigned long   mills = max_mills;//INFINITE; 5 seconds getRes
		unsigned long a(GetTickCount()),b(0);
		if (pipe->GetWriteResult( mills ) != (int)writeSize)
		{
			b = GetTickCount();
			if (b - a >= max_lock)
			{
				if (tr && tr->tAcsLog)
					tr->tAcsLog->Printf("\n Lock is detected in service. Lock period is %d.",
					b-a);
				tr->ShowPeriod();
			}
			return false;
		}
		b = GetTickCount();
		if (b - a >= max_lock)
		{
			if (tr && tr->tAcsLog)
				tr->tAcsLog->Printf("\n Lock is detected in service. Lock period is %d.",
				b-a);
			tr->ShowPeriod();
		}

		writeSize = 0;
		return true;
	}
	// end of VS_TransportRouterService_Implementation::GetResult

	bool SendMes(VS_RouterMessage* mes) override
	{
		if (!mes || !mes->IsValid())	return false;

		bool res = false;

		if (tr->threadId != GetCurrentThreadId())
		{
			EnterCriticalSection( &avoidManyPosts );
			if (!pipe->IsWrite() || GetWriteResult())
			{
				res = (Write( mes ) && GetWriteResult());
			}
			LeaveCriticalSection( &avoidManyPosts );
		} else
		{
			res = tr->CallServiceSendMes( mes );
		}
		return 	res;
	}
	// end of VS_TransportRouterService_Implementation::SendMes

	bool PostMes(VS_RouterMessage* mes) override
	{
		if (!mes || !mes->IsValid()) return false;

		bool res = false;
		bool getWR = false;
		long th_id = 0;
		if (tr->threadId != (th_id=GetCurrentThreadId()))
		{
			EnterCriticalSection( &avoidManyPosts );
			const auto sz = mes->BodySize();

			if (!pipe->IsWrite() || (getWR=GetWriteResult()))
			{
				if (pipe->IsWrite())
				{
					///Debug log
					if (tr->tAcsLog)
						tr->tAcsLog->Printf("\n Warning!!! BadState[%d]. Two Thread in one method.",th_id);
					bool result = Write( mes );

					if (result)
						res = true;
				}
				if (!res && Write( mes ))
				{
					res =  true;
				} else
				{
					///!Write
					if (tr->tAcsLog)
						tr->tAcsLog->Printf("\n Warning!!! Service cannot PostMes. !Write.[%d]",th_id);
					res = false;
				}
			}
			else
			{
				if (tr->tAcsLog)
					tr->tAcsLog->CPrintf("\n\t Warning!!! Service cannot PostMes. !GetWriteResult.[%d]",th_id);
				res = false;
			}
			if (res) { AddMessageStatistics((const char *)serviceName, sz); }
			LeaveCriticalSection( &avoidManyPosts );
		} else
		{
			const auto sz = mes->BodySize();
			if (tr->CallServicePostMes( mes ))
			{
				res = true;
			}else
			{
				if (tr->tAcsLog)
					tr->tAcsLog->Printf("\n Warring!!! Service cannot PostMes. !tr->CallServicePostMes( mes ).");

				res = false;
			}
			if (res) { AddMessageStatistics((const char *)serviceName, sz); }
		}

		return res;
	}
	// end of VS_TransportRouterService_Implementation::PostMes

	inline int ReceiveMesAct(std::unique_ptr<VS_RouterMessage>& mes, unsigned long& milliseconds)
	{
		int ret;
		std::unique_ptr<VS_RouterMessage> mes_loc;
		switch (stateRcv)
		{
		case 0 :
			ret = pipe->GetReadResult( milliseconds );		if (ret < 0)	return ret;
			if (ret != sizeof(transport::MessageFixedPart))
			{	/* Here it will be necessary to throw off in TRACE */	return -1;	}
			if (!readHead.mark1 || readHead.version < 1
					|| readHead.head_length < sizeof(transport::MessageFixedPart) + 6)
			{	/* Here it will be necessary to throw off in TRACE */	return -1;	}
			readSize = readHead.head_length + readHead.body_length + 1;
			readBuffer = (unsigned char *)malloc( (size_t)readSize );
			if (!readBuffer)
			{	/* Here it will be necessary to throw off in TRACE */	return -1;	}
			*(transport::MessageFixedPart*)readBuffer = readHead;
			readSize -= sizeof(transport::MessageFixedPart);
			stateRcv = 1;
			if (!pipe->Read((void *)&readBuffer[sizeof(transport::MessageFixedPart)], readSize))
				return -1;
		case 1 :
			ret = pipe->GetReadResult( milliseconds );		if (ret < 0)	return ret;
			if (ret != readSize)
			{	/* Here it will be necessary to throw off in TRACE */
				free( (void *)readBuffer );		return -1;	}
			mes_loc = std::make_unique<VS_RouterMessage>(readBuffer, sizeof(transport::MessageFixedPart) + readSize);
			free(readBuffer);
			readBuffer = 0;
			if (!mes_loc->IsValid())
			{	/* Here it will be necessary to throw off in TRACE */
				return -1;
			}
			readSize = sizeof(transport::MessageFixedPart);
			stateRcv = 0;
			pipe->Read((void *)&readHead, sizeof(transport::MessageFixedPart));
			mes = std::move(mes_loc);
			return 1;
		default :	return -1;
	}	}
	// end of VS_TransportRouterService_Implementation::ReceiveMesAct

	int ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, bool allAdopted) override
	{
		int ret = -2;
		while (ret == -2)
		{
			std::chrono::steady_clock::duration wait_time = std::chrono::steady_clock::duration::max();
			ret = ReceiveMes(mes, wait_time, allAdopted);
		}
		return ret;
	}

	int ReceiveMes(std::unique_ptr<VS_RouterMessage>& mes, std::chrono::steady_clock::duration& wait_time, bool allAdopted) override
	{
		unsigned long milliseconds = clamp_cast<unsigned long>(std::chrono::duration_cast<std::chrono::milliseconds>(wait_time).count());
		VS_SCOPE_EXIT { wait_time = std::chrono::milliseconds(milliseconds); };

		if (tr->threadId == GetCurrentThreadId() || !hostInst)
		{
			if (!hostInst)
			{
				if (skipRecv>skipRecvMax)
				{
					return -1;
				}else
				{
					Sleep(0);
					++skipRecv;
					return -2;
				}
			}
			return -1;
		}
		if (allAdopted)
		{	HANDLE   hs[MAXIMUM_WAIT_OBJECTS];
go_again1:	memset( (void *)hs, 0, sizeof(hs) );
			DWORD   diffTick = GetTickCount(),
					cs = hostInst->tEnv->GetEvents( hs ),
					res = WaitForMultipleObjects( cs, hs, FALSE, milliseconds );
			if (res == WAIT_OBJECT_0)
			{	ResetEvent( hostInst->tEnv->adoptedEvent );
				if (!hostInst->tEnv->continueThread)
					return -1;
				else
				{	diffTick = GetTickCount() - diffTick;
					if (diffTick > milliseconds) {	milliseconds = 0;	return -2;	}
					milliseconds -= diffTick;	goto go_again1;
			}	}
			else if (res > WAIT_OBJECT_0 && res < (WAIT_OBJECT_0 + cs))
				return hostInst->tEnv->adoptedInsts[--res]->ReceiveMesAct( mes, milliseconds );
			else if (res == WAIT_TIMEOUT) {		milliseconds = 0;	return -2;	}
			else {	/* Here it will be necessary to throw off in TRACE */
				return -1;	}
		}
		else
		{	HANDLE   hs[] = { hostInst->tEnv->adoptedEvent, (HANDLE)pipe->OvReadEvent() };
go_again2:	DWORD   diffTick = GetTickCount(),
					res = WaitForMultipleObjects( 2, hs, FALSE, milliseconds );
			if (res == WAIT_OBJECT_0)
			{	ResetEvent( hostInst->tEnv->adoptedEvent );
				if (!hostInst->tEnv->continueThread)
					return -1;
				else
				{	diffTick = GetTickCount() - diffTick;
					if (diffTick > milliseconds) {	milliseconds = 0;	return -2;	}
					milliseconds -= diffTick;	goto go_again2;
			}	}
			else if (res == (WAIT_OBJECT_0 + 1))	return ReceiveMesAct( mes, milliseconds );
			else if (res == WAIT_TIMEOUT) {		milliseconds = 0;	return -2;	}
			else {	/* Here it will be necessary to throw off in TRACE */
				return -1;	}
	}	}
	// end of VS_TransportRouterService_Implementation::ReceiveMes
};
// end of VS_TransportRouterService_Implementation struct

struct VS_PoolThreadsTask_Implementation : public VS_PoolThreadsTask::Impl
{
	explicit VS_PoolThreadsTask_Implementation(VS_PoolThreadsTask* task_)
		: task(task_)
	{
	}
	void FinishTask() override;

	VS_PoolThreadsTask* task;
};

#endif  // VS_TRANSPORT_ROUTER_SERVICE_TYPES_H
