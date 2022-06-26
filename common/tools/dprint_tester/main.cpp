#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <functional>

#include <boost/algorithm/string/replace.hpp>

#include <cstdio>
#include <csignal>

#include "parg.h"

#include "DprintTester.h"
#include "PrimitiveTimer.h"
#include "sysinfo.h"
#include "std-generic/cpplib/TimeUtils.h"

// application driver
static char *argv0 = nullptr;

static std::function<void(int)> signal_handler_function;

static void sighandler(int signal)
{
	if (signal_handler_function)
	{
		signal_handler_function(signal);
	}
}

static void show_help()
{
	std::cerr << "Usage:\n"
		<< argv0 << " -t <time in minutes> -n <number of threads> -o <file path> -r <file path>\n\n"
		"\tOptions:\n"
		"-t <minutes> - test execution time (default: 3);\n"
		"-n <number> - number of threads (default: 1);\n"
		"-o <file path> - output file path (default: standard output);\n"
		"-r <file path> - report file path (default: standard protocol);\n"
		"-h - show this help and exit.\n";
}

static void bad_option_argument(const char option)
{
	std::cerr << "Bad argument for option \'" << option << "\'\n\n";
	show_help();
}

static std::string bytes_into_readable_form(uint64_t bytes)
{
	std::stringstream out;
	uint64_t res;
	if ((res = bytes / (1024 * 1024)) > 0) // MB
	{
		out << res << " MiB";
	}
	else if ((res = bytes / (1024 * 1024)) > 0) // KB
	{
		out << res << " KiB";
	}
	else
	{
		out << res << " Bytes";
	}

	return out.str();
}

static std::string useconds_into_readable_form(uint64_t us)
{
	std::stringstream out;
	bool is_first = true;

	auto millis = us / 1000;
	us = us % 1000;

	auto seconds = millis / 1000;
	millis = millis % 1000;

	auto minutes = seconds / 60;
	seconds = seconds % 60;

	auto hours = minutes / 60;
	minutes = minutes % 60;

	auto days = hours / 24;
	hours = hours % 24;

	if (days)
	{
		if (is_first)
		{
			is_first = false;
		}
		else
		{
			out << " ";
		}
		out << days << "d";
	}

	if (hours)
	{
		if (is_first)
		{
			is_first = false;
		}
		else
		{
			out << " ";
		}
		out << hours << "h";
	}

	if (minutes)
	{
		if (is_first)
		{
			is_first = false;
		}
		else
		{
			out << " ";
		}
		out << minutes << "m";
	}

	if (seconds)
	{
		if (is_first)
		{
			is_first = false;
		}
		else
		{
			out << " ";
		}
		out << seconds << "s";
	}

	if (millis)
	{
		if (is_first)
		{
			is_first = false;
		}
		else
		{
			out << " ";
		}
		out << millis << "ms";
	}

	if (us || out.str().size() == 0)
	{
		if (is_first)
		{
			is_first = false;
		}
		else
		{
			out << " ";
		}
		out << us << "us";
	}

	return out.str();
}

// Beware! It is incomplete, ineffective and might be incorrect for some inputs.
static std::string escape(const char *str)
{
	std::string result = str;

	boost::algorithm::replace_all(result, "\\", "\\\\");
	boost::algorithm::replace_all(result, "\a", "\\a");
	boost::algorithm::replace_all(result, "\b", "\\b");
	boost::algorithm::replace_all(result, "\f", "\\f");
	boost::algorithm::replace_all(result, "\n", "\\n");
	boost::algorithm::replace_all(result, "\r", "\\r");
	boost::algorithm::replace_all(result, "\t", "\\t");
	boost::algorithm::replace_all(result, "\'", "\\'");
	boost::algorithm::replace_all(result, "\"", "\\\"");
	boost::algorithm::replace_all(result, "\?", "\\?");

	return result;
}

