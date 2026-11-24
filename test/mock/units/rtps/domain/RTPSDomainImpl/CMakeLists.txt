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

# Link-seam mock: compiled against the real RTPSDomainImpl.hpp. Tests set expectations on the RTPSDomainImplMock
# companion of each instance, reached with eprosima::fastdds::mock::mock_of().
# The unit also holds the RTPSDomain static functions, compiled against the real RTPSDomain.hpp. While a RTPSDomainMock
# exists, they forward to it. Otherwise they do nothing and return nullptr or false. Participants are created with the
# RTPSParticipant mock, as its constructor is private.
add_mock_library(RTPSDomainImpl
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/fastdds/rtps/RTPSDomainMock.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RTPSDomain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/rtps/domain/RTPSDomainImplMock.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RTPSDomainImpl.cpp
    DEPENDS
    RTPSParticipant
    REPLACES
    rtps::domain
    )

# The real header initializes members with OpenSSLInit when security is enabled.
target_link_libraries(fastdds-mock-RTPSDomainImpl PUBLIC
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
)
