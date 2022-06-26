#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <functional>
#include <vector>
#include <thread>
#include <memory>

#include "std/VS_ProfileTools.h"
#include "std/cpplib/ThreadUtils.h"

#define AUTO_PROF_ON

struct VS_CollectorMapItem {
	VS_CollectorMapItem() :count(0), min(0), max(0), total(0), avg(0) {}
	unsigned long count;
	double        min;
	double        max;
	double        total;
	double        avg;
	std::string   funcname;
};

void parseCollectorItems(const std::string& report_filename, std::vector<VS_CollectorMapItem>& report_items) {

	std::string s;
	std::ifstream report_file(report_filename.c_str());
	ASSERT_TRUE(report_file.is_open());

	for (std::getline(report_file, s); !report_file.eof();report_file.peek())  {
		VS_CollectorMapItem item;
		char fname[256];
		std::getline(report_file, s);
		std::replace(s.begin(), s.end(), '|', ' ');
		int numfields = sscanf( s.c_str(), "%s %ld %lf %lf %lf %lf", fname,
			&item.count,
			&item.total,
			&item.avg,
			&item.min,
			&item.max);
		ASSERT_EQ(numfields, 6);
		item.funcname.assign(fname);
		report_items.push_back(item);
	}
}

const int iternum = 10 * 1000;

void TestLoadFunc()
{
	AUTO_PROF
	double f = 0;
	for (int i = 0; i < iternum; i++) {
		f += sqrt(i*i);
	}
}

void TestLoadFunc_2()
{
	AUTO_PROF
	double f = 0;
	for (int i = 0; i < iternum; i++) {
		f += sqrt(i*i);
	}
}

void Call(const std::function<void(void)>& LoadFunc, int n) {
	VS_AutoCountTime::SetShortPeriod(60 * 1000 * 1000 * 1000LL);

	if (n == 0) {
		return;
	}
	for (int i = 0; i < n - 1; i++) {

		LoadFunc();
	}
	VS_AutoCountTime::SetShortPeriod(0); //to ensure table will be saved
	LoadFunc();
}


TEST(AutoCountTime, LoadFuncSingleCall) {
	std::vector<VS_CollectorMapItem> report_items;
	Call(TestLoadFunc, 1);
	parseCollectorItems("report_file.txt", report_items);
	ASSERT_EQ(report_items.size(), 1u);
	ASSERT_EQ(report_items[0].count, 1u);
}

TEST(AutoCountTime, LoadFuncMultiCall) {
	std::vector<VS_CollectorMapItem> report_items;
	VS_AutoCountTime::Clear();
	Call(TestLoadFunc, 100);
	parseCollectorItems("report_file.txt", report_items);
	ASSERT_EQ(report_items.size(), 1u);
	ASSERT_EQ(report_items[0].count, 100u);
}


TEST(AutoCountTime, MultiLoadFuncMultiCall) {
	std::vector<VS_CollectorMapItem> report_items;
	VS_AutoCountTime::Clear();
	Call(TestLoadFunc, 50);
	Call(TestLoadFunc_2, 50);
	parseCollectorItems("report_file.txt", report_items);
	ASSERT_EQ(report_items.size(), 2u);
	ASSERT_EQ(report_items[0].count, 50u);
	ASSERT_EQ(report_items[0].count, 50u);
}


TEST(AutoCountTime, LoadFuncMultiThread) {
	std::vector<VS_CollectorMapItem> report_items;
	std::vector<std::thread> threads;
	VS_AutoCountTime::Clear();
	for (int i = 0; i < 10; i++) {
		threads.emplace_back([]() {
			vs::SetThreadName("T:ProfileTools");
			Call(TestLoadFunc, 10);
		});
	}
	for (int i = 0; i < 10; i++) {
		threads[i].join();
	}
	parseCollectorItems("report_file.txt", report_items);
	ASSERT_EQ(report_items.size(), 1u);
	ASSERT_EQ(report_items[0].count, 100u);
}


TEST(AutoCountTime, MultiLoadFuncMultiThread) {
	std::vector<VS_CollectorMapItem> report_items;
	std::vector<std::thread> threads;
	VS_AutoCountTime::Clear();
	int i = 0;
	for (; i < 5; i++) {
		threads.emplace_back([]() {
			vs::SetThreadName("T:ProfileTools");
			Call(TestLoadFunc, 10);
		});
	}

	for (; i < 10; i++) {
		threads.emplace_back([]() {
			vs::SetThreadName("T:ProfileTools");
			Call(TestLoadFunc_2, 10);
		});
	}
	for (int i = 0; i < 10; i++) {
		threads[i].join();
	}

	parseCollectorItems("report_file.txt", report_items);
	EXPECT_EQ(report_items.size(), 2u);
	EXPECT_EQ(report_items[0].count, 50u);
	EXPECT_EQ(report_items[1].count, 50u);
}
