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
target_include_directories(MaxAsioBuffersTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(MaxAsioBuffersTests
    GTest::gtest
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${QNX}>:socket>
    )
gtest_discover_tests(MaxAsioBuffersTests PROPERTIES LABELS "NoMemoryCheck")

get_property(fastdds_include_directories TARGET ${PROJECT_NAME} PROPERTY INCLUDE_DIRECTORIES)

###########################################################################
# MessageReceiverTests
###########################################################################
set(MESSAGE_RECEIVER_TESTS_SOURCE MessageReceiverTests.cpp)

add_executable(MessageReceiverTests ${MESSAGE_RECEIVER_TESTS_SOURCE})


target_link_units(MessageReceiverTests UNITS core)
target_include_directories(MessageReceiverTests PRIVATE
    ${fastdds_include_directories}
    )
target_link_libraries(MessageReceiverTests
    GTest::gmock
    )
gtest_discover_tests(MessageReceiverTests)
