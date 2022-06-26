#include "VS_BufferedConnectionBase.h"
#include "VS_ConnectionOv.h"
#include "VS_ConnectionTCP.h"
#include "../Lib/VS_AcsLib.h"
#include "std-generic/compat/memory.h"

#include <boost/make_shared.hpp>

#include "std-generic/compat/memory.h"
#include <cassert>

#define OV_READ_COMPLETE 1
#define OV_WRITE_COMPLETE 2
#define OV_READ_COMPLETE_DESTROY 3
#define OV_WRITE_COMPLETE_DESTROY 4

#define MESSAGE_SHUTDOWN 1
#define MESSAGE_SEND 2
#define MESSAGE_INIT 3
#define MESSAGE_DELETELATER 4

static const size_t c_buffer_max_size = 1024 * 1024;
static const size_t c_read_chunk_size = 65535;
static const std::chrono::steady_clock::duration c_connection_timeout = std::chrono::seconds(10);

VS_BufferedConnectionBase::VS_BufferedConnectionBase(void)
	: m_state(state::empty)
	, m_read_buffer_size(0)
	, m_read_data_size(0)
	, m_delete_after_write_finished(false)
{
}

VS_BufferedConnectionBase::~VS_BufferedConnectionBase(void)
{
	assert(m_state == state::empty);
	assert(m_out_queue.empty());
}

void VS_BufferedConnectionBase::Init()
{
	m_timeoutHandler = boost::make_shared<TimeoutHandler>(shared_from_this());
	m_msgHandler = boost::make_shared<MessageHandler>(shared_from_this());
}

bool VS_BufferedConnectionBase::SetConnection(VS_Connection *c, const boost::shared_ptr<VS_WorkThread> &_thread, const void* init_data, size_t init_data_size)
{
	if (m_state != state::empty)
		return false;

	assert(!m_c);
	m_c.reset(c);
	m_thread = _thread;
	m_c->SetIOHandler(this);
	m_c->SetIOThread(m_thread);
	m_c->SetOvReadFields(OV_READ_COMPLETE);
	m_c->SetOvWriteFields(OV_WRITE_COMPLETE);
	m_c->SetKeepAliveMode(true, 30000, 3000);
	m_thread->RegisterTimeout(m_timeoutHandler);
	m_read_data_size = 0; // needed?
	self = shared_from_this();
	m_state = state::connected;

	if (init_data && init_data_size > 0)
	{
		m_read_buffer_size = std::max(c_read_chunk_size, init_data_size);
		m_read_buffer = vs::make_unique_default_init<unsigned char[]>(m_read_buffer_size);
		std::memcpy(m_read_buffer.get(), init_data, init_data_size);
		m_read_data_size = init_data_size;
		m_thread->Post(m_msgHandler, boost::make_shared<VS_MessageData>(MESSAGE_INIT, nullptr, 0));
	}
	else
		Read();

	return true;
}

bool VS_BufferedConnectionBase::Connect(const VS_IPPortAddress& host, const boost::shared_ptr<VS_WorkThread> &thread) {
	if (m_c) return false;

	void *event(0);
	auto c(vs::make_unique<VS_ConnectionTCP>());
	if (!c->ConnectAsynch(host, event))
		return false;

	self = shared_from_this();
	m_thread = thread;
	m_c = std::move(c);
	m_state = state::connecting;
	m_connection_timeout_time = std::chrono::steady_clock::now() + c_connection_timeout;
	m_thread->RegisterTimeout(m_timeoutHandler);
	return true;
}

void VS_BufferedConnectionBase::Shutdown(int errorCode)
{
	if (!m_thread)
	{
		assert(m_state == state::empty);
		onError(errorCode);
		return;
	}
	m_thread->Post(m_msgHandler, boost::make_shared<VS_MessageData>(MESSAGE_SHUTDOWN, (void*)errorCode, 0));
}

void VS_BufferedConnectionBase::DeleteAfterWriteFinished()
{
	if (!m_thread)
	{
		Shutdown();
		return;
	}
	m_thread->Post(m_msgHandler, boost::make_shared<VS_MessageData>(MESSAGE_DELETELATER, nullptr, 0));
}

bool VS_BufferedConnectionBase::Send(vs::SharedBuffer&& buffer)
{
	if (m_state != state::connected && m_state != state::connecting)
		return false;

	std::unique_ptr<vs::SharedBuffer> new_buffer(new vs::SharedBuffer(std::move(buffer)));
	m_thread->Post(m_msgHandler, boost::make_shared<VS_MessageData>(MESSAGE_SEND, new_buffer.release(), 0));
	return true;
}

void VS_BufferedConnectionBase::onSend()
{
}

void VS_BufferedConnectionBase::onConnect(bool res)
{
}

void VS_BufferedConnectionBase::Timeout()
{
	if (m_c == NULL || self == NULL) return;

	if (m_state == state::connecting) {
		if (static_cast<VS_ConnectionTCP*>(m_c.get())->GetConnectResult())
		{
			m_c->SetIOHandler(this);
			m_c->SetOvReadFields(OV_READ_COMPLETE);
			m_c->SetOvWriteFields(OV_WRITE_COMPLETE);
			m_c->SetKeepAliveMode(true, 30000, 3000);

			bool res = m_thread->SetHandledConnection(m_c.get());

			onConnect(res);
			if (!res) {
				m_state = state::shutdown;
				CompleteShutdown();
				return;
			}
			m_state = state::connected;

			Read();
			if (!m_out_queue.empty() && !m_c->IsWrite())
				m_c->Write(m_out_queue.front().data<const void>(), m_out_queue.front().size());
		}
		else if (std::chrono::steady_clock::now() > m_connection_timeout_time)
		{
			m_state = state::shutdown;
			onConnect(false);
			CompleteShutdown();
		}
	}
}

