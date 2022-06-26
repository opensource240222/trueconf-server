/**
 **************************************************************************
 * \file VSInterface.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Base class used in many classes to communicate whith GUI
 *
 * \b Project Client
 * \author Melechko Ivan
 * \date 27.02.2004
 *
 * $Revision: 6 $
 *
 * $History: VSInterface.h $
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 18.04.12   Time: 18:45
 * Updated in $/VSNA/VSClient
 * - bugfix#11012
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 16.05.11   Time: 19:10
 * Updated in $/VSNA/VSClient
 * -goooogle bug :)
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 25.08.10   Time: 19:26
 * Updated in $/VSNA/VSClient
 * - some optimization in interfaces
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 9.07.10    Time: 16:07
 * Updated in $/VSNA/VSClient
 * - some funk
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 23.06.10   Time: 19:29
 * Updated in $/VSNA/VSClient
 * - interface for group conf
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
 * User: Melechko     Date: 19.01.05   Time: 17:02
 * Updated in $/VS/VSClient
 * some changes :)
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 ****************************************************************************/
#ifndef _VS_INTERFACE_H
#define _VS_INTERFACE_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#ifdef _WIN32
#include <comutil.h>
#else
#include "std/clib/vs_defines.h"
#endif
#include "../std/cpplib/VS_Map.h"
#include "../std/cpplib/VS_SimpleStr.h"

class VS_RegistryKey;

/**
 **************************************************************************
 * \brief Operations passes to Process()
 ****************************************************************************/
enum VS_INTERFACE_OPERATION{
	SET_PARAM,
	GET_PARAM,
	RUN_COMMAND,
	READ_CONFIGURATION,
	WRITE_CONFIGURATION
};

enum VS_INTERFACE_TYPE{
	IT_VIDEOCAPTURE,
	IT_AUDIOCAPTURE
};

/****************************************************************************
 * Defines
 ****************************************************************************/
#define VS_INTERFACE_OK 0
#define VS_INTERFACE_NO_FUNCTION 1
#define VS_INTERFACE_NO_INTERFACE 2
#define VS_INTERFACE_INTERNAL_ERROR 3

/****************************************************************************
 * Classes
 ****************************************************************************/
/**
 **************************************************************************
 * \brief Base class used in many classes to communicate whith GUI
 ****************************************************************************/
class CVSInterface{
private:
	bool m_bKeyOwner;
	CVSInterface* m_pParentInterface;
protected:
	VS_RegistryKey *m_pKey;
	char   m_InterfaceName[MAX_PATH];
	VS_Map m_InterfaceList;
	virtual int ProcessFunction(VS_INTERFACE_OPERATION /*VS_OPERATION*/, const char* /*pSection*/, VARIANT* /*pVar*/) { return VS_INTERFACE_NO_FUNCTION; }
	void WriteParam(const char* szName, VARIANT* pVar);
	int ReadParam(const char* szName, VARIANT* pVar);
	void GetEnumInterface(VS_Map::Iterator *pIt){*(pIt)=m_InterfaceList.Begin();};
	CVSInterface *GetNextInterface(VS_Map::Iterator * pIt){
		CVSInterface *pRet=NULL;
		pRet=(CVSInterface *)(((*(pIt))!=m_InterfaceList.End())?(**pIt).data:NULL);
		++*pIt;
		return pRet;
	};
	void Apply2Children(VS_INTERFACE_TYPE TypeChecked, VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar);
	CVSInterface* FindChildrenByName(const char* szName);
	CVSInterface* GetParent() {return m_pParentInterface;}
public:
	/**
	* \param szName - имя секции
	* \param szRootKey - инициализация основного ключа реестра
	* \param pParentInterface - родительский интерфейс;
	* \param bNeedKey - секция открывает ключ соответствующий своему имени.
	*/
	CVSInterface(const char *szName,CVSInterface* pParentInterface,char *szRootKey=NULL,bool bNeedKey=false);
	virtual ~CVSInterface();
	char * GetInterfaceName(){ return m_InterfaceName;};
	virtual int Process(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar);
	void InterfaceRegistration(CVSInterface* pInterface);
	void RemoveInterface(CVSInterface* pInterface);
	virtual bool IsInterfaceSupport(VS_INTERFACE_TYPE TypeChecked){return false;};
	long ExtractVars(VARIANT *&vars, VARIANT *pVar);
	bool CombineVars(VARIANT *var, VARIANT *pVar, int num);
	bool IsBSTR(VARIANT* pVar);
};
/***************************************************************/
#endif