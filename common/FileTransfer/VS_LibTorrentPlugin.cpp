#include "VS_LibTorrentPlugin.h"

#include <boost/pool/pool_alloc.hpp>

#include "std-generic/cpplib/hton.h"
#include "std-generic/cpplib/mem_operations.h"

#include "libtorrent/alert_manager.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/bt_peer_connection.hpp"
#include "libtorrent/extensions.hpp"
#include "libtorrent/peer_connection_handle.hpp"


#if (LIBTORRENT_VERSION_NUM < 10200) /*< 1.2.0*/ && !defined(TORRENT_NO_DEPRECATE)
/**********************************************************************************/
// Build fix for boost 1.58, 1.59
// Bug: https://svn.boost.org/trac/boost/ticket/11702

#include <boost/version.hpp>

#if BOOST_VERSION >= 105800 && BOOST_VERSION < 106000
#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )
namespace boost {
	namespace _bi {
		template<class A> struct list_add_cref<std::auto_ptr<A> > {
			typedef std::auto_ptr<A>& type;
		};
	}
}
#endif //!defined( BOOST_NO_CXX11_RVALUE_REFERENCES )
#endif //BOOST_VERSION >= 105800 && BOOST_VERSION < 106000
/**********************************************************************************/
#endif //(LIBTORRENT_VERSION_NUM < 10200) /*< 1.2.0*/ && !defined(TORRENT_NO_DEPRECATE)

#if (LIBTORRENT_VERSION_NUM < 10200)
#define LT_PLUGIN_STR_TYPE char const *
#define LT_PLUGIN_BUFFER_TYPE lt::buffer::const_interval
#define LT_PLUGIN_BUFFER_BEGIN(x) ((x).begin)
#define LT_PLUGIN_ON_HANDSHAKE_TYPE LT_PLUGIN_STR_TYPE
#define LT_PLUGIN_SEND_BUFFER(x, msg) (m_pc.send_buffer((msg), (sizeof(msg))))
static constexpr auto LT_BT_CONNECTION = lt::peer_connection::bittorrent_connection;
#else
#define LT_PLUGIN_STR_TYPE lt::string_view
#define LT_PLUGIN_BUFFER_TYPE lt::span<char const>
#define LT_PLUGIN_BUFFER_BEGIN(x) ((x).begin())
#define LT_PLUGIN_ON_HANDSHAKE_TYPE LT_PLUGIN_BUFFER_TYPE
#define LT_PLUGIN_SEND_BUFFER(x, msg) (m_pc.send_buffer({ (msg), (sizeof(msg)) }))
static constexpr auto LT_BT_CONNECTION = lt::connection_type::bittorrent;
#endif //#if (LIBTORRENT_VERSION_NUM < 10200)

//FIX: 'undefined reference' for GCC-6
constexpr lt::alert_category_t VS_PeerProgressAlert::static_category;

class VS_PeerProgressBase : public lt::peer_plugin
{
public:
	explicit VS_PeerProgressBase(lt::bt_peer_connection &pc, lt::torrent &t, int id)
		: m_pc(pc)
		, m_torrent(t)
		, m_progress(0)
		, m_id(id)
		, m_messageIndex(0)
		, m_canDisconnect(false)
	{
	}

	LT_PLUGIN_STR_TYPE type() const noexcept override { return "vs_peer_progress"; }

	void add_handshake(lt::entry &h) override
	{
		lt::entry &messages = h["m"];
		messages["vs_peer_progress"] = m_id;
	}

	bool on_extension_handshake(const lt::bdecode_node &h) override
	{
		m_messageIndex = 0;

		if (h.type() != lt::bdecode_node::dict_t) return false;
		const lt::bdecode_node messages = h.dict_find_dict("m");
		if (!messages) return false;

        //use int because vs_peer_progress does't return more or less 13/-13
		const int index = static_cast<int>(messages.dict_find_int_value("vs_peer_progress", -1));
		if (index == -1) return false;

		m_messageIndex = std::abs(index);
		m_canDisconnect = index == m_id;

		return true;
	}

	bool can_disconnect(lt::error_code const &) noexcept override
	{
		return m_canDisconnect;
	}

protected:
	lt::bt_peer_connection &m_pc;
	lt::torrent &m_torrent;
	uint64_t m_progress;
	const int m_id;
	int m_messageIndex;
	bool m_canDisconnect;
};

template<class PeerPlugin>
class VS_LibTorrentPlugin : public lt::torrent_plugin
{
public:
	VS_LibTorrentPlugin(lt::torrent &t)
	: m_torrent(t)
	{}

	lt::shared_ptr<lt::peer_plugin> new_connection(const lt::peer_connection_handle &c) override
	{
		if (c.type() != LT_BT_CONNECTION)
		{
			return {};
		}
		lt::bt_peer_connection *pc = static_cast<lt::bt_peer_connection*>(c.native_handle().get());
		return lt::allocate_shared<PeerPlugin>(m_pool, *pc, m_torrent, this);
	}
private:
	lt::torrent &m_torrent;
	boost::fast_pool_allocator<PeerPlugin> m_pool;
};

