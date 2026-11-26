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
    )

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
target_link_units(XMLProfileParserTests UNITS xmlparser::parser rtps::common xtypes rtps::builtin-data
    xmlparser::common MOCKS xtypes::TypeObjectUtils Log RTPSDomainImpl)
target_include_directories(XMLProfileParserTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )

target_link_libraries(XMLProfileParserTests
    fastcdr
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${QNX}>:socket>
    ${TINYXML2_LIBRARY}
    )
gtest_discover_tests(XMLProfileParserTests)
###################################  XMLProfileParserTests  ####################################################



######################################  XMLParserTests  ########################################################
set(XMLPARSER_SOURCE
    XMLParserTests.cpp
    XMLElementParserTests.cpp
    )

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND XMLPARSER_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

add_executable(XMLParserTests ${XMLPARSER_SOURCE})
target_link_units(XMLParserTests UNITS xmlparser::parser rtps::common xtypes rtps::builtin-data
    xmlparser::common MOCKS xtypes::TypeObjectUtils RTPSDomainImpl)
target_include_directories(XMLParserTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )

target_link_libraries(XMLParserTests
    fastcdr
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${QNX}>:socket>
    ${TINYXML2_LIBRARY}
    )
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
    )

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND XMLENDPOINTPARSERTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

add_executable(XMLEndpointParserTests ${XMLENDPOINTPARSERTESTS_SOURCE})
target_link_units(XMLEndpointParserTests UNITS rtps::proxy-data
    rtps::shm-transport rtps::common xtypes rtps::builtin-data xmlparser::common
    xmlparser::endpoint-parser xmlparser::parser MOCKS xtypes::TypeObjectUtils MessageReceiver RTPSDomainImpl
    XMLProfileManager)
target_include_directories(XMLEndpointParserTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )

target_link_libraries(XMLEndpointParserTests
    fastcdr
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${QNX}>:socket>
    ${TINYXML2_LIBRARY}
    )
gtest_discover_tests(XMLEndpointParserTests)

######################################  XMLLoadFileTests  ######################################################
set(XMLLOADFILE_SOURCE
    XMLLoadFileTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLDynamicParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLElementParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLParserExtras.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/xmlparser/XMLProfileManager.cpp

    # buffer

    # locators
    )

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND XMLLOADFILE_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

add_executable(XMLLoadFileTests ${XMLLOADFILE_SOURCE})
target_compile_definitions(XMLLoadFileTests PRIVATE
    HAVE_LOG_NO_INFO=0 FASTDDS_ENFORCE_LOG_INFO # Need LOG_INFO in this test
    )
target_link_units(XMLLoadFileTests UNITS rtps::shm-transport rtps::udp-transport
    rtps::common xtypes rtps::builtin-data xmlparser::common MOCKS
    xtypes::TypeObjectUtils RTPSDomainImpl)
target_include_directories(XMLLoadFileTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    )

target_link_libraries(XMLLoadFileTests
    $<$<BOOL:${QNX}>:socket>
    fastcdr
    GTest::gmock
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${QNX}>:socket>
    ${TINYXML2_LIBRARY}
    )
gtest_discover_tests(XMLLoadFileTests)
