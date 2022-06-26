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
 * $History: VS_ConnectionUDP.h $
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
 * *****************  Version 15  *****************
 * User: Mushakov     Date: 7.09.06    Time: 19:46
 * Updated in $/VS/acs/connection
 * - RAS multicast in h323Terminal;
 * - Unregistration when delete H323Terminal;
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 15.09.05   Time: 12:38
 * Updated in $/VS/acs/Connection
 * Добавлен метод ReceiveFrom в класс VS_ConnectionSock_Implementation
 *
 * *****************  Version 13  *****************
 * User: Avlaskin     Date: 4.08.05    Time: 10:15
 * Updated in $/VS/acs/connection
 * new overload of RecieveFrom
 *
 * *****************  Version 12  *****************
 * User: Avlaskin     Date: 27.07.05   Time: 20:11
 * Updated in $/VS/acs/connection
 * привязка к ip при мультикасте
 *
 * *****************  Version 11  *****************
 * User: Avlaskin     Date: 27.04.05   Time: 12:36
 * Updated in $/VS/acs/connection
 * Stream UDP multicast added
 *
 * *****************  Version 10  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 20:07
 * Updated in $/VS/acs/connection
 *
 ****************************************************************************/


#ifndef VS_CONNECTION_UDP_H
#define VS_CONNECTION_UDP_H

#include "VS_ConnectionSock.h"

#include "../Lib/VS_IPPortAddress.h"
#include "../Lib/VS_QoSEnabledSocket.h"

struct _QualityOfService;
struct in6_addr;

class VS_ConnectionUDP : public VS_ConnectionSock, public VS_QoSEnabledSocketInterface
{
public:
	VS_ConnectionUDP( bool useIPv6 = false ) :VS_ConnectionSock(useIPv6) {};
	virtual ~VS_ConnectionUDP( void ) {};
	bool	Socket( void );
	bool	Connect( const char *my_host, const unsigned short my_port,
							const char *host, const unsigned short port, const bool exclusiveUseAddr );
	bool	ConnectMulticast( const char *my_host, const unsigned short my_port,
									const char *mcast_host, const unsigned short port,
									const bool isSender, const char * peer_host,
									const bool exclusiveUseAddr);
	bool	ConnectInRange( const char *my_host,
							const unsigned short min_port, const unsigned short max_port,
							const char *host, const unsigned short port,
							const bool exclusiveUseAddr,
							unsigned short *fixed_port /*= 0 */);
	// Связывает сокет с IPv4 или IPv6 адресом, с которым связан хост host.
	bool	Bind( const char *host, const unsigned short port, const bool exclusiveUseAddr);
	// Связывает сокет с IPv4 адресом.
	bool	Bind(const unsigned long ip, const unsigned short port, const bool exclusiveUseAddr);
	// Связывает сокет с IPv6 адресом.
	bool	Bind(const in6_addr ip, const unsigned short port, const bool exclusiveUseAddr);
	// Bind socket using VS_IPPortAddress format
	bool	Bind(const VS_IPPortAddress& ipport, const bool exclusiveUseAddr);

	bool	BindInRange( const char *host,
							const unsigned short min_port, const unsigned short max_port,
							const bool exclusiveUseAddr,
							unsigned short *fixed_port = 0 );

	bool	Connect( const unsigned long ip, const unsigned short port , const bool isQoSSocket = false , _QualityOfService * QoSdata = NULL);
	bool	ConnectV6( const in6_addr ip, const unsigned short port , const bool isQoSSocket = false , _QualityOfService * QoSdata = NULL);
	bool	Connect( const VS_IPPortAddress& ipport, const bool isQoSSocket = false , _QualityOfService * QoSdata = NULL);

	bool	RWrite( const VS_Buffer *buffers, const unsigned long n_buffers );
	bool	Write( const void *buffer, const unsigned long n_bytes );

	bool	RWriteTo( const VS_Buffer *buffers, const unsigned long n_buffers,
						const unsigned long to_ip, const unsigned short to_port );
	bool	RWriteToV6( const VS_Buffer *buffers, const unsigned long n_buffers,
						const in6_addr to_ip, const unsigned short to_port );
	bool	RWriteTo(  const VS_Buffer *buffers, const unsigned long n_buffers, const VS_IPPortAddress& ipport );

	bool	WriteTo( const void *buffer, const unsigned long n_bytes,
						const unsigned long to_ip, const unsigned short to_port );
	bool	WriteToV6( const void *buffer, const unsigned long n_bytes,
						const in6_addr to_ip, const unsigned short to_port );
	bool	WriteTo( const void *buffer, const unsigned long n_bytes,
						const VS_IPPortAddress& ipport );

	int		GetWriteResult( unsigned long &milliseconds );
	int		SetWriteResult( const unsigned long b_trans, const struct VS_Overlapped *ov );
	bool	RRead( const unsigned long n_bytes );
	bool	Read( void *buffer, const unsigned long n_bytes );
	int		GetReadResult( unsigned long &milliseconds,
								void **buffer = 0, const bool portion = false );
	int		SetReadResult( const unsigned long b_trans, const struct VS_Overlapped *ov,
								void **buffer = 0, const bool portion = false );
	int		ReceiveFrom( void *buffer, const unsigned long n_bytes,
								char *from_host, const unsigned long from_host_size,
								unsigned short *port, unsigned long &milliseconds );
	int		ReceiveFrom( void *buffer, const unsigned long n_bytes,
								unsigned long *ip ,	unsigned short *port,
								unsigned long &milliseconds );
	// Асинхронное получение пакета с использованием IPv4.
	bool AsynchReceiveFrom(void* buf,const unsigned long nSizeBuf,char addr_buff[16],
									unsigned long* &puIPFrom, unsigned short* &puPortFrom);
	// Асинхронное получение пакета с использованием IPv6.
	bool AsynchReceiveFrom(void* buf,const unsigned long nSizeBuf,char* addr_buff,
									in6_addr* &puIPFrom, unsigned short* &puPortFrom);

	// Отправляет сообщение по адресу IPv4.
	int		SendTo( void *buffer, const unsigned long n_bytes,
								const unsigned long to_ip, const unsigned short to_port );
	// Отправляет сообщение по адресу IP64.
	int		SendToV6( void *buffer, const unsigned long n_bytes,
								in6_addr to_ip, const unsigned short to_port );
	// Отправляет сообщение по строковому адресу (имя хоста, ipv4, ipv6).
	int		SendTo( void *buffer, const unsigned long n_bytes,
								const char* to_host, const unsigned short to_port );
	// Send packet using VS_IPPortAddress address format
	int		SendTo( void *buffer, const unsigned long n_bytes,
								const VS_IPPortAddress& ipport );

	bool	BecomeMulticastSender();
	bool	BecomeMulticastReciever(const char *mcast_host);
	bool	SetMulticastTTL(unsigned long ttl);
	bool	SetQoS(bool qos = false, _QualityOfService *qos_params = 0);

	// QoS
	void SetQoSFlow(const net::QoSFlowSharedPtr &flow) override;
	net::QoSFlowSharedPtr GetQoSFlow(void) override;

	// Mostly for UDP
	bool AddExtraFlow(const net::QoSFlowSharedPtr &flow, const VS_IPPortAddress &addr) override;
	bool RemoveExtraFlow(const net::QoSFlowSharedPtr &flow) override;
};
// end VS_ConnectionUDP class

#endif // VS_CONNECTION_UDP_H
