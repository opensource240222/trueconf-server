#include "std/cpplib/VS_ChildProcess.h"
#include "std-generic/cpplib/scope_exit.h"

#include <gtest/gtest.h>

#include <cstdlib>
#include <cstdio>

#ifndef _WIN32
#define _unlink unlink
#endif

#ifdef _WIN32

#define COMPLETABLE_COMMAND "ping 127.0.0.1 -n 1"
#define NON_COMPLETABLE_COMMAND "ping 127.0.0.1 -t"

#else
#define COMPLETABLE_COMMAND "dd if=/dev/zero of=/dev/null bs=1 count=1 conv=notrunc"
#define NON_COMPLETABLE_COMMAND "dd if=/dev/zero of=/dev/null"
#endif

namespace child_process_test {
	class ChildProcessTest :
		public ::testing::Test
	{
	protected:
		ChildProcessTest()
		{}

		virtual void SetUp()
		{}

		virtual void TearDown()
		{}
	};

	TEST_F(ChildProcessTest, ParseCommandStringTest)
	{
		std::vector<std::string> tokens;

		///
		EXPECT_TRUE(VS_ChildProcess::ParseCommandString("ls some_file", tokens));

		EXPECT_EQ(tokens[0], "ls");
		EXPECT_EQ(tokens[1], "some_file");
		tokens.clear();
		///
		EXPECT_TRUE(VS_ChildProcess::ParseCommandString("ls -l -a /etc", tokens));

		EXPECT_EQ(tokens[0], "ls");
		EXPECT_EQ(tokens[1], "-l");
		EXPECT_EQ(tokens[2], "-a");
		EXPECT_EQ(tokens[3], "/etc");
		tokens.clear();
		///
		EXPECT_TRUE(VS_ChildProcess::ParseCommandString("/bin/ls -l -a \"/some/directory/with whitespace/\" ", tokens));

		EXPECT_EQ(tokens[0], "/bin/ls");
		EXPECT_EQ(tokens[1], "-l");
		EXPECT_EQ(tokens[2], "-a");
		EXPECT_EQ(tokens[3], "/some/directory/with whitespace/");
		tokens.clear();
		///
		EXPECT_TRUE(VS_ChildProcess::ParseCommandString("ls -l --colour=auto /", tokens));

		EXPECT_EQ(tokens[0], "ls");
		EXPECT_EQ(tokens[1], "-l");
		EXPECT_EQ(tokens[2], "--colour=auto");
		EXPECT_EQ(tokens[3], "/");
		tokens.clear();
		///
		EXPECT_TRUE(VS_ChildProcess::ParseCommandString("/bin/ls -l -a \"/some/directory/\\\"with quotes and spaces\\\"\" ", tokens));

		EXPECT_EQ(tokens[0], "/bin/ls");
		EXPECT_EQ(tokens[1], "-l");
		EXPECT_EQ(tokens[2], "-a");
		EXPECT_EQ(tokens[3], "/some/directory/\"with quotes and spaces\"");
		tokens.clear();
		///
		EXPECT_TRUE(VS_ChildProcess::ParseCommandString("/bin/ls -l -a \"/some/directory/\\\"with\\\\backspases\\\\and spaces\\\"\" ", tokens));

		EXPECT_EQ(tokens[0], "/bin/ls");
		EXPECT_EQ(tokens[1], "-l");
		EXPECT_EQ(tokens[2], "-a");
		EXPECT_EQ(tokens[3], "/some/directory/\"with\\backspases\\and spaces\"");
		tokens.clear();
		///
		EXPECT_TRUE(VS_ChildProcess::ParseCommandString("/bin/ls -l -a \'/some/directory/\\\'with single quotes\\\'\' ", tokens));

		EXPECT_EQ(tokens[0], "/bin/ls");
		EXPECT_EQ(tokens[1], "-l");
		EXPECT_EQ(tokens[2], "-a");
		EXPECT_EQ(tokens[3], "/some/directory/\'with single quotes\'");
		tokens.clear();
		///
		EXPECT_TRUE(VS_ChildProcess::ParseCommandString("/bin/ls -l -a \"/some/directory/\\\'with single quotes\\\'\" ", tokens));

		EXPECT_EQ(tokens[0], "/bin/ls");
		EXPECT_EQ(tokens[1], "-l");
		EXPECT_EQ(tokens[2], "-a");
		EXPECT_EQ(tokens[3], "/some/directory/\'with single quotes\'");
		tokens.clear();
		///
	}

