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
 * @file DiscoveryDataBase.cpp
 *
 */

#include <fastdds/dds/log/Log.hpp>

#include "./DiscoveryDataBase.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

bool DiscoveryDataBase::pdp_is_relevant(
        const eprosima::fastrtps::rtps::CacheChange_t& change,
        const eprosima::fastrtps::rtps::GUID_t& reader_guid) const
{
    (void)change;
    (void)reader_guid;
    return true;
}

bool DiscoveryDataBase::edp_publications_is_relevant(
        const eprosima::fastrtps::rtps::CacheChange_t& change,
        const eprosima::fastrtps::rtps::GUID_t& reader_guid) const
{
    (void)change;
    (void)reader_guid;
    return true;
}

bool DiscoveryDataBase::edp_subscriptions_is_relevant(
        const eprosima::fastrtps::rtps::CacheChange_t& change,
        const eprosima::fastrtps::rtps::GUID_t& reader_guid) const
{
    (void)change;
    (void)reader_guid;
    return true;
}

bool DiscoveryDataBase::process_data_queue()
{
    // std::unique_lock<std::mutex> guard(sh_mutex);
    data_queue_.Swap();
    while (!data_queue_.Empty())
    {
        DiscoveryDataQueueInfo data_queue_info = data_queue_.Front();

        if (data_queue_info.cache_change()->kind == eprosima::fastrtps::rtps::ALIVE)
        {
            // update(participants_);
        }

        data_queue_.Pop();
    }


    return false;
}

bool DiscoveryDataBase::is_builtin_participant_publication(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    return memcmp(
            &eprosima::fastrtps::rtps::c_EntityId_SPDPWriter,
            &ch->write_params.sample_identity().writer_guid().entityId,
            5) == 0;
}

bool DiscoveryDataBase::is_builtin_publications_publication(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    return memcmp(
            &eprosima::fastrtps::rtps::c_EntityId_SEDPPubWriter,
            &ch->write_params.sample_identity().writer_guid().entityId,
            3) == 0;
}

bool DiscoveryDataBase::is_builtin_subscriptions_publication(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    return memcmp(
            &eprosima::fastrtps::rtps::c_EntityId_SEDPPubWriter,
            &ch->write_params.sample_identity().writer_guid().entityId,
            3) == 0;
}

} // namespace ddb
} // namespace rtps
} // namespace fastdds
} // namespace eprosima
