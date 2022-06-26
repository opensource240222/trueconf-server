#pragma once

#include "std-generic/compat/memory.h"

#include <atomic>
#include <cstring>
#include <memory>

namespace ffl {

class TraceLog;

class Node : public vs::enable_shared_from_this<Node>
{
public:
	static const size_t max_name_length = 20;

	virtual ~Node();

	const char* Name() const { return m_name; }
	unsigned ChainID() const { return m_chain_id; }

	virtual bool SetChainID(unsigned value = 0, bool replace = false);
	virtual void EnableTrace(const std::weak_ptr<TraceLog>& log);

protected:

	Node()
		: m_chain_id(0)
	{
		m_name[0] = '\0';
	}

	template <size_t N>
	void SetName(const char (&name)[N])
	{
		static_assert(N - 1 <= max_name_length, "Node name too long");
		std::memcpy(m_name, name, N - 1);
		m_name[N - 1] = '\0';
	}

	std::weak_ptr<TraceLog> m_log;
	unsigned m_chain_id;

private:
	char m_name[max_name_length + 1];

	static std::atomic<unsigned> last_chain_id;
};

}
