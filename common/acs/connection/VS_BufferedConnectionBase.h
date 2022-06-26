#pragma once

#include <chrono>
#include <queue>

#include <boost/shared_ptr.hpp>

#include "VS_IOHandler.h"
#include "VS_Connection.h"

#include "../../std/cpplib/VS_MessageHandler.h"
#include "../../std/cpplib/VS_TimeoutHandler.h"
#include "../../std/cpplib/VS_WorkThread.h"
#include "std-generic/cpplib/SharedBuffer.h"
#include "../../std/cpplib/enable_shared_from_this_virtual_std.h"

struct VS_Overlapped;
class VS_IPPortAddress;

class VS_BufferedConnectionBase : public VS_IOHandler, public VS_TimeoutHandler, public VS_MessageHandler,
		public enable_shared_from_this_virtual<VS_BufferedConnectionBase>
{
public:
	enum class state
	{
		empty,
		connected,
		connecting,
		shutdown,
	};

	class TimeoutHandler : public VS_TimeoutHandler {
		std::weak_ptr<VS_BufferedConnectionBase> m_buff_connection;
	public:
		explicit TimeoutHandler(const std::shared_ptr<VS_BufferedConnectionBase>& buff_connection) :m_buff_connection(buff_connection) {}
		void Timeout() override {
			if (auto conn = m_buff_connection.lock()) conn->Timeout();
		}
	};

	class MessageHandler : public VS_MessageHandler {
		std::weak_ptr<VS_BufferedConnectionBase> m_buff_connection;
	public:
		explicit MessageHandler(const std::shared_ptr<VS_BufferedConnectionBase>& buff_connection) :m_buff_connection(buff_connection) {}
		void HandleMessage(const boost::shared_ptr<VS_MessageData> &message) override {
			if (auto conn = m_buff_connection.lock()) conn->HandleMessage(message);
		}
	};

	VS_BufferedConnectionBase(void);
	virtual ~VS_BufferedConnectionBase(void);
	void Init();

	state State() const { return m_state; }
	bool SetConnection(VS_Connection *c, const boost::shared_ptr<VS_WorkThread> &thread, const void* init_data = nullptr, size_t init_data_size = 0);
	bool Connect(const VS_IPPortAddress& host, const boost::shared_ptr<VS_WorkThread> &thread);
	void Shutdown(int errorCode = -1);

	void DeleteAfterWriteFinished();

	bool Send(vs::SharedBuffer&& buffer); // Queues data block for send.
	bool Send(const vs::SharedBuffer& buffer)
	{
		return Send(vs::SharedBuffer(buffer));
	}

protected:
	virtual size_t onReceive(const void* data, size_t size) = 0; // Called when new data arrives. Should return amount of consumed bytes.
	virtual void onError(unsigned err) = 0; // Called when unrecoverable error occurs, connection will be Shutdown() after that.
	virtual void onSend(); // Called when send operation is finished (or cancelled).
	virtual void onConnect(bool res);
	void Timeout() override;

	state m_state;
	std::unique_ptr<VS_Connection> m_c;
	boost::shared_ptr<VS_WorkThread> m_thread;

private:
	void Handle(const unsigned long trans, const VS_Overlapped *ov);
	void HandleError(const unsigned long err, const VS_Overlapped *ov);
	void HandleMessage(const boost::shared_ptr<VS_MessageData> &message) override;

	bool CompleteShutdown();
	void ProcessReadBuffer();
	void Read();

	std::shared_ptr<VS_BufferedConnectionBase> self; // We need to hold shared pointer to ourselves to prevent deletion while I/O operartions are active
	std::unique_ptr<unsigned char[]> m_read_buffer;
	size_t m_read_buffer_size;
	size_t m_read_data_size;
	std::queue<vs::SharedBuffer> m_out_queue;

	bool m_delete_after_write_finished;
	std::chrono::steady_clock::time_point m_connection_timeout_time;
	boost::shared_ptr<TimeoutHandler> m_timeoutHandler;
	boost::shared_ptr<MessageHandler> m_msgHandler;
};

