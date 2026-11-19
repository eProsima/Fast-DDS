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

# Link-seam mock: compiled against the real DomainParticipantFactory.hpp. It only implements the singleton access and
# the TypeObjectRegistry access, which forwards to the RTPSDomainImpl. Using any other function fails to link.
# The QoS members of the factory need the dds::participant-qos unit, which tests not linking core link with the
# XMLProfileManager mock.
add_mock_library(DomainParticipantFactory
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/DomainParticipantFactory.cpp
    REPLACES
    dds::participant-factory
    )