int main(int argc, char **argv)
{
	char *out_file_path = nullptr;
	char *report_file_path = nullptr;
	int n_threads = 1;
	int time_mins = 3;

	ARGBEGIN {
	case 't':
		time_mins = atoi(EARGF(bad_option_argument(ARGC())));
		if (time_mins <= 0)
		{
			std::cerr << "Wrong timeout value: " << time_mins << '\n';
			show_help();
			return 1;
		}
		break;
	case 'n':
		n_threads = atoi(EARGF(bad_option_argument(ARGC())));
		if (n_threads <= 0)
		{
			std::cerr << "Wrong threads count: " << n_threads << '\n';
			show_help();
			return 1;
		}
		break;
	case 'o':
		out_file_path = EARGF(bad_option_argument(ARGC()));
		break;
	case 'r':
		report_file_path = EARGF(bad_option_argument(ARGC()));
		break;
	case 'h':
	case '?':
		show_help();
		return 1;
		break;
	default:
		std::cerr << "Unknown option: " << ARGC() << '\n';
		show_help();
		return 1;
		break;
	} ARGEND;

	try
	{
		//ostream *report_stream = nullptr;
		std::ofstream report_file_stream;
		DprintTester tester;
		DprintTester::Report result_report;
		std::stringstream report_text;

		if (out_file_path)
		{
			if (freopen(out_file_path, "wt", stdout) == nullptr)
			{
				throw std::exception("Can't reopen stdout!");
			}
		}

		/*if (report_file_path)
		{
			report_file_stream.open(report_file_path);
			report_stream = static_cast<ostream *>(&report_file_stream);
		}
		else
		{
			report_stream = &std::cerr;
		}*/

		// do measurements
		{
			PrimitiveTimer timer;

			tester.Init(n_threads);
			signal_handler_function = [&tester, &timer](int signal) -> void {
				if (signal == SIGINT)
				{
					timer.Stop();
					std::cerr << "Got SIGINT signal (Ctrl-C). Stopping...\n";
					tester.Stop();
				}
			};

			timer.SetCallback([&tester](void)->void { // timer handler
				tester.Stop();
			});
			timer.Start(time_mins * 60, true);

			signal(SIGINT, sighandler); // set Ctrl-C handler
			tester.Start();
			signal(SIGINT, SIG_DFL); // remove Ctrl-C signal handler
		}

		// handle results
		tester.GatherReport(result_report);

		// report name
		{
			size_t len1 = 0, len2 = 0;
			report_text << "=====================================================================\n\n";
			len1 = report_text.str().size();
			report_text << "\tReport for " << GetComputerNameString() << " (" << tu::TimeToString(std::chrono::system_clock::now(), "%d/%m/%Y %X", true) << ")\n";
			len2 = report_text.str().size();
			if (len1 != len2)
			{
				report_text << "\t";
			}
			for (size_t i = 2; i < len2 - len1; i++)
			{
				report_text  << "*";
			}

			report_text << "\n\n";
		}
		// OS/hardware information
		{
			// print hardware information
			report_text << "\tOperating System Information\n"
				<< "Name:\t\t " << GetFriendlyOSNameString() << '\n'
				<< "Architecture:\t " << GetCPUArchitectureString() << '\n'
				<< "Summary:\t " << GetOSInfoString() << '\n'
				<< "====\n\n";

			unsigned int cpus_count = 0, active_cpus_count = 0;
			cpus_count = GetCPUsCount(&active_cpus_count);

			report_text << "\tHardware Information\n"
				<< "CPUs count:\t " << cpus_count << " (active: " << active_cpus_count << ")" << '\n';
			for (unsigned int i = 0; i < cpus_count; i++)
			{
				report_text << "CPU" << i << ":\t\t " << GetCPUInfoString(i) << '\n';
			}

			uint64_t ram_total = 0, ram_avalable = 0;
			unsigned int ram_load = 0;

			report_text << '\n';
			if (GetPhysicalMemoryInfo(ram_total, ram_avalable, ram_load))
			{
				report_text << "RAM Total:\t " << bytes_into_readable_form(ram_total) << '\n';
				report_text << "RAM Available:\t " << bytes_into_readable_form(ram_avalable) << " (memory load: " << ram_load << "%" << ")" << '\n';
			}

			report_text << "====" << '\n' << '\n';
		}

		// sampling data
		{
			report_text << "\tPerformance Report" << '\n';
			report_text << "Threads count:\t\t\t " << n_threads << '\n';
			report_text << "Calls count:\t\t\t " << result_report.m_call_count << '\n';
			report_text << "Total execution time:\t\t " << useconds_into_readable_form(result_report.m_total_execution_time.count()) << '\n';
			report_text << "Output to file:\t\t\t " << (out_file_path != nullptr ? "yes" : "no") << '\n';
			report_text << "Average call execution time:\t " << useconds_into_readable_form(result_report.m_real_execution_time.count() / result_report.m_call_count) << '\n';
			report_text << "MAX call execution time:\t " << useconds_into_readable_form(result_report.m_max_dprint_execution_time.count()) << '\n';
			report_text << "\tFormat string:\t\t " << "\"" << escape((result_report.m_max_dprint_execution_time_fmt != nullptr ? result_report.m_max_dprint_execution_time_fmt : "<NULL>")) << "\"" << '\n';
			report_text << "MIN call execution time:\t " << useconds_into_readable_form(result_report.m_min_dprint_execution_time.count()) << '\n';
			report_text << "\tFormat string:\t\t " << "\"" << escape((result_report.m_max_dprint_execution_time_fmt != nullptr ? result_report.m_min_dprint_execution_time_fmt : "<NULL>")) << "\"" << '\n';
			{

				auto time_seconds = (result_report.m_total_execution_time.count() / 1000 / 1000);
				report_text << "Performance:\t\t\t ";
				if (time_seconds > 0)
				{
					report_text << (result_report.m_call_count / time_seconds) << " calls/sec" << '\n';
				}
				else
				{
					report_text << "<not enough samples>" << '\n';
				}
			}
			report_text << "====" << '\n';
		}

		std::cerr << report_text.str();
		if (report_file_path)
		{
			report_file_stream.open(report_file_path);
			report_file_stream << report_text.str();
			report_file_stream.close();
		}
		/*
		*report_stream << report_text.str() << "\n";
		// cleanup
		if (report_stream != &std::cerr)
		{
			report_file_stream.close();
		}*/
	}
	catch (std::exception e)
	{
		std::cerr << "Fatal eror :" << e.what() << '\n';
	}

    return 0;
}