lt::shared_ptr<lt::torrent_plugin> CreateVSLibTorrentPluginServer(const lt::torrent_handle &th, void*)
{
	lt::torrent *t = th.native_handle().get();

	//server peer does not connect to server peer
	constexpr int SERVER_TORRENT_ID = 13;

	class VS_PeerProgressServer final : public VS_PeerProgressBase
	{
	public:
		explicit VS_PeerProgressServer(lt::bt_peer_connection &pc, lt::torrent &t, void */*tp*/)
			: VS_PeerProgressBase(pc, t, SERVER_TORRENT_ID)
		{}

		void tick() override
		{
			if (m_canDisconnect) return;
			if (!m_messageIndex) return;
			if (!m_torrent.valid_metadata()) return;
			if (m_pc.is_disconnecting()) return;
			if (m_pc.associated_torrent().expired()) return;

			lt::torrent_status s;
			m_torrent.bytes_done(s, {});

			assert(s.total_wanted_done >= 0);
			if (m_progress >= static_cast<decltype(m_progress)>(s.total_wanted_done)) return;

			m_progress = s.total_wanted_done;

			const auto pr = vs_htonll(m_progress);

			char msg[]{
				0, 0, 0, 10, // message size (4-byte uint)
				lt::bt_peer_connection::msg_extended,
				(char)m_messageIndex, //before fix - uint8_t(m_message_index),
				(char)(pr & 0xff),   // download progress
				(char)((pr >> 8) & 0xff),
				(char)((pr >> 16) & 0xff),
				(char)((pr >> 24) & 0xff),
				(char)((pr >> 32) & 0xff),
				(char)((pr >> 40) & 0xff),
				(char)((pr >> 48) & 0xff),
				(char)((pr >> 56) & 0xff),
			};

			LT_PLUGIN_SEND_BUFFER(m_pc, msg);
			m_pc.stats_counters().inc_stats_counter(lt::counters::num_outgoing_extended);

			assert(s.total_wanted >= 0);
			m_canDisconnect = m_progress >= static_cast<decltype(m_progress)>(s.total_wanted);
		}

		//TODO: redundant code; if defined VS_TORRENT_DISABLE_LEGACY then the Client < ~7.4.1 < will break
#ifndef VS_TORRENT_DISABLE_LEGACY
		bool on_extended(const int length, const int msg, LT_PLUGIN_BUFFER_TYPE /*body*/) override
		{
			if (msg != m_messageIndex) return false;
			if (m_messageIndex == 0) return false;
			if(length == 8)
			{
				m_pc.stats_counters().inc_stats_counter(lt::counters::num_incoming_extended);
				return true;
			}
			return false;
		}
#endif //VS_TORRENT_DISABLE_LEGACY
	};

	return lt::make_shared<VS_LibTorrentPlugin<VS_PeerProgressServer>>(*t);
}

lt::shared_ptr<lt::torrent_plugin> CreateVSLibTorrentPluginClient(const lt::torrent_handle &th, void*) {
	lt::torrent *t = th.native_handle().get();

	//client peer does not connect to client peer
	constexpr int CLIENT_TORRENT_ID = -13;

	class VS_PeerProgressClient;

	class VS_LibTorrentPluginClient final : public VS_LibTorrentPlugin<VS_PeerProgressClient>
	{
	public:
		explicit VS_LibTorrentPluginClient(lt::torrent &t)
			: VS_LibTorrentPlugin<VS_PeerProgressClient>(t)
			, m_useExtended(true)
		{}

		bool UseExtended() const noexcept
		{
			return m_useExtended;
		}

		void UseExtended(bool value) noexcept
		{
			m_useExtended = value;
		}

	private:
		bool m_useExtended;
	};

	class VS_PeerProgressClient : public VS_PeerProgressBase
	{
	public:
		explicit VS_PeerProgressClient(lt::bt_peer_connection &pc, lt::torrent &t, void *tp)
			: VS_PeerProgressBase(pc, t, CLIENT_TORRENT_ID)
			, m_tp(*static_cast<VS_LibTorrentPluginClient*>(tp))
		{}

		bool on_handshake(LT_PLUGIN_ON_HANDSHAKE_TYPE) override
		{
			return m_tp.UseExtended();
		}

		bool on_have_all() override
		{
			lt::torrent_status s;
			m_torrent.bytes_done(s, {});
			post_progress_alert(s.total_wanted);
			m_pc.stats_counters().inc_stats_counter(lt::counters::num_incoming_have_all);
			return false;
		}

		bool on_have_none() override
		{
			post_progress_alert(0);
			m_pc.stats_counters().inc_stats_counter(lt::counters::num_incoming_have_none);
			return false;
		}

		bool on_extended(const int length, const int msg, LT_PLUGIN_BUFFER_TYPE body) override
		{
			if (msg != m_messageIndex) return false;
			if (m_messageIndex == 0) return false;
			if (length == 8)
			{
				if (!m_canDisconnect)
				{
					const uint64_t pr = vs_ntohll(vs::mem_cast<uint64_t>(LT_PLUGIN_BUFFER_BEGIN(body)));

					if (pr > m_progress)
					{
						post_progress_alert(pr);

						lt::torrent_status s;
						m_torrent.bytes_done(s, {});

						m_progress = pr;
						m_canDisconnect = m_progress >= static_cast<decltype(m_progress)>(s.total_wanted);
						m_tp.UseExtended(!m_canDisconnect);
					}
				}
				m_pc.stats_counters().inc_stats_counter(lt::counters::num_incoming_extended);
				return true;
			}
			return false;
		}

	private:
		void post_progress_alert(uint64_t progress) const
		{
			auto &&alerts = m_torrent.alerts();
			if (alerts.should_post<VS_PeerProgressAlert>())
			{
				alerts.emplace_alert<VS_PeerProgressAlert>(m_torrent.get_handle(), m_pc.peer_info_struct()->ip(), m_pc.pid(), true, /*connected_with_server*/ progress);
			}
		}

	private:
		VS_LibTorrentPluginClient &m_tp;
	};

	return lt::make_shared<VS_LibTorrentPluginClient>(*t);
}

#undef LT_PLUGIN_SEND_BUFFER
#undef LT_PLUGIN_ON_HANDSHAKE_TYPE
#undef LT_PLUGIN_BUFFER_BEGIN
#undef LT_PLUGIN_BUFFER_TYPE
#undef LT_PLUGIN_STR_TYPE