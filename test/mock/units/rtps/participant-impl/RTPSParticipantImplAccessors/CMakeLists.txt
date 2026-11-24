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

# Link-seam mock: compiled against the real RTPSParticipantImpl.hpp, replacing the rtps::participant-impl unit
# (RTPSParticipantImpl.cpp). It is meant for tests compiling code that refers to a participant without creating one:
# only the attribute accessors are defined, and no constructor or destructor. The RTPSParticipantImpl mock, for tests
# creating participants, depends on it.
add_mock_library(RTPSParticipantImplAccessors
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/RTPSParticipantImplAccessors.cpp
    REPLACES
    rtps::participant-impl
    )
target_include_directories(fastdds-mock-RTPSParticipantImplAccessors PRIVATE
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    )
