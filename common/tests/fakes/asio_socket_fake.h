#pragma once

#include "std/cpplib/function.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

#include "std-generic/compat/memory.h"
#include <cstdint>
#include <mutex>
#include <queue>
#include <vector>

namespace test { namespace asio {

template <class Protocol>
class basic_socket_fake : public boost::asio::socket_base
{
public:
	// Asio types
	using protocol_type = Protocol;
	using endpoint_type = typename Protocol::endpoint;
#if defined(_WIN32)
	using native_handle_type = uintptr_t;
#else
	using native_handle_type = int;
#endif

public:
	class impl
	{
	public:
		template <class Callback>
		static void on_new(Callback&& cb)
		{
			impl::new_callbacks().emplace(std::forward<Callback>(cb));
		}

		explicit impl(boost::asio::io_service& ios)
			: ios_(ios)
			, is_open_(false)
			, opt_no_delay_(false)
			, opt_send_buffer_size_(0)
			, opt_receive_buffer_size_(0)
		{
			auto& cbs = new_callbacks();
			if (!cbs.empty())
			{
				cbs.front()(this);
				cbs.pop();
			}
		}

		class handle
		{
		public:
			handle(impl* p) : p_(p)
			{
				p_->mutex_.lock();
			}
			handle(const handle&) = delete;
			handle(handle&& x) : p_(x.p_)
			{
				x.p_ = nullptr;
			}
			handle& operator=(const handle&) = delete;
			handle& operator=(handle&& x)
			{
				if (this == &x)
					return *this;
				p_ = x.p_;
				x.p_ = nullptr;
				return *this;
			}
			~handle()
			{
				if (p_)
					p_->mutex_.unlock();
			}

			bool&                 is_open()         { return p_->is_open_; }
			endpoint_type&        remote_endpoint() { return p_->remote_endpoint_; }
			endpoint_type&        local_endpoint()  { return p_->local_endpoint_; }
			std::vector<uint8_t>& write_data()      { return p_->write_data_; }
			std::vector<uint8_t>& read_data()       { return p_->read_data_; }

			bool&     no_delay()            { return p_->opt_no_delay_; }
			unsigned& send_buffer_size()    { return p_->opt_send_buffer_size_; }
			unsigned& receive_buffer_size() { return p_->opt_receive_buffer_size_; }

			std::vector<uint8_t> get_write_data(size_t size)
			{
				size = std::min(size, p_->write_data_.size());
				const auto begin = p_->write_data_.begin();
				const auto end = begin + size;

				std::vector<uint8_t> result(begin, end);
				p_->write_data_.erase(begin, end);
				return result;
			}

			void drop_write_data(size_t size)
			{
				size = std::min(size, p_->write_data_.size());
				p_->write_data_.erase(p_->write_data_.begin(), p_->write_data_.begin() + size);
			}

			template <class InputIterator>
			handle& add_read_data(InputIterator first, InputIterator last)
			{
				p_->read_data_.insert(p_->read_data_.end(), first, last);
				return *this;
			}

			handle& add_read_data(const void* p, size_t size)
			{
				return add_read_data(static_cast<const uint8_t*>(p), static_cast<const uint8_t*>(p) + size);
			}

			template <class T, class = typename std::enable_if<std::is_standard_layout<T>::value>::type>
			handle& add_read_data(const T& x)
			{
				return add_read_data(reinterpret_cast<const uint8_t*>(&x), sizeof(T));
			}

		private:
			impl* p_;
		};
		handle get_state()
		{
			return { this };
		}

		boost::asio::io_service& ios_;

	private:
		std::mutex mutex_;
		bool is_open_;
		endpoint_type local_endpoint_;
		endpoint_type remote_endpoint_;
		std::vector<uint8_t> write_data_; // Data that was written by async_send
		std::vector<uint8_t> read_data_; // Data that can be read by async_receive

		bool opt_no_delay_;
		unsigned opt_send_buffer_size_;
		unsigned opt_receive_buffer_size_;

		// TODO: C++17: inline variable
		static std::queue<vs::function<void (impl*)>>& new_callbacks()
		{
			static std::queue<vs::function<void (impl*)>> value;
			return value;
		}

		friend class basic_socket_fake;
	};
	std::unique_ptr<impl> impl_;

public:
	// Asio interface
	// All throwing overloads are not implemented because we don't want crashes.
	// All synchronous calls are not implemented because we don't use them.

