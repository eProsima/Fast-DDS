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

# Link-seam mock: compiled against the real TypeLookupManager.hpp, replacing the dds::type-lookup-manager
# unit (TypeLookupManager.cpp). Tests set expectations on the TypeLookupManagerMock companion of each instance, reached
# with eprosima::fastdds::mock::mock_of().
add_mock_library(TypeLookupManager
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/fastdds/builtin/type_lookup_service/TypeLookupManagerMock.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TypeLookupManager.cpp
    REPLACES
    dds::type-lookup-manager
    )
