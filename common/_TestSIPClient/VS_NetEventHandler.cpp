#include "VS_NetEventHandler.h"
#include "../SIPParserLib/VS_SIPError.h"

VS_NetEventHandler::VS_NetEventHandler():m_op(0),m_conn(0)
{
}

int VS_NetEventHandler::SetField(  unsigned long field,
						unsigned long fieldValue)
{
	m_map[ field ] = fieldValue;
	return e_ok;
}

int VS_NetEventHandler::GetField( const unsigned long field,
						unsigned long &fieldValue)
{	
	if (m_map.empty()) return e_bad;
	m_it = m_map.find( field );
	if (m_it==m_map.end())
		return e_bad;
	fieldValue = m_it->second;
	return e_ok;
}
