/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: Реализация клиента протокола медиа потоков, отсылающего медиа фреймы
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_StreamClientSender.h
/// \brief Реализация клиента протокола медиа потоков, отсылающего медиа фреймы
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
/// Методы инициирующие соединение смотри в VS_StreamClient.h !!!
///
/// \brief Данный метод интерфейса отсылки используют прикладные модули.
/// \param buffer - адресс буфера данных
/// \param n_bytes - (0 - VS_STREAM_MAX_SIZE_FRAME) размер передаваемых данных
/// \param track - (0 - 255) дополнительный механизм адресации в пределах одного медиа соединения
/// \param milliseconds - пытаться начать передавать в течении этого времени,\n
///	                      декрементировать значение
/// \return     n_bytes - если фрейм начал передаваться \n
///             -1      - соединение обламалось или и не устанавливалось \n
///             -2      - timeout,канал перегружен,т.е.еще не доотправлен предыдущий фрейм
///             -3      - канал в порядке, но указанный track не обслуживается
///
	int		SendFrame		( const void *buffer, const int n_bytes,
								stream::Track track, unsigned long *milliseconds = 0);
///
/// \brief Функция возвращает HANDLE of Event который будет установлен в Signaled State,\n
///        когда предыдущий пакет будет отправлен в сеть .
///
	void	*GetSendEvent( void );
};
// VS_StreamClientSender class

#endif  // VS_STREAM_CLIENT_SENDER_H
