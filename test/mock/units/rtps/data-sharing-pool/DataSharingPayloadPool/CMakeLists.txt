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

# Link-seam mock: compiled against the real DataSharingPayloadPool.hpp, replacing the rtps::data-sharing-pool unit
# (DataSharingPayloadPool.cpp). No data sharing pool can be created, so data sharing is never used.
add_mock_library(DataSharingPayloadPool
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/DataSharingPayloadPool.cpp
    REPLACES
    rtps::data-sharing-pool
    )

target_include_directories(fastdds-mock-DataSharingPayloadPool PRIVATE ${THIRDPARTY_BOOST_INCLUDE_DIR})
