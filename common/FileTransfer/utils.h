#pragma once

#include <string>

#include "std-generic/cpplib/string_view.h"

#include "fwd.h"
#include "libtorrent/bdecode.hpp"

#if (LIBTORRENT_VERSION_NUM < 10200)
#include "libtorrent/add_torrent_params.hpp"
#else
#include "libtorrent/torrent_flags.hpp"
#include "libtorrent/sha1_hash.hpp"
#endif //(LIBTORRENT_VERSION_NUM < 10200)

namespace libtorrent
{
	/*
	* !!! ONLY FOR LIBTORRENT
	* Cross platform separator because libtorrent usage convert_separators:
	* WIN: '/' -> '\\'
	* Other OS - it isn't using convertor
	*/
	static constexpr char FILE_SEPARATOR = '/';

	std::string hash_to_string(const sha1_hash &h);
	sha1_hash string_to_hash(::string_view str, error_code &ec);

	void vs_parse_magnet_uri(::string_view uri, add_torrent_params &p, uint64_t &xl, /*xl = [Size in Bytes]*/ error_code &ec) noexcept;
	std::string vs_make_magnet_uri(const torrent_handle &handle);

	std::string vs_parent_path(std::string const& f) noexcept;

	std::string get_id(const session &ses) noexcept;

#if (LIBTORRENT_VERSION_NUM < 10200)
	
	typedef std::underlying_type<add_torrent_params::flags_t>::type torrent_flags_t;

	namespace torrent_flags
	{
		static constexpr torrent_flags_t paused = add_torrent_params::flag_paused;
		static constexpr torrent_flags_t auto_managed = add_torrent_params::flag_auto_managed;
		static constexpr torrent_flags_t upload_mode = add_torrent_params::flag_upload_mode;
		static constexpr torrent_flags_t update_subscribe = add_torrent_params::flag_update_subscribe;
		static constexpr torrent_flags_t duplicate_is_error = add_torrent_params::flag_duplicate_is_error;
		static constexpr torrent_flags_t apply_ip_filter = add_torrent_params::flag_apply_ip_filter;
		static constexpr torrent_flags_t seed_mode = add_torrent_params::flag_seed_mode;
		static constexpr torrent_flags_t share_mode = add_torrent_params::flag_share_mode;
		static constexpr torrent_flags_t stop_when_ready = add_torrent_params::flag_stop_when_ready;
		static constexpr torrent_flags_t super_seeding = add_torrent_params::flag_super_seeding;
		static constexpr torrent_flags_t sequential_download = add_torrent_params::flag_sequential_download;
	} //namespace torrent_flags
#endif //(LIBTORRENT_VERSION_NUM < 10200)

	void unset_flags(torrent_handle &handle, torrent_flags_t flags) noexcept;
	void set_flags(torrent_handle &handle, torrent_flags_t flags) noexcept;

	entry get_resume_data(const save_resume_data_alert &p) noexcept;
	
	add_torrent_params resume_data(std::vector<char> data, error_code &ec) noexcept;

	bool is_auto_managed(torrent_status &s) noexcept;
	bool is_paused(torrent_status &s) noexcept;
	bool is_sequential_download(torrent_status &s) noexcept;

} // namespace libtorrent