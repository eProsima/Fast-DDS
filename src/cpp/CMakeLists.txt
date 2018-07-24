# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include(${PROJECT_SOURCE_DIR}/cmake/dev/set_sources.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/dev/generate_msvc_libraries.cmake)

# Set source files
set(${PROJECT_NAME}_source_files
    log/Log.cpp
    log/StdoutConsumer.cpp
    utils/eClock.cpp
    utils/IPFinder.cpp
    utils/md5.cpp
    utils/StringMatching.cpp
    rtps/resources/ResourceEvent.cpp
    rtps/resources/TimedEvent.cpp
    rtps/resources/TimedEventImpl.cpp
    rtps/resources/AsyncWriterThread.cpp
    rtps/resources/AsyncInterestTree.cpp
    rtps/Endpoint.cpp
    rtps/writer/RTPSWriter.cpp
    rtps/writer/StatefulWriter.cpp
    rtps/writer/ReaderProxy.cpp
    rtps/writer/StatelessWriter.cpp
    rtps/writer/ReaderLocator.cpp
    rtps/writer/timedevent/PeriodicHeartbeat.cpp
    rtps/writer/timedevent/NackResponseDelay.cpp
    rtps/writer/timedevent/NackSupressionDuration.cpp
    rtps/history/CacheChangePool.cpp
    rtps/history/History.cpp
    rtps/history/WriterHistory.cpp
    rtps/history/ReaderHistory.cpp
    rtps/reader/timedevent/HeartbeatResponseDelay.cpp
    rtps/reader/timedevent/WriterProxyLiveliness.cpp
    rtps/reader/timedevent/InitialAckNack.cpp
    rtps/reader/CompoundReaderListener.cpp
    rtps/reader/WriterProxy.cpp
    rtps/reader/StatefulReader.cpp
    rtps/reader/StatelessReader.cpp
    rtps/reader/RTPSReader.cpp
    rtps/reader/FragmentedChangePitStop.cpp
    rtps/messages/CDRMessagePool.cpp
    rtps/messages/RTPSMessageCreator.cpp
    rtps/messages/RTPSMessageGroup.cpp
    rtps/messages/MessageReceiver.cpp
    rtps/messages/submessages/AckNackMsg.hpp
    rtps/messages/submessages/DataMsg.hpp
    rtps/messages/submessages/GapMsg.hpp
    rtps/messages/submessages/HeartbeatMsg.hpp
    rtps/network/NetworkFactory.cpp
    rtps/network/SenderResource.cpp
    rtps/network/ReceiverResource.cpp
    rtps/participant/RTPSParticipant.cpp
    rtps/participant/RTPSParticipantImpl.cpp
    rtps/RTPSDomain.cpp
    Domain.cpp
    participant/Participant.cpp
    participant/ParticipantImpl.cpp
    publisher/Publisher.cpp
    publisher/PublisherImpl.cpp
    publisher/PublisherHistory.cpp
    subscriber/Subscriber.cpp
    subscriber/SubscriberImpl.cpp
    subscriber/SubscriberHistory.cpp
    transport/UDPv4Transport.cpp
    transport/UDPv6Transport.cpp
    transport/test_UDPv4Transport.cpp
    qos/ParameterList.cpp
    qos/ParameterTypes.cpp
    qos/QosList.cpp
    qos/QosPolicies.cpp
    qos/WriterQos.cpp
    qos/ReaderQos.cpp
    rtps/builtin/BuiltinProtocols.cpp
    rtps/builtin/discovery/participant/PDPSimple.cpp
    rtps/builtin/discovery/participant/PDPSimpleListener.cpp
    rtps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.cpp
    rtps/builtin/discovery/participant/timedevent/ResendParticipantProxyDataPeriod.cpp
    rtps/builtin/discovery/endpoint/EDP.cpp
    rtps/builtin/discovery/endpoint/EDPSimple.cpp
    rtps/builtin/discovery/endpoint/EDPSimpleListeners.cpp
    rtps/builtin/discovery/endpoint/EDPStatic.cpp
    rtps/builtin/liveliness/WLP.cpp
    rtps/builtin/liveliness/WLPListener.cpp
    rtps/builtin/liveliness/timedevent/WLivelinessPeriodicAssertion.cpp
    rtps/builtin/data/ParticipantProxyData.cpp
    rtps/builtin/data/WriterProxyData.cpp
    rtps/builtin/data/ReaderProxyData.cpp
    rtps/flowcontrol/ThroughputController.cpp
    rtps/flowcontrol/ThroughputControllerDescriptor.cpp
    rtps/flowcontrol/FlowController.cpp
    rtps/exceptions/Exception.cpp
    rtps/attributes/PropertyPolicy.cpp
    rtps/common/Token.cpp
    xmlparser/XMLParserCommon.cpp
    xmlparser/XMLElementParser.cpp
    xmlparser/XMLEndpointParser.cpp
    xmlparser/XMLParser.cpp
    xmlparser/XMLProfileManager.cpp
    rtps/writer/PersistentWriter.cpp
    rtps/writer/StatelessPersistentWriter.cpp
    rtps/writer/StatefulPersistentWriter.cpp
    rtps/reader/StatelessPersistentReader.cpp
    rtps/reader/StatefulPersistentReader.cpp
    rtps/persistence/PersistenceFactory.cpp
    rtps/persistence/SQLite3PersistenceService.cpp
    rtps/persistence/sqlite3.c
    )

