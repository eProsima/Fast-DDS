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
 * @file DiscoveryEndpointInfo.hpp
 *
 */

#ifndef _FASTDDS_RTPS_DISCOVERY_ENDPOINT_INFO_H_
#define _FASTDDS_RTPS_DISCOVERY_ENDPOINT_INFO_H_

#include <rtps/builtin/discovery/database/DiscoverySharedInfo.hpp>

#include <fastcdr/cdr/fixed_size_string.hpp>

#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>

#include <nlohmann/json.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

/**
 * Class to join the main info required from a reader or writer in the Discovery Data Base
 *@ingroup DISCOVERY_MODULE
 */
class DiscoveryEndpointInfo : public DiscoverySharedInfo
{

public:

    DiscoveryEndpointInfo(
            CacheChange_t* change,
            std::string topic,
            bool is_virtual,
            const GuidPrefix_t& known_participant)
        : DiscoverySharedInfo(change, known_participant)
        , topic_(topic)
        , is_virtual_(is_virtual)
    {
    }

    ~DiscoveryEndpointInfo()
    {
    }

    const std::string topic()
    {
        return topic_;
    }

    void is_virtual(
            bool is_virtual)
    {
        is_virtual_ = is_virtual;
    }

    bool is_virtual()
    {
        return is_virtual_;
    }

    void to_json(
            nlohmann::json& j) const
    {
        DiscoverySharedInfo::to_json(j);
        j["topic"] = topic_;
    }

private:

    std::string topic_;
    bool is_virtual_;

};

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_ENDPOINT_INFO_H_ */
