
#pragma once
#include <vector>
#include "../acs/Lib/VS_Buffer.h"

class VS_NetMessageQuery
{
public:
	VS_NetMessageQuery();
	~VS_NetMessageQuery();
	bool AddMessageToHeader( VS_Buffer & mess);
	bool GetMessageFromBack( VS_Buffer & mess);
	bool RemoveMessageFromBack( VS_Buffer * mess = 0);
	bool IsEmpty();
	unsigned int GetMessagesNumber();
protected:
	std::vector<VS_Buffer> m_query;
	std::vector<VS_Buffer>::iterator m_it;
};

enum TNetConnectionState
{
	e_connectionAborted = 0x01,
	e_connecting = 0x02,
	e_connected  = 0x04,
	e_reading    = 0x08,
	e_writing    = 0x10
};


class VS_NetConnectionInterface
{
public:		
	virtual void UpdateState(int aField=0,unsigned int bField=0, void * buffer=0) = 0;
protected:
private:
};