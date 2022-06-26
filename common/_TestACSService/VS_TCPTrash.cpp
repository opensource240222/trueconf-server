#include "VS_TCPTrash.h"

signed long		VS_TCPTrash::m_requestCount = 0;
HANDLE			VS_TCPTrash::m_hWTRASH = CreateEvent(NULL, true, false, NULL);
VS_TCPTrash*	VS_TCPTrash::m_hInstance = VS_TCPTrash::GetInstance();
volatile signed long VS_TCPTrash::m_terminate = 0;

VS_TCPTrash::VS_TCPTrash(void) {
	m_hTTRASH = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)garbageThread, this, 0, NULL);
	if (!m_hTTRASH) {
		printf("\nError : trash thread creation failed");
		return;
	}
	m_pTcpTable = (_MIB_TCPTABLE *)malloc(TRASH_SIZE);
	memset((void *)m_pTcpTable, 0, TRASH_SIZE);
}

VS_TCPTrash::~VS_TCPTrash(void) {
	free((void *)m_pTcpTable);
}

VS_TCPTrash *VS_TCPTrash::GetInstance() {
	if (!m_hInstance) {
		m_hInstance = new VS_TCPTrash();
	}
	m_requestCount ++;
    return m_hInstance;
}

void VS_TCPTrash::FreeInstance() {
	m_requestCount --;
	if (!m_requestCount) {
		delete this;
		m_hInstance = NULL;
	}
}

DWORD WINAPI VS_TCPTrash::garbageThread(void *arg) {
        return ((VS_TCPTrash *)arg)->doGarbage();
};

DWORD VS_TCPTrash::doGarbage() {
	while (true) {
		DWORD dwSize = TRASH_SIZE;
		WaitForSingleObject(VS_TCPTrash::m_hWTRASH, TRASH_TIMEOUT);
		if (InterlockedExchange(&m_terminate, m_terminate)) {
			::printf("\nExitting thread %d", m_hTTRASH);
			break;
		}
		if (GetTcpTable(m_pTcpTable, &dwSize, TRUE) == NO_ERROR) {
			for (unsigned int i = 0; i < m_pTcpTable->dwNumEntries; i++) {
				if (m_pTcpTable->table[i].dwState == MIB_TCP_STATE_CLOSE_WAIT) {
					m_pTcpTable->table[i].dwState = MIB_TCP_STATE_DELETE_TCB;
					SetTcpEntry(&(m_pTcpTable->table[i]));
	} } } } 
	m_hInstance = 0;
	ExitThread(0);
	return 0;
};
