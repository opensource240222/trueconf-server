#include "VS_ChildProcess.h"
#include <string>
#include <vector>


#ifdef _WIN32
#include <windows.h>
#include <cassert>
#include <cstdlib>
#include <cstring>

class VS_ChildProcessImpl {
	friend VS_ChildProcess;
private:
	enum States
	{
		StInit = 0,
		StStarted,
		StStopped,
	};
public:
	VS_ChildProcessImpl(void);
	VS_ChildProcessImpl(const char *command);
	~VS_ChildProcessImpl(void);

	bool RedirectOutputToFile(const char *output_file_path, bool append);

	bool Start(void);
	bool Start(const char *command);
	bool Wait(std::chrono::milliseconds timeout);
	void Terminate(const int exit_code);
	bool Alive(void);

	unsigned long GetPID(void) const;
	bool GetExitCode(int &exit_code);
private:
	void Cleanup(void);
	bool RedirectOutput(const void *output_file_path, bool unicode, bool append);
private:
	int m_state;
	std::string m_command;

	STARTUPINFOW m_si;
	PROCESS_INFORMATION m_pi;
	HANDLE m_job;
};

VS_ChildProcessImpl::VS_ChildProcessImpl(void)
	: m_state(StInit), m_job(NULL)
{
	memset(&m_si, 0, sizeof(m_si));
	m_si.cb = sizeof(m_si);
	memset(&m_pi, 0, sizeof(m_pi));
}

VS_ChildProcessImpl::VS_ChildProcessImpl(const char *command)
	: VS_ChildProcessImpl()
{
	m_command = (command && *command != '\0' ? command : "");
}

void VS_ChildProcessImpl::Cleanup(void)
{
	if (m_pi.hThread)
	{
		CloseHandle(m_pi.hThread);
	}

	if (m_pi.hProcess)
	{
		CloseHandle(m_pi.hProcess);
	}

	memset(&m_pi, 0, sizeof(m_pi));

	if (m_si.hStdError)
	{
		CloseHandle(m_si.hStdError);
	}

	if (m_si.hStdOutput)
	{
		CloseHandle(m_si.hStdOutput);
	}

	if (m_si.hStdInput)
	{
		CloseHandle(m_si.hStdInput);
	}

	memset(&m_si, 0, sizeof(m_si));

	if (m_job)
	{
		CloseHandle(m_job);
		m_job = NULL;
	}
}

VS_ChildProcessImpl::~VS_ChildProcessImpl(void)
{
	if (m_state == StStarted)
	{
		Terminate(1);
	}

	Cleanup();
}

unsigned long VS_ChildProcessImpl::GetPID(void) const
{
	if (m_state < StStarted)
	{
		return VS_ChildProcess::ILLEGAL_PID;
	}
	return m_pi.dwProcessId;
}

void VS_ChildProcessImpl::Terminate(const int exit_code)
{
	if (m_state != StStarted)
		return;

	TerminateProcess(m_pi.hProcess, (UINT)exit_code);
	Wait(std::chrono::milliseconds::max());
}

