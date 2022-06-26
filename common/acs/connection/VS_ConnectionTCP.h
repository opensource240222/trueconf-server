/**
 **************************************************************************
 * \file VS_ConnectionTCP.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Wrapper around Tcp connection.
 *
 * Realization see in VS_ConnectionTypes.h
 *
 * \b Project
 * \author SlavetskyA
 * \date 08.10.02
 *
 * $Revision: 3 $
 *
 * $History: VS_ConnectionTCP.h $
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 7.08.12    Time: 21:42
 * Updated in $/VSNA/acs/connection
 * - static var isDisableNagle  removed
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 14.12.09   Time: 17:09
 * Updated in $/VSNA/acs/connection
 * - bugfix 6848
 * - bugfix 6849
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/acs/connection
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 6.04.07    Time: 18:06
 * Updated in $/VS2005/acs/connection
 *
 * *****************  Version 3  *****************
 * User: Dront78      Date: 20.03.07   Time: 19:44
 * Updated in $/VS2005/acs/connection
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 7.02.07    Time: 12:56
 * Updated in $/VS2005/acs/connection
 * fixed warning
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/acs/connection
 *
 * *****************  Version 19  *****************
 * User: Mushakov     Date: 13.10.06   Time: 10:47
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 18  *****************
 * User: Avlaskin     Date: 22.06.06   Time: 14:12
 * Updated in $/VS/acs/connection
 * Bind method added to TCP
 *
 * *****************  Version 17  *****************
 * User: Avlaskin     Date: 9.06.06    Time: 17:38
 * Updated in $/VS/acs/connection
 * SIP add ons
 *
 * *****************  Version 16  *****************
 * User: Avlaskin     Date: 15.11.05   Time: 20:19
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 15  *****************
 * User: Mushakov     Date: 15.11.05   Time: 13:45
 * Updated in $/VS/acs/connection
 * новый NHP Handshake
 *
 * *****************  Version 14  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 20:07
 * Updated in $/VS/acs/connection
 *
 ****************************************************************************/
#ifndef VS_CONNECTION_TCP_H
#define VS_CONNECTION_TCP_H


#include "VS_ConnectionSock.h"

#include "../Lib/VS_IPPortAddress.h"
#include "../Lib/VS_QoSEnabledSocket.h"

struct in6_addr;

struct _QualityOfService;

#define VS_SOCKS_DEFAULT_PORT	1080

