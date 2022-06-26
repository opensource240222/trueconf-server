SET(UNIT_NAME "mdnslib")

SET(${UNIT_NAME}_SRCS
Header.cpp
Query.cpp
RRecord.cpp
Parser.cpp
Former.cpp
tools.cpp
)
SET(${UNIT_NAME}_HDRS
Header.h
Query.h
RRecord.h
Parser.h
Former.h
tools.h
parser_types.h
addons/SRV.h
addons/TXT.h
)
ADD_LIBRARY(
    ${UNIT_NAME}
    STATIC
    ${${UNIT_NAME}_SRCS}
    ${${UNIT_NAME}_HDRS}
)

SET( INCLUDEDIRS
${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(${UNIT_NAME} PRIVATE ${INCLUDEDIRS})
