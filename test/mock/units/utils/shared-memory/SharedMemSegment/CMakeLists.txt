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

# Link-seam mock: compiled against the real SharedMemSegment.hpp. The extra size of the allocations in a segment can't
# be computed, as if the temporary segment could not be created, so no shared memory segment can be set up.
add_mock_library(SharedMemSegment
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/SharedMemSegment.cpp
    REPLACES
    utils::shared-memory
    )
target_include_directories(fastdds-mock-SharedMemSegment PRIVATE ${THIRDPARTY_BOOST_INCLUDE_DIR})
