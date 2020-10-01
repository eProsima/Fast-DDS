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
    , server_acked_by_all_(false)
{
}

bool DiscoveryDataBase::pdp_is_relevant(
        const eprosima::fastrtps::rtps::CacheChange_t& change,
        const eprosima::fastrtps::rtps::GUID_t& reader_guid) const
{

    // Get identity of the participant that generated the DATA(p|Up)
    fastrtps::rtps::GuidPrefix_t change_guid_prefix = guid_from_change(&change).guidPrefix;

    // Own DATA(p|Up) is always relevant for remote PDP readers. Server's PDP ReaderProxy will never
    // be queried for relevance, since Participant's own PDP writer and reader are not matched,
    // and there for there is no ReaderProxy for participant's own PDP reader.
    if (server_guid_prefix_ == change_guid_prefix)
    {
        return true;
    }

    // Lock(shared mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);

    auto it = participants_.find(change_guid_prefix);
    if (it != participants_.end())
    {
        // it is relevant if the ack has not been received yet
        // in NOT_ALIVE case the set_disposal unmatches every participant
        return (it->second.is_relevant_participant(reader_guid.guidPrefix) &&
               !it->second.is_matched(reader_guid.guidPrefix));
    }
    // Not relevant
    return false;
}

bool DiscoveryDataBase::edp_publications_is_relevant(
        const eprosima::fastrtps::rtps::CacheChange_t& change,
        const eprosima::fastrtps::rtps::GUID_t& reader_guid) const
{
    // Get identity of the participant that generated the DATA
    fastrtps::rtps::GUID_t change_guid = guid_from_change(&change);

    // Lock(shared mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);

    auto itp = participants_.find(change_guid.guidPrefix);
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

    auto itw = writers_.find(change_guid);
    if (itw != writers_.end())
    {
        // it is relevant if the ack has not been received yet
        return (itw->second.is_relevant_participant(reader_guid.guidPrefix) &&
               !itw->second.is_matched(reader_guid.guidPrefix));
    }
    // not relevant
    return false;
}

bool DiscoveryDataBase::edp_subscriptions_is_relevant(
        const eprosima::fastrtps::rtps::CacheChange_t& change,
        const eprosima::fastrtps::rtps::GUID_t& reader_guid) const
{
    // Get identity of the participant that generated the DATA
    fastrtps::rtps::GUID_t change_guid = guid_from_change(&change);

    // Lock(shared mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);

    auto itp = participants_.find(change_guid.guidPrefix);
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

    auto itr = readers_.find(change_guid);
    if (itr != readers_.end())
    {
        // it is relevant if the ack has not been received yet
        return (itr->second.is_relevant_participant(reader_guid.guidPrefix) &&
               !itr->second.is_matched(reader_guid.guidPrefix));
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
        logInfo(DISCOVERY_DATABASE,
                "Adding DATA(p) ACK for change " << change->instanceHandle << " to " << acked_entity);
        auto it = participants_.find(guid_from_change(change).guidPrefix);
        if (it != participants_.end())
        {
            // Only add ACK if the change in the database is the same as the incoming change. Else, the change in the
            // database has been updated, so this ACK is not relevant anymore
            if (it->second.change()->write_params.sample_identity() == change->write_params.sample_identity())
            {
                it->second.add_or_update_ack_participant(acked_entity, true);
            }
        }
    }
    else if (is_writer(change))
    {
        logInfo(DISCOVERY_DATABASE,
                "Adding DATA(w) ACK for change " << change->instanceHandle << " to " << acked_entity);
        auto it = writers_.find(guid_from_change(change));
        if (it != writers_.end())
        {
            // Only add ACK if the change in the database is the same as the incoming change. Else, the change in the
            // database has been updated, so this ACK is not relevant anymore
            if (it->second.change()->write_params.sample_identity() == change->write_params.sample_identity())
            {
                it->second.add_or_update_ack_participant(acked_entity, true);
            }
        }
    }
    else if (is_reader(change))
    {
        logInfo(DISCOVERY_DATABASE,
                "Adding DATA(r) ACK for change " << change->instanceHandle << " to " << acked_entity);
        auto it = readers_.find(guid_from_change(change));
        if (it != readers_.end())
        {
            // Only add ACK if the change in the database is the same as the incoming change. Else, the change in the
            // database has been updated, so this ACK is not relevant anymore
            if (it->second.change()->write_params.sample_identity() == change->write_params.sample_identity())
            {
                it->second.add_or_update_ack_participant(acked_entity, true);
            }
        }
    }
}

