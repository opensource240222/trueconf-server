#pragma once
#include "tools/Watchdog/VS_Testable.h"

#include <functional>

#define   VS_WATCHDOG_MAX_TESTED   32

#ifndef _WIN32
#define __cdecl
#endif

struct VS_Testable_Implementation
{
	VS_Testable* tst;
	unsigned coef_tst, n_coef;
	inline bool Test(void)
	{
		if (++n_coef > coef_tst)
		{
			n_coef = 0;
			return tst->Test();
		}
		return true;
	}
};

class VS_RoutersWatchdog
{
	bool   isValid, isInit;
	static volatile bool   flag_continue, flag_exit, flag_restart;
	static std::function<void(const char *, const bool)> restart_function;
	static std::function<void()> shutdown_function;
	unsigned   in_seconds;
	VS_Testable_Implementation   tsts[VS_WATCHDOG_MAX_TESTED];		unsigned nTested;
	unsigned int testable_that_false;
	void(__cdecl *old_Abnormal_termination)(int),
		(__cdecl *old_Floating_point_error)(int),
		(__cdecl *old_Illegal_instruction)(int),
		(__cdecl *old_CTRL_C_signal)(int),
		(__cdecl *old_Illegal_storage_access)(int),
		(__cdecl *old_Termination_request)(int);

public:
	VS_RoutersWatchdog();
	bool		IsValid( void ) const {	return isValid;		}
	bool		Init(const std::function<void(const char*, const bool)>& restart_func,
						const std::function<void()>& shutdown_func);
	bool		IsInit( void ) const {		return isInit;		}
	bool		Exit( void ) const {		return flag_exit;	}
	bool		AddTestable( VS_Testable *tst, const unsigned coef_tst );
	bool		Test( void );
	void		Restart( const unsigned in_seconds = 0 );
	bool		IsRestart( void ) const {	return flag_restart;	}
	unsigned	InSeconds( void ) const {	return in_seconds;		}
	void		Shutdown( void );
	bool		IsShutdown() const { return (!flag_continue && flag_exit); }

	static void __cdecl Sig_Abnormal_termination(int /*sig_val*/)
	{
		if (restart_function)	(restart_function)("Abnormal termination.", true);
	}

	static void __cdecl Sig_Floating_point_error(int /*sig_val*/)
	{
		if (restart_function)	(restart_function)("Floating-point error, such as overflow, division by zero, or invalid operation.", true);
	}

	static void __cdecl Sig_Illegal_instruction(int /*sig_val*/)
	{
		if (restart_function)	(restart_function)("Illegal instruction.", true);
	}

	static void __cdecl Sig_CTRL_C_signal(int /*sig_val*/)
	{
#ifdef _WIN32
		flag_continue = false;	flag_exit = true;
#endif
		if (restart_function)	(restart_function)("SIGINT request sent to the program.", true);
	}

	static void __cdecl Sig_Illegal_storage_access(int /*sig_val*/)
	{
		if (restart_function)	(restart_function)("Illegal storage access.", true);
	}

	static void __cdecl Sig_Termination_request(int /*sig_val*/)
	{
		if (restart_function)	(restart_function)("Termination request sent to the program.", true);
	}
};

#ifndef _WIN32
#undef __cdecl
#endif
