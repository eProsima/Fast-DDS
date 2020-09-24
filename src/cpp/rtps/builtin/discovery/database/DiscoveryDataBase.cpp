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

#include <mutex>
#include <shared_mutex>

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
        const eprosima::fastrtps::rtps::GuidPrefix_t& acked_entity)
{
    if (is_participant(change))
    {
        auto it = participants_.find(guid_from_change(change).guidPrefix);
        it->second.add_or_update_ack_participant(acked_entity, true);
    }
}

bool DiscoveryDataBase::update(
        eprosima::fastrtps::rtps::CacheChange_t* change,
        std::string topic_name)
{
    (void)change;
    (void)topic_name;
    return true;
}

const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> DiscoveryDataBase::changes_to_dispose()
{
    // lock(sharing mode) mutex locally
    std::shared_lock<std::shared_timed_mutex> lock(sh_mtx_);
    return disposals_;
}

void DiscoveryDataBase::clear_changes_to_dispose()
{
    // lock(exclusive mode) mutex locally
    std::unique_lock<std::shared_timed_mutex> lock(sh_mtx_);
    disposals_.clear();
}

////////////
// Functions to process_to_send_lists()
const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> DiscoveryDataBase::pdp_to_send()
{
    // lock(sharing mode) mutex locally
    std::shared_lock<std::shared_timed_mutex> lock(sh_mtx_);
    return pdp_to_send_;
}

void DiscoveryDataBase::clear_pdp_to_send()
{
    // lock(exclusive mode) mutex locally
    std::unique_lock<std::shared_timed_mutex> lock(sh_mtx_);
    pdp_to_send_.clear();
}

const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> DiscoveryDataBase::edp_publications_to_send()
{
    // lock(sharing mode) mutex locally
    std::shared_lock<std::shared_timed_mutex> lock(sh_mtx_);
    return edp_publications_to_send_;
}

void DiscoveryDataBase::clear_edp_publications_to_send()
{
    // lock(exclusive mode) mutex locally
    std::unique_lock<std::shared_timed_mutex> lock(sh_mtx_);
    edp_publications_to_send_.clear();
}

const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> DiscoveryDataBase::edp_subscriptions_to_send()
{
    // lock(sharing mode) mutex locally
    std::shared_lock<std::shared_timed_mutex> lock(sh_mtx_);
    return edp_subscriptions_to_send_;
}

void DiscoveryDataBase::clear_edp_subscriptions_to_send()
{
    // lock(exclusive mode) mutex locally
    std::unique_lock<std::shared_timed_mutex> lock(sh_mtx_);
    edp_subscriptions_to_send_.clear();
}

bool DiscoveryDataBase::process_data_queue()
{
    // std::unique_lock<std::mutex> guard(sh_mutex);
    bool is_dirty_topic = false;

    data_queue_.Swap();

    while (!data_queue_.Empty())
    {
        DiscoveryDataQueueInfo data_queue_info = data_queue_.Front();
        eprosima::fastrtps::rtps::CacheChange_t* change = data_queue_info.cache_change();
        eprosima::fastrtps::string_255 topic_name = data_queue_info.topic_name();

        if (change->kind == eprosima::fastrtps::rtps::ALIVE)
        {
            if (is_participant(change))
            {
                create_participant_from_change(change);
            }
            else if (is_writer(change))
            {
                create_writers_from_change(change, topic_name);
            }
            else if (is_reader(change))
            {
                create_readers_from_change(change, topic_name);
            }

            if(std::find(dirty_topics_.begin(), dirty_topics_.end(), topic_name) == dirty_topics_.end())
            {
                dirty_topics_.push_back(topic_name);
                is_dirty_topic = true;
            }
        }
        else
        {
            if (is_participant(change))
            {
                process_dispose_participant(change);
            }
            else if (is_writer(change))
            {
                process_dispose_writer(change, topic_name);
            }
            else if (is_reader(change))
            {
                process_dispose_reader(change, topic_name);
            }
        }

        data_queue_.Pop();
    }


    return false;
}

