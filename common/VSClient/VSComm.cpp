/**
**************************************************************************
* \file VSComm.cpp
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
* \brief CVSComm Implementation
*
* \b Project Client
* \author Melechko Ivan
* \date 06.05.2003
*
* $Revision: 1 $
*
* $History: VSComm.cpp $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 6  *****************
 * User: Melechko     Date: 3.03.05    Time: 19:18
 * Updated in $/VS/VSClient
 * COMM port compress fix
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
*
* *****************  Version 4  *****************
* User: Melechko     Date: 1.07.04    Time: 15:01
* Updated in $/VS/VSClient
* COMM operation fix
*
* *****************  Version 3  *****************
* User: Melechko     Date: 3.06.03    Time: 16:31
* Updated in $/VS/VSClient
* New COMM sheme
*
* *****************  Version 2  *****************
* User: Melechko     Date: 16.05.03   Time: 17:47
* Updated in $/VS/VSClient
* new interface
*
* *****************  Version 1  *****************
* User: Melechko     Date: 6.05.03    Time: 10:56
* Created in $/VS/VSClient
* add comm
*
* *****************  Version 47  *****************
* User: Melechko     Date: 30.04.03    Time: 16:31
* Updated in $/VS/VSClient
* Hardware comp
*
****************************************************************************/

/****************************************************************************
* Includes
****************************************************************************/
#include "VSComm.h"
#include "std-generic/clib/rangecd.h"

/****************************************************************************
* Classes
****************************************************************************/
CVSComm::CVSComm(){
	m_hPort=INVALID_HANDLE_VALUE;
	m_hBuff=(unsigned char*)malloc(0x10000);
};

CVSComm::~CVSComm(){
	if(m_hPort!=INVALID_HANDLE_VALUE){
		Close();
	}
	free(m_hBuff);
};

int CVSComm::Open(char*PortName,int speed){
	DCB dc;
	COMMTIMEOUTS tout;
	COMSTAT cst;
	DWORD er;
	if(m_hPort!=INVALID_HANDLE_VALUE)
		Close();
	m_hPort=CreateFile(PortName,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,0);
	if(m_hPort!=INVALID_HANDLE_VALUE){
		GetCommState(m_hPort,&dc);
		switch(speed){
	  case    300:dc.BaudRate=CBR_300   ;break;
	  case    600:dc.BaudRate=CBR_600   ;break;
	  case   1200:dc.BaudRate=CBR_1200  ;break;
	  case   2400:dc.BaudRate=CBR_2400  ;break;
	  case   4800:dc.BaudRate=CBR_4800  ;break;
	  case   9600:dc.BaudRate=CBR_9600  ;break;
	  case  14400:dc.BaudRate=CBR_14400 ;break;
	  case  19200:dc.BaudRate=CBR_19200 ;break;
	  case  38400:dc.BaudRate=CBR_38400 ;break;
	  case  57600:dc.BaudRate=CBR_57600 ;break;
	  case 115200:dc.BaudRate=CBR_115200;break;
	  default:dc.BaudRate=CBR_38400;
		}
		dc.Parity=NOPARITY;
		dc.StopBits=ONESTOPBIT;
		dc.ByteSize=8;

		SetCommState(m_hPort,&dc);
		SetupComm(m_hPort,50240,50240);
		GetCommTimeouts(m_hPort,&tout);
		SetCommTimeouts(m_hPort,&tout);
		ClearCommError(m_hPort,&er,&cst);
		return 0;
	}
	return -1;

};

int CVSComm::Close(){
	if(m_hPort!=INVALID_HANDLE_VALUE){
		COMSTAT cst;
		DWORD er;
		ClearCommError(m_hPort,&er,&cst);
		CloseHandle(m_hPort);
	}
	m_hPort=INVALID_HANDLE_VALUE;
	return 0;
};

int CVSComm::Read(unsigned char *pBuff){
	COMSTAT cst;
	DWORD er;
	int iSize;
	if(m_hPort!=INVALID_HANDLE_VALUE){
		ClearCommError(m_hPort,&er,&cst);
		iSize=cst.cbInQue;
		if(iSize>0){
			int iSize_=iSize;
			ReadFile(m_hPort,m_hBuff,iSize,(LPDWORD)&iSize,NULL);
			iSize=RCDV_Encode(m_hBuff,iSize,pBuff);
			unsigned short *pSh=(unsigned short *)(pBuff+iSize);
			*pSh=iSize_;
			return iSize+2;
		}
	}
	return 0;
};

int CVSComm::Write(unsigned char *pBuff,int Size){
	if(m_hPort!=INVALID_HANDLE_VALUE){
		unsigned short *pSh;
		pSh=(unsigned short *)(pBuff+Size-2);
		Size=RCDV_Decode(pBuff,m_hBuff,*pSh);
		Size=*pSh;
		return WriteFile(m_hPort,m_hBuff,Size,(LPDWORD)&Size,NULL);
	}
	return 0;
};