	explicit basic_socket_fake(boost::asio::io_service& io_service)
		: impl_(vs::make_unique<impl>(io_service))
	{
	}
	/* Not implemented: those ctors call open() and bind(), but throwing calls are not implemented.
	basic_socket_fake(boost::asio::io_service& io_service, const protocol_type& protocol);
	basic_socket_fake(boost::asio::io_service& io_service, const endpoint_type& endpoint);
	*/
	/* Not implemented: we don't support native handles
	basic_socket_fake(boost::asio::io_service& io_service, const protocol_type& protocol, const native_handle_type& native_socket);
	*/
	basic_socket_fake(basic_socket_fake&& other)
		: impl_(std::move(other.impl_))
	{
	}
	/* Not implemented: ???
	template <typename Protocol1>
	basic_socket_fake(basic_socket_fake<Protocol1>&& other);
	*/

	basic_socket_fake& operator=(basic_socket_fake&& other)
	{
		impl_ = std::move(other.impl_);
		return *this;
	}
	/* Not implemented: ???
	template <typename Protocol1>
	basic_socket_fake& operator=(basic_socket_fake<Protocol1>&& other);
	*/

	/* Not implemented: we don't support native handles
	boost::system::error_code assign(const protocol_type& protocol, const native_handle_type& native_socket, boost::system::error_code& ec);
	*/

	template <typename ConnectHandler>
	void async_connect(const endpoint_type& peer_endpoint, ConnectHandler&& handler)
	{
		{
			auto s = impl_->get_state();
			s.io_open() = true;
			s.remote_endpoint() = peer_endpoint;
		}
		impl_->ios_.post([handler]() mutable { handler(boost::system::error_code()); });
	}

	// TCP-only
	template <typename MutableBufferSequence, typename ReadHandler>
	void async_read_some(const MutableBufferSequence& buffers, ReadHandler&& handler)
	{
		async_receive(buffers, std::forward<ReadHandler>(handler));
	}

	template <typename MutableBufferSequence, typename ReadHandler>
	void async_receive(const MutableBufferSequence& buffers, ReadHandler&& handler)
	{
		size_t bytes = 0;
		{
			auto s = impl_->get_state();
			bytes = boost::asio::buffer_copy(buffers, boost::asio::buffer(s.read_data()));
			assert(bytes <= s.read_data().size());
			s.read_data().erase(s.read_data().begin(), s.read_data().begin() + bytes);
		}
		if (bytes > 0)
			impl_->ios_.post([handler, bytes]() mutable { handler(boost::system::error_code(), bytes); });
		// TODO: support non-immediate reads (bytes == 0)
	}

	template <typename MutableBufferSequence, typename ReadHandler>
	void async_receive(const MutableBufferSequence& buffers, socket_base::message_flags /*flags*/, ReadHandler&& handler)
	{
		async_receive(buffers, std::forward<ReadHandler>(handler));
	}

	/* Not implemented: not needed at the moment.
	// UDP-only
	template <typename MutableBufferSequence, typename ReadHandler>
	void async_receive_from(const MutableBufferSequence& buffers, endpoint_type& sender_endpoint, ReadHandler&& handler);

	// UDP-only
	template <typename MutableBufferSequence, typename ReadHandler>
	void async_receive_from(const MutableBufferSequence& buffers, endpoint_type& sender_endpoint, socket_base::message_flags flags, ReadHandler&& handler);
	*/

	template <typename ConstBufferSequence, typename WriteHandler>
	void async_send(const ConstBufferSequence& buffers, WriteHandler&& handler)
	{
		const size_t bytes = boost::asio::buffer_size(buffers);
		{
			auto s = impl_->get_state();
			const auto data_size = s.write_data().size();
			s.write_data().resize(data_size + bytes);
			const size_t copied = boost::asio::buffer_copy(boost::asio::buffer(s.write_data().data() + data_size, bytes), buffers);
			assert(copied == bytes);
			(void)copied; // Silence warning in release build
		}
		impl_->ios_.post([handler, bytes]() mutable { handler(boost::system::error_code(), bytes); });
	}

	template <typename ConstBufferSequence, typename WriteHandler>
	void async_send(const ConstBufferSequence& buffers, socket_base::message_flags /*flags*/, WriteHandler&& handler)
	{
		async_send(buffers, std::forward<WriteHandler>(handler));
	}

	/* Not implemented: not needed at the moment.
	// UDP-only
	template <typename ConstBufferSequence, typename WriteHandler>
	void async_send_to(const ConstBufferSequence& buffers, const endpoint_type& destination, WriteHandler&& handler);

	// UDP-only
	template <typename ConstBufferSequence, typename WriteHandler>
	void async_send_to(const ConstBufferSequence& buffers, const endpoint_type& destination, socket_base::message_flags flags, WriteHandler&& handler);
	*/

	// TCP-only
	template <typename ConstBufferSequence, typename WriteHandler>
	void async_write_some(const ConstBufferSequence& buffers, WriteHandler&& handler)
	{
		async_send(buffers, std::forward<WriteHandler>(handler));
	}