# Add sources to Makefile.am
set_sources(${${PROJECT_NAME}_source_files})


# Security sources
set(${PROJECT_NAME}_security_source_files
    rtps/security/exceptions/SecurityException.cpp
    rtps/security/common/SharedSecretHandle.cpp
    rtps/security/SecurityManager.cpp
    rtps/security/SecurityPluginFactory.cpp
    rtps/security/timedevent/HandshakeMessageTokenResent.cpp
    security/OpenSSLInit.cpp
    security/authentication/PKIDH.cpp
    security/accesscontrol/Permissions.cpp
    security/cryptography/AESGCMGMAC.cpp
    security/cryptography/AESGCMGMAC_KeyExchange.cpp
    security/cryptography/AESGCMGMAC_KeyFactory.cpp
    security/cryptography/AESGCMGMAC_Transform.cpp
    security/cryptography/AESGCMGMAC_Types.cpp
    security/authentication/PKIIdentityHandle.cpp
    security/authentication/PKIHandshakeHandle.cpp
    security/accesscontrol/AccessPermissionsHandle.cpp
    security/accesscontrol/CommonParser.cpp
    security/accesscontrol/GovernanceParser.cpp
    security/accesscontrol/PermissionsParser.cpp
    )

# Add sources to Makefile.am
set_sources(SECTION SECURITY)
set_sources(${${PROJECT_NAME}_security_source_files})
set_sources(ENDSECTION)

if(SECURITY)

    list(APPEND ${PROJECT_NAME}_source_files
        ${${PROJECT_NAME}_security_source_files}
        )

    include_directories(${OPENSSL_INCLUDE_DIR})
    set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES} ${OPENSSL_LIBRARIES})

    set(HAVE_SECURITY 1)
else()
    set(HAVE_SECURITY 0)
endif()

if(WIN32)
    list(APPEND ${PROJECT_NAME}_source_files
        ${PROJECT_SOURCE_DIR}/src/cpp/fastrtps.rc
        )
endif()

# External sources
if(TINYXML2_SOURCE_DIR)
    list(APPEND ${PROJECT_NAME}_source_files
        ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
        )
    set_sources(${TINYXML2_SOURCE_DIR}/tinyxml2.cpp)
endif()

if(ANDROID)
    list(APPEND ${PROJECT_NAME}_source_files
        ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
        )
endif()

configure_file(${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/config.h.in
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}/config.h)

# Definitions
# If Windows, set define for export dll functions
add_definitions(-D${PROJECT_NAME_UPPER}_SOURCE)
if(WIN32)
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
        add_definitions(-D_WIN32_WINNT=0x0603 -DSQLITE_OS_WINRT)
    else()
        add_definitions(-D_WIN32_WINNT=0x0601)
    endif()
endif()
# If INTERNAL_DEBUG = ON, define symbol for extensive debug messages
if(INTERNAL_DEBUG)
    add_definitions(-D__INTERNALDEBUG)
endif()

add_definitions(
    -DBOOST_ASIO_STANDALONE
    -DASIO_STANDALONE
    )

if(ANDROID)
    if(NOT HAVE_CXX14 AND NOT HAVE_CXX1Y)
        add_definitions(-DASIO_DISABLE_STD_STRING_VIEW)
    endif()
endif()

set(${PROJECT_NAME}_include_dirs
    ${ASIO_INCLUDE_DIR}
    ${TINYXML2_INCLUDE_DIR}
)

