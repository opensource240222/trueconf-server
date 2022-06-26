#!/bin/sh

die() {
	echo >&2 error: "$@"
	exit 2
}

check_prog() {
	command -v "$1" >/dev/null 2>&1 || die "Program '$1' is required, but wasn't found."
}

check_prog objcopy
check_prog objdump

in="$1"
[ -n "${in}" ] || { echo "Usage: $0 file"; exit 1; }

has_section() {
	objdump --section-headers --section="$1" -- "${in}" 1>/dev/null 2>/dev/null
}

dbg="${in}.debug"

has_section .debug_info || has_section .zdebug_info || die "File '${in}' doesn't contain debug information"
has_section .gnu_debuglink && die "File '${in}' already has a debug link set"

objcopy --only-keep-debug -- "${in}" "${dbg}" || die "objcopy --only-keep-debug failed"
objcopy --strip-debug -- "${in}" || die "objcopy --strip-debug failed"
objcopy --add-gnu-debuglink="${dbg}" -- "${in}" || die "objcopy --add-gnu-debuglink failed"
