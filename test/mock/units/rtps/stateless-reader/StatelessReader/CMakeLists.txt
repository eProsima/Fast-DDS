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

# Link-seam mock: compiled against the real StatelessReader.hpp, replacing the rtps::stateless-reader unit
# (StatelessReader.cpp). Tests create TestStatelessReader instances and set expectations on the StatelessReaderMock
# companion of each one, reached with eprosima::fastdds::mock::mock_of(). The BaseReader mock provides the base.
add_mock_library(StatelessReader
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/rtps/reader/StatelessReaderMock.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StatelessReader.cpp
    DEPENDS
    BaseReader
    REPLACES
    rtps::stateless-reader
    )
target_include_directories(fastdds-mock-StatelessReader PRIVATE ${Asio_INCLUDE_DIR} ${THIRDPARTY_BOOST_INCLUDE_DIR})
