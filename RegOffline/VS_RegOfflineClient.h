#pragma once
#include "version.h"
#include "../common/std/cpplib/VS_SimpleStr.h"
#include "std-generic/cpplib/VS_Container.h"

#include <Windows.h>

#define WM_SERVICE_RC			WM_APP+1
// internal messages
#define WM_RC_DESTROY			WM_USER+1
// external messages
#define WM_RC_NOTIFY			WM_USER+2

class VS_RegOfflineClient
{
	VS_SimpleStr m_reg_file_path;
	DWORD		m_ThreadId;
	VS_SimpleStr m_Ep;
	VS_SimpleStr m_RegEp;
	HANDLE	m_hThread;
	HANDLE m_hDie;
	bool m_Valid;
	DWORD	ThreadCycle();
	void	Servise(MSG *msg);
	VS_SimpleStr m_reg_result_file;
	HANDLE			m_hRegCompleteEvent;
	bool			m_IsRegSuccess;
	VS_Container	m_reg_replay;
	VS_SimpleStr	m_Error;

	int m_reg_process;

	bool PrepareRegDataAlloc(void *&buf_out, unsigned long &out_sz);


public:
	VS_SimpleStr	m_server_name;
	VS_SimpleStr	m_server_id;

	VS_RegOfflineClient();
	~VS_RegOfflineClient();
	bool Init(const char *reg_file_path);
	bool MakeRegistration(HANDLE hRegComplete,const char* reg_result_file);
	bool IsRegSuccess();
	const char *ErrorDescr();
	void Release();

	friend DWORD WINAPI ThreadProc(void* param);
};