#if defined(_WIN32) // Not ported yet

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
 ****************************************************************************************
 * \file VS_PoolThreadsService.cpp
 * \brief (c) 2002 Visicron Inc.  http://www.visicron.net/ \n
 * \author A.Slavetsky
 * \version
 * \date 06.06.03
 *
 ****************************************************************************************/

#include <stdio.h>

#include "VS_PoolThreadsService.h"
#include "VS_TransportRouterServiceTypes.h"
#include "../../std/cpplib/VS_Utils.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"
#include "std-generic/clib/vs_time.h"
#include "../../std/debuglog/VS_Debug.h"

#define   VS_POOL_THREADS_PUT_TASK   "KSAT_TUP_SDAERHT_LOOP_SV"
#define   VS_POOL_THREADS_END_TASK   "KSAT_DNE_SDAERHT_LOOP_SV"
#define   VS_POOL_THREAD_NAME_PREFIX   "PT:"
#define   VS_MIN_PT_THREADS   10
#define   VS_MAX_PT_THREADS   200
#define   VS_MIN_PT_QUEUE_TASKS   100
#define   VS_MAX_PT_QUEUE_TASKS   1000
#define   VS_MIN_PT_PRIORITIES   1
#define   VS_MAX_PT_PRIORITIES   10
#define   VS_MIN_PT_DEF_LIFETIME   10
#define   VS_MAX_PT_DEF_LIFETIME   300

#define DEBUG_CURRENT_MODULE VS_DM_LOGS

//////////////////////////////////////////////////////////////////////////////////////////

struct VS_TaskArgs
{
	struct Head
	{	unsigned   chkSum, sz, nameExtension;
		VS_PoolThreadsTask   *task;		unsigned   lifetimeSec, priority;
	} head;
	// end VS_TaskArgs::Head struct

	static inline VS_TaskArgs *Constructor( VS_PoolThreadsTask *task,
										const char *nameExtension, const unsigned lifetimeSec,
										const unsigned priority )
	{
		const unsigned   sz_nameExtension = !nameExtension ? 0 : (unsigned)strlen( nameExtension ) + 1,
							sz = sizeof(Head) + sz_nameExtension;
		VS_TaskArgs   *args = (VS_TaskArgs *)malloc( (size_t)sz );
		if (!args)	return 0;		memset( (void *)args, 0, (size_t)sz );
		Head   &head = *(Head *)args;	unsigned   ind = sizeof(Head);	char   *bf = &((char *)args)[ind];
		if (nameExtension) {	strcpy( bf, nameExtension );	head.nameExtension = ind;	bf += sz_nameExtension;		ind += sz_nameExtension;	}
		if (ind != sz) {	free( (void *)args );	return 0;	}
		head.sz = sz;	head.task = task;	head.lifetimeSec = lifetimeSec;		head.priority = priority;
		head.chkSum = VS_SimpleChkSum( (void *)&head.sz, sz - sizeof(head.chkSum) );
		return args;
	}
	// end VS_TaskArgs::Constructor

	inline bool IsValid( void ) const
	{	return (!head.nameExtension || head.nameExtension == sizeof(Head)) && head.sz >= sizeof(Head) && (head.chkSum == VS_SimpleChkSum( (void *)&head.sz, head.sz - sizeof(head.chkSum) ));		}
	// end VS_TaskArgs::IsValid

	inline unsigned Sz( void ) const {		return head.sz;		}
	// end VS_TaskArgs::Sz

	inline VS_PoolThreadsTask *Task( void ) const {		return head.task;	}
	// end VS_TaskArgs::Task

	inline char *NameExtension( void ) const {	return !head.nameExtension ? 0 : (char *)this + head.nameExtension;		}
	// end VS_TaskArgs::NameExtension

	inline unsigned LifetimeSec( void ) const {		return head.lifetimeSec;	}
	// end VS_TaskArgs::LifetimeSec

	inline unsigned Priority( void ) const {	return head.priority;	}
	// end VS_TaskArgs::Priority
};
// end VS_TaskArgs struct

//////////////////////////////////////////////////////////////////////////////////////////

