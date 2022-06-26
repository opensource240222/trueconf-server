
#include <cstdarg>
#include <atomic>
#include <memory>
#include <thread>
#include "VS_EchoDebugger.h"

VS_EchoDebugger::VS_EchoDebugger()
{
	m_Enabled = false;
}

VS_EchoDebugger::~VS_EchoDebugger()
{
	Clear();
}

VS_EchoDebugger& VS_EchoDebugger::GetInstance()
{
	static std::unique_ptr<VS_EchoDebugger> impl;

	// MSVC doesn't support (yet) thread safe local static initialization, so we do it manually
	static std::atomic<bool> init_done(false);
	static std::atomic_flag init_started = ATOMIC_FLAG_INIT;
	if (!init_done.load(std::memory_order_acquire))
	{
		if (!init_started.test_and_set())
		{
			impl.reset(new VS_EchoDebugger);
			init_done.store(true, std::memory_order_release);
		}
		else
			while (!init_done.load(std::memory_order_acquire))
				std::this_thread::yield();
	}

	return *impl;
}

void VS_EchoDebugger::Clear()
{
	for (std::ofstream& f : m_DataFiles)
	{
		if (f.is_open())
			f.close();
	}

	m_Enabled = false;
}

bool VS_EchoDebugger::Init(const std::string& prefix)
{
	Clear();

	const char defaultExt[] = ".pcm";

	const char* filesNames[VS_EchoDebugger::DT_NUM] =
	{
		"echo",
		"nearend",
		"farend",
		"echo_log"
	};

	m_DataFiles[DT_ECHO].open(prefix + "_" + filesNames[DT_ECHO] + defaultExt, std::ios::binary);
	m_DataFiles[DT_NEAREND].open(prefix + "_" + filesNames[DT_NEAREND] + defaultExt, std::ios::binary);
	m_DataFiles[DT_FAREND].open(prefix + "_" + filesNames[DT_FAREND] + defaultExt, std::ios::binary);
	m_DataFiles[DT_LOG].open(prefix + "_" + filesNames[DT_LOG] + ".log");

	for (std::ofstream& f : m_DataFiles)
	{
		if (!f.is_open())
			return false;
	}

	m_Enabled = true;

	return true;
}

void VS_EchoDebugger::WriteData(DataType dt, const void* buf, size_t size)
{
	if (m_Enabled)
		m_DataFiles[dt].write((const char*)buf, size);
}

std::ofstream & VS_EchoDebugger::Log()
{
	return m_DataFiles[DT_LOG];
}
