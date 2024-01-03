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

file(GLOB XTYPES_SOURCE
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TopicDataType.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/*.cpp
    )

set(XTYPES_TEST_SOURCE
    XTypesTests.cpp
    ${XTYPES_SOURCE}
    )

if(ANDROID)
    if (ANDROID_NATIVE_API_LEVEL LESS 24)
        list(APPEND XTYPES_TEST_SOURCE
            ${ANDROID_IFADDRS_SOURCE_DIR}/ifaddrs.c
            )
    endif()
endif()

include_directories(mock/)

add_executable(XTypesTests ${XTYPES_TEST_SOURCE})
target_compile_definitions(XTypesTests PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(XTypesTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(XTypesTests GTest::gtest ${MOCKS} ${THIRDPARTY_BOOST_LINK_LIBS})
if(MSVC OR MSVC_IDE)
    target_link_libraries(XTypesTests ${PRIVACY} fastcdr foonathan_memory iphlpapi Shlwapi ws2_32)
else()
    target_link_libraries(XTypesTests ${PRIVACY} fastcdr foonathan_memory)
endif()
gtest_discover_tests(XTypesTests)
