#!/bin/bash

if [[ -z "$1" || -z "$2" ]]; then
	echo "Usage: ${0##*/} path_to_libspeex_src path_to_libspeexdsp_src" >&2
	exit 1
fi

pushd "$(dirname "$0")"

git rm -rf speex src
mkdir -p speex src

cp -v -- "$2"/include/speex/*.h speex/
cp -v -- "$1"/include/speex/*.h speex/

cp -v -- "$2"/libspeexdsp/*.{h,c} src/
cp -v -- "$1"/libspeex/*.{h,c} src/
cp -v -- "$1"/win32/config.h src/
rm -v -- src/test*.c

git add speex src

popd
