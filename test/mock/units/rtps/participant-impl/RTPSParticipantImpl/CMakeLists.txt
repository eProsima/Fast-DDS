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
# (RTPSParticipantImpl.cpp). Tests create TestRTPSParticipantImpl instances and set expectations on the
# RTPSParticipantImplMock companion of each one, reached with eprosima::fastdds::mock::mock_of(). Constructing a
# participant constructs its bases and members, so the mock depends on the mocks providing them. Tests also link a
# SecurityManager, either its mock or its real source.
add_mock_library(RTPSParticipantImpl
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/rtps/participant/RTPSParticipantImplMock.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RTPSParticipantImpl.cpp
    DEPENDS
    MonitorService
    RTPSParticipantImplAccessors
    SecurityPluginFactory
    StatisticsBase
    REPLACES
    rtps::participant-impl
    )
target_include_directories(fastdds-mock-RTPSParticipantImpl PRIVATE
    ${Asio_INCLUDE_DIR}
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    )