class VS_ConnectionTCP : public VS_ConnectionSock, public VS_QoSEnabledSocketInterface
{
public:
	VS_ConnectionTCP( bool useIPv6 = false ) :VS_ConnectionSock(useIPv6){};
	virtual ~VS_ConnectionTCP( void ) {};
	bool	Socket( void );
	bool	Bind( const char *host, const unsigned short port, const bool exclusiveUseAddr );
	bool	Bind( const unsigned long ip, const unsigned short port, const bool exclusiveUseAddr );
	bool	Bind( const in6_addr ip, const unsigned short port, const bool exclusiveUseAddr );
	bool	Bind( const VS_IPPortAddress& ipport, const bool exclusiveUseAddr );
	bool	Listen( const char *host, const unsigned short port, const bool exclusiveUseAddr,
							const int backlog = 0, const bool waitAccept = false );
	bool	Listen( const unsigned long ip, const unsigned short port, const bool exclusiveUseAddr,
							const int backlog = 0, const bool waitAccept = false );
	bool	Listen( const in6_addr ip, const unsigned short port, const bool exclusiveUseAddr,
							const int backlog = 0, const bool waitAccept = false );
	bool	Listen( const VS_IPPortAddress& ipport, const bool exclusiveUseAddr,
							const int backlog = 0, const bool waitAccept = false );
	bool	Listen( const unsigned short port, const bool exclusiveUseAddr,
		const int backlog = 0, const bool waitAccept = false );
	bool	ListenInRange( const char *host,
							const unsigned short min_port, const unsigned short max_port, const bool exclusiveUseAddr,
							unsigned short *fixed_port/* = 0*/,
							const int backlog /*= 0*/, const bool waitAccept /*= false */);
	virtual bool	Connect( const char *host, const unsigned short port,
							unsigned long &milliseconds ,
							bool isFastSocket = false, const bool isQoSSocket = false , _QualityOfService * QoSdata = NULL);
	virtual bool	Connect( const unsigned long ip, const unsigned short port,
							unsigned long &milliseconds  ,
							bool isFastSocket = false, const bool isQoSSocket = false , _QualityOfService * QoSdata = NULL);
	virtual bool	ConnectV6( const in6_addr ip, const unsigned short port,
							unsigned long &milliseconds  ,
							bool isFastSocket = false, const bool isQoSSocket = false , _QualityOfService * QoSdata = NULL);
	virtual bool	Connect( const VS_IPPortAddress& ipport, unsigned long &milliseconds,
							bool isFastSocket = false, const bool isQoSSocket = false , _QualityOfService * QoSdata = NULL);
	// Асинхронное подключение к узлу по IPv4 адресу.
	virtual bool    ConnectAsynch(const unsigned long ip, const unsigned short port, void *& event	);
	// Асинхронное подключение к узлу по IPv6 адресу.
	virtual bool    ConnectAsynchV6(in6_addr ip6, const unsigned short port, void *& event);
	// Асинхронное подключение к узлу по строковому адресу (имя хоста, ipv4, ipv6).
	virtual bool    ConnectAsynch(const char* host, const unsigned short port, void *& event);
	// Asynch connection using VS_IPPortAddress address format.
	virtual bool    ConnectAsynch(const VS_IPPortAddress& addr, void *& event);
	virtual bool    GetConnectResult(unsigned long mills = 0,bool noWait = true,bool isFastSocket = true, bool qos = false, _QualityOfService * qos_params = NULL);
	bool		PassSocks4( const char *host, const unsigned short port,
								unsigned long &milliseconds );
	unsigned	PassSocks5( const char *host, const unsigned short port,
								const char *user, const char *passwd,
								unsigned long &milliseconds );
	unsigned	PassHttpTnl( const char *host, const unsigned short port,
								const char *user, const char *passwd,
								unsigned long &milliseconds );
	virtual bool	Accept( const char *host, const unsigned short port,
								unsigned long &milliseconds,
								const bool exclusiveUseAddr,
								bool isFastSocket = false,
								bool qos = false,
								_QualityOfService * qos_params = NULL );
	virtual int		Accept( VS_ConnectionTCP *listener, unsigned long &milliseconds,
								bool isFastSocket = false,
								bool qos = false,
								_QualityOfService * qos_params = NULL );
	virtual bool	Accept( VS_ConnectionTCP *listener,
								bool isFastSocket = false,
								bool qos = false,
								_QualityOfService * qos_params = NULL);
	virtual bool	SetAcceptResult( const unsigned long b_trans,
						const struct VS_Overlapped *ov, VS_ConnectionTCP *listener,
								bool isFastSocket = false,
								bool qos = false,
								_QualityOfService * qos_params = NULL);
	virtual bool	IsAccept( void ) const;
	virtual bool	RWrite( const VS_Buffer *buffers, const unsigned long n_buffers );
	virtual bool	Write( const void *buffer, const unsigned long n_bytes );
	virtual int		GetWriteResult( unsigned long &milliseconds );
	virtual int		SetWriteResult( const unsigned long b_trans,
								const struct VS_Overlapped *ov );
	virtual bool	RRead( const unsigned long n_bytes );
	virtual bool	Read( void *buffer, const unsigned long n_bytes );
	virtual int		GetReadResult( unsigned long &milliseconds,
							void **buffer = 0, const bool portion = false );
	virtual int		SetReadResult( const unsigned long b_trans,
							const struct VS_Overlapped *ov,
							void **buffer = 0, const bool portion = false );
	virtual int		Send( const void *buffer, const unsigned long n_bytes, unsigned long &mills, const bool keep_blocked = true );
	virtual int		Send(const void *buffer, const unsigned long n_bytes);
	virtual int		Receive( void *buffer, const unsigned long n_bytes, unsigned long &mills, bool portion = false);
	virtual int		Receive( void *buffer, const unsigned long n_bytes);
	virtual void	Close(void);
	bool	SetMulticastTTL(unsigned long ttl);
	bool	SetQOSSocket(_QualityOfService *qos_params);
	void	SetFastSocket(bool isFast);
	bool	SetKeepAliveMode(bool isKeepAlive, unsigned long keepaliveTime, unsigned long keepalliveInterval);

	virtual void SetQoSFlow(const net::QoSFlowSharedPtr &flow) override;
	virtual net::QoSFlowSharedPtr GetQoSFlow(void) override;
	virtual bool AddExtraFlow(const net::QoSFlowSharedPtr &flow, const VS_IPPortAddress &addr) override;
	virtual bool RemoveExtraFlow(const net::QoSFlowSharedPtr &flow) override;
protected:
	VS_Overlapped &GetReadOv();
	VS_Overlapped &GetWriteOv();
};
// end VS_ConnectionTCP class

#endif // VS_CONNECTION_TCP_H
