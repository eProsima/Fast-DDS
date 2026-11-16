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

# Link-seam mock: compiled against the real RTPSWriter.hpp and BaseWriter.hpp, replacing the rtps::writer-base unit
# (RTPSWriter.cpp, BaseWriter.cpp and StatisticsWriterImpl.cpp). Tests needing a writer use TestBaseWriter, which
# mocks the virtual methods, and set expectations on the non-virtual ones through the BaseWriterMock companion of each
# instance, reached with eprosima::fastdds::mock::mock_of().
add_mock_library(BaseWriter
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/rtps/writer/BaseWriterMock.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BaseWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RTPSWriter.cpp
    $<$<BOOL:${FASTDDS_STATISTICS}>:${CMAKE_CURRENT_SOURCE_DIR}/StatisticsWriterImpl.cpp>
    REPLACES
    rtps::writer-base
    )
target_include_directories(fastdds-mock-BaseWriter PRIVATE ${Asio_INCLUDE_DIR})
