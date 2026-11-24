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

# Link-seam mock: compiled against the real SecurityManager.h, replacing the rtps::security-manager unit
# (SecurityManager.cpp), which only exists with SECURITY. Only defines the methods those tests reach, which reject every
# remote entity and provide no token, and the construction and destruction the participant owning it does.
set(SECURITY_MANAGER_REPLACES)
if(SECURITY)
    set(SECURITY_MANAGER_REPLACES REPLACES rtps::security-manager)
endif()
add_mock_library(SecurityManager
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/SecurityManager.cpp
    ${SECURITY_MANAGER_REPLACES}
    )
