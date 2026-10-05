# Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/DataSharing/DataSharingPayloadPool.cpp
    )

add_executable(SHMSegmentTests ${SHMSEGMENTTESTS_SOURCE})
target_include_directories(SHMSegmentTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/test/mock/rtps/SharedMemSegment
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    )
target_link_libraries(SHMSegmentTests
    fastcdr fastdds foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS}
    ${CMAKE_DL_LIBS}
    ${THIRDPARTY_BOOST_LINK_LIBS})
gtest_discover_tests(SHMSegmentTests)