bool DiscoveryDataBase::update(
        eprosima::fastrtps::rtps::CacheChange_t* change,
        DiscoveryParticipantChangeData participant_change_data)
{
    if (!is_participant(change))
    {
        logError(DISCOVERY_DATABASE, "Change is not a DATA(p|Up): " << change->instanceHandle);
        return false;
    }
    logInfo(DISCOVERY_DATABASE, "Adding DATA(p|Up) to the queue: " << change->instanceHandle);
    //  Add the DATA(p|Up) to the PDP queue to process
    pdp_data_queue_.Push(eprosima::fastdds::rtps::ddb::DiscoveryPDPDataQueueInfo(change, participant_change_data));
    return true;
}

bool DiscoveryDataBase::update(
        eprosima::fastrtps::rtps::CacheChange_t* change,
        std::string topic_name)
{
    if (!is_writer(change) && !is_reader(change))
    {
        logError(DISCOVERY_DATABASE, "Change is not a DATA(w|Uw|r|Ur): " << change->instanceHandle);
        return false;
    }

    logInfo(DISCOVERY_DATABASE, "Adding DATA(w|Uw|r|Ur) to the queue: " << change->instanceHandle);
    //  add the DATA(w|Uw|r|Ur) to the EDP queue to process
    edp_data_queue_.Push(eprosima::fastdds::rtps::ddb::DiscoveryEDPDataQueueInfo(change, topic_name));
    return true;
}

const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> DiscoveryDataBase::changes_to_dispose()
{
    // lock(sharing mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);
    return disposals_;
}

void DiscoveryDataBase::clear_changes_to_dispose()
{
    // lock(exclusive mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);
    disposals_.clear();
}

////////////
// Functions to process_to_send_lists()
const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> DiscoveryDataBase::pdp_to_send()
{
    // lock(sharing mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);
    return pdp_to_send_;
}

void DiscoveryDataBase::clear_pdp_to_send()
{
    // lock(exclusive mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);
    pdp_to_send_.clear();
}

const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> DiscoveryDataBase::edp_publications_to_send()
{
    // lock(sharing mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);
    return edp_publications_to_send_;
}

void DiscoveryDataBase::clear_edp_publications_to_send()
{
    // lock(exclusive mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);
    edp_publications_to_send_.clear();
}

const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> DiscoveryDataBase::edp_subscriptions_to_send()
{
    // lock(sharing mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);
    return edp_subscriptions_to_send_;
}

void DiscoveryDataBase::clear_edp_subscriptions_to_send()
{
    // lock(exclusive mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);
    edp_subscriptions_to_send_.clear();
}

const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> DiscoveryDataBase::changes_to_release()
{
    // lock(sharing mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);
    return changes_to_release_;
}

void DiscoveryDataBase::clear_changes_to_release()
{
    // lock(exclusive mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);
    changes_to_release_.clear();
}

////////////
// Functions to process PDP and EDP data queues
void DiscoveryDataBase::process_pdp_data_queue()
{
    // Lock(exclusive mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);

    // Swap DATA queues
    pdp_data_queue_.Swap();

    // Process all messages in the queque
    while (!pdp_data_queue_.Empty())
    {
        // Process each message with Front()
        DiscoveryPDPDataQueueInfo data_queue_info = pdp_data_queue_.Front();

        // If the change is a DATA(p)
        if (data_queue_info.change()->kind == eprosima::fastrtps::rtps::ALIVE)
        {
            // Update participants map
            logInfo(DISCOVERY_DATABASE, "DATA(p) received from: " << data_queue_info.change()->instanceHandle);
            create_participant_from_change(data_queue_info.change(), data_queue_info.participant_change_data());
        }
        // If the change is a DATA(Up)
        else
        {
            process_dispose_participant(data_queue_info.change());
        }

        // Pop the message from the queue
        pdp_data_queue_.Pop();
    }
}

