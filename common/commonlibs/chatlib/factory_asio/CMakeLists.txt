SET(UNIT_NAME "chat_factory_asio")
INCLUDE_DIRECTORIES(
${INCLUDEDIRS}
${BOOST_DIR}
)
SET(${UNIT_NAME}_SRCS
make_layers_asio.cpp
)
SET(${UNIT_NAME}_HDRS
make_layers_asio.h
)

ADD_LIBRARY(
    ${UNIT_NAME}
    STATIC
    ${${UNIT_NAME}_SRCS}
    ${${UNIT_NAME}_HDRS}
)

target_compile_definitions(${UNIT_NAME} PRIVATE -DBOOST_ASIO_SEPARATE_COMPILATION)
