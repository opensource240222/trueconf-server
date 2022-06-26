#ifndef TORRENT_CONFIG_H
#define TORRENT_CONFIG_H

#include "libtorrent/version.hpp"
#include "libtorrent/config.hpp"

#include "std-generic/compat/type_traits.h"

namespace libtorrent
{
	template<typename T, typename = vs::void_t<>> 
	struct underlying final
	{ 
		typedef T type;
	};

	template<typename T>
	struct underlying <T, vs::void_t<typename T::underlying_type>> final
	{
		typedef typename T::underlying_type type;
	};

	template<typename T>
	using underlying_t = typename underlying<T>::type;

#if (LIBTORRENT_VERSION_NUM < 10200)
	typedef int alert_category_t;
	typedef int peer_class_t;
	typedef int queue_position_t;
	typedef int session_flag_t;
	typedef int status_flags_t;
	typedef int piece_index_t;
	typedef int file_index_t;
	typedef int remove_flags_t;
	typedef std::string string_t;

	using boost::allocate_shared;
#else
	typedef lt::string_view string_t;

	using std::shared_ptr;
	using std::make_shared;
	using std::allocate_shared;
#endif //(LIBTORRENT_VERSION_NUM < 10200)
} //namespace libtorrent

#if (LIBTORRENT_VERSION_NUM < 10105) // < 1.1.5
namespace lt = libtorrent;
#endif //LIBTORRENT_VERSION_NUM < 10105

#endif //TORRENT_CONFIG_H