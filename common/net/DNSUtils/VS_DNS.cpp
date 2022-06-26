#include "VS_DNS.h"
#include "VS_DNSResolver.h"

#ifdef _WIN32
	#ifdef DNS_EVENT_FD
		static_assert(false, "Unsupported ExtraDefinitions: DNS_EVENT_FD only for WIN");
	#endif

	#if defined(DNS_POLL) && defined(DNS_WSA_EVENT)
		static_assert(false, "Don't use at the same time ExtraDefinitions: DNS_POLL and DNS_WSA_EVENT. Can be used: empty ExtraDefinition, DNS_WSA_EVENT or DNS_POLL")
	#endif
#else
	#ifdef DNS_WSA_EVENT
		static_assert(false, "Unsupported ExtraDefinitions: DNS_WSA_EVENT only for BSD");
	#endif
#endif

#include <cassert>
#include <cstring>
#include <queue>
#include <thread>
#include <atomic>
#include <ares.h>
#include <mutex>

#ifndef _WIN32
#include <netdb.h>	// struct hostent
#include <fcntl.h>
#include <unistd.h>
#ifdef DNS_EVENT_FD
#include <sys/eventfd.h>
#endif
#endif

#include "std/nameser.h"

#include "std/cpplib/fast_mutex.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/event.h"
#include "std/cpplib/MakeShared.h"

#include "std-generic/compat/tuple.h"
#include "std-generic/compat/map.h"
#include "std-generic/compat/memory.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/VS_ClockWrapper.h"

#define DEBUG_CURRENT_MODULE VS_DM_OTHER


#ifdef _WIN32
	#ifdef DNS_POLL
		#define poll WSAPoll
	#endif
#endif

namespace net
{
	namespace dns
	{
		namespace
		{
			Resolver *g_resolver = nullptr;

			class signal final
			{
			public:
				typedef
#ifdef _WIN32
#ifdef DNS_WSA_EVENT
					WSAEVENT
#else
					SOCKET
#endif
#else
					int
#endif
							native_handle_type;
			signal()
			{
				init();
			}

			~signal()
			{
				close();
			}

			signal(const signal& other) = delete;
			signal& operator=(const signal& other) = delete;

			signal(signal&& other) noexcept;
			signal& operator=(signal&& other) noexcept;

			void set();
			void reset();

			native_handle_type fd() const noexcept
			{
#if defined(DNS_EVENT_FD) || defined(DNS_WSA_EVENT)
				return m_fd;
#else
				return m_fds[0];
#endif
			}

			private:
				void init();
				void close() noexcept;
			private:
				native_handle_type
#if defined(DNS_EVENT_FD) || defined(DNS_WSA_EVENT)
									m_fd;
#else
									m_fds[2]; // NOTES: fd[0] is set up for reading, fd[1] is set up for writing
#endif
			};

#ifdef _WIN32
#ifdef DNS_WSA_EVENT
			signal::signal(signal&& other) noexcept
			{
				m_fd = other.m_fd;
				other.m_fd = WSA_INVALID_EVENT;
			}

			signal& signal::operator=(signal&& other) noexcept
			{
				if (this == &other)
					return *this;

				m_fd = other.m_fd;
				other.m_fd = WSA_INVALID_EVENT;

				return *this;
			}

			void signal::init()
			{
			 	if((m_fd = ::WSACreateEvent()) == WSA_INVALID_EVENT)
					throw std::system_error(::GetLastError(), std::system_category(), "WSACreateEvent");
			}

			void signal::close() noexcept
			{
				::WSACloseEvent(m_fd);
			}

			void signal::set()
			{
				if(::WSASetEvent(m_fd) == 0)
					throw std::system_error(::GetLastError(), std::system_category(), "WSASetEvent");
			}

			void signal::reset()
			{
				if (::WSAResetEvent(m_fd) == 0)
					throw std::system_error(::GetLastError(), std::system_category(), "WSAResetEvent");
			}
#else
			signal::signal(signal&& other) noexcept
			{
				::memcpy(m_fds, other.m_fds, sizeof(m_fds));
				static_assert(INVALID_SOCKET == (~0), "!");
				::memset(other.m_fds, INVALID_SOCKET, sizeof(other.m_fds));
			}

			signal& signal::operator=(signal&& other) noexcept
			{
				if (this == &other)
					return *this;

				::memcpy(m_fds, other.m_fds, sizeof(m_fds));
				static_assert(INVALID_SOCKET == (~0), "!");
				::memset(other.m_fds, INVALID_SOCKET, sizeof(other.m_fds));

				return *this;
			}

			void signal::init()
			{
				struct sockaddr_in inaddr;
				struct sockaddr addr;

				SOCKET lst = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (lst == INVALID_SOCKET)
					throw std::system_error(::WSAGetLastError(), std::system_category(), "socket");

				memset(&inaddr, 0, sizeof(inaddr));
				memset(&addr, 0, sizeof(addr));
				inaddr.sin_family = AF_INET;
				inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
				inaddr.sin_port = 0;
				int yes = 1;

				if (::setsockopt(lst, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)) == SOCKET_ERROR)
				{
					::closesocket(lst);
					throw std::system_error(::WSAGetLastError(), std::system_category(), "setsockopt");
				}

				if (::bind(lst, (struct sockaddr *)&inaddr, sizeof(inaddr)) == SOCKET_ERROR)
				{
					::closesocket(lst);
					throw std::system_error(::WSAGetLastError(), std::system_category(), "bind");
				}

				if (::listen(lst, 1) == SOCKET_ERROR)
				{
					::closesocket(lst);
					throw std::system_error(::WSAGetLastError(), std::system_category(), "listen");
				}

				int len = sizeof(inaddr);
				if (::getsockname(lst, &addr, &len) == SOCKET_ERROR)
				{
					::closesocket(lst);
					throw std::system_error(::WSAGetLastError(), std::system_category(), "getsockname");
				}

				m_fds[0] = ::socket(AF_INET, SOCK_STREAM, 0);

				if (m_fds[0] == INVALID_SOCKET)
				{
					::closesocket(lst);
					throw std::system_error(::WSAGetLastError(), std::system_category(), "socket");
				}


				if (::connect(m_fds[0], &addr, len) == SOCKET_ERROR)
				{
					::closesocket(lst);
					throw std::system_error(::WSAGetLastError(), std::system_category(), "connect");
				}


				m_fds[1] = ::accept(lst, NULL, NULL);
				if (m_fds[1] == INVALID_SOCKET)
				{
					::closesocket(lst);
					throw std::system_error(::WSAGetLastError(), std::system_category(), "accept");
				}

				::closesocket(lst);

				u_long flags = 1;
				if (::ioctlsocket(m_fds[0], FIONBIO, &flags) != NO_ERROR) //set non blocking
					throw std::system_error(::WSAGetLastError(), std::system_category(), "ioctlsocket");

				flags = 1;
				if (::ioctlsocket(m_fds[1], FIONBIO, &flags) != NO_ERROR) //set non blocking
					throw std::system_error(::WSAGetLastError(), std::system_category(), "ioctlsocket");
			}

			void signal::close() noexcept
			{
				::closesocket(m_fds[0]);
				::closesocket(m_fds[1]);
			}

			void signal::set()
			{
				char buf;
				for(;;)
				{
					const int len = ::send(m_fds[1], (const char *)&buf, sizeof(buf), 0);

					if (len >= 0) //full system buffer or success
						return;

					if (len == SOCKET_ERROR)
					{
						const int error = ::WSAGetLastError();
						switch (error)
						{
						case WSAEWOULDBLOCK: return;
						case WSAEINTR:
							// an interrupt (signal) has been catched
							// should be ingore in most cases
							break;
						default:
							throw std::system_error(error, std::system_category(), "send");  // socket has been closed or shutdown for send
						}
					}
				}
			}

