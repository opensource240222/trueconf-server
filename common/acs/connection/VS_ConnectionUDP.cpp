/**
 **************************************************************************
 * \file VS_ConnectionUDP.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Wrapper around UDP connection.
 *
 * Realization see in VS_ConnectionTypes.h
 *
 * \b Project
 * \author SlavetskyA
 * \date 08.10.02
 *
 * $Revision: 4 $
 *
 * $History: VS_ConnectionUDP.cpp $
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 25.01.10   Time: 15:02
 * Updated in $/VSNA/acs/connection
 *  - direct UDP realized
 *
 * *****************  Version 3  *****************
 * User: Dront78      Date: 14.12.09   Time: 17:09
 * Updated in $/VSNA/acs/connection
 * - bugfix 6848
 * - bugfix 6849
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 4.12.09    Time: 11:35
 * Updated in $/VSNA/acs/connection
 * - qos support initial added
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/acs/connection
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 20.03.07   Time: 19:44
 * Updated in $/VS2005/acs/connection
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/acs/connection
 *
 * *****************  Version 19  *****************
 * User: Mushakov     Date: 7.09.06    Time: 19:46
 * Updated in $/VS/acs/connection
 * - RAS multicast in h323Terminal;
 * - Unregistration when delete H323Terminal;
 *
 * *****************  Version 18  *****************
 * User: Mushakov     Date: 15.09.05   Time: 12:38
 * Updated in $/VS/acs/Connection
 * Добавлен метод ReceiveFrom в класс VS_ConnectionSock_Implementation
 *
 * *****************  Version 17  *****************
 * User: Avlaskin     Date: 4.08.05    Time: 10:15
 * Updated in $/VS/acs/connection
 * new overload of RecieveFrom
 *
 * *****************  Version 16  *****************
 * User: Avlaskin     Date: 1.08.05    Time: 14:40
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 15  *****************
 * User: Avlaskin     Date: 30.07.05   Time: 11:31
 * Updated in $/VS/acs/connection
 * new stream headers for UDP & UDP multicast
 *
 * *****************  Version 14  *****************
 * User: Avlaskin     Date: 27.07.05   Time: 20:11
 * Updated in $/VS/acs/connection
 * привязка к ip при мультикасте
 *
 * *****************  Version 13  *****************
 * User: Avlaskin     Date: 27.07.05   Time: 16:16
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 12  *****************
 * User: Avlaskin     Date: 27.04.05   Time: 12:36
 * Updated in $/VS/acs/connection
 * Stream UDP multicast added
 *
 * *****************  Version 11  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 20:07
 * Updated in $/VS/acs/connection
 *
 ****************************************************************************/


#include "VS_ConnectionUDP.h"
#include "VS_ConnectionTypes.h"

bool VS_ConnectionUDP::Socket( void )
{	return imp->Socket( vs_connection_type_dgram );		}
// end VS_ConnectionUDP::Socket

bool VS_ConnectionUDP::Connect( const char *my_host, const unsigned short my_port,
						const char *host, const unsigned short port, const bool exclusiveUseAddr)
{
	return imp->Bind(my_host, my_port, vs_connection_type_dgram,exclusiveUseAddr)
			&& imp->Connect( host, port, vs_connection_type_dgram );
}
// end VS_ConnectionUDP::Connect

