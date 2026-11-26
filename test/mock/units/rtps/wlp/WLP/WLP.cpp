// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Link-seam implementation of WLP: replaces src/cpp/rtps/builtin/liveliness/WLP.cpp. Liveliness is never asserted and
// no remote endpoint is removed.

#include <rtps/builtin/liveliness/WLP.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

WLP::~WLP()
{
}

void WLP::removeRemoteEndpoints(
        ParticipantProxyData*)
{
}

bool WLP::assert_liveliness(
        GUID_t,
        dds::LivelinessQosPolicyKind,
        dds::Duration_t)
{
    return false;
}

bool WLP::assert_liveliness_manual_by_participant()
{
    return false;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
