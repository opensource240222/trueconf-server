#pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
// win
#include <winsock2.h>
#include <process.h>

// Google Test
#include <gtest/gtest.h>

// std
#include <functional>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <random>
#include <list>
#include <vector>
#include <thread>

#include <cstdlib>
#include <cstdarg>

// boost
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>

// custom
#include "RandomData.h"

#include "../../../acs/connection/VS_ConnectionTLS.h"
#include "../../../acs/connection/VS_ConnectionTypes.h"
#include "../../../std/cpplib/VS_WorkThreadIOCP.h"


// certificates and keys
#include "cert_data.h"

namespace tls_test_common {

	// unit test class
	class TLSTests : public testing::Test {
	protected:
		virtual void SetUp() {
			/* initialize winsock */
			WSAStartup(MAKEWORD(2, 2), &wsaData);
		}

		virtual void TearDown() {
			WSACleanup();
		}
	private:
		WSADATA wsaData;
	};

	/* shared code between Server and Client */
	class Common {
	public:
		bool GetStatus() const;
	private:
		virtual bool LoadCertAndKey() = 0;
	protected:
		Common(RandomData &data, boost::shared_ptr<VS_WorkThread> &worker)
			: conn(std::make_shared<VS_ConnectionTLS>(nullptr, worker, TLS_method())), data(data), ev_done(INVALID_HANDLE_VALUE), status(false)
		{
		}

		virtual ~Common()
		{
			if (ev_done != INVALID_HANDLE_VALUE)
				CloseHandle(ev_done);
			conn->Disconnect();
			conn->Close();
		}

		virtual bool SendAll(const void *buf, const unsigned long size);
		virtual bool ReceiveAll(void *buf, const unsigned long size);
		void SetStatus(bool status);

		std::shared_ptr<VS_ConnectionTLS> conn;
		RandomData &data;
		HANDLE ev_done;
		bool status;
	};
}
