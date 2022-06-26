#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <functional>

class VS_ChildProcessImpl;

class VS_ChildProcess {
public:
	static const unsigned long ILLEGAL_PID;
public:
	VS_ChildProcess(const char *command);
	VS_ChildProcess(void);

	~VS_ChildProcess(void);

	bool RedirectOutputToFile(const char *output_file_path, bool append = false);

	bool Start(void);
	bool Start(const char *command);

	bool Wait(std::chrono::milliseconds timeout = std::chrono::milliseconds::max());

	//bool AskToStop(const unsigned long timeout_ms = TIMEOUT_INFINITE); // Nicely ask process to exit. Impossible to implement correctly on Windows for console processes.
	void Terminate(const int exit_code); // kill process

	bool Alive(void);
	unsigned long GetPID(void) const;
	bool GetExitCode(int &exit_code);
public:
	/*
	Artem Boldarev (31.01.2018):

	Unix command string recursive descent parser based on the following grammar.
	Seems to be pretty complete (or, at least, sufficient enough).
	It is intended to be used with the Unix process creation functions (exec() family).

	S = command .
	command =  part { part }  end .
	part = { whitespace } ( double_quoted_part | single_quoted_part | unquoted_part ) .
	double_quoted_part = " { character } " .
	single_quoted_part = ' { character } ' .
	unquoted_part = non_whitespace { non_whitespace } .
	character =  whitespace | non_whitespace .
	whitespace = <any_whitespace_character> .
	non_whitespace = <any_printable_non_whitespace_character> .
	end = '\0'.

	NOTES:
	You can use escaped single quotes, double quotes and backslahes inside strings (\\\", \\\' and \\\\ in C notation or \", \' and \\ as text).

	*/
	typedef std::function<void(const std::string &)> parse_command_string_callback;

	static bool ParseCommandString(const char *cmd_string, const parse_command_string_callback &callback);
	static bool ParseCommandString(const char *cmd_string, std::vector<std::string> &parsed_tokens);
private:
	// non copyable
	VS_ChildProcess(const VS_ChildProcess &other) = delete;
	VS_ChildProcess &operator=(const VS_ChildProcess &) = delete;
private:
	VS_ChildProcessImpl *impl;
};