#include "VS_SendFrameQueueBase.h"
#include "VS_SendFrameQueueNhp.h"
#include "VS_SendFrameQueueTCP.h"

VS_SendFrameQueueBase* VS_SendFrameQueueBase::Factory(bool useNhp, bool useSVC)
{
	VS_SendFrameQueueBase* p = 0;
	if (useNhp)	p = new VS_SendFrameQueueNhp(useSVC);
	else		p = new VS_SendFrameQueueTCP(useSVC);
	return p;
}

void VS_SendFrameQueueBase::SetLimitSize(int limitSize)
{
	m_LimitSize = limitSize;
	if (m_LimitSize < VIDEODATASIZE_MIN) m_LimitSize = VIDEODATASIZE_MIN;
	if (m_LimitSize > VIDEODATASIZE_MAX) m_LimitSize = VIDEODATASIZE_MAX;
}
