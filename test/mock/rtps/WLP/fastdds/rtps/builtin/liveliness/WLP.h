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
 * @file WLP.h
 *
 */

#ifndef _FASTDDS_RTPS_WLP_H_
#define _FASTDDS_RTPS_WLP_H_

#include <vector>
#include <mutex>
#include <gmock/gmock.h>

#include <fastdds/rtps/common/Guid.h>
#include <fastrtps/qos/QosPolicies.h>

namespace eprosima {
namespace fastrtps {
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

    MOCK_METHOD3(assert_liveliness, bool(
            GUID_t writer,
            LivelinessQosPolicyKind kind,
            Duration_t lease_duration));
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_WLP_H_ */
