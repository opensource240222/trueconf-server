#include "VS_SysBenchmarkWindows.h"

VS_SysBenchmarkWindows::VS_SysBenchmarkWindows()
{
	m_BenchEvent = CreateEvent(0, 0, 0, 0);
}

VS_SysBenchmarkWindows::~VS_SysBenchmarkWindows()
{
	DesactivateThread();
	Release();
	CloseHandle(m_BenchEvent);
	delete m_pAutoLevelCaps;
}

bool VS_SysBenchmarkWindows::Run()
{
	InitBenchmark();

	if (m_bInit)
		ActivateThread(this);
	else
		Release();

	return m_bInit;
}

DWORD VS_SysBenchmarkWindows::Loop(LPVOID hEvDie)
{
	CalcLevels();

	SetEvent(m_BenchEvent);

	return 0;
}

bool VS_SysBenchmarkWindows::CheckNeedDecreaseRating()
{
	OSVERSIONINFOA osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOA));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	if (GetVersionEx(&osvi)) {
		if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && osvi.dwMajorVersion >= 6) {
			return false;
		}
	}
	return true;
}
