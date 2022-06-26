#include "std/cpplib/ASIOThreadPool.h"

namespace vs {

ASIOThreadPoolBalancing::ASIOThreadPoolBalancing(Callback registerCallback, unsigned n_threads, string_view thread_name_prefix) :
						ASIOThreadPool(n_threads, thread_name_prefix),
						m_registerThreadCallback(std::move(registerCallback))
{
}

ASIOThreadPoolBalancing::~ASIOThreadPoolBalancing()
{
	Stop(); // We need to call Stop() here to ensure that exiting threads will call our implementation of OnThreadExit().
}

void ASIOThreadPoolBalancing::OnThreadStart()
{
	m_registerThreadCallback(true);
}

void ASIOThreadPoolBalancing::OnThreadExit()
{
	m_registerThreadCallback(false);
}

}
