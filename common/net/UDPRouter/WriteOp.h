#pragma once

#include "net/UDPRouter/OpBase.h"
#include "std-generic/cpplib/move_handler.h"

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/socket_base.hpp>

#include <type_traits>
#include "std-generic/compat/memory.h"

namespace net { namespace ur {

template <class Socket>
class WriteOp;
template <class Socket>
using WriteOpPtr = std::unique_ptr<WriteOp<Socket>>;

template <class Socket>
class WriteOp : public detail::OpBase
{
public:
	WriteOpPtr<Socket> next;

	template <class ConstBufferSequence, class WriteHandler>
	WriteOpPtr<Socket> Make(const ConstBufferSequence& buffers, const typename Socket::endpoint_typeE& destination, boost::asio::socket_base::message_flags flags, WriteHandler&& handler);

	friend bool Start(WriteOpPtr<Socket> op, Socket& socket)
	{
		auto op_raw = op.get();
		return op_raw->Start(std::move(op), socket);
	}

private:
	virtual bool Start(Socket& socket) = 0;
};

namespace detail {

template <class Socket, class ConstBufferSequence, class WriteHandler>
class WriteOpImpl final : public WriteOp
{
	using state = OpBase::state;

public:
	using endpoint_type = typename Socket::endpoint_type;

	template <class WH>
	WriteOpImpl(const ConstBufferSequence& buffers, const endpoint_type& destination, boost::asio::socket_base::message_flags flags, WH&& Handler)
		: m_buffers(buffers)
		, m_destination(destination)
		, m_flags(flags)
		, m_handler(std::forward<WH>(handler))
	{
	}

	bool Start(WriteOpPtr<Socket> self, Socket& socket) override
	{
		switch (m_state.exchange(state::completed, std::memory_order_acq_rel))
		{
		case state::waiting:
			socket.async_send_to(m_buffers, m_destination, m_flags, std::move(m_handler));
			return true;
		case state::canceled:
			socket.get_io_service().post(vs::move_handler([this, self = std::move(self)]() {
				m_handler(boost::asio::error::operation_aborted, 0);
			}));
			return true;
		case state::completed:
			// Operation is already started or completed
			return false;
		default:
			assert(false);
			return false;
		}
	}

private:
	ConstBufferSequence m_buffers;
	endpoint_type m_destination;
	boost::asio::socket_base::message_flags m_flags;
	WriteHandler m_handler;
};

}

template <class Socket>
template <class ConstBufferSequence, class WriteHandler>
WriteOpPtr<Socket> WriteOp<Socket>::Make(const ConstBufferSequence& buffers, const typename Socket::endpoint_typeE& destination, boost::asio::socket_base::message_flags flags, WriteHandler&& handler)
{
	return vs::make_unique<detail::WriteOpImpl<Socket, ConstBufferSequence, typename std::decay<WriteHandler>::type>>(
		buffers, endpoint, flags, std::forward<WriteHandler>(handler)
	);
}

}}
