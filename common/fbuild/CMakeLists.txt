#cmake_minimum_required(VERSION 3.5)
cmake_minimum_required(VERSION 3.2)
project(bff-config)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

# Validate host system
if(CMAKE_HOST_WIN32)
	set(ON_WINDOWS true)
elseif(CMAKE_HOST_SYSTEM MATCHES "Linux")
	set(ON_LINUX true)
	execute_process(COMMAND uname -m OUTPUT_VARIABLE HOST_ARCH)
	string(REPLACE "\n" "" HOST_ARCH "${HOST_ARCH}")
	if(NOT HOST_ARCH MATCHES "i686|x86_64")
		message(WARNING "Unknown architecture: \"${HOST_ARCH}\", assuming x86_64")
		set(HOST_ARCH "x86_64")
	endif()
else()
	message(FATAL_ERROR "Unknown platform")
endif()

#
# Configuration and checks
#

if(ON_WINDOWS)
	option(ENABLE_Win32 "Enable Win32 build platform" ON)
	option(ENABLE_Win32XP "Enable Win32XP build platform" ON)

	set(pf86 "ProgramFiles(x86)")

	# Visual C++ 12.0
	find_path(MSVC120_DIR "VC/bin/cl.exe" PATHS
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\12.0\\Setup\\VS;ProductDir]"
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\VisualStudio\\12.0\\Setup\\VS;ProductDir]"
		"$ENV{ProgramFiles}/Microsoft Visual Studio 12.0"
		"$ENV{${pf86}}/Microsoft Visual Studio 12.0"
		"C:/Program Files/Microsoft Visual Studio 12.0"
		"C:/Program Files (x86)/Microsoft Visual Studio 12.0"
		DOC "Path to Microsoft Visual Studio 12.0"
		)

	# Visual C++ 14.0
	find_path(MSVC140_DIR "VC/bin/cl.exe" PATHS
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\14.0\\Setup\\VS;ProductDir]"
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\VisualStudio\\14.0\\Setup\\VS;ProductDir]"
		"$ENV{ProgramFiles}/Microsoft Visual Studio 14.0"
		"$ENV{${pf86}}/Microsoft Visual Studio 14.0"
		"C:/Program Files/Microsoft Visual Studio 14.0"
		"C:/Program Files (x86)/Microsoft Visual Studio 14.0"
		DOC "Path to Microsoft Visual Studio 14.0"
		)

	if(NOT MSVC120_DIR AND NOT MSVC140_DIR)
		message(FATAL_ERROR "Microsoft Visual Studio (12.0 or 14.0) not found")
	endif()
	set(MSVC true)

	if(MSVC140_DIR)
		# Windows SDK 10
		find_path(WINSDK10_DIR "Include/10.0.10240.0/ucrt/stdlib.h" PATHS
			"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]"
			"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]"
			"$ENV{ProgramFiles}/Windows Kits/10"
			"$ENV{${pf86}}/Windows Kits/10"
			"C:/Program Files/Windows Kits/10"
			"C:/Program Files (x86)/Windows Kits/10"
			DOC "Path to Windows SDK 10"
			)
		if (NOT WINSDK10_DIR)
			message(FATAL_ERROR "Windows SDK 10 not found")
		endif()
	endif()

	# Windows SDK 8.1
	find_path(WINSDK81_DIR "Include/um/Windows.h" PATHS
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v8.1;InstallationFolder]"
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Microsoft SDKs\\Windows\\v8.1;InstallationFolder]"
		"$ENV{ProgramFiles}/Windows Kits/8.1"
		"$ENV{${pf86}}/Windows Kits/8.1"
		"C:/Program Files/Windows Kits/8.1"
		"C:/Program Files (x86)/Windows Kits/8.1"
		DOC "Path to Windows SDK 8.1"
		)
	if (NOT WINSDK81_DIR)
		message(FATAL_ERROR "Windows SDK 8.1 not found")
	endif()

	# Windows SDK 7.1A
	find_path(WINSDK71A_DIR "Include/Windows.h" PATHS
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v7.1A;InstallationFolder]"
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Microsoft SDKs\\Windows\\v7.1A;InstallationFolder]"
		"$ENV{ProgramFiles}/Microsoft SDKs/Windows/v7.1A"
		"$ENV{${pf86}}/Microsoft SDKs/Windows/v7.1A"
		"C:/Program Files/Microsoft SDKs/Windows/v7.1A"
		"C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A"
		DOC "Path to Windows SDK 7.1A"
		)
	if (NOT WINSDK71A_DIR)
		message(FATAL_ERROR "Windows SDK 7.1A not found")
	endif()

	# DirectX SDK
	find_path(DXSDK_DIR "Include/d3d9.h" PATHS
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\DirectX\\Microsoft DirectX SDK (June 2010);InstallPath]"
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\DirectX\\Microsoft DirectX SDK (June 2010);InstallPath]"
		"$ENV{ProgramFiles}/Microsoft DirectX SDK (June 2010)"
		"$ENV{${pf86}}/Microsoft DirectX SDK (June 2010)"
		"C:/Program Files/Microsoft DirectX SDK (June 2010)"
		"C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)"
		"$ENV{DXSDK_DIR}"
		DOC "Path to DirectX SDK"
		)
	if (NOT DXSDK_DIR)
		message(FATAL_ERROR "DirectX SDK not found")
	endif()
