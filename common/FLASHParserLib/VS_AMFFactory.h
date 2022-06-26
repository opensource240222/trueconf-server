#pragma once
#include "VS_AMFBase.h"

#include "VS_AMFPacket.h"
#include "VS_FLASHSessionData.h"

class VS_AMFFactory
{
private:
	static VS_AMFFactory*	m_factory;

protected:
	VS_AMFFactory();

public:
	~VS_AMFFactory();

	static VS_AMFFactory* Instance();

	VS_AMFBase* CreateDecoder(const void* in);
	VS_AMFBase* CreateDecoder(const unsigned int content_type);
};