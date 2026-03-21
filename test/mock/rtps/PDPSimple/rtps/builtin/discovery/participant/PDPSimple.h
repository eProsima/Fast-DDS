// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file PDPSimple.h
 */

#ifndef FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDPSIMPLE_H
#define FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDPSIMPLE_H

#include <gmock/gmock.h>

#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/discovery/endpoint/EDP.h>
#include <rtps/messages/CDRMessage.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class PDPSimple
{
public:

    MOCK_METHOD2(notifyAboveRemoteEndpoints, void(const ParticipantProxyData&, bool));

    MOCK_METHOD1(get_participant_proxy_data_serialized, CDRMessage_t(Endianness_t));

    EDP* get_edp()
    {
        return &edp_;
    }

private:

    EDP edp_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDPSIMPLE_H