	TEST_F(ChildProcessTest, BasicConsoleTest)
	{
		VS_ChildProcess proc;
		int exit_code = 0;

		ASSERT_EQ(proc.GetPID(), VS_ChildProcess::ILLEGAL_PID);
		ASSERT_FALSE(proc.Alive());
		ASSERT_TRUE(proc.Start(NON_COMPLETABLE_COMMAND));
		ASSERT_TRUE(proc.Alive());
		ASSERT_FALSE(proc.Start());
		ASSERT_NE(proc.GetPID(), VS_ChildProcess::ILLEGAL_PID);
		ASSERT_FALSE(proc.Wait(std::chrono::milliseconds(500)));
		ASSERT_FALSE(proc.GetExitCode(exit_code));
		proc.Terminate(-1);
		ASSERT_FALSE(proc.Alive());
		ASSERT_TRUE(proc.GetExitCode(exit_code));
		ASSERT_EQ(exit_code, -1);
		ASSERT_NE(proc.GetPID(), VS_ChildProcess::ILLEGAL_PID);
	}

	/*TEST_F(ChildProcessTest, BasicGuiTest)
	{
		VS_ChildProcess proc("notepad");
		int exit_code = 0;

		ASSERT_TRUE(proc.Start());
		ASSERT_TRUE(proc.Alive());
		ASSERT_NE(proc.GetPID(), VS_ChildProcess::ILLEGAL_PID);
		ASSERT_TRUE(proc.AskToStop(1000));
		ASSERT_FALSE(proc.Alive());
		ASSERT_TRUE(proc.GetExitCode(exit_code));
		ASSERT_NE(proc.GetPID(), VS_ChildProcess::ILLEGAL_PID);
	}*/

	TEST_F(ChildProcessTest, OutputRedirectTest)
	{
		bool delete_tmp_file = false;
		VS_SCOPE_EXIT{
			if (delete_tmp_file)
			{
				_unlink("proc_output.log");
			}
		};
		{
			VS_ChildProcess proc(COMPLETABLE_COMMAND);

			ASSERT_TRUE(proc.RedirectOutputToFile("proc_output.log"));
			ASSERT_TRUE(proc.Start());
			delete_tmp_file = true;
			ASSERT_TRUE(proc.Wait());
		}
	}

	/*TEST_F(ChildProcessTest, CmdTest)
	{
		VS_ChildProcess proc("cmd");
		ASSERT_TRUE(proc.Start());
		ASSERT_TRUE(proc.Alive());
		ASSERT_TRUE(proc.AskToStop(1000));
		ASSERT_FALSE(proc.Alive());
	}*/

	TEST_F(ChildProcessTest, AppendTest)
	{
		bool delete_tmp_file = false;
		VS_SCOPE_EXIT{
			if (delete_tmp_file)
			{
				_unlink("proc_output_append.log");
			}
		};
		{
			VS_ChildProcess proc(COMPLETABLE_COMMAND);

			ASSERT_TRUE(proc.RedirectOutputToFile("proc_output_append.log", true));
			ASSERT_TRUE(proc.Start());
			delete_tmp_file = true;
			ASSERT_TRUE(proc.Wait());
		}
		{
			VS_ChildProcess proc(COMPLETABLE_COMMAND);

			ASSERT_TRUE(proc.RedirectOutputToFile("proc_output_append.log", true));
			ASSERT_TRUE(proc.Start());
			delete_tmp_file = true;
			ASSERT_TRUE(proc.Wait());
		}
	}
}
