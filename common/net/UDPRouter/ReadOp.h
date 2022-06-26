#pragma once

#include "net/UDPRouter/OpBase.h"
#include "std-generic/cpplib/move_handler.h"

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>

#include <type_traits>
#include "std-generic/compat/memory.h"

namespace net { namespace ur {

class ReadOp;
using ReadOpPtr = std::unique_ptr<ReadOp>;

class ReadOp : public detail::OpBase
{
public:
	ReadOpPtr next;

	template <class MutableBufferSequence, class ReadHandler>
	static ReadOpPtr Make(const MutableBufferSequence& buffers, ReadHandler&& handler);

	friend bool Complete(ReadOpPtr op, boost::asio::io_service& ios, const boost::system::error_code& ec, const void* data = nullptr, size_t size = 0)
	{
		auto op_raw = op.get();
		return op_raw->Complete(std::move(op), ios, ec, data, size);
	}

private:
	virtual bool Complete(ReadOpPtr self, boost::asio::io_service& ios, const boost::system::error_code& ec, const void* data, size_t size) = 0;
};

namespace detail {

template <class MutableBufferSequence, class ReadHandler>
class ReadOpImpl final : public ReadOp
{
	using state = OpBase::state;

public:
	template <class RH>
	ReadOpImpl(const MutableBufferSequence& buffers, RH&& handler)
		: m_buffers(buffers)
		, m_handler(std::forward<RH>(handler))
	{
	}

private:
	bool Complete(ReadOpPtr self, boost::asio::io_service& ios, const boost::system::error_code& ec, const void* data, size_t size) override
	{
		switch (m_state.exchange(state::completed, std::memory_order_acq_rel))
		{
		case state::waiting:
		{
			const auto bytes_transferred = data
				? boost::asio::buffer_copy(m_buffers, boost::asio::const_buffer(data, size))
				: 0;
			ios.post(vs::move_handler([this, self = std::move(self), ec, bytes_transferred]() {
				m_handler(ec, bytes_transferred);
			}));
			return true;
		}
		case state::canceled:
			ios.post(vs::move_handler([this, self = std::move(self)]() {
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
	MutableBufferSequence m_buffers;
	ReadHandler m_handler;
};

}

template <class MutableBufferSequence, class ReadHandler>
ReadOpPtr ReadOp::Make(const MutableBufferSequence& buffers, ReadHandler&& handler)
{
	return vs::make_unique<detail::ReadOpImpl<MutableBufferSequence, typename std::decay<ReadHandler>::type>>(
		buffers, std::forward<ReadHandler>(handler)
	);
}

}}
