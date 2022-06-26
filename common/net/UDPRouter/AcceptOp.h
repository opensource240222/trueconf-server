#pragma once

#include "net/UDPRouter/fwd.h"
#include "net/UDPRouter/OpBase.h"
#include "std-generic/cpplib/move_handler.h"

#include <boost/asio/io_service.hpp>

#include <type_traits>
#include "std-generic/compat/memory.h"

namespace net { namespace ur {

template <class Protocol>
class AcceptOp;
template <class Protocol>
using AcceptOpPtr = std::unique_ptr<AcceptOp<Protocol>>;

template <class Protocol>
class AcceptOp : public detail::OpBase
{
public:
	AcceptOpPtr<Protocol> next;

	template <class AcceptHandler>
	static AcceptOpPtr<Protocol> Make(AcceptHandler&& handler);

	friend bool Complete(AcceptOpPtr<Protocol> op, boost::asio::io_service& ios, const boost::system::error_code& ec)
	{
		auto op_raw = op.get();
		return op_raw->Complete(std::move(op), ios, ec, Connection<Protocol>());
	}
	friend bool Complete(AcceptOpPtr<Protocol> op, boost::asio::io_service& ios, Connection<Protocol> connection)
	{
		auto op_raw = op.get();
		return op_raw->Complete(std::move(op), ios, boost::system::error_code(), std::move(connection));
	}

private:
	virtual bool Complete(AcceptOpPtr<Protocol> self, boost::asio::io_service& ios, const boost::system::error_code& ec, Connection<Protocol> connection) = 0;
};

namespace detail {

template <class Protocol, class AcceptHandler>
class AcceptOpImpl final : public AcceptOp<Protocol>
{
	using state = OpBase::state;

public:
	template <class AH>
	explicit AcceptOpImpl(AH&& handler)
		: m_handler(std::forward<AH>(handler))
	{
	}

private:
	bool Complete(AcceptOpPtr<Protocol> self, boost::asio::io_service& ios, const boost::system::error_code& ec, Connection<Protocol> connection) override
	{
		switch (this->m_state.exchange(state::completed, std::memory_order_acq_rel))
		{
		case state::waiting:
			ios.post(vs::move_handler([this, self = std::move(self), ec, connection = std::move(connection)]() mutable {
				m_handler(ec, std::move(connection));
			}));
			return true;
		case state::canceled:
			ios.post(vs::move_handler([this, self = std::move(self)]() {
				m_handler(boost::asio::error::operation_aborted, Connection<Protocol>());
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
	AcceptHandler m_handler;
};

}

template <class Protocol>
template <class AcceptHandler>
AcceptOpPtr<Protocol> AcceptOp<Protocol>::Make(AcceptHandler&& handler)
{
	return vs::make_unique<detail::AcceptOpImpl<Protocol, typename std::decay<AcceptHandler>::type>>(
		std::forward<AcceptHandler>(handler)
	);
}

}}