bool VS_ConnectionUDP::ConnectMulticast( const char *my_host, const unsigned short my_port,
									const char *mcast_host, const unsigned short port,
									const bool isSender, const char * peer_host,
									const bool exclusiveUseAddr)
{
	VS_IPPortAddress my_addr(my_host);
	VS_IPPortAddress mcast_addr(mcast_host);
	char my_new_addr[46];

	strncpy(my_new_addr, my_host, 45);
	if (my_addr.getAddressType() != mcast_addr.getAddressType()) {
		// try to find appropriate IP address of another version of the same interface
		DWORD dwRetVal = 0;

		ULONG family = AF_UNSPEC;

		PIP_ADAPTER_ADDRESSES pAddresses = NULL;
		ULONG outBufLen = 0;

		PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
		PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;

		unsigned index = 0;

		GetAdaptersAddresses(family, 0, NULL, nullptr, &outBufLen);

		if (outBufLen <= 0)
			return false;

		std::vector<char> _pAddress(outBufLen);
		pAddresses = (PIP_ADAPTER_ADDRESSES)_pAddress.data();
		dwRetVal = GetAdaptersAddresses(family, 0, NULL, pAddresses, &outBufLen);

		if (dwRetVal == NO_ERROR) {
			pCurrAddresses = pAddresses;
			bool found = false;

			while (pCurrAddresses && !found) {
				pUnicast = pCurrAddresses->FirstUnicastAddress;
				if (pUnicast != NULL) {
					for (; pUnicast != NULL && !found; pUnicast = pUnicast->Next) {
						VS_IPPortAddress addr(*pUnicast->Address.lpSockaddr);
						if (my_addr == addr) { // found interface
							// start again and search for the address at the same interface but with different family
							PIP_ADAPTER_UNICAST_ADDRESS pNewUnicast = pCurrAddresses->FirstUnicastAddress;
							for (; pNewUnicast != NULL && !found; pNewUnicast = pNewUnicast->Next) {
								VS_IPPortAddress addr(*pNewUnicast->Address.lpSockaddr);
								if (addr.getAddressType() != my_addr.getAddressType()) {
									addr.GetHostByIp(my_new_addr, 45);
								}
							}
							found = true;
						}
					}
				}
				pCurrAddresses = pCurrAddresses->Next;
			}
		}
	}

	if(isSender)
		return imp->Bind(my_new_addr, my_port, vs_connection_type_dgram, exclusiveUseAddr)
		 	 && imp->SocketBecomeMulticastSender()
			 && imp->Connect( mcast_host, port, vs_connection_type_dgram )
			 && imp->SetMulticastDefaultGateway(my_new_addr);

	else return imp->Bind(my_new_addr, port, vs_connection_type_dgram, exclusiveUseAddr)
			 && imp->SocketBecomeMulticastReciever( mcast_host );
}
bool VS_ConnectionUDP::BecomeMulticastReciever(const char *mcast_host)
{
	return imp ? imp->SocketBecomeMulticastReciever(mcast_host) : false;
}
bool VS_ConnectionUDP::BecomeMulticastSender()
{
	return imp ? imp->SocketBecomeMulticastSender() : false;
}

// end VS_ConnectionUDP::Connect

bool VS_ConnectionUDP::ConnectInRange( const char *my_host,
							const unsigned short min_port, const unsigned short max_port,
							const char *host, const unsigned short port,
							const bool exclusiveUseAddr,
							unsigned short *fixed_port )
{
	return BindInRange(my_host, min_port, max_port, exclusiveUseAddr, fixed_port)
			&& imp->Connect( host, port, vs_connection_type_dgram );	}
// end of VS_ConnectionUDP::ConnectInRange

bool VS_ConnectionUDP::Bind( const char *host, const unsigned short port, const bool exclusiveUseAddr )
{
	return imp->Bind(host, port, vs_connection_type_dgram, exclusiveUseAddr);
}
// end VS_ConnectionUDP::Bind
bool VS_ConnectionUDP::Bind(const unsigned long ip, const unsigned short port, const bool exclusiveUseAddr)
{
	return imp->Bind(ip, port, vs_connection_type_dgram, exclusiveUseAddr);
}

bool VS_ConnectionUDP::Bind(in6_addr ip, const unsigned short port, const bool exclusiveUseAddr)
{
	return imp->BindV6(ip, port, vs_connection_type_dgram, exclusiveUseAddr);
}

bool VS_ConnectionUDP::Bind(const VS_IPPortAddress& ipport, const bool exclusiveUseAddr)
{
	switch(ipport.getAddressType())
	{
	case VS_IPPortAddress::ADDR_IPV4:
		return imp->Bind(ipport.ipv4(), ipport.port(), vs_connection_type_dgram, exclusiveUseAddr);
	case VS_IPPortAddress::ADDR_IPV6:
		return imp->BindV6(ipport.ipv6(), ipport.port(), vs_connection_type_dgram, exclusiveUseAddr);
	default:
		return false;
	}
}

bool VS_ConnectionUDP::BindInRange( const char *host,
							const unsigned short min_port, const unsigned short max_port,
							const bool exclusiveUseAddr,
							unsigned short *fixed_port )
{
	if (!host || !*host || !min_port || !max_port)	return false;
	unsigned short   port = min_port, port_end = max_port;
	if (port > port_end) { port = max_port;	port_end = min_port; }
	for (; port <= port_end; ++port)
	{
		if (imp->Bind(host, port, vs_connection_type_dgram, exclusiveUseAddr))
		{
			if (fixed_port)		*fixed_port = port;
			return true;
		}
	}
	return false;
}
// end of VS_ConnectionUDP::BindInRange

bool VS_ConnectionUDP::Connect( const unsigned long ip, const unsigned short port , const bool isQoSSocket, _QualityOfService * QoSdata)
{	return imp->Connect( ip, port, vs_connection_type_dgram , false, isQoSSocket, QoSdata );	}
// end of VS_ConnectionUDP::Connect

