# Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

include(CheckCXXSourceCompiles)

add_subdirectory(fastdds/log)
add_subdirectory(fastdds/xtypes)

# Set source files
if(WIN32)

file(GLOB_RECURSE ALL_HEADERS
    ${PROJECT_SOURCE_DIR}/include/**/*.h
    ${PROJECT_SOURCE_DIR}/include/**/*.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/**/*.h
    ${PROJECT_SOURCE_DIR}/src/cpp/**/*.hpp
    )

add_definitions(-DNOMINMAX)

endif(WIN32)

file(GLOB DDSSQLFILTER_SOURCES fastdds/topic/DDSSQLFilter/*.cpp)


set(${PROJECT_NAME}_source_files
    ${ALL_HEADERS}

    ${DDSSQLFILTER_SOURCES}

    fastdds/builtin/type_lookup_service/detail/rpc_typesPubSubTypes.cxx
    fastdds/builtin/type_lookup_service/detail/TypeLookupTypesPubSubTypes.cxx
    fastdds/builtin/type_lookup_service/TypeLookupManager.cpp
    fastdds/builtin/type_lookup_service/TypeLookupRequestListener.cpp
    fastdds/builtin/type_lookup_service/TypeLookupReplyListener.cpp
    fastdds/core/condition/Condition.cpp
    fastdds/core/condition/ConditionNotifier.cpp
    fastdds/core/condition/GuardCondition.cpp
    fastdds/core/condition/StatusCondition.cpp
    fastdds/core/condition/StatusConditionImpl.cpp
    fastdds/core/condition/WaitSet.cpp
    fastdds/core/condition/WaitSetImpl.cpp
    fastdds/core/Entity.cpp
    fastdds/core/policy/ParameterList.cpp
    fastdds/core/policy/QosPolicyUtils.cpp
    fastdds/core/Time_t.cpp
    fastdds/domain/DomainParticipant.cpp
    fastdds/domain/DomainParticipantFactory.cpp
    fastdds/domain/DomainParticipantImpl.cpp
    fastdds/domain/qos/DomainParticipantFactoryQos.cpp
    fastdds/domain/qos/DomainParticipantQos.cpp
    fastdds/log/FileConsumer.cpp
    fastdds/log/Log.cpp
    fastdds/log/OStreamConsumer.cpp
    fastdds/log/StdoutConsumer.cpp
    fastdds/log/StdoutErrConsumer.cpp
    fastdds/publisher/DataWriter.cpp
    fastdds/publisher/DataWriterHistory.cpp
    fastdds/publisher/DataWriterImpl.cpp
    fastdds/publisher/Publisher.cpp
    fastdds/publisher/PublisherImpl.cpp
    fastdds/publisher/qos/DataWriterQos.cpp
    fastdds/publisher/qos/PublisherQos.cpp
    fastdds/publisher/qos/WriterQos.cpp
    fastdds/rpc/ServiceImpl.cpp
    fastdds/rpc/ReplierImpl.cpp
    fastdds/rpc/RequesterImpl.cpp
    fastdds/rpc/ServiceTypeSupport.cpp
    fastdds/subscriber/DataReader.cpp
    fastdds/subscriber/DataReaderImpl.cpp
    fastdds/subscriber/history/DataReaderHistory.cpp
    fastdds/subscriber/qos/DataReaderQos.cpp
    fastdds/subscriber/qos/ReaderQos.cpp
    fastdds/subscriber/qos/SubscriberQos.cpp
    fastdds/subscriber/ReadCondition.cpp
    fastdds/subscriber/Subscriber.cpp
    fastdds/subscriber/SubscriberImpl.cpp
    fastdds/topic/ContentFilteredTopic.cpp
    fastdds/topic/ContentFilteredTopicImpl.cpp
    fastdds/topic/qos/TopicQos.cpp
    fastdds/topic/Topic.cpp
    fastdds/topic/TopicImpl.cpp
    fastdds/topic/TopicProxyFactory.cpp
    fastdds/topic/TypeSupport.cpp
    fastdds/utils/QosConverters.cpp
    fastdds/utils/TypePropagation.cpp
    fastdds/xtypes/dynamic_types/AnnotationDescriptorImpl.cpp
    fastdds/xtypes/dynamic_types/DynamicDataFactory.cpp
    fastdds/xtypes/dynamic_types/DynamicDataImpl.cpp
    fastdds/xtypes/dynamic_types/DynamicDataFactoryImpl.cpp
    fastdds/xtypes/dynamic_types/DynamicPubSubType.cpp
    fastdds/xtypes/dynamic_types/DynamicTypeImpl.cpp
    fastdds/xtypes/dynamic_types/DynamicTypeBuilderFactory.cpp
    fastdds/xtypes/dynamic_types/DynamicTypeBuilderImpl.cpp
    fastdds/xtypes/dynamic_types/DynamicTypeBuilderFactoryImpl.cpp
    fastdds/xtypes/dynamic_types/DynamicTypeMemberImpl.cpp
    fastdds/xtypes/dynamic_types/MemberDescriptorImpl.cpp
    fastdds/xtypes/dynamic_types/TypeDescriptorImpl.cpp
    fastdds/xtypes/dynamic_types/VerbatimTextDescriptorImpl.cpp
    fastdds/xtypes/exception/Exception.cpp
    fastdds/xtypes/serializers/idl/dynamic_type_idl.cpp
    fastdds/xtypes/serializers/json/dynamic_data_json.cpp
    fastdds/xtypes/type_representation/dds_xtypes_typeobjectPubSubTypes.cxx
    fastdds/xtypes/type_representation/TypeObjectRegistry.cpp
    fastdds/xtypes/type_representation/TypeObjectUtils.cpp
    fastdds/xtypes/utils.cpp
    rtps/attributes/EndpointSecurityAttributes.cpp
    rtps/attributes/PropertyPolicy.cpp
    rtps/attributes/RTPSParticipantAttributes.cpp
    rtps/attributes/ServerAttributes.cpp
    rtps/attributes/ThreadSettings.cpp
    rtps/builtin/BuiltinProtocols.cpp
    rtps/builtin/data/ParticipantBuiltinTopicData.cpp
    rtps/builtin/data/ParticipantProxyData.cpp
    rtps/builtin/data/PublicationBuiltinTopicData.cpp
    rtps/builtin/data/SubscriptionBuiltinTopicData.cpp
    rtps/builtin/data/ReaderProxyData.cpp
    rtps/builtin/data/WriterProxyData.cpp
    rtps/builtin/discovery/database/backup/SharedBackupFunctions.cpp
    rtps/builtin/discovery/database/DiscoveryDataBase.cpp
    rtps/builtin/discovery/database/DiscoveryParticipantInfo.cpp
    rtps/builtin/discovery/database/DiscoveryParticipantsAckStatus.cpp
    rtps/builtin/discovery/database/DiscoverySharedInfo.cpp
    rtps/builtin/discovery/endpoint/EDP.cpp
    rtps/builtin/discovery/endpoint/EDPClient.cpp
    rtps/builtin/discovery/endpoint/EDPServer.cpp
    rtps/builtin/discovery/endpoint/EDPServerListeners.cpp
    rtps/builtin/discovery/endpoint/EDPSimple.cpp
    rtps/builtin/discovery/endpoint/EDPSimpleListeners.cpp
    rtps/builtin/discovery/endpoint/EDPStatic.cpp
    rtps/builtin/discovery/participant/DirectMessageSender.cpp
    rtps/builtin/discovery/participant/PDP.cpp
    rtps/builtin/discovery/participant/PDPClient.cpp
    rtps/builtin/discovery/participant/PDPClientListener.cpp
    rtps/builtin/discovery/participant/PDPListener.cpp
    rtps/builtin/discovery/participant/PDPServer.cpp
    rtps/builtin/discovery/participant/PDPServerListener.cpp
    rtps/builtin/discovery/participant/PDPSimple.cpp
    rtps/builtin/discovery/participant/simple/PDPStatelessWriter.cpp
    rtps/builtin/discovery/participant/timedevent/DSClientEvent.cpp
    rtps/builtin/discovery/participant/timedevent/DServerEvent.cpp
    rtps/builtin/liveliness/WLP.cpp
    rtps/builtin/liveliness/WLPListener.cpp
    rtps/common/GuidPrefix_t.cpp
    rtps/common/SerializedPayload.cpp
    rtps/common/LocatorWithMask.cpp
    rtps/common/Time_t.cpp
    rtps/common/Token.cpp
    rtps/DataSharing/DataSharingListener.cpp
    rtps/DataSharing/DataSharingNotification.cpp
    rtps/DataSharing/DataSharingPayloadPool.cpp
    rtps/exceptions/Exception.cpp
    rtps/flowcontrol/FlowControllerConsts.cpp
    rtps/flowcontrol/FlowControllerFactory.cpp
    rtps/history/CacheChangePool.cpp
    rtps/history/History.cpp
    rtps/history/ReaderHistory.cpp
    rtps/history/TopicPayloadPool.cpp
    rtps/history/TopicPayloadPoolRegistry.cpp
    rtps/history/WriterHistory.cpp
    rtps/messages/CDRMessage.cpp
    rtps/messages/MessageReceiver.cpp
    rtps/messages/RTPSGapBuilder.cpp
    rtps/messages/RTPSMessageCreator.cpp
    rtps/messages/RTPSMessageGroup.cpp
    rtps/messages/SendBuffersManager.cpp
    rtps/messages/submessages/AckNackMsg.hpp
    rtps/messages/submessages/DataMsg.hpp
    rtps/messages/submessages/GapMsg.hpp
    rtps/messages/submessages/HeartbeatMsg.hpp
    rtps/network/NetworkBuffer.cpp
    rtps/network/NetworkFactory.cpp
    rtps/network/ReceiverResource.cpp
    rtps/network/utils/external_locators.cpp
    rtps/network/utils/netmask_filter.cpp
    rtps/network/utils/network.cpp
    rtps/participant/RTPSParticipant.cpp
    rtps/participant/RTPSParticipantImpl.cpp
    rtps/persistence/PersistenceFactory.cpp
    rtps/reader/BaseReader.cpp
    rtps/reader/reader_utils.cpp
    rtps/reader/RTPSReader.cpp
    rtps/reader/StatefulPersistentReader.cpp
    rtps/reader/StatefulReader.cpp
    rtps/reader/StatelessPersistentReader.cpp
    rtps/reader/StatelessReader.cpp
    rtps/reader/WriterProxy.cpp
    rtps/resources/ResourceEvent.cpp
    rtps/resources/TimedEvent.cpp
    rtps/resources/TimedEventImpl.cpp
    rtps/RTPSDomain.cpp
    rtps/transport/ChainingTransport.cpp
    rtps/transport/ChannelResource.cpp
    rtps/transport/network/NetmaskFilterKind.cpp
    rtps/transport/network/NetworkInterface.cpp
    rtps/transport/network/NetworkInterfaceWithFilter.cpp
    rtps/transport/PortBasedTransportDescriptor.cpp
    rtps/transport/shared_mem/SharedMemTransportDescriptor.cpp
    rtps/transport/tcp/RTCPMessageManager.cpp
    rtps/transport/tcp/TCPControlMessage.cpp
    rtps/transport/TCPAcceptor.cpp
    rtps/transport/TCPAcceptorBasic.cpp
    rtps/transport/TCPChannelResource.cpp
    rtps/transport/TCPChannelResourceBasic.cpp
    rtps/transport/TCPTransportInterface.cpp
    rtps/transport/TCPv4Transport.cpp
    rtps/transport/TCPv6Transport.cpp
    rtps/transport/test_UDPv4Transport.cpp
    rtps/transport/TransportInterface.cpp
    rtps/transport/UDPChannelResource.cpp
    rtps/transport/UDPTransportInterface.cpp
    rtps/transport/UDPv4Transport.cpp
    rtps/transport/UDPv6Transport.cpp
    rtps/writer/BaseWriter.cpp
    rtps/writer/LivelinessManager.cpp
    rtps/writer/LocatorSelectorSender.cpp
    rtps/writer/PersistentWriter.cpp
    rtps/writer/ReaderLocator.cpp
    rtps/writer/ReaderProxy.cpp
    rtps/writer/RTPSWriter.cpp
    rtps/writer/StatefulPersistentWriter.cpp
    rtps/writer/StatefulWriter.cpp
    rtps/writer/StatelessPersistentWriter.cpp
    rtps/writer/StatelessWriter.cpp
    statistics/fastdds/domain/DomainParticipant.cpp
    statistics/fastdds/publisher/qos/DataWriterQos.cpp
    statistics/fastdds/subscriber/qos/DataReaderQos.cpp
    utils/Host.cpp
    utils/IPFinder.cpp
    utils/IPLocator.cpp
    utils/md5.cpp
    utils/StringMatching.cpp
    utils/SystemInfo.cpp
    utils/TimedConditionVariable.cpp
    utils/UnitsParser.cpp
    xmlparser/attributes/TopicAttributes.cpp
    xmlparser/XMLDynamicParser.cpp
    xmlparser/XMLElementParser.cpp
    xmlparser/XMLEndpointParser.cpp
    xmlparser/XMLParser.cpp
    xmlparser/XMLParserCommon.cpp
    xmlparser/XMLProfileManager.cpp
    )

# Statistics support
if (FASTDDS_STATISTICS)

    set(statistics_sources
        statistics/fastdds/domain/DomainParticipantImpl.cpp
        statistics/fastdds/domain/DomainParticipantStatisticsListener.cpp
        statistics/rtps/monitor-service/MonitorService.cpp
        statistics/rtps/monitor-service/MonitorServiceListener.cpp
        statistics/rtps/reader/StatisticsReaderImpl.cpp
        statistics/rtps/StatisticsBase.cpp
        statistics/rtps/writer/StatisticsWriterImpl.cpp
        statistics/types/monitorservice_typesPubSubTypes.cxx
        statistics/types/monitorservice_typesTypeObjectSupport.cxx
        statistics/types/typesPubSubTypes.cxx
        statistics/types/typesTypeObjectSupport.cxx
        )

    list(APPEND ${PROJECT_NAME}_source_files
        ${statistics_sources}
        )

endif()

# SHM Transport
if(IS_THIRDPARTY_BOOST_OK)
    list(APPEND ${PROJECT_NAME}_source_files
        rtps/transport/shared_mem/test_SharedMemTransport.cpp
        rtps/transport/shared_mem/SharedMemTransport.cpp
        )
endif()

# TLS Support
if(TLS_FOUND)
    list(APPEND ${PROJECT_NAME}_source_files
        rtps/transport/TCPChannelResourceSecure.cpp
        rtps/transport/TCPAcceptorSecure.cpp
        )
endif()

# Security sources
set(${PROJECT_NAME}_security_source_files
    rtps/security/exceptions/SecurityException.cpp
    rtps/security/common/SharedSecretHandle.cpp
    rtps/security/logging/Logging.cpp
    rtps/security/logging/LoggingLevel.cpp
    rtps/security/SecurityManager.cpp
    rtps/security/SecurityPluginFactory.cpp
    rtps/builtin/discovery/participant/DS/PDPSecurityInitiatorListener.cpp
    security/authentication/PKIDH.cpp
    security/accesscontrol/Permissions.cpp
    security/accesscontrol/DistinguishedName.cpp
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
    security/logging/LogTopic.cpp
    security/artifact_providers/FileProvider.cpp
    security/artifact_providers/Pkcs11Provider.cpp
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
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds.rc
        )
endif()


#SQLITE3 persistence service sources
set(${PROJECT_NAME}_sqlite3_source_files
    rtps/persistence/SQLite3PersistenceService.cpp
    rtps/persistence/sqlite3.c
    )

if(SQLITE3_SUPPORT)
    list(APPEND ${PROJECT_NAME}_source_files
        ${${PROJECT_NAME}_sqlite3_source_files}
        )
    set(HAVE_SQLITE3 1)
    # sqlite.c requires a C compiler
    enable_language(C)
else()
    set(HAVE_SQLITE3 0)
endif()


# External sources
if(TINYXML2_SOURCE_DIR)
    set(TINYXML2_SOURCE_DIR_ ${TINYXML2_SOURCE_DIR})
    list(APPEND ${PROJECT_NAME}_source_files
        ${TINYXML2_SOURCE_DIR_}/tinyxml2.cpp
        )
endif()

if(ANDROID)
    if((ANDROID_PLATFORM LESS_EQUAL 23) OR (ANDROID_NATIVE_API_LEVEL LESS_EQUAL 23))
        list(APPEND ${PROJECT_NAME}_source_files
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

# Option to enable strict real-time. In this case, several API functions have a real-time behaviour.
# * Publisher::write() - Uses ReliabilityQosPolicy.max_blocking_time
# * Subscriber::takeNextData() - Uses ReliabilityQosPolicy.max_blocking_time
# * Subscriber::readNextData() - Uses ReliabilityQosPolicy.max_blocking_time
option(STRICT_REALTIME "Enable a strict real-time behaviour." OFF)
if(STRICT_REALTIME)
    set(HAVE_STRICT_REALTIME 1)
else()
    set(HAVE_STRICT_REALTIME 0)
endif()

if(NOT ANDROID)
    find_package(Threads REQUIRED)
endif()

if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()

# Find out if libatomic link is required in this platform
find_package(Atomic MODULE)

# Check if the shared_mutex provided by the platform STL library
# prioritizes writes

# try_run cannot manage targets yet
get_target_property(CMAKE_ATOMIC_LIB eProsima_atomic INTERFACE_LINK_LIBRARIES)
if(NOT CMAKE_ATOMIC_LIB)
    set(CMAKE_ATOMIC_LIB)
endif()

try_run(SM_RUN_RESULT SM_COMPILE_RESULT
        "${PROJECT_BINARY_DIR}/shmtest"
        "${PROJECT_SOURCE_DIR}/cmake/modules/check_shared_mutex_priority.cpp"
        LINK_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_ATOMIC_LIB}
        RUN_OUTPUT_VARIABLE SM_RUN_OUTPUT)

if(SM_COMPILE_RESULT AND NOT SM_RUN_RESULT)
    string(STRIP ${SM_RUN_OUTPUT} SM_RUN_OUTPUT)
    message(STATUS "Framework's shared_mutex is ${SM_RUN_OUTPUT}")
endif()

if(SM_RUN_OUTPUT STREQUAL "PTHREAD_RWLOCK_PREFER_READER_NP")
    set(USER_CAN_CHOOSE_SHARED_MEMORY_THIRDPARTY ON)
else()
    message(STATUS "Forcing third party shared_mutex")
    set(USE_THIRDPARTY_SHARED_MUTEX ON)
endif()

cmake_dependent_option(
    USE_THIRDPARTY_SHARED_MUTEX [=[
Forces the use of a Boost-based shared_mutex implementation
instead of the framework one. Useful to cope with issues on
framework implementations like misguided sanitizer reports.
This implementation will be used by default on frameworks
lacking the shared_mutex feature like those not fulfilling
C++17.
]=] OFF
    "USER_CAN_CHOOSE_SHARED_MEMORY_THIRDPARTY"
    ON)

unset(USER_CAN_CHOOSE_SHARED_MEMORY_THIRDPARTY)
unset(SM_RUN_RESULT)
unset(SM_COMPILE_RESULT)
unset(SM_RUN_OUTPUT)
unset(CMAKE_ATOMIC_LIB)

# Generate the proper configure file
configure_file(${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/config.hpp.in
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}/config.hpp)

#Create library
add_library(${PROJECT_NAME} ${${PROJECT_NAME}_source_files})
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )

option(INTERNAL_DEBUG "Activate developer debug messages" OFF)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    ${PROJECT_NAME_UPPER}_SOURCE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    ASIO_DISABLE_VISIBILITY
    EPROSIMA_USER_DLL_EXPORT
    SQLITE_WIN32_GETVERSIONEX=0
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    $<$<AND:$<BOOL:${WIN32}>,$<STREQUAL:"${CMAKE_SYSTEM_NAME}","WindowsStore">>:_WIN32_WINNT=0x0603>
    $<$<AND:$<BOOL:${WIN32}>,$<NOT:$<STREQUAL:"${CMAKE_SYSTEM_NAME}","WindowsStore">>>:_WIN32_WINNT=0x0601>
    $<$<AND:$<BOOL:${WIN32}>,$<STREQUAL:"${CMAKE_SYSTEM_NAME}","WindowsStore">>:SQLITE_OS_WINRT>
    $<$<AND:$<BOOL:${ANDROID}>,$<NOT:$<BOOL:${HAVE_CXX14}>>,$<NOT:$<BOOL:${HAVE_CXX1Y}>>>:ASIO_DISABLE_STD_STRING_VIEW>
    $<$<BOOL:${WIN32}>:_ENABLE_ATOMIC_ALIGNMENT_FIX>
    $<$<NOT:$<BOOL:${IS_THIRDPARTY_BOOST_SUPPORTED}>>:FASTDDS_SHM_TRANSPORT_DISABLED> # Do not compile SHM Transport
    $<$<BOOL:${SHM_TRANSPORT_DEFAULT}>:SHM_TRANSPORT_BUILTIN> # Enable SHM as built-in transport
    $<$<BOOL:${STDOUTERR_LOG_CONSUMER}>:STDOUTERR_LOG_CONSUMER> # Enable StdoutErrConsumer as default LogConsumer
    INTERFACE
    $<$<BOOL:${WIN32}>:${PROJECT_NAME_UPPER}_NO_LIB>
    PUBLIC
    $<$<TARGET_EXISTS:${PROJECT_NAME}>:$<$<STREQUAL:$<TARGET_PROPERTY:${PROJECT_NAME},TYPE>,SHARED_LIBRARY>:${PROJECT_NAME_UPPER}_DYN_LINK>>
    )

# Define public headers
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/cpp>
    $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
    PRIVATE
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${TINYXML2_INCLUDE_DIR}>:${TINYXML2_INCLUDE_DIR}>
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    )

# No need to expose linked libs when target is a shared library on MSVC.
if(BUILD_SHARED_LIBS AND MSVC)
    set(PRIVACY "PRIVATE")
else()
    set(PRIVACY "PUBLIC")
endif()

# Link library to external libraries.
target_link_libraries(${PROJECT_NAME}
    ${PRIVACY}
    fastcdr
    foonathan_memory
    ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS}
    ${TINYXML2_LIBRARY}
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    ${THIRDPARTY_BOOST_LINK_LIBS}

    PRIVATE
    eProsima_atomic
    )

if (APPLE)
    target_link_libraries(${PROJECT_NAME}
    ${PRIVACY}
    "-framework CoreFoundation"
    "-framework IOKit"
    )
endif()

if(MSVC OR MSVC_IDE)
    # On installed binaries use manifest to specify dependencies
    if(INSTALLER_PLATFORM AND OPENSSL_FOUND)

        # Get Fast DDS version suitable manifest format
        set(FASTDDS_CANONICAL_VERSION ${fastdds_VERSION})

        if( NOT FASTDDS_CANONICAL_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
            message(FATAL_ERROR "Fast DDS version number ${fastdds_VERSION} must include major, minor and patch.")
        endif()

        if( NOT fastdds_VERSION_TWEAK)
            set(FASTDDS_CANONICAL_VERSION "${FASTDDS_CANONICAL_VERSION}.0")
        endif()

        # Get OpenSSL version suitable manifest format
        execute_process( COMMAND PowerShell -NoLogo -Command "&{ param([string]$original)
        if ($original -notmatch '\\d+$')
            { $res = $original.Substring(0,$original.length-1) + '.' +
            ([int]$original[$original.length-1]-[int][char]'a'+1); }
            else { while(($original -split '\\.').count -le 3)
            { $original += '.0'; } $res = $original; } $res }" -original ${OPENSSL_VERSION}
            OUTPUT_VARIABLE OPENSSL_CANONICAL_VERSION
            )
        string(STRIP ${OPENSSL_CANONICAL_VERSION} OPENSSL_CANONICAL_VERSION)

        # replace the values in the manifest template
        configure_file(
            "${PROJECT_SOURCE_DIR}/cmake/packaging/windows/fastdds.manifest.in"
            "${PROJECT_BINARY_DIR}/fastdds.manifest"
            @ONLY
            )

        file(TO_NATIVE_PATH "${PROJECT_BINARY_DIR}/fastdds.manifest" MANIFEST_FILE_PATH)
        target_link_options(${PROJECT_NAME} PRIVATE "/MANIFEST:EMBED" "/MANIFESTINPUT:${MANIFEST_FILE_PATH}")

        unset(WINDOWS_SOURCE_DIR)
        unset(FASTDDS_CANONICAL_VERSION)
        unset(OPENSSL_CANONICAL_VERSION)
        unset(MANIFEST_FILE_PATH)
    endif()

    set_target_properties(${PROJECT_NAME} PROPERTIES
        RELEASE_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        RELWITHDEBINFO_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        DEBUG_POSTFIX d-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        )

    get_target_property(TARGET_TYPE ${PROJECT_NAME} TYPE)
    if(TARGET_TYPE STREQUAL "SHARED_LIBRARY")
        # Export symbols in DLL library
        set_target_properties(${PROJECT_NAME} PROPERTIES
            PDB_NAME_DEBUG ${PROJECT_NAME}d-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
            PDB_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/symbols"
            )
    else()
        # Rename the library to have a "lib" before.
        set_target_properties(${PROJECT_NAME} PROPERTIES
            OUTPUT_NAME lib${PROJECT_NAME}
            COMPILE_PDB_NAME_DEBUG lib${PROJECT_NAME}d-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
            COMPILE_PDB_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/symbols"
            )
    endif()

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
        set_target_properties(${PROJECT_NAME} PROPERTIES VS_WINRT_COMPONENT "true")
    endif()
elseif(BUILD_SHARED_LIBS)
    set(VERSION_SCRIPT_SUPPORT_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libfastdds.version)
    set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS} ${VERSION_SCRIPT_SUPPORT_FLAGS})
    CHECK_CXX_SOURCE_COMPILES("int main(int, char**){return 0;}" HAS_VERSION_SCRIPT_SUPPORT)
    set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
    unset(OLD_CMAKE_REQUIRED_FLAGS)
    if(HAS_VERSION_SCRIPT_SUPPORT)
        message(STATUS "Using linker version script to select exported symbols")
        target_link_options(${PROJECT_NAME} PRIVATE ${VERSION_SCRIPT_SUPPORT_FLAGS})
    endif()
endif()

###############################################################################
# Packaging
###############################################################################

# Install public headers

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT headers
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
    PATTERN "*.ipp"
    )

# Install config.hpp header
install(FILES ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}/config.hpp
    DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}
    COMPONENT headers
    )

# Install public statistics idls
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/fastdds/statistics
    DESTINATION ${INCLUDE_INSTALL_DIR}/fastdds
    FILES_MATCHING
    PATTERN "*.idl"
    )

# Install libraries

install(TARGETS ${PROJECT_NAME} eProsima_atomic
    EXPORT ${PROJECT_NAME}-targets
    COMPONENT libraries
    RUNTIME DESTINATION "${BIN_INSTALL_DIR}${MSVCARCH_DIR_EXTENSION}"
    LIBRARY DESTINATION "${LIB_INSTALL_DIR}${MSVCARCH_DIR_EXTENSION}"
    ARCHIVE DESTINATION "${LIB_INSTALL_DIR}${MSVCARCH_DIR_EXTENSION}"
    )

# Generate different target names depending on linking
get_target_property(TARGET_TYPE ${PROJECT_NAME} TYPE)
if(TARGET_TYPE STREQUAL "SHARED_LIBRARY")
    set(FASTDDS_LINKING shared)
else()
    set(FASTDDS_LINKING static)
endif()

if(INSTALLER_PLATFORM)
    set(INSTALL_DESTINATION_PATH ${DATA_INSTALL_DIR}/${PROJECT_NAME}-${INSTALLER_PLATFORM}/cmake)
else()
    set(INSTALL_DESTINATION_PATH ${DATA_INSTALL_DIR}/${PROJECT_NAME}/cmake${MSVCARCH_DIR_EXTENSION_EXT})
endif()

install(EXPORT ${PROJECT_NAME}-targets
    DESTINATION ${INSTALL_DESTINATION_PATH}
    FILE ${PROJECT_NAME}-${FASTDDS_LINKING}-targets.cmake
    COMPONENT cmake
    )

if(MSVC OR MSVC_IDE)

    # first try dll symbols
    get_target_property(PDB_FILE ${PROJECT_NAME} PDB_NAME_DEBUG)
    if(PDB_FILE)
        get_target_property(PDB_DIR ${PROJECT_NAME} PDB_OUTPUT_DIRECTORY_DEBUG)
        set(PDB_FILE "${PDB_DIR}/${PDB_FILE}.pdb")
    else()
        # fallback to static lib symbols
        get_target_property(PDB_FILE ${PROJECT_NAME} COMPILE_PDB_NAME_DEBUG)
        if(PDB_FILE)
            get_target_property(PDB_DIR ${PROJECT_NAME} COMPILE_PDB_OUTPUT_DIRECTORY_DEBUG)
            set(PDB_FILE "${PDB_DIR}/${PDB_FILE}.pdb")
        endif()
    endif()

    # install symbols if any
    if(PDB_FILE)
        install(FILES ${PDB_FILE}
            DESTINATION ${LIB_INSTALL_DIR}${MSVCARCH_DIR_EXTENSION}
            COMPONENT symbols
            CONFIGURATIONS Debug
            )
    endif()
endif()

###############################################################################
# Create CMake package config file
###############################################################################
include(CMakePackageConfigHelpers)

if(BUILD_SHARED_LIBS)
    set(FASTDDS_PACKAGE_OPT_BIN_DIR_CONDITION "if(MSVC OR MSVC_IDE)")
else()
    set(FASTDDS_PACKAGE_OPT_BIN_DIR_CONDITION "if(0)")
endif()

# include the option parser targets
set(INCLUDE_FASTDDS_TARGETS "include(\${CMAKE_CURRENT_LIST_DIR}/optionparser-targets.cmake)")

if(COMPILE_TOOLS)
    # include the fast-discovery-server targets (not for linking but to hint binaries location and other target properties)
    set(INCLUDE_FASTDDS_TARGETS "${INCLUDE_FASTDDS_TARGETS}\ninclude(\${CMAKE_CURRENT_LIST_DIR}/fast-discovery-server-targets.cmake)")
endif()

# Add fastdds dependencies in its CMake config file.
if(LINK_SSL)
    if(INSTALLER_PLATFORM)
        # OpenSSL dependency is only propagated on static linking
        # We must use the deployed OpenSSL binaries to prevent compatibility issues
        set(FASTDDS_INSTALLER_DEPS_ANCILLARY "\
        function(find_fastdds_deployed_openssl)\n\
            if(OPENSSL_FOUND AND NOT(OPENSSL_VERSION STREQUAL \"${OPENSSL_VERSION}\") )\n\
                message(WARNING \"OpenSSL already loaded. Version \${OPENSSL_VERSION} differs from version ${OPENSSL_VERSION}, used in Fast DDS binaries building, ABI issues may arise\")\n\
            endif()\n\
            set(OPENSSL_USE_STATIC_LIBS TRUE)\n\
            set(OPENSSL_ROOT_DIR  \"\${PACKAGE_PREFIX_DIR}/lib/${INSTALLER_PLATFORM}\")\n\
            find_package(OpenSSL REQUIRED)\n\
        endfunction()"
        )
        set(FASTDDS_INSTALLER_OPT_DEPS "\
        if (\${type} STREQUAL \"static\")\n\
            find_fastdds_deployed_openssl()\n\
        endif()")

    elseif(WIN32)
        # windows from sources only requires OpenSSL on static libraries
        set(FASTDDS_PACKAGE_WIN32_OPT_DEPS "find_package(OpenSSL REQUIRED)")
    else()
        # linux from sources requires OpenSSL always
        set(FASTDDS_PACKAGE_UNIX_OPT_DEPS "find_package(OpenSSL REQUIRED)")
    endif()
endif()

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/packaging/Config.cmake.in
    ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${INSTALL_DESTINATION_PATH}
    PATH_VARS BIN_INSTALL_DIR INCLUDE_INSTALL_DIR LIB_INSTALL_DIR
    )
write_basic_package_version_file(${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
    )
install(FILES ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config.cmake
    ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config-version.cmake
    DESTINATION ${INSTALL_DESTINATION_PATH}
    COMPONENT cmake
    )
