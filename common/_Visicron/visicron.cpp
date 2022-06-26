/**
 **************************************************************************
 * \file visicron.cpp
 * \brief contain dll interface functions implementation
 *
 * \b Project Visicron
 * \author Melehcko Ivan
 * \date 25.11.02
 *
 ****************************************************************************/

/*! \mainpage
 *
 * \par Documentation:
 * - \ref intro
 * - \ref install
 * - \ref reestr
 *  - \ref reestr1
 *  - \ref reestr2
 *  - \ref reestr3
 *  - \ref reestr4
 *  - \ref reestr5
 *  - \ref reestr6
 *  .
 * - \ref technologies
 *  - \ref technologies1
 *  - \ref technologies2
 *  - \ref technologies3
 *  - \ref technologies4
 *  .
 *
 * \section intro  раткое описание

Visicron.dll представл€ет собой модуль, предназначенный дл€ совместной работы с базовыми клиентскими приложени€ми системы Visicron Ѓ. ћодуль не обеспечивает выделение ресурсов пользовательского интерфейса Ц эта функциональность обеспечиваетс€ базовыми клиентскими приложени€ми. ћодуль обеспечивает следующую функциональность:
 -	выдачу списков устройств видео и аудио
 -	захват видео данных от выбранных устройств захвата видео
 -	показ видеоданных
 -	комрессию/декомпрессию видеоданных
 -	захват аудио данных от выбранного устройства захвата аудио
 -	проигрывание аудиоданных на выбранном устройстве проигрывани€ аудио
 -	комрессию/декомпрессию аудиоданных
 -	регулировку громкости аудиоданных, регулировку насыщенности цвета отображаемых видеоданных
 -	св€зь с сервером и передачу управл€ющих данных по транспортному протоколу Visicron Ѓ. ѕод управл€ющими данными подразумеваютс€ все операции обращени€ к серверу, инициируемые пользователем с базового клиентского приложени€, такие как:
  -	авторизаци€ пользовател€
  -	установка конференции
  -	запросы из адресных книг
  -	сообщени€ чата
 -	передачу медиаданных по транспортному протоколу Visicron Ѓ в режиме конференции
 -	изменение сетевых настроек поключени€ к серверу Visicron Ѓ
 -	проведение сетевого теста с текущими сетевыми настройками между модулем и сервером Visicron Ѓ.

 *****************************************************************************
 * \section install »нсталл€ци€
 *
 ѕроисходит во врем€ инстал€ции базового клиентского приложени€
 *****************************************************************************
 * \section reestr –абота с реестром
 *
—охранение настроек модул€ производитс€ в реестр в ветку
\b HKEY_CURRENT_USER\\Software\\Visicron\\ApplicationName\\

\subsection reestr1 ¬етка ..\\Client\\
 - (t)	Ђ\e ARenderStrictBuffDurrї Ц не используетс€
 -	Ђ\e Bandwidthї Ц максимальное значение исход€щего битрейта в конференции. ќпределен диапазон [3..2048]. ƒефолтное значение 128.
 -	Ђ\e DisableDirectї Ц ненулевое значенине означает не пытатьс€ соединитьс€ медиа страмами по пр€мым IP а также с использованием технологии NHP. ƒефолтное значение 0.
 - (t)	Ђ\e EnableNoiseGenї Ц не используетс€
 - (t)	Ђ\e FPSRateї Ц коэффициент регулирующий отношение между частотой следовани€ кадров и степенью сжати€ кадра, чем больше, тем выше фпс и больше степерь сжати€. ќпределен диапазон [8..56]. ƒефолтное значеие 32.
 -	Ђ\e InputBandwidthї Ц максимальное значение вход€щего битрейта в приватной конференции. ќпределен диапазон [3..4096]. ƒефолтное значение 128.
 -	Ђ\e RenderAudioNameї Ц строковый идентификатор используемого устройства воспроизведени€ аудио. ƒанное устройство будет использовано дл€ всех ресиверов в конференции.
 -	Ђ\e RenderAudioVolumeї Ц масштабный коеффициент громкости воспроизводимого аудио. ќпределен диапазон [0..65536]. ƒефолтное значение 32767.
 -	Ђ\e Saturationї Ц коеффициент насыщенности цвета отображаемыз видеоданных. ќпределен диапазон [0..200]. ƒефолтное значение 120.
 -	Ђ\e UseDefaultAudioRenderї Ц ненулевое значение означает, что используетс€ дефолтное (первое в списке) аудиоустройство.

\subsection reestr1_1 ¬етка ..\\Client\\AudioCaptureSlot\\
 -	Ђ\e DeviceNameї Ц строковый идентификатор используемого устройства захвата аудио.
 - (t)	Ђ\e EnableAGCї Ц ненулевое значение включает алгоритм автоматической регулировки усилени€. ƒефолтное значение 0.
 -	Ђ\e UseDefaultDeviceї Ц ненулевое значение означает, что используетс€ дефолтное (первое в списке) аудиоустройство.
 - 	Ђ\e UseXPAecї Ц ненулевое значение означает попытку инициализации и использовани€ устройств DirectSound Audio с эффектом "эхоподавление". ¬ этом режиме не работает дефолтный алгоритм эхоподавлени€. ѕо умолчанию 0.
 -	Ђ\e Volumeї Ц масштабный коеффициент громкости захвата аудио. ќпределен диапазон [0..65535]. ƒефолтное значение 65535.

\subsection reestr1_2 ¬етка ..\\Client\\VideoCaptureSlot\\
 -	Ђ\e Channelї Ц номер канала видеоустройства (S-video, Component, ... ect).
 - (t)	Ђ\e CptFmtї Ц желаемые размеры кадра, захватываемые с устройтва видео. «адаетс€ формулой: W/8 + (H/8)*256. ѕри нулевом значении используютс€ размеры из ћедиа‘ормата. ƒефолтное значение 0.
 - (t)	Ђ\e Deinterlaceї Ц ненулевое значение означает использование алгоритма ƒеинтерлейсинга. ѕо умолчанию 0.
 -	Ђ\e DeviceNameї Ц строковый идентификатор используемого устройства захвата video.
 - (t)	Ђ\e DVC90DimFixї Ц ненулевое значение означает не использовать разрешение 352х288, захватываемое с видеоустройства. Ќеобходимо дл€ устройства "DVC90". ƒефолтное значение 0.
 - (t)	Ђ\e FrameRateї Ц максимальна€ частота следовани€ кадров, запрашиваема€ с устройства видеозахвата. ќпределен диапазон: [5..30]. ƒефолтное значение 15.
 - (t)	Ђ\e InvertUVї Ц ненулевое значение означает смену местами U и V плоскостей в видеокадре YV12. ѕо умолчанию 0.
 - (t)	Ђ\e ScaleCapturedImageї Ц ненулевое значение означает применение алгоритма масштабировани€ при несоответствии заказанных размерах видео и разрешенными размерами видео, получаемыми с устройства видео. ѕри нулевом значении используетс€ механизм вырезани€/вставки. ƒефолтное значение 0.
 -	Ђ\e UseDefaultDeviceї Ц ненулевое значение означает, что используетс€ дефолтное (первое в списке) видеоустройство.
 -	Ђ\e VideoModeї Ц номер режима видео (NTSC, PAL,... ect).

\subsection reestr2 ¬етка ..\\Current configuration\\
 - (t)	Ђ\e AudioBufferDurrї Ц выбор длительности одной порции аудиоданных в ћедиа‘ормате, определены следующие значени€ {-1, 30..120}. ƒефолтное значение -1.
 - (t)	Ђ\e AudioCodecї Ц выбор аудиокодека ћедиа‘ормата, определены следующие значени€:
	- 0 - кодек GSM6.10
	- 1 - кодек g723.1
	- 2 - кодек g729a
	- 3 - кодек g728
	- 4 - кодек g722
	- 5 - кодек speex, WB, частота дискретизации звука 16 к√ц
	- ѕо умолчанию 1.
 - (t)	Ђ\e BrLoadFactorї Ц коеффициент, учитывающий загрузку буффера медиаброкера к другому клиенту в конференции один-на-один. ќпределен диапазон [50..250]. ƒефолтное значение 100.
 - (t)	Ђ\e BrAutoDisabledї Ц положительное значение означает неиспользование автоматической регулировки битрейта . ƒефолтное значение 0.
 - (t)	Ђ\e Debug Modulesї Ц флаги дл€ включени€ трассировки модулей в дебажной сборке, определены следующие флаги:
	- 0 - ни один модуль,
	- 1 - модуль аудио
	- 2 - модуль св€зи с сервером
	- 4 - модуль видео
	- 8 - модуль ресивера и сендера
	- 16 - модуль клиентского интерфейса (от √”»)
	- ѕо умолчанию 0.
 - (t)	Ђ\e Debug OutTypeї Ц выбор типа вывода трассировки в дебажной сборке, определены следующие значени€:
	- 0 - нет вывода
	- 1 - вывод в окно дебагера
	- 2 - вывод в консоль
	- 3 - вывод в файл
	- ѕо умолчанию 0.
 -	Ђ\e DefaultServerї Ц им€ дефолтного сервера.
 -	Ђ\h DirectPortї Ц порт приложени€, по которому к нему можно присоединитьс€ напр€мую. ѕо умолчанию 5050.
 - (t)	Ђ\e ForceNhpї Ц ненулевое значение означает использование NHP соединени€ при удачном тесте NHP. ѕо умолчанию 0.
 - (t)	Ђ\e KeepAspectRatioї Ц ненулевое значение означает выдержавание пропорций изображени€ при отрисовке (учитыва€, что пиксель "квадратный") ѕо умолчанию 1.
 -h	Ђ\e MarkQosї Ц ненулевое значение означает использование маркировки QOS дл€ отсылаемых медиаданных. ѕо умолчанию 0.
 -	Ђ\e NetworkModeї Ц 	выбор типа сетевого соединени€ с сервером, определены следующие значени€:
	- 0 - использовать пр€мое соединение
	- 3 Ц использовать HTTP туннелирование
	- 2 Ц »спользовать настройки Windos
	- 1 Ц использовать ручную настройку
	- ѕо умолчанию 0.
 -	Ђ\e NetworkTypeї Ц тип физического соединени€ с сетью (»нтернет), определены следующие значени€:
	- 0 Ц "I dont know"
	- 1 - "Dial-up"
	- 2 Ц "GPRS/EDGE"
	- 3 Ц "DSL"
	- 4 Ц "Cable"
	- 5 - "T1"
	- 6 Ц "Wi-Fi"
	- ѕо умолчанию 0.
 -	Ђ\e ProxyHostї Ц сетевое им€ или сетевой адрес прокси-сервера
 -	Ђ\e ProxyPortї Ц порт прокси-сервера
 -	Ђ\e ProxyTypeї Ц тип прокси-сервера, определены следующие значени€:
	- 0 Ц HTTP
	- 1 Ц SOCKS 4
	- 2 Ц SOCKS 5
 -	Ђ\e ProxyUserNameї Ц им€ пользовател€ прокси-сервера
 -	Ђ\e ProxyPasswordї Ц пароль пользовател€ прокси-сервера
 - (t)	Ђ\e ResizeModeї Ц  выбор режима масштабировани€ видео, определены следующие значени€:
	- 0 - обычный режим, биликубик+ƒирект, хорошее качество, но полосы, низка€ загрузка
	- 1 - программный билинеар, удовлетв качество, без полос, умеренна€ загрузка
	- 2 - программный бикубик, хорошее качество, без полос, высока€ загрузка
	- 3 - высокачественный программный бикубик, улучшенное качество, максимальна€ загрузка.
	- ѕо умолчанию 0.
 -	Ђ\e Serverї Ц четырехсимвольный идентификатор текущего сервера.
 - (t)	Ђ\e UseNhpї Ц ненулевое значение означает проведение теста NHP. ѕо умолчанию 0.
 - (t)	Ђ\e VideoCodecї Ц  выбор видеокодека ћедиа‘ормата, определены следующие значени€:
	- 0 - кодек xc02
	- 1 - кодек h261
	- 2 - кодек h263
	- 3 - кодек h263+
	- 4 - кодек h264
	- ѕо умолчанию 0.

\subsection reestr3 ¬етка ..\\Current configuration\\Servers\\
Ёта ветка содержит только ветки, описываюаща€ парамеры, имеющие отношение к каждому серверу. —одержит ветки с четырехсимвольными названи€ми серверов (XXXX).

\subsection reestr4 ¬етка ..\\Current configuration\\Servers\\XXXX\\
 -	Ђ\e Endpointї Ц им€ эндпоинта клиентского приложени€ (символьный идентификатор приложени€ в системе Visicron Ѓ.
 -	Ђ\e HomeBrokerї Ц им€ эндпоинта сервера, к которому подсоедин€етс€ клиентское приложение. ѕо умолчанию XXXX:1.
 -	Ђ\e Keyї Ц строковый идентификатор, авторизующий данный ендпоинт клиентского приложени€ на данном сервере
 -	Ђ\e NetworkTestї Ц результат проведенного сетевого теста в бинарном виде.

\subsection reestr5 ¬етка ..\\Current configuration\\Servers\\XXXX\\XXXX:YYYY\\
 -	Ђ\e LastUpdateї Ц  врем€ последнего обновлени€ информации о данном едпоинте сервера

\subsection reestr6 ¬етка ..\\Endpoints\\
—одержит ветки с именами эндпоинтов, содержащих данные подключени€ к ним по сети. (см. документацию о траспортной библиотеке).

 *****************************************************************************
 * \section technologies »спользуемые технологии
 *
 * \subsection technologies1 Cжатие видео
 *
	ƒл€ сжати€ видеоданных в системе Visicron Ѓ используетс€ кодек Cyclon, разработанный программистами компании Visicron Ѓ специально дл€ применени€ в видеоконференци€х. ¬ кодеке применены последние достижени€ в области сжати€ видео:
 -	Wavelet сжатие Ц  позвол€ющее получить изображение не имеющее блочности на ключевых кадрах.
 -	“ехнлоги€ обнаружени€ участков изображени€ содержащих области лица и рук позвол€ет кодировать такие участки с большим качеством, что улучшает субъективное воспри€тие вашего собеседника в видеоконференции.
 -	“ри стадии фильтрации: предобработка изображени€, фильтраци€ в цикле кодировани€, фильтраци€ декодированного изображени€ дают ощутимые преимущества в качестве изображени€ по сравнению со стандартными видеокодеками.
 -	ќптимизаци€ алгоритма по скорости, с применением 3-х уровневой системы компенсации движени€, оптимизаци€ под различные типы процессоров позвол€ют использовать клиентское приложение на различных машинах без ощутимых различий в скорости работы клиентского приложени€.

 *****************************************************************************
 * \subsection technologies2 ќтображение видео
 *
	ƒл€ отображени€ видео в клиентском приложении модуль использует технологию DirectX, позвол€ющую не загружать процессор при отображении видеданных. ¬ модуле используетс€ точное масштабирование изображени€, разработанное программистами Visicron Ѓ, имеющее качество увеличени€ превышающее качество увеличени€ стандартного алгоритма Bicubic.  роме того данный алгоритм работает в несколько раз быстрее алгоритма Bicubic.

 *****************************************************************************
 * \subsection technologies3 Ѕуфферизации медиаданных и минимизаци€ задержки при передаче звука
 *
	¬ реальных услови€ работы клиентских приложений в »нтернете передача медиаданных между приложени€ми носит не непрерывый а прерывистый характер. ¬ этом случае возможны паузы между отдельными принимаемыми пакетами в 0.5 секунд и более (так называемый джитер). ≈сли приложение не имеет в запасе данных дл€ проигрывани€ звука в этих паузах и показа видео, возникают ощутимые на слух паузы в звуке, а видео приостанавливаетс€ на врем€ этих пауз. ƒл€ исключени€ такой ситуации в модуле реализована адаптивна€ буфферизаци€ медиаданных. ѕринимые данные сначала записываютс€ в буффер, а затем равномерно проигрываютс€. ≈сли возникают паузы в приеме, то данных в буффере хватает дл€ проигрывание на врем€ всей паузы.
	¬ приложени€х использующих буфферизацю данных на приеме стараютс€ сделать приемный буффер как можно больше дл€ погашени€ джитера в сети. Ётот метод годитс€ только дл€ односторонней св€зи, когда только одна сторона получает медиаданные, например в приложени€х ЂVideo On Demendї, когда нельз€ определить реальную разницу между моментом какого-либо событи€ и моментом ее воспроизведени€ в приложении. „ем больше приемный буффер, тем больше эта разница (задержка при воспроизведении) во времени. ¬ отличие от односторонней св€зи, в системах видеконференций реального времени нельз€ неограниченно увеличивать размер приемног буффера, так как это будет приводить к слышымой обоими участниками конференции задержке звука. ¬озникает противоречива€ задача: с одной стороны необходимо погасить джитер сети, увеличива€ размер приемного буффера, с другой Ц уменьшить размер буффера дл€ обеспчени€ минимальной задержки. ƒл€ этой цели в модуле по ориганальному алгоритму рассчитываетс€ длина буффера, на основании построени€ статистической модели распределени€ сетевого траффика на данный момент времени.
 *****************************************************************************
 * \subsection technologies4 ќбеспечение непрерывности и минимизации потерь при воспроизведении звука
 *
	≈сли пауза в приеме аудиоданных превысила предсказанное значение, и данных в приемном буффере оказалось недостаточно дл€ непрерывного проигрывани€, может возникнуть пауза в воспроизведении звука.
	ƒл€ исключени€ этой ситуации в модуле примен€тс€ технологи€ генерации фонового шума, которым	заполн€ютс€ такие паузы в воспроизведении, тем самым обеспечиваетс€ непрерывность воспроизведени€ аудио. ¬ противоположность другой ситуации, данных в буффере может оказатьс€ больше, и часть из них нужно удалить. Ёто достигаетс€ нахождением в них участков не содержащих речевую информацю (участков аудио данных, содержащих тишину) с помощью алгоритма обнаружени€ активности звука (VAD). “ем сам
обеспечиваетс€ минимизизи€ потерь воспроизведени€ важной речевой информации.
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "../ClientInterface/ClientInterface.h"
#include "Utils.h"

int VS_HardwareTest();

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * Functions
 ****************************************************************************/
