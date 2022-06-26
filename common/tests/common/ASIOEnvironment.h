#pragma once

#include "std/cpplib/ASIOThreadPool.h"

#include <gtest/gtest.h>

class ASIOEnvironment : public ::testing::Environment
{
public:
	void SetUp() override
	{
		m_atp.Start();
	}

	void TearDown() override
	{
		m_atp.Stop();
	}

	boost::asio::io_service& IOService() { return m_atp.get_io_service(); }

private:
	vs::ASIOThreadPool m_atp;
};

extern ASIOEnvironment* g_asio_environment;
