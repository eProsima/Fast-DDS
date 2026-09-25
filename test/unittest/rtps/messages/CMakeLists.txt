# Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DNOMINMAX)
endif()

get_property(fastdds_sources_files TARGET ${PROJECT_NAME} PROPERTY SOURCES)
list(REMOVE_ITEM fastdds_sources_files "${PROJECT_SOURCE_DIR}/src/cpp/fastdds/domain/DomainParticipantFactory.cpp")
get_property(fastdds_compile_definitions TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS)
list(FILTER fastdds_compile_definitions EXCLUDE REGEX ".*FASTDDS_DYN_LINK.*")
get_property(fastdds_include_directories TARGET ${PROJECT_NAME} PROPERTY INCLUDE_DIRECTORIES)
get_property(fastdds_link_libraries TARGET ${PROJECT_NAME} PROPERTY LINK_LIBRARIES)

###########################################################################
# MessageReceiverTests
###########################################################################
set(MESSAGE_RECEIVER_TESTS_SOURCE MessageReceiverTests.cpp)

add_executable(MessageReceiverTests ${MESSAGE_RECEIVER_TESTS_SOURCE})

target_compile_definitions(MessageReceiverTests PRIVATE
    ${fastdds_compile_definitions}
)

target_sources(MessageReceiverTests
    PRIVATE
    ${fastdds_sources_files}
)

target_include_directories(MessageReceiverTests PRIVATE
    ${PROJECT_SOURCE_DIR}/test/mock/dds/DomainParticipantFactory
    ${fastdds_include_directories}
)
target_link_libraries(MessageReceiverTests
    GTest::gmock
    ${fastdds_link_libraries}
)
gtest_discover_tests(MessageReceiverTests)
