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

DiscoveryDataBase::DiscoveryDataBase(
        fastrtps::rtps::GuidPrefix_t server_guid_prefix)
    : server_guid_prefix_(server_guid_prefix)
{
}

bool DiscoveryDataBase::pdp_is_relevant(
        const eprosima::fastrtps::rtps::CacheChange_t& change,
        const eprosima::fastrtps::rtps::GUID_t& reader_guid) const
{
    // Lock(shared mode) mutex locally
    std::shared_lock<std::shared_timed_mutex> lock(sh_mtx_);

    // Get identity of the participant that generated the DATA(p|Up)
    fastrtps::rtps::GuidPrefix_t change_guid_prefix = guid_from_change(&change).guidPrefix;

    // Own DATA(p|Up) is always relevant
    if (server_guid_prefix_ == change_guid_prefix)
    {
        return true;
    }

    auto it = participants_.find(change_guid_prefix);
    if (it != participants_.end())
    {
        // it is relevant if the ack has not been received yet
        // in NOT_ALIVE case the set_disposal unmatches every participant
        return !it->second.is_matched(reader_guid.guidPrefix);
    }
    // not relevant
    return false;
}

bool DiscoveryDataBase::edp_publications_is_relevant(
        const eprosima::fastrtps::rtps::CacheChange_t& change,
        const eprosima::fastrtps::rtps::GUID_t& reader_guid) const
{
    // Lock(shared mode) mutex locally
    std::shared_lock<std::shared_timed_mutex> lock(sh_mtx_);

    auto itp = participants_.find(guid_from_change(&change).guidPrefix);
    if (itp == participants_.end())
    {
        // not relevant
        return false;
    }
    else if (!itp->second.is_matched(reader_guid.guidPrefix))
    {
        // Reader client hasn't matched with the writer client
        return false;
    }

    auto itw = writers_.find(guid_from_change(&change));
    if (itw != writers_.end())
    {
        // it is relevant if the ack has not been received yet
        return !itw->second.is_matched(reader_guid.guidPrefix);
    }
    // not relevant
    return false;
}

