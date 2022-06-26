/**
 **************************************************************************
 * \file VSComm.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Comm port comunication class
 *
 * \b Project Client
 * \author Melechko Ivan
 * \date 06.05.2003
 *
 * $Revision: 1 $
 *
 * $History: VSComm.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
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
* User: Melechko     Date: 19.02.04   Time: 16:35
* Updated in $/VS/VSClient
*
* *****************  Version 2  *****************
* User: Melechko     Date: 3.06.03    Time: 16:31
* Updated in $/VS/VSClient
* New COMM sheme
*
* *****************  Version 1  *****************
* User: Melechko     Date: 6.05.03    Time: 10:56
* Created in $/VS/VSClient
* add comm
*
* *****************  Version 20  *****************
* User: Melechko     Date: 29.04.03    Time: 16:31
* Updated in $/VS/VSClient
* Hardware comp
*
****************************************************************************/
#ifndef _VSCOMM_
#define _VSCOMM_

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSClientBase.h"

/****************************************************************************
 * Classes
 ****************************************************************************/
/**
 **************************************************************************
 * \brief State of comm port
 ****************************************************************************/
class CVSCommTransportState{
public:
	int                           InputSpeed;
	int                           OutputSpeed;
	int                           Mode;
	char                          szInputPort[MAX_PATH];
	char                          szOutputPort[MAX_PATH];
	int                           iPortEnabled;
};

/**
 **************************************************************************
 * \brief Comm port comunication class
 ****************************************************************************/
class CVSComm
{
public:
	CVSComm();
	~CVSComm();
	int Open(char*PortName,int speed);
	int Close();
	int Read(unsigned char *pBuff);
	int Write(unsigned char *pBuff,int Size);
private:
	HANDLE m_hPort;
	unsigned char* m_hBuff;
};

#endif
