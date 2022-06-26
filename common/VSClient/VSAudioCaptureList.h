/**
 **************************************************************************
 * \file VSAudioCaptureList.h
 * (c) 2002-2005 Visicron Inc.  http://www.visicron.net/
 * \brief AudioCaptureList class declaration
 * \b Project Client
 * \author Melechko Ivan
 * \date 14.01.2005
 *
 * $Revision: 2 $
 *
 * $History: VSAudioCaptureList.h $
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 13.09.07   Time: 15:51
 * Updated in $/VS2005/VSClient
 * - bug with device ststuses when started without devices
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 1  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:00
 * Created in $/VS/VSClient
 * some changes :)
 *
*/
#ifndef _VSAUDCAP_LIST_H
#define _VSAUDCAP_LIST_H
/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSInterface.h"
#include "VSThinkClient.h"
/****************************************************************************
 * Declarations
 ****************************************************************************/
/**
 **************************************************************************
 * \brief CAudioCaptureList  retrive avaliable
 * devices
 ****************************************************************************/
class CAudioCaptureList: public CDeviceList,public CVSInterface{
protected:
	/// see CVSInterface
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
	/**********************/
	const static char _funcACList[];		///< Audio Render List
	int iGetDeviceList();
public:
	CAudioCaptureList(CVSInterface* pParentInterface);
};

#endif