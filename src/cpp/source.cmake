set(${PROJECT_NAME}_module_dirs
    fastdds/log
    fastdds/xtypes
    fastdds/builtin
    utils
    rtps
    fastdds/core
    fastdds/domain
    fastdds/publisher
    fastdds/subscriber
    fastdds/topic
    statistics
    security
    xmlparser
    )

set(${PROJECT_NAME}_source_files
    ${ALL_HEADERS}

    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/BuiltinProtocols.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/database/backup/SharedBackupFunctions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/database/DiscoveryDataBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/database/DiscoveryParticipantInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/database/DiscoveryParticipantsAckStatus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/database/DiscoverySharedInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPServerListeners.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPSimple.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPSimpleListeners.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/endpoint/EDPStatic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/DirectMessageSender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPClientListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPServerListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/PDPSimple.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/simple/PDPStatelessWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/timedevent/DSClientEvent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/timedevent/DServerEvent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/liveliness/WLPListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/DataSharing/DataSharingListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/DataSharing/DataSharingNotification.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/SendBuffersManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/submessages/AckNackMsg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/submessages/DataMsg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/submessages/GapMsg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/messages/submessages/HeartbeatMsg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/reader/reader_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/reader/StatefulPersistentReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/reader/StatelessPersistentReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/reader/WriterProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/test_UDPv4Transport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/writer/PersistentWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/writer/StatefulPersistentWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/writer/StatelessPersistentWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xmlparser/attributes/TopicAttributes.cpp
    )

# SHM Transport
if(IS_THIRDPARTY_BOOST_OK)
    list(APPEND ${PROJECT_NAME}_source_files
        ${CMAKE_CURRENT_SOURCE_DIR}/rtps/transport/shared_mem/test_SharedMemTransport.cpp
        )
endif()

# Security sources
set(${PROJECT_NAME}_security_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/security/SecurityPluginFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtps/builtin/discovery/participant/DS/PDPSecurityInitiatorListener.cpp
    )

if(SECURITY)
    list(APPEND ${PROJECT_NAME}_source_files
        ${${PROJECT_NAME}_security_source_files}
        )
    set(HAVE_SECURITY 1)
else()
    set(HAVE_SECURITY 0)
endif()

if(WIN32 AND (MSVC OR MSVC_IDE))
    list(APPEND ${PROJECT_NAME}_source_files
        ${CMAKE_CURRENT_SOURCE_DIR}/fastdds.rc
        )
endif()


if(SQLITE3_SUPPORT)
    set(HAVE_SQLITE3 1)
    # sqlite.c requires a C compiler
    enable_language(C)
else()
    set(HAVE_SQLITE3 0)
endif()


# External sources
if(ANDROID)
    if((ANDROID_PLATFORM LESS_EQUAL 23) OR (ANDROID_NATIVE_API_LEVEL LESS_EQUAL 23))
        list(APPEND ${PROJECT_NAME}_source_files
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()
