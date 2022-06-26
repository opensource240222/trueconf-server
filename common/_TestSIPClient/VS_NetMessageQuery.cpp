///////////////////////////////////////////////////////////////////////////////
///			VS_NetMessageQuery
///////////////////////////////////////////////////////////////////////////////
#include "VS_NetMessageQuery.h"

VS_NetMessageQuery::VS_NetMessageQuery()
{
}
VS_NetMessageQuery::~VS_NetMessageQuery()
{
}
bool VS_NetMessageQuery::AddMessageToHeader(VS_Buffer &mess)
{
	m_query.push_back( mess );
	return true;
}
bool VS_NetMessageQuery::GetMessageFromBack( VS_Buffer & mess)
{
	if (m_query.empty())
		return false;
	mess = *m_query.begin();
	return true;
}
bool VS_NetMessageQuery::RemoveMessageFromBack( VS_Buffer * mess)
{
	if (m_query.empty())
		return false;
	m_it = m_query.begin();
	if (mess)
	{
		*mess = *m_it;
	}
	m_query.erase(m_it);
	return true;
}
bool VS_NetMessageQuery::IsEmpty()
{
	return m_query.empty();
}
unsigned int VS_NetMessageQuery::GetMessagesNumber()
{
	if (IsEmpty())
		return 0;
	return (unsigned int)(m_query.capacity() / sizeof(VS_Buffer));
}