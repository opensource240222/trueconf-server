/**
 **************************************************************************
 * \file VS_ProfileTools.h
 * \brief profile macros
 *
 * \b Project Standart Libraries
 * \author SMirnovK & Matvey
 ****************************************************************************/
#ifndef VS_PROFILETOOLS_H
#define VS_PROFILETOOLS_H

#include <boost/timer/timer.hpp>

#include <cstdint>

#define AUTO_PROF_ON

#ifdef AUTO_PROF_ON
	#define AUTO_PROF VS_AutoCountTime ___autoCountPerf(__FUNCTION__);
	#define AUTO_PROF_FUNCTION(name) VS_AutoCountTime ___autoCountPerf(name);
#else
	#define AUTO_PROF
#endif


/**
 **************************************************************************
 * \brief performance snap auto class
 ****************************************************************************/
class VS_AutoCountTime
{
	boost::timer::cpu_timer m_timer;
	std::string m_func_name;

public:

	VS_AutoCountTime(const char *func_name);
	~VS_AutoCountTime();

	static void SetShortPeriod(int64_t short_period_ns);
	static void SetLongPeriod(int64_t long_period_ns);
	static void Clear();
};

#endif /*VS_PROFILETOOLS_H*/
