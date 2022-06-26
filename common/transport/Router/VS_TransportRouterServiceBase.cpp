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
/// \file VS_TransportRouterService.cpp
/// \brief
/// \note
///

#include "VS_TransportRouterServiceBase.h"
#include "VS_RouterMessage.h"
#include "std-generic/cpplib/string_view.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

VS_TransportRouterServiceBase::VS_TransportRouterServiceBase()
	: m_TimeInterval(std::chrono::steady_clock::duration::max())
	, m_processingThread(0)
{
}

VS_TransportRouterServiceBase::~VS_TransportRouterServiceBase() = default;

void VS_TransportRouterServiceBase::Thread()
{
	m_processingThread.store(vs::GetThreadID(), std::memory_order_release);

	m_messBlock = vs::make_unique<VS_MessageFloodBlock>(VS_TransportRouterServiceBase::OurService());

	auto startTime = std::chrono::steady_clock::now();
	while (1) {
		std::chrono::steady_clock::duration wait_time = m_TimeInterval; // make a copy, because it will be updated
		std::unique_ptr<VS_RouterMessage> recvMess;
		const int res = ReceiveMes(recvMess, wait_time);
		if (res == -1)
		{
			m_processingThread.store(0, std::memory_order_release);
			std::atomic_thread_fence(std::memory_order_acquire); // don't reorder anything above this store as it may brake logic
		}

		auto currTime = std::chrono::steady_clock::now();

		if (res==1) {
			bool isBlocked = false;

			auto cid = recvMess->SrcCID_sv();
			if (cid.empty())
				cid = recvMess->SrcUser_sv();
			if (!cid.empty())
				isBlocked = m_messBlock->Add(cid, currTime);

			if (!isBlocked)
				Processing(std::move(recvMess));
		}

		std::vector<CallFunction> toCall = std::move(*m_callFucntions.lock());
		for (const auto& f : toCall)
		{
			auto delay = std::chrono::steady_clock::now() - f.first;
			if (delay > std::chrono::seconds(1))
				dstream4 << "Call function delay is " << delay.count() << "seconds\n";
			f.second();	// call body of function
		};

		if (res==-1)
		{
			printf(" Async %s is exit\n", VS_TransportRouterServiceBase::OurService());
			break;
		}

		if (currTime - m_LastTimerTime > m_TimeInterval) {
			m_LastTimerTime = currTime;
			Timer(std::chrono::duration_cast<std::chrono::milliseconds>(currTime - startTime).count());
			VS_ReadDebugKeys();
		}
		m_messBlock->UpdateState(currTime);
	}
}

bool VS_TransportRouterServiceBase::CallInProcessingThread(std::function<void()>&& f)
{
	if (m_processingThread.load(std::memory_order_acquire) == 0)
		return false; // processing thread have exited and won't call any new functions

	m_callFucntions->emplace_back(std::chrono::steady_clock::now(), std::move(f));

	char dummy(0);
	auto msg = new VS_RouterMessage(OurService(), "CallFunction", OurService(), 0, 0, OurEndpoint(), OurEndpoint(), 30000, &dummy, sizeof(char));

	if (!PostMes(msg))
	{
		delete msg;
		return false;
	}
	else
		return true;
}

bool VS_TransportRouterServiceBase::IsInProcessingThread() const
{
	const auto id = m_processingThread.load(std::memory_order_acquire);
	return id == 0 || id == vs::GetThreadID();
	// TODO: Remove the check for 0 above.
	// Thread safety can not be guaranteed in that case because processing thread may start while we are executing code that expects to be executed in processing thread.
	// The check was initially added to fix status tests (StatusTestFixture).
}

int VS_TransportRouterServiceBase::GetProcessingThreadId() const
{
	return m_processingThread.load(std::memory_order_relaxed);
}