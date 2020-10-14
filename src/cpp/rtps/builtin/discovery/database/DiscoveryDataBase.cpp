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

void DiscoveryDataBase::add_ack_(
        const eprosima::fastrtps::rtps::CacheChange_t* change,
        const eprosima::fastrtps::rtps::GuidPrefix_t* acked_entity)
{
    (void)change;
    (void)acked_entity;
}

bool DiscoveryDataBase::update(
        eprosima::fastrtps::rtps::CacheChange_t* change,
        std::string topic_name,
        eprosima::fastrtps::rtps::GUID_t* entity)
{
    (void)change;
    (void)topic_name;
    (void)entity;
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

bool DiscoveryDataBase::delete_entity_of_change(
        fastrtps::rtps::CacheChange_t* change)
{
    (void)change;
    return true;
    /*
       if (change->kind != fastrtps::rtps::ChangeKind_t::ALIVE)
       {
        logWarning(DISCOVERY_DATABASE, "Attempting to delete information of an ALIVE entity: " << guid_from_change_(change));
        return false;
       }

       if (DiscoveryDataBase::is_participant_(change))
       {
        return DiscoveryDataBase::remove_participant_(change);
       }
       else if (is_reader_(change))
       {
        return remove_reader_(change);
       }
       else if (is_writer_(change))
       {
        return remove_writer_(change);
       }
       return false;
     */
}

eprosima::fastrtps::rtps::GUID_t DiscoveryDataBase::guid_from_change_(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    return fastrtps::rtps::iHandle2GUID(ch->instanceHandle);
}

DiscoveryDataBase::AckedFunctor::AckedFunctor(
        DiscoveryDataBase* db,
        eprosima::fastrtps::rtps::CacheChange_t* change)
    : db_(db)
    , change_(change)
{
    db_->exclusive_lock_();
}

DiscoveryDataBase::AckedFunctor::~AckedFunctor()
{
    db_->exclusive_unlock_();
}

void DiscoveryDataBase::AckedFunctor::operator () (
        eprosima::fastrtps::rtps::ReaderProxy* reader_proxy)
{
    // Check whether the change has been acknowledged by a given reader
    bool is_acked = reader_proxy->change_is_acked(change_->sequenceNumber);
    if (is_acked)
    {
        // In the discovery database, mark the change as acknowledged by the reader
        db_->add_ack_(change_, &reader_proxy->guid().guidPrefix);
    }
    pending_ |= !is_acked;
}

} // namespace ddb
} // namespace rtps
} // namespace fastdds
} // namespace eprosima
