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

#######################
# MaxAsioBuffers test #
#######################

set(MAXASIOBUFFERSTESTS_SOURCE MaxAsioBuffersTests.cpp)

add_executable(MaxAsioBuffersTests ${MAXASIOBUFFERSTESTS_SOURCE})
target_compile_definitions(MaxAsioBuffersTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(MaxAsioBuffersTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(MaxAsioBuffersTests
    GTest::gtest
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${QNX}>:socket>
    )
gtest_discover_tests(MaxAsioBuffersTests PROPERTIES LABELS "NoMemoryCheck")

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DNOMINMAX)
endif()

# Target sources may be relative to src/cpp. Make them absolute.
get_property(fastdds_target_sources_files TARGET ${PROJECT_NAME} PROPERTY SOURCES)
set(fastdds_sources_files)
foreach(fastdds_source_file ${fastdds_target_sources_files})
    get_filename_component(fastdds_source_file "${fastdds_source_file}" ABSOLUTE BASE_DIR "${PROJECT_SOURCE_DIR}/src/cpp")
    list(APPEND fastdds_sources_files "${fastdds_source_file}")
endforeach()
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
