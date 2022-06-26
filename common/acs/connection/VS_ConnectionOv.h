
#ifndef VS_CONNECTION_OV_H
#define VS_CONNECTION_OV_H
class VS_IOHandler;
#include "windows.h"
struct VS_Overlapped
{
	OVERLAPPED   over;
	VS_ACS_Field   field1, field2, field3;
	unsigned long   b_ov, b_last, b_trans, b_want;
	HANDLE   hiocp;
	DWORD   error;
	VS_IOHandler	*io_handler;
};
// end VS_Overlapped struct

#endif // VS_CONNECTION_OV_H
