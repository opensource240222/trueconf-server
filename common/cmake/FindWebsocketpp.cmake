# FindWebsocketpp
#
# Searches for websocketpp headers, will define:
#	 WEBSOCKETPP_FOUND       - True if websocketpp is found
#    WEBSOCKETPP_INCLUDE_DIR - Path to header files

find_path(WEBSOCKETPP_INCLUDE_DIR NAMES "websocketpp/version.hpp")

if(WEBSOCKETPP_INCLUDE_DIR AND EXISTS "${WEBSOCKETPP_INCLUDE_DIR}/version.hpp")
    file(STRINGS "${websocketpp_INCLUDE_DIR}/version.hpp" websocketpp_version_lines
		REGEX "(major|minor|patch)_version[\t ]*=[\t ]*[0-9]+;$")

    unset(WEBSOCKETPP_VERSION_STRING)
    foreach(part major minor patch)
        foreach(line ${websocketpp_version_lines})
            if(line MATCHES "${part}_version[\t ]*=[\t ]*([0-9]+);$")
				list(APPEND WEBSOCKETPP_VERSION_STRING "${CMAKE_MATCH_1}")
            endif()
        endforeach()
    endforeach()
	string(REPLACE WEBSOCKETPP_VERSION_STRING ";" ".")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WEBSOCKETPP
	REQUIRED_VARS WEBSOCKETPP_INCLUDE_DIR
	VERSION_VAR WEBSOCKETPP_VERSION_STRING
	)
