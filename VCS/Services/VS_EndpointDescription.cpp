#include "VS_EndpointDescription.h"

#include "../../common/std/cpplib/VS_Protocol.h"

#include "std-generic/compat/memory.h"

VS_EndpointDescription::VS_EndpointDescription(void) :
	m_status(DISCONNECTED_STATUS),
	m_broker_id(0),
	m_confIPSize(0),
	m_protocolVersion(INIT_PROTOCOL_VERSION)
{
}

VS_EndpointDescription::~VS_EndpointDescription(void)
{
}

VS_EndpointDescription::VS_EndpointDescription(const VS_EndpointDescription& ed)
:
  m_name(ed.m_name), m_status(ed.m_status),m_broker_id(ed.m_broker_id),
	m_loggedUser(ed.m_loggedUser), m_lastUser(ed.m_lastUser),
	m_autologin(ed.m_autologin),m_type(ed.m_type),
	m_confIPSize(0),
	m_protocolVersion(ed.m_protocolVersion),
	m_clientVersion(ed.m_clientVersion),
	m_registrar(ed.m_registrar),
	m_registered(ed.m_registered),
	m_lastConnected(ed.m_lastConnected),
	m_systemConfiguration(ed.m_systemConfiguration)

{
	SetConnectionInfo(ed.m_confIP.get(), ed.m_confIPSize, 0);
}

const VS_EndpointDescription& VS_EndpointDescription::operator=(const VS_EndpointDescription& ed)
{
// Setup
	m_name = ed.m_name; m_status=ed.m_status; m_broker_id = ed.m_broker_id;
	m_loggedUser = ed.m_loggedUser; m_lastUser=ed.m_lastUser;
	m_autologin = ed.m_autologin; m_type = ed.m_type;

	m_clientVersion = ed.m_clientVersion;
	m_registrar = ed.m_registrar;
	m_lastConnected = ed.m_lastConnected;
	m_registered = ed.m_registered;
	m_systemConfiguration = ed.m_systemConfiguration;

	SetConnectionInfo(ed.m_confIP.get(), ed.m_confIPSize, 0);
	m_protocolVersion = ed.m_protocolVersion;
	return *this;
}

bool VS_EndpointDescription::IsValid( void ) const
{
	return (!!m_name);
}

void VS_EndpointDescription::SetConnectionInfo(void * confIP,
											   const unsigned long confIPSize,
											   bool directConnect)
{
	if (confIPSize > 0)
	{
		if (m_confIPSize < confIPSize)
			m_confIP = vs::make_unique_default_init<unsigned char[]>(confIPSize);
		m_confIPSize = confIPSize;
		memcpy(m_confIP.get(), confIP, m_confIPSize);
	}
	else
	{
		m_confIP = nullptr;
		m_confIPSize = 0;
	}
}
