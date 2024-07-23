# Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

if(SQLITE3_SUPPORT)
    # this test includes C sources
    enable_language(C)

    set(PERSISTENCETESTS_SOURCE
        PersistenceTests.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/PropertyPolicy.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/SerializedPayload.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/history/CacheChangePool.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/PersistenceFactory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/sqlite3.c
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/persistence/SQLite3PersistenceService.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
        )

    add_executable(PersistenceTests ${PERSISTENCETESTS_SOURCE})
    target_compile_definitions(PersistenceTests PRIVATE
        BOOST_ASIO_STANDALONE
        ASIO_STANDALONE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_include_directories(PersistenceTests PRIVATE
        ${Asio_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/test/mock/rtps/WriterHistory
        ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/cpp
        )
    target_link_libraries(PersistenceTests
        fastcdr
        fastdds::log
        foonathan_memory
        GTest::gmock
        ${CMAKE_DL_LIBS}
        )
    if(MSVC OR MSVC_IDE)
        target_link_libraries(PersistenceTests ${PRIVACY}
            iphlpapi Shlwapi
            )
    endif()
    gtest_discover_tests(PersistenceTests)

endif(SQLITE3_SUPPORT)