			void signal::reset()
			{
				char buf[64];
				for(;;)
				{
					const int len = ::recv(m_fds[0], buf, sizeof(buf), 0);

					if(len == 0)
						throw std::runtime_error("Peer disconnected");  // socket has been closed or shutdown for send

					if (len == SOCKET_ERROR)
					{
						const int error = ::WSAGetLastError();
						switch (error)
						{
						case WSAEWOULDBLOCK: return;        // Socket is O_NONBLOCK and there is no data available
						case WSAEINTR:
							// an interrupt (signal) has been catched
							// should be ingore in most cases
							break;
						default:
							throw std::system_error(error, std::system_category(), "recv");  // socket has been closed or shutdown for send
						}
					}
				}
			}
#endif

#elif defined(DNS_EVENT_FD)
			signal::signal(signal&& other) noexcept
			{
				m_fd = other.m_fd;
				other.m_fd = -1;
			}

			signal& signal::operator=(signal&& other) noexcept
			{
				if (this == &other)
					return *this;

				m_fd = other.m_fd;
				other.m_fd = -1;

				return *this;
			}

			void signal::init()
			{
				unsigned int val = 0;
				m_fd = ::eventfd(val, O_NONBLOCK);
				if (m_fd < 0)
					throw std::system_error(errno, std::system_category(), "eventfd");
			}

			void signal::close() noexcept
			{
				::close(m_fd);
			}

			void signal::set()
			{
				uint64_t value = 1;
				for(;;)
				{
					const int len = ::write(m_fd, &value, sizeof(value));

					if (len >= 0)
						return;

					if(len < 0)
					{
						switch (errno)
						{
						case EAGAIN: return;
						case EINTR: break; //interrupt (signal)
						default:
							throw std::system_error(errno, std::system_category(), "write");
						}
					}
				}
			}

			void signal::reset()
			{
				uint64_t value;
				for(;;)
				{
					const int len = ::read(m_fd, &value, sizeof(value));

					if(len == 0)
						throw std::runtime_error("Peer disconnected");

					if (len < 0)
					{
						switch (errno)
						{
						case EAGAIN: return; //no data yet
						case EINTR: break; //interrupt (signal)
						default:
							throw std::system_error(errno, std::system_category(), "read");
						}
					}
				}
			}
#else
			signal::signal(signal&& other) noexcept
			{
				::memcpy(m_fds, other.m_fds, sizeof(m_fds));
				::memset(other.m_fds, -1, sizeof(other.m_fds));
			}

			signal& signal::operator=(signal&& other) noexcept
			{
				if (this == &other)
					return *this;

				::memcpy(m_fds, other.m_fds, sizeof(m_fds));
				::memset(other.m_fds, -1, sizeof(other.m_fds));

				return *this;
			}

			void signal::init()
			{
				if (::pipe(m_fds) < 0)
					throw std::system_error(errno, std::system_category(), "pipe");

				if (::fcntl(m_fds[0], F_SETFL, O_NONBLOCK) < 0)
					throw std::system_error(errno, std::system_category(), "fcntl");

				if (::fcntl(m_fds[1], F_SETFL, O_NONBLOCK) < 0)
					throw std::system_error(errno, std::system_category(), "fcntl");
			}

			void signal::close() noexcept
			{
				::close(m_fds[0]);
				::close(m_fds[1]);
			}

			void signal::set()
			{
				char buf;
				for(;;)
				{
					const int len = ::write(m_fds[1], &buf, sizeof(buf));

					if (len == 0)
						return;

					if(len < 0)
					{
						switch (errno)
						{
						case EAGAIN: return;
						case EINTR: break; //interrupt (signal)
						default:
							throw std::system_error(errno, std::system_category(), "read");  // pipe closed
						}
					}
				}
			}

			void signal::reset()
			{
				char buf[64];
				for(;;)
				{
					const int len = ::read(m_fds[0], buf, sizeof(buf));

					if(len == 0)
						throw std::runtime_error("Peer disconnected");

					if (len < 0)
					{
						switch (errno)
						{
						case EAGAIN: return; //no data yet
						case EINTR: break; //interrupt (signal)
						default:
							throw std::system_error(errno, std::system_category(), "read");  // pipe closed
						}
					}
				}
			}
#endif

			struct dns_category_impl final : public boost::system::error_category
			{
				const char* name() const BOOST_SYSTEM_NOEXCEPT override;
				std::string message(int ev) const BOOST_SYSTEM_NOEXCEPT override;
			};


#if defined(ARES_VERSION)

			//////////////////////////////////////////////////////////////////////////

			const char* dns_category_impl::name() const BOOST_SYSTEM_NOEXCEPT
			{
				return "c-ares";
			}

			std::string dns_category_impl::message(int ev) const BOOST_SYSTEM_NOEXCEPT
			{
				return ::ares_strerror(ev);
			}

			//////////////////////////////////////////////////////////////////////////

			inline void library_init()
			{
#ifdef _WIN32
				WSADATA wsa_data;
				::WSAStartup(MAKEWORD(2, 2), &wsa_data);	// it's safe to call WSAStartup many times in one process
#endif

#ifdef CARES_HAVE_ARES_LIBRARY_INIT
				boost::system::error_code ec = static_cast<errors::error_code_enum>(::ares_library_init(ARES_LIB_INIT_ALL));
				if (ec)
				{
					throw boost::system::system_error(ec);
				}
#endif
			}

			inline void library_cleanup() noexcept
			{
#ifdef CARES_HAVE_ARES_LIBRARY_INIT
				::ares_library_cleanup();
#endif

#ifdef _WIN32
				::WSACleanup();
#endif
			}

#if defined(_WIN32)
#ifndef NDEBUG
			inline bool winsock_initialized() noexcept
			{
				SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // no matter tcp or udp, just verify opened or not
				if (s == INVALID_SOCKET && ::WSAGetLastError() == WSANOTINITIALISED)
				{
					return false;
				}
				::closesocket(s);
				return true;
			}
#endif
#endif

			class AresResolverImpl : public Resolver
			{
				struct Cache final
				{
					template<class K, class V>
					using map_t = std::map<K, V>;

					template<typename T>
					struct Record final
					{
						T cache;
						boost::system::error_code ec;
						std::chrono::steady_clock::time_point expiry;
					};

					Cache()
						: m_cachePositiveTTL(0)
						, m_cacheNegativeTTL(0)
						, m_cacheMaxRecords(0)
					{}

					template<typename Query, class Reply>
					bool FindRecord(const map_t<Query, Record<Reply>> &record, const Query& query, Reply &res, boost::system::error_code &ec, bool insensitiveTLL) const
					{
						auto it = record.find(query);

						if (it != record.cend() && (insensitiveTLL || it->second.expiry > clock.now()))
						{
							res = it->second.cache;
							ec = it->second.ec;
							return true;
						}

						ec = errors::error_code_enum::not_found;
						return false;
					}

