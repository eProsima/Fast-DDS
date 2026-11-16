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

# Link-seam mock: compiled against the real DataWriter.hpp, replacing the dds::data-writer unit (DataWriter.cpp).
# Only the constructors, enable() and write() are implemented. Tests create TestDataWriter instances and set
# expectations on the strict DataWriterMock companion of each one, reached with eprosima::fastdds::mock::mock_of().
add_mock_library(DataWriter
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/fastdds/dds/publisher/DataWriterMock.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DataWriter.cpp
    REPLACES
    dds::data-writer
    )
