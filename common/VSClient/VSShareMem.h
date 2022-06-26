/**
 **************************************************************************
 * \file VSShareMem.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Clases managed shared memory between different proceses
 *
 * \b Project Client
 * \author Melechko Ivan
 * \date 07.10.2002
 *
 * $Revision: 1 $
 *
 * $History: VSShareMem.h $
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
 * User: Melechko     Date: 19.01.05   Time: 17:02
 * Updated in $/VS/VSClient
 * some changes :)
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 ****************************************************************************/
#ifndef _VSSHARE_MEM_H
#define _VSSHARE_MEM_H


/****************************************************************************
 * Includes
 ****************************************************************************/
//#include <streams.h>
#include <Windows.h>
#include <Winbase.h>

/**
 **************************************************************************
 * \brief Shared memory master class
 ****************************************************************************/
class CSharedMemoryMaster{
public:
	CSharedMemoryMaster(char*szFileName);
	//char*GetTmpName(){return m_szFileName;};
	int Alloc(int size);
	void Free();
	LPVOID GetPointer(){return m_pMem;};
private:
	HANDLE m_hMMFile;
	LPVOID m_pMem;
	char m_szFileName[MAX_PATH];
	//char m_szFileName_[MAX_PATH];
};

/**
 **************************************************************************
 * \brief Shared memory slave class
 ****************************************************************************/
class CSharedMemorySlave{
public:
	CSharedMemorySlave(char*szFileName);
	int Alloc(int size);
	void Free();
	LPVOID GetPointer(){return m_pMem;};
private:
	char m_szFileName[MAX_PATH];
	HANDLE m_hMMFile;
	LPVOID m_pMem;
};

#endif
