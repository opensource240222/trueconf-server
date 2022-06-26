#pragma once

#include "fwd.h"
#include "NHP/VS_NHP_Types.h"
#include "NHP/VS_NHP_Handshake_Impl.h"

#include <string>
#include <vector>
#include <map>
#include <queue>

class VS_ConnectionUDP;
#define VS_UDP_ROUTER_MAX_PORTS	31


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct VS_SomeData
{
	virtual void FakeFunction()	{ int a = 0; }
};
///////////////////////////////////////////////////////////////////////////////
struct VS_EndpointInformationData : public VS_SomeData
{
	std::string endpoint_name;
	unsigned long ip_nat;
	unsigned short port_nat;
	unsigned long ip_local;
	unsigned short port_local;

};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class VS_USR_SomeHandler
{
public:
	typedef void (VS_USR_SomeHandler::*TYPE_CallBackMember)(const VS_SomeData * data_in );
	virtual ~VS_USR_SomeHandler();
	virtual void SetDataEvent(const VS_SomeData * data_in ) = 0 ;
	virtual bool GetHandlerEventsName(std::vector<std::string> *& names_out );
	virtual void AddCallBack( const char * name , TYPE_CallBackMember ptr );
	virtual bool RunCallBack( const char * name , const VS_SomeData * data_in );
protected:
	typedef std::map< unsigned int , TYPE_CallBackMember >::iterator Container_it;
	typedef std::vector< std::string >::iterator	Names_it;
	Names_it & FindName( const char * name , unsigned int &index_out);
	VS_USR_SomeHandler();
	std::map< unsigned int , TYPE_CallBackMember >	m_ptr_container;
	std::vector< std::string >				names;
private:
	Container_it	it;
	Names_it		nit;
};
//end VS_USR_SomeHandler

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class VS_USR_SomeHandlersServer
{
public:
	virtual ~VS_USR_SomeHandlersServer();
	virtual bool AddHandler(	VS_USR_SomeHandler  * handler,
								unsigned int * my_index_out ) ;
	virtual bool RemoveHandler( VS_USR_SomeHandler * handler ) ;
	virtual bool RemoveAllHandlers( void ) ;
	virtual unsigned int GetIndex( VS_USR_SomeHandler * handler );
protected:
	virtual bool PlusOneHandle();
	VS_USR_SomeHandlersServer();
	typedef std::map< unsigned int , VS_USR_SomeHandler* > TYPE_Handle_container;
	typedef TYPE_Handle_container::iterator TYPE_Handle_iterator;
	TYPE_Handle_container handles_container;
	unsigned int current_index;
private:
	TYPE_Handle_iterator it;
};
// end VS_USR_SomeHandlersServer
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef std::vector< unsigned int > TYPE_Index;
///////////////////////////////////////////////////////////////////////////////
class VS_USR_SomeName
{
public:

	VS_USR_SomeName();
	VS_USR_SomeName(const char * );
	~VS_USR_SomeName();
	const char * GetName() const { return name.c_str(); }
	bool SetName(const char * );
	bool AddIndex( unsigned int index );
	bool GetAllIndexes( const TYPE_Index *& list ) const;
	bool RemoveIndex( unsigned int index );
	bool RemoveAllIndexes( void );
protected:

	typedef TYPE_Index::iterator TYPE_Iterator;
	bool FindIndex(	unsigned int ,
					TYPE_Iterator & );
	TYPE_Index		indexes;
	std::string		name;
private:
	TYPE_Iterator	it;
};
//end VS_USR_SomeName
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///\class VS_USR_SomeNameServer
class VS_USR_SomeNameServer
{
public:
	virtual bool AddName( const char * name ,
						  unsigned int * my_index_in_out );
	virtual bool IsNameExist( const char * name );
	virtual bool IsGenerateCondition();
	virtual bool RemoveName( const char * name );
	virtual bool RemoveAllName( void );
	virtual bool RemoveIndex( unsigned int index );
	virtual bool GetIndexesByName( const char * name  ,
								   const TYPE_Index *& );
	virtual unsigned int GenerateIndex( void );

protected:

	typedef std::vector< VS_USR_SomeName* >  TYPE_Names;
	typedef TYPE_Names::iterator			 TYPE_Names_Iterator;
	TYPE_Names								names_container;
	virtual bool FindName( const char * name ,
						 TYPE_Names_Iterator & it_out);
private:
	TYPE_Names_Iterator it;
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class VS_USR_HandleServer : protected VS_USR_SomeHandlersServer,
							protected VS_USR_SomeNameServer
{
public:
	VS_USR_HandleServer();
	~VS_USR_HandleServer();
	bool AddHandler( VS_USR_SomeHandler  * handler );
	bool RemoveHandler( VS_USR_SomeHandler  * handler );
	virtual bool RunByIndex( const char * name ,
							const VS_SomeData * data ,
							unsigned int index );
	virtual bool SetCustomEvent( const char * name ,
								const VS_SomeData * data);
protected:
	std::vector< std::string >::iterator nnit;
private:
};
// end VS_USR_HandleServer::
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VS_USR_DetermineIpHandler : public VS_USR_SomeHandler
{
public:
	VS_USR_DetermineIpHandler();
	virtual ~VS_USR_DetermineIpHandler();
	virtual void ReturnEndpointInformation_Event(	const char * endpoint,
											const unsigned long host_nat ,
											const unsigned short port_nat,
											const unsigned long	host_local,
											const unsigned short port_local) = 0;
protected:
	void SetDataEvent(const VS_SomeData * data_in );
private:
};
// end VS_USR_DetermineIpHandler
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///\class VS_UdpStreamRouter_implementation
/// Абстрактный класс реализаций.
/// Реализован патерн бридж для использования сперва тестовой
/// реализации, а затем реальной с портами завершения.
class VS_UdpStreamRouter_implementation
{
public:
	VS_UdpStreamRouter_implementation():m_flag(0) {}
	virtual ~VS_UdpStreamRouter_implementation(){}
	virtual void * Start( const char * endpoint_name ) = 0;
	virtual bool Init() = 0;
	virtual bool Init(unsigned short port, const char* bind_host) = 0;
	virtual bool OpenPort(const unsigned short port, const char *bind_host) = 0;
	virtual bool Stop( void ) = 0;
	virtual bool IsValid( void ) = 0;
protected:

	virtual void Loop( void ) = 0;

protected:
	int m_flag;
private:
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct _CurrClientInfoType
{
	unsigned long	IP;
	unsigned short	port;
	unsigned long	local_IP;
	unsigned short	local_port;
	SYN_TYPE		syndata;
	ACK_TYPE		ackdata;
	unsigned long	getting_syn_time;
	unsigned long	conn_index;
} ClientInfoType;
typedef std::map<std::string,ClientInfoType> WaitingClientsTableType;

class VS_UdpStreamRouter_Impl :
	public VS_UdpStreamRouter_implementation
{
public:
	enum SetState
	{
		e_s_null = 0,
		e_s_init = 0x01,
		e_s_start = 0x02,
		e_s_loop_start = 0x04,
		e_s_loop_stop = 0x8
	};


protected:
	WaitingClientsTableType		m_ClientsTable;

	VS_ConnectionUDP			*m_conns[VS_UDP_ROUTER_MAX_PORTS];
	VS_NHP_Handshake_Impl		*m_NHPHandshakeObjs[VS_UDP_ROUTER_MAX_PORTS];
	HANDLE						m_hEventArr[MAXIMUM_WAIT_OBJECTS];
	unsigned int				m_count_wait_objects;

	unsigned short				m_port;
	void*						stopEvent;
	void *						m_loop_th;
///////////
	static void					Thread(void *arg);
	void						Loop();


	void						Respond(const char* ReceivedBuf, const unsigned long sz,const unsigned long  ip, const unsigned short port,const unsigned long conn_index);
	unsigned long				PrepareRespond(const char				*ReceivedBuf,
												const unsigned long		sz,
												const unsigned long		ip,
												const unsigned short	port,
												const unsigned long		conn_index,
												char			*&RespondData /**[out]*/); //Возвращает длинну буфера
	ClientInfoType*				GetClient(unsigned long ip,unsigned short port);
	void						NewClient(const unsigned long ip,const unsigned short port,const SYN_TYPE& syndata, const unsigned long conn_index);
	void						DelClient(unsigned long ip,unsigned short port);
	bool						ClientIsActual(unsigned long ip, unsigned short port);
	bool						ClientIsActual(const ClientInfoType *pClient);
	void						DeactiveClient(ClientInfoType	*pClient);
	void						DeactiveClient(unsigned long ip, unsigned short port);
	void						NotifyClients(ClientInfoType* pClient, const ACK_TYPE	&ackdata);
	void						VerifyClients();
	void						SendRespond(const char *senddata,unsigned long sz,unsigned long ip, unsigned short port,const unsigned long conn_index);

	void						ProcAck(unsigned long ip,unsigned short port,ACK_TYPE ackdata);//Не нужна
	void						TerminateAll();

public:

	VS_UdpStreamRouter_Impl();
	virtual ~VS_UdpStreamRouter_Impl();

	virtual void * Start( const char * endpoint_name ) ;
	virtual bool Init();
	virtual bool Init(unsigned short port, const char* bind_host);
	virtual bool OpenPort(const unsigned short port, const char* bind_host);
	virtual bool Stop( void );
	virtual bool IsValid( void );

};
///////////////////////////////////////////////////////////////////////////////
class VS_UdpStreamRouter_TestImp :
	public VS_UdpStreamRouter_implementation
{
public:

enum State
{
	e_null = 0,
	e_init = 0x01,
	e_add_handler = 0x03,
	e_start = 0x07,
	e_loop_start = 0x0f,
	e_loop_stop = 0x1f
};

enum SetState
{
	e_s_null = 0,
	e_s_init = 0x01,
	e_s_add_handler = 0x02,
	e_s_start = 0x04,
	e_s_loop_start = 0x08,
	e_s_loop_stop = 0x10
};
enum ClientState
{
	e_none		= 0,
	e_SYN		= 1,
	e_SYN_ACK	= 1,
	e_ACK		= 3
};
struct StateClientInfoType
{
	ClientState				curstate;
	VS_NHP_ConnectionData	nat;

	SYN_TYPE				syndata;
	SYN_ACK_TYPE			synackdata;
	ACK_TYPE				ackdata;
};
typedef std::map<std::string,StateClientInfoType>	ClientStateContainer;
typedef std::map<std::string,VS_NHP_Container>		ClientsTableType;

public:

	VS_UdpStreamRouter_TestImp();
	virtual ~VS_UdpStreamRouter_TestImp();
	virtual void * Start( const char * endpoint_name );
	virtual bool AddHandler( VS_USR_SomeHandler  * handler );
	virtual bool Init();
	virtual bool Stop( void );
	virtual bool IsValid( void );
	virtual bool ProcessingMessage( char * data, int sz);
	virtual bool PeriodicTick();
protected:

	virtual void Loop( void );
	static void Thread(void * arg);

protected:
	unsigned short m_port;
	VS_ConnectionUDP * m_conn;
	VS_USR_HandleServer * handle_server;
	void * readEvent , * writeEvent;
	void * loop_th;
	unsigned long time_interval;

	ClientsTableType						m_ClientTable; //Уже прошли Handshake
	ClientStateContainer					m_CurrClients; //которые проходят

	bool ProcNHPData(char* data,int sz,unsigned long IP, unsigned short port);
	//bool SendSynAck(StateClientInfoType* cl);

	bool	ProcReqClientPack(char * data, int sz, unsigned long IP, unsigned short port);
	int		SendUdpFrameAct( const void *buffer, const int n_bytes,
							stream::Track track, unsigned long UID, unsigned long ip, unsigned short port);

private:
};
// end VS_UdpStreamRouter_TestImp
class VS_UdpStreamRouter_NHPRegImpl :
	public VS_UdpStreamRouter_implementation
{
public:
private:
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VS_UdpStreamRouter
{
public:
	~VS_UdpStreamRouter();
	VS_UdpStreamRouter();
	bool Init(VS_UdpStreamRouter_implementation * imp_, char* bind_host, unsigned short port);
	bool Init(VS_UdpStreamRouter_implementation * imp_, char *Endpoint, bool try_all_ip = false);
	bool OpenPort(const unsigned short port, const char *bind_host = 0);
//	bool AddHandler( VS_USR_SomeHandler  * handler );
	void * Start( const char * endpoint_name );
	bool Stop();
protected:
	VS_UdpStreamRouter_implementation* imp;
private:
};
//end VS_UdpStreamRouter
///////////////////////////////////////////////////////////////////////////////
class VS_USR_DetermineIpHandlerTest : public VS_USR_DetermineIpHandler
{
public:
	VS_USR_DetermineIpHandlerTest(){}
	~VS_USR_DetermineIpHandlerTest(){}
	void ReturnEndpointInformation_Event(	const char * endpoint,
											const unsigned long ip_nat ,
											const unsigned short port_nat,
											const unsigned long	ip_local,
											const unsigned short port_local)
	{
		const unsigned char * ipch =
			reinterpret_cast<const unsigned char*>(&ip_nat);
		const unsigned char * locipch =
			reinterpret_cast<const unsigned char*>(&ip_local);
		printf("\n\t DIP: %s Nat Ip: %u.%u.%u.%u Nat Port: %d Local Ip:%u.%u.%u.%u. Local Port: %d ",endpoint,
			ipch[3],ipch[2],ipch[1],ipch[0],port_nat,locipch[3],locipch[2],locipch[1],locipch[0],port_local);
	}
};