struct VS_PoolThreadsService_Implementation
{
	struct QueueTask
	{	unsigned long   startTick;
		VS_PoolThreadsTask   *task;		char   *nameExtension;	unsigned long   lifeTicks;
		QueueTask   *next;
	};	// end QueueTask struct
	struct TasksQueue
	{	unsigned   number;		QueueTask   *tail, *head;
	};	// end TasksQueue struct
	struct RunTask
	{
		unsigned long cyclicNum, startTick;
		VS_PoolThreadsTask   *task;		char   *nameExtension;	unsigned long   lifeTicks;
	};	// end RunTask struct
	VS_PoolThreadsService_Implementation( VS_TransportRouterServiceBase *serv,
											const unsigned maxPoolThreads,
											const unsigned maxQueueTasks,
											const unsigned nPriorities,
											const unsigned tasksLifetimeSec ) :
			isValid(false), serv(serv), tr(0), maxQueueTasks(maxQueueTasks),
			maxPoolThreads(maxPoolThreads), nPriorities(nPriorities),
			tasksLifetimeSec(tasksLifetimeSec), nPoolThreads(0), nQueueTasks(0),
			tq(0), rt(0), cyclicNum(0)
	{
		VS_RegistryKey cfg_root(false, CONFIGURATION_KEY);
		if (cfg_root.IsValid())
		{	long   val = 0;
			if (cfg_root.GetValue(&val, sizeof(val), VS_REG_INTEGER_VT, PTH_MAX_THREADS_TAG) == sizeof(val)
					&& val >= VS_MIN_PT_THREADS && val <= VS_MAX_PT_THREADS)
				VS_PoolThreadsService_Implementation::maxPoolThreads = (unsigned)val;
			if (cfg_root.GetValue(&(val = 0), sizeof(val), VS_REG_INTEGER_VT, PTH_MAX_QUEUE_TASKS_TAG) == sizeof(val)
					&& val >= VS_MIN_PT_QUEUE_TASKS && val <= VS_MAX_PT_QUEUE_TASKS)
				VS_PoolThreadsService_Implementation::maxQueueTasks = (unsigned)val;
			if (cfg_root.GetValue(&(val = 0), sizeof(val), VS_REG_INTEGER_VT, PTH_N_PRIORITIES_TAG) == sizeof(val)
					&& val >= VS_MIN_PT_PRIORITIES && val <= VS_MAX_PT_PRIORITIES)
				VS_PoolThreadsService_Implementation::nPriorities = (unsigned)val;
			if (cfg_root.GetValue(&(val = 0), sizeof(val), VS_REG_INTEGER_VT, PTH_DEF_LIFETIME_TAG) == sizeof(val)
					&& val >= VS_MIN_PT_DEF_LIFETIME && val <= VS_MAX_PT_DEF_LIFETIME)
				VS_PoolThreadsService_Implementation::tasksLifetimeSec = (unsigned)val;
		}
		if (VS_PoolThreadsService_Implementation::maxPoolThreads < VS_MIN_PT_THREADS || VS_PoolThreadsService_Implementation::maxPoolThreads > VS_MAX_PT_THREADS
				|| VS_PoolThreadsService_Implementation::maxQueueTasks < VS_MIN_PT_QUEUE_TASKS || VS_PoolThreadsService_Implementation::maxQueueTasks > VS_MAX_PT_QUEUE_TASKS
				|| VS_PoolThreadsService_Implementation::nPriorities < VS_MIN_PT_PRIORITIES || VS_PoolThreadsService_Implementation::nPriorities > VS_MAX_PT_PRIORITIES
				|| VS_PoolThreadsService_Implementation::tasksLifetimeSec < VS_MIN_PT_DEF_LIFETIME || VS_PoolThreadsService_Implementation::tasksLifetimeSec > VS_MAX_PT_DEF_LIFETIME)
			return;
		size_t   sz = sizeof(TasksQueue) * VS_PoolThreadsService_Implementation::nPriorities;
		tq = (TasksQueue *)malloc( sz );	if (!tq)	return;
		memset( (void *)tq, 0, sz );
		sz = sizeof(RunTask) * VS_PoolThreadsService_Implementation::maxPoolThreads;
		rt = (RunTask *)malloc( sz );		if (!rt)	return;
		memset( (void *)rt, 0, sz );
		isValid = true;
	}
	// end VS_PoolThreadsService_Implementation::VS_PoolThreadsService_Implementation

	~VS_PoolThreadsService_Implementation( void )
	{
		if (tq)
		{	if (nQueueTasks)
			{	for (unsigned i = 0; i < nPriorities; ++i)
				{	QueueTask   *tail = tq[i].tail, *old_tail;
					while (tail)
					{	delete tail->task;	old_tail = tail;
						tail = tail->next;	free( (void *)old_tail );
			}	}	}
			free( (void *)tq );
		}
		if (rt) free(rt);
	}
	// end VS_PoolThreadsService_Implementation::~VS_PoolThreadsService_Implementation