bool DiscoveryDataBase::process_edp_data_queue()
{
    bool is_dirty_topic = false;

    // Lock(exclusive mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);

    // Swap DATA queues
    edp_data_queue_.Swap();

    eprosima::fastrtps::rtps::CacheChange_t* change;
    std::string topic_name;

    // Process all messages in the queque
    while (!edp_data_queue_.Empty())
    {
        // Process each message with Front()
        DiscoveryEDPDataQueueInfo data_queue_info = edp_data_queue_.Front();
        change = data_queue_info.change();
        topic_name = data_queue_info.topic();

        // If the change is a DATA(w|r)
        if (change->kind == eprosima::fastrtps::rtps::ALIVE)
        {
            logInfo(DISCOVERY_DATABASE, "ALIVE change received from: " << change->instanceHandle);
            // DATA(w) case
            if (is_writer(change))
            {
                logInfo(DISCOVERY_DATABASE, "DATA(w) in topic " << topic_name << " received from: "
                                                                << change->instanceHandle);
                create_writers_from_change(change, topic_name);
            }
            // DATA(r) case
            else if (is_reader(change))
            {
                logInfo(DISCOVERY_DATABASE, "DATA(r) in topic " << topic_name << " received from: "
                                                                << change->instanceHandle);
                create_readers_from_change(change, topic_name);
            }

            // Update set of dirty_topics
            if (std::find(
                        dirty_topics_.begin(),
                        dirty_topics_.end(),
                        topic_name) == dirty_topics_.end())
            {
                logInfo(DISCOVERY_DATABASE, "Setting topic " << topic_name << " as dirty");
                dirty_topics_.push_back(topic_name);
                is_dirty_topic = true;
            }
        }
        // If the change is a DATA(Uw|Ur)
        else
        {
            // DATA(Uw) case
            if (is_writer(change))
            {
                logInfo(DISCOVERY_DATABASE, "DATA(Uw) received from: " << change->instanceHandle);
                process_dispose_writer(change, topic_name);
            }
            // DATA(Ur) case
            else if (is_reader(change))
            {
                logInfo(DISCOVERY_DATABASE, "DATA(Ur) received from: " << change->instanceHandle);
                process_dispose_reader(change, topic_name);
            }
        }

        // Pop the message from the queue
        edp_data_queue_.Pop();
    }

    return is_dirty_topic;
}

void DiscoveryDataBase::create_participant_from_change(
        eprosima::fastrtps::rtps::CacheChange_t* ch,
        const DiscoveryParticipantChangeData& change_data)
{
    DiscoveryParticipantInfo part(ch, server_guid_prefix_, change_data);
    std::pair<std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator, bool> ret =
            participants_.insert(std::make_pair(guid_from_change(ch).guidPrefix, part));

    // If insert resturn false, means that the participant already existed (DATA(p) is an update). In that case
    // we need to update the change related to the participant and return the old change to the pool
    if (!ret.second)
    {
        // Add old change to changes_to_release_
        logInfo(DISCOVERY_DATABASE, "Participant updating. Marking old change to release");
        update_change_and_unmatch_(ch, ret.first->second);
    }
    else
    {
        fastrtps::rtps::GUID_t change_guid = guid_from_change(ch);
        logInfo(DISCOVERY_DATABASE, "New participant added: " << change_guid.guidPrefix);

        // if its our own server guid, we put the DATA(P) in the history
        if (change_guid.guidPrefix == server_guid_prefix_)
        {
            if (std::find(
                        pdp_to_send_.begin(),
                        pdp_to_send_.end(),
                        ch) == pdp_to_send_.end())
            {
                logInfo(DISCOVERY_DATABASE, "Addind Server DATA(p) to send: "
                        << ch->instanceHandle);
                pdp_to_send_.push_back(ch);
            }
        }
        else
        {
            // if it is a new participant, we have to wait to make ack to our DATA(P)
            server_acked_by_all(false);
        }
    }

}

