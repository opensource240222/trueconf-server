#pragma once

#include <atomic>
#include <cassert>

namespace net { namespace ur {

namespace detail {

class OpBase
{
public:
	bool Cancel()
	{
		state expected = state::waiting;
		return m_state.compare_exchange_strong(expected, state::canceled, std::memory_order_release, std::memory_order_relaxed);
	}

protected:
	virtual ~OpBase()
	{
		// Assert on the next line means that user handler was never called.
		assert(m_state.load(std::memory_order_relaxed) == state::completed);
	}

	enum class state
	{
		waiting,
		canceled,
		completed,
	};

	std::atomic<state> m_state {state::waiting};
};

}

}}