	bool   isValid;
	VS_TransportRouterServiceBase   *serv;		VS_TransportRouter_CallService   *tr;
	unsigned   maxPoolThreads, maxQueueTasks, tasksLifetimeSec, nPriorities;
	unsigned   nPoolThreads, nQueueTasks;		TasksQueue   *tq;
	unsigned long   cyclicNum;		RunTask   *rt;

	inline bool ProcessingQueue( void )
	{
		if (nPoolThreads >= maxPoolThreads || !nQueueTasks)		return true;
		for (unsigned i = 0; i < maxPoolThreads; ++i)
		{	RunTask   &rt = VS_PoolThreadsService_Implementation::rt[i];
			if (!rt.task)
			{	for (unsigned j = 0; j < nPriorities; ++j)
				{	TasksQueue   &tq = VS_PoolThreadsService_Implementation::tq[j];
					if (tq.number)
					{
						QueueTask   *qt = tq.tail;
						unsigned long   startTick = qt->startTick;
						VS_PoolThreadsTask   *task = qt->task;
						char   *nameExtension = qt->nameExtension;
						unsigned long   lifeTicks = qt->lifeTicks,
								cyclicNum = VS_PoolThreadsService_Implementation::cyclicNum;
						if (!++cyclicNum)	++cyclicNum;

						char   name[VS_TRANSPORT_MAX_SIZE_SERVICE_NAME + 1];
						char chtime[32];
						time_t curt;
						time(&curt);
						tm curt_tm;
						strftime(chtime, 30, "%d/%m/%Y %H:%M:%S", localtime_r(&curt, &curt_tm));

						sprintf(name, "%s %03u %08lX %.16s - %.30s", VS_POOL_THREAD_NAME_PREFIX, i + 1, cyclicNum,  nameExtension ? nameExtension : "unk", chtime);
						if (tr->AddService(name, task))
						{	tq.tail = qt->next;		free( (void *)qt );
							if (!--tq.number)	tq.tail = tq.head = 0;
							--nQueueTasks;
							rt.cyclicNum = cyclicNum;
							rt.startTick = startTick;	rt.task = task;
							rt.nameExtension = nameExtension;	rt.lifeTicks = lifeTicks;
							VS_PoolThreadsService_Implementation::cyclicNum = cyclicNum;
							task->VS_PoolThreadsTask::imp = new VS_PoolThreadsTask_Implementation(task);
							task->SetThread();
						}
						return true;
		}	}	}	}
		return false;
	}
	// end VS_PoolThreadsService_Implementation::ProcessingQueue

	inline bool Processing( std::unique_ptr<VS_RouterMessage>&&recvMess )
	{
		const char   *topic = recvMess->AddString();
		if (!topic) {	/* Here it will be necessary to throw off in TRACE */	}
		else if (!strcmp( VS_POOL_THREADS_PUT_TASK, topic ))
		{
			const auto args = reinterpret_cast<const VS_TaskArgs*>(recvMess->Body());
			if (!args->IsValid())
			{	/* Here it will be necessary to throw off in TRACE */	}
			else
			{	VS_PoolThreadsTask   *task = args->Task();
				const char   *nameExtension = args->NameExtension();
				unsigned   priority = args->Priority(), lifetimeSec = args->LifetimeSec();
				if (!task || !priority || !lifetimeSec)
				{	/* Here it will be necessary to throw off in TRACE */	}
				else if (nQueueTasks >= maxQueueTasks)
				{	/* Here it will be necessary to throw off in TRACE */	}
				else
				{	if (priority > nPriorities)		priority = nPriorities;
					if (lifetimeSec > tasksLifetimeSec)		lifetimeSec = tasksLifetimeSec;
					TasksQueue   &tq = VS_PoolThreadsService_Implementation::tq[--priority];
					QueueTask   *qt = (QueueTask *)malloc( sizeof(QueueTask) );
					if (!qt)
					{	/* Here it will be necessary to throw off in TRACE */	}
					else
					{	qt->startTick = GetTickCount();	qt->task = task;	task = 0;
						qt->nameExtension = !nameExtension ? 0 : _strdup( nameExtension );
						qt->lifeTicks = lifetimeSec * 1000;		qt->next = 0;
						if (!tq.number)		tq.tail = qt;
						else	tq.head->next = qt;
						tq.head = qt;	++tq.number;	++nQueueTasks;
				}	}
				if (task)
				{	delete task;
		}	}	}
		else if (!strcmp( VS_POOL_THREADS_END_TASK, topic ))
		{	const char   *name = recvMess->SrcService();
			if (!name || !strncmp( name, VS_POOL_THREAD_NAME_PREFIX, (sizeof(VS_POOL_THREAD_NAME_PREFIX) - 1) ))
			{	unsigned   i = ~0;		unsigned long   cyclicNum = 0;
				if (sscanf( &name[sizeof(VS_POOL_THREAD_NAME_PREFIX)], "%03u %lX", &i, &cyclicNum ) == 2
						&& --i < maxPoolThreads)
				{	RunTask   &rt = VS_PoolThreadsService_Implementation::rt[i];
					unsigned long diff = GetTickCount() - rt.startTick;
					if (diff > rt.lifeTicks)
						dprint0("Long task (%ldms > %ldms): %s\n", diff, rt.lifeTicks, name);
					if (rt.task && rt.cyclicNum == cyclicNum)
					{
						if (!tr->RemoveService(name))
						{	/* Here it will be necessary to throw off in TRACE */	}
						delete rt.task->VS_PoolThreadsTask::imp;
						delete rt.task;
						if (rt.nameExtension)	free( (void *)rt.nameExtension );
						memset( (void *)&rt, 0, sizeof(RunTask) );
				}	}
				else
				{	/* Here it will be necessary to throw off in TRACE */
			}	}
			else
			{	/* Here it will be necessary to throw off in TRACE */
		}	}
		else {	/* Here it will be necessary to throw off in TRACE */	}
		return ProcessingQueue();
	}
	// end VS_PoolThreadsService_Implementation::Processing

