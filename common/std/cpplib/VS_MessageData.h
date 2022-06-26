#pragma once

class VS_MessageData
{
	unsigned long	m_messageType;
	void*			m_mess;
	unsigned long	m_size_mess;
public:
	VS_MessageData(const unsigned long type,void *mess,const unsigned long size_mess);
	virtual ~VS_MessageData()
	{
	}
	void *GetMessPointer(unsigned long &type, unsigned long &sz) const;

};