void DiscoveryDataBase::create_writers_from_change(
        eprosima::fastrtps::rtps::CacheChange_t* ch,
        const std::string& topic_name)
{
    const eprosima::fastrtps::rtps::GUID_t& writer_guid = guid_from_change(ch);

    // Update participants_::writers
    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator w_pit =
            participants_.find(writer_guid.guidPrefix);
    if (w_pit != participants_.end())
    {
        w_pit->second.add_writer(writer_guid);
    }

    // Update writers_
    DiscoveryEndpointInfo tmp_writer(ch, topic_name, server_guid_prefix_);

    std::pair<std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator, bool> ret =
            writers_.insert(std::make_pair(writer_guid, tmp_writer));

    // If writer does not exists, create the change
    if (ret.second)
    {
        std::map<std::string, std::vector<eprosima::fastrtps::rtps::GUID_t>>::iterator readers_it =
                readers_by_topic_.find(topic_name);

        if (readers_it != readers_by_topic_.end())
        {
            for (eprosima::fastrtps::rtps::GUID_t reader_it: readers_it->second)
            {
                // Update the participant ack status list from writers_
                ret.first->second.add_or_update_ack_participant(reader_it.guidPrefix);

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

                if (w_pit != participants_.end())
                {
                    if (!w_pit->second.is_matched(reader_it.guidPrefix))
                    {
                        w_pit->second.add_or_update_ack_participant(reader_it.guidPrefix);
                    }
                }
            }
        }

        // Update writers_by_topic
        std::map<std::string, std::vector<eprosima::fastrtps::rtps::GUID_t>>::iterator topic_it =
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
        // This is the first writer in the topic
        else
        {
            std::vector<fastrtps::rtps::GUID_t> writers_in_topic = {writer_guid};
            auto topic_iterator = writers_by_topic_.insert(
                std::pair<std::string, std::vector<fastrtps::rtps::GUID_t>>(topic_name, writers_in_topic));
            if (!topic_iterator.second)
            {
                logError(DISCOVERY_DATABASE, "Could not insert writer " << writer_guid << " in topic " << topic_name);
            }
        }
    }
    // If writer exists, update the change and set all participant ack status to false
    else
    {
        update_change_and_unmatch_(ch, ret.first->second);
    }

}

