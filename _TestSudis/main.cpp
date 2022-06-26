#include <boost/thread.hpp>
#include "../../common/apache_thrift/unit_solutions/VS_Sudis.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/acs/Lib/VS_AcsLib.h"
#include "../../common/std/debuglog/VS_Debug.h"
#include <windows.h>

unsigned long g_start_tick;

void func1(const char* user, const char* pass)
{
	std::string user_token;
	unsigned long t1 = GetTickCount();
	bool res = VS_Sudis::CheckAccount(user/*"ttest001"*/, pass/*"0P4lrr7E"*/, user_token);
	unsigned long t2 = GetTickCount();
	printf("CheckAccount result: res:%d, user_token:%s\n", res, user_token.c_str());
	if (res && user_token.length())
	{
		bool res2 = VS_Sudis::CheckUserRole(user/*"ttest001"*/, user_token.c_str());
		printf("CheckRole result: res:%d\n", res2);
	}
	if (res)
	{
		bool res2 = VS_Sudis::CheckUserList(user/*"ttest001"*/);
		printf("CheckUserList result: res:%d\n", res2);
	}
	//printf("+++++++ Task, t_diff: %d, curr_tick: %d, all_diff: %d\n", t2-t1, GetTickCount(), GetTickCount()-g_start_tick);
}

//////////////////// kt: test templates
class A
{
public:
	void fff(int x)
	{
		x = 1;
	}
};

template<class T, class Arg, void (T::*ptr)(Arg) >
void foo(Arg a)
{
	//typedef void (T::* my_func)(Arg);			// no typedef needed

	T t;
	(t.*ptr)(a);		//	i want to call: t.bar(arg);
}
///////////////////////

int main(int argc, char* argv[])
{
	///// kt: test templates
	foo<A, int, &A::fff>(3);
	///////////////////////////


	VS_AcsLibInitial();
	VS_RegistryKey::SetRoot("SVKS-M\\Server");
	if (!VS_Sudis::Init())
	{
		printf("Init sudis failed\n");
		return 0;
	}
	//	ttest001 0P4lrr7E
	//	ttest002 tY3Su9V5
	//	ttest003 U6te6Me5
	//	ttest004 0R8ievCE
	//	ttest005 w42ZfJ2m
	//	ttest006 o867yZwV
	//	ttest007 TlEH2e61
	//	ttest008 A3craO50
	//	ttest009 7Fgz7uQY
	//	ttest0010 nk4xT6SI
	const char* user = "ttest001";
	const char* pass = "0P4lrr7E";
	if (argc > 1)
		user = argv[1];
	if (argc > 2)
		pass = argv[2];
	func1(user, pass);
	//int NUM_TASK = 1;
	//if (argc>1)
	//	NUM_TASK = atoi(argv[1]);
	//
	//boost::thread_group g;
	//printf("---------- Start %d tasks ----(tick: %d)-----\n", NUM_TASK, GetTickCount());
	//g_start_tick = GetTickCount();
	//for(int i=0; i<NUM_TASK; ++i)
	//{
	//	g.create_thread(&func1);
	//}
	//printf("---------- End start %d tasks ----(tick: %d)-----\n", NUM_TASK, GetTickCount());
	//printf("TIME: %d\n", GetTickCount() - g_start_tick);
	//g.join_all();
	return 0;
}