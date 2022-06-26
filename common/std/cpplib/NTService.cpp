#ifdef _WIN32

#include "NTService.h"
#include <memory>
#include <mutex>

#include <windows.h>
#include <cstdlib>
#include <cwchar>

#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

/*
Note by Artem Boldarev (15.02.2019):
Mostly based on https://bitbucket.org/arbv/foobar_service/ which is, in turn, based on https://code.msdn.microsoft.com/windowsapps/CppWindowsService-cacf4948/view/SourceCode#content.
*/

namespace vs {
	class NTServiceAccessor;

	class NTService {
		friend  NTServiceAccessor;
	public:
		static const DWORD DEF_WAIT_HINT;

		static std::shared_ptr<NTService> GetInstance();

		void Init(const std::function<int(void)> &service_body, const std::function<void(void)> &on_stop);
		bool RunServiceDispatcher(int &service_exit_status);
		bool IsInitialised(void);
	private:
		NTService();
		void StopService(void);
		void RunService(void);
		bool ReportStatusToSCM(DWORD dwCurrentState, DWORD dwExitCode, DWORD dwWaitHint);

		void AddToEventLog(WORD wType, DWORD dwErr, const char *msg);
	private:
		static std::mutex g_service_lock;
		static std::shared_ptr<NTService> g_service;
	private:
		int m_exit_status;
		bool m_is_initalised;
		std::string m_service_name;
		std::function<int(void)> m_service_body;
		std::function<void(void)> m_on_stop;
		SERVICE_STATUS ssStatus;
		SERVICE_STATUS_HANDLE sshStatusHandle;
	};

	const DWORD NTService::DEF_WAIT_HINT = 15000; // 15 Sec
	std::mutex NTService::g_service_lock;
	std::shared_ptr<NTService> NTService::g_service;

	class NTServiceAccessor
	{
	public:
		NTServiceAccessor(std::shared_ptr<NTService> service)
			: m_service(service)
		{}

		int CallServiceEntry(void)
		{
			m_service->m_service_body();
		}

		void CallStopService(void)
		{
			m_service->StopService();
		}

		void RunService(void)
		{
			m_service->RunService();
		}

		void SetServiceName(const char *service_name)
		{
			if (service_name != nullptr)
			{
				m_service->m_service_name = service_name;
			}
		}

		bool ReportStatusToSCM(DWORD dwCurrentState, DWORD dwExitCode, DWORD dwWaitHint)
		{
			return m_service->ReportStatusToSCM(dwCurrentState, dwExitCode, dwWaitHint);
		}

		bool ReportCurrentStatusToSCM(DWORD dwExitCode, DWORD dwWaitHint)
		{
			return ReportStatusToSCM(m_service->ssStatus.dwCurrentState, dwExitCode, dwWaitHint);
		}

		std::shared_ptr<NTService> m_service;
	};

	NTService::NTService()
		: m_is_initalised(false), sshStatusHandle(nullptr), m_exit_status(EXIT_FAILURE)
	{
		memset(&ssStatus, 0, sizeof(ssStatus));
	}

	void NTService::Init(const std::function<int(void)> &service_body, const std::function<void(void)> &on_stop)
	{
		if (m_is_initalised)
			return;
		if (service_body == nullptr || on_stop == nullptr)
		{
			return;
		}

		m_service_body = service_body;
		m_on_stop = on_stop;

		m_is_initalised = true;
	}

	bool NTService::IsInitialised(void)
	{
		return m_is_initalised;
	}

	bool NTService::ReportStatusToSCM(DWORD dwCurrentState, DWORD dwExitCode, DWORD dwWaitHint)
	{
		static DWORD dwCheckPoint = 1;
		BOOL bResult = TRUE;

		if (dwCurrentState == SERVICE_START_PENDING || dwCurrentState == SERVICE_STOP_PENDING)
			ssStatus.dwControlsAccepted = 0;
		else
			ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_NETBINDCHANGE;

		ssStatus.dwCurrentState = dwCurrentState;
		ssStatus.dwWin32ExitCode = dwExitCode;
		ssStatus.dwWaitHint = dwWaitHint;

		if (dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED)
			ssStatus.dwCheckPoint = 0;
		else
			ssStatus.dwCheckPoint = dwCheckPoint++;

		bResult = SetServiceStatus(sshStatusHandle, &ssStatus);
		if (!bResult)
		{
			AddToEventLog(EVENTLOG_ERROR_TYPE, GetLastError(), "SetServiceStatus() error.");
		}
		return bResult == TRUE;

	}

