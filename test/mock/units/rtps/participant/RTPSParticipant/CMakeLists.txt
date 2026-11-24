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

# Link-seam mock: compiled against the real RTPSParticipant.hpp, replacing the rtps::participant unit
# (RTPSParticipant.cpp). Its constructor is private, so instances are created by the RTPSDomain mock implementation.
# Tests set expectations on the RTPSParticipantMock companion of each instance, reached with
# eprosima::fastdds::mock::mock_of().
add_mock_library(RTPSParticipant
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/fastdds/rtps/participant/RTPSParticipantMock.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RTPSParticipant.cpp
    REPLACES
    rtps::participant
    )
