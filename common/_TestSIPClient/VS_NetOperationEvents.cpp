#include "VS_NetOperationEvents.h"
#include <memory.h>

VS_NetOperationEvents::VS_NetOperationEvents():	
	m_buffer (0),
	m_trasfered( 0)
{
	m_bodySize = (sizeof(VS_NetOperationEvents));
	memset(&m_ov,0,sizeof(m_ov));

}

VS_NetOperationEvents::~VS_NetOperationEvents()
{}