					template<class K, class Data, class Query, class Reply>
					void SaveRecord(map_t<K, Record<Data>> &record, Query &&query, Reply &&reply, boost::system::error_code ec)
					{
						if (m_cacheMaxRecords == 0)
							return;

						std::chrono::seconds ttl;

						if (ec) //if error
						{
							//https://docs.rs/c-ares/1.0.0/c_ares/enum.Error.html
							if (ec != errors::no_data && ec != errors::not_found && ec != errors::of)
								return;

							ttl = m_cacheNegativeTTL;
						}
						else
							ttl = m_cachePositiveTTL;

						if (ttl.count() <= 0)
							return;

						auto it = record.find(query);
						if (it != record.cend())
						{
							it->second.cache = std::forward<Reply>(reply);
							it->second.ec = ec;
							it->second.expiry = clock.now() + ttl;
						}
						else
						{
							if (record.size() + 1 <= m_cacheMaxRecords //cache upper limit for records
								|| RemoveExpiryRecord(record))
							{
								record.emplace(std::forward<Query>(query), Record<Data>{ std::forward<Reply>(reply), ec, clock.now() + ttl });
							}
						}
					}

					void PrintRecords(std::ostream &stream)
					{
						stream << "-------- A RECORDS --------" << std::endl;
						PrintRecordTraits(aRecords, stream);

						stream << "-------- AAAA RECORDS --------" << std::endl;
						PrintRecordTraits(aaaaRecords, stream);

						stream << "-------- SRV RECORDS --------" << std::endl;
						PrintRecordTraits(srvRecords, stream);

						stream << "-------- NAPTR RECORDS --------" << std::endl;
						PrintRecordTraits(naptrRecords, stream);

						stream << "-------- PTR RECORDS --------" << std::endl;
						PrintRecordTraits(ptrRecords, stream);
					}

					void SetSettings(std::chrono::seconds cachePositiveTTL, std::chrono::seconds cacheNegativeTTL, std::size_t cacheMaxRecords)
					{
						if (cacheMaxRecords > aRecords.max_size() /*for different types*/)
							cacheMaxRecords = aRecords.max_size();

						m_cachePositiveTTL = cachePositiveTTL;
						m_cacheNegativeTTL = cacheNegativeTTL;
						m_cacheMaxRecords = cacheMaxRecords;
					}

					map_t<std::string, Record<hostent_reply>> aRecords;
					map_t<std::string, Record<hostent_reply>> aaaaRecords;
					map_t<std::string, Record<srv_reply_list>> srvRecords;
					map_t<std::string, Record<naptr_reply_list>> naptrRecords;
					map_t<net::address, Record<hostent_reply>> ptrRecords;

					steady_clock_wrapper clock;

					private:
						std::chrono::seconds m_cachePositiveTTL;
						std::chrono::seconds m_cacheNegativeTTL;
						std::size_t m_cacheMaxRecords;

						template<typename C>
						bool RemoveExpiryRecord(C &cache) noexcept
						{
							for (auto it = cache.cbegin(), end = cache.cend(); it != end; ++it)
							{
								if (clock.now() >= it->second.expiry)
								{
									cache.erase(it);
									return true;
								}
							}
							return false;
						}

						template<typename T>
						void PrintRecordTraits(T &records, std::ostream &stream)
						{
							for (const auto &record : records)
							{
								const auto ttl = std::chrono::duration_cast<std::chrono::seconds>(record.second.expiry - clock.now()).count();
								PrintRecord(stream, record.second.cache) << " TTL=" << (ttl > 0 ? ttl : 0) << std::endl;
							}
						}

						template<typename T>
						static std::ostream &PrintRecord(std::ostream &stream, const list_t<T> &list)
						{
							stream << "{";
							for (auto &item : list)
							{
								stream << " " << item;
							}
							stream << " }";

							return stream;
						}

						template<typename T>
						static std::ostream &PrintRecord(std::ostream &stream, const T &obj)
						{
							return stream << obj;
						}
				};

				struct Task
				{
					virtual ~Task() = default;
					virtual void Execute(std::shared_ptr<Task> &&task) = 0;
				};

				template<typename Query, typename Callback, typename ...Types>
				class TaskQueryWrap : public Task
				{
					typedef TaskQueryWrap<Query, Callback, Types...> self_t;

				protected:
					TaskQueryWrap(::ares_channel channel, int flags, Query &&query, Callback &&callback)
						: m_channel(channel)
						, m_query(std::move(query))
						, m_callback(std::move(callback))
						, m_flags(flags)
					{
					}

					virtual bool Send(const Query &query, ::ares_channel channel, std::tuple<Types...> &args) = 0;

					virtual void Parse(int /*status*/, bool /*ipv6*/, ::hostent */*host*/, std::tuple<Types...> &/*args*/)
					{
						assert(false);
					}

					virtual void Parse(int /*status*/, unsigned char */*buf*/, int /*len*/, std::tuple<Types...> &/*args*/)
					{
						assert(false);
					}

					virtual void OnComplete(Query &&/*query*/, std::tuple<Types...> &&/*args*/)
					{
						//stub
					}

					void CallOnComplete()
					{
						if(m_callback)
						{
							vs::apply(m_callback, m_args);
						}

						OnComplete(std::move(m_query), std::move(m_args));
						m_self.reset();
					}

					int Flags() const noexcept
					{
						return m_flags;
					}

					void *GetArg() noexcept
					{
						return static_cast<void *>(this);
					}

					static hostent_reply ParseHostent(const hostent *host) noexcept
					{
						assert(!!host);

						hostent_reply res;

						if (host->h_name)
							res.name = host->h_name;
						if (host->h_aliases) {
							char **palias = host->h_aliases;
							while (*palias != nullptr) {
								res.aliases.emplace_back(*palias);
								palias++;
							}
						}

						if (host->h_addr_list && (host->h_length == 4 || host->h_length == 16))
						{
							char **paddr = host->h_addr_list;
							while (*paddr != nullptr)
							{
								net::address addr;
								if (host->h_addrtype == AF_INET)
									addr = AddressFromBytes<net::address_v4>(*paddr);
								else
									addr = AddressFromBytes<net::address_v6>(*paddr);

								if (!addr.is_unspecified())
									res.addrs.push_back(std::move(addr));
								paddr++;
							}
						}

						return res;
					}

					template<bool Ipv6, typename... Args>
					static void Processing(void *arg, int status, int /*timeout*/, Args ...args)
					{
						ProcessingImpl(arg, status, Ipv6, args...);
					}

					template<typename... Args>
					static void Processing(void *arg, int status, int /*timeout*/, Args ...args)
					{
						ProcessingImpl(arg, status, args...);
					}

					static errors::error_code_enum ErrorCode(int e) noexcept
					{
						return static_cast<errors::error_code_enum>(e);
					}

				private:
					template<typename ...Args>
					static void ProcessingImpl(void *arg, Args ...args)
					{
						assert(!!arg);

						self_t *query = static_cast<self_t *>(arg);

						assert(!!query);

						query->Parse(args..., query->m_args);
					}

					template<typename Addr>
					static Addr AddressFromBytes(const char *bytes) noexcept
					{
						using bytes_type = typename Addr::bytes_type;

						bytes_type addr_bytes{};
						::memcpy(addr_bytes.data(), bytes, addr_bytes.size());

						return Addr{ addr_bytes };
					}

					void Execute(std::shared_ptr<Task> &&task) override final
					{
						m_self = std::move(task);
						if (!Send(m_query, m_channel, m_args))
						{
							assert(!!m_callback);
							vs::apply(m_callback, std::move(m_args));
							m_self.reset();
						}
					}
				private:
					std::shared_ptr<Task> m_self;
					::ares_channel m_channel;
					Query m_query;
					Callback m_callback;
					std::tuple<Types...> m_args;
					int m_flags;
				};

			public:
				AresResolverImpl()
					: m_running(false)
				{}

