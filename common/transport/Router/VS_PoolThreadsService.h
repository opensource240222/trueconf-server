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
 * \file VS_PoolThreadsService.h
 * \brief (c) 2002 Visicron Inc.  http://www.visicron.net/ \n
 * \author A.Slavetsky
 * \version
 * \date 06.06.03
 *
 ****************************************************************/

#pragma once

#include "VS_TransportRouterServiceBase.h"

#define   VS_POOL_THREADS_SERVICE_NAME   "POOL_THREADS"

//////////////////////////////////////////////////////////////////////////////////////////

class VS_PoolThreadsTask : public virtual VS_TransportRouterServiceBase
{
public:
	class Impl
	{
	public:
		virtual ~Impl() {};

		virtual void FinishTask() = 0;
	};

	virtual void Run() = 0;
	virtual bool IsValid() const { return true; }
	bool Init(const char* /*our_endpoint*/, const char* /*our_service*/, const bool /*permittedAll*/) override { return true; }

	Impl* imp = nullptr;

protected:
	void Thread() override
	{
		if (!imp)
			return;
		Run();
		imp->FinishTask();
	}

};
// end VS_PoolThreadsTask class

//////////////////////////////////////////////////////////////////////////////////////////

class VS_PoolThreadsService : public VS_TransportRouterServiceBase
{
public:
		VS_PoolThreadsService( const unsigned maxPoolThreads = 100,
								const unsigned maxQueueTasks = 500,
								const unsigned nPriorities = 3,
								const unsigned tasksLifetimeSec = 30);
	virtual ~VS_PoolThreadsService( void );
	struct VS_PoolThreadsService_Implementation   *imp;
	bool	IsValid( void ) const {		return imp != 0;	}
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	bool Timer(unsigned long tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;
};
// end VS_PoolThreadsService class

//////////////////////////////////////////////////////////////////////////////////////////
