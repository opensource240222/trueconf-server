/**
 **************************************************************************
 * \file VSInterface.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Base class used in many classes to communicate whith GUI
 *
 * \b Project Client
 * \author Melechko Ivan
 * \author SMirnovK
 * \date 27.02.2004
 *
 * $Revision: 11 $
 *
 * $History: VSInterface.cpp $
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 16.05.11   Time: 19:10
 * Updated in $/VSNA/VSClient
 * -goooogle bug :)
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 25.04.11   Time: 17:55
 * Updated in $/VSNA/VSClient
 * - copy reg keys
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 25.08.10   Time: 19:26
 * Updated in $/VSNA/VSClient
 * - some optimization in interfaces
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 9.07.10    Time: 16:07
 * Updated in $/VSNA/VSClient
 * - some funk
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 23.06.10   Time: 19:29
 * Updated in $/VSNA/VSClient
 * - interface for group conf
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 21.12.09   Time: 13:44
 * Updated in $/VSNA/VSClient
 * - unicode capability ReadParam
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 10.12.09   Time: 19:04
 * Updated in $/VSNA/VSClient
 * - unicode capability for hardware lists
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 3.12.09    Time: 11:13
 * Updated in $/VSNA/VSClient
 * - code cleanup vzochat7
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 20.02.08   Time: 21:44
 * Updated in $/VSNA/VSClient
 * undo NA for root name
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 18.01.08   Time: 17:38
 * Updated in $/VSNA/VSClient
 * - Discovery functions verified
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 2.06.06    Time: 13:58
 * Updated in $/VS/VSClient
 * - check not valid registry key
 *
 * *****************  Version 10  *****************
 * User: Melechko     Date: 6.03.06    Time: 16:52
 * Updated in $/VS/VSClient
 * Перенес обработку WRITE_CONFIGURATION и READ_CONFIGURATION в базовый
 * класс
 *
 * *****************  Version 9  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:02
 * Updated in $/VS/VSClient
 * some changes :)
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSClientBase.h"
#include "VS_ApplicationInfo.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../std/cpplib/VS_SimpleStr.h"


/**
 **************************************************************************
 * \brief Check  if current registry key in HKCU is empty
 ****************************************************************************/
void VS_CheckEmptyCase() {
	VS_RegistryKey KeyConfig(true, REG_CurrentConfiguratuon);
	if (!KeyConfig.IsValid()) {
		std::string name("Software\\");
		name += VS_RegistryKey::GetDefaultRoot();
		VS_RegistryKey::CopyKey(false, name.c_str(), true, name.c_str());
	}
}

/****************************************************************************
 * Classes
 ****************************************************************************/
CVSInterface::CVSInterface(const char *szName,CVSInterface* pParentInterface,char *szRootKey,bool bNeedKey)
{
	//
	if( szRootKey) {
		VS_RegistryKey::SetDefaultRoot(szRootKey);

		if (!VS_RegistryKey::InitDefaultBackend("registry:force_lm=false"))
			throw std::runtime_error("Can't initialize registry backend!");

		VS_CheckEmptyCase();
	}

	strcpy(m_InterfaceName,szName);
	// Список внешних интерфейсов
	m_InterfaceList.SetPredicate(VS_SimpleStr::Predicate);
	m_InterfaceList.SetKeyFactory(VS_SimpleStr::Factory, VS_SimpleStr::Destructor);
	m_pKey=NULL;
	m_bKeyOwner=false;
	m_pParentInterface=NULL;
	if(pParentInterface){
		m_pParentInterface=pParentInterface;
		m_pParentInterface->InterfaceRegistration(this);
		m_pKey=m_pParentInterface->m_pKey;
	}
	if (bNeedKey) {
		if (m_pKey && m_pKey->IsValid()) {
			char pBuff[MAX_PATH];
			char *szOldName=(char*)m_pKey->GetName();
			pBuff[0]=0;
			strcat(pBuff,szOldName);
			strcat(pBuff,"\\");
			strcat(pBuff,szName);
			m_pKey=new VS_RegistryKey(TRUE,pBuff,FALSE,TRUE);
		}
		else
			m_pKey=new VS_RegistryKey(TRUE,szName,FALSE,TRUE);
		m_bKeyOwner=true;
	}
}

CVSInterface::~CVSInterface()
{
	if((m_bKeyOwner)&&(m_pKey))
		delete m_pKey;
	if(m_pParentInterface)
		m_pParentInterface->RemoveInterface(this);
	m_InterfaceName[0] = 0;
}

CVSInterface *CVSInterface::FindChildrenByName(const char* szName)
{
	VS_Map::ConstIterator i=m_InterfaceList.Find(szName);
	if(i==m_InterfaceList.End())
		return NULL;
	return (CVSInterface*)(*i).data;
}

int CVSInterface::Process(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	const char* pSubStr = strchr(pSection, int('\\'));
	if(pSubStr==NULL){// Поиск среди функций
		switch(VS_OPERATION)
		{
		case READ_CONFIGURATION:{
			ReadParam(pSection,pVar);
			return VS_INTERFACE_OK;
								}
		case WRITE_CONFIGURATION:{
			WriteParam(pSection,pVar);
			return  VS_INTERFACE_OK;
								 }
		default:
			return ProcessFunction(VS_OPERATION,pSection,pVar);
		}

	}
	else {//Поис среди дочерних интерфейсов
		char InterfaceName[MAX_PATH];
		int iSize=pSubStr-pSection;
		if(iSize>sizeof(InterfaceName)-1)
			iSize=sizeof(InterfaceName)-1;
		strncpy(InterfaceName,pSection,iSize);
		InterfaceName[iSize]=0;
		VS_Map::ConstIterator i=m_InterfaceList.Find(InterfaceName);
		if(i==m_InterfaceList.End())
			return VS_INTERFACE_NO_INTERFACE;
		CVSInterface* pInterface=(CVSInterface*)(*i).data;
		return pInterface->Process(VS_OPERATION,pSubStr+1,pVar);
	}
	return VS_INTERFACE_OK;
}

void CVSInterface::InterfaceRegistration(CVSInterface* pInterface){
	if(pInterface==NULL)
		return;
	m_InterfaceList.Insert((void*)pInterface->GetInterfaceName(),(void*)pInterface);
}

void CVSInterface::RemoveInterface(CVSInterface* pInterface)
{
	if(pInterface==NULL)
		return;
	VS_Map::Iterator i=m_InterfaceList.Find((void*)pInterface->GetInterfaceName());
	if(i==m_InterfaceList.End())
		return;
	m_InterfaceList.Erase(i);
}

int CVSInterface::ReadParam(const char* szName, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;
	if(m_pKey==NULL)
		return -1;
	switch(var->vt)
	{
	case VT_BSTR:
		{
			wchar_t pBuff[MAX_PATH];
			pBuff[0]=0;
			if(m_pKey->GetValue(pBuff,MAX_PATH*sizeof(wchar_t),VS_REG_WSTRING_VT,szName)<=0)
				return -1;
			*var = (_bstr_t)pBuff;
			return 0;
		}
	case VT_BOOL:
	case VT_INT:
	case VT_UINT:
	case VT_I2:
	case VT_I4:
	case VT_UI2:
	case VT_UI4:
		{
			int iValue=0;
			if(m_pKey->GetValue(&iValue,sizeof(int),VS_REG_INTEGER_VT,szName)<=0)
				return -1;
			*var=iValue;
			return 0;
		}
	}
	return -1;
}

void CVSInterface::WriteParam(const char* szName, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;
	if(m_pKey==NULL)
		return;
	switch(var->vt)
	{
	case VT_BSTR:
		{
			m_pKey->SetValue((wchar_t*)(_bstr_t)*var, 0, VS_REG_WSTRING_VT, szName);
			return ;
		}
	case VT_BOOL:
	case VT_INT:
	case VT_UINT:
	case VT_I2:
	case VT_I4:
	case VT_UI2:
	case VT_UI4:
		{
			int iValue=int(*var);
			m_pKey->SetValue(&iValue,sizeof(int),VS_REG_INTEGER_VT,szName);
			return;
		}
	}
}

void CVSInterface::Apply2Children(VS_INTERFACE_TYPE TypeChecked, VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	VS_Map::Iterator pIterator;
	GetEnumInterface(&pIterator);
	CVSInterface *pInt;
	while (pInt=GetNextInterface(&pIterator)) {
		char *szName=pInt->GetInterfaceName();
		if (pInt->IsInterfaceSupport(TypeChecked)) {
			pInt->Process(VS_OPERATION,pSection,pVar);
		}
		pInt->Apply2Children(TypeChecked,VS_OPERATION,pSection,pVar);
	}
}

long CVSInterface::ExtractVars(VARIANT *&vars, VARIANT *pVar)
{
	vars = 0;
	int num = 0;
	if (pVar->vt==(VT_ARRAY|VT_VARIANT)) {
		long l, u;
		SAFEARRAY *psa = pVar->parray;
		SafeArrayGetLBound(psa, 1, &l);
		SafeArrayGetUBound(psa, 1, &u);
		num = u - l + 1;
		if (num > 0) {
			vars = new VARIANT[num];
			for(long i = 0; i < num; ++i) {
				VariantInit(&vars[i]);
				SafeArrayGetElement(psa, &i, &vars[i]);
			}
		}
	}
	return num;
}

bool CVSInterface::CombineVars(VARIANT *var, VARIANT *pVar, int num)
{
	SAFEARRAYBOUND rgsabound[1];
	SAFEARRAY * psa;
	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = num;
	psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
	if (psa==0)
		return false;

	var->parray=psa;
	var->vt= VT_ARRAY | VT_VARIANT;
	for (long i = 0; i<num; i++)
		SafeArrayPutElement(psa, &i, &pVar[i]);
	return true;
}

bool CVSInterface::IsBSTR(VARIANT* pVar)
{
	bool res = pVar->vt==VT_BSTR && pVar->bstrVal!=0;
	return res;
}