	static int GetErrorTextDescription(DWORD err_code, char *buf, int buflen)
	{
		char *s = NULL;
		size_t len;
		int printed = 0;

		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, err_code,
			MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
			(char *)&s, 0, NULL);
		if (s != NULL)
		{
			/* remove \r\n */
			if ((len = strlen(s)) >= 2)
			{
				s[len - 2] = '\0';
				s[len - 3] = '\0';
			}
			printed = snprintf(buf, buflen, "%s", s);
			LocalFree(s);
		}
		return printed;
	}


	void NTService::AddToEventLog(WORD wType, DWORD dwErr, const char *lpszMsg)
	{
		std::string message = m_service_name;
		HANDLE  hEventSource;
		const char *lpszStrings[2];
		char win_err_desc[1024] = { 0 };


		/* Use event logging to log the error. */
		hEventSource = RegisterEventSourceA(NULL, m_service_name.c_str());
		message += " WinAPI error code: ";
		message += std::to_string(dwErr);
		if (GetErrorTextDescription(dwErr, win_err_desc, sizeof(win_err_desc)) > 0)
		{
			message += " (";
			message += win_err_desc;
			message += ")";
		}
		message += ".";

		if (hEventSource != NULL)
		{
			lpszStrings[0] = message.c_str();
			lpszStrings[1] = lpszMsg;

			/* TODO: we might want to add an empty event 0 ID description to the executable's resource data */
			ReportEventA(hEventSource, /* handle of event source */
				wType,  /* event type */
				0,                    /* event category */
				0,                    /* event ID */
				NULL,                 /* current user's SID */
				2,                    /* strings in lpszStrings */
				0,                    /* no bytes of raw data */
				(const char **)lpszStrings,          /* array of error strings */
				(LPVOID)NULL);                /* no raw data */

			DeregisterEventSource(hEventSource);
		}

		dprint3("%s", message.c_str());
	}

	static void WINAPI ServiceCtrlHandler(DWORD dwCtrlCode)
	{
		NTServiceAccessor sa(NTService::GetInstance());
		/* Handle the requested control code. */
		switch (dwCtrlCode)
		{
		case SERVICE_CONTROL_SHUTDOWN:
		case SERVICE_CONTROL_STOP:
			sa.ReportStatusToSCM(SERVICE_STOP_PENDING, NO_ERROR, NTService::DEF_WAIT_HINT);
			/* Send event that stops our service */
			sa.CallStopService();
			return;
			/* Update the service status. */
		case SERVICE_CONTROL_INTERROGATE:
			break;
			/* SERVICE_ACCEPT_NETBINDCHANGE codes */
		case SERVICE_CONTROL_NETBINDADD:
			break;
		case SERVICE_CONTROL_NETBINDREMOVE:
			break;
		case SERVICE_CONTROL_NETBINDENABLE:
			break;
		case SERVICE_CONTROL_NETBINDDISABLE:
			break;
			/* unexpected control code */
		default:
			break;
		}
		sa.ReportCurrentStatusToSCM(NO_ERROR, 0);
	}

	static void WINAPI ServiceMain(DWORD argc, char *argv[])
	{
		NTServiceAccessor sa(NTService::GetInstance());
		if (argc >= 1)
		{
			sa.SetServiceName(argv[0]);
		}
		sa.RunService();
	}

	void NTService::RunService(void) try
	{
		sshStatusHandle = RegisterServiceCtrlHandlerA(m_service_name.c_str(), ServiceCtrlHandler);
		if (!sshStatusHandle)
		{
			throw std::runtime_error("Call to RegisterServiceCtrlHandler() has failed.");
		}

		ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		ssStatus.dwServiceSpecificExitCode = 0;
		if (!ReportStatusToSCM(SERVICE_START_PENDING, NO_ERROR, DEF_WAIT_HINT))
		{
			throw std::runtime_error("Call to ReportStatusToSCM() has failed.");
		}

		AddToEventLog(EVENTLOG_INFORMATION_TYPE, 0, "Service has been started.");
		ReportStatusToSCM(SERVICE_RUNNING, NO_ERROR, 0);

		m_exit_status = m_service_body();

		ReportStatusToSCM(SERVICE_STOPPED, GetLastError(), 0);
		std::string exit_message = "Service has stopped with the exit code ";
		exit_message += std::to_string(m_exit_status);
		exit_message += ".";
		AddToEventLog(EVENTLOG_INFORMATION_TYPE, 0, exit_message.c_str());
	}
	catch (const std::exception& e)
	{
		if (sshStatusHandle)
			ReportStatusToSCM(SERVICE_STOPPED, GetLastError(), 0);

		AddToEventLog(EVENTLOG_ERROR_TYPE, GetLastError(), e.what());
	}

	bool NTService::RunServiceDispatcher(int &service_exit_status)
	{
		char empty_service_name[1] = { 0 };
		/*
		 * Service Dispatch Table.
		 * Here we specify ServiceMain(),
		 * that called by SCM when service starts.
		*/
		if (!IsInitialised())
		{
			return false;
		}

		SERVICE_TABLE_ENTRYA dispatchTable[2];

		dispatchTable[0].lpServiceName = empty_service_name; // it is not required for SERVICE_WIN32_OWN_PROCESS
		dispatchTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTIONA)ServiceMain;
		/* Second structure must be filled with NULL */
		dispatchTable[1].lpServiceName = NULL;
		dispatchTable[1].lpServiceProc = NULL;

		/* Should be OK when executable started by Service Control Manager */
		if (StartServiceCtrlDispatcherA(dispatchTable) == 0)
			return false;

		service_exit_status = m_exit_status;

		return true;
	}

	void NTService::StopService(void)
	{
		m_on_stop();
	}

	static std::mutex g_service_lock;
	static std::shared_ptr<NTService> g_service;

	std::shared_ptr<NTService> NTService::GetInstance()
	{
		std::lock_guard<std::mutex> l(g_service_lock);

		if (g_service == nullptr)
		{
			g_service.reset(new NTService);
		}

		return g_service;
	}

	bool RunNTServiceDispatcher(int &service_exit_status, const std::function<int(void)> &service_body, const std::function<void(void)> &on_stop)
	{
		auto nts = NTService::GetInstance();

		if (nts->IsInitialised())
			return false;

		nts->Init(service_body, on_stop);
		return nts->RunServiceDispatcher(service_exit_status);
	}
}

#endif // _WIN32
