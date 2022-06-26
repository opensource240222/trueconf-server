/**
 **************************************************************************
 * \file VS_PerformanceMonitor.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Retrive processor perfomance inforamation
 *
 * \b Project Standart Libraries
 * \author SMirnovK
 * \date 22.07.03
 *
 * $Revision: 1 $
 *
 * $History: VS_PerformanceMonitor.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 ****************************************************************************/
#pragma once

#include <memory>

class VS_PerformanceMonitor
{
	class Impl;

public:
	static VS_PerformanceMonitor& Instance()
	{
		static VS_PerformanceMonitor instance;
		return instance;
	}

	VS_PerformanceMonitor();
	~VS_PerformanceMonitor();

	// Starts perfomance monitor. Not thread safe.
	void Start();
	// Stops perfomance monitor. Not thread safe.
	void Stop();
	// Returns average system load in range [0., 100.] since unspecified point in time, but no earlier than time of Start() call.
	// Returns 0. while information isn't yet available.
	// Returns negative value if error has occured.
	// Thread safe by itself, not thread safe if Start() or Stop() is executed concurrently.
	double GetTotalProcessorTime() const;
	///
	double GetTotalProcessTime() const;
	///
	double GetTotalThreadTime(int32_t tid) const;
	///
	void AddMonitoringThread(int32_t tid);
	///
	void RemoveMonitoringThread(int32_t tid);

private:
	std::unique_ptr<Impl> m_impl;
};
