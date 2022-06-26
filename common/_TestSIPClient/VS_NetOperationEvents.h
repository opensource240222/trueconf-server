#ifndef VS_NETOPERATIONEVENTS_H_HEADER_INCLUDED_BB8BBC71
#define VS_NETOPERATIONEVENTS_H_HEADER_INCLUDED_BB8BBC71

#include "VS_NetOperation.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include "..\acs\connection\VS_Connection.h"
#include "..\acs\connection\VS_ConnectionOv.h"
#include "../acs/connection/VS_ConnectionTCP.h"
#include "../acs/connection/VS_ConnectionUDP.h"

///Насколько это будет совместимо с конектами. Думаю, полностью, не 
///писать же их с нуля.
struct VS_NetOverlapped : public VS_Overlapped
{
};
//##ModelId=44747EE001A9
class VS_NetOperationEvents : public VS_NetOperation
{
public:
	VS_NetOperationEvents();
	virtual ~VS_NetOperationEvents();
	VS_NetOverlapped m_ov;
	unsigned long m_trasfered;
	char		* m_buffer;
};



#endif /* VS_NETOPERATIONEVENTS_H_HEADER_INCLUDED_BB8BBC71 */