elseif(ON_LINUX)
	option(ENABLE_Linux32 "Enable Linux32 platform" ON)
	option(ENABLE_Linux64 "Enable Linux64 platform" ON)

	if(NOT GCC_BIN)
		file(GLOB gcc_bin_hints
			/usr/bin/${HOST_ARCH}-pc-linux-gnu
			/usr/${HOST_ARCH}-pc-linux-gnu/bin
			/usr/${HOST_ARCH}-pc-linux-gnu/gcc-bin/*
			/usr/bin
			/bin
			)
		find_program(gcc_bin gcc PATHS ${gcc_bin_hints} NO_DEFAULT_PATH)
		if(gcc_bin)
			get_filename_component(gcc_bin "${gcc_bin}" REALPATH)
			set(GCC_BIN "${gcc_bin}" CACHE FILEPATH
				"Path to real GCC binary")
		unset(gcc_bin CACHE)
		endif()
	endif()

	if(NOT GCC_LIBEXECDIR)
		file(GLOB gcc_libexecdir_hints
			/usr/libexec/gcc/${HOST_ARCH}-pc-linux-gnu/*
			/usr/${HOST_ARCH}-pc-linux-gnu/libexec/gcc
			/usr/libexec
			)
		find_program(gcc_cc1plus_bin cc1plus PATHS ${gcc_libexecdir_hints} NO_DEFAULT_PATH)
		if(gcc_cc1plus_bin)
			get_filename_component(gcc_cc1plus_bin "${gcc_cc1plus_bin}" REALPATH)
			get_filename_component(GCC_LIBEXECDIR "${gcc_cc1plus_bin}" DIRECTORY)
			set(GCC_LIBEXECDIR "${GCC_LIBEXECDIR}" CACHE PATH
				"Path to directory that contains real GCC helper binaries (cc1, cc1plus)")
			unset(gcc_cc1plus_bin CACHE)
		endif()
	endif()

	if(NOT BINUTILS_BINDIR)
		file(GLOB binutils_bindir_hints
			/usr/bin/${HOST_ARCH}-pc-linux-gnu/bin
			/usr/${HOST_ARCH}-pc-linux-gnu/bin
			/usr/${HOST_ARCH}-pc-linux-gnu/binutils-bin/*
			/usr/bin
			/bin
			)
		find_program(as_bin as PATHS ${binutils_bindir_hints} NO_DEFAULT_PATH)
		if(as_bin)
			get_filename_component(as_bin "${as_bin}" REALPATH)
			get_filename_component(BINUTILS_BINDIR "${as_bin}" DIRECTORY)
			set(BINUTILS_BINDIR "${BINUTILS_BINDIR}" CACHE PATH
				"Path to directory that contains real binutils binaries (as, ar, ld)")
			unset(as_bin CACHE)
		endif()
	endif()

	if(NOT CLANG_BIN)
		file(GLOB clang_bin_hints
			/usr/bin/${HOST_ARCH}-pc-linux-gnu
			/usr/${HOST_ARCH}-pc-linux-gnu/bin
			/usr/bin
			/bin
			)
		find_program(clang_bin clang PATHS ${clang_bin_hints} NO_DEFAULT_PATH)
		if(clang_bin)
			get_filename_component(clang_bin "${clang_bin}" REALPATH)
			set(CLANG_BIN "${clang_bin}" CACHE FILEPATH
				"Path to real Clang binary")
			unset(clang_bin CACHE)
		endif()
	endif()
endif()

option(ENABLE_Debug "Enable Debug configuration" ON)
option(ENABLE_Release "Enable Release configuration" ON)

option(ENABLE_Default "Enable Default trait" ON)
option(ENABLE_SVKSM "Enable SVKSM trait" ON)
option(ENABLE_Conferendo "Enable Conferendo trait" ON)
option(ENABLE_Terminal "Enable Terminal trait" OFF)

# Required libs
if(ON_WINDOWS)
	if(MSVC120_DIR)
		set(Boost_COMPILER "-vc120")
	elseif(MSVC140_DIR)
		set(Boost_COMPILER "-vc140")
	endif()
	set(Boost_USE_STATIC_RUNTIME true)

	# On Windows we use autolinking, so we search only for one lib to populate Boost_LIBRARY_DIRS...
	find_package(Boost 1.56.0 REQUIRED COMPONENTS system)
	# ...and redefine Boost_LIBRARIES to store it with appropriate flag.
	set(Boost_LIBRARIES "")
	foreach(dir ${Boost_LIBRARY_DIRS})
		set(Boost_LIBRARIES "${Boost_LIBRARIES} /LIBPATH:\"${dir}\"")
	endforeach()
elseif(ON_LINUX)
	find_package(Boost 1.56.0 REQUIRED COMPONENTS
		date_time
		regex
		signals
		system
		thread
		)
endif()

set(OPENSSL_MSVC_STATIC_RT true)
find_package(OpenSSL REQUIRED)

find_package(websocketpp 0.5.1 EXACT REQUIRED)

# Optional libs
option(WITH_SYSTEM_GOOGLETEST "Use system gtest/gmock instead of bundled version" OFF)
if(WITH_SYSTEM_GOOGLETEST)
	find_package(GTest REQUIRED)
	find_package(GMock REQUIRED)
endif()
option(WITH_SYSTEM_CURL "Use system libcurl instead of bundled version" OFF)
if(WITH_SYSTEM_LIBCURL)
	find_package(CURL)
endif()

set(WITH_WEBRTC_PEERCONNECTION_MSVC120 "" CACHE PATH "Path to webrtc-peerconnection built with MSVC120")
set(WITH_WEBRTC_PEERCONNECTION_MSVC140 "" CACHE PATH "Path to webrtc-peerconnection built with MSVC140")

#
# local.bff creation
#

include(CMakeParseArguments)

function (bff_add_component)
	# Parse, validate and prepare arguments
	cmake_parse_arguments("arg" "" "NAME" "" ${ARGN})
	if (NOT arg_UNPARSED_ARGUMENTS)
		message(AUTHOR_WARNING "bff_add_component requires arguments")
		return()
	endif()
	if(NOT arg_NAME)
		list(GET arg_UNPARSED_ARGUMENTS 0 arg_NAME)
		string(TOLOWER "${arg_NAME}" arg_NAME)
	endif()

	# Check if all packages are found, and collect variables
	set(INCLUDE_DIRS "")
	set(LIBRARIES "")
	foreach(package ${arg_UNPARSED_ARGUMENTS})
		if (NOT ${package}_FOUND)
			return()
		endif()
		# Some packages define INCLUDE_DIRS and others INCLUDE_DIR
		list(APPEND INCLUDE_DIRS ${${package}_INCLUDE_DIR})
		list(APPEND INCLUDE_DIRS ${${package}_INCLUDE_DIRS})
		list(APPEND LIBRARIES ${${package}_LIBRARIES})
	endforeach()

	string(TOUPPER "${arg_NAME}" output)
	set(output "#define HAVE_${output}\n")

	# Write Includes variable
	set(output "${output}.${arg_NAME}_Includes =\n{\n")
	list(REMOVE_DUPLICATES INCLUDE_DIRS)
	foreach(dir ${INCLUDE_DIRS})
		set(output "${output}\t'${dir}'\n")
	endforeach()
	set(output "${output}}\n")

	# Write LinkerOptions variables
	set(LinkerOptions_Debug "")
	set(LinkerOptions_Release "")
	foreach(val ${LIBRARIES})
		if(val STREQUAL debug)
			set(debug_lib true)
		elseif(val STREQUAL optimized)
			set(release_lib true)
		else()
			if(debug_lib OR NOT release_lib) # debug_lib OR (NOT debug_lib AND NOT release_lib)
				list(APPEND LinkerOptions_Debug "${val}")
			endif()
			if(release_lib OR NOT debug_lib) # release_lib OR (NOT debug_lib AND NOT release_lib)
				list(APPEND LinkerOptions_Release "${val}")
			endif()
			unset(debug_lib)
			unset(release_lib)
		endif()
	endforeach()
	list(REMOVE_DUPLICATES LinkerOptions_Debug)
	list(REMOVE_DUPLICATES LinkerOptions_Release)
	string(REPLACE ";" " " LinkerOptions_Debug "${LinkerOptions_Debug}")
	string(REPLACE ";" " " LinkerOptions_Release "${LinkerOptions_Release}")
	set(output "${output}.${arg_NAME}_LinkerOptions_Debug   = ' ${LinkerOptions_Debug}'\n")
	set(output "${output}.${arg_NAME}_LinkerOptions_Release = ' ${LinkerOptions_Release}'\n")

	set(local_bff "${local_bff}${output}\n" PARENT_SCOPE)
endfunction()

function(bff_add_enabled_list)
	# Parse, validate and prepare arguments
	cmake_parse_arguments("arg" "" "NAME" "" ${ARGN})
	if (NOT arg_UNPARSED_ARGUMENTS)
		message(AUTHOR_WARNING "bff_add_enabled_list requires arguments")
		return()
	endif()
	if(NOT arg_NAME)
		message(AUTHOR_WARNING "bff_add_enabled_list requires NAME argument")
		return()
	endif()

	set(output ".Enabled_${arg_NAME} = {")
	foreach(opt ${arg_UNPARSED_ARGUMENTS})
		if(ENABLE_${opt})
			set(output "${output} '${opt}'")
		endif()
	endforeach()
	set(local_bff "${local_bff}${output} }\n" PARENT_SCOPE)
endfunction()

set(local_bff "")

# Write platform check
if(ON_WINDOWS)
	set(local_bff "${local_bff}#if !__WINDOWS__\n#error This file is valid for Windows only!\n#endif\n\n")
elseif(ON_LINUX)
	set(local_bff "${local_bff}#if !__LINUX__\n#error This file is valid for Linux only!\n#endif\n\n")
endif()

# Write toolchain configurations
if(ON_WINDOWS)
	if(MSVC120_DIR)
		set(local_bff "${local_bff}#define HAVE_MSVC120\n")
		set(local_bff "${local_bff}.msvc120_dir = '${MSVC120_DIR}'\n")
	endif()
	if(MSVC140_DIR)
		set(local_bff "${local_bff}#define HAVE_MSVC140\n")
		set(local_bff "${local_bff}.msvc140_dir = '${MSVC140_DIR}'\n")
	endif()
	if(WINSDK10_DIR)
		set(local_bff "${local_bff}.winsdk10_dir = '${WINSDK10_DIR}'\n")
	endif()
	set(local_bff "${local_bff}.winsdk81_dir = '${WINSDK81_DIR}'\n")
	set(local_bff "${local_bff}.winsdk71a_dir = '${WINSDK71A_DIR}'\n")
	set(local_bff "${local_bff}.dxsdk_dir = '${DXSDK_DIR}'\n")
elseif(ON_LINUX)
	if (GCC_BIN AND GCC_LIBEXECDIR AND BINUTILS_BINDIR)
		set(local_bff "${local_bff}#define HAVE_GCC\n")
		set(local_bff "${local_bff}.gcc_bin = '${GCC_BIN}'\n")
		set(local_bff "${local_bff}.gcc_libexecdir = '${GCC_LIBEXECDIR}'\n")
		set(local_bff "${local_bff}.binutils_bindir = '${BINUTILS_BINDIR}'\n")
	endif()
	if (CLANG_BIN)
		set(local_bff "${local_bff}#define HAVE_CLANG\n")
		set(local_bff "${local_bff}.clang_bin = '${CLANG_BIN}'\n")
	endif()
endif()
set(local_bff "${local_bff}\n")

bff_add_enabled_list(NAME Platforms Win32 Win32XP Linux32 Linux64)
bff_add_enabled_list(NAME Configs Debug Release)
bff_add_enabled_list(NAME Traits Default SVKSM Conferendo Terminal)
set(local_bff "${local_bff}\n")

bff_add_component(Boost)
bff_add_component(OPENSSL)
bff_add_component(WEBSOCKETPP)
bff_add_component(NAME googletest GTEST GMOCK)
bff_add_component(NAME libcurl CURL)

# Write webrtc_dir
set(local_bff "${local_bff}.webrtc_dir_MSVC120 = '${WITH_WEBRTC_PEERCONNECTION_MSVC120}'\n")
set(local_bff "${local_bff}.webrtc_dir_MSVC140 = '${WITH_WEBRTC_PEERCONNECTION_MSVC140}'\n")

file(WRITE "${CMAKE_BINARY_DIR}/local.bff" "${local_bff}")

message(STATUS "\n\t+--------------------------------+\n\t| local.bff created successfully |\n\t+--------------------------------+\n")
message(STATUS "Ignore the following error, its purpose is to prevent generator from running and generating a bunch of crap we don't want.")
message(FATAL_ERROR)

# vim: tw=0
