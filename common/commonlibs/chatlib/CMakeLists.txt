SET(UNIT_NAME "chatlib")

SET(${UNIT_NAME}_SRCS
chain/adjust_msg_order.cpp
chain/BucketsOfMessages.cpp
chain/ChainContainer.cpp
chain/ChainOfMessages.cpp
helpers/AccountInfo.cpp
layers/ChatLayerAbstract.cpp
layers/SyncContext.cpp
msg/attr.cpp
msg/chat_messages_construct.cpp
msg/ChatMessage.cpp
msg/parse_message.cpp
msg/parse_message_content.cpp
notify/ChatEventsFuncs.cpp
storage/ChatStorage.cpp
storage/DBProcWrap.cpp
storage/make_chat_storage.cpp
utils/chat_utils.cpp
utils/msg_utils.cpp
)
SET(${UNIT_NAME}_HDRS
callbacks.h
chain/adjust_msg_order.h
chain/BucketsOfMessages.h
chain/ChainContainer.h
chain/ChainOfMessages.h
chat_defs.h
chatinfo/info_types.h
helpers/AccountInfo.h
helpers/ExternalComponentsInterface.h
helpers/ResolverInterface.h
helpers/vs_def.h
interface/GlobalConfigInterface.h
interface/SyncChatInterface.h
interface/TransportChannel.h
layers/AppLayer.h
layers/AppLayerImpl.h
layers/ChatLayerAbstract.h
layers/DeliveryLayer.h
layers/DeliveryLayerImpl.h
layers/IntegrityLayer.h
layers/IntegrityLayerImpl.h
layers/MainStorageLayer.h
layers/MainStorageLayerImpl.h
layers/SyncContext.h
layers/SystemChatLayer.h
layers/SystemChatLayerImpl.h
layers/TransportLayer.h
layers/TransportLayerImpl.h
messages_types.h
msg_content_types.h
msg/attr.h
msg/chat_messages_construct.h
msg/ChatMessage.h
msg/parse_message.h
msg/parse_message_content.h
notify/ChatEventsFuncs.h
notify/GlobalChatEvents.h
notify/notify_cb.h
storage/ChatStorage.h
storage/DBProcWrap.h
storage/helpers.h
storage/make_chat_storage.h
storage/PostgreSQLProcDecl.h
storage/SQLITEProcDecl.h
utils/chat_utils.h
utils/msg_utils.h
log/chatlog.h
)
ADD_LIBRARY(
    ${UNIT_NAME}
    STATIC
    ${${UNIT_NAME}_SRCS}
    ${${UNIT_NAME}_HDRS}
)

SET( INCLUDEDIRS
${CMAKE_CURRENT_SOURCE_DIR}/..
${CMAKE_CURRENT_SOURCE_DIR}/../..
${CLIENTLIB_EXTLIBS_DIR}/include
${CLIENTLIB_EXTLIBS_DIR}/rapidjson/include
${CLIENTLIB_EXTLIBS_DIR}/cppdb/include
${CLIENTLIB_EXTLIBS_DIR}/sqlite3/Include
)

target_include_directories(${UNIT_NAME} PRIVATE ${INCLUDEDIRS})
target_compile_definitions(${UNIT_NAME} PRIVATE CHAT_LOG_VERBOSE)
