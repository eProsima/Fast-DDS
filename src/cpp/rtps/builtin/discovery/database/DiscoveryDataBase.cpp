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
#include <fastdds/rtps/common/EntityId_t.hpp>

#include "./DiscoveryDataBase.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

DiscoveryDataBase::DiscoveryDataBase(
        fastrtps::rtps::GuidPrefix_t server_guid_prefix,
        std::vector<fastrtps::rtps::GuidPrefix_t> servers)
    : server_guid_prefix_(server_guid_prefix)
    , server_acked_by_all_((servers.size() > 0) ? false : true)
    , servers_(servers)
    , enabled_(true)
{
}

DiscoveryDataBase::~DiscoveryDataBase()
{
    if (!clear().empty())
    {
        logError(DISCOVERY_DATABASE, "Destroying a NOT cleared database");
    }
}

std::vector<fastrtps::rtps::CacheChange_t*> DiscoveryDataBase::clear()
{
    // Cannot clear an enabled database, since there could be inconsistencies after the process
    if (enabled_)
    {
        logError(DISCOVERY_DATABASE, "Cannot clear an enabled database");
        return std::vector<fastrtps::rtps::CacheChange_t*>({});
    }
    logInfo(DISCOVERY_DATABASE, "Clearing DiscoveryDataBase");

    std::unique_lock<std::recursive_mutex> lock(mutex_);

    /* Clear receive queues */
    pdp_data_queue_.Clear();
    edp_data_queue_.Clear();

    /* Clear by_topic collections */
    writers_by_topic_.clear();
    readers_by_topic_.clear();

    /* Clear list of dirty topics */
    dirty_topics_.clear();

    /* Clear disposals list */
    disposals_.clear();

    /* Clear to_send collections */
    pdp_to_send_.clear();
    edp_publications_to_send_.clear();
    edp_subscriptions_to_send_.clear();

    /* Clear writers_ */
    for (auto writers_it = writers_.begin(); writers_it != writers_.end();)
    {
        writers_it = delete_writer_entity_(writers_it);
    }

    /* Clear readers_ */
    for (auto readers_it = readers_.begin(); readers_it != readers_.end();)
    {
        readers_it = delete_reader_entity_(readers_it);
    }

    /* Clear participants_ */
    for (auto participants_it = participants_.begin(); participants_it != participants_.end();)
    {
        participants_it = delete_participant_entity_(participants_it);
    }

    /* Reset state parameters */
    server_acked_by_all_ = false;

    /* Clear changes to release */
    std::vector<fastrtps::rtps::CacheChange_t*> leftover_changes = changes_to_release_;
    changes_to_release_.clear();

    /* Return the collection of changes that are no longer owned by the database */
    return leftover_changes;
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
    std::unique_lock<std::recursive_mutex> lock(mutex_);

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
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    auto itp = participants_.find(change_guid.guidPrefix);
    if (itp == participants_.end())
    {
        // not relevant
        return false;
    }
    else if (!itp->second.is_matched(reader_guid.guidPrefix))
    {
        // not relevant
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
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    auto itp = participants_.find(change_guid.guidPrefix);
    if (itp == participants_.end())
    {
        // not relevant
        return false;
    }
    else if (!itp->second.is_matched(reader_guid.guidPrefix))
    {
        // not relevant
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

void DiscoveryDataBase::update_change_and_unmatch_(
        fastrtps::rtps::CacheChange_t* new_change,
        ddb::DiscoverySharedInfo& entity)
{
    changes_to_release_.push_back(entity.update_and_unmatch(new_change));
    // Manually set relevant participants ACK status of this server, and of the participant that sent the
    // change, to 1. This way, we avoid backprogation of the data.
    entity.add_or_update_ack_participant(server_guid_prefix_, true);
    entity.add_or_update_ack_participant(new_change->writerGUID.guidPrefix, true);
}

void DiscoveryDataBase::add_ack_(
        const eprosima::fastrtps::rtps::CacheChange_t* change,
        const eprosima::fastrtps::rtps::GuidPrefix_t& acked_entity)
{
    if (!enabled_)
    {
        logWarning(DISCOVERY_DATABASE, "Discovery Database is disabled");
        return;
    }

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
    if (!enabled_)
    {
        logWarning(DISCOVERY_DATABASE, "Discovery Database is disabled");
        return false;
    }

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
    if (!enabled_)
    {
        logWarning(DISCOVERY_DATABASE, "Discovery Database is disabled");
        return false;
    }

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
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    return disposals_;
}

void DiscoveryDataBase::clear_changes_to_dispose()
{
    // lock(exclusive mode) mutex locally
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    disposals_.clear();
}

////////////
// Functions to process_to_send_lists()
const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> DiscoveryDataBase::pdp_to_send()
{
    // lock(sharing mode) mutex locally
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    return pdp_to_send_;
}

void DiscoveryDataBase::clear_pdp_to_send()
{
    // lock(exclusive mode) mutex locally
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    pdp_to_send_.clear();
}

const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> DiscoveryDataBase::edp_publications_to_send()
{
    // lock(sharing mode) mutex locally
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    return edp_publications_to_send_;
}

void DiscoveryDataBase::clear_edp_publications_to_send()
{
    // lock(exclusive mode) mutex locally
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    edp_publications_to_send_.clear();
}

const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> DiscoveryDataBase::edp_subscriptions_to_send()
{
    // lock(sharing mode) mutex locally
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    return edp_subscriptions_to_send_;
}

void DiscoveryDataBase::clear_edp_subscriptions_to_send()
{
    // lock(exclusive mode) mutex locally
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    edp_subscriptions_to_send_.clear();
}

const std::vector<eprosima::fastrtps::rtps::CacheChange_t*> DiscoveryDataBase::changes_to_release()
{
    // lock(sharing mode) mutex locally
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    return changes_to_release_;
}

void DiscoveryDataBase::clear_changes_to_release()
{
    // lock(exclusive mode) mutex locally
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    changes_to_release_.clear();
}

////////////
// Functions to process PDP and EDP data queues
void DiscoveryDataBase::process_pdp_data_queue()
{
    if (!enabled_)
    {
        logWarning(DISCOVERY_DATABASE, "Discovery Database is disabled");
        return;
    }

    // Lock(exclusive mode) mutex locally
    std::unique_lock<std::recursive_mutex> lock(mutex_);

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
            create_participant_from_change_(data_queue_info.change(), data_queue_info.participant_change_data());
        }
        // If the change is a DATA(Up)
        else
        {
            logInfo(DISCOVERY_DATABASE, "DATA(Up) received from: " << data_queue_info.change()->instanceHandle);
            process_dispose_participant_(data_queue_info.change());
        }

        // Pop the message from the queue
        pdp_data_queue_.Pop();
    }
}

bool DiscoveryDataBase::process_edp_data_queue()
{
    if (!enabled_)
    {
        logWarning(DISCOVERY_DATABASE, "Discovery Database is disabled");
        return false;
    }

    bool is_dirty_topic = false;

    // Lock(exclusive mode) mutex locally
    std::unique_lock<std::recursive_mutex> lock(mutex_);

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
                create_writers_from_change_(change, topic_name);
            }
            // DATA(r) case
            else if (is_reader(change))
            {
                logInfo(DISCOVERY_DATABASE, "DATA(r) in topic " << topic_name << " received from: "
                                                                << change->instanceHandle);
                create_readers_from_change_(change, topic_name);
            }

            // Update set of dirty_topics
            set_dirty_topic_(topic_name);
        }
        // If the change is a DATA(Uw|Ur)
        else
        {
            // DATA(Uw) case
            if (is_writer(change))
            {
                logInfo(DISCOVERY_DATABASE, "DATA(Uw) received from: " << change->instanceHandle);
                process_dispose_writer_(change);
            }
            // DATA(Ur) case
            else if (is_reader(change))
            {
                logInfo(DISCOVERY_DATABASE, "DATA(Ur) received from: " << change->instanceHandle);
                process_dispose_reader_(change);
            }
        }

        // Pop the message from the queue
        edp_data_queue_.Pop();
    }

    return is_dirty_topic;
}