bool DiscoveryDataBase::process_dirty_topics()
{
    return true;
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

bool DiscoveryDataBase::is_participant(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    (void) ch;
    return true;
}

bool DiscoveryDataBase::is_writer(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    (void) ch;
    return true;
}

bool DiscoveryDataBase::is_reader(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    (void) ch;
    return true;
}

eprosima::fastrtps::rtps::GUID_t DiscoveryDataBase::guid_from_change(
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
        db_->add_ack_(change_, reader_proxy->guid().guidPrefix);
    }
    pending_ |= !is_acked;
}

static bool DiscoveryDataBase::is_participant(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    return eprosima::fastrtps::rtps::c_EntityId_RTPSParticipant == ch->guid_from_change.entityId;
}

static bool DiscoveryDataBase::is_writer(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    constexpr uint8_t entity_id_is_writer_bit = 0x04;
    const eprosima::fastrtps::rtps::GUID_t& change_guid = guid_from_cache(ch);

    return ((guid_from_cache(ch).entityId.value[3] & entity_id_is_writer_bit) != 0);
}

static bool DiscoveryDataBase::is_reader(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    constexpr uint8_t entity_id_is_reader_bit = 0x04;
    const eprosima::fastrtps::rtps::GUID_t& change_guid = guid_from_cache(ch);

    return ((guid_from_cache(ch).entityId.value[3] & entity_id_is_reader_bit) != 0);
}

void DiscoveryDataBase::mark_dispose_writer(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    const eprosima::fastrtps::rtps::GUID_t& writer_guid = guid_from_cache(ch);

    auto wit = writers_.find(writer_guid);
    if (wit != writers_.end())
    {
        wit->second.cache_change() = ch;
    }

    auto pit = participants_.find(writer_guid.guidPrefix);
    if (pit != participants_.end())
    {
        pit->second.writers.erase(
                std::remove(pit->second.writers.begin(), pit->second.writers.end(), writer_guid),
                pit->second.writers.end());
    }

    auto wit = writers_by_topic_.find(writer_guid);
    if (wit != writers_by_topic_.end())
    {
        wit->second.erase(
                std::remove(wit->second.begin(), wit->second.end(), writer_guid),
                wit->second.end());
    }

    if(std::find(disposals_.begin(), disposals_.end(), ch) == disposals_.end())
    {
        disposals_.push_back(ch);
    }

}

void DiscoveryDataBase::process_dispose_participant(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    const eprosima::fastrtps::rtps::GUID_t& participant_guid = guid_from_cache(ch);

    // Change DATA(p) with DATA(Up) in participants map
    auto pit = participants_.find(participant_guid.guidPrefix);
    if (pit != participants_.end())
    {
        pit->second.cache_change() = ch;
    }

    // Delete entries from writers_/readers_ belonging to the participant
    for (auto wit = writers_.begin(); wit != writers_.end(); ++wit)
    {
        if (wit->first.guidPrefix == participant_guid.guidPrefix)
        {
            writers_.erase(wit->first);
            --wit;
        }
    }

    for (auto rit = readers_.begin(); rit != readers_.end(); ++rit)
    {
        if (rit->first.guidPrefix == participant_guid.guidPrefix)
        {
            readers_.erase(rit->first);
            --rit;
        }
    }

    // Delete Participant entries from readers_by_topic and writers_by_topic
    for (auto tit = readers_by_topic_.begin(); tit != readers_by_topic_.end(); ++tit)
    {
        for (auto rit = tit->second.begin(); rit != tit->second.end(); ++rit)
        {
            if(rit->guid_prefix == participant_guid.guid_prefix)
            {
                tit->second.erase(rit);
                --rit;
            }
        }
    }

    for (auto tit = writers_by_topic_.begin(); tit != writers_by_topic_.end(); ++tit)
    {
        for (auto wit = tit->second.begin(); wit != tit->second.end(); ++wit)
        {
            if(wit->guid_prefix == participant_guid.guid_prefix)
            {
                tit->second.erase(wit);
                --wit;
            }
        }
    }

    // Remove participant from others participants_[]::relevant_participants_builtin_ack_status
    for (auto pit = participants_.begin(); pit != participants_.end(); ++pit)
    {
        pit->second.relevant_participants_builtin_ack_status.erase(participant_guid);
    }

    // Remove participant from others writers_[]::relevant_participants_builtin_ack_status
    for (auto wit = writers_.begin(); wit != writers_.end(); ++wit)
    {
        wit->second.relevant_participants_builtin_ack_status.erase(participant_guid);
    }

    // Remove participant from others readers_[]::relevant_participants_builtin_ack_status
    for (auto pit = readers_.begin(); pit != readers_.end(); ++pit)
    {
        pit->second.relevant_participants_builtin_ack_status.erase(participant_guid);
    }

    if(std::find(disposals_.begin(), disposals_.end(), ch) == disposals_.end())
    {
        disposals_.push_back(ch);
    }
}

