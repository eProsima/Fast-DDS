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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../profiles/test_xml_profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/test_xml_profile.xml
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../profiles/test_xml_service_easy_mode.xml
    ${CMAKE_CURRENT_BINARY_DIR}/test_xml_service_easy_mode.xml
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../profiles/test_xml_easy_mode_config.xml
    ${CMAKE_CURRENT_BINARY_DIR}/test_xml_easy_mode_config.xml
    COPYONLY)

set(PARTICIPANTTESTS_SOURCE
        ParticipantTests.cpp
    )

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND PARTICIPANTTESTS_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()


add_executable(ParticipantTests ${PARTICIPANTTESTS_SOURCE})
target_compile_definitions(ParticipantTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(ParticipantTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/test/utils
    )
target_link_libraries(ParticipantTests fastdds fastcdr foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})

if(EPROSIMA_TEST_DNS_NOT_SET_UP)
    message(STATUS "Ignoring 'ParticipantTests.SimpleParticipantRemoteServerListConfigurationDNS'")
    set(dns_filter "-ParticipantTests.SimpleParticipantRemoteServerListConfigurationDNS")
endif()

gtest_discover_tests(ParticipantTests TEST_FILTER ${dns_filter})

unset(dns_filter)