bool VS_ChildProcessImpl::RedirectOutput(const void *output_file_path, bool unicode, bool append)
{
	SECURITY_ATTRIBUTES saAttr;
	HANDLE hStdOut = NULL, hStdError = NULL;
	if (m_state != StInit || output_file_path == NULL)
		return false;

	if (unicode && (*((wchar_t *)output_file_path)) == L'\0')
		return false;
	else if ((*((char *)output_file_path)) == '\0')
		return false;

	memset(&saAttr, 0, sizeof(saAttr));
	saAttr.nLength = sizeof(saAttr);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	auto TryCreateFile = [&saAttr, &unicode, &output_file_path](DWORD dwDesiredAccess, DWORD dwCreationDisposition) -> HANDLE {
		HANDLE output;

		if (unicode)
			output = CreateFileW((wchar_t *)output_file_path, dwDesiredAccess, FILE_SHARE_WRITE | FILE_SHARE_READ, &saAttr, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
		else
			output = CreateFileA((char *)output_file_path, dwDesiredAccess, FILE_SHARE_WRITE | FILE_SHARE_READ, &saAttr, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

		if (output == INVALID_HANDLE_VALUE)
			return NULL;

		return output;
	};

	// redirect standart output and standard protocl to the file
	if (append)
	{
		// try to append new data to the end of the existing file
		hStdOut = TryCreateFile(FILE_APPEND_DATA, OPEN_EXISTING);
		// file does not exist, create a new one
		if (!hStdOut && GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			hStdOut = TryCreateFile(GENERIC_WRITE, CREATE_ALWAYS);
		}
	}
	else
	{
		// create new file
		hStdOut = TryCreateFile(GENERIC_WRITE, CREATE_ALWAYS);
	}

	if (!hStdOut)
		return false;

	if (!DuplicateHandle(GetCurrentProcess(), hStdOut,
		GetCurrentProcess(), &hStdError,
		0, TRUE, DUPLICATE_SAME_ACCESS))
	{
		CloseHandle(hStdOut);
		return false;
	}

	// close existing descriptors, if any, and change descitpors to the new ones
	if (m_si.hStdOutput)
	{
		CloseHandle(m_si.hStdOutput);
		m_si.hStdOutput = NULL;
	}

	if (m_si.hStdError)
	{
		CloseHandle(m_si.hStdError);
		m_si.hStdInput = NULL;
	}

	m_si.hStdOutput = hStdOut;
	m_si.hStdError = hStdError;
	m_si.dwFlags |= STARTF_USESTDHANDLES;

	return true;
}

bool VS_ChildProcessImpl::RedirectOutputToFile(const char *output_file_path, bool append)
{
	if (m_state != StInit)
		return false;

	std::vector<wchar_t> out;
	{
		auto size = MultiByteToWideChar(CP_UTF8, 0, output_file_path, -1, NULL, 0);
		if (size == 0)
			return false;

		out.resize(size);

		auto res = MultiByteToWideChar(CP_UTF8, 0, output_file_path, -1, &out[0], out.size());
		if (res == 0)
			return false;
	}

	return RedirectOutput((void *)&out[0], true, append);
}

bool VS_ChildProcessImpl::Start(void)
{
	BOOL res;
	BOOL inherit_handles;
	if (m_state != StInit)
		return false;

	// create job object
	{
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_limit_info;

		memset(&job_limit_info, 0, sizeof(job_limit_info));

		job_limit_info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		m_job = CreateJobObject(NULL, NULL);
		if (m_job == NULL)
		{
			return false;
		}

		// kill child process on parent exit
		if (SetInformationJobObject(m_job, JobObjectExtendedLimitInformation, &job_limit_info, sizeof(job_limit_info)) == 0)
		{
			CloseHandle(m_job);
			m_job = NULL;
			return false;
		}
	}

	// create process
	{
		std::vector<wchar_t> vcmd;

		{
			auto size = MultiByteToWideChar(CP_UTF8, 0, m_command.c_str(), -1, NULL, 0);
			if (size == 0)
				return false;

			vcmd.resize(size);

			auto res = MultiByteToWideChar(CP_UTF8, 0, m_command.c_str(), -1, &vcmd[0], vcmd.size());
			if (res == 0)
				return false;
		}


		if ((m_si.dwFlags & STARTF_USESTDHANDLES) == 0)
		{
			inherit_handles = FALSE;
		}
		else
		{
			inherit_handles = TRUE;
		}

		res = CreateProcessW(NULL, (WCHAR *)&vcmd[0], NULL, NULL, inherit_handles, CREATE_NEW_CONSOLE, NULL, NULL, &m_si, &m_pi);
		if (res == FALSE)
			return false;
	}

	// Assign new process to the job object (should not fail in general).
	if (AssignProcessToJobObject(m_job, m_pi.hProcess) == 0)
	{
		CloseHandle(m_job);
		m_job = NULL;
		TerminateProcess(m_pi.hProcess, EXIT_FAILURE);
		CloseHandle(m_pi.hProcess);
		CloseHandle(m_pi.hThread);
		memset(&m_pi, 0, sizeof(m_pi));
		return false;
	}

	m_state++;

	return true;
}

bool VS_ChildProcessImpl::Start(const char *command)
{
	if (m_state != StInit)
		return false;

	if (command && *command != '\0')
	{
		m_command = command;
	}

	return Start();
}

bool VS_ChildProcessImpl::GetExitCode(int &exit_code)
{
	DWORD dwExitCode;
	if (m_state != StStopped)
		return false;

	if (GetExitCodeProcess(m_pi.hProcess, &dwExitCode) != FALSE)
	{
		exit_code = int (dwExitCode);
		return true;
	}

	return true;
}

bool VS_ChildProcessImpl::Wait(std::chrono::milliseconds timeout)
{
	if (m_state == StInit)
		return false;
	if (m_state == StStopped)
		return true;

	assert(m_state == StStarted);

	DWORD ms = timeout.count() > INFINITE ? INFINITE : static_cast<DWORD>(timeout.count());
	const auto res = WaitForSingleObject(m_pi.hProcess, ms);
	if (res == WAIT_OBJECT_0)
	{
		m_state = StStopped;
		return true;
	}
	return false;
}

bool VS_ChildProcessImpl::Alive(void)
{
	if (m_state != StStarted)
	{
		return false;
	}
	return !Wait(std::chrono::milliseconds(0));
}

#else // POSIX

#include <thread>

#include <unistd.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/resource.h>
#include <sys/wait.h>

class VS_ChildProcessImpl {
	friend VS_ChildProcess;
private:
	enum States
	{
		StInit = 0,
		StStarted,
		StStopped,
	};
public:
	VS_ChildProcessImpl(void);
	VS_ChildProcessImpl(const char *command);
	~VS_ChildProcessImpl(void);

	bool RedirectOutputToFile(const char *output_file_path, bool append);

	bool Start(void);
	bool Start(const char *command);
	bool Wait(std::chrono::milliseconds timeout);
	void Terminate(const int exit_code);
	bool Alive(void);

	unsigned long GetPID(void) const;
	bool GetExitCode(int &exit_code);
private:
	void Cleanup(void);
	pid_t ForkExec(const std::vector<std::string> &args);
	bool CloseInheritedFileDescriptors(void);
	bool RedirectStandardFileDescriptors(void);
private:
	int m_state;
	std::string m_command;
	pid_t m_pid;
	int m_exit_code;
	int m_output_handle;
};

VS_ChildProcessImpl::VS_ChildProcessImpl(void)
	: m_state(StInit), m_pid(static_cast<pid_t>(VS_ChildProcess::ILLEGAL_PID)),
	m_exit_code(EXIT_FAILURE), m_output_handle(-1)
{
}

VS_ChildProcessImpl::VS_ChildProcessImpl(const char *command)
	: VS_ChildProcessImpl()
{
	if (command != nullptr && *command != '\0')
	{
		m_command = command;
	}
}

VS_ChildProcessImpl::~VS_ChildProcessImpl(void)
{
	if (m_state == StStarted)
	{
		Terminate(EXIT_FAILURE);
	}

	Cleanup();
}


void VS_ChildProcessImpl::Terminate(int exit_code) // exit code is ignored
{
	if (m_state != StStarted)
		return;

	if (Alive() && kill(m_pid, SIGKILL) == 0)
	{
		Wait(std::chrono::milliseconds::max());
		m_exit_code = exit_code;
	}
}

void VS_ChildProcessImpl::Cleanup()
{
	if (m_output_handle != -1)
	{
		close(m_output_handle);
	}
}

bool VS_ChildProcessImpl::Alive(void)
{
	if (m_state != StStarted)
		return false;

	int status = -1;
	const auto res = waitpid(m_pid, &status, WNOHANG);
	if (res == -1)
	{
		assert(errno != EINTR && "waitpid(,,WNOHANG) shouldn't fail with EINTR");
		assert(false);
		return false;
	}

	if (res == 0)
		return true; // no state change so far

	if (WIFEXITED(status)) // process has been terminated
	{
		m_exit_code = WEXITSTATUS(status);
		m_state = StStopped;
		return false;
	}
	else if (WIFSIGNALED(status)) // process has been terminated by a signal
	{
		m_exit_code = -(WTERMSIG(status));
		m_state = StStopped;
		return false;
	}
	else if (WIFSTOPPED(status)) // process has been stopped by a signal (but has not been terminated yet)
		return true;
	else // unexpected (unlikely)
	{
		m_exit_code = status;
		m_state = StStopped;
		return false;
	}
}

bool VS_ChildProcessImpl::GetExitCode(int &exit_code)
{
	if (m_state != StStopped)
		return false;

	exit_code = m_exit_code;

	return true;
}

unsigned long VS_ChildProcessImpl::GetPID(void) const
{
	return static_cast<unsigned long>(m_pid);
}

bool VS_ChildProcessImpl::CloseInheritedFileDescriptors(void)
{
	struct rlimit rl;

	memset(&rl, 0, sizeof(rl));
	if (getrlimit(RLIMIT_NOFILE, &rl) != 0)
	{
		return -1;
	}

	for (int i = 3; i < (int)rl.rlim_cur; i++)
	{
		if (i != m_output_handle)
		{
			close(i);
		}
	}

	return true;
}

bool VS_ChildProcessImpl::RedirectStandardFileDescriptors(void)
{
	int dev_null_fd = open("/dev/null", O_RDWR);

	if (dev_null_fd < 0)
	{
		return false;
	}

	//dup2(dev_null_fd, 0); // stdin
	if (m_output_handle >= 0)
	{
		if (dup2(m_output_handle, 1) == -1 || dup2(m_output_handle, 2) == -1)
		{
			close(m_output_handle);
			close(dev_null_fd);
			return false;
		}
	}
	else
	{
		if (dup2(dev_null_fd, 1) == -1 || dup2(dev_null_fd, 2) == -1)
		{
			close(dev_null_fd);
			return false;
		}
	}

	close(dev_null_fd);
	return true;
}

bool VS_ChildProcessImpl::Wait(std::chrono::milliseconds timeout)
{
	if (m_state == StInit)
		return false;
	if (m_state == StStopped)
		return true;

	assert(m_state == StStarted);

	// handle non-blocking case without querying time
	if (timeout == std::chrono::milliseconds(0))
		return !Alive();

	const auto start_time = std::chrono::steady_clock::now();
	timeout = std::min<std::chrono::milliseconds>(timeout, std::chrono::hours(24*365)); // large timeout values may overflow when converted to steady_clock::duration
	while (Alive())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		if (std::chrono::steady_clock::now() - start_time >= timeout)
			return false; // timeout
	}

	return true;
}

pid_t VS_ChildProcessImpl::ForkExec(const std::vector<std::string> &args)
{
	pid_t pid = -1;

	switch ((pid = fork())) /* fork process */
	{
	case -1: /* error */
		return -1;
		break;
	case 0:  /* first  child */
	{
		if (!CloseInheritedFileDescriptors())
		{
			_exit(EXIT_FAILURE);
		}

		if (!RedirectStandardFileDescriptors())
		{
			_exit(EXIT_FAILURE);
		}

		if (m_output_handle >= 0)
		{
			close(m_output_handle);
		}

		/* create session */
		if (setsid() == -1) /* error */
		{
			_exit(EXIT_FAILURE);
		}

		std::vector<char*> argv;
		argv.reserve(args.size() + 1);

		/* initialize data for execvp() */
		auto exename = args[0].c_str();
		for (const auto& arg : args)
		{
			argv.push_back(const_cast<char*>(arg.c_str()));
		}
		argv.push_back(nullptr);

		/* run program */
		if (execvp(exename, argv.data()) == -1)
		{
			_exit(EXIT_FAILURE);
		}
		/* unreachable on success */
	}
	break;
	default: /* parent */
	{
		return pid;
	}
	break;
	}

	/* not reachable */
	return -1;
}

bool VS_ChildProcessImpl::Start(void)
{
	if (m_state != StInit)
		return false;

	std::vector<std::string> args;
	if (!VS_ChildProcess::ParseCommandString(m_command.c_str(), args))
	{
		return false;
	}

	auto pid = ForkExec(args);
	if (pid != -1)
	{
		m_pid = pid;
		m_state++;
		return true;
	}
	return false;
}

bool VS_ChildProcessImpl::Start(const char *command)
{
	if (m_state != StInit)
		return false;

	if (command && *command != '\0')
	{
		m_command = command;
	}

	return Start();
}


bool VS_ChildProcessImpl::RedirectOutputToFile(const char *output_file_path, bool append)
{
	if (m_state != StInit || output_file_path == NULL || *output_file_path == '\0')
		return false;

	int flags = O_WRONLY | O_CREAT;
	if (append)
	{
		flags |= O_APPEND;
	}

	int fd = open(output_file_path, flags, 0660);
	if (fd <= -1)
	{
		return false;
	}

	if (m_output_handle >= 0)
	{
		close(m_output_handle);
	}

	m_output_handle = fd;

	return true;
}

#endif

const unsigned long VS_ChildProcess::ILLEGAL_PID = -1;

VS_ChildProcess::VS_ChildProcess(void)
{
	impl = new VS_ChildProcessImpl();
}

VS_ChildProcess::VS_ChildProcess(const char *command)
{
	impl = new VS_ChildProcessImpl(command);
}

VS_ChildProcess::~VS_ChildProcess(void)
{
	delete impl;
}

bool VS_ChildProcess::RedirectOutputToFile(const char *output_file_path, bool append)
{
	return impl->RedirectOutputToFile(output_file_path, append);
}

bool VS_ChildProcess::Start(void)
{
	return impl->Start();
}

bool VS_ChildProcess::Start(const char *command)
{
	return impl->Start(command);
}

bool VS_ChildProcess::Wait(std::chrono::milliseconds timeout)
{
	return impl->Wait(timeout);
}

void VS_ChildProcess::Terminate(const int exit_code)
{
	impl->Terminate(exit_code);
}

bool VS_ChildProcess::Alive(void)
{
	return impl->Alive();
}

unsigned long VS_ChildProcess::GetPID(void) const
{
	return impl->GetPID();
}

bool VS_ChildProcess::GetExitCode(int &exit_code)
{
	return impl->GetExitCode(exit_code);
}

bool VS_ChildProcess::ParseCommandString(const char *cmd_string, const parse_command_string_callback &callback)
{
	if (callback == nullptr || cmd_string == nullptr || *cmd_string == '\0')
	{
		return false;
	}

	size_t pos = 0;
	std::string token;
	bool inside_token = false;
	const char *in = cmd_string;

	// utils
	auto match = [&](char c) -> bool {
		return in[pos] == c;
	};

	auto advance = [&]() -> void {
		if (inside_token)
		{
			token.push_back(in[pos]);
		}
		pos++;
	};

	auto skip = [&]() -> void {
		pos++;
	};

	auto getchar = [&]() -> int {
		return in[pos];
	};

	// parsing rules
	auto end = [&]() -> bool {
		if (match('\0'))
		{
			// advance(); // do not advance past '\0' character.
			return true;
		}

		return false;
	};

	auto whitespace = [&]() -> bool
	{
		if (isspace(getchar()))
		{
			advance();
			return true;
		}

		return false;
	};

	auto non_whitespace = [&]() -> bool
	{
		int c = getchar();
		if (c > ' ' && // ignore control and space characters
			c != '\"' && c != '\'' && c != '\0' &&
			c != 127) // 127 - DEL Control Character
		{
			if (c != '\\')
			{
				advance();
				return true;
			}
			else
			{
				skip();
				if (match('\\') || match('\"') || match('\''))
				{
					advance();
					return true;
				}
				else
				{
					skip(); // unexpected escaped character
					return false;
				}
			}
		}

		return false;
	};

	auto character = [&]() -> bool
	{
		return whitespace() || non_whitespace();
	};

	auto unquoted_part = [&]() -> bool
	{
		if (!non_whitespace())
			return false;

		while (non_whitespace())
			;

		return true;
	};


	auto double_quoted_part = [&]() -> bool
	{
		if (match('\"'))
			skip();
		else
			return false;

		while (character())
			;

		if (match('\"'))
			skip();
		else
			return false;

		return true;
	};

	auto single_quoted_part = [&]() -> bool
	{
		if (match('\''))
			skip();
		else
			return false;

		while (character())
			;

		if (match('\''))
			skip();
		else
			return false;

		return true;
	};

	auto part = [&]() -> bool
	{
		while (whitespace())
			;

		inside_token = true;
		auto res = double_quoted_part() || single_quoted_part() || unquoted_part();

		if (res == true)
		{
			callback(token);
			token.clear();
		}

		inside_token = false;
		return res;
	};

	auto command = [&]() -> bool
	{
		if (!part())
			return false;

		while (part())
			;

		return end();
	};

	// command - start symbol;
	return command();
}


bool VS_ChildProcess::ParseCommandString(const char *cmd_string, std::vector<std::string> &parsed_tokens)
{
	std::vector<std::string> tokens;
	auto callback = [&tokens](const std::string &token) -> void {
		tokens.push_back(token);
	};

	auto res = ParseCommandString(cmd_string, callback);
	if (res == true)
	{
		parsed_tokens = std::move(tokens);
	}

	return res;
}
