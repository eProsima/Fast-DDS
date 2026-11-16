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

# Link-seam mock: compiled against the real StatefulReader.hpp, replacing the rtps::stateful-reader unit
# (StatefulReader.cpp). Tests create TestStatefulReader instances and set expectations on the StatefulReaderMock
# companion of each one, reached with eprosima::fastdds::mock::mock_of(). The BaseReader mock provides the base.
add_mock_library(StatefulReader
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/rtps/reader/StatefulReaderMock.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StatefulReader.cpp
    DEPENDS
    BaseReader
    REPLACES
    rtps::stateful-reader
    )
target_include_directories(fastdds-mock-StatefulReader PRIVATE ${Asio_INCLUDE_DIR} ${THIRDPARTY_BOOST_INCLUDE_DIR})
