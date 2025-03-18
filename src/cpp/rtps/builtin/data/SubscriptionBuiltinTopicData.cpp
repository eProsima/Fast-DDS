// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SubscriptionBuiltinTopicData.cpp
 */

#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>

#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

SubscriptionBuiltinTopicData::SubscriptionBuiltinTopicData(
        const size_t max_unicast_locators,
        const size_t max_multicast_locators,
        const VariableLengthDataLimits& data_limits,
        const fastdds::rtps::ContentFilterProperty::AllocationConfiguration& content_filter_limits)
    : content_filter(content_filter_limits)
    , remote_locators(max_unicast_locators, max_multicast_locators)
{
    user_data.set_max_size(data_limits.max_user_data);
    partition.set_max_size(static_cast<uint32_t>(data_limits.max_partitions));
    data_sharing.set_max_domains(static_cast<uint32_t>(data_limits.max_datasharing_domains));
}

}   // namespace rtps
}   // namespace fastdds
}   // namespace eprosima
