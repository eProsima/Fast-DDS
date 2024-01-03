# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

if (SQLITE3_SUPPORT AND NOT QNX)
    enable_language(C)
    set(DDSSQLFILTERTEST_SOURCES DDSSQLFilterTests.cpp)
    file(GLOB DDSSQLFILTER_TYPE_SOURCES data_types/*.cxx)

    file(GLOB DDSSQLFILTER_SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/*.cpp
        )

    file(GLOB DDSSQLFILTER_LIB_SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/builtin/type_lookup_service/detail/*.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/builtin/type_lookup_service/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Entity.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/qos/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/history/DataReaderHistory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/qos/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/utils/QosConverters.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/exception/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/*.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/database/backup/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/database/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/endpoint/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/participant/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/participant/timedevent/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/liveliness/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/DataSharing/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/messages/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/participant/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/sqlite3.c
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/resources/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/ChannelResource.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/SharedMemTransportDescriptor.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/tcp/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptor.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorBasic.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResource.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceBasic.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPTransportInterface.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPv4Transport.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPv6Transport.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPChannelResource.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPTransportInterface.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPv4Transport.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/UDPv6Transport.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/xmlparser/*.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/publisher/qos/DataWriterQos.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/reader/StatisticsReaderImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/StatisticsBase.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/writer/StatisticsWriterImpl.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/*.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/*.cpp
        )

    if (FASTDDS_STATISTICS)
        file(GLOB statistics_sources
            ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/*.cpp
            )
        list(APPEND DDSSQLFILTER_LIB_SOURCES ${statistics_sources})
    endif()

    if (IS_THIRDPARTY_BOOST_OK)
        list(APPEND DDSSQLFILTER_LIB_SOURCES
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/test_SharedMemTransport.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/SharedMemTransport.cpp
            )
    endif()

    if(TLS_FOUND)
        list(APPEND DDSSQLFILTER_LIB_SOURCES
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPChannelResourceSecure.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/TCPAcceptorSecure.cpp
            )
    endif()

    if(SECURITY)
        file(GLOB security_sources
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/exceptions/Exception.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Token.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/exceptions/SecurityException.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/common/SharedSecretHandle.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/logging/Logging.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/*.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/discovery/participant/DS/PDPSecurityInitiatorListener.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/*.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/artifact_providers/*.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/*.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/*.cpp
            ${PROJECT_SOURCE_DIR}/src/cpp/security/logging/*.cpp
            )
        list(APPEND DDSSQLFILTER_LIB_SOURCES ${security_sources})
    endif()

    # External sources
    if(TINYXML2_SOURCE_DIR)
        list(APPEND DDSSQLFILTER_LIB_SOURCES
            ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
            )
    endif()

    if(WIN32)
        add_definitions(-D_WIN32_WINNT=0x0601)
    endif()

    # Only include tinyxml2 headers manually if the associated config predates the use of CMake targets
    if(TINYXML2_INCLUDE_DIR)
        include_directories(${TINYXML2_INCLUDE_DIR})
    endif(TINYXML2_INCLUDE_DIR)

    add_executable(DDSSQLFilterTests
        ${DDSSQLFILTERTEST_SOURCES}
        ${DDSSQLFILTER_TYPE_SOURCES}
        ${DDSSQLFILTER_SOURCES}
        ${DDSSQLFILTER_LIB_SOURCES}
        )
    target_compile_definitions(DDSSQLFilterTests PRIVATE
        BOOST_ASIO_STANDALONE
        ASIO_STANDALONE
        ASIO_DISABLE_VISIBILITY
        SQLITE_WIN32_GETVERSIONEX=0
        $<$<AND:$<BOOL:${WIN32}>,$<STREQUAL:"${CMAKE_SYSTEM_NAME}","WindowsStore">>:SQLITE_OS_WINRT>
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_include_directories(DDSSQLFilterTests PRIVATE
        ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/cpp
        ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
        ${THIRDPARTY_BOOST_INCLUDE_DIR}
        $<$<BOOL:${OPENSSL_INCLUDE_DIR}>:${OPENSSL_INCLUDE_DIR}>
        )
    target_link_libraries(DDSSQLFilterTests fastcdr foonathan_memory
        GTest::gmock
        ${TINYXML2_LIBRARY}
        ${THIRDPARTY_BOOST_LINK_LIBS}
        ${CMAKE_DL_LIBS}
        $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
        )
    gtest_discover_tests(DDSSQLFilterTests)
endif()
