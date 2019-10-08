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

#ifndef _FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_PDPSIMPLE_H_
#define _FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_PDPSIMPLE_H_

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDP.h>

#include <gmock/gmock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class PDPSimple
{
    public:

        MOCK_METHOD1(notifyAboveRemoteEndpoints, void(const ParticipantProxyData&));

        MOCK_METHOD1(get_participant_proxy_data_serialized, CDRMessage_t(Endianness_t));

        EDP* getEDP() { return &edp_; }

    private:

        EDP edp_;
};

} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_PDPSIMPLE_H_