void DiscoveryDataBase::create_participant_from_change(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    participants_.insert(guid_from_cache(ch).guidPrefix, new DiscoveryParticipantInfo(ch));
}

void DiscoveryDataBase::create_writers_from_change(
        const eprosima::fastrtps::rtps::CacheChange_t* ch,
        const eprosima::fastrtps::string_255& topic_name)
{
    const eprosima::fastrtps::rtps::GUID_t& writer_guid = guid_from_cache(ch);

    bool ret = writers_.insert(writer_guid, new DiscoveryEndpointInfo(ch, topic_name));

    if (ret.second) {
        auto readers_it = readers_by_topic_.find(topic_name);
        if (readers_it != readers_by_topic_.end())
        {
            for (auto reader_it: readers_it->second)
            {
                writers_.find(guid_from_cache(ch))->second.add_participant(reader_it->guidPrefix);

                auto rit = readers_.find(reader_it);
                if (rit != readers_.end())
                {
                    rit->second.add_participant(writer_guid.guidPrefix);
                }

                auto pit = participants_.find(reader_it->guidPrefix);
                if (pit != participants_.end())
                {
                    pit->second.add_participant(writer_guid.guidPrefix);
                }
            }
        }

        auto pit = participants_.find(writer_guid.guidPrefix);
        if (pit != participants_.end())
        {
            pit->second.writers.push_back(writer_guid);
        }

        writers_by_topic[topic_name].push_back(writer_guid);
    }
}

void DiscoveryDataBase::create_readers_from_change(
        const eprosima::fastrtps::rtps::CacheChange_t* ch,
        const eprosima::fastrtps::string_255& topic_name)
{
    const eprosima::fastrtps::rtps::GUID_t& reader_guid = guid_from_cache(ch);

    bool ret = readers_.insert(reader_guid, new DiscoveryEndpointInfo(ch, topic_name));

    if (ret.second) {
        auto writers_it = writers_by_topic_.find(topic_name);
        if (writers_it != writers_by_topic_.end())
        {
            for (auto writer_it: writers_it->second)
            {
                readers_.find(reader_guid)->second.add_participant(writer_it->guidPrefix);

                auto rit = writers_.find(writer_it);
                if (rit != writers_.end())
                {
                    rit->second.add_participant(reader_guid.guidPrefix);
                }

                auto pit = participants_.find(writer_it->guidPrefix);
                if (pit != participants_.end())
                {
                    pit->second.add_participant(reader_guid.guidPrefix);
                }
            }

        }

        auto pit = participants_.find(reader_guid.guidPrefix);
        if (pit != participants_.end())
        {
            pit->second.readers.push_back(reader_guid);
        }

        readers_by_topic[topic_name].push_back(reader_guid);
    }
}


} // namespace ddb
} // namespace rtps
} // namespace fastdds
} // namespace eprosima
