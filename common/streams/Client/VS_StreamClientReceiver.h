/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: –еализаци€ клиента протокола медиа потоков, принимающего медиа фреймы
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_StreamClientReceiver.h
/// \brief –еализаци€ клиента протокола медиа потоков, принимающего медиа фреймы
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
/// ћетоды инициирующие соединение смотри в VS_StreamClient.h !!!
///
/// \brief ƒанный метод интерфейса приема используют прикладные модули.
/// \param buffer - адресс буфера данных
/// \param s_buffer - размер предлагаемого буфера данных
/// \param track - (0 - 255) дополнительный механизм адресации в пределах одного медиа соединени€
///			       (сюда будет положен номер трека,которым был промаркирован данный фрейм)
/// \param milliseconds - пытатьс€ прин€ть в течении этого времени, декрементировать значение
/// \return     (0 - VS_STREAM_MAX_SIZE_FRAME) - если фрейм был прин€т (если данное \n
///                       значение больше s_buffer, то в буфер была положена только \n
///                       перва€ часть фрейма - s_buffer байт) \n
///             -1      - соединение обламалось или и не устанавливалось \n
///             -2      - timeout,канал пуст .
///
	int		ReceiveFrame	( void *buffer, const int s_buffer,
								stream::Track* track, unsigned long *milliseconds = 0);
///
/// \brief ‘ункци€ возвращает HANDLE of Event который будет установлен в Signaled State,\n
///        когда соедиенение разорвано или есть вход€щие данные с возможность их чтени€ с\n
///        минимальным Timeout .
///        (ќбратить внимание! ѕосле сигнализации этим Event, ReceiveFrame, помимо -1 - раз-\n
///                            выв соединени€, может вернуть и -2, что следует рассматривать
///                            как нормальный код возврата (это может произойти по разным\n
///                            причинам, ну например этот клиент может работать в multithreads.)
///
	void	*GetReceiveEvent( void );
};
// VS_StreamClientReceiver class

#endif  // VS_STREAM_CLIENT_RECEIVER_H
