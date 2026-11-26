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

# Link-seam mock: compiled against the real RTPSReader.hpp and BaseReader.hpp, replacing the rtps::reader-base unit
# (RTPSReader.cpp, BaseReader.cpp and StatisticsReaderImpl.cpp). Tests needing a reader use TestBaseReader, which
# mocks the virtual methods, and set expectations on the non-virtual ones through the BaseReaderMock companion of each
# instance, reached with eprosima::fastdds::mock::mock_of().
add_mock_library(BaseReader
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/rtps/reader/BaseReaderMock.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BaseReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RTPSReader.cpp
    $<$<BOOL:${FASTDDS_STATISTICS}>:${CMAKE_CURRENT_SOURCE_DIR}/StatisticsReaderImpl.cpp>
    REPLACES
    rtps::reader-base
    )
target_include_directories(fastdds-mock-BaseReader PRIVATE ${Asio_INCLUDE_DIR} ${THIRDPARTY_BOOST_INCLUDE_DIR})
