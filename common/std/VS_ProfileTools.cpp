/**
 **************************************************************************
 * \file VS_ProfileTools.cpp
 * \brief profiling classes
 *
 * \b Project Standart Libraries
 * \author SMirnovK & Matvey
 *
 ****************************************************************************/
#include "VS_ProfileTools.h"
#include "std-generic/cpplib/TimeUtils.h"

#include <boost/version.hpp>

#include <map>
#include <string>
#include <memory>
#include <mutex>

// In versions 1.60-1.66 Boost.Timer doesn't autolink Boost.Chrono which is used in its implementation.
#if defined(_WIN32) && (BOOST_VERSION > 105900 && BOOST_VERSION < 106700)
#	define BOOST_LIB_NAME boost_chrono
#	include <boost/config/auto_link.hpp>
#	define BOOST_LIB_NAME boost_system
#	include <boost/config/auto_link.hpp>
#endif

/****************************************************************************
 * Classes
 ****************************************************************************/

/**
 **************************************************************************
 * \brief one perfomance object container
 ****************************************************************************/

class ProfileItem
{
	unsigned long m_count;
	double        m_min;
	double        m_max;
	double        m_total;
public:
	ProfileItem() : m_count(0), m_min(0x7fffffff), m_max(0), m_total(0)
	{}

	~ProfileItem() {
	}
	void PushPerf(double t) {
		m_total+=t; m_count++;
		if (m_min > t) m_min = t;
		if (m_max < t) m_max = t;
	}
	unsigned long Count(){return m_count;}
	double Total() {return m_total;}
	double Avg(){return m_total/m_count;}
	double Min(){return m_min;}
	double Max(){return m_max;}
};

/**
 **************************************************************************
 * \brief perfomance collector
 ****************************************************************************/
class VS_ProfileDataCollector
{
	friend class VS_AutoCountTime;
protected:
	static std::unique_ptr<VS_ProfileDataCollector> instance;
	std::map<std::string, ProfileItem> m_perf_map;
	boost::timer::cpu_times m_time_min;
	boost::timer::cpu_times m_time_hour;
	boost::timer::cpu_timer m_timer;
	std::mutex m_mutex;
	std::map<std::string, std::string> m;

	VS_ProfileDataCollector();
	bool WriteToFile(const char *file = 0);
	void Periodic();
	void Clear();
public:
	static VS_ProfileDataCollector* Instance();
	~VS_ProfileDataCollector();
	void MarkPerfomance(const std::string& key, double work_time_s);
};


/****************************************************************************
 * VS_AutoCountTime
 ****************************************************************************/

static int64_t s_short_period_ns = 60000000000LL; // 60LL * 1000 * 1000 * 1000;

static int64_t s_long_period_ns = 3600000000000LL; // 60 * 60 * 1000 * 1000 * 1000;

/**
**************************************************************************
****************************************************************************/
VS_AutoCountTime::VS_AutoCountTime(const char *func_name)
{
	m_func_name = func_name;
}

/**
**************************************************************************
****************************************************************************/
VS_AutoCountTime::~VS_AutoCountTime()
{
	VS_ProfileDataCollector::Instance()->MarkPerfomance(m_func_name, static_cast<double>(m_timer.elapsed().wall) / 1000000000);
}

void VS_AutoCountTime::SetShortPeriod(int64_t short_period_ns)
{
	s_short_period_ns = short_period_ns;
}

void VS_AutoCountTime::SetLongPeriod(int64_t long_period_ns)
{
	s_long_period_ns = long_period_ns;
}

void VS_AutoCountTime::Clear()
{
	VS_ProfileDataCollector::Instance()->Clear();
}

/****************************************************************************
 * VS_ProfileDataCollector
 ****************************************************************************/
/// instance
std::unique_ptr<VS_ProfileDataCollector> VS_ProfileDataCollector::instance;

/**
**************************************************************************
****************************************************************************/
VS_ProfileDataCollector::VS_ProfileDataCollector()
{
	m_time_min = m_timer.elapsed();
	m_time_hour = m_timer.elapsed();
}

/**
**************************************************************************
****************************************************************************/
VS_ProfileDataCollector::~VS_ProfileDataCollector()
{
	WriteToFile();
}

/**
**************************************************************************
****************************************************************************/
void VS_ProfileDataCollector::Clear()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_perf_map.clear();
}

/**
**************************************************************************
****************************************************************************/
VS_ProfileDataCollector * VS_ProfileDataCollector::Instance()
{
	if (!instance)
		instance.reset(new VS_ProfileDataCollector);
	return instance.get();
}

/**
**************************************************************************
****************************************************************************/
void VS_ProfileDataCollector::MarkPerfomance(const std::string& key, double work_time_s)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_perf_map[key].PushPerf(work_time_s);
	Periodic();
}

/**
**************************************************************************
****************************************************************************/
bool VS_ProfileDataCollector::WriteToFile(const char *file)
{
	std::string file_name;
	if (file && *file)
		file_name = file;
	else
		file_name = "report_file.txt";
	FILE *fd = fopen(file_name.c_str(), "wt+");
	if (fd) {
		fprintf(fd, "                      name                         |  count | total[s] |     avg  |     min  |     max  |\n");
		for (auto it = m_perf_map.begin(); it != m_perf_map.end(); ++it) {
			ProfileItem item = it->second;
			std::string name = it->first;
			if (name.size() > 50) {
				name = name.substr(name.size() - 50, std::string::npos);
			}
			fprintf(fd, "%-50s | %6lu | %8.3f | %8.2f | %8.2f | %8.2f |\n", name.c_str(),
				item.Count(), item.Total(), item.Avg() * 1000, item.Min() * 1000, item.Max() * 1000);
		}
		fclose(fd);
	}
	return true;
}

/**
**************************************************************************
****************************************************************************/
void VS_ProfileDataCollector::Periodic()
{
	boost::timer::cpu_times t = m_timer.elapsed();
	if (t.wall - m_time_min.wall>=s_short_period_ns) {
		m_time_min = t;
		WriteToFile();
	}
	if (t.wall - m_time_hour.wall >= s_long_period_ns) {
		m_time_hour = t;
		WriteToFile(tu::TimeToString(std::chrono::system_clock::now(), "%Y %m-%d %H-%M-%S GMT").c_str());
		m_perf_map.clear();
	}
}

