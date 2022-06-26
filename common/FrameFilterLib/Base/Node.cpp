#include "FrameFilterLib/Base/Node.h"
#include "FrameFilterLib/Base/TraceLog.h"

namespace ffl {

std::atomic<unsigned> Node::last_chain_id(0);

Node::~Node()
{
	if (auto log = m_log.lock())
		log->TraceDeletion(this);
}

bool Node::SetChainID(unsigned value, bool replace)
{
	if (m_chain_id != 0 && !replace)
		return false;

	if (value != 0)
		m_chain_id = value;
	else
		m_chain_id = ++last_chain_id;
	return true;
}

void Node::EnableTrace(const std::weak_ptr<TraceLog>& log)
{
	m_log = log;
}

}