	inline bool Init( const char *our_endpoint, const char *our_service )
	{	return serv->imp && (tr = static_cast<VS_TransportRouterServiceBase_Implementation*>(serv->imp.get())->tr);		}
	// end VS_PoolThreadsService_Implementation::Init

	inline bool Timer( unsigned long tickcount )
	{
		for (unsigned j = 0; j < nPriorities; ++j)
		{	TasksQueue   &tq = VS_PoolThreadsService_Implementation::tq[j];
			if (tq.number)
			{	QueueTask   *qt = tq.tail, *qt_prev = 0;
				while (qt)
				{	if (!qt->startTick)		qt->startTick = GetTickCount();
					else if ((GetTickCount() - qt->startTick) > qt->lifeTicks)
					{
						if (nQueueTasks>0)
							--nQueueTasks;
						dprint0("PoolTaskSRV: del task from queue by lifeTicks=%ld (startTick=%ld), nQueueTasks = %d\n", qt->lifeTicks, qt->startTick, nQueueTasks);
						VS_PoolThreadsTask   *task = qt->task;
						/* Here it will be necessary to throw off in TRACE */
						QueueTask   *qt_del = qt;	qt = qt->next;
						if (!qt_prev)	tq.tail = qt;
						else			qt_prev->next = qt;
						if (!qt)		tq.head = qt_prev;
						if (!--tq.number) {		tq.tail = tq.head = 0;	}
						delete task;
						if (qt_del->nameExtension)	free( (void *)qt_del->nameExtension );
						free( (void *)qt_del );
					}
					else
					{	qt_prev = qt;	qt = qt->next;
		}	}	}	}
		for (unsigned i = 0; i < maxPoolThreads; ++i)
		{	RunTask   &rt = VS_PoolThreadsService_Implementation::rt[i];
			if (rt.task)
			{	if (!rt.startTick)	rt.startTick = GetTickCount();
				else if ((GetTickCount() - rt.startTick) > rt.lifeTicks)
				{
					VS_PoolThreadsTask   *task = rt.task;
					if (task
					 && task->VS_TransportRouterServiceBase::imp
					 && static_cast<VS_TransportRouterService_Implementation*>(task->VS_TransportRouterServiceBase::imp.get())->tEnv
					 && static_cast<VS_TransportRouterService_Implementation*>(task->VS_TransportRouterServiceBase::imp.get())->tEnv->thread) // kt: we cannot remove service, because it is still running
					{
						if (WaitForSingleObject(static_cast<VS_TransportRouterService_Implementation*>(task->VS_TransportRouterServiceBase::imp.get())->tEnv->thread, 0) != WAIT_OBJECT_0)
							continue;
					}
					if (!tr->RemoveService(task->OurService()/*name*/))
					{	/* Here it will be necessary to throw off in TRACE */	}
					delete rt.task;
					if (rt.nameExtension)	free( (void *)rt.nameExtension );
					memset( (void *)&rt, 0, sizeof(RunTask) );
					/* Here it will be necessary to throw off in TRACE */
		}	}	}
		return true;
	}
	// end VS_PoolThreadsService_Implementation::Timer
};
// end VS_PoolThreadsService_Implementation struct

