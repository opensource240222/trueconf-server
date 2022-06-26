#pragma once

#include <future>
#include <ostream>
#include <vector>

#include "net/Port.h"
#include "net/Address.h"

struct hostent;

namespace net
{
	namespace dns
	{
		struct Resolver;

		void set_resolver_TEST(Resolver *) noexcept;
		Resolver *get_resolver_TEST() noexcept;

		boost::system::error_category &get_error_category();

		namespace errors
		{
			enum error_code_enum
			{
				no_error = 0,

				/*Server error codes*/
				no_data = 1,
				form_error = 2,
				server_fail = 3,
				not_found = 4,
				not_imp = 5,
				refused = 6,

				/* Locally generated error codes */
				bad_query = 7,
				bad_name = 8,
				bad_family = 9,
				bad_response = 10,
				conn_refused = 11,
				time_out = 12,
				of = 13,
				file = 14,
				no_mem = 15,
				destruction = 16,
				bad_str = 17,

				/* Uninitialized library error code */
				not_initialized = 21,

				load_iphlpapi = 22,
				addr_get_network_params = 23,

				cancelled = 24
			};

			inline boost::system::error_code make_error_code(error_code_enum e)
			{
				return boost::system::error_code(static_cast<int>(e), get_error_category());
			}

			inline boost::system::error_condition make_error_condition(error_code_enum e)
			{
				return boost::system::error_condition(static_cast<int>(e), get_error_category());
			}

		} //namespace errors

		enum flags_t
		{
			real_inet = 1,
			use_cache = 2,
			force_search = 4,
			insensitive_ttl = 8
		};

		template<typename T>
		using list_t = std::vector<T>;

		struct naptr_reply final
		{
			std::string flag;
			std::string service;
			std::string regexp;
			std::string replacement;
			unsigned short order;
			unsigned short preference;

			friend std::ostream& operator<<(std::ostream& os, const naptr_reply& obj);

		};

		struct srv_reply final
		{
			std::string host;
			unsigned short priority;
			unsigned short weight;
			net::port port;

			friend std::ostream& operator<<(std::ostream& os, const srv_reply& obj);
		};

		struct hostent_reply final
		{
			list_t<std::string> aliases;
			list_t<net::address> addrs;
			std::string name;

			friend std::ostream& operator<<(std::ostream& os, const hostent_reply& host);
		};

		struct a_hostent_reply final
		{
			hostent_reply host;
			boost::system::error_code ec;
		};

		typedef a_hostent_reply aaaa_hostent_reply;

		struct init_options final
		{
			const char *servers; //string format: host[:port][,host[:port]]...
			std::chrono::milliseconds timeout; //timeout for one query
			std::chrono::seconds cachePositiveTTL;
			std::chrono::seconds cacheNegativeTTL;
			std::size_t cacheMaxRecords; // max records for one caches bucket
			int tries; //retransmit count query by `timeout` for one query
			bool primaryFlag; //only query the first server in the list of servers to query.
		};

		init_options default_init_options() noexcept;

		void init(const init_options &options = default_init_options());
		void destroy();

		//!!!!!!!! don't hold the callback because it works in the network thread !!!!!!!!
		typedef std::function<void(hostent_reply /*host*/, boost::system::error_code /*ec*/)> a_callback;
		typedef a_callback aaaa_callback; //typedef std::function<void(host_ent /*host*/, boost::system::error_code /*ec*/)> aaaa_callback;

		typedef std::function<void(a_hostent_reply aReply, aaaa_hostent_reply aaaaReply)> a_aaaa_callback;

		typedef list_t<srv_reply> srv_reply_list;
		typedef std::function<void(srv_reply_list /*reply*/, boost::system::error_code /*ec*/)> srv_callback;

		typedef std::function<void(hostent_reply /*host*/, boost::system::error_code /*ec*/)> ptr_callback;

		typedef list_t<naptr_reply> naptr_reply_list;
		typedef std::function<void(naptr_reply_list /*reply*/, boost::system::error_code /*ec*/)> naptr_callback;

		//!!!!!!!! don't hold the callback because it works in the network thread !!!!!!!!
		void async_make_a_lookup(std::string hostname, a_callback callback, int flags = flags_t::real_inet | flags_t::use_cache);

		//!!!!!!!! don't hold the callback because it works in the network thread !!!!!!!!
		void async_make_aaaa_lookup(std::string hostname, aaaa_callback callback, int flags = flags_t::real_inet | flags_t::use_cache);

		//!!!!!!!! don't hold the callback because it works in the network thread !!!!!!!!
		void async_make_srv_lookup(std::string service, srv_callback callback, int flags = flags_t::real_inet | flags_t::use_cache);

		//!!!!!!!! don't hold the callback because it works in the network thread !!!!!!!!
		void async_make_ptr_lookup(const net::address &addr, ptr_callback callback, int flags = flags_t::real_inet | flags_t::use_cache);

		void async_make_a_aaaa_lookup(std::string hostname, a_aaaa_callback callback, int flags = flags_t::real_inet | flags_t::use_cache);

		//true if the current thread is executing a network worker thread
		bool running_in_this_thread();

