#include "VS_NetReactor.h"
#include "VS_NetRouterEvents.h"
#include "VS_NetHandlerFactoryEvents.h"
#include "VS_NetRouter.h"

///////////////////////////////////////////////////////////////////////////////
VS_NetReactor::VS_NetReactor():m_factory(0),m_router(0)
{
}
///////////////////////////////////////////////////////////////////////////////
VS_NetReactor::~VS_NetReactor()
{
}
///////////////////////////////////////////////////////////////////////////////

int VS_NetReactor::Init()
{
	VS_NetHandlerFactory * ptr = 0;

	if (e_ok!=(res=GetHandlerFactory( ptr )))
		return res;

	if (!m_factory)
		return e_bad;

	if (e_ok!=(res=m_factory->CreateRouter( m_router )))
		return res;
	
	if (m_router)	return m_router->Start(this);
	return e_null;
}
///////////////////////////////////////////////////////////////////////////////
//##ModelId=447836B70259
int VS_NetReactor::RegisterHandler(int type, VS_NetEventHandler *handler)
{
	if (!m_router) 
		return e_badObjectState;
	VS_NetOperation * theOp = 0;

	if (e_ok!=handler->GetHandle(theOp))
		return e_bad;

	if (e_ok!=(res=m_router->AddOperation( theOp )))
	{
		return res;
	}
	
	m_handlers[ theOp->m_index ] = handler;
	return e_ok;
}
///////////////////////////////////////////////////////////////////////////////
//##ModelId=447836B7025A
int VS_NetReactor::RemoveHandler(VS_NetOperation * aOperation)
{
	if (!aOperation) 
		return e_bad;
	m_router->RemoveOperation( aOperation );

	m_it = m_handlers.find( aOperation->m_index );

	if (m_it==m_handlers.end())
		return e_bad;

	m_handlers.erase( m_it );
	
	return 0;
}
///////////////////////////////////////////////////////////////////////////////

//##ModelId=44800EB1025F
int VS_NetReactor::GetHandlerFactory(VS_NetHandlerFactory*& aPtr)
{
	if (!m_factory)
	{
		m_factory = new VS_NetHandlerFactoryEvents;
	}
	if (m_factory)
	{
        aPtr = m_factory;
		return e_ok;
	}
	return e_bad;
}
int VS_NetReactor::HandleEvent(VS_NetOperation * aOperation)
{
	if (m_handlers.empty())
		return e_bad;

	unsigned int counter = aOperation->m_index;

	m_it = m_handlers.find( counter );

	if (m_it==m_handlers.end())
		return e_bad;

	if (e_ok!=m_it->second->HandleEvent(aOperation))
	{
		return RemoveHandler(aOperation);
	}
	return e_ok;
}
