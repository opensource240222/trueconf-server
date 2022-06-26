/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: Реализация multi-conference менеджера клиентов медиа-стримов
//
//  Created: 04.03.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_StreamClientsBM.h
/// \brief Реализация multi-conference менеджера клиентов медиа-стримов
/// \note
///


#ifndef VS_STREAM_CLIENTS_BM_H
#define VS_STREAM_CLIENTS_BM_H

#define   VS_STREAM_CLIENT_BM_MAX_PORTS   60

class VS_StreamClientsBM
{
public:

	class ReceiverAppeared
	{
	public:
		virtual void NewReceiver( class VS_StreamClientReceiver *rcv,
								const char *receiver_host, const unsigned n_port ) = 0;
	};
	// end ReceiverAppeared class

			VS_StreamClientsBM( void );
			VS_StreamClientsBM( const char *host, const unsigned short port,
										const unsigned n_ports, ReceiverAppeared *rai );
	~VS_StreamClientsBM();
	struct VS_StreamClientsBM_Implementation   *imp;

	bool	IsValid( void ) const {		return imp != 0;	}
	bool	IsInit( void ) const;

	bool   Init( const char *host, const unsigned short port,
						const unsigned n_ports, ReceiverAppeared *rai );
	void	Shutdown( void );
	bool	SetNewSender( class VS_StreamClientSender *sender,
							unsigned *d_port = 0, const unsigned n_port = ~0 );
};
// end VS_StreamClientsBM class

#endif  // VS_STREAM_CLIENTS_BM_H