bool VS_ConnectionUDP::ConnectV6( const in6_addr ip, const unsigned short port , const bool isQoSSocket, _QualityOfService * QoSdata)
{	return imp->ConnectV6( ip, port, vs_connection_type_dgram , false, isQoSSocket, QoSdata );	}
// end of VS_ConnectionUDP::Connect

bool VS_ConnectionUDP::Connect( const VS_IPPortAddress& ipport, const bool isQoSSocket , _QualityOfService * QoSdata)
{
	switch(ipport.getAddressType())
	{
	case VS_IPPortAddress::ADDR_IPV4:
		return imp->Connect( ipport.ipv4(), ipport.port(), vs_connection_type_dgram );
	case VS_IPPortAddress::ADDR_IPV6:
		return imp->ConnectV6( ipport.ipv6(), ipport.port(), vs_connection_type_dgram );
	default:
		return false;
	}
}

bool VS_ConnectionUDP::RWrite( const VS_Buffer *buffers, const unsigned long n_buffers )
{	return imp->RWriteDgram( buffers, n_buffers );		}
// end VS_ConnectionUDP::RWrite

bool VS_ConnectionUDP::Write( const void *buffer, const unsigned long n_bytes )
{	return imp->WriteDgram( buffer, n_bytes );		}
// end VS_ConnectionUDP::Write
bool VS_ConnectionUDP::RWriteTo( const VS_Buffer *buffers, const unsigned long n_buffers,
						const unsigned long to_ip, const unsigned short to_port )
{
	return imp->RWriteDgramTo(buffers,n_buffers,to_ip,to_port);
}
bool VS_ConnectionUDP::RWriteToV6( const VS_Buffer *buffers, const unsigned long n_buffers,
						const in6_addr to_ip, const unsigned short to_port )
{
	return imp->RWriteDgramToV6(buffers,n_buffers,to_ip,to_port);
}

bool VS_ConnectionUDP::RWriteTo(  const VS_Buffer *buffers, const unsigned long n_buffers, const VS_IPPortAddress& ipport )
{
	switch(ipport.getAddressType())
	{
	case VS_IPPortAddress::ADDR_IPV4:
		return imp->RWriteDgramTo(buffers,n_buffers,ipport.ipv4(),ipport.port());
	case VS_IPPortAddress::ADDR_IPV6:
		return imp->RWriteDgramToV6(buffers,n_buffers,ipport.ipv6(),ipport.port());
	default:
		return false;
	}
}

bool VS_ConnectionUDP::WriteTo( const void *buffer, const unsigned long n_bytes,
								const unsigned long to_ip, const unsigned short to_port )
{	return imp->WriteDgramTo( buffer, n_bytes, to_ip, to_port );		}
// end VS_ConnectionUDP::WriteTo

bool VS_ConnectionUDP::WriteToV6( const void *buffer, const unsigned long n_bytes,
								const in6_addr to_ip, const unsigned short to_port )
{	return imp->WriteDgramToV6( buffer, n_bytes, to_ip, to_port );		}
// end VS_ConnectionUDP::WriteTo

bool VS_ConnectionUDP::WriteTo( const void *buffer, const unsigned long n_bytes,
						const VS_IPPortAddress& ipport )
{
	switch(ipport.getAddressType())
	{
	case VS_IPPortAddress::ADDR_IPV4:
		return imp->WriteDgramTo( buffer, n_bytes, ipport.ipv4(), ipport.port() );
	case VS_IPPortAddress::ADDR_IPV6:
		return imp->WriteDgramToV6( buffer, n_bytes, ipport.ipv6(), ipport.port() );
	default:
		return false;
	}
}

int VS_ConnectionUDP::GetWriteResult( unsigned long &milliseconds )
{	return imp->GetWriteDgramResult( milliseconds );	}
// end VS_ConnectionUDP::GetWriteResult

int VS_ConnectionUDP::SetWriteResult( const unsigned long b_trans,
											const struct VS_Overlapped *ov )
{	return imp->SetWriteDgramResult( b_trans, ov );	}
// end VS_ConnectionUDP::SetWriteResult

bool VS_ConnectionUDP::RRead( const unsigned long n_bytes )
{	return imp->RReadDgram( n_bytes );		}
// end VS_ConnectionUDP::RRead

bool VS_ConnectionUDP::Read( void *buffer, const unsigned long n_bytes )
{	return imp->ReadDgram( buffer, n_bytes );	}
// end VS_ConnectionUDP::Read

