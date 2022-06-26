/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_RoutersWatchdog.cpp
/// \brief
/// \note
///

#include "time.h"

#include "VS_RoutersWatchdog.h"
#include "../Server/VS_Server.h"
#include "../../std/cpplib/VS_PerformanceMonitor.h"
#include "../../std/debuglog/VS_Debug.h"

#include <signal.h>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

volatile bool VS_RoutersWatchdog::flag_continue = false;
volatile bool VS_RoutersWatchdog::flag_exit = false;
volatile bool VS_RoutersWatchdog::flag_restart = false;
std::function<void(const char *, const bool)> VS_RoutersWatchdog::restart_function;
std::function<void()> VS_RoutersWatchdog::shutdown_function;

VS_RoutersWatchdog::VS_RoutersWatchdog()
	: isValid(false)
	, isInit(false)
	, in_seconds(0)
	, nTested(0)
	, old_Abnormal_termination(nullptr)
	, old_Floating_point_error(nullptr)
	, old_Illegal_instruction(nullptr)
	, old_CTRL_C_signal(nullptr)
	, old_Illegal_storage_access(nullptr)
	, old_Termination_request(nullptr)
{
	memset((void *)&tsts, 0, sizeof(tsts));
	static bool   flagUnique = false;	if (flagUnique)		return;
	flag_continue = flag_exit = flag_restart = false;	flagUnique = isValid = true;
	testable_that_false = 0;
}

bool VS_RoutersWatchdog::Init(const std::function<void(const char*, const bool)>& restart_func,
								const std::function<void()>& shutdown_func)
{
	if (!isValid)	return false;
	VS_RoutersWatchdog::restart_function = restart_func;
	VS_RoutersWatchdog::shutdown_function = shutdown_func;
#if defined(_WIN32)
	// Intercept crash-related signals on Windows only.
	// On Linux we will be restarted by external watchdog. Its better to crash normally to allow creation of core dump.
	old_Abnormal_termination = (signal)(SIGABRT, Sig_Abnormal_termination);
	old_Floating_point_error = (signal)(SIGFPE, Sig_Floating_point_error);
	old_Illegal_instruction = (signal)(SIGILL, Sig_Illegal_instruction);
	old_Illegal_storage_access = (signal)(SIGSEGV, Sig_Illegal_storage_access);
#endif
	old_CTRL_C_signal = (signal)(SIGINT, Sig_CTRL_C_signal);
	old_Termination_request = (signal)(SIGTERM, Sig_Termination_request);
	return isInit = flag_continue = true;
}

bool VS_RoutersWatchdog::AddTestable( VS_Testable *tst, const unsigned coef_tst )
{
	if (!tst || nTested >= VS_WATCHDOG_MAX_TESTED)
		return false;
	VS_Testable_Implementation &ts = tsts[nTested];
	ts.tst = tst;
	ts.coef_tst = coef_tst;
	++nTested;
	return true;
}

bool VS_RoutersWatchdog::Test( void )
{
	const auto pt = VS_PerformanceMonitor::Instance().GetTotalProcessorTime();
	if (pt >= 90 || pt < 0)
	{	const time_t   tm = time( 0 );
		dprint1( "The total processor time is %i%%\n", static_cast<int>(pt));
	}
	const bool ret = [&]() -> bool {
		if (!isInit || !flag_continue)		return false;
		for (unsigned i = 0; i < nTested; ++i)		if (!tsts[i].Test())
		{
			testable_that_false = tsts[i].coef_tst;
			return false;
		}
		return true;
	}();
	if (!ret) dprint1("Index of testable service that has been false  : %d\n",testable_that_false);
	return ret;
}

void VS_RoutersWatchdog::Restart( const unsigned in_sec )
{
	flag_continue = false;
	flag_restart = true;
	in_seconds = in_sec;
}

void VS_RoutersWatchdog::Shutdown( void )
{
	flag_continue = false;
	flag_exit = true;
	if(shutdown_function)
		shutdown_function();
}
