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

set(SHMSEGMENTTESTS_SOURCE SHMSegmentTests.cpp
#    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/ReaderProxy.cpp
#    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/subscriber/qos/ReaderQos.cpp
#    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/Log.cpp
#    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/OStreamConsumer.cpp
#    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutConsumer.cpp
#    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/log/StdoutErrConsumer.cpp
#    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
#    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/writer/LocatorSelectorSender.cpp
#    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/FlowControllerConsts.cpp
#    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/flowcontrol/ThroughputControllerDescriptor.cpp
#    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
)

# if (FASTDDS_STATISTICS)
# 
#     set(statistics_sources
#         ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipant.cpp
#         ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantImpl.cpp
#         ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/domain/DomainParticipantStatisticsListener.cpp
#         ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/publisher/qos/DataWriterQos.cpp
#         ${PROJECT_SOURCE_DIR}/src/cpp/statistics/fastdds/subscriber/qos/DataReaderQos.cpp
#         ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/StatisticsBase.cpp
#         ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/reader/StatisticsReaderImpl.cpp
#         ${PROJECT_SOURCE_DIR}/src/cpp/statistics/rtps/writer/StatisticsWriterImpl.cpp
#         ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/types.cxx
#         ${PROJECT_SOURCE_DIR}/src/cpp/statistics/types/typesPubSubTypes.cxx
#         )
#     list(APPEND SHMSEGMENTTESTS_SOURCE ${statistics_sources})
# 
# endif()

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

add_executable(SHMSegmentTests ${SHMSEGMENTTESTS_SOURCE})
target_compile_definitions(SHMSegmentTests PRIVATE FASTRTPS_NO_LIB
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(SHMSegmentTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/SharedMemSegment
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    )
target_link_libraries(SHMSegmentTests
    fastcdr fastrtps foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS}
    ${CMAKE_DL_LIBS}
    ${THIRDPARTY_BOOST_LINK_LIBS})
add_gtest(SHMSegmentTests SOURCES ${SHMSEGMENTTESTS_SOURCE})

if(ANDROID)
    set_property(TARGET SHMSegmentTests PROPERTY CROSSCOMPILING_EMULATOR "adb;shell;cd;${CMAKE_CURRENT_BINARY_DIR};&&")
endif()