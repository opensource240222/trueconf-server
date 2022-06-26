#include "utils.h"

#include <cinttypes>

#include "std-generic/cpplib/IntConv.h"

#include "libtorrent/session.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/magnet_uri.hpp"

#if (LIBTORRENT_VERSION_NUM >= 10200)
#include "libtorrent/hex.hpp"
#include "libtorrent/write_resume_data.hpp"
#include "libtorrent/read_resume_data.hpp"
#include "libtorrent/aux_/path.hpp"
#define SHA1_HASH_SIZE (lt::sha1_hash::size())
#else
#define SHA1_HASH_SIZE (lt::sha1_hash::size)
#endif //(LIBTORRENT_VERSION_NUM >= 10200)

namespace detail
{
	static inline uint64_t parse_magnet_xl(::string_view uri) noexcept
	{
		assert(!uri.empty());

		const auto pos = uri.find("xl=");
		return pos == decltype(uri)::npos ? 0 : vs::atoull_sv(uri.substr(pos + 3, uri.find_first_of('&', pos + 3)));
	}

	static inline void add_xl_param(std::string &uri, uint64_t size) noexcept
	{
		assert(!uri.empty());
		if (uri.find("xl=") == std::string::npos)
		{
			char buffer[std::numeric_limits<decltype(size)>::digits10 + 1 + 1 /*sign*/ + 1 /*0-terminator*/];
			::sprintf(buffer, "%" PRIu64, size);
			uri.append("&xl=").append(buffer);
		}
	}

	static inline bool from_hex(::string_view in, lt::sha1_hash &out) noexcept
	{
		assert(in.length() == SHA1_HASH_SIZE * 2);

#if (LIBTORRENT_VERSION_NUM < 10200) // < 1.2.0
		return lt::from_hex(in.data(), lt::sha1_hash::size * 2, out.data());
#else
		return lt::aux::from_hex({ in.data(), lt::sha1_hash::size() * 2 }, out.data());
#endif //LIBTORRENT_VERSION_NUM < 10200
	}

#if (LIBTORRENT_VERSION_NUM < 10200) // < 1.2.0
	static inline void set_flags(lt::torrent_handle &handle ,lt::torrent_flags_t const flags, lt::torrent_flags_t const mask) noexcept
	{
		if (mask & lt::torrent_flags::upload_mode)
		{
			handle.set_upload_mode(bool(flags & lt::torrent_flags::upload_mode));
		}
		if (mask & lt::torrent_flags::share_mode)
		{
			handle.set_share_mode(bool(flags & lt::torrent_flags::share_mode));
		}
		if (mask & lt::torrent_flags::apply_ip_filter)
		{
			handle.apply_ip_filter(bool(flags & lt::torrent_flags::apply_ip_filter));
		}
		if (mask & lt::torrent_flags::paused)
		{
			if (flags & lt::torrent_flags::paused)
			{
				handle.pause(lt::torrent_handle::graceful_pause);
			}
			else
			{
				handle.resume();
			}
		}
		if (mask & lt::torrent_flags::auto_managed)
		{
			handle.auto_managed(bool(flags & lt::torrent_flags::auto_managed));
		}
		if (mask & lt::torrent_flags::super_seeding)
		{
			handle.super_seeding(bool(flags & lt::torrent_flags::super_seeding));
		}
		if (mask & lt::torrent_flags::sequential_download)
		{
			handle.set_sequential_download(bool(flags & lt::torrent_flags::sequential_download));
		}
		if (mask & lt::torrent_flags::stop_when_ready)
		{
			handle.stop_when_ready(bool(flags & lt::torrent_flags::stop_when_ready));
		}
	}
#endif //LIBTORRENT_VERSION_NUM < 10200

}//namespace detail 

namespace libtorrent
{
	std::string hash_to_string(const sha1_hash &h)
	{
#if (LIBTORRENT_VERSION_NUM < 10200) // < 1.2.0
		char out[(SHA1_HASH_SIZE * 2) + 1 /*0-terminator*/];
		lt::to_hex(h.data(), SHA1_HASH_SIZE, out);
		return std::string{ out, sizeof(out) - 1 /*0-terminator*/ };
#else
		return aux::to_hex(h);
#endif //(LIBTORRENT_VERSION_NUM < 10200) // < 1.2.0
	}

	sha1_hash string_to_hash(::string_view str, error_code &ec)
	{
		sha1_hash info_hash;
		if (str.length() == SHA1_HASH_SIZE * 2 && ::detail::from_hex(str, info_hash))
		{
			ec.clear();
			return info_hash;
		}
		ec = errors::invalid_info_hash;
		return info_hash;
	}

	void vs_parse_magnet_uri(::string_view uri, add_torrent_params &p, uint64_t &xl, error_code &ec) noexcept
	{
		parse_magnet_uri({ uri.data(), uri.length() }, p, ec);
		if (ec) // error
		{
			return;
		}

		xl = ::detail::parse_magnet_xl(uri);
	}

	std::string vs_make_magnet_uri(const torrent_handle &handle)
	{
		std::string uri = make_magnet_uri(handle);
		if (!uri.empty())
		{
			assert(handle.is_valid());
			auto&& status = handle.status();
			::detail::add_xl_param(uri, status.total_wanted);
		}
		return uri;
	}

	std::string vs_parent_path(std::string const &f) noexcept
	{
		auto parent_path = lt::parent_path(f);
		if (!parent_path.empty() && (
#ifdef TORRENT_WINDOWS
			parent_path.back() == '\\' ||
#endif //TORRENT_WINDOWS
			parent_path.back() == '/')
			)
		{
			parent_path.pop_back();
		}

		return parent_path;
	}

	std::string get_id(const lt::session &ses) noexcept
	{
#if	(LIBTORRENT_VERSION_NUM >= 10107) /*>= 1.1.7*/ && defined(TORRENT_NO_DEPRECATE)
		return ses.get_settings().get_str(settings_pack::peer_fingerprint);
#else
		return ses.id().to_string();
#endif //(LIBTORRENT_VERSION_NUM >= 10107) /*>= 1.1.7*/ && defined(TORRENT_NO_DEPRECATE)
	}

	void unset_flags(torrent_handle &handle, torrent_flags_t flags) noexcept
	{
#if LIBTORRENT_VERSION_NUM < 10200
		::detail::set_flags(handle, torrent_flags_t{}, flags);
#else
		handle.unset_flags(flags);
#endif //LIBTORRENT_VERSION_NUM < 10200
	}

	void set_flags(torrent_handle &handle, torrent_flags_t flags) noexcept
	{
#if LIBTORRENT_VERSION_NUM < 10200
		::detail::set_flags(handle, ~torrent_flags_t{}, flags);
#else
		handle.set_flags(flags);
#endif //LIBTORRENT_VERSION_NUM < 10200
	}

	entry get_resume_data(const save_resume_data_alert &p) noexcept
	{
#if LIBTORRENT_VERSION_NUM < 10200
		return *(p.resume_data);
#else
		return write_resume_data(p.params);
#endif //LIBTORRENT_VERSION_NUM < 10200
	}

	add_torrent_params resume_data(std::vector<char> data, lt::error_code &ec) noexcept
	{
#if LIBTORRENT_VERSION_NUM < 10200
		add_torrent_params p;
		p.resume_data = std::move(data);
		ec.clear();
		return p;
#else
		return read_resume_data(std::move(data), ec);
#endif //LIBTORRENT_VERSION_NUM < 10200
	}

	bool is_auto_managed(torrent_status &s) noexcept
	{
#if LIBTORRENT_VERSION_NUM < 10200
		return s.auto_managed;
#else
		return bool { s.flags & torrent_flags::auto_managed };
#endif //LIBTORRENT_VERSION_NUM < 10200
	}

	bool is_paused(torrent_status &s) noexcept
	{
#if LIBTORRENT_VERSION_NUM < 10200
		return s.auto_managed;
#else
		return bool{ s.flags & torrent_flags::paused };
#endif //LIBTORRENT_VERSION_NUM < 10200
	}

	bool is_sequential_download(torrent_status &s) noexcept
	{
#if LIBTORRENT_VERSION_NUM < 10200
		return s.sequential_download;
#else
		return bool{ s.flags & torrent_flags::sequential_download };
#endif //LIBTORRENT_VERSION_NUM < 10200
	}
} //namespace libtorrent


#undef SHA1_HASH_SIZE