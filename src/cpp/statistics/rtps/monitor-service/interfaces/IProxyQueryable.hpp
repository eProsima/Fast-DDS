// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file IProxyQueryable.hpp
 *
 */

#ifndef _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_IPROXYQUERYABLE_HPP_
#define _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_IPROXYQUERYABLE_HPP_

#include <vector>

#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/Guid.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

struct IProxyQueryable
{
    /**
     * @brief Interface for retrieving the serialized
     * proxy of an entity. This is in the form of
     * a sequence of octets.
     *
     * @param [in] guid The GUID_t identifying the target entity
     * @param [out] msg Pointer containig the serialized proxy
     * @return Whether the operation succeeds or not
     */
    virtual bool get_serialized_proxy(
            const fastdds::rtps::GUID_t& guid,
            fastdds::rtps::CDRMessage_t* msg) = 0;

    /**
     * @brief Interface for retrieving all the guids of the
     * local entities. This includes the participant
     * and user endpoints
     *
     * @param [out] guids The collections of GUID_t of the local entities
     */
    virtual bool get_all_local_proxies(
            std::vector<fastdds::rtps::GUID_t>& guids) = 0;
};

} // rtps
} // statistics
} // fastdds
} // eprosima

#endif // _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_IPROXYQUERYABLE_HPP_

