/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: ���������� ������� ��������� ����� �������, ������������ ����� ������
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_StreamClientReceiver.h
/// \brief ���������� ������� ��������� ����� �������, ������������ ����� ������
/// \note
///

#ifndef VS_STREAM_CLIENT_RECEIVER_H
#define VS_STREAM_CLIENT_RECEIVER_H

#include "../fwd.h"
#include "VS_StreamClient.h"

class VS_StreamClientReceiver : public virtual VS_StreamClient
{
public:
			VS_StreamClientReceiver		( void );
	virtual	~VS_StreamClientReceiver	( void );
///
/// ������ ������������ ���������� ������ � VS_StreamClient.h !!!
///
/// \brief ������ ����� ���������� ������ ���������� ���������� ������.
/// \param buffer - ������ ������ ������
/// \param s_buffer - ������ ������������� ������ ������
/// \param track - (0 - 255) �������������� �������� ��������� � �������� ������ ����� ����������
///			       (���� ����� ������� ����� �����,������� ��� ������������� ������ �����)
/// \param milliseconds - �������� ������� � ������� ����� �������, ���������������� ��������
/// \return     (0 - VS_STREAM_MAX_SIZE_FRAME) - ���� ����� ��� ������ (���� ������ \n
///                       �������� ������ s_buffer, �� � ����� ���� �������� ������ \n
///                       ������ ����� ������ - s_buffer ����) \n
///             -1      - ���������� ���������� ��� � �� ��������������� \n
///             -2      - timeout,����� ���� .
///
	int		ReceiveFrame	( void *buffer, const int s_buffer,
								stream::Track* track, unsigned long *milliseconds = 0);
///
/// \brief ������� ���������� HANDLE of Event ������� ����� ���������� � Signaled State,\n
///        ����� ����������� ��������� ��� ���� �������� ������ � ����������� �� ������ �\n
///        ����������� Timeout .
///        (�������� ��������! ����� ������������ ���� Event, ReceiveFrame, ������ -1 - ���-\n
///                            ��� ����������, ����� ������� � -2, ��� ������� �������������
///                            ��� ���������� ��� �������� (��� ����� ��������� �� ������\n
///                            ��������, �� �������� ���� ������ ����� �������� � multithreads.)
///
	void	*GetReceiveEvent( void );
};
// VS_StreamClientReceiver class

#endif  // VS_STREAM_CLIENT_RECEIVER_H
