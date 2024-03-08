# Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

set(RTPS_DOMAIN_TESTS_EXEC RTPSDomainTests)

file(GLOB RTPS_DOMAIN_TESTS_SOURCE
    RTPSDomainTests.cpp
    )

# External sources
if(TINYXML2_SOURCE_DIR)
    list(APPEND RTPS_DOMAIN_TESTS_SOURCE
        ${TINYXML2_SOURCE_DIR}/tinyxml2.cpp
        )
endif()

if(TINYXML2_INCLUDE_DIR)
    include_directories(${TINYXML2_INCLUDE_DIR})
endif(TINYXML2_INCLUDE_DIR)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_executable(
    ${RTPS_DOMAIN_TESTS_EXEC}
    ${RTPS_DOMAIN_TESTS_SOURCE}
    )

target_compile_definitions(${RTPS_DOMAIN_TESTS_EXEC} PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )

target_include_directories(
    ${RTPS_DOMAIN_TESTS_EXEC}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${Asio_INCLUDE_DIR}
    )

target_link_libraries(
    ${RTPS_DOMAIN_TESTS_EXEC}
    GTest::gmock
    ${CMAKE_DL_LIBS}
    fastcdr
    fastdds
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto>
    ${TINYXML2_LIBRARY}
    )

gtest_discover_tests(${RTPS_DOMAIN_TESTS_EXEC})
