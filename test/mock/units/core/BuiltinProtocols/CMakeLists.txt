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

# Link-seam mock: compiled against the real BuiltinProtocols.h, for unit tests not linking core, where
# BuiltinProtocols.cpp is. Tests fill the public members they need; destroying it does not touch the protocols they
# point to.
add_mock_library(BuiltinProtocols
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/BuiltinProtocols.cpp
    )