		/*
		query:
		-urn: cid.urn.arpa,
		-uri: http.uri.arpa,
		-url: www.foo.com,
		-e164: 2.1.2.1.5.5.5.0.7.7.1.e164.arpa for +1-770-555-1212 number*/
		enum class naptr_type
		{
			urn,
			uri,
			domain,
			e164
		};

		 //!!!!!!!! don't hold the callback because it works in the network thread !!!!!!!!
		void async_make_naptr_lookup(std::string query, naptr_type type, naptr_callback callback, int flags = flags_t::real_inet | flags_t::use_cache);

		std::future<void> cancel_all_requests();

		namespace detail
		{
			template<typename... Args>
			struct function_traits { /*stub*/ };

			template<typename A1, typename A2, typename... Args>
			struct function_traits<A1, A2, Args...>
			{
				using first_arg = A1;
				using second_arg = A2;
			};

			template <typename... Args>
			struct extract_function_args final { /*stub*/ };

			template<class R, typename... Args>
			struct extract_function_args<std::function<R(Args...)>> final : public function_traits<Args...> { };

			template<class R, typename... Args>
			struct extract_function_args<R(*)(Args...)> final : public function_traits<Args...> { };

			template<typename Ret, typename Func, typename... Args>
			std::future<Ret> make_lookup(Func func, int flags, Args& ...args)
			{
				using first_arg_cb = typename Ret::first_type;
				using second_arg_cb = typename Ret::second_type;

				//TODO:FIXME(fix creation promise in the heap which is required for std::function capture list)
				std::promise<Ret> *p = new std::promise<Ret>{};
				auto res_future = p->get_future();
				func(std::move(args)..., [p](first_arg_cb arg1, second_arg_cb arg2)
				{
					p->set_value(std::make_pair(std::move(arg1), std::move(arg2)));
					delete p;
				}, flags);

				return res_future;
			}
		} //namespace detail

		inline std::future<std::pair<detail::extract_function_args<a_callback>::first_arg, detail::extract_function_args<a_callback>::second_arg>> make_a_lookup(std::string hostname, int flags = flags_t::real_inet | flags_t::use_cache)
		{
			return detail::make_lookup<std::pair<detail::extract_function_args<a_callback>::first_arg, detail::extract_function_args<a_callback>::second_arg>, decltype(&async_make_a_lookup), decltype(hostname)>(&async_make_a_lookup, flags, hostname);
		}

		inline std::future<std::pair<detail::extract_function_args<aaaa_callback>::first_arg, detail::extract_function_args<aaaa_callback>::second_arg>> make_aaaa_lookup(std::string hostname, int flags = flags_t::real_inet | flags_t::use_cache)
		{
			return detail::make_lookup<std::pair<detail::extract_function_args<aaaa_callback>::first_arg, detail::extract_function_args<aaaa_callback>::second_arg>, decltype(&async_make_aaaa_lookup), decltype(hostname)>(&async_make_aaaa_lookup, flags, hostname);
		}

		inline std::future<std::pair<detail::extract_function_args<srv_callback>::first_arg, detail::extract_function_args<srv_callback>::second_arg>> make_srv_lookup(std::string service, int flags = flags_t::real_inet | flags_t::use_cache)
		{
			return detail::make_lookup<std::pair<detail::extract_function_args<srv_callback>::first_arg, detail::extract_function_args<srv_callback>::second_arg>, decltype(&async_make_srv_lookup), decltype(service)>(&async_make_srv_lookup, flags, service);
		}

		inline std::future<std::pair<detail::extract_function_args<ptr_callback>::first_arg, detail::extract_function_args<ptr_callback>::second_arg>> make_ptr_lookup(const net::address &addr, int flags = flags_t::real_inet | flags_t::use_cache)
		{
			return detail::make_lookup<std::pair<detail::extract_function_args<ptr_callback>::first_arg, detail::extract_function_args<ptr_callback>::second_arg>, decltype(&async_make_ptr_lookup), decltype(addr)>(&async_make_ptr_lookup, flags, addr);
		}

		inline std::future<std::pair<detail::extract_function_args<naptr_callback>::first_arg, detail::extract_function_args<naptr_callback>::second_arg>> make_naptr_lookup(std::string query, naptr_type type, int flags = flags_t::real_inet | flags_t::use_cache)
		{
			return detail::make_lookup<std::pair<detail::extract_function_args<naptr_callback>::first_arg, detail::extract_function_args<naptr_callback>::second_arg>, decltype(&async_make_naptr_lookup), decltype(query), decltype(type)>(&async_make_naptr_lookup, flags, query, type);
		}

		inline std::future<std::pair<detail::extract_function_args<a_aaaa_callback>::first_arg, detail::extract_function_args<a_aaaa_callback>::second_arg>> make_a_aaaa_lookup(std::string query, int flags = flags_t::real_inet | flags_t::use_cache)
		{
			return detail::make_lookup<std::pair<detail::extract_function_args<a_aaaa_callback>::first_arg, detail::extract_function_args<a_aaaa_callback>::second_arg>, decltype(&async_make_a_aaaa_lookup), decltype(query)>(&async_make_a_aaaa_lookup, flags, query);
		}

		void print_dns_cache(std::ostream &stream);

	} //namespace dns
} //namespace net

namespace boost {
	namespace system {
		template<>
		struct is_error_code_enum<net::dns::errors::error_code_enum> : public std::true_type { };
	} //namespace system
} //namespace boost
