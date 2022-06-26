#pragma once

#include "libtorrent/alert_types.hpp"
#include "config.h"

#if (LIBTORRENT_VERSION_NUM < 10200) && !defined(TORRENT_NO_DEPRECATE)
#define LT_CLONE(name) \
		std::auto_ptr<alert> clone_impl() const override { return std::auto_ptr<alert>(new name(*this)); }
#else
#define LT_CLONE(name)
#endif //LIBTORRENT_VERSION_NUM < 10200 && !defined(TORRENT_NO_DEPRECATE)

#if (LIBTORRENT_VERSION_NUM < 10100) // < 1.1.0
namespace libtorrent {
	struct torrent_handle;
	struct torrent_plugin;
	namespace aux {
		struct stack_allocator;
	}
}
#endif //(LIBTORRENT_VERSION_NUM < 10100)

//================================================================================================================================================

struct VS_PeerProgressAlert final : public lt::peer_alert
{
	VS_PeerProgressAlert(libtorrent::aux::stack_allocator &alloc, const libtorrent::torrent_handle &h, const libtorrent::tcp::endpoint &ip,
		const libtorrent::peer_id &pid, bool srvPeer, uint64_t srvProgress)
		: lt::peer_alert(alloc, h, ip, pid)
		, serverProgress(srvProgress)
		, isServerPeer(srvPeer)
	{}

	static constexpr lt::alert_category_t static_category =
#if LIBTORRENT_VERSION_NUM < 10200 || TORRENT_ABI_VERSION == 1
		lt::alert::progress_notification |
#endif //LIBTORRENT_VERSION_NUM < 10200 || TORRENT_ABI_VERSION == 1

		lt::alert::peer_notification;

	static constexpr int priority = lt::alert_priority_high;

	static constexpr int alert_type = -1;
	int type() const noexcept override { return alert_type; }
	lt::alert_category_t category() const noexcept override { return static_category; }
	char const *what() const noexcept override { return ""; }

	LT_CLONE(VS_PeerProgressAlert)

	uint64_t serverProgress;
	bool isServerPeer;
};

lt::shared_ptr<lt::torrent_plugin> CreateVSLibTorrentPluginServer(const lt::torrent_handle &th, void *);
lt::shared_ptr<lt::torrent_plugin> CreateVSLibTorrentPluginClient(const lt::torrent_handle &th, void *);

//================================================================================================================================================

#undef LT_CLONE