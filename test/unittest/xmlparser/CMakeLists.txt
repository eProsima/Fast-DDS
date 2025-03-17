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

if(WIN32)
    add_definitions(
        -D_WIN32_WINNT=0x0601
        -D_CRT_SECURE_NO_WARNINGS
        )
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_xml_duration_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/test_xml_duration_profile.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_xml_root_library_settings_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/test_xml_root_library_settings_profile.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_xml_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/test_xml_profile.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_xml_profile_env_var.xml
    ${CMAKE_CURRENT_BINARY_DIR}/test_xml_profile_env_var.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_xml_deprecated.xml
    ${CMAKE_CURRENT_BINARY_DIR}/test_xml_deprecated.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_xml_security_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/test_xml_security_profile.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_xml_rooted_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/test_xml_rooted_profile.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_xml_rooted_deprecated.xml
    ${CMAKE_CURRENT_BINARY_DIR}/test_xml_rooted_deprecated.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/log_stdouterr_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/log_stdouterr_profile.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/log_stdouterr_wrong_property_name_profile_invalid.xml
    ${CMAKE_CURRENT_BINARY_DIR}/log_stdouterr_wrong_property_name_profile_invalid.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/log_stdouterr_wrong_property_value_profile_invalid.xml
    ${CMAKE_CURRENT_BINARY_DIR}/log_stdouterr_wrong_property_value_profile_invalid.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/log_stdouterr_two_thresholds_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/log_stdouterr_two_thresholds_profile.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/log_def_file_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/log_def_file_profile.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/log_inactive_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/log_inactive_profile.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/log_node_file_append_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/log_node_file_append_profile.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tls_config_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/tls_config_profile.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/UDP_transport_descriptors_config_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/UDP_transport_descriptors_config_profile.xml
    COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/SHM_transport_descriptors_config_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/SHM_transport_descriptors_config_profile.xml
    COPYONLY)
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/regressions" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

###################################  XMLProfileParserTests  ####################################################
set(XMLPROFILEPARSER_SOURCE
    XMLProfileParserTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/UnitsParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLDynamicParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLElementParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserCommon.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLProfileManager.cpp
    )

# External sources
if(TINYXML2_SOURCE_DIR)
    list(APPEND XMLPROFILEPARSER_SOURCE
        ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
        )
endif()

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND XMLPROFILEPARSER_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

# Only include tinyxml2 headers manually if the associated config predates the use of CMake targets
if(TINYXML2_INCLUDE_DIR)
    include_directories(${TINYXML2_INCLUDE_DIR})
endif(TINYXML2_INCLUDE_DIR)

add_executable(XMLProfileParserTests ${XMLPROFILEPARSER_SOURCE})
target_compile_definitions(XMLProfileParserTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    $<$<BOOL:${MSVC}>:NOMINMAX> # avoid conflic with std::min & std::max in visual studio
    )
target_include_directories(XMLProfileParserTests PRIVATE
    ${PROJECT_SOURCE_DIR}/test/mock/xtypes/TypeObjectRegistry
    ${PROJECT_SOURCE_DIR}/test/mock/xtypes/TypeObjectUtils
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/Log
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomainImpl # Dependency of fastdds::xtypes::type_representation
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/SharedMemTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv4TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv6TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv4TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv6TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantAttributes
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )

target_link_libraries(XMLProfileParserTests
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    ${TINYXML2_LIBRARY}
    fastcdr
    fastdds::xtypes::dynamic-types::impl
    GTest::gmock
    )
if(QNX)
    target_link_libraries(XMLProfileParserTests socket)
endif()
gtest_discover_tests(XMLProfileParserTests)
###################################  XMLProfileParserTests  ####################################################



######################################  XMLParserTests  ########################################################
set(XMLPARSER_SOURCE
    XMLParserTests.cpp
    XMLElementParserTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/UnitsParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLDynamicParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLElementParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserCommon.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLProfileManager.cpp
    )

# External sources
if(TINYXML2_SOURCE_DIR)
    list(APPEND XMLPARSER_SOURCE
        ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
        )
endif()

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND XMLPARSER_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

add_executable(XMLParserTests ${XMLPARSER_SOURCE})
target_compile_definitions(XMLParserTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    $<$<BOOL:${MSVC}>:NOMINMAX> # avoid conflic with std::min & std::max in visual studio
    )
target_include_directories(XMLParserTests PRIVATE
    ${PROJECT_SOURCE_DIR}/test/mock/xtypes/TypeObjectRegistry
    ${PROJECT_SOURCE_DIR}/test/mock/xtypes/TypeObjectUtils
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomainImpl # Dependency of fastdds::xtypes::type_representation
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/SharedMemTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv4TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv6TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv4TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv6TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantAttributes
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )

target_link_libraries(XMLParserTests
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    ${TINYXML2_LIBRARY}
    fastcdr
    fastdds::log
    fastdds::xtypes::dynamic-types::impl
    GTest::gmock
    )
if(QNX)
    target_link_libraries(XMLParserTests socket)
endif()
if(EPROSIMA_TEST_DNS_NOT_SET_UP)
    message(STATUS "Ignoring 'getXMLLocatorDNS*'")
    set(IGNORE_COMMAND "-getXMLLocatorDNS*")
endif()
gtest_discover_tests(XMLParserTests TEST_FILTER ${IGNORE_COMMAND})
######################################  XMLParserTests  ########################################################

#######################################  XMLTreeTests  #########################################################
set(XMLTREE_SOURCE
    XMLTreeTests.cpp
    )

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND XMLTREE_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

add_executable(XMLTreeTests ${XMLTREE_SOURCE})
target_compile_definitions(XMLTreeTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(XMLTreeTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )

target_link_libraries(XMLTreeTests GTest::gtest
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    )
gtest_discover_tests(XMLTreeTests)
#######################################  XMLTreeTests  #########################################################


###################################  XMLENDPOINTPARSERTESTS  ###################################################
set(XMLENDPOINTPARSERTESTS_SOURCE
    XMLEndpointParserTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/PublicationBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/builtin/data/SubscriptionBuiltinTopicData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/UnitsParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLElementParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLEndpointParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserCommon.cpp

    # locators
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/UnitsParser.cpp
    )

# External sources
if(TINYXML2_SOURCE_DIR)
    list(APPEND XMLENDPOINTPARSERTESTS_SOURCE
        ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
        )
endif()

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND XMLENDPOINTPARSERTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

add_executable(XMLEndpointParserTests ${XMLENDPOINTPARSERTESTS_SOURCE})
target_compile_definitions(XMLEndpointParserTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    )
target_include_directories(XMLEndpointParserTests PRIVATE
    ${PROJECT_SOURCE_DIR}/test/mock/xtypes/TypeObjectRegistry
    ${PROJECT_SOURCE_DIR}/test/mock/xtypes/TypeObjectUtils
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/ReaderProxyData
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomainImpl
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/SharedMemTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv4TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv6TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv4TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv6TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterProxyData
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/XMLProfileManager
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )

target_link_libraries(XMLEndpointParserTests
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    ${TINYXML2_LIBRARY}
    fastcdr
    fastdds::log
    fastdds::xtypes::dynamic-types::impl
    GTest::gmock
    )
if(QNX)
    target_link_libraries(XMLEndpointParserTests socket)
endif()
gtest_discover_tests(XMLEndpointParserTests)

######################################  XMLLoadFileTests  ######################################################
set(XMLLOADFILE_SOURCE
    XMLLoadFileTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/WriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/ThreadSettings.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/PortBasedTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/UnitsParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLDynamicParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLElementParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserCommon.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLProfileManager.cpp

    # locators
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/UnitsParser.cpp
    )

# External sources
if(TINYXML2_SOURCE_DIR)
    list(APPEND XMLLOADFILE_SOURCE
        ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
        )
endif()

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND XMLLOADFILE_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

add_executable(XMLLoadFileTests ${XMLLOADFILE_SOURCE})
target_compile_definitions(XMLLoadFileTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<BOOL:${MSVC}>:NOMINMAX> # avoid conflict with std::min & std::max in visual studio
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    HAVE_LOG_NO_INFO=0 FASTDDS_ENFORCE_LOG_INFO # Need LOG_INFO in this test
    )
target_include_directories(XMLLoadFileTests PRIVATE
    ${PROJECT_SOURCE_DIR}/test/mock/xtypes/TypeObjectRegistry
    ${PROJECT_SOURCE_DIR}/test/mock/xtypes/TypeObjectUtils
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSDomainImpl # Dependency of fastdds::xtypes::type_representation
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/SharedMemTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv4TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/TCPv6TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPTransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv4TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/UDPv6TransportDescriptor
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/RTPSParticipantAttributes
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )

target_link_libraries(XMLLoadFileTests
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    ${TINYXML2_LIBRARY}
    fastcdr
    fastdds::log
    fastdds::xtypes::dynamic-types::impl
    GTest::gmock
    )
if(QNX)
    target_link_libraries(XMLLoadFileTests socket)
endif()
gtest_discover_tests(XMLLoadFileTests)
