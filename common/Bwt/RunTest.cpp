#include "RunTest.h"
#include "ContentHandler.h"
#include "Handshake.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/MakeShared.h"

#include <chrono>
#include <exception>
#include <thread>

using boost::asio::ip::tcp;
using namespace std::chrono;

namespace bwt
{

	void TestSend(boost::asio::io_service& srv, const Endpoint& endpoint, Handshake handshake, std::shared_ptr<Intermediate> callback)
	{
		auto sender = vs::MakeShared<ContentHandler>(srv, endpoint, handshake, callback);
		sender->start_send();
		while (!callback->Done())
		{
			srv.run_one();
		}
	}

	void TestReceive(boost::asio::io_service& srv, const Endpoint& endpoint, Handshake handshake, std::shared_ptr<Intermediate> callback)
	{
		handshake.type = VS_ACS_LIB_RECEIVER;
		auto receiver = vs::MakeShared<ContentHandler>(srv, endpoint, handshake, callback);
		receiver->start_recv();
		while (!callback->Done())
		{
			srv.run_one();
		}
	}

	void TestDuplex(boost::asio::io_service& srv, const Endpoint& endpoint, Handshake handshake, std::shared_ptr<Intermediate> callback)
	{
		auto sender = vs::MakeShared<ContentHandler>(srv, endpoint, handshake, callback);
		handshake.type = VS_ACS_LIB_RECEIVER;
		auto receiver = vs::MakeShared<ContentHandler>(srv, endpoint, handshake, callback);
		sender->start_send();
		receiver->start_recv();
		while (callback->GetFinishedTestCount()<2)
		{
			srv.run_one();
		}
	}

	void TestHalfDuplex(boost::asio::io_service& srv, const Endpoint& endpoint, Handshake handshake, std::shared_ptr<Intermediate> callback)
	{
		TestSend(srv, endpoint, handshake, callback);
		TestReceive(srv, endpoint, handshake, callback);
	}

	void RunTest(const Endpoint& endpoint, std::shared_ptr<Intermediate> callback, const unsigned mode, const unsigned seconds,		const unsigned long frame_size, const unsigned long period_ms)
	{
		boost::asio::io_service srv;
		boost::asio::io_service::work work(srv);
		if (endpoint.endpoint.empty() ||
			seconds < VS_BWT_MIN_TEST_SECONDS || seconds > VS_BWT_MAX_TEST_SECONDS
			|| frame_size < VS_ACS_BWT_MIN_HEX_BUFFER_SIZE || frame_size > VS_ACS_BWT_MAX_HEX_BUFFER_SIZE
			|| period_ms > VS_ACS_BWT_MAX_PERIOD)
			throw(std::logic_error("Bad parameter"));

		Handshake handshake(VS_ACS_LIB_SENDER, seconds * 1000, frame_size, period_ms);
		switch (mode)
		{
		case VS_BWT_MODE_OUT:
			TestSend(srv, endpoint, handshake, callback);
			break;
		case VS_BWT_MODE_IN:
			TestReceive(srv, endpoint, handshake, callback);
			break;
		case VS_BWT_MODE_DUPLEX:
			TestDuplex(srv, endpoint, handshake, callback);
			break;
		case VS_BWT_MODE_HALFDUPLEX:
			TestHalfDuplex(srv, endpoint, handshake, callback);
		}
	}

	void RunTestAsync(const Endpoint& endpoint, std::shared_ptr<Intermediate> callback,
		const unsigned mode, const unsigned seconds,
		const unsigned long frame_size, const unsigned long period_ms)
	{
		std::thread([endpoint, callback = std::move(callback), mode, seconds, frame_size, period_ms]() {
			vs::SetThreadName("BwtAsync");
			RunTest(endpoint, callback, mode, seconds, frame_size, period_ms);
		}).detach();
	}

	Intermediate::Intermediate() :m_status(0),
		out_response_ms(0),
		in_response_ms(0),
		out_bytes(0),
		in_bytes(0),
		out_bps(0),
		in_bps(0),
		out_jitter_min_ms(0),
		in_jitter_min_ms(0),
		out_jitter_max_ms(0),
		in_jitter_max_ms(0),
		loc_offset_ms(0),
		m_out_tcs(0),
		m_in_tcs(0),
		m_jitter_diff(0xFFFFFFFF),
		m_jitter_ms(0),
		m_finished_test_count(0)
	{
	}

	bool Intermediate::Result(const unsigned status, const void *, const unsigned)
	{
		m_status = status;
		if (Done())
			m_finished_test_count++;
		return true;
	}

	void Intermediate::SaveToFile()
	{

	}

	bool Intermediate::Done() const
	{
		return (m_status == VS_BWT_ST_CONNECT_ERROR) ||
			(m_status == VS_BWT_ST_HANDSHAKE_ERROR) ||
			(m_status == VS_BWT_ST_CONNECT_ERROR) ||
			(m_status == VS_BWT_ST_CONNECTION_DIED) ||
			(m_status == VS_BWT_ST_CONNECTION_ERROR) ||
			(m_status == VS_BWT_ST_FINISH_TEST);
	}

	uint32_t Intermediate::GetStatus() const
	{
		return m_status;
	}

	uint32_t Intermediate::GetFinishedTestCount() const
	{
		return m_finished_test_count;
	}

	void Intermediate::UpdateStat(uint32_t type, int64_t send_time_ms, int64_t)
	{
		if (m_jitter_diff != 0xFFFFFFFF)
		{
			m_jitter_ms = send_time_ms - m_jitter_diff;
			if (type == VS_ACS_LIB_SENDER)
			{
				if (out_jitter_min_ms > m_jitter_ms)
					out_jitter_min_ms = m_jitter_ms;
				if (out_jitter_max_ms < m_jitter_ms)
					out_jitter_max_ms = m_jitter_ms;
			}
			else if (type == VS_ACS_LIB_RECEIVER)
			{
				if (in_jitter_min_ms > m_jitter_ms)
					in_jitter_min_ms = m_jitter_ms;
				if (in_jitter_max_ms < m_jitter_ms)
					in_jitter_max_ms = m_jitter_ms;
			}
			SaveToFile();
		}
		else
		{
			m_jitter_diff = send_time_ms;
		}

		if (m_out_tcs)
			out_bps = (float)((double)(out_bytes * 1000) / m_out_tcs);
		if (m_in_tcs)
			in_bps = (float)((double)(in_bytes * 1000) / m_in_tcs);

		Result(VS_BWT_ST_INTER_RESULT, this, 0);
	}

}