if(ANDROID)
    list(APPEND ${PROJECT_NAME}_include_dirs ${ANDROID_IFADDRS_INCLUDE_DIR})
else()
    find_package(Threads REQUIRED)
endif()

if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()

include_directories(${${PROJECT_NAME}_include_dirs})

# Create libraries
if(MSVC OR MSVC_IDE)
    if(EPROSIMA_INSTALLER)
        generate_msvc_libraries(i86Win32VS2013)
        generate_msvc_libraries(x64Win64VS2013)
        generate_msvc_libraries(i86Win32VS2015)
        generate_msvc_libraries(x64Win64VS2015)
    elseif(NOT EPROSIMA_INSTALLER)
        add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_source_files})

        add_library(${PROJECT_NAME}_static STATIC ${${PROJECT_NAME}_source_files})
        # Rename the library to have a "lib" before.
        set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME lib${PROJECT_NAME})

        set_target_properties(${PROJECT_NAME} ${PROJECT_NAME}_static PROPERTIES RELEASE_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
        set_target_properties(${PROJECT_NAME} ${PROJECT_NAME}_static PROPERTIES RELWITHDEBINFO_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
        set_target_properties(${PROJECT_NAME} ${PROJECT_NAME}_static PROPERTIES DEBUG_POSTFIX d-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})

        if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
            set_target_properties(${PROJECT_NAME} ${PROJECT_NAME}_static PROPERTIES VS_WINRT_COMPONENT "true")
        endif()

        # Export symbols in DLL library
        target_compile_definitions(${PROJECT_NAME} PUBLIC ${PROJECT_NAME_UPPER}_DYN_LINK)

        # Define public headers
        target_include_directories(${PROJECT_NAME} PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/cpp>
            $<INSTALL_INTERFACE:include>
            )

        # Define public headers for static library
        target_include_directories(${PROJECT_NAME}_static PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/cpp>
            $<INSTALL_INTERFACE:include>
            )

        # Made linked libraries PRIVATE to prevent local directories in Windows installer.
        set(PRIVACY "")
        if(EPROSIMA_INSTALLER_MINION)
            set(PRIVACY "PRIVATE")
        endif()

        # Link library to external win32 libraries.
        target_link_libraries(${PROJECT_NAME} ${PRIVACY} fastcdr
            ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS}
            iphlpapi Shlwapi kernel32 user32.dll
            ${EXTRA_LIBRARIES}
            )

        target_link_libraries(${PROJECT_NAME}_static ${PRIVACY} fastcdr)

        if(TINYXML2_LIBRARY)
            target_link_libraries(${PROJECT_NAME} ${PRIVACY}
                ${TINYXML2_LIBRARY}
                )
        endif()
    endif()
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-D__DEBUG)
    endif()
    #Create library
    add_library(${PROJECT_NAME} ${${PROJECT_NAME}_source_files})
    set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
    set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

    # Define public headers
    target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/cpp>
        $<INSTALL_INTERFACE:include>
        )

    # Link library to external libraries.
    target_link_libraries(${PROJECT_NAME} fastcdr
        ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS}
        ${EXTRA_LIBRARIES}
        )

    if(TINYXML2_LIBRARY)
        target_link_libraries(${PROJECT_NAME}
            ${TINYXML2_LIBRARY}
            )
    endif()
endif()

###############################################################################
# Packaging
###############################################################################

# Install public headers
set_public_headers_directory(${PROJECT_SOURCE_DIR}/include ${PROJECT_NAME}
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT headers
    INSTALL
    )

# Install config.h header
set_public_header(${PROJECT_BINARY_DIR}/include ${PROJECT_NAME} config.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}
    COMPONENT headers
    INSTALL
    )


# Install external sources
if(UNIX AND EPROSIMA_INSTALLER)
    set_public_header(${TINYXML2_INCLUDE_DIR} "" tinyxml2.h
        DESTINATION thirdparty/tinyxml2
        COMPONENT headers
        )

    set_public_headers_directory(${ASIO_INCLUDE_DIR} ""
        DESTINATION thirdparty/asio
        COMPONENT headers
        )
endif()

set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers" PARENT_SCOPE)
set(CPACK_COMPONENT_HEADERS_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} C++ Headers" PARENT_SCOPE)