				~AresResolverImpl()
				{
					AresResolverImpl::Destroy();
				}

				void Init(const init_options &options) override
				{
					if (m_running.exchange(true, std::memory_order_acq_rel))
					{
						return;
					}

					library_init();

					struct ares_options opts{};

					const int optmask = ARES_OPT_FLAGS | ARES_OPT_LOOKUPS | ARES_OPT_ROTATE | ARES_OPT_TRIES | ARES_OPT_TIMEOUTMS;

					opts.flags = ARES_FLAG_NOCHECKRESP | (options.primaryFlag ? ARES_FLAG_PRIMARY : 0);

					char lookup[] = "fb";
					opts.lookups = lookup; //whether to check files or DNS or both

					opts.tries = options.tries;
					opts.timeout = options.timeout.count() >= INT_MAX ? INT_MAX : static_cast<int>(options.timeout.count());

					auto ec = ::ares_init_options(&m_channel, &opts, optmask);
					if (ec != ARES_SUCCESS)
						throw boost::system::system_error(static_cast<errors::error_code_enum>(ec));

					if (options.servers != NULL)
					{
						ec = ares_set_servers_ports_csv(m_channel, options.servers);
						if (ec != ARES_SUCCESS)
							throw boost::system::system_error(static_cast<errors::error_code_enum>(ec));
					}

					m_cache.SetSettings(options.cachePositiveTTL, options.cacheNegativeTTL, options.cacheMaxRecords);

					m_workerThread = std::thread(&AresResolverImpl::MainLoop, this, m_channel);
				}

				void Destroy() override
				{
					if (!m_running.exchange(false, std::memory_order_acq_rel))
					{
						return;
					}

					m_signal.set();

					assert(m_workerThread.joinable());
					m_workerThread.join();

					//::ares_cancel(channel); //TODO: Do I need to cancel all requests if ::ares_destroy call ARES_EDESTRUCTION to callbacks ?!
					::ares_destroy(m_channel);

					library_cleanup();
				}

				void Make_A_Lookup(std::string hostname, a_callback callback, int flags) override
				{
					if (!(flags & flags_t::real_inet || flags & flags_t::use_cache))
						return;

					//if you don't send a DNS query and use_cache - must be valid callback
					if (!(flags & flags_t::real_inet) && flags & flags_t::use_cache && !callback)
						return;

					if (hostname.empty())
					{
						if (callback)
							callback({}, errors::error_code_enum::bad_query);

						return;
					}

					class TaskQueryAWrap : public TaskQueryWrap<std::string, a_callback, hostent_reply, boost::system::error_code>
					{
					protected:
						TaskQueryAWrap(Cache &cache, int flags, ::ares_channel channel, std::string &&query, a_callback &&callback)
							: TaskQueryWrap<std::string, a_callback, hostent_reply, boost::system::error_code>(channel, flags, std::move(query), std::move(callback))
							, m_cache(cache)
						{
						}

						void Parse(int status, bool /*ipv6*/, ::hostent *host, std::tuple<hostent_reply, boost::system::error_code> &args) override
						{
							VS_SCOPE_EXIT{ CallOnComplete(); };

							auto &reply = std::get<0>(args);
							auto &ec = std::get<1>(args);

							ec = ErrorCode(status);

							if (ec) //if error
								return;

							reply = ParseHostent(host);
						}

						bool Send(const std::string &query, ::ares_channel channel, std::tuple<hostent_reply, boost::system::error_code> &args) override
						{
							if (Flags() & flags_t::use_cache)
							{
								if (m_cache.FindRecord(m_cache.aRecords, query, std::get<0>(args), std::get<1>(args), Flags() & flags_t::insensitive_ttl))
									return false;
							}

							if (Flags() & flags_t::real_inet)
							{
								::ares_gethostbyname(channel, query.c_str(), AF_INET, &Processing<false, struct hostent *>, GetArg());
								return true;
							}

							return false;
						}

						void OnComplete(std::string &&query, std::tuple<hostent_reply, boost::system::error_code> &&args) override
						{
							m_cache.SaveRecord(m_cache.aRecords, std::move(query), std::move(std::get<0>(args)), std::get<1>(args));
						}

					private:
						Cache &m_cache;
					};

					AddTask(vs::MakeShared<TaskQueryAWrap>(m_cache, flags, m_channel, std::move(hostname), std::move(callback)));
				}

				void Make_AAAA_Lookup(std::string hostname, aaaa_callback callback, int flags) override
				{
					if (!(flags & flags_t::real_inet || flags & flags_t::use_cache))
						return;

					//if you don't send a DNS query and use_cache - must be valid callback
					if (!(flags & flags_t::real_inet) && flags & flags_t::use_cache && !callback)
						return;

					if (hostname.empty())
					{
						if (callback)
							callback({}, errors::error_code_enum::bad_query);

						return;
					}

					class TaskQueryAAAAWrap : public TaskQueryWrap<std::string, aaaa_callback, hostent_reply, boost::system::error_code>
					{
					protected:
						TaskQueryAAAAWrap(Cache &cache, int flags, ::ares_channel channel, std::string &&query, aaaa_callback &&callback)
							: TaskQueryWrap<std::string, aaaa_callback, hostent_reply, boost::system::error_code>(channel, flags, std::move(query), std::move(callback))
							, m_cache(cache)
						{
						}

						void Parse(int status, bool /*ipv6*/, ::hostent *host, std::tuple<hostent_reply, boost::system::error_code> &args) override
						{
							VS_SCOPE_EXIT{ CallOnComplete(); };

							auto &reply = std::get<0>(args);
							auto &ec = std::get<1>(args);

							ec = ErrorCode(status);

							if (ec) //if error
								return;

							reply = ParseHostent(host);
						}

						bool Send(const std::string &query, ::ares_channel channel, std::tuple<hostent_reply, boost::system::error_code> &args) override
						{
							if (Flags() & flags_t::use_cache)
							{
								if (m_cache.FindRecord(m_cache.aaaaRecords, query, std::get<0>(args), std::get<1>(args), Flags() & flags_t::insensitive_ttl))
									return false;
							}

							if (Flags() & flags_t::real_inet)
							{
								::ares_gethostbyname(channel, query.c_str(), AF_INET6, &Processing<true, struct hostent *>, GetArg());
								return true;
							}

							return false;
						}

						void OnComplete(std::string &&query, std::tuple<hostent_reply, boost::system::error_code> &&args) override
						{
							m_cache.SaveRecord(m_cache.aaaaRecords, std::move(query), std::move(std::get<0>(args)), std::get<1>(args));
						}

					private:
						Cache &m_cache;
					};

					AddTask(vs::MakeShared<TaskQueryAAAAWrap>(m_cache, flags, m_channel, std::move(hostname), std::move(callback)));
				}

