/**
 **************************************************************************
 * \file VSAudioCaptureList.cpp
 * (c) 2002-2005 Visicron Inc.  http://www.visicron.net/
 * \brief AudioCaptureList class definition
 * \b Project Client
 * \author Melechko Ivan
 * \date 14.01.2005
 *
 * $Revision: 5 $
 *
 * $History: VSAudioCaptureList.cpp $
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 25.08.10   Time: 19:26
 * Updated in $/VSNA/VSClient
 * - some optimization in interfaces
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 6.04.10    Time: 18:04
 * Updated in $/VSNA/VSClient
 * - were enabled DMO AEC ("EnableDMO" in registry)
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 27.10.09   Time: 16:12
 * Updated in $/VSNA/VSClient
 * - fix reenumerate DS devices list
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
 * *****************  Version 2  *****************
 * User: Melechko     Date: 25.01.05   Time: 16:47
 * Updated in $/VS/VSClient
 * Device list array HighBound fix
 *
 * *****************  Version 1  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:00
 * Created in $/VS/VSClient
 * some changes :)
 *
*/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSAudioCaptureList.h"
#include "../std/cpplib/VS_Protocol.h"
/****************************************************************************
 * Static
 ****************************************************************************/
/****************************************************************************
* CAudioCaptureList
****************************************************************************/
/**********************************/
CAudioCaptureList::CAudioCaptureList(CVSInterface* pParentInterface):
CVSInterface("AudioCaptureDevices",pParentInterface)
{
}

/**********************************/
int CAudioCaptureList::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;
	if (strcmp(pSection, "AudioCaptureList")==0) {
		if (VS_OPERATION==GET_PARAM) {
			bool res = false;
			int n = m_pSourceList->iGetMaxString();
			_variant_t* vars = new _variant_t[n];
			if (n > 0) {
				for(int i=0; i< n; i++)
					vars[i] = m_pSourceList->szGetStringByNumber(i);
				res = CombineVars(pVar, vars, n);
			}
			else
				res = CombineVars(pVar, vars, 0);
			return res ? VS_INTERFACE_OK : VS_INTERFACE_INTERNAL_ERROR;
		}
		else if (VS_OPERATION==RUN_COMMAND) {
			int ret = iGetDeviceList();
			g_DevStatus.SetStatus(DVS_SND_NOTPRESENT, true, ret!=0);
			return ret!=0 ? VS_INTERFACE_INTERNAL_ERROR : VS_INTERFACE_OK;
		}
	}
	return VS_INTERFACE_NO_FUNCTION;
}

/**********************************/
int CAudioCaptureList::iGetDeviceList(void)
{
	return VS_AudioDeviceManager::GetDeviceList(true, m_pSourceList);
}