int VS_ConnectionUDP::GetReadResult( unsigned long &milliseconds,
											void **buffer, const bool portion )
{	return imp->GetReadDgramResult( milliseconds, buffer, portion );	}
// end VS_ConnectionUDP::GetReadResult

int VS_ConnectionUDP::SetReadResult( const unsigned long b_trans,
										const struct VS_Overlapped *ov,
										void **buffer, const bool portion )
{	return imp->SetReadDgramResult( b_trans, ov, buffer, portion );		}
// end VS_ConnectionUDP::SetReadResult

int VS_ConnectionUDP::ReceiveFrom( void *buffer, const unsigned long n_bytes,
									unsigned long * ip, unsigned short *port,
									unsigned long &milliseconds )
{	return imp->ReceiveFrom( buffer, n_bytes, ip , port, milliseconds );	}
// end VS_ConnectionUDP::ReceiveFrom

int VS_ConnectionUDP::ReceiveFrom( void *buffer, const unsigned long n_bytes,
									char *from_host, const unsigned long from_host_size,
									unsigned short *port, unsigned long &milliseconds )
{	return imp->ReceiveFrom( buffer, n_bytes, from_host, from_host_size, port, milliseconds );	}

// end VS_ConnectionUDP::ReceiveFrom
bool VS_ConnectionUDP::AsynchReceiveFrom(void* buf,const unsigned long nSizeBuf,char addr_buffer[16],
									unsigned long* &puIPFrom, unsigned short* &puPortFrom)
{	return imp->ReceiveFrom( buf, nSizeBuf, addr_buffer, puIPFrom,puPortFrom );	}

bool VS_ConnectionUDP::AsynchReceiveFrom(void* buf,const unsigned long nSizeBuf,char* addr_buffer,
									in6_addr* &puIPFrom, unsigned short* &puPortFrom)
{	return imp->ReceiveFromV6( buf, nSizeBuf, addr_buffer, puIPFrom,puPortFrom );	}


int VS_ConnectionUDP::SendTo( void *buffer, const unsigned long n_bytes,
								const unsigned long to_ip, const unsigned short to_port )
{	return imp->SendTo( buffer, n_bytes, to_ip, to_port );	}
// end VS_ConnectionUDP::SendTo

int VS_ConnectionUDP::SendToV6( void *buffer, const unsigned long n_bytes,
								in6_addr to_ip, const unsigned short to_port )
{	return imp->SendToV6( buffer, n_bytes, to_ip, to_port );	}
// end VS_ConnectionUDP::SendTo

int VS_ConnectionUDP::SendTo( void *buffer, const unsigned long n_bytes,
								const char* to_host, const unsigned short to_port )
{	return imp->SendTo( buffer, n_bytes, to_host, to_port );	}
// end VS_ConnectionUDP::SendTo

int VS_ConnectionUDP::SendTo( void *buffer, const unsigned long n_bytes,
								const VS_IPPortAddress& ipport )
{
	switch(ipport.getAddressType())
	{
	case VS_IPPortAddress::ADDR_IPV4:
		return imp->SendTo( buffer, n_bytes, ipport.ipv4(), ipport.port() );
	case VS_IPPortAddress::ADDR_IPV6:
		return imp->SendToV6( buffer, n_bytes, ipport.ipv6(), ipport.port() );
	default:
		return false;
	}
}

bool VS_ConnectionUDP::SetMulticastTTL(unsigned long ttl)
{	return imp ? imp->SetMulticastTTL(ttl) : false;		}
// end of VS_ConnectionUDP::SetMulticastTTL

bool VS_ConnectionUDP::SetQoS(bool qos, _QualityOfService *qos_params)
{	return qos ? imp ? imp->SetQOSSocket(qos_params) : false : false;	}
// end of VS_ConnectionUDP::SetQoS


// qWAVE API based QoS support
void VS_ConnectionUDP::SetQoSFlow(const net::QoSFlowSharedPtr &flow)
{
	if (imp)
		imp->SetQoSFlow(flow);
}

net::QoSFlowSharedPtr VS_ConnectionUDP::GetQoSFlow(void)
{
	if (imp)
		return imp->GetQoSFlow();
	return nullptr;
}

bool VS_ConnectionUDP::AddExtraFlow(const net::QoSFlowSharedPtr &flow, const VS_IPPortAddress &addr)
{
	if (imp && flow)
	{
		return imp->AddExtraFlow(flow, addr);
	}

	return false;
}

bool VS_ConnectionUDP::RemoveExtraFlow(const net::QoSFlowSharedPtr &flow)
{
	if (imp && flow)
	{
		return imp->RemoveExtraFlow(flow);
	}
	return false;
}