				void Make_A_AAAA_Lookup(std::string hostname, a_aaaa_callback callback, int flags) override
				{
					if (!(flags & flags_t::real_inet || flags & flags_t::use_cache))
						return;

					//if you don't send a DNS query and use_cache - must be valid callback
					if (!(flags & flags_t::real_inet) && flags & flags_t::use_cache && !callback)
						return;

					if (hostname.empty())
					{
						if (callback)
							callback({ {}, errors::error_code_enum::bad_query }, { {}, errors::error_code_enum::bad_query });

						return;
					}

					class TaskQueryA_AAAAWrap : public TaskQueryWrap<std::string, a_aaaa_callback, a_hostent_reply, aaaa_hostent_reply>
					{
					protected:
						TaskQueryA_AAAAWrap(Cache &cache, int flags, ::ares_channel channel, std::string &&query, a_aaaa_callback &&callback)
							: TaskQueryWrap<std::string, a_aaaa_callback, a_hostent_reply, aaaa_hostent_reply>(channel, flags, std::move(query), std::move(callback))
							, m_cache(cache)
							, m_remaining(0)
						{
						}

						void Parse(int status, bool ipv6, ::hostent *host, std::tuple<a_hostent_reply, aaaa_hostent_reply> &args) override
						{
							VS_SCOPE_EXIT
							{
								if(--m_remaining <= 0)
								{
									CallOnComplete();
								}
							};

							auto &reply = ipv6 ? std::get<1>(args) : std::get<0>(args);

							reply.ec = ErrorCode(status);

							if (reply.ec) //if error
								return;

							reply.host = ParseHostent(host);
						}

						bool Send(const std::string &query, ::ares_channel channel, std::tuple<a_hostent_reply, aaaa_hostent_reply> &args) override
						{
							if (Flags() & flags_t::use_cache)
							{
								auto &a_data = std::get<0>(args);
								auto &aaaa_data = std::get<1>(args);

								if (m_cache.FindRecord(m_cache.aRecords, query, a_data.host, a_data.ec, Flags() & flags_t::insensitive_ttl)
									| m_cache.FindRecord(m_cache.aaaaRecords, query, aaaa_data.host, aaaa_data.ec, Flags() & flags_t::insensitive_ttl))
									return false;
							}

							if (Flags() & flags_t::real_inet)
							{
								m_remaining = 2;

								::ares_gethostbyname(channel, query.c_str(), AF_INET, &Processing<false, struct hostent *>, GetArg());
								::ares_gethostbyname(channel, query.c_str(), AF_INET6, &Processing<true, struct hostent *>, GetArg());
								return true;
							}

							return false;
						}

						void OnComplete(std::string &&query, std::tuple<a_hostent_reply, aaaa_hostent_reply> &&args) override
						{
							auto &a_data = std::get<0>(args);
							auto &aaaa_data = std::get<1>(args);

							m_cache.SaveRecord(m_cache.aRecords, query, std::move(a_data.host), a_data.ec);
							m_cache.SaveRecord(m_cache.aaaaRecords, std::move(query), std::move(aaaa_data.host), aaaa_data.ec);
						}

					private:
						Cache &m_cache;
						int m_remaining;
					};

					AddTask(vs::MakeShared<TaskQueryA_AAAAWrap>(m_cache, flags, m_channel, std::move(hostname), std::move(callback)));
				}

				void Make_SRV_Lookup(std::string hostname, srv_callback callback, int flags) override
				{
					if (!(flags & flags_t::real_inet || flags & flags_t::use_cache))
						return;

					//if you don't send a DNS query and use_cache - must be valid callback
					if (!(flags & flags_t::real_inet) && flags & flags_t::use_cache && !callback)
						return;

					if (hostname.empty())
					{
						if (callback)
							callback({}, errors::error_code_enum::bad_query);

						return;
					}

					class TaskQuerySRVWrap : public TaskQueryWrap<std::string, srv_callback, srv_reply_list, boost::system::error_code>
					{
					protected:
						TaskQuerySRVWrap(Cache &cache, int flags, ::ares_channel channel, std::string &&query, srv_callback &&callback)
							: TaskQueryWrap<std::string, srv_callback, srv_reply_list, boost::system::error_code>(channel, flags, std::move(query), std::move(callback))
							, m_cache(cache)
						{
						}

						void Parse(int status, unsigned char *buf, int len, std::tuple<srv_reply_list, boost::system::error_code> &args) override
						{
							VS_SCOPE_EXIT{ CallOnComplete(); };

							auto &reply = std::get<0>(args);
							auto &ec = std::get<1>(args);

							ec = ErrorCode(status);
							if (ec) //if error
								return;

							struct ares_srv_reply *srv_res;
							ec = ErrorCode(::ares_parse_srv_reply(buf, len, &srv_res));

							if (ec) //if error
								return;

							auto p_reply = srv_res;

							while (p_reply)
							{
								reply.push_back({ static_cast<const char *>(p_reply->host), p_reply->priority, p_reply->weight, p_reply->port });
								p_reply = p_reply->next;
							}

							::ares_free_data(srv_res);
						}

						bool Send(const std::string &query, ::ares_channel channel, std::tuple<srv_reply_list, boost::system::error_code> &args) override
						{
							if (Flags() & flags_t::use_cache)
							{
								if (m_cache.FindRecord(m_cache.srvRecords, query, std::get<0>(args), std::get<1>(args), Flags() &flags_t::insensitive_ttl))
									return false;
							}

							if (Flags() & flags_t::real_inet)
							{
								::ares_query(channel, query.c_str(), C_IN, T_SRV, &Processing<unsigned char * /*abuf*/, int /*alen*/>, GetArg());
								return true;
							}

							return false;
						}

						void OnComplete(std::string &&query, std::tuple<srv_reply_list, boost::system::error_code> &&args) override
						{
							m_cache.SaveRecord(m_cache.srvRecords, std::move(query), std::move(std::get<0>(args)), std::get<1>(args));
						}

					private:
						Cache &m_cache;
					};

					AddTask(vs::MakeShared<TaskQuerySRVWrap>(m_cache, flags, m_channel, std::move(hostname), std::move(callback)));

				}

				void Make_PTR_Lookup(net::address addr, ptr_callback callback, int flags) override
				{
					if (!(flags & flags_t::real_inet || flags & flags_t::use_cache))
						return;

					//if you don't send a DNS query and use_cache - must be valid callback
					if (!(flags & flags_t::real_inet) && flags & flags_t::use_cache && !callback)
						return;

					if(addr.is_unspecified())
					{
						if(callback)
							callback({}, errors::error_code_enum::bad_query);

						return;
					}

					class TaskQueryPTRWrap : public TaskQueryWrap<net::address, ptr_callback, hostent_reply, boost::system::error_code>
					{
					protected:
						TaskQueryPTRWrap(Cache &cache, int flags, ::ares_channel channel, address &&query, ptr_callback &&callback)
							: TaskQueryWrap<net::address, ptr_callback, hostent_reply, boost::system::error_code>(channel, flags, std::move(query), std::move(callback))
							, m_cache(cache)
						{
						}

						void Parse(int status, bool /*ipv6*/, ::hostent *host, std::tuple<hostent_reply, boost::system::error_code> &args) override
						{
							VS_SCOPE_EXIT{ CallOnComplete(); };

							auto &reply = std::get<0>(args);
							auto &ec = std::get<1>(args);

							ec = ErrorCode(status);
							if (ec) // if error
								return;

							reply = ParseHostent(host);
						}

						bool Send(const address &query, ::ares_channel channel, std::tuple<hostent_reply, boost::system::error_code> &args) override
						{
							if (Flags() & flags_t::use_cache)
							{
								auto &reply = std::get<0>(args);
								auto &ec = std::get<1>(args);

								if (m_cache.FindRecord(m_cache.ptrRecords, query, reply, ec, Flags() & flags_t::insensitive_ttl))
								{
									return false;
								}

								assert(ec == errors::error_code_enum::not_found);

								if(Flags() & flags_t::force_search)
								{
									const auto &record = query.is_v4() ? m_cache.aRecords : m_cache.aaaaRecords;
									for(const auto &item : record)
									{
										if(!item.second.ec && (Flags() & flags_t::insensitive_ttl || item.second.expiry > m_cache.clock.now()))
										{
											for(const auto &addr : item.second.cache.addrs)
											{
												if(query == addr)
												{
													reply = item.second.cache;
													ec.clear();

													return false;
												}
											}
										}
									}
								}
							}

							if (Flags() & flags_t::real_inet)
							{
								const bool is_ipv6 = query.is_v6();
								if (is_ipv6)
								{
									auto addr = query.to_v6();
									auto bytes_addr = addr.to_bytes();
									::ares_gethostbyaddr(channel, &bytes_addr.front(), bytes_addr.size(), AF_INET6, Processing<true, struct ::hostent *>, GetArg());
								}
								else
								{
									auto addr = query.to_v4();
									auto bytes_addr = addr.to_bytes();
									::ares_gethostbyaddr(channel, &bytes_addr.front(), bytes_addr.size(), AF_INET, Processing<false, struct ::hostent *>, GetArg());
								}

								return true;
							}

							return false;
						}

						void OnComplete(net::address &&query, std::tuple<hostent_reply, boost::system::error_code> &&args) override
						{
							m_cache.SaveRecord(m_cache.ptrRecords, std::move(query), std::move(std::get<0>(args)), std::get<1>(args));
						}

					private:
						Cache &m_cache;
					};

					AddTask(vs::MakeShared<TaskQueryPTRWrap>(m_cache, flags, m_channel, std::move(addr), std::move(callback)));

				}