//////////////////////////////////////////////////////////////////////////////////////////

VS_PoolThreadsService::VS_PoolThreadsService( const unsigned maxPoolThreads,
												const unsigned maxQueueTasks,
												const unsigned nPriorities,
												const unsigned tasksLifetimeSec )
												: imp(0)
{
	m_TimeInterval = std::chrono::seconds(1);
	imp = new VS_PoolThreadsService_Implementation( this, maxPoolThreads, maxQueueTasks, nPriorities, tasksLifetimeSec );
	if (!imp)	return;
	if (!imp->isValid) {	delete imp;		imp = 0;	}
}
// end VS_PoolThreadsService::VS_PoolThreadsService

VS_PoolThreadsService::~VS_PoolThreadsService( void ) {		if (imp) 	delete imp;		}
// end VS_PoolThreadsService::~VS_PoolThreadsService

bool VS_PoolThreadsService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess )
{	return !imp ? false : imp->Processing( std::move(recvMess) );	}
// end VS_PoolThreadsService::Processing

bool VS_PoolThreadsService::Init( const char *our_endpoint, const char *our_service, const bool permittedAll )
{	return !imp ? false : imp->Init( our_endpoint, our_service );	}
// end VS_PoolThreadsService::Init

bool VS_PoolThreadsService::Timer( unsigned long tickcount )
{	return !imp ? false : imp->Timer( tickcount );	}
// end VS_PoolThreadsService::Timer

//////////////////////////////////////////////////////////////////////////////////////////

void VS_PoolThreadsTask_Implementation::FinishTask()
{
	/*
	VS_RouterMessage   *mes = new VS_RouterMessage(task->OurEndpoint(),
									task->OurService(), VS_POOL_THREADS_END_TASK,
									task->OurEndpoint(), VS_POOL_THREADS_SERVICE_NAME, ~0,
									(const void *)&task, sizeof(task) );
	*/
	VS_RouterMessage   *mes = new VS_RouterMessage(0,
									task->OurService(), VS_POOL_THREADS_END_TASK,
									0, VS_POOL_THREADS_SERVICE_NAME, ~0,
									(const void *)&task, sizeof(task),0, 0, task->OurEndpoint(), task->OurEndpoint());
	if (!task->SendMes(mes))
	{	delete mes;
		/* Here it will be necessary to throw off in TRACE */
	}
}
// end VS_PoolThreadsTask::Thread

//////////////////////////////////////////////////////////////////////////////////////////

bool VS_TransportRouterServiceBase_Implementation::PutTask(VS_PoolThreadsTask* task, const char* nameExtension,
									const unsigned lifetimeSec, const unsigned priority )
{
	if (!task || !priority || !lifetimeSec)		return false;
	bool   ret = false;		VS_TaskArgs   *args = 0;	VS_RouterMessage   *mes = 0;
	args = VS_TaskArgs::Constructor( task, nameExtension, lifetimeSec, priority );
	if (!args || !args->IsValid())		goto go_ret;
	/*
	mes = new VS_RouterMessage( OurEndpoint(),
								VS_POOL_THREADS_SERVICE_NAME, VS_POOL_THREADS_PUT_TASK,
								OurEndpoint(), VS_POOL_THREADS_SERVICE_NAME, ~0,
								(const void *)args, (const unsigned long)args->Sz() );
	*/
	mes = new VS_RouterMessage( 0,
								VS_POOL_THREADS_SERVICE_NAME, VS_POOL_THREADS_PUT_TASK,
								0, VS_POOL_THREADS_SERVICE_NAME, ~0,
								(const void *)args, (const unsigned long)args->Sz(), 0, 0, OurEndpoint(), OurEndpoint());
	if (!mes || !mes->IsValid())	goto go_ret;
	ret = PostMes( mes );		if (ret)	mes = 0;
go_ret:
	if (mes)	delete mes;		if (args)	free( (void *)args );
	return ret;
}
// end VS_TransportRouterServiceBase_Implementation::PutTask

//////////////////////////////////////////////////////////////////////////////////////////

#endif
