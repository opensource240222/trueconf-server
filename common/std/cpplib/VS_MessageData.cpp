#include "VS_MessageData.h"

VS_MessageData::VS_MessageData(const unsigned long type,void *mess,const unsigned long size_mess) : m_messageType(type), m_mess(mess),m_size_mess(size_mess)
{}
void * VS_MessageData::GetMessPointer(unsigned long &type, unsigned long &sz) const
{
	type = m_messageType;
	sz = m_size_mess;
	return m_mess;
}