__declspec(dllexport)DWORD VSProcess(DWORD_PTR lParam,int Operation,char*szInterface,VARIANT *pVar){return Process(lParam,Operation,szInterface,pVar);};
__declspec(dllexport)int VSInterfaceGetVersion(DWORD_PTR pGUI_version,DWORD_PTR pAPP_version){ return InterfaceGetVersion(pGUI_version,pAPP_version);};
__declspec(dllexport)DWORD_PTR VSInitialize(DWORD_PTR dwFlag,HWND hwnd,char*szRegInit,char *szHomeDomain){return Initialize(dwFlag,hwnd,szRegInit,szHomeDomain);};
__declspec(dllexport)void VSRelease(DWORD_PTR lParam){Release(lParam);};
__declspec(dllexport)int  VSStartSender(DWORD_PTR lParam/*,DWORD_PTR pAudio,DWORD_PTR pVideo,HWND hwnd,DWORD_PTR pProc,LPVOID* inst*/,int Width,int Height,int Freq){return StartSender(lParam/*,pAudio,pVideo,hwnd,(RENDERPROC*)pProc,inst*/,Width,Height,Freq);};
__declspec(dllexport)void VSStopSender(DWORD_PTR lParam){ StopSender(lParam);};
__declspec(dllexport)void VSSetEchoMode(DWORD_PTR lParam, int mode){}
__declspec(dllexport)void VSConnectSender(DWORD_PTR lParam,HWND hReportHwnd,DWORD_PTR Type){ConnectSender(lParam,hReportHwnd,Type);};
__declspec(dllexport)void VSDisconnectSender(DWORD_PTR lParam){DisconnectSender(lParam);};
__declspec(dllexport)void VSSetHistory(DWORD_PTR lParam,DWORD_PTR lParam1){SetHistory(lParam,lParam1);};
__declspec(dllexport)void VSGetHistory(DWORD_PTR lParam,DWORD_PTR lParam1){GetHistory(lParam,lParam1);};
__declspec(dllexport)void VSGetTraffic(DWORD_PTR lParam,DWORD_PTR lParam1,DWORD_PTR lParam2){GetTraffic(lParam,lParam1,lParam2);};
__declspec(dllexport)void VSSetFlags(DWORD_PTR lParam,DWORD_PTR lParam1){SetFlags(lParam,lParam1);};
__declspec(dllexport)void VSGetFlags(DWORD_PTR lParam,DWORD_PTR lParam1){GetFlags(lParam,lParam1);};
__declspec(dllexport)void VSSetDisableDirect(DWORD_PTR lParam,DWORD_PTR lParam1){SetDisableDirect(lParam,lParam1);};
__declspec(dllexport)void VSForceBicubic(DWORD_PTR lParam,DWORD_PTR force){ForceBicubic(lParam,force);};
__declspec(dllexport)int VSLoginToServer(DWORD_PTR lParam, DWORD_PTR pLogin,DWORD_PTR pPassword,DWORD_PTR iAutoLogin, int Encrypt){return LoginToServer(lParam,pLogin,pPassword,iAutoLogin, Encrypt);};
__declspec(dllexport)void VSLogoutServer(DWORD_PTR lParam){LogoutServer(lParam);};
__declspec(dllexport)int  VSGetMyName(DWORD_PTR lParam,DWORD_PTR UserName,DWORD_PTR FirstName,DWORD_PTR LastName){return GetMyName(lParam,UserName,FirstName,LastName);};
__declspec(dllexport)void VSGetOtherName(DWORD_PTR lParam,DWORD_PTR UserName,DWORD_PTR FirstName,DWORD_PTR LastName){GetOtherName(lParam,UserName,FirstName,LastName);};
__declspec(dllexport)int  VSAcceptProtocolConnect(DWORD_PTR lParam){return AcceptProtocolConnect(lParam);};
__declspec(dllexport)void VSRejectProtocolConnect(DWORD_PTR lParam){RejectProtocolConnect(lParam);};
__declspec(dllexport)void VSHangupProtocolConnect(DWORD_PTR lParam,int Strong){HangupProtocolConnect(lParam,Strong);};
__declspec(dllexport)DWORD VSBwtStart(DWORD_PTR lParam, DWORD_PTR hwnd){return BwtStart(lParam, hwnd);}
__declspec(dllexport)DWORD VSBwtStop(DWORD_PTR lParam){return BwtStop(lParam);}
__declspec(dllexport)DWORD VSBwtGet(DWORD_PTR lParam, DWORD_PTR Status, DWORD_PTR Id){return BwtGet(lParam, Status, Id);}
__declspec(dllexport)void VSBwtWizardOn(DWORD_PTR lParam, int mode){BwtWizardOn(lParam, mode);}
__declspec(dllexport)DWORD VSSetProxySet(DWORD_PTR array){return SetProxySet(array);}
__declspec(dllexport)void  VSGetProxySet(DWORD_PTR array){GetProxySet(array);}
__declspec(dllexport)void  VSSetProxyDialog(DWORD_PTR func){SetProxyDialog(func);}
__declspec(dllexport)void  VSProxyDialogEnd(int ret){ProxyDialogEnd(ret);}
__declspec(dllexport)void  VSSetProxyNetMode(int* cfg){SetProxyNetMode(cfg);}
__declspec(dllexport)int   VSSetNetType(int mode){return SetNetType(mode);}
__declspec(dllexport)int   VSSetManualPort(unsigned short* port, char* host, int mode){return SetManualPort(port, host, mode);}
__declspec(dllexport)void VSGetSkinName(DWORD_PTR lParam,DWORD_PTR FileName){GetSkinName(lParam,FileName);};
__declspec(dllexport)void VSSetSkinName(DWORD_PTR lParam,DWORD_PTR FileName){SetSkinName(lParam,FileName);};
__declspec(dllexport)void VSSendMessage(DWORD_PTR lParam,DWORD_PTR pMessage,DWORD_PTR szOther){SendMessages(lParam,pMessage,(void*)szOther);};
__declspec(dllexport)void VSSendCommand(DWORD_PTR lParam, char* command, char* to){SendCommands(lParam, command, to);}
__declspec(dllexport)int  VSGetServers(DWORD_PTR lParam,DWORD_PTR size,DWORD_PTR pName,DWORD_PTR pDesk){return GetServers(lParam,size,pName,pDesk);};
__declspec(dllexport)int  VSGetDefaultServer(DWORD_PTR lParam, DWORD_PTR name){return GetDefaultServer(lParam, name);}
__declspec(dllexport)void VSSetCurrentServer(DWORD_PTR lParam,DWORD_PTR pName){SetCurrentServer(lParam,pName);};
__declspec(dllexport)void VSSetSaturation(DWORD_PTR lParam,DWORD_PTR iSaturation){SetSaturation(lParam,iSaturation);};
__declspec(dllexport)void VSGetLoggedUsersList(DWORD_PTR lParam,DWORD_PTR UserList,int Type){GetLoggedUsersList(lParam,UserList,Type);};
__declspec(dllexport)DWORD VSGetProperty(DWORD_PTR lParam,DWORD_PTR lParam1,DWORD_PTR lParam2){return GetProperty(lParam,lParam1,lParam2);};
__declspec(dllexport)int  VSGetChatMessage(DWORD_PTR lParam,int Id, char* From, char* Mess, char* to, char* Dn){return GetChatMessage(lParam,Id,From,Mess,to,Dn);};
__declspec(dllexport)int  VSGetChatMessageV2(DWORD_PTR lParam,int Id, char* From, char* Mess, char* to, char* Dn, long long *time){return GetChatMessageV2(lParam,Id,From,Mess,to,Dn, time);};
__declspec(dllexport)void  VSGetCommandMessage(DWORD_PTR lParam,int Id, char* From, char* Command){GetCommandMessage(lParam, Id, From, Command);}
__declspec(dllexport)void  VSBanUser(DWORD_PTR lParam,char*UserID,int Strong){BanUser(lParam,UserID,Strong);};
__declspec(dllexport)void  VSSetServer(DWORD_PTR lParam,char *szName,char *szIP,char*szPort){SetServer(lParam,szName,szIP,szPort);};
__declspec(dllexport)DWORD VSAddressBook(DWORD_PTR lParam,DWORD abCommand,char*param,long addressBook, long hash){return AddressBook(lParam,abCommand,param,addressBook, hash);};
__declspec(dllexport)DWORD VSQueryUserInfo(DWORD_PTR lParam,char*szUID){return QueryUserInfo(lParam,szUID);};
__declspec(dllexport)DWORD VSReadUserInfo(DWORD_PTR lParam,char*szUID,char**szFileds){return ReadUserInfo(lParam,szUID,szFileds);};
__declspec(dllexport)void  VSSetAutologin(DWORD_PTR lParam,DWORD ALogin){SetAutologin(lParam,ALogin);};
__declspec(dllexport)DWORD VSGetAutologin(DWORD_PTR lParam){return GetAutologin(lParam);};
__declspec(dllexport)void  VSCommOperation(DWORD_PTR lParam,char*szPort,int *pSpeed,int *pMode,int read){CommOperation(lParam,szPort,pSpeed,pMode,read);};
__declspec(dllexport)void  VSCommEnabled(DWORD_PTR lParam,int iEnabled){CommEnabled(lParam,iEnabled);};
__declspec(dllexport)void  VSSetJitter(DWORD_PTR lParam,int Mode){SetJitter(lParam,Mode);};
__declspec(dllexport)int   VSCreateLoopBackChannel(DWORD_PTR lParam,int bAccept,int iPort){return CreateLoopBackChannel(lParam,bAccept,iPort);};
__declspec(dllexport)void  VSCloseLoopBackChannel(DWORD_PTR lParam){CloseLoopBackChannel(lParam);};
__declspec(dllexport)DWORD VSGetReceiverState(DWORD_PTR lParam,int ID,char *name){return GetReceiverState(lParam,ID,name);};
__declspec(dllexport)void  VSSetTrackCallBack(DWORD_PTR lParam,void *pCallBack,void*pParam){SetTrackCallBack(lParam,pCallBack,pParam);};
__declspec(dllexport)void  VSSetExtTrackCallBack(DWORD_PTR lParam,void *pCallBack,void*pParam){SetExtTrackCallBack(lParam,pCallBack,pParam);};
__declspec(dllexport)void  VSHardwareTest(){VS_HardwareTest();}
__declspec(dllexport)int   VSCopyRegKeys(char *s, char* d){return VS_CopyRegKeys(s, d);}
__declspec(dllexport)int   VSDetectAVXSupport(){ return VS_CheckAVXOnInstall(); }
/**********************************************************/

#ifdef __cplusplus
}
#endif
