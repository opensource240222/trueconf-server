#ifndef TORRENT_FWD_H
#define TORRENT_FWD_H

#include "libtorrent/version.hpp"
#include "libtorrent/fwd.hpp"

#if (LIBTORRENT_VERSION_NUM < 10105) // < 1.1.5
namespace lt = libtorrent;
#endif //(LIBTORRENT_VERSION_NUM < 10105)

#endif //#define TORRENT_FWD_H