void DiscoveryDataBase::create_readers_from_change(
        eprosima::fastrtps::rtps::CacheChange_t* ch,
        const std::string& topic_name)
{
    const eprosima::fastrtps::rtps::GUID_t& reader_guid = guid_from_change(ch);

    // Update participants_::readers
    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator r_pit =
            participants_.find(reader_guid.guidPrefix);
    if (r_pit != participants_.end())
    {
        r_pit->second.add_reader(reader_guid);
    }

    DiscoveryEndpointInfo tmp_reader(ch, topic_name, server_guid_prefix_);

    std::pair<std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator, bool> ret =
            readers_.insert(std::make_pair(reader_guid, tmp_reader));

    // If reader does not exists, create the change
    if (ret.second)
    {
        std::map<std::string, std::vector<eprosima::fastrtps::rtps::GUID_t>>::iterator writers_it =
                writers_by_topic_.find(topic_name);
        if (writers_it != writers_by_topic_.end())
        {
            for (eprosima::fastrtps::rtps::GUID_t writer_it: writers_it->second)
            {
                // Update the participant ack status list from readers_
                ret.first->second.add_or_update_ack_participant(writer_it.guidPrefix);

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

                if (r_pit != participants_.end())
                {
                    if (!r_pit->second.is_matched(writer_it.guidPrefix))
                    {
                        r_pit->second.add_or_update_ack_participant(writer_it.guidPrefix);
                    }
                }
            }
        }

        // Update readers_by_topic
        std::map<std::string, std::vector<eprosima::fastrtps::rtps::GUID_t>>::iterator topic_it =
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
        // This is the first reader in the topic
        else
        {
            std::vector<fastrtps::rtps::GUID_t> readers_in_topic = {reader_guid};
            auto topic_iterator = readers_by_topic_.insert(
                std::pair<std::string, std::vector<fastrtps::rtps::GUID_t>>(topic_name, readers_in_topic));
            if (!topic_iterator.second)
            {
                logError(DISCOVERY_DATABASE, "Could not insert reader " << reader_guid << " in topic " << topic_name);
            }
        }
    }
    // If reader exists, update the change and set all participant ack status to false
    else
    {
        update_change_and_unmatch_(ch, ret.first->second);
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
<<<<<<< HEAD
        // Only update DATA(p), leaving the change info untouched. This is because DATA(Up) does not have the
        // participant's meta-information, but we don't want to loose it here.
        changes_to_release_.push_back(pit->second.update_and_unmatch(ch));
    // Delete entries from writers_ belonging to the participant
    {
        if (wit->first.guidPrefix == participant_guid.guidPrefix)
        {
            changes_to_release_.push_back(wit->second.change());
            wit = writers_.erase(wit);
            continue;
        }
        ++wit;
    }

    // Delete entries from readers_ belonging to the participant
    for (auto rit = readers_.begin(); rit != readers_.end();)
    {
        if (rit->first.guidPrefix == participant_guid.guidPrefix)
        {
            changes_to_release_.push_back(rit->second.change());
            rit = readers_.erase(rit);
            continue;
        }
        ++rit;
    }

    // Delete Participant entries from writers_by_topic_
    for (auto tit = writers_by_topic_.begin(); tit != writers_by_topic_.end();)
    {
        for (auto wit = tit->second.begin(); wit != tit->second.end();)
        {
            if (wit->guidPrefix == participant_guid.guidPrefix)
            {
                wit = tit->second.erase(wit);
                continue;
            }
            ++wit;
        }

        if (tit->second.empty())
        {
            tit = writers_by_topic_.erase(tit);
            continue;
        }
        ++tit;
    }

    // Delete Participant entries from readers_by_topic_
    for (auto tit = readers_by_topic_.begin(); tit != readers_by_topic_.end();)
    {
        for (auto rit = tit->second.begin(); rit != tit->second.end();)
        {
            if (rit->guidPrefix == participant_guid.guidPrefix)
            {
                rit = tit->second.erase(rit);
                continue;
            }
            ++rit;
        }

        if (tit->second.empty())
        {
            tit = readers_by_topic_.erase(tit);
            continue;
        }
        ++tit;
    }

    // Remove participant from others participants_[]::relevant_participants_builtin_ack_status
    for (pit = participants_.begin(); pit != participants_.end(); ++pit)
    {
        pit->second.remove_participant(participant_guid.guidPrefix);
    }

    // Remove participant from others writers_[]::relevant_participants_builtin_ack_status
    for (auto wit = writers_.begin(); wit != writers_.end(); ++wit)
    {
        wit->second.remove_participant(participant_guid.guidPrefix);
    }

    // Remove participant from others readers_[]::relevant_participants_builtin_ack_status
    for (auto rit = readers_.begin(); rit != readers_.end(); ++rit)
    {
        rit->second.remove_participant(participant_guid.guidPrefix);
    }

    // Add entry to disposals_
    if (std::find(disposals_.begin(), disposals_.end(), ch) == disposals_.end())
    {
        disposals_.push_back(ch);
    }
}

void DiscoveryDataBase::process_dispose_writer(
        eprosima::fastrtps::rtps::CacheChange_t* ch,
        const std::string& topic_name)
{
    const eprosima::fastrtps::rtps::GUID_t& writer_guid = guid_from_change(ch);

    // Change DATA(w) with DATA(Uw)
    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator wit = writers_.find(writer_guid);
    if (wit != writers_.end())
    {
<<<<<<< HEAD
        changes_to_release_.push_back(wit->second.update_and_unmatch(ch));
=======
        update_change_and_unmatch_(ch, wit->second);
>>>>>>> b701c4bae... Refs #9445: functions signature
    }

    // Update own entry participants_::writers
    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
            participants_.find(writer_guid.guidPrefix);
    if (pit != participants_.end())
    {
        pit->second.remove_writer(writer_guid);
    }

    // Update own entry writers_by_topic_
    std::map<std::string, std::vector<eprosima::fastrtps::rtps::GUID_t>>::iterator tit =
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
        const std::string& topic_name)
{
    const eprosima::fastrtps::rtps::GUID_t& reader_guid = guid_from_change(ch);

    // Change DATA(r) with DATA(Ur)
    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator rit = readers_.find(reader_guid);
    if (rit != readers_.end())
    {
<<<<<<< HEAD
        changes_to_release_.push_back(rit->second.update_and_unmatch(ch));
=======
        update_change_and_unmatch_(ch, rit->second);
>>>>>>> b701c4bae... Refs #9445: functions signature
    }

    // Update own entry participants_::readers
    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
            participants_.find(reader_guid.guidPrefix);
    if (pit != participants_.end())
    {
        pit->second.remove_reader(reader_guid);
    }

    // Update own entry readers_by_topic_
    std::map<std::string, std::vector<eprosima::fastrtps::rtps::GUID_t>>::iterator tit =
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
    // logInfo(DISCOVERY_DATABASE, "process_dirty_topics start");
    // Get shared lock
    std::unique_lock<share_mutex_t> lock(sh_mtx_);

    // Iterator objects are declared here because they are reused in each iteration of the loops
    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator parts_reader_it;
    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator parts_writer_it;
    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator readers_it;
    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator writers_it;

    // Iterate over dirty_topics_
    for (auto topic_it = dirty_topics_.begin(); topic_it != dirty_topics_.end();)
    {
        logInfo(DISCOVERY_DATABASE, "Processing topic: " << *topic_it);
        // Flag to store whether a topic can be cleared.
        bool is_clearable = true;

        // Get all the writers in the topic
        std::vector<fastrtps::rtps::GUID_t> writers;
        auto ret = writers_by_topic_.find(*topic_it);
        if (ret != writers_by_topic_.end())
        {
            writers = ret->second;
        }
        // Get all the readers in the topic
        std::vector<fastrtps::rtps::GUID_t> readers;
        ret = readers_by_topic_.find(*topic_it);
        if (ret != readers_by_topic_.end())
        {
            readers = ret->second;
        }

        for (fastrtps::rtps::GUID_t writer: writers)
        // Iterate over writers in the topic:
        {
            logInfo(DISCOVERY_DATABASE, "[" << *topic_it << "]" << " Processing writer: " << writer);
            // Iterate over readers in the topic:
            for (fastrtps::rtps::GUID_t reader : readers)
            {
                logInfo(DISCOVERY_DATABASE, "[" << *topic_it << "]" << " Processing reader: " << reader);
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
                            logInfo(DISCOVERY_DATABASE, "Addind DATA(w) to send: "
                                    << writers_it->second.change()->instanceHandle);
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
                        logInfo(DISCOVERY_DATABASE, "Addind writer's DATA(p) to send: "
                                << parts_writer_it->second.change()->instanceHandle);
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
                            logInfo(DISCOVERY_DATABASE, "Addind DATA(r) to send: "
                                    << readers_it->second.change()->instanceHandle);
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
                        logInfo(DISCOVERY_DATABASE, "Addind readers's DATA(p) to send: "
                                << parts_reader_it->second.change()->instanceHandle);
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
            logInfo(DISCOVERY_DATABASE, "Topic " << *topic_it << " has been cleaned");
            topic_it = dirty_topics_.erase(topic_it);
        }
        else
        {
            // Proceed with next topic
            logInfo(DISCOVERY_DATABASE, "Topic " << *topic_it << " is still dirty");
            ++topic_it;
        }
    }

    // Return whether there still are dirty topics
    logInfo(DISCOVERY_DATABASE, "Are there dirty topics? " << !dirty_topics_.empty());
    return !dirty_topics_.empty();
}

bool DiscoveryDataBase::delete_entity_of_change(
        fastrtps::rtps::CacheChange_t* change)
{
    // Lock(exclusive mode) mutex locally
    std::unique_lock<share_mutex_t> lock(sh_mtx_);

    if (change->kind == fastrtps::rtps::ChangeKind_t::ALIVE)
    {
        logWarning(DISCOVERY_DATABASE,
                "Attempting to delete information of an ALIVE entity: " << guid_from_change(change));
        return false;
    }

    if (is_participant(change))
    {
        // The information related to this participant is cleaned up in process_data_queue()
        auto it = participants_.find(guid_from_change(change).guidPrefix);
        if (it == participants_.end())
        {
            return false;
        }
        changes_to_release_.push_back(it->second.change());
        participants_.erase(it);
        return true;
    }
    else if (is_reader(change))
    {
        // The information related to this reader is cleaned up in process_data_queue()
        auto it = readers_.find(guid_from_change(change));
        if (it == readers_.end())
        {
            return false;
        }
        changes_to_release_.push_back(it->second.change());
        readers_.erase(it);
        return true;
    }
    else if (is_writer(change))
    {
        // The information related to this writer is cleaned up in process_data_queue()
        auto it = writers_.find(guid_from_change(change));
        if (it == writers_.end())
        {
            return false;
        }
        changes_to_release_.push_back(it->second.change());
        writers_.erase(it);
        return true;
    }
    return false;
}

bool DiscoveryDataBase::data_queue_empty()
{
    return (pdp_data_queue_.BothEmpty() && edp_data_queue_.BothEmpty());
}

bool DiscoveryDataBase::is_participant(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    return eprosima::fastrtps::rtps::c_EntityId_RTPSParticipant == guid_from_change(ch).entityId;
}

bool DiscoveryDataBase::is_writer(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    constexpr uint8_t entity_id_is_writer_bit = 0x03;
    return ((guid_from_change(ch).entityId.value[3] & ~entity_id_is_writer_bit) == 0);
}

bool DiscoveryDataBase::is_reader(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    constexpr uint8_t entity_id_is_reader_bit = 0x04;
    return ((guid_from_change(ch).entityId.value[3] & ~entity_id_is_reader_bit) == 0);
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

const std::vector<fastrtps::rtps::GuidPrefix_t> DiscoveryDataBase::direct_clients_and_servers()
{
    std::vector<fastrtps::rtps::GuidPrefix_t> direct_clients_and_servers;
    // Iterate over participants to add the remote ones that are direct clients or servers
    for (auto participant: participants_)
    {
        // Only add participants other than the server
        if (server_guid_prefix_ != participant.first)
        {
            // Only add direct clients or server, not relayed ones.
            if (participant.second.is_my_client() || participant.second.is_my_server())
            {
                direct_clients_and_servers.push_back(participant.first);
            }
        }
    }
    return direct_clients_and_servers;
}

fastrtps::rtps::LocatorList_t DiscoveryDataBase::participant_metatraffic_locators(
        fastrtps::rtps::GuidPrefix_t participant_guid_prefix)
{
    fastrtps::rtps::LocatorList_t locators;
    auto part_it = participants_.find(participant_guid_prefix);
    if (part_it != participants_.end())
    {
        for (auto locator : part_it->second.metatraffic_locators().unicast)
        {
            locators.push_back(locator);
        }
    }
    return locators;
}

DiscoveryDataBase::AckedFunctor DiscoveryDataBase::functor(
        eprosima::fastrtps::rtps::CacheChange_t* change)
{
    return DiscoveryDataBase::AckedFunctor(this, change);
}

DiscoveryDataBase::AckedFunctor::AckedFunctor(
        DiscoveryDataBase* db,
        eprosima::fastrtps::rtps::CacheChange_t* change)
    : db_(db)
    , change_(change)
    , pending_(false)
    // references its own state
    , external_pending_(pending_)
{
    // RAII only for the stateful object
    db_->exclusive_lock_();
}

DiscoveryDataBase::AckedFunctor::AckedFunctor(
        const DiscoveryDataBase::AckedFunctor& r)
// references original state
    : external_pending_(r.external_pending_)
{
    db_ = r.db_;
    change_ = r.change_;
}

DiscoveryDataBase::AckedFunctor::~AckedFunctor()
{
    if (&external_pending_ == &pending_)
    {
        // only the stateful object manages the lock
        db_->exclusive_unlock_();
    }
}

void DiscoveryDataBase::AckedFunctor::operator () (
        eprosima::fastrtps::rtps::ReaderProxy* reader_proxy)
{
    // Check whether the change has been acknowledged by a given reader
    if (reader_proxy->rtps_is_relevant(change_))
    {
        if (reader_proxy->change_is_acked(change_->sequenceNumber))
        {
            // In the discovery database, mark the change as acknowledged by the reader
            db_->add_ack_(change_, reader_proxy->guid().guidPrefix);
        }
        else
        {
            // This change is relevant and has not been acked, so there are pending acknowledgements
            external_pending_ = true;
        }
    }
}

void erase_participant_(eprosima::fastrtps::rtps::GuidPrefix_t& guid);

// unmatch in every other entity including its readers and writers
void unmatch_participant_(eprosima::fastrtps::rtps::GuidPrefix_t& guid);

// unmatch all the readers
void unmatch_writer_(eprosima::fastrtps::rtps::GUID_t& guid);

// erase a writer from a participant and clean the entity populting changes_to_release_
void erase_writer_(eprosima::fastrtps::rtps::GUID_t& guid);

// unmatch all the writers
void unmatch_reader_(eprosima::fastrtps::rtps::GUID_t& guid);

// erase a reader from a participant and clean the entity populting changes_to_release_
void erase_reader_(eprosima::fastrtps::rtps::GUID_t& guid);

// check if the participants must be matched and add/erase in the acks data if they
// should know each other. This works finding the related topics and entities below
// this function should be called when a reader or writer is disposed
bool update_matching_(
    eprosima::fastrtps::rtps::GuidPrefix_t& participant1,
    eprosima::fastrtps::rtps::GuidPrefix_t& participant2);

} // namespace ddb
} // namespace rtps
} // namespace fastdds
} // namespace eprosima