void VS_BufferedConnectionBase::HandleMessage(const boost::shared_ptr<VS_MessageData> &message)
{
	unsigned long type, size;
	void* data = message->GetMessPointer(type, size);
	switch ( type )
	{
	case MESSAGE_SHUTDOWN:
		if (m_state == state::empty)
			return;

		if (m_state == state::connected)
		{
			m_c->SetOvReadFields( OV_READ_COMPLETE_DESTROY );
			m_c->SetOvWriteFields( OV_WRITE_COMPLETE_DESTROY );
			m_c->Close();
		}
		m_state = state::shutdown;
		onError((int)data);
		CompleteShutdown();
		break;
	case MESSAGE_SEND:
	{
		std::unique_ptr<vs::SharedBuffer> new_buffer(static_cast<vs::SharedBuffer*>(data));
		if (m_state != state::connected && m_state != state::connecting)
			return;
		m_out_queue.emplace(std::move(*new_buffer));
		if (m_state != state::connected)
			return;
		assert(m_c);
		if (!m_c->IsWrite())
			if (!m_c->Write(m_out_queue.front().data<const void>(), m_out_queue.front().size()))
				Shutdown();
	}
		break;
	case MESSAGE_INIT:
		ProcessReadBuffer();
		Read();
		break;
	case MESSAGE_DELETELATER:
		m_delete_after_write_finished = true;
		if (!m_c) return;
		if (!m_c->IsWrite()) Shutdown();
		break;
	}
}

void VS_BufferedConnectionBase::Handle(const unsigned long trans, const VS_Overlapped *ov)
{
	if (!m_c)
		return;

	switch (ov->field1)
	{
	case OV_READ_COMPLETE:
		{
			int size = m_c->SetReadResult(trans, ov, nullptr, true);
			assert(size != -2); // Read isn't complete, but since we are accepting partial reads, this shouldn't happen
			if (size < 0)
			{
				Shutdown(ov->error);
				return;
			}
			m_read_data_size += size;
			ProcessReadBuffer();
			Read();
		}
		break;
	case OV_WRITE_COMPLETE:
		{
			int size = m_c->SetWriteResult(trans, ov);
			if (size < 0)
			{
				Shutdown(ov->error);
				return;
			}
			assert(!m_out_queue.empty());
			assert(size == m_out_queue.front().size());
			m_out_queue.pop();
			onSend();
			if (!m_out_queue.empty())
				if (!m_c->Write(m_out_queue.front().data<const void>(), m_out_queue.front().size()))
					Shutdown(ov->error);
			else if (m_delete_after_write_finished)
				Shutdown();
		}
		break;
	case OV_WRITE_COMPLETE_DESTROY:
	case OV_READ_COMPLETE_DESTROY:
		if (ov->field1 == OV_READ_COMPLETE_DESTROY)
			m_c->SetReadResult(trans, ov, nullptr, true);
		else
			m_c->SetWriteResult(trans, ov);
		CompleteShutdown();
		break;
	}
}

void VS_BufferedConnectionBase::HandleError(const unsigned long err, const VS_Overlapped *ov)
{
	if (!m_c)
		return;

	switch (ov->field1)
	{
	case OV_READ_COMPLETE:
	case OV_WRITE_COMPLETE:
		if (ov->field1 == OV_READ_COMPLETE)
			m_c->SetReadResult(0, ov, nullptr, true);
		else
			m_c->SetWriteResult(0, ov);
		Shutdown(err);
		break;
	case OV_WRITE_COMPLETE_DESTROY:
	case OV_READ_COMPLETE_DESTROY:
		Handle(0, ov);
		break;
	}
}

bool VS_BufferedConnectionBase::CompleteShutdown()
{
	if (m_c->IsRW())
		return false;

	m_c.reset();
	m_thread->UnregisterTimeout(m_timeoutHandler);
	while (!m_out_queue.empty())
	{
		m_out_queue.pop();
		onSend();
	}
	m_state = state::empty;
	self.reset();
	return true;
}

void VS_BufferedConnectionBase::ProcessReadBuffer()
{
	size_t data_size = m_read_data_size;
	auto data = m_read_buffer.get();
	while (data_size > 0)
	{
		const auto consumed = onReceive(data, data_size);
		if (consumed == 0)
			break;
		if (consumed < data_size)
		{
			data_size -= consumed;
			data += consumed;
		}
		else
			data_size = 0;
	}
	if (data_size > 0)
	{
		assert(data <= m_read_buffer.get() + m_read_data_size);
		std::memmove(m_read_buffer.get(), data, data_size);
	}
	m_read_data_size = data_size;
}

void VS_BufferedConnectionBase::Read()
{
	const size_t read_size = std::min(c_read_chunk_size, c_buffer_max_size - m_read_data_size);
	if (m_read_buffer_size < m_read_data_size + read_size)
	{
		m_read_buffer_size = std::min(c_buffer_max_size, std::max(m_read_data_size + read_size, m_read_data_size * 3 / 2));
		assert(m_read_buffer_size >= m_read_data_size + read_size);

		auto new_buffer = vs::make_unique_default_init<unsigned char[]>(m_read_buffer_size);
		if (m_read_data_size > 0)
			std::memcpy(new_buffer.get(), m_read_buffer.get(), m_read_data_size);
		m_read_buffer = std::move(new_buffer);
	}
	if (!m_c->Read(m_read_buffer.get() + m_read_data_size, read_size))
		Shutdown();
}
