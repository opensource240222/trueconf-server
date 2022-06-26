/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: ���������� ������� ��������� ����� �������, ����������� ����� ������
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_StreamClientSender.h
/// \brief ���������� ������� ��������� ����� �������, ����������� ����� ������
/// \note
///

#ifndef VS_STREAM_CLIENT_SENDER_H
#define VS_STREAM_CLIENT_SENDER_H

#include "../fwd.h"
#include "VS_StreamClient.h"

class VS_StreamClientSender : public virtual VS_StreamClient
{
public:
			VS_StreamClientSender		( void );
	virtual	~VS_StreamClientSender		( void );
///
/// ������ ������������ ���������� ������ � VS_StreamClient.h !!!
///
/// \brief ������ ����� ���������� ������� ���������� ���������� ������.
/// \param buffer - ������ ������ ������
/// \param n_bytes - (0 - VS_STREAM_MAX_SIZE_FRAME) ������ ������������ ������
/// \param track - (0 - 255) �������������� �������� ��������� � �������� ������ ����� ����������
/// \param milliseconds - �������� ������ ���������� � ������� ����� �������,\n
///	                      ���������������� ��������
/// \return     n_bytes - ���� ����� ����� ������������ \n
///             -1      - ���������� ���������� ��� � �� ��������������� \n
///             -2      - timeout,����� ����������,�.�.��� �� ����������� ���������� �����
///             -3      - ����� � �������, �� ��������� track �� �������������
///
	int		SendFrame		( const void *buffer, const int n_bytes,
								stream::Track track, unsigned long *milliseconds = 0);
///
/// \brief ������� ���������� HANDLE of Event ������� ����� ���������� � Signaled State,\n
///        ����� ���������� ����� ����� ��������� � ���� .
///
	void	*GetSendEvent( void );
};
// VS_StreamClientSender class

#endif  // VS_STREAM_CLIENT_SENDER_H
