#!/bin/bash
# Usage:
#   git bisect start <bad commit> <good commit>
#   git bisect run common/fbuild/bisect-helper.sh
# Build directory can be specified in the first argument.
# (useful when bisecting tc-common but building targets from tc3 (e.g. Test))
# By default runs tests, other targets may be specified in the second argument.

build_dir="${1:-.}"
targets="${2:-Test-MSVC120-Win32-Debug-Default}"

git submodule update || exit 125
[ -f fbuild.bff ] || exit 125

pushd "${build_dir}"
# Calling cmd as a workaround for bug in MinGW bash: https://sourceforge.net/p/mingw/bugs/747/
cmd //v:on //c "fbuild -j6 -cache ${targets} & if !errorlevel! lss 0 exit 10"
res=$?
popd
exit $res
