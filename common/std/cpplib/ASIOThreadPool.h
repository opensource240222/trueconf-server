#pragma once

#include "std-generic/cpplib/ASIOThreadPool.h"

namespace vs {

class ASIOThreadPoolBalancing : public ASIOThreadPool
{
public:
	typedef std::function<void(bool)> Callback;

	ASIOThreadPoolBalancing(Callback registerCallback, unsigned n_threads = 0, string_view thread_name_prefix = {});
	~ASIOThreadPoolBalancing();

protected:
	void OnThreadStart() override;
	void OnThreadExit() override;

private:
	Callback m_registerThreadCallback;
};

}
