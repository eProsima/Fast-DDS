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

set(DDSSQLFILTERTEST_SOURCES DDSSQLFilterTests.cpp)
file(GLOB DDSSQLFILTER_TYPE_SOURCES data_types/*.cxx)

file(GLOB DDSSQLFILTER_SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/*.cpp
    )

file(GLOB DDSSQLFILTER_LIB_SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/dynamic-types/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/condition/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Entity.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/policy/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/qos/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/publisher/qos/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/history/DataReaderHistory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/qos/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/utils/QosConverters.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/exception/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/xtypes/type_representation/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/attributes/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/participant/RTPSParticipant.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/shared_mem/SharedMemTransportDescriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/xmlparser/*.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/publisher/qos/DataWriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/*.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/publisher/qos/DataWriterQos.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/string_convert.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    )

if (FASTDDS_STATISTICS)

    file(GLOB statistics_sources
        ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/*.cpp
        )

    list(APPEND DDSSQLFILTER_LIB_SOURCES ${statistics_sources})

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
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(DDSSQLFilterTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    )
target_link_libraries(DDSSQLFilterTests fastcdr foonathan_memory
    GTest::gmock
    ${TINYXML2_LIBRARY}
    ${THIRDPARTY_BOOST_LINK_LIBS}
    ${CMAKE_DL_LIBS})
gtest_discover_tests(DDSSQLFilterTests)