# Install sources
if(UNIX AND EPROSIMA_INSTALLER)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/cpp
        DESTINATION src
        COMPONENT sources
        FILES_MATCHING
        PATTERN "*.c"
        PATTERN "*.cpp"
        PATTERN "*.h"
        PATTERN "*.hpp"
        )

    # Install external sources
    install(FILES ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
        ${TINYXML2_SOURCE_DIR}/tinyxml2.h
        DESTINATION thirdparty/tinyxml2
        COMPONENT sources
        )

    install(DIRECTORY ${PROJECT_BINARY_DIR}/autotools/
        DESTINATION .
        USE_SOURCE_PERMISSIONS
        OPTIONAL
        COMPONENT sources
        )
endif()

# Install libraries
if(MSVC OR MSVC_IDE)
    if(EPROSIMA_INSTALLER AND IS_TOP_LEVEL)
        install_msvc_libraries(i86Win32VS2013)
        install_msvc_libraries(x64Win64VS2013)
        install_msvc_libraries(i86Win32VS2015)
        install_msvc_libraries(x64Win64VS2015)
    elseif(NOT EPROSIMA_INSTALLER)
        set(DIR_EXTENSION "")
        if(EPROSIMA_INSTALLER_MINION)
            set(DIR_EXTENSION "/${MSVC_ARCH}")
        endif()

        install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_static
            EXPORT ${PROJECT_NAME}Targets
            RUNTIME DESTINATION ${BIN_INSTALL_DIR}${DIR_EXTENSION}
            LIBRARY DESTINATION ${LIB_INSTALL_DIR}${DIR_EXTENSION}
            ARCHIVE DESTINATION ${LIB_INSTALL_DIR}${DIR_EXTENSION}
            COMPONENT libraries_${MSVC_ARCH}
            )

        export(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_static FILE ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}Targets.cmake)
        install(EXPORT ${PROJECT_NAME}Targets
            DESTINATION ${LIB_INSTALL_DIR}${DIR_EXTENSION}/${PROJECT_NAME}/cmake
            COMPONENT cmake
            )


        # Install pdb files
        install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug/
            DESTINATION ${LIB_INSTALL_DIR}${DIR_EXTENSION}
            COMPONENT libraries_${MSVC_ARCH}
            CONFIGURATIONS Debug
            FILES_MATCHING
            PATTERN "*.pdb"
            )

        if(NOT (("${MSVC_ARCH}" STREQUAL "i86Win32VS2013") OR ("${MSVC_ARCH}" STREQUAL "x64Win64VS2013")))
            install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_static.dir/Debug/${PROJECT_NAME}_static.pdb
                DESTINATION ${LIB_INSTALL_DIR}${DIR_EXTENSION}
                COMPONENT libraries_${MSVC_ARCH}
                CONFIGURATIONS Debug
                )
        endif()

        if(EPROSIMA_INSTALLER_MINION AND SECURITY)
            install(FILES ${OPENSSL_ROOT_DIR}/lib/libeay32.dll
                DESTINATION ${BIN_INSTALL_DIR}${DIR_EXTENSION}
                COMPONENT libraries_${MSVC_ARCH}
                )

            install(FILES ${OPENSSL_ROOT_DIR}/lib/VC/static/libeay32MD.lib
                ${OPENSSL_ROOT_DIR}/lib/VC/static/libeay32MDd.lib
                DESTINATION ${LIB_INSTALL_DIR}${DIR_EXTENSION}
                COMPONENT libraries_${MSVC_ARCH}
                )
        endif()


        string(TOUPPER "${MSVC_ARCH}" MSVC_ARCH_UPPER)
        set(CPACK_COMPONENT_LIBRARIES_${MSVC_ARCH_UPPER}_DISPLAY_NAME "${MSVC_ARCH}" PARENT_SCOPE)
        set(CPACK_COMPONENT_LIBRARIES_${MSVC_ARCH_UPPER}_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} libraries for platform ${MSVC_ARCH}" PARENT_SCOPE)
        set(CPACK_COMPONENT_LIBRARIES_${MSVC_ARCH_UPPER}_GROUP "Libraries" PARENT_SCOPE)
    endif()

    set(CPACK_COMPONENT_GROUP_LIBRARIES_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} libraries" PARENT_SCOPE)
else()
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
        COMPONENT libraries
        )

    export(TARGETS ${PROJECT_NAME} FILE ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}Targets.cmake)
    install(EXPORT ${PROJECT_NAME}Targets
        DESTINATION ${LIB_INSTALL_DIR}/${PROJECT_NAME}/cmake
        COMPONENT cmake
        )

    set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries" PARENT_SCOPE)
    set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} libraries" PARENT_SCOPE)
endif()