				void Make_NAPTR_Lookup(std::string query, naptr_type type, naptr_callback callback, int flags) override
				{
					if (!(flags & flags_t::real_inet || flags & flags_t::use_cache))
						return;

					//if you don't send a DNS query and use_cache - must be valid callback
					if (!(flags & flags_t::real_inet) && flags & flags_t::use_cache && !callback)
						return;

					if (query.empty())
					{
						if (callback)
							callback({}, errors::error_code_enum::bad_query);

						return;
					}

					std::string query_arpa;
					if(type == naptr_type::domain)
					{
						query_arpa = std::move(query);
					}
					else
					{
						switch (type)
						{
						case naptr_type::urn:
							query_arpa = std::move(query) + "urn.arpa";
							break;
						case naptr_type::uri:
							query_arpa = std::move(query) + "uri.arpa";
							break;
						case naptr_type::e164:
						{
							const auto end_it = query.front() == '+' ? query.crend() - 1 : query.crend();
							const char e164_arpa[] = "e164.arpa";
							query_arpa.reserve(query.size() - static_cast<int>(!!(end_it != query.crend())) + (sizeof(e164_arpa) - 1));
							for(auto it = query.crbegin(); it != end_it; ++it)
							{
								query_arpa += *it;
								query_arpa += '.';
							}
							query_arpa.append(e164_arpa, sizeof(e164_arpa) - 1);
						}
						break;
						default:
							assert(false);
							break;
						}
					}

					class TaskQueryNAPTRWrap : public TaskQueryWrap<std::string, naptr_callback, naptr_reply_list, boost::system::error_code>
					{
					protected:
						TaskQueryNAPTRWrap(Cache &cache, int flags, ::ares_channel channel, std::string &&query, naptr_callback &&callback)
							: TaskQueryWrap<std::string, naptr_callback, naptr_reply_list, boost::system::error_code>(channel, flags, std::move(query), std::move(callback))
							, m_cache(cache)
						{
						}

						void Parse(int status, unsigned char *buf, int len, std::tuple<naptr_reply_list, boost::system::error_code> &args) override
						{
							VS_SCOPE_EXIT{ CallOnComplete(); };

							auto &reply = std::get<0>(args);
							auto &ec = std::get<1>(args);

							ec = ErrorCode(status);
							if (ec) //if error
								return;

							struct ares_naptr_reply *naptr_res;
							ec = ErrorCode(::ares_parse_naptr_reply(buf, len, &naptr_res));

							if (ec) // if error
								return;

							auto p_reply = naptr_res;
							while (p_reply)
							{
								reply.push_back({ (const char *)p_reply->flags, (const char *)p_reply->service, (const char *)p_reply->regexp, static_cast<const char *>(p_reply->replacement), p_reply->order, p_reply->preference });
								p_reply = p_reply->next;
							}

							::ares_free_data(naptr_res);
						}

						bool Send(const std::string &query, ::ares_channel channel, std::tuple<naptr_reply_list, boost::system::error_code> &args) override
						{
							if (Flags() & flags_t::use_cache)
							{
								if (m_cache.FindRecord(m_cache.naptrRecords, query, std::get<0>(args), std::get<1>(args), Flags() & flags_t::insensitive_ttl))
									return false;
							}

							if (Flags() & flags_t::real_inet)
							{
								::ares_query(channel, query.c_str(), C_IN, T_NAPTR, &Processing<unsigned char * /*abuf*/, int /*alen*/>, GetArg());
								return true;
							}

							return false;
						}

						void OnComplete(std::string &&query, std::tuple<naptr_reply_list, boost::system::error_code> &&args) override
						{
							m_cache.SaveRecord(m_cache.naptrRecords, std::move(query), std::move(std::get<0>(args)), std::get<1>(args));
						}

					private:
						Cache &m_cache;
					};

					AddTask(vs::MakeShared<TaskQueryNAPTRWrap>( m_cache, flags, m_channel, std::move(query_arpa), std::move(callback)));
				}

				bool RunningInThisThread() const noexcept override
				{
					return m_workerThread.get_id() == std::this_thread::get_id();
				}

				void PrintCache(std::ostream &stream) override
				{
					vs::event ev {false};

					class TaskPrintCache final : public Task
					{
					public:
						TaskPrintCache(Cache &cache, std::ostream &stream, vs::event &ev)
							: m_cache(cache)
							, stream(stream)
							, m_ev(ev)
						{
						}
						void Execute(std::shared_ptr<Task> &&/*task*/) override
						{
							m_cache.PrintRecords(stream);
							m_ev.set();
						}

					private:
						Cache &m_cache;
						std::ostream &stream;
						vs::event &m_ev;
					};

					AddTask(std::make_shared<TaskPrintCache>(m_cache, stream, ev));
					ev.wait();
				}

				std::future<void> CancelAllRequests() override
				{
					std::promise<void> p;

					class TaskCancel final : public Task
					{
					public:
						TaskCancel(::ares_channel channel, std::promise<void> &&p)
							: m_channel(channel)
							, m_p(std::move(p))
						{
						}

						void Execute(std::shared_ptr<Task> &&/*task*/) override
						{
							::ares_cancel(m_channel);
							m_p.set_value();
						}

					private:
						::ares_channel m_channel;
						std::promise<void> m_p;
					};

					auto future = p.get_future();

					AddTask(std::make_shared<TaskCancel>(m_channel, std::move(p)));

					return future;
				}

			private:
				inline void AddTask(std::shared_ptr<Task> &&task)
				{
					{
						std::lock_guard<decltype(m_queueMonitor)> _{ m_queueMonitor };
						m_taskQueue.push(std::move(task));
						if (m_taskQueue.size() > 1)
							return;
					}

					m_signal.set();
				}

				void SignalProcessing()
				{
					m_signal.reset();

					bool read_done = true;
					do
					{
						{
							std::unique_lock<decltype(m_queueMonitor)> lock{ m_queueMonitor };
							if (!m_taskQueue.empty())
							{
								auto task = std::move(m_taskQueue.front());
								m_taskQueue.pop();

								read_done = m_taskQueue.empty();

								lock.unlock();
#ifdef _WIN32
								assert(winsock_initialized());
#endif
								auto task_p = task.get();
								task_p->Execute(std::move(task));
							}
						}

					} while (!read_done);
				}