void DiscoveryDataBase::create_participant_from_change_(
        eprosima::fastrtps::rtps::CacheChange_t* ch,
        const DiscoveryParticipantChangeData& change_data)
{
    fastrtps::rtps::GUID_t change_guid = guid_from_change(ch);
    auto participant_it = participants_.find(change_guid.guidPrefix);

    // The participant was already known in the database
    if (participant_it != participants_.end())
    {
        // Only update database if the change is newer than the one we already have
        if (ch->write_params.sample_identity().sequence_number() >
            participant_it->second.change()->write_params.sample_identity().sequence_number())
        {
            // Update the change related to the participant and return the old change to the pool
            logInfo(DISCOVERY_DATABASE, "Participant updating. Marking old change to release");
            // Update participant's change in the database, set all relevant participants ACK status to 0, and add
            // old change to changes_to_release_.
            update_change_and_unmatch_(ch, participant_it->second);
        }
    }
    // New participant
    else
    {
        DiscoveryParticipantInfo part(ch, server_guid_prefix_, change_data);
        std::pair<std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator, bool> ret =
                participants_.insert(std::make_pair(change_guid.guidPrefix, part));
        // If insert was successful
        if (ret.second)
        {
            logInfo(DISCOVERY_DATABASE, "New participant added: " << change_guid.guidPrefix);

            // Manually set to 1 the relevant participants ACK status of the participant that sent the change. This way,
            // we avoid backprogation of the data.
            ret.first->second.add_or_update_ack_participant(ch->writerGUID.guidPrefix, true);

            // If the DATA(p) it's from this server, put the DATA(p) in the history
            if (change_guid.guidPrefix == server_guid_prefix_)
            {
                // Only add our own DATA(p) to pdp_to_send_ if it's not there already
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
            // The participant is not this server
            else
            {
                // If the participant is a new participant, mark that not everyone has ACKed this server's DATA(p)
                server_acked_by_all(false);
            }

            if (ret.first->second.is_local_server())
            {
                /* Create virtual writer */
                // Create a GUID for the virtual writer from the local server GUID prefix and the virtual writer entity
                // ID.
                fastrtps::rtps::GUID_t virtual_writer_guid(change_guid.guidPrefix,
                    fastrtps::rtps::ds_server_virtual_writer);
                // Create a populate the Cache Change with the necessary information.
                fastrtps::rtps::CacheChange_t* virtual_writer_change = new fastrtps::rtps::CacheChange_t();
                virtual_writer_change->kind = fastrtps::rtps::ChangeKind_t::ALIVE;
                virtual_writer_change->writerGUID = virtual_writer_change->writerGUID;
                virtual_writer_change->instanceHandle = fastrtps::rtps::InstanceHandle_t(virtual_writer_guid);
                // Populate sample identity
                fastrtps::rtps::SampleIdentity virtual_writer_sample_id;
                virtual_writer_sample_id.writer_guid(virtual_writer_guid);
                virtual_writer_sample_id.sequence_number(eprosima::fastrtps::rtps::SequenceNumber_t(0));
                // Set write params
                eprosima::fastrtps::rtps::WriteParams virtual_writer_writer_params;
                virtual_writer_writer_params.sample_identity(virtual_writer_sample_id);
                virtual_writer_writer_params.related_sample_identity(virtual_writer_sample_id);
                virtual_writer_change->write_params = std::move(virtual_writer_writer_params);
                // Create the virtual writer
                create_writers_from_change_(virtual_writer_change, virtual_topic_);
                // TODO: Delete this Cache Change upon participant destruction. Make sure ~DiscoveryDataBase() deletes
                // the ones remaining.

                /* Create virtual reader */
                // Create a GUID for the virtual reader from the local server GUID prefix and the virtual reader entity
                // ID.
                fastrtps::rtps::GUID_t virtual_reader_guid(change_guid.guidPrefix,
                    fastrtps::rtps::ds_server_virtual_reader);
                // Create a populate the Cache Change with the necessary information.
                fastrtps::rtps::CacheChange_t* virtual_reader_change = new fastrtps::rtps::CacheChange_t();
                virtual_reader_change->kind = fastrtps::rtps::ChangeKind_t::ALIVE;
                virtual_reader_change->writerGUID = virtual_reader_change->writerGUID;
                virtual_reader_change->instanceHandle = fastrtps::rtps::InstanceHandle_t(virtual_reader_guid);
                // Populate sample identity
                fastrtps::rtps::SampleIdentity virtual_reader_sample_id;
                virtual_reader_sample_id.writer_guid(virtual_reader_guid);
                virtual_reader_sample_id.sequence_number(eprosima::fastrtps::rtps::SequenceNumber_t(0));
                // Set write params
                eprosima::fastrtps::rtps::WriteParams virtual_reader_writer_params;
                virtual_reader_writer_params.sample_identity(virtual_reader_sample_id);
                virtual_reader_writer_params.related_sample_identity(virtual_reader_sample_id);
                virtual_writer_change->write_params = std::move(virtual_reader_writer_params);
                // Create the virtual reader
                create_readers_from_change_(virtual_reader_change, virtual_topic_);
                // TODO: Delete this Cache Change upon participant destruction. Make sure ~DiscoveryDataBase() deletes
                // the ones remaining.
            }
        }
        else
        {
            logError(DISCOVERY_DATABASE, "Failed adding new participant " << change_guid.guidPrefix);
        }
    }
}

void DiscoveryDataBase::create_writers_from_change_(
        eprosima::fastrtps::rtps::CacheChange_t* ch,
        const std::string& topic_name)
{
    const eprosima::fastrtps::rtps::GUID_t& writer_guid = guid_from_change(ch);
    auto writer_it = writers_.find(writer_guid);

    // Update writers_
    DiscoveryEndpointInfo tmp_writer(ch, topic_name, server_guid_prefix_);
    tmp_writer.add_or_update_ack_participant(ch->writerGUID.guidPrefix, true);

    std::pair<std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator, bool> ret =
            writers_.insert(std::make_pair(writer_guid, tmp_writer));

    // If writer does not exists, create the change
    if (ret.second)
    {
        // Add entry to writers_
        DiscoveryEndpointInfo tmp_writer(ch, topic_name, server_guid_prefix_);
        std::pair<std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator, bool> ret =
                writers_.insert(std::make_pair(writer_guid, tmp_writer));
        if (!ret.second)
        {
            logError(DISCOVERY_DATABASE, "Error inserting writer " << writer_guid);
            return;
        }
        writer_it = ret.first;

        // Add entry to participants_[guid_prefix]::writers
        std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator writer_part_it =
                participants_.find(writer_guid.guidPrefix);
        if (writer_part_it != participants_.end())
        {
            writer_part_it->second.add_writer(writer_guid);
        }
        else
        {
            logError(DISCOVERY_DATABASE, "Writer " << writer_guid << " as no associated participant. Skipping");
            return;
        }

        // Manually set to 1 the relevant participants ACK status of the participant that sent the change. This way,
        // we avoid backprogation of the data.
        writer_it->second.add_or_update_ack_participant(ch->writerGUID.guidPrefix, true);

        // Get all readers in the topic
        std::map<std::string, std::vector<eprosima::fastrtps::rtps::GUID_t>>::iterator readers_it =
                readers_by_topic_.find(topic_name);

        if (readers_it != readers_by_topic_.end())
        {
            // Iterate over readers in the topic
            for (eprosima::fastrtps::rtps::GUID_t reader_guid: readers_it->second)
            {
                // Update the participant ack status list from writers_ (if needed). The reader will only know the
                // new writer if the reader is a server endpoint, or if the reader and writer are from the same
                // participant.
                if (!ret.first->second.is_matched(reader_it.guidPrefix))
                {
                    ret.first->second.add_or_update_ack_participant(reader_it.guidPrefix);
                }

                // Update the participant ack status list from readers_
                std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator rit =
                        readers_.find(reader_it);
                if (rit != readers_.end() && !rit->second.is_matched(writer_guid.guidPrefix))
                {
                    logError(DISCOVERY_DATABASE, "Reader " << reader_guid << " as no associated participant. Skipping");
                    continue;
                }

                /* The writer is from a local CLIENT */
                if (writer_part_it->second.is_local_client())
                {
                    /* The reader is from a local CLIENT or SERVER */
                    if (!reader_part_it->second.is_external())
                    {
                        // Add reader's participant to writer's participant ACK list with status 0 (unless already
                        // matched)
                        if (!writer_part_it->second.is_matched(reader_guid.guidPrefix))
                        {
                            writer_part_it->second.add_or_update_ack_participant(reader_guid.guidPrefix);
                        }

                        // Add reader's participant to writer's ACK list with status 0.
                        writer_it->second.add_or_update_ack_participant(reader_guid.guidPrefix);
                    }

                    /* The reader is a real endpoint (not virtual) */
                    if (!reader_it->second.is_virtual())
                    {
                        // Add writer's participant to reader's participant ACK list with status 0 (unless already
                        // matched)
                        if (!reader_part_it->second.is_matched(writer_guid.guidPrefix))
                        {
                            reader_part_it->second.add_or_update_ack_participant(writer_guid.guidPrefix);
                        }
                        // Add writer's participant to reader's ACK list with status 0
                        reader_it->second.add_or_update_ack_participant(writer_guid.guidPrefix);
                    }
                }
                /* The writer is virtual (therefore associated with a local SERVER) */
                else if (topic_name == virtual_topic_)
                {
                    writer_it->second.is_virtual(true);
                    /* The reader is a real endpoint (not virtual) */
                    if (!reader_it->second.is_virtual())
                    {
                        // Add writer's participant to reader's participant ACK list with status 0 (if will never be
                        // there, since this case only happens upon local SERVER DATA(p) reception)
                        reader_part_it->second.add_or_update_ack_participant(writer_guid.guidPrefix);
                        // Add writer's participant to reader's ACK list with status 0
                        reader_it->second.add_or_update_ack_participant(writer_guid.guidPrefix);
                        // Set reader's topic to dirty.
                        set_dirty_topic_(reader_it->second.topic());
                    }
                }
                /* The writer is from a external CLIENT or SERVER */
                else if (writer_part_it->second.is_external())
                {
                    // Add reader's participant to writer's participant ACK list with status 0 (unless already
                    // matched)
                    if (!writer_part_it->second.is_matched(reader_guid.guidPrefix))
                    {
                        writer_part_it->second.add_or_update_ack_participant(reader_guid.guidPrefix);
                    }

                    // Add reader's participant to writer's ACK list with status 0.
                    writer_it->second.add_or_update_ack_participant(reader_guid.guidPrefix);
                }
            }
        }

        // Add writer to writers_by_topic_[topic_name]
        add_writer_to_topic_(writer_guid, topic_name);
    }
}

void DiscoveryDataBase::create_readers_from_change_(
        eprosima::fastrtps::rtps::CacheChange_t* ch,
        const std::string& topic_name)
{
    const eprosima::fastrtps::rtps::GUID_t& reader_guid = guid_from_change(ch);
    auto reader_it = readers_.find(reader_guid);

    DiscoveryEndpointInfo tmp_reader(ch, topic_name, server_guid_prefix_);
    tmp_reader.add_or_update_ack_participant(ch->writerGUID.guidPrefix, true);

    std::pair<std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator, bool> ret =
            readers_.insert(std::make_pair(reader_guid, tmp_reader));

    // If reader does not exists, create the change
    if (ret.second)
    {
        // Add entry to readers_
        DiscoveryEndpointInfo tmp_reader(ch, topic_name, server_guid_prefix_);
        std::pair<std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator, bool> ret =
                readers_.insert(std::make_pair(reader_guid, tmp_reader));
        if (!ret.second)
        {
            logError(DISCOVERY_DATABASE, "Error inserting reader " << reader_guid);
            return;
        }
        reader_it = ret.first;

        // Add entry to participants_[guid_prefix]::readers
        std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator reader_part_it =
                participants_.find(reader_guid.guidPrefix);
        if (reader_part_it != participants_.end())
        {
            reader_part_it->second.add_reader(reader_guid);
        }
        else
        {
            logError(DISCOVERY_DATABASE, "Reader " << reader_guid << " as no associated participant. Skipping");
            return;
        }

        // Manually set to 1 the relevant participants ACK status of the participant that sent the change. This way,
        // we avoid backprogation of the data.
        reader_it->second.add_or_update_ack_participant(ch->writerGUID.guidPrefix, true);

        // Get all writers in the topic
        std::map<std::string, std::vector<eprosima::fastrtps::rtps::GUID_t>>::iterator writers_it =
                writers_by_topic_.find(topic_name);

        if (writers_it != writers_by_topic_.end())
        {
            // Iterate over readers in the topic
            for (eprosima::fastrtps::rtps::GUID_t writer_guid: writers_it->second)
            {
                logInfo(DISCOVERY_DATABASE, "Matching Data(r): " << reader_guid << " with writer: "
                                                                 << writer_it);

                // Update the participant ack status list from readers_ (if needed). The writer will only know the
                // new reader if the writier is a server endpoint, or if the reader and writer are from the same
                // participant.
                if (!ret.first->second.is_matched(writer_it.guidPrefix))
                {
                    ret.first->second.add_or_update_ack_participant(writer_it.guidPrefix);
                }

                // Update the participant ack status list from writers_
                std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator wit =
                        writers_.find(writer_it);
                if (wit != writers_.end() && !wit->second.is_matched(reader_guid.guidPrefix))
                {
                    logError(DISCOVERY_DATABASE, "Writer " << writer_guid << " as no associated participant. Skipping");
                    continue;
                }

                /* The reader is from a local CLIENT */
                if (reader_part_it->second.is_local_client())
                {
                    /* The writer is from a local CLIENT or SERVER */
                    if (!writer_part_it->second.is_external())
                    {
                        // Add writer's participant to writer's participant ACK list with status 0 (unless already
                        // matched)
                        if (!reader_part_it->second.is_matched(writer_guid.guidPrefix))
                        {
                            reader_part_it->second.add_or_update_ack_participant(writer_guid.guidPrefix);
                        }

                        // Add writer's participant to writer's ACK list with status 0.
                        reader_it->second.add_or_update_ack_participant(writer_guid.guidPrefix);
                    }

                    /* The writer is a real endpoint (not virtual) */
                    if (!writer_it->second.is_virtual())
                    {
                        // Add reader's participant to writer's participant ACK list with status 0 (unless already
                        // matched)
                        if (!writer_part_it->second.is_matched(reader_guid.guidPrefix))
                        {
                            writer_part_it->second.add_or_update_ack_participant(reader_guid.guidPrefix);
                        }
                        // Add reader's participant to writer's ACK list with status 0
                        writer_it->second.add_or_update_ack_participant(reader_guid.guidPrefix);
                    }
                }
                /* The reader is virtual (therefore associated with a local SERVER) */
                else if (topic_name == virtual_topic_)
                {
                    reader_it->second.is_virtual(true);
                    /* The writer is a real endpoint (not virtual) */
                    if (!writer_it->second.is_virtual())
                    {
                        // Add reader's participant to writers's participant ACK list with status 0 (if will never be
                        // there, since this case only happens upon local SERVER DATA(p) reception)
                        writer_part_it->second.add_or_update_ack_participant(reader_guid.guidPrefix);
                        // Add reader's participant to writers's ACK list with status 0
                        writer_it->second.add_or_update_ack_participant(reader_guid.guidPrefix);
                        // Set writer's topic to dirty.
                        set_dirty_topic_(writer_it->second.topic());
                    }
                }
                /* The reader is from a external CLIENT or SERVER */
                else if (reader_part_it->second.is_external())
                {
                    // Add writer's participant to reader's participant ACK list with status 0 (unless already
                    // matched)
                    if (!reader_part_it->second.is_matched(writer_guid.guidPrefix))
                    {
                        reader_part_it->second.add_or_update_ack_participant(writer_guid.guidPrefix);
                    }

                    // Add writer's participant to reader's ACK list with status 0.
                    reader_it->second.add_or_update_ack_participant(writer_guid.guidPrefix);
                }
            }
        }

        // Add reader to readers_by_topic_[topic_name]
        add_reader_to_topic_(reader_guid, topic_name);
    }
}

bool DiscoveryDataBase::set_dirty_topic_(std::string topic)
{
    if (std::find(
                dirty_topics_.begin(),
                dirty_topics_.end(),
                topic) == dirty_topics_.end())
    {
        logInfo(DISCOVERY_DATABASE, "Setting topic " << topic << " as dirty");
        dirty_topics_.push_back(topic);
        return true;
    }
    return false;
}


void DiscoveryDataBase::process_dispose_participant_(
        eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    const eprosima::fastrtps::rtps::GUID_t& participant_guid = guid_from_change(ch);

    // Change DATA(p) with DATA(Up) in participants map
    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
            participants_.find(participant_guid.guidPrefix);
    if (pit != participants_.end())
    {
        // Only update DATA(p), leaving the change info untouched. This is because DATA(Up) does not have the
        // participant's meta-information, but we don't want to loose it here.
        update_change_and_unmatch_(ch, pit->second);
    }
    else
    {
        logError(DISCOVERY_DATABASE, "Processing disposal from an unexisting Participant"
                << participant_guid.guidPrefix);
    }

    // Delete entries from writers_ belonging to the participant
    while (!pit->second.writers().empty())
    {
        auto writer_guid = pit->second.writers().back();

        // erase writer from topic
        unmatch_writer_(writer_guid);

        // release the change and remove entity without setting Data(Uw)
        delete_writer_entity_(writer_guid);
    }

    // Delete entries from readers_ belonging to the participant
    while (!pit->second.readers().empty())
    {
        auto reader_guid = pit->second.readers().back();

        // this unmatch must erase the entity from writers
        unmatch_reader_(reader_guid);

        // release the change and remove entity without setting Data(Ur)
        delete_reader_entity_(reader_guid);
    }

    // all participant endoints must be already unmatched in others endopoints relevant_ack maps

    // unmatch own participant
    unmatch_participant_(participant_guid.guidPrefix);

    // Add entry to disposals_
    if (std::find(disposals_.begin(), disposals_.end(), ch) == disposals_.end())
    {
        disposals_.push_back(ch);
    }
}

void DiscoveryDataBase::process_dispose_writer_(
        eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    const eprosima::fastrtps::rtps::GUID_t& writer_guid = guid_from_change(ch);

    // check if the writer is still alive (if DATA(Up) is processed before it will be erased)
    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator wit = writers_.find(writer_guid);
    if (wit != writers_.end())
    {
        // Change DATA(w) with DATA(Uw)
        update_change_and_unmatch_(ch, wit->second);

        // remove writer from topic
        remove_writer_from_topic_(writer_guid, wit->second.topic());

        // Add entry to disposals_
        if (std::find(disposals_.begin(), disposals_.end(), ch) == disposals_.end())
        {
            disposals_.push_back(ch);
        }
    }
}

void DiscoveryDataBase::process_dispose_reader_(
        eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    const eprosima::fastrtps::rtps::GUID_t& reader_guid = guid_from_change(ch);

    // check if the writer is still alive (if DATA(Up) is processed before it will be erased)

    std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator rit = readers_.find(reader_guid);
    if (rit != readers_.end())
    {
        // Change DATA(r) with DATA(Ur)
        update_change_and_unmatch_(ch, rit->second);

        // remove reader from topic
        remove_reader_from_topic_(reader_guid, rit->second.topic());

        // Add entry to disposals_
        if (std::find(disposals_.begin(), disposals_.end(), ch) == disposals_.end())
        {
            disposals_.push_back(ch);
        }
    }
}

bool DiscoveryDataBase::process_dirty_topics()
{
    if (!enabled_)
    {
        logWarning(DISCOVERY_DATABASE, "Discovery Database is disabled");
        return false;
    }

    // logInfo(DISCOVERY_DATABASE, "process_dirty_topics start");
    // Get shared lock
    std::unique_lock<std::recursive_mutex> lock(mutex_);

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
                    // Add DATA(p) of the client with the writer to `pdp_to_send_` (if it's not there).
                    if (std::find(
                                pdp_to_send_.begin(),
                                pdp_to_send_.end(),
                                parts_reader_it->second.change()) == pdp_to_send_.end())
                    {
                        logInfo(DISCOVERY_DATABASE, "Addind readers' DATA(p) to send: "
                                << parts_reader_it->second.change()->instanceHandle);
                        pdp_to_send_.push_back(parts_reader_it->second.change());
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
                    // Add DATA(p) of the client with the reader to `pdp_to_send_` (if it's not there).
                    if (std::find(
                                pdp_to_send_.begin(),
                                pdp_to_send_.end(),
                                parts_writer_it->second.change()) == pdp_to_send_.end())
                    {
                        logInfo(DISCOVERY_DATABASE, "Addind writers' DATA(p) to send: "
                                << parts_writer_it->second.change()->instanceHandle);
                        pdp_to_send_.push_back(parts_writer_it->second.change());
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
    if (!enabled_)
    {
        logWarning(DISCOVERY_DATABASE, "Discovery Database is disabled");
        return false;
    }

    // Lock(exclusive mode) mutex locally
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    if (change->kind == fastrtps::rtps::ChangeKind_t::ALIVE)
    {
        logWarning(DISCOVERY_DATABASE,
                "Attempting to delete information of an ALIVE entity: " << guid_from_change(change));
        return false;
    }

    if (is_participant(change))
    {
        // The information related to this participant is cleaned up in process_data_queue()
        // when a disposal arrives, and it cleans also its children entities
        return delete_participant_entity_(guid_from_change(change).guidPrefix);
    }
    else if (is_reader(change))
    {
        // The information related to this reader is cleaned up in process_data_queue()
        return delete_reader_entity_(guid_from_change(change));
    }
    else if (is_writer(change))
    {
        // The information related to this writer is cleaned up in process_data_queue()
        return delete_writer_entity_(guid_from_change(change));
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
    // RTPS Specification v2.3
    // For writers: NO_KEY = 0x03, WITH_KEY = 0x02
    // For built-in writers: NO_KEY = 0xc3, WITH_KEY = 0xc2
    const eprosima::fastrtps::rtps::octet identifier = guid_from_change(ch).entityId.value[3];
    return ((identifier == 0x02) ||
           (identifier == 0xc2) ||
           (identifier == 0x03) ||
           (identifier == 0xc3));
}

bool DiscoveryDataBase::is_reader(
        const eprosima::fastrtps::rtps::CacheChange_t* ch)
{
    // RTPS Specification v2.3
    // For readers: NO_KEY = 0x04, WITH_KEY = 0x07
    // For built-in readers: NO_KEY = 0xc4, WITH_KEY = 0xc7
    const eprosima::fastrtps::rtps::octet identifier = guid_from_change(ch).entityId.value[3];
    return ((identifier == 0x04) ||
           (identifier == 0xc4) ||
           (identifier == 0x07) ||
           (identifier == 0xc7));
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

bool DiscoveryDataBase::server_acked_by_my_servers()
{
    if (servers_.size() == 0)
    {
        return true;
    }

    // Find the server's participant and check whether all its servers have ACKed the server's DATA(p)
    auto this_server = participants_.find(server_guid_prefix_);
    for (auto prefix : servers_)
    {
        if (!this_server->second.is_matched(prefix))
        {
            return false;
        }
    }
    return true;
}

std::vector<fastrtps::rtps::GuidPrefix_t> DiscoveryDataBase::ack_pending_servers()
{
    std::vector<fastrtps::rtps::GuidPrefix_t> ack_pending_servers;
    // Find the server's participant and check whether all its servers have ACKed the server's DATA(p)
    auto this_server = participants_.find(server_guid_prefix_);
    for (auto prefix : servers_)
    {
        if (!this_server->second.is_matched(prefix))
        {
            ack_pending_servers.push_back(prefix);
        }
    }
    return ack_pending_servers;
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
    db_->lock_();
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
        db_->unlock_();
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

void DiscoveryDataBase::unmatch_participant_(
        const eprosima::fastrtps::rtps::GuidPrefix_t& guid_prefix)
{
    logInfo(DISCOVERY_DATABASE, "unmatching participant: " << guid_prefix);

    auto pit = participants_.find(guid_prefix);
    if (pit == participants_.end())
    {
        logWarning(DISCOVERY_DATABASE,
                "Attempting to unmatch an unexisting participant: " << guid_prefix);
    }

    // for each relevant participant make not relevant
    for (eprosima::fastrtps::rtps::GuidPrefix_t relevant_participant : pit->second.relevant_participants())
    {
        auto rpit = participants_.find(relevant_participant);
        if (rpit == participants_.end())
        {
            logWarning(DISCOVERY_DATABASE,
                    "Matched with an unexisting participant: " << guid_prefix);
        }

        rpit->second.remove_participant(guid_prefix);
    }
}

void DiscoveryDataBase::unmatch_writer_(
        const eprosima::fastrtps::rtps::GUID_t& guid)
{
    logInfo(DISCOVERY_DATABASE, "unmatching writer: " << guid);

    auto wit = writers_.find(guid);
    if (wit == writers_.end())
    {
        logWarning(DISCOVERY_DATABASE,
                "Attempting to unmatch an unexisting writer: " << guid);
        return;
    }

    // get writer topic
    std::string topic = wit->second.topic();

    // remove it from writer by topic
    remove_writer_from_topic_(guid, topic);

    // it there are more than one writer in this topic in the same participant we do not unmatch the endpoints
    if (!repeated_writer_topic_(guid.guidPrefix, topic))
    {
        // for each reader in same topic make not relevant. It could be none in readers
        auto tit = readers_by_topic_.find(topic);
        if (tit != readers_by_topic_.end())
        {
            for (auto reader : tit->second)
            {
                auto rit = readers_.find(reader);
                if (rit == readers_.end())
                {
                    logWarning(DISCOVERY_DATABASE,
                            "Unexisting reader " << reader << " in topic: " << topic);
                }

                rit->second.remove_participant(guid.guidPrefix);
            }
        }
    }
}

void DiscoveryDataBase::unmatch_reader_(
        const eprosima::fastrtps::rtps::GUID_t& guid)
{
    logInfo(DISCOVERY_DATABASE, "unmatching reader: " << guid);

    auto rit = readers_.find(guid);
    if (rit == readers_.end())
    {
        logWarning(DISCOVERY_DATABASE,
                "Attempting to unmatch an unexisting reader: " << guid);
        return;
    }

    // get reader topic
    std::string topic = rit->second.topic();

    // remove it from reader by topic
    remove_reader_from_topic_(guid, topic);

    // it there are more than one reader in this topic in the same participant we do not unmatch the endpoints
    if (!repeated_reader_topic_(guid.guidPrefix, topic))
    {
        // for each writer in same topic make not relevant. It could be none in writers
        auto tit = writers_by_topic_.find(topic);
        if (tit != writers_by_topic_.end())
        {
            for (auto writer : tit->second)
            {
                auto wit = writers_.find(writer);
                if (wit == writers_.end())
                {
                    logWarning(DISCOVERY_DATABASE,
                            "Unexisting writer " << writer << " in topic: " << topic);
                }

                wit->second.remove_participant(guid.guidPrefix);
            }
        }
    }
}

bool DiscoveryDataBase::repeated_writer_topic_(
        const eprosima::fastrtps::rtps::GuidPrefix_t& participant,
        const std::string& topic_name)
{
    int count = 0;

    auto pit = participants_.find(participant);
    if (pit == participants_.end())
    {
        logWarning(DISCOVERY_DATABASE,
                "Checking repeated writer topics in an unexisting participant: " << participant);
        return false;
    }

    for (auto writer_guid : pit->second.writers())
    {
        auto wit = writers_.find(writer_guid);
        if (wit == writers_.end())
        {
            logWarning(DISCOVERY_DATABASE,
                    "writer missing: " << writer_guid);
        }

        if (wit->second.topic() == topic_name)
        {
            ++count;
            if (count > 1)
            {
                return true;
            }
        }
    }

    // we already know is false. Safety check
    return count > 1;
}

// return if there are more than one reader in the participant in the same topic
bool DiscoveryDataBase::repeated_reader_topic_(
        const eprosima::fastrtps::rtps::GuidPrefix_t& participant,
        const std::string& topic_name)
{
    int count = 0;

    auto pit = participants_.find(participant);
    if (pit == participants_.end())
    {
        logWarning(DISCOVERY_DATABASE,
                "Checking repeated reader topics in an unexisting participant: " << participant);
        return false;
    }

    for (auto reader_guid : pit->second.readers())
    {
        auto rit = readers_.find(reader_guid);
        if (rit == readers_.end())
        {
            logWarning(DISCOVERY_DATABASE,
                    "reader missing: " << reader_guid);
        }

        if (rit->second.topic() == topic_name)
        {
            ++count;
            if (count > 1)
            {
                return true;
            }
        }
    }

    // we already know is false. Safety check
    return count > 1;
}

void DiscoveryDataBase::remove_writer_from_topic_(
        const eprosima::fastrtps::rtps::GUID_t& writer_guid,
        const std::string& topic_name)
{
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
}

void DiscoveryDataBase::remove_reader_from_topic_(
        const eprosima::fastrtps::rtps::GUID_t& reader_guid,
        const std::string& topic_name)
{
    logInfo(DISCOVERY_DATABASE, "removing: " << reader_guid << " from topic " << topic_name);

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
}

void DiscoveryDataBase::add_writer_to_topic_(
        const eprosima::fastrtps::rtps::GUID_t& writer_guid,
        const std::string& topic_name)
{
    /* Manage adding a virtual writer */
    bool virtual_topic_is_new = true;
    // If the topic is virtual, add  the writer to all existing topics
    if (topic_name == virtual_topic_)
    {
        // Add writer to all topics
        for (auto topic: writers_by_topic_)
        {
            // Check if the writer is already there before adding it
            auto topic_it = std::find(topic.second.begin(), topic.second.end(), writer_guid);
            if (topic_it == topic.second.end())
            {
                logInfo(DISCOVERY_DATABASE,
                    "Adding virtual writer " << writer_guid << " to writers_by_topic_[ " << topic_name << "]");
                topic.second.push_back(writer_guid);
            }
            // We should never try to add a new entry if the writer was already in the topic
            else
            {
                logWarning(DISCOVERY_DATABASE,
                    "Trying to add already present virtual writer " << writer_guid << " to writers_by_topic_[ "
                    << topic_name << "]. Ignored");
            }

            // The virtual topic already existed, so we don't need to create the entry for it later on
            if (topic.first == virtual_topic_)
            {
                virtual_topic_is_new = false;
            }
        }

        // There is no virtual_topic_ entry in writers_by_topic_. Therefore, we need to create one and add this virutal
        // writer
        if (virtual_topic_is_new)
        {
            // Try to insert the writer
            auto retu = writers_by_topic_.insert(
                std::pair<std::string, std::vector<fastrtps::rtps::GUID_t>>(
                    topic_name,
                    std::vector<fastrtps::rtps::GUID_t>({writer_guid})
                )
            );
            if (!retu.second)
            {
                logError(DISCOVERY_DATABASE, "Could not insert writer " << writer_guid << " in topic " << topic_name);
            }
            else
            {
                logInfo(DISCOVERY_DATABASE, "New topic in writers_by_topic_: " << topic_name);
            }
        }
        // If the topic was virtual, then there is no extra work to do
        return;
    }

    /* Update writers by topic */
    // At this point we know that the topic is real (NOT virutal)
    // Find the topic_name entry in writers_by_topic_
    std::map<std::string, std::vector<eprosima::fastrtps::rtps::GUID_t> >::iterator topic_it =
            writers_by_topic_.find(topic_name);

    // There was not topic_name entry in writers_by_topic_, so the topic is new
    if (topic_it == writers_by_topic_.end())
    {
        // Try to insert the writer
        auto retu = writers_by_topic_.insert(
            std::pair<std::string, std::vector<fastrtps::rtps::GUID_t>>(
                topic_name,
                std::vector<fastrtps::rtps::GUID_t>({writer_guid})
            )
        );
        if (!retu.second)
        {
            logError(DISCOVERY_DATABASE, "Could not insert writer " << writer_guid << " in topic " << topic_name);
        }
        else
        {
            logInfo(DISCOVERY_DATABASE, "New topic in writers_by_topic_: " << topic_name);

            // Add all known virtual writers to this new topic
            auto virtual_topic_it = writers_by_topic_.find(virtual_topic_);
            if (virtual_topic_it != writers_by_topic_.end())
            {
                for (auto writer: virtual_topic_it->second)
                {
                    logInfo(DISCOVERY_DATABASE, "Adding virtual writer " << writer << " to topic " << topic_name);
                    retu.first->second.push_back(writer);
                }
            }
        }
    }
    // The topic entry already existed
    else
    {
        // Check if the writer was already present in the collection before adding it
        std::vector<eprosima::fastrtps::rtps::GUID_t>::iterator writer_by_topic_it =
                std::find(topic_it->second.begin(), topic_it->second.end(), writer_guid);
        if (writer_by_topic_it == topic_it->second.end())
        {
            logInfo(DISCOVERY_DATABASE, "New writer " << writer_guid << " in writers_by_topic: " << topic_name);
            topic_it->second.push_back(writer_guid);
        }
    }
}

void DiscoveryDataBase::add_reader_to_topic_(
        const eprosima::fastrtps::rtps::GUID_t& reader_guid,
        const std::string& topic_name)
{
    // Update readers_by_topic
    std::map<std::string, std::vector<eprosima::fastrtps::rtps::GUID_t>>::iterator topic_it =
            readers_by_topic_.find(topic_name);
    if (topic_it != readers_by_topic_.end())
    {
        std::vector<eprosima::fastrtps::rtps::GUID_t>::iterator reader_by_topic_it =
                std::find(topic_it->second.begin(), topic_it->second.end(), reader_guid);
        if (reader_by_topic_it == topic_it->second.end())
        {
            logInfo(DISCOVERY_DATABASE, "New reader " << reader_guid << " in readers_by_topic: " << topic_name);
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
        readers_by_topic_[topic_name] = std::vector<fastrtps::rtps::GUID_t>{reader_guid};
        logInfo(DISCOVERY_DATABASE, "New topic in readers_by_topic: " << topic_name);
    }
}

bool DiscoveryDataBase::delete_participant_entity_(
        const fastrtps::rtps::GuidPrefix_t& guid_prefix)
{
    auto it = participants_.find(guid_prefix);
    if (it == participants_.end())
    {
        return false;
    }
    delete_participant_entity_(it);

    return true;
}

std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator
DiscoveryDataBase::delete_participant_entity_(
        std::map<eprosima::fastrtps::rtps::GuidPrefix_t, DiscoveryParticipantInfo>::iterator it)
{
    logInfo(DISCOVERY_DATABASE, "Deleting participant: " << it->first);
    if (it == participants_.end())
    {
        return participants_.end();
    }
    changes_to_release_.push_back(it->second.change());
    return participants_.erase(it);
}

bool DiscoveryDataBase::delete_reader_entity_(
        const fastrtps::rtps::GUID_t& guid)
{
    // find own reader
    auto it = readers_.find(guid);
    if (it == readers_.end())
    {
        return false;
    }

    delete_reader_entity_(it);
    return true;
}

std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator DiscoveryDataBase::delete_reader_entity_(
        std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator it)
{
    logInfo(DISCOVERY_DATABASE, "Deleting reader: " << it->first.guidPrefix);
    if (it == readers_.end())
    {
        return readers_.end();
    }
    // Remove entity from participant readers vector
    auto pit = participants_.find(it->first.guidPrefix);
    if (pit == participants_.end())
    {
        logError(DISCOVERY_DATABASE, "Attempting to delete and orphan reader");
        return it;
    }
    pit->second.remove_reader(it->first);

    changes_to_release_.push_back(it->second.change());

    // remove entity in readers_ map
    return readers_.erase(it);
}

bool DiscoveryDataBase::delete_writer_entity_(
        const fastrtps::rtps::GUID_t& guid)
{
    // find own writer
    auto it = writers_.find(guid);
    if (it == writers_.end())
    {
        return false;
    }

    delete_writer_entity_(it);
    return true;
}

std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator DiscoveryDataBase::delete_writer_entity_(
        std::map<eprosima::fastrtps::rtps::GUID_t, DiscoveryEndpointInfo>::iterator it)
{
    logInfo(DISCOVERY_DATABASE, "Deleting writer: " << it->first.guidPrefix);
    if (it == writers_.end())
    {
        return writers_.end();
    }
    // Remove entity from participant writers vector
    auto pit = participants_.find(it->first.guidPrefix);
    if (pit == participants_.end())
    {
        logError(DISCOVERY_DATABASE, "Attempting to delete and orphan writer");
        return it;
    }
    pit->second.remove_writer(it->first);

    changes_to_release_.push_back(it->second.change());

    // remove entity in writers_ map
    return writers_.erase(it);
}

} // namespace ddb
} // namespace rtps
} // namespace fastdds
} // namespace eprosima
