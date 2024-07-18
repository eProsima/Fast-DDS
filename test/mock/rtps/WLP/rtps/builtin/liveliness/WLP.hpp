// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file WLP.hpp
 *
 */

#ifndef FASTDDS_RTPS_BUILTIN_LIVELINESS__WLP_HPP
#define FASTDDS_RTPS_BUILTIN_LIVELINESS__WLP_HPP

#include <mutex>
#include <vector>

#include <gmock/gmock.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/common/Guid.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class BuiltinProtocols;

/**
 * Class WLP that implements the Writer Liveliness Protocol described in the RTPS specification.
 * @ingroup LIVELINESS_MODULE
 */
class WLP
{

public:

    /**
     * Constructor
     * @param prot Pointer to the BuiltinProtocols object.
     */
    WLP(
            BuiltinProtocols*)
    {
    }

    virtual ~WLP()
    {
    }

    MOCK_METHOD0(assert_liveliness_manual_by_participant, bool());

    MOCK_METHOD1(removeRemoteEndpoints, void(ParticipantProxyData * pdata));

    MOCK_METHOD3(assert_liveliness, bool(
                GUID_t writer,
                dds::LivelinessQosPolicyKind kind,
                dds::Duration_t lease_duration));
};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* FASTDDS_RTPS_BUILTIN_LIVELINESS__WLP_HPP */
