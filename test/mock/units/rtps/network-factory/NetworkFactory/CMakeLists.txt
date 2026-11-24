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

# Link-seam mock: compiled against the real NetworkFactory.hpp. Only replaces NetworkFactory.cpp, the rest of the
# rtps::network-factory unit is real. No transport is registered: every locator is supported, allowed, reachable and
# local, and remote locators are not transformed.
add_mock_library(rtps::network-factory::NetworkFactory
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/NetworkFactory.cpp
    )