	bool at_mark(boost::system::error_code& /*ec*/) const
	{
		return false;
	}

	std::size_t available(boost::system::error_code& /*ec*/) const
	{
		return boost::asio::buffer_size(impl_->get_state().read_data());
	}

	boost::system::error_code bind(const endpoint_type& endpoint, boost::system::error_code& ec)
	{
		impl_->get_state().local_endpoint() = endpoint;
		return ec;
	}

	boost::system::error_code cancel(boost::system::error_code& ec)
	{
		// No-op, all async operations are completed immediately
		return ec;
	}

	boost::system::error_code close(boost::system::error_code& ec)
	{
		impl_->get_state().is_open() = false;
		return ec;
	}

	boost::asio::io_service& get_io_service()
	{
		return impl_->ios_;
	}

	// TODO: Other options
	boost::system::error_code get_option(boost::asio::ip::tcp::no_delay& option, boost::system::error_code& ec) const
	{
		option = impl_->get_state().no_delay();
		return ec;
	}
	boost::system::error_code get_option(boost::asio::ip::tcp::socket::send_buffer_size& option, boost::system::error_code& ec) const
	{
		option = impl_->get_state().send_buffer_size();
		return ec;
	}
	boost::system::error_code get_option(boost::asio::ip::tcp::socket::receive_buffer_size& option, boost::system::error_code& ec) const
	{
		option = impl_->get_state().receive_buffer_size();
		return ec;
	}

	/* Not implemented: TODO
	template <typename IoControlCommand>
	boost::system::error_code io_control(IoControlCommand & command, boost::system::error_code& ec);
	*/

	bool is_open() const
	{
		return impl_->get_state().is_open();
	}

	endpoint_type local_endpoint(boost::system::error_code& /*ec*/) const
	{
		return impl_->get_state().local_endpoint();
	}

	/* Not implemented: we don't have lowest_layer_type
	lowest_layer_type& lowest_layer();
	const lowest_layer_type& lowest_layer() const;
	*/

	native_handle_type native_handle()
	{
		// We don't support native handles but have to provide this function, so return invalid handle value to make code compile.
		return static_cast<native_handle_type>(-1);
	}

	/* Not implemented: we don't support native handles
	bool native_non_blocking() const;
	boost::system::error_code native_non_blocking(bool mode, boost::system::error_code& ec);
	*/

	bool non_blocking() const
	{
		// Return value doesn't matter because synchronous calls are not implemented.
		return false;
	}

	boost::system::error_code non_blocking(bool mode, boost::system::error_code& ec)
	{
		// No-op because synchronous calls are not implemented.
		(void)mode;
		return ec;
	}

	boost::system::error_code open(const protocol_type& /*protocol*/, boost::system::error_code& ec)
	{
		impl_->get_state().io_open() = true;
		return ec;
	}

	endpoint_type remote_endpoint(boost::system::error_code& /*ec*/) const
	{
		return impl_->get_state().remote_endpoint();
	}

	// TODO: Other options
	boost::system::error_code set_option(const boost::asio::ip::tcp::no_delay& option, boost::system::error_code& ec)
	{
		impl_->get_state().no_delay() = option.value();
		return ec;
	}
	boost::system::error_code set_option(const boost::asio::ip::tcp::socket::send_buffer_size& option, boost::system::error_code& ec)
	{
		impl_->get_state().send_buffer_size() = option.value();
		return ec;
	}
	boost::system::error_code set_option(const boost::asio::ip::tcp::socket::receive_buffer_size& option, boost::system::error_code& ec)
	{
		impl_->get_state().receive_buffer_size() = option.value();
		return ec;
	}

	boost::system::error_code shutdown(shutdown_type /*what*/, boost::system::error_code& ec)
	{
		// No-op, because ???
		return ec;
	}
};

using tcp_socket_fake = basic_socket_fake<boost::asio::ip::tcp>;
using udp_socket_fake = basic_socket_fake<boost::asio::ip::udp>;

#if !(defined(_MSC_VER) && _MSC_VER < 1900)
static_assert(!std::is_copy_constructible<tcp_socket_fake>::value, "!");
static_assert(!std::is_copy_assignable   <tcp_socket_fake>::value, "!");
static_assert( std::is_move_constructible<tcp_socket_fake>::value, "!");
static_assert( std::is_move_assignable   <tcp_socket_fake>::value, "!");

static_assert(!std::is_copy_constructible<udp_socket_fake>::value, "!");
static_assert(!std::is_copy_assignable   <udp_socket_fake>::value, "!");
static_assert( std::is_move_constructible<udp_socket_fake>::value, "!");
static_assert( std::is_move_assignable   <udp_socket_fake>::value, "!");
#endif

}}