bool DiscoveryDataBase::edp_subscriptions_is_relevant(
        const eprosima::fastrtps::rtps::CacheChange_t& change,
        const eprosima::fastrtps::rtps::GUID_t& reader_guid) const
{
    // Lock(shared mode) mutex locally
    std::shared_lock<std::shared_timed_mutex> lock(sh_mtx_);

    auto itp = participants_.find(guid_from_change(&change).guidPrefix);
    if (itp == participants_.end())
    {
        // not relevant
        return false;
    }
    else if (!itp->second.is_matched(reader_guid.guidPrefix))
    {
        // participant still not matched
        return false;
    }

    auto itr = readers_.find(guid_from_change(&change));
    if (itr != readers_.end())
    {
        // it is relevant if the ack has not been received yet
        return !itr->second.is_matched(reader_guid.guidPrefix);
    }
    // not relevant
    return false;
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
        eprosima::fastrtps::string_255 topic_name)
{
    //  add the data to the queue to process
    data_queue_.Push(eprosima::fastdds::rtps::ddb::DiscoveryDataQueueInfo(change, topic_name));

    // not way to check if is an error
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

////////////
// Functions to process_data_queue()
bool DiscoveryDataBase::process_data_queue()
{
    bool is_dirty_topic = false;

    // Lock(exclusive mode) mutex locally
    std::unique_lock<std::shared_timed_mutex> lock(sh_mtx_);

    // Swap DATA queues
    data_queue_.Swap();

    eprosima::fastrtps::rtps::CacheChange_t* change;
    eprosima::fastrtps::string_255 topic_name;

    // Process all messages in the queque
    while (!data_queue_.Empty())
    {
        // Process each message with Front()
        DiscoveryDataQueueInfo data_queue_info = data_queue_.Front();
        change = data_queue_info.change();
        topic_name = data_queue_info.topic();

        // If the change is a DATA(p|w|r)
        if (change->kind == eprosima::fastrtps::rtps::ALIVE)
        {
            // DATA(p) case
            if (is_participant(change))
            {
                // Update participants map
                create_participant_from_change(change);
            }
            // DATA(w) case
            else if (is_writer(change))
            {
                create_writers_from_change(change, topic_name);
            }
            // DATA(r) case
            else if (is_reader(change))
            {
                create_readers_from_change(change, topic_name);
            }
            // Update set of dirty_topics
            if (std::find(dirty_topics_.begin(), dirty_topics_.end(), topic_name) == dirty_topics_.end())
            {
                dirty_topics_.push_back(topic_name);
                is_dirty_topic = true;
            }
        }
        // If the change is a DATA(Up|Uw|Ur)
        else
        {
            // DATA(Up) case
            if (is_participant(change))
            {
                process_dispose_participant(change);
            }
            // DATA(Uw) case
            else if (is_writer(change))
            {
                process_dispose_writer(change, topic_name);
            }
            // DATA(Ur) case
            else if (is_reader(change))
            {
                process_dispose_reader(change, topic_name);
            }
        }

        // Pop the message from the queue
        data_queue_.Pop();
    }

    return is_dirty_topic;
}

void DiscoveryDataBase::create_participant_from_change(
        eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    DiscoveryParticipantInfo part(ch);
    participants_.insert(std::make_pair(guid_from_change(ch).guidPrefix, part));
}

void DiscoveryDataBase::create_writers_from_change(
        eprosima::fastrtps::rtps::CacheChange_t* ch,
        const eprosima::fastrtps::string_255& topic_name)
{
    const eprosima::fastrtps::rtps::GUID_t& writer_guid = guid_from_change(ch);

    DiscoveryEndpointInfo tmp_writer(ch, topic_name);
    std::pair<std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator, bool> ret =
            writers_.insert(std::make_pair(writer_guid, tmp_writer));

    // If writer does not exists, create the change
    if (ret.second)
    {
        std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator writer_it =
                writers_.find(writer_guid);

        std::map<eprosima::fastrtps::string_255, std::vector<eprosima::fastrtps::rtps::GUID_t> >::iterator readers_it =
                readers_by_topic_.find(topic_name);
        if (readers_it != readers_by_topic_.end())
        {
            for (eprosima::fastrtps::rtps::GUID_t reader_it: readers_it->second)
            {
                // Update the participant ack status list from writers_
                writer_it->second.add_or_update_ack_participant(reader_it.guidPrefix);

                // Update the participant ack status list from readers_
                std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator rit =
                        readers_.find(reader_it);
                if (rit != readers_.end())
                {
                    rit->second.add_or_update_ack_participant(writer_guid.guidPrefix);
                }

                // Update the participant ack status list from participants_
                std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
                        participants_.find(reader_it.guidPrefix);
                if (pit != participants_.end())
                {
                    if (!pit->second.is_matched(writer_guid.guidPrefix))
                    {
                        pit->second.add_or_update_ack_participant(writer_guid.guidPrefix);
                    }
                }
            }
        }

        // Update participants_::writers
        std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
                participants_.find(writer_guid.guidPrefix);
        if (pit != participants_.end())
        {
            pit->second.add_writer(writer_guid);
        }

        // Update writers_by_topic
        std::map<eprosima::fastrtps::string_255, std::vector<eprosima::fastrtps::rtps::GUID_t> >::iterator topic_it =
                writers_by_topic_.find(topic_name);
        if (topic_it != writers_by_topic_.end())
        {
            std::vector<eprosima::fastrtps::rtps::GUID_t>::iterator writer_by_topic_it =
                    std::find(topic_it->second.begin(), topic_it->second.end(), writer_guid);
            if (writer_by_topic_it == topic_it->second.end())
            {
                topic_it->second.push_back(writer_guid);
            }
            else
            {
                *writer_by_topic_it = writer_guid;
            }
        }
    }
    // If writer exists, update the change and set all participant ack status to false
    else
    {
        ret.first->second.set_change_and_unmatch(ch);
    }

}

void DiscoveryDataBase::create_readers_from_change(
        eprosima::fastrtps::rtps::CacheChange_t* ch,
        const eprosima::fastrtps::string_255& topic_name)
{
    const eprosima::fastrtps::rtps::GUID_t& reader_guid = guid_from_change(ch);

    DiscoveryEndpointInfo tmp_reader(ch, topic_name);
    std::pair<std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator, bool> ret =
            readers_.insert(std::make_pair(reader_guid, tmp_reader));

    // If reader does not exists, create the change
    if (ret.second)
    {
        std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator reader_it =
                readers_.find(reader_guid);

        std::map<eprosima::fastrtps::string_255, std::vector<eprosima::fastrtps::rtps::GUID_t> >::iterator writers_it =
                writers_by_topic_.find(topic_name);
        if (writers_it != writers_by_topic_.end())
        {
            for (eprosima::fastrtps::rtps::GUID_t writer_it: writers_it->second)
            {
                // Update the participant ack status list from readers_
                reader_it->second.add_or_update_ack_participant(writer_it.guidPrefix);

                // Update the participant ack status list from writers_
                std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator wit =
                        writers_.find(writer_it);
                if (wit != writers_.end())
                {
                    wit->second.add_or_update_ack_participant(reader_guid.guidPrefix);
                }

                // Update the participant ack status list from participants_
                std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
                        participants_.find(writer_it.guidPrefix);
                if (pit != participants_.end())
                {
                    if (!pit->second.is_matched(reader_guid.guidPrefix))
                    {
                        pit->second.add_or_update_ack_participant(reader_guid.guidPrefix);
                    }
                }
            }

        }
        // Update participants_::readers
        std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
                participants_.find(reader_guid.guidPrefix);
        if (pit != participants_.end())
        {
            pit->second.add_reader(reader_guid);
        }
        // Update readers_by_topic
        std::map<eprosima::fastrtps::string_255, std::vector<eprosima::fastrtps::rtps::GUID_t> >::iterator topic_it =
                readers_by_topic_.find(topic_name);
        if (topic_it != readers_by_topic_.end())
        {
            std::vector<eprosima::fastrtps::rtps::GUID_t>::iterator reader_by_topic_it =
                    std::find(topic_it->second.begin(), topic_it->second.end(), reader_guid);
            if (reader_by_topic_it == topic_it->second.end())
            {
                topic_it->second.push_back(reader_guid);
            }
            else
            {
                *reader_by_topic_it = reader_guid;
            }
        }
    }
    // If reader exists, update the change and set all participant ack status to false
    else
    {
        ret.first->second.set_change_and_unmatch(ch);
    }
}

void DiscoveryDataBase::process_dispose_participant(
        eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    const eprosima::fastrtps::rtps::GUID_t& participant_guid = guid_from_change(ch);

    // Change DATA(p) with DATA(Up) in participants map
    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
            participants_.find(participant_guid.guidPrefix);
    if (pit != participants_.end())
    {
        pit->second.set_disposal(ch);
    }

    // Delete entries from writers_ belonging to the participant
    for (auto wit = writers_.begin(); wit != writers_.end(); ++wit)
    {
        if (wit->first.guidPrefix == participant_guid.guidPrefix)
        {
            writers_.erase(wit->first);
            --wit;
        }
    }

    // Delete entries from readers_ belonging to the participant
    for (auto rit = readers_.begin(); rit != readers_.end(); ++rit)
    {
        if (rit->first.guidPrefix == participant_guid.guidPrefix)
        {
            readers_.erase(rit->first);
            --rit;
        }
    }

    // Delete Participant entries from writers_by_topic
    for (auto tit = writers_by_topic_.begin(); tit != writers_by_topic_.end(); ++tit)
    {
        for (auto wit = tit->second.begin(); wit != tit->second.end(); ++wit)
        {
            if (wit->guidPrefix == participant_guid.guidPrefix)
            {
                tit->second.erase(wit);
                --wit;
            }
        }

        if (tit->second.empty())
        {
            writers_by_topic_.erase(tit);
            --tit;
        }
    }

    // Delete Participant entries from readers_by_topic
    for (auto tit = readers_by_topic_.begin(); tit != readers_by_topic_.end(); ++tit)
    {
        for (auto rit = tit->second.begin(); rit != tit->second.end(); ++rit)
        {
            if (rit->guidPrefix == participant_guid.guidPrefix)
            {
                tit->second.erase(rit);
                --rit;
            }
        }

        if (tit->second.empty())
        {
            readers_by_topic_.erase(tit);
            --tit;
        }
    }

    // Remove participant from others participants_[]::relevant_participants_builtin_ack_status
    for (auto pit = participants_.begin(); pit != participants_.end(); ++pit)
    {
        pit->second.remove_participant(participant_guid.guidPrefix);
    }

    // Remove participant from others writers_[]::relevant_participants_builtin_ack_status
    for (auto wit = writers_.begin(); wit != writers_.end(); ++wit)
    {
        wit->second.remove_participant(participant_guid.guidPrefix);
    }

    // Remove participant from others readers_[]::relevant_participants_builtin_ack_status
    for (auto pit = readers_.begin(); pit != readers_.end(); ++pit)
    {
        pit->second.remove_participant(participant_guid.guidPrefix);
    }

    // Add entry to disposals_
    if (std::find(disposals_.begin(), disposals_.end(), ch) == disposals_.end())
    {
        disposals_.push_back(ch);
    }
}

void DiscoveryDataBase::process_dispose_writer(
        eprosima::fastrtps::rtps::CacheChange_t* ch,
        const eprosima::fastrtps::string_255& topic_name)
{
    const eprosima::fastrtps::rtps::GUID_t& writer_guid = guid_from_change(ch);

    // Change DATA(w) with DATA(Uw)
    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator wit = writers_.find(writer_guid);
    if (wit != writers_.end())
    {
        wit->second.set_disposal(ch);
    }

    // Update own entry participants_::writers
    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
            participants_.find(writer_guid.guidPrefix);
    if (pit != participants_.end())
    {
        pit->second.remove_writer(writer_guid);
    }

    // Update own entry writers_by_topic_
    std::map<eprosima::fastrtps::string_255, std::vector<eprosima::fastrtps::rtps::GUID_t> >::iterator tit =
            writers_by_topic_.find(topic_name);
    if (tit != writers_by_topic_.end())
    {
        for (std::vector<eprosima::fastrtps::rtps::GUID_t>::iterator writer_it = tit->second.begin();
                writer_it != tit->second.end();
                ++writer_it)
        {
            if (*writer_it == writer_guid)
            {
                tit->second.erase(writer_it);
                break;
            }
        }

        if (tit->second.empty())
        {
            writers_by_topic_.erase(tit);
            --tit;
        }
    }

    // Add entry to disposals_
    if (std::find(disposals_.begin(), disposals_.end(), ch) == disposals_.end())
    {
        disposals_.push_back(ch);
    }

}

void DiscoveryDataBase::process_dispose_reader(
        eprosima::fastrtps::rtps::CacheChange_t* ch,
        const eprosima::fastrtps::string_255& topic_name)
{
    const eprosima::fastrtps::rtps::GUID_t& reader_guid = guid_from_change(ch);

    // Change DATA(r) with DATA(Ur)
    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator rit = readers_.find(reader_guid);
    if (rit != readers_.end())
    {
        rit->second.set_disposal(ch);
    }

    // Update own entry participants_::readers
    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
            participants_.find(reader_guid.guidPrefix);
    if (pit != participants_.end())
    {
        pit->second.remove_reader(reader_guid);
    }

    // Update own entry readers_by_topic_
    std::map<eprosima::fastrtps::string_255, std::vector<eprosima::fastrtps::rtps::GUID_t> >::iterator tit =
            readers_by_topic_.find(topic_name);
    if (tit != readers_by_topic_.end())
    {
        for (std::vector<eprosima::fastrtps::rtps::GUID_t>::iterator reader_it = tit->second.begin();
                reader_it != tit->second.end();
                ++reader_it)
        {
            if (*reader_it == reader_guid)
            {
                tit->second.erase(reader_it);
                break;
            }
        }

        if (tit->second.empty())
        {
            readers_by_topic_.erase(tit);
            --tit;
        }
    }

    // Add entry to disposals_
    if (std::find(disposals_.begin(), disposals_.end(), ch) == disposals_.end())
    {
        disposals_.push_back(ch);
    }
}

bool DiscoveryDataBase::process_dirty_topics()
{
    // Get shared lock
    std::shared_lock<std::shared_timed_mutex> lock(sh_mtx_);

    // Iterator objects are declared here because they are reused in each iteration of the loops
    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator parts_reader_it;
    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator parts_writer_it;
    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator readers_it;
    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator writers_it;

    // Iterate over dirty_topics_
    for (auto topic_it = dirty_topics_.begin(); topic_it != dirty_topics_.end();)
    {
        // Flag to store whether a topic can be cleared.
        bool is_clearable = true;

        // Get all the writers in the topic
        std::vector<fastrtps::rtps::GUID_t> writers = writers_by_topic_[*topic_it];
        // Get all the readers in the topic
        std::vector<fastrtps::rtps::GUID_t> readers = readers_by_topic_[*topic_it];

        for (fastrtps::rtps::GUID_t writer: writers)
        // Iterate over writers in the topic:
        {
            // Iterate over readers in the topic:
            for (fastrtps::rtps::GUID_t reader : readers)
            {
                // Find participants with writer info and participant with reader info in participants_
                parts_reader_it = participants_.find(reader.guidPrefix);
                parts_writer_it = participants_.find(writer.guidPrefix);
                // Find reader info in readers_
                readers_it = readers_.find(reader);
                // Find writer info in writers_
                writers_it = writers_.find(writer);

                // Check in `participants_` whether the client with the reader has acknowledge the PDP of the client
                // with the writer.
                if (parts_reader_it != participants_.end() && parts_reader_it->second.is_matched(writer.guidPrefix))
                {
                    // Check the status of the writer in `readers_[reader]::relevant_participants_builtin_ack_status`.
                    if (readers_it != readers_.end() && !readers_it->second.is_matched(writer.guidPrefix))
                    {
                        // If the status is 0, add DATA(w) to a `edp_publications_to_send_` (if it's not there).
                        if (std::find(
                                    edp_publications_to_send_.begin(),
                                    edp_publications_to_send_.end(),
                                    writers_it->second.change()) == edp_publications_to_send_.end())
                        {
                            edp_publications_to_send_.push_back(writers_it->second.change());
                        }
                    }
                }
                else
                {
                    // Add DATA(p) of the client with the writer to `pdp_to_send_` (if it's not there).
                    if (std::find(
                                pdp_to_send_.begin(),
                                pdp_to_send_.end(),
                                parts_writer_it->second.change()) == pdp_to_send_.end())
                    {
                        pdp_to_send_.push_back(parts_writer_it->second.change());
                    }
                    // Set topic as not-clearable.
                    is_clearable = false;
                }

                // Check in `participants_` whether the client with the writer has acknowledge the PDP of the client
                // with the reader.
                if (parts_writer_it != participants_.end() && parts_writer_it->second.is_matched(reader.guidPrefix))
                {
                    // Check the status of the reader in `writers_[writer]::relevant_participants_builtin_ack_status`.
                    if (writers_it != writers_.end() && !writers_it->second.is_matched(reader.guidPrefix))
                    {
                        // If the status is 0, add DATA(r) to a `edp_subscriptions_to_send_` (if it's not there).
                        if (std::find(
                                    edp_subscriptions_to_send_.begin(),
                                    edp_subscriptions_to_send_.end(),
                                    readers_it->second.change()) == edp_subscriptions_to_send_.end())
                        {
                            edp_subscriptions_to_send_.push_back(readers_it->second.change());
                        }
                    }
                }
                else
                {
                    // Add DATA(p) of the client with the reader to `pdp_to_send_` (if it's not there).
                    if (std::find(
                                pdp_to_send_.begin(),
                                pdp_to_send_.end(),
                                parts_reader_it->second.change()) == pdp_to_send_.end())
                    {
                        pdp_to_send_.push_back(parts_reader_it->second.change());
                    }
                    // Set topic as not-clearable.
                    is_clearable = false;
                }
            }
        }

        // Check whether the topic is still dirty or it can be cleared
        if (is_clearable)
        {
            // Delete topic from dirty_topics_
            topic_it = dirty_topics_.erase(topic_it);
        }
        else
        {
            // Proceed with next topic
            ++topic_it;
        }
    }

    // Return whether there still are dirty topics
    return dirty_topics_.empty();
}

bool DiscoveryDataBase::delete_entity_of_change(
        fastrtps::rtps::CacheChange_t* change)
{
    // Lock(exclusive mode) mutex locally
    std::unique_lock<std::shared_timed_mutex> lock(sh_mtx_);

    if (change->kind == fastrtps::rtps::ChangeKind_t::ALIVE)
    {
        logWarning(DISCOVERY_DATABASE,
                "Attempting to delete information of an ALIVE entity: " << guid_from_change(change));
        return false;
    }

    if (is_participant(change))
    {
        // The information related to this participant is cleaned up in process_data_queue
        return participants_.erase(guid_from_change(change).guidPrefix);
    }
    else if (is_reader(change))
    {
        // The information related to this reader is cleaned up in process_data_queue
        return readers_.erase(guid_from_change(change));
    }
    else if (is_writer(change))
    {
        // The information related to this writer is cleaned up in process_data_queue
        return writers_.erase(guid_from_change(change));
    }
    return false;
}

bool DiscoveryDataBase::is_participant(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    return eprosima::fastrtps::rtps::c_EntityId_RTPSParticipant == guid_from_change(ch).entityId;
}

bool DiscoveryDataBase::is_writer(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    constexpr uint8_t entity_id_is_writer_bit = 0x04;

    return ((guid_from_change(ch).entityId.value[3] & entity_id_is_writer_bit) != 0);
}

bool DiscoveryDataBase::is_reader(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    constexpr uint8_t entity_id_is_reader_bit = 0x04;

    return ((guid_from_change(ch).entityId.value[3] & entity_id_is_reader_bit) != 0);
}

eprosima::fastrtps::rtps::GUID_t DiscoveryDataBase::guid_from_change(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    return fastrtps::rtps::iHandle2GUID(ch->instanceHandle);
}

fastrtps::rtps::CacheChange_t* DiscoveryDataBase::cache_change_own_participant()
{
    auto part_it = participants_.find(server_guid_prefix_);
    if (part_it != participants_.end())
    {
        return part_it->second.change();
    }
    return nullptr;
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

} // namespace ddb
} // namespace rtps
} // namespace fastdds
} // namespace eprosima