				//NOTES: don't catch exceptions
				void MainLoop(::ares_channel channel)
				{
					vs::SetThreadName("DNSResolver");

					int nfds;
					int count;
					struct timeval *tvp, tv;

					const auto signal_fd = m_signal.fd();
					assert(signal_fd);

#if defined(_WIN32) && defined(DNS_WSA_EVENT)

					ares_socket_t socks[ARES_GETSOCK_MAXNUM];

					signal ares_signal;
					const auto ares_signal_fd = ares_signal.fd();

					HANDLE events[2]{ ares_signal_fd, signal_fd };

#elif !defined(DNS_POLL)
					fd_set read_fds, write_fds;

#else
					ares_socket_t socks[ARES_GETSOCK_MAXNUM];
					struct pollfd pfd[ARES_GETSOCK_MAXNUM + 1 /*signal_fd*/];

#endif
					while (m_running.load(std::memory_order_acquire))
					{

#if defined(_WIN32) && defined(DNS_WSA_EVENT)

						int bitmask = ::ares_getsock(channel, socks, ARES_GETSOCK_MAXNUM);

						count = 0;

						if (bitmask)
						{
							for (; count < ARES_GETSOCK_MAXNUM; count++)
							{
								const auto event_mask = (ARES_GETSOCK_READABLE(bitmask, count) ? FD_READ : 0) | (ARES_GETSOCK_WRITABLE(bitmask, count) ? FD_WRITE : 0);

								if (!event_mask)
									break;

								if (::WSAEventSelect(socks[count], ares_signal_fd, event_mask | FD_CLOSE) == SOCKET_ERROR)
									throw std::system_error(::WSAGetLastError(), std::system_category(), "WSAEventSelect");
							}
						}

						tvp = ::ares_timeout(channel, NULL, &tv);
						const DWORD timeout_ms = (tvp == NULL ? INFINITE : (tvp->tv_sec * 1000) + (tvp->tv_usec / 1000));

						static_assert((sizeof(events) / sizeof(events[0])) == 2);

						nfds = ::WSAWaitForMultipleEvents(sizeof(events) / sizeof(events[0]), events, FALSE, timeout_ms, FALSE);

						if (nfds == WSA_WAIT_FAILED)
							throw std::system_error(::WSAGetLastError(), std::system_category(), "WSAWaitForMultipleEvents");

						if (nfds == WSA_WAIT_TIMEOUT) //timeout
						{
							::ares_process_fd(channel, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
						}
						else {

							if (WSA_WAIT_EVENT_0 == nfds) //ares event
							{
								while ((count--) > 0)
								{
									WSANETWORKEVENTS network_events;
									if (::WSAEnumNetworkEvents(socks[count], ares_signal_fd, &network_events) == SOCKET_ERROR)
									{
										const auto err = ::WSAGetLastError();
										if (err != WSAEINPROGRESS)
										{
											if(err != WSAENOTSOCK)
												throw std::system_error(err, std::system_category(), "WSAEnumNetworkEvents");

											::ares_process_fd(channel, socks[count], ARES_SOCKET_BAD); //NOTE: socket closed
										}
									}
									else
									{
										::ares_process_fd(channel,
											(network_events.lNetworkEvents & (FD_READ | FD_CLOSE)) ? socks[count] : ARES_SOCKET_BAD,
											(network_events.lNetworkEvents & (FD_WRITE | FD_CLOSE)) ? socks[count] : ARES_SOCKET_BAD);
									}
								}

								ares_signal.reset();
							}
							else if (WSA_WAIT_EVENT_0 + 1 == nfds)
							{
								SignalProcessing();
							}
						}

#elif !defined(DNS_POLL)
						FD_ZERO(&read_fds);
						FD_ZERO(&write_fds);

						nfds = ::ares_fds(channel, &read_fds, &write_fds);
						tvp = ::ares_timeout(channel, NULL, &tv);

						FD_SET(signal_fd, &read_fds);

#ifndef _WIN32 // ::select ignores first arg (WIN)
						if (signal_fd >= nfds)
						{
							nfds = signal_fd + 1;
						}
#endif
						count = ::select(nfds, &read_fds, &write_fds, NULL, tvp);
						if (count < 0)
						{
							const int err =
#ifdef _WIN32
								::WSAGetLastError();
							if (err != WSAEINTR && err != WSAEWOULDBLOCK)

#else
								errno;
							if (errno != EINTR && errno != EAGAIN)
#endif
								throw std::system_error(err, std::system_category(), "select");

						}
						else
						{
							if (count == 0) //timeout
							{
								::ares_process_fd(channel, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
							}
							else
							{
								const bool is_signal_ev = FD_ISSET(signal_fd, &read_fds);

								if(!(count == 1 && is_signal_ev))
									::ares_process(channel, &read_fds, &write_fds);

								if(is_signal_ev)
									SignalProcessing();
							}
						}

#else
						int bitmask = ares_getsock(channel, socks, ARES_GETSOCK_MAXNUM);

						count = 0;

						if (bitmask)
						{
							for (; count < ARES_GETSOCK_MAXNUM; count++)
							{
								pfd[count].events = 0;
								pfd[count].revents = 0;

								const auto event_mask = (ARES_GETSOCK_READABLE(bitmask, count) ? POLLRDNORM | POLLIN : 0) | (ARES_GETSOCK_WRITABLE(bitmask, count) ? POLLWRNORM | POLLOUT : 0);

								if (!event_mask)
									break;

								pfd[count].fd = socks[count];
								pfd[count].events |= event_mask;

							}
						}

						pfd[count].fd = signal_fd;
						pfd[count++].events |= POLLRDNORM | POLLIN;


						tvp = ::ares_timeout(channel, NULL, &tv);
						const int timeout_ms = (tvp == NULL ? -1 /*INFINITE*/ : (tvp->tv_sec * 1000) + (tvp->tv_usec / 1000));

						assert(count);
						nfds = poll(pfd, count, timeout_ms);

						if (nfds == -1)
						{
							const int err =
#ifdef _WIN32
								::WSAGetLastError();
							if (err != WSAEINTR && err != WSAEWOULDBLOCK)

#else
								errno;
							if (errno != EINTR && errno != EAGAIN)
#endif
								throw std::system_error(err, std::system_category(), "poll");
						}
						else
						{
							if (nfds == 0) //timeout
							{
								::ares_process_fd(channel, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
							}
							else
							{
								const bool is_signal_ev = pfd[count - 1].revents & (POLLRDNORM | POLLIN);

								if (!(nfds == 1 && is_signal_ev))
								{
									/* move through the descriptors and ask for processing on them */
									while (count-- > 1)
										::ares_process_fd(channel,
											pfd[count - 1].revents & (POLLRDNORM | POLLIN) ?
											pfd[count - 1].fd : ARES_SOCKET_BAD,
											pfd[count - 1].revents & (POLLWRNORM | POLLOUT) ?
											pfd[count - 1].fd : ARES_SOCKET_BAD);
								}

								if (is_signal_ev)
									SignalProcessing();
							}
						}
#endif
					}

					SignalProcessing(); //posted existing queries before destroy
				}


				template<typename Callback>
				inline bool ValidationFlags(int flags, const Callback &cb) noexcept
				{
					return !(flags & flags_t::use_cache && !(flags & flags_t::real_inet) && !cb);
				}

			private:
				typedef vs::fast_mutex mutex_t;

				template<class T>
				using queue_t = std::queue<T>;

				ares_channel m_channel;
				queue_t<std::shared_ptr<Task>> m_taskQueue;
				mutex_t m_queueMonitor;
				std::thread m_workerThread;
				Cache m_cache;
				std::atomic_bool m_running;
				signal m_signal;
			};
#endif
		}

		namespace errors
		{
#if defined(ARES_VERSION)
			static_assert(ARES_SUCCESS == error_code_enum::no_error, "not eq ARES_SUCCESS with error_code_enum::no_error");

			static_assert(ARES_ENODATA == error_code_enum::no_data, "not eq ARES_ENODATA with error_code_enum::no_data");
			static_assert(ARES_EFORMERR == error_code_enum::form_error, "not eq ARES_EFORMERR with error_code_enum::form_error");
			static_assert(ARES_ESERVFAIL == error_code_enum::server_fail, "not eq ARES_ESERVFAIL with error_code_enum::server_fail");
			static_assert(ARES_ENOTFOUND == error_code_enum::not_found, "not eq ARES_ENOTFOUND with error_code_enum::not_found");
			static_assert(ARES_ENOTIMP == error_code_enum::not_imp, "not eq ARES_ENOTIMP with error_code_enum::not_imp");
			static_assert(ARES_EREFUSED == error_code_enum::refused, "not eq ARES_EREFUSED with error_code_enum::refused");

			static_assert(ARES_EBADQUERY == error_code_enum::bad_query, "not eq ARES_EBADQUERY with error_code_enum::bad_query");
			static_assert(ARES_EBADNAME == error_code_enum::bad_name, "not eq ARES_EBADNAME with error_code_enum::bad_name");
			static_assert(ARES_EBADFAMILY == error_code_enum::bad_family, "not eq ARES_EBADFAMILY with error_code_enum::bad_family");
			static_assert(ARES_EBADRESP == error_code_enum::bad_response, "not eq ARES_EBADRESP with error_code_enum::bad_response");
			static_assert(ARES_ECONNREFUSED == error_code_enum::conn_refused, "not eq ARES_ECONNREFUSED with error_code_enum::conn_refused");
			static_assert(ARES_ETIMEOUT == error_code_enum::time_out, "not eq ARES_ETIMEOUT with error_code_enum::time_out");
			static_assert(ARES_EOF == error_code_enum::of, "not eq ARES_EOF with error_code_enum::of");
			static_assert(ARES_EFILE == error_code_enum::file, "not eq ARES_EFILE with error_code_enum::file");
			static_assert(ARES_ENOMEM == error_code_enum::no_mem, "not eq ARES_ENOMEM with error_code_enum::no_mem");
			static_assert(ARES_EDESTRUCTION == error_code_enum::destruction, "not eq ARES_EDESTRUCTION with error_code_enum::destruction");
			static_assert(ARES_EBADSTR == error_code_enum::bad_str, "not eq ARES_EBADSTR with error_code_enum::bad_str");

#if (ARES_VERSION >= 0x010700)
			static_assert(ARES_ENOTINITIALIZED == error_code_enum::not_initialized, "not eq ARES_ENOTINITIALIZED with error_code_enum::not_initialized");
			static_assert(ARES_ELOADIPHLPAPI == error_code_enum::load_iphlpapi, "not eq ARES_ELOADIPHLPAPI with error_code_enum::load_iphlpapi");
			static_assert(ARES_EADDRGETNETWORKPARAMS == error_code_enum::addr_get_network_params, "not eq ARES_EADDRGETNETWORKPARAMS with error_code_enum::addr_get_network_params");
			static_assert(ARES_ECANCELLED == error_code_enum::cancelled, "not eq ARES_ECANCELLED with error_code_enum::cancelled");
#endif

#else
			static_assert(false); // not found c-ares lib
#endif
		}

		boost::system::error_category &get_error_category()
		{
			static dns_category_impl impl{};
			return impl;
		}

		init_options default_init_options() noexcept
		{
			init_options op{};
			op.timeout = std::chrono::seconds(2); //2 sec.
			op.cachePositiveTTL = std::chrono::minutes(30);
			op.cacheNegativeTTL = decltype(op.cacheNegativeTTL){ 0};
			op.cacheMaxRecords = 150;
			op.tries = 2;
			op.primaryFlag = false;
			return op;
		}

#ifdef ARES_VERSION
		void init(const init_options &options)
		{
			static AresResolverImpl impl{};
			impl.Init(options);
			g_resolver = &impl;
		}
		void destroy() {
			assert(g_resolver != nullptr);
			g_resolver->Destroy();
		}
#endif

		void set_resolver_TEST(Resolver *resolver) noexcept
		{
			g_resolver = resolver;
		}

		Resolver* get_resolver_TEST() noexcept
		{
			return g_resolver;
		}

		std::ostream& operator<<(std::ostream& os, const hostent_reply& host)
		{
			os << "{ ";
			os << "'" << host.name << "' "
				<< "aliases=[";
			for(std::size_t i = 0; i <  host.aliases.size(); i++)
			{
				if(i > 0)
					os << ", ";

				os << host.aliases[i];
			}
			os << "] addrs=[";

			for(std::size_t i = 0; i < host.addrs.size(); i++)
			{
				if (i > 0)
					os << ", ";
				os << host.addrs[i];
			}

			os << "] }";
			return os;
		}

		//////////////////////////////////////////////////////////////////////////

		std::ostream& operator<<(std::ostream& os, const srv_reply& obj)
		{
			return os << "[host='" << obj.host << "' priority=" << obj.priority << " weight=" << obj.weight << " port=" << obj.port << "]";
		}


		std::ostream& operator<<(std::ostream& os, const naptr_reply& obj)
		{
			return os << "[flag=" << obj.flag << " service=" << obj.service << " regexp=" << obj.regexp << " replacement=" <<
				obj.replacement << " order=" << obj.order << " preference=" << obj.preference << "]";
		}

		//////////////////////////////////////////////////////////////////////////


		void async_make_a_lookup(std::string hostname, a_callback callback, int flags)
		{
			g_resolver->Make_A_Lookup(std::move(hostname), std::move(callback), flags);
		}

		void async_make_aaaa_lookup(std::string hostname, aaaa_callback callback, int flags)
		{
			g_resolver->Make_AAAA_Lookup(std::move(hostname), std::move(callback), flags);
		}

		void async_make_srv_lookup(std::string service, srv_callback callback, int flags)
		{
			g_resolver->Make_SRV_Lookup(std::move(service), std::move(callback), flags);
		}

		void async_make_ptr_lookup(const net::address& addr, ptr_callback callback, int flags)
		{
			g_resolver->Make_PTR_Lookup(addr, std::move(callback), flags);
		}

		bool running_in_this_thread()
		{
			return g_resolver->RunningInThisThread();
		}

		void async_make_naptr_lookup(std::string query, naptr_type type, naptr_callback callback, int flags)
		{
			g_resolver->Make_NAPTR_Lookup(std::move(query), type, std::move(callback), flags);
		}

		std::future<void> cancel_all_requests()
		{
			return g_resolver->CancelAllRequests();
		}

		void async_make_a_aaaa_lookup(std::string hostname, a_aaaa_callback callback, int flags)
		{
			g_resolver->Make_A_AAAA_Lookup(std::move(hostname) , std::move(callback), flags);
		}

		void print_dns_cache(std::ostream &stream)
		{
			g_resolver->PrintCache(stream);
		}

	} //namespace dns
} //namespace net


#ifdef _WIN32
	#ifdef DNS_POLL
		#undef poll
	#endif
#endif