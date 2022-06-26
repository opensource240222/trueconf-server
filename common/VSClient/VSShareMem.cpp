/**
 **************************************************************************
 * \file VSShareMem.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief shared memory classes implementation
 *
 * \b Project Client
 * \author Melechko Ivan
 * \date 07.10.2002
 *
 * $Revision: 1 $
 *
 * $History: VSShareMem.cpp $
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


/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSShareMem.h"

#include <stdlib.h>
#include <stdio.h>

/****************************************************************************
 * CSharedMemorySlave
 ****************************************************************************/
CSharedMemorySlave::CSharedMemorySlave(char*szFileName){

	strcpy(m_szFileName,szFileName);
	return;
};

int CSharedMemorySlave::Alloc(int size){
	char szFileName[MAX_PATH];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char ext[_MAX_EXT];

    _splitpath(m_szFileName, drive, dir,szFileName, ext );

	m_hMMFile=CreateFileMapping (INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,size,szFileName);
	if(m_hMMFile==INVALID_HANDLE_VALUE){
		return -1;
	}
	m_pMem=MapViewOfFile(m_hMMFile,FILE_MAP_ALL_ACCESS,0,0,0);
	if(m_pMem==NULL){
		CloseHandle(m_hMMFile);
		return -1;
	}
	return size;
};

void CSharedMemorySlave::Free(){
	if(m_pMem)
		UnmapViewOfFile(m_pMem);
	if(m_hMMFile)
		CloseHandle(m_hMMFile);
	return;
};

/****************************************************************************
 * CSharedMemoryMaster
 ****************************************************************************/
CSharedMemoryMaster::CSharedMemoryMaster(char*szFileName){
	/*char szPathName[MAX_PATH];
	GetTempPath (MAX_PATH, szPathName);
	m_szFileName_[0]=0;
	GetTempFileName (szPathName,"VSC",0,m_szFileName_);*/
	strcpy(m_szFileName,szFileName);
	return;
};

int CSharedMemoryMaster::Alloc(int size){
	char szFileName[MAX_PATH];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char ext[_MAX_EXT];

    _splitpath(m_szFileName, drive, dir,szFileName, ext );

	m_hMMFile=CreateFileMapping (INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE|SEC_COMMIT,0,size,szFileName);
	if(m_hMMFile==INVALID_HANDLE_VALUE){
		return -1;
	}
	m_pMem=MapViewOfFile(m_hMMFile,FILE_MAP_ALL_ACCESS,0,0,0);
	if(m_pMem==NULL){
		CloseHandle(m_hMMFile);
		return -1;
	}
	return size;
};

void CSharedMemoryMaster::Free(){
	UnmapViewOfFile(m_pMem);
	CloseHandle(m_hMMFile);
	DeleteFile(m_szFileName);
	return;
};

