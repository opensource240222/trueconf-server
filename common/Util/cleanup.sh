#!/bin/bash

die() {
	echo >&2 error: "$@"
	exit 2
}

dir="$1"
if [[ -z "${dir}" ]]; then
	# Directory to clean is not specified, try to find repository root and clean it
	pushd "${0%/*}" > /dev/null
	dir="$(git rev-parse --show-superproject-working-tree)"
	[[ -z "${dir}" ]] && dir="$(git rev-parse --show-toplevel)"
	popd > /dev/null
	[[ -n "${dir}" ]] || die "Can't find repository root"
fi
[[ -f "${dir}/fbuild.bff" ]] || die "Repository root is '${dir}', but it doesn't look like a TCS repository"

# Remove Windows build results
find "${dir}" -regex '.*/\(MSVC\(120\|140\|141\)\|Clang\)-Win\(32\|32XP\|64\).*\(.dll\|.exe\|.exp\|.ilk\|.lib\|.map\|.obj\|.pdb\|.res\|TestOutput.txt\)' -print -delete
# Remove Linux build results
find "${dir}" -regex '.*/\(GCC\|Clang\|GCC47\|Clang34\)-Linux\(32\|64\).*\(.a\|.o\|.so\|TestOutput.txt\)' -print -delete
# Remove Linux binaries, they can't be identified by extension so we need a significantly different pattern
find "${dir}" -regex '.*/\(GCC\|Clang\|GCC47\|Clang34\)-Linux\(32\|64\)[^/]*/[^/.]*' -print -delete
# Remove empty directories
find "${dir}" -depth -type d -empty -print -delete
