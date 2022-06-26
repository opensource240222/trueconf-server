#!/bin/bash

find "${0%/*}" -type f ! -name "${0##*/}" -exec sed -i -e 's/BOOST_NO_EXCEPTIONS/VS_TORRENT_NO_EXCEPTIONS/g' '{}' +
