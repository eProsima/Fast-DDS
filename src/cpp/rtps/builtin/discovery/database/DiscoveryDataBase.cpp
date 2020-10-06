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

using namespace eprosima::fastdds::rtps::ddb;

DiscoveryDataBase::DiscoveryDataBase(
        GuidPrefix_t server_guid_prefix)
    : server_guid_prefix_(server_guid_prefix)
{
}

bool DiscoveryDataBase::pdp_is_relevant(
        const CacheChange_t& change,
        const GUID_t& reader_guid) const
{
    // Lock(shared mode) mutex locally
    std::lock_guard<share_mutex_t> lock(sh_mtx_);

    // Get identity of the participant that generated the DATA(p|Up)
    GuidPrefix_t change_guid_prefix = guid_from_change(&change).guidPrefix;

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
        return (it->second.is_relevant_participant(reader_guid.guidPrefix) &&
               !it->second.is_matched(reader_guid.guidPrefix));
    }
    else
    {
        // should be there
        logError(DISCOVERY_DATABASE, "Change in PDP writer: " << change_guid_prefix
                << " without related entry in database.");
    }
    // Not relevant
    return false;
}

bool DiscoveryDataBase::edp_publications_is_relevant(
        const CacheChange_t& change,
        const GUID_t& reader_guid) const
{
    // Lock(shared mode) mutex locally
    std::lock_guard<share_mutex_t> lock(sh_mtx_);

    GUID_t writer_guid = guid_from_change(&change);
    auto itp = participants_.find(writer_guid.guidPrefix);

    if (itp == participants_.end())
    {
        // should be there
        logError(DISCOVERY_DATABASE, "Change in publications EDP writer: " << writer_guid
                << " without related participant entry in database.");
        return false;
    }
    else if (!itp->second.is_matched(reader_guid.guidPrefix))
    {
        // Reader client hasn't matched with the writer client
        return false;
    }

    auto itw = writers_.find(writer_guid);
    if (itw != writers_.end())
    {
        // it is relevant if the ack has not been received yet
        return (itw->second.is_relevant_participant(reader_guid.guidPrefix) &&
               !itw->second.is_matched(reader_guid.guidPrefix));
    }
    else
    {
        // should be there
        logError(DISCOVERY_DATABASE, "Change in Publishers EDP writer: " << writer_guid
                << " without related entry in database.");
    }

    // not relevant
    return false;
}

bool DiscoveryDataBase::edp_subscriptions_is_relevant(
        const CacheChange_t& change,
        const GUID_t& reader_guid) const
{
    // Lock(shared mode) mutex locally
    std::lock_guard<share_mutex_t> lock(sh_mtx_);

    GUID_t endpoint_guid = guid_from_change(&change);
    auto itp = participants_.find(endpoint_guid.guidPrefix);

    if (itp == participants_.end())
    {
        // should be there
        logError(DISCOVERY_DATABASE, "Change in Subscriptions EDP writer: " << endpoint_guid
                << " without related participant entry in database.");
        return false;
    }
    else if (!itp->second.is_matched(reader_guid.guidPrefix))
    {
        // participant still not matched
        return false;
    }

    auto itr = readers_.find(endpoint_guid);
    if (itr != readers_.end())
    {
        // it is relevant if the ack has not been received yet
        return (itr->second.is_relevant_participant(reader_guid.guidPrefix) &&
               !itr->second.is_matched(reader_guid.guidPrefix));
    }
    else
    {
        // should be there
        logError(DISCOVERY_DATABASE, "Change in Subscribers EDP writer: " << endpoint_guid
                << " without related entry in database.");
    }

    // not relevant
    return false;
}


void DiscoveryDataBase::add_ack_(
        const CacheChange_t* change,
        const GuidPrefix_t& acked_entity)
{
    // This method is only called from AckedFunctor thus the ddb lock is already taken

    if (is_participant(change))
    {
        logInfo(DISCOVERY_DATABASE,
                "Adding DATA(p) ACK for change " << change->instanceHandle << " to " << acked_entity);
        auto it = participants_.find(guid_from_change(change).guidPrefix);
        if (it != participants_.end())
        {
            it->second.add_or_update_ack_participant(acked_entity, true);
        }
        else
        {
            // should be there
            logError(DISCOVERY_DATABASE, "Acknowledgement callback for change: " << change->instanceHandle
                    << " without related participant entry in database.");
        }
    }
    else if (is_writer(change))
    {
        logInfo(DISCOVERY_DATABASE,
                "Adding DATA(w) ACK for change " << change->instanceHandle << " to " << acked_entity);
        auto it = writers_.find(guid_from_change(change));
        if (it != writers_.end())
        {
            it->second.add_or_update_ack_participant(acked_entity, true);
        }
        else
        {
            // should be there
            logError(DISCOVERY_DATABASE, "Acknowledgement callback for change: " << change->instanceHandle
                    << " without related publisher entry in database.");
        }
    }
    else if (is_reader(change))
    {
        logInfo(DISCOVERY_DATABASE,
                "Adding DATA(r) ACK for change " << change->instanceHandle << " to " << acked_entity);
        auto it = readers_.find(guid_from_change(change));
        if (it != readers_.end())
        {
            it->second.add_or_update_ack_participant(acked_entity, true);
        }
        else
        {
            // should be there
            logError(DISCOVERY_DATABASE, "Acknowledgement callback for change: " << change->instanceHandle
                    << " without related publisher entry in database.");
        }
    }
}

bool DiscoveryDataBase::update(
        CacheChange_t* change,
        std::string topic_name)
{
    logInfo(DISCOVERY_DATABASE, "Adding change to the queue: " << change->instanceHandle);
    //  add the data to the queue to process
    data_queue_.Push(eprosima::fastdds::rtps::ddb::DiscoveryDataQueueInfo(change, topic_name));

    // not way to check if is an error
    return true;
}

const std::vector<CacheChange_t*> DiscoveryDataBase::changes_to_dispose()
{
    // lock(sharing mode) mutex locally
    std::lock_guard<share_mutex_t> lock(sh_mtx_);
    return disposals_;
}

void DiscoveryDataBase::clear_changes_to_dispose()
{
    // lock(exclusive mode) mutex locally
    std::lock_guard<share_mutex_t> lock(sh_mtx_);
    disposals_.clear();
}

////////////
// Functions to process_to_send_lists()
const std::vector<CacheChange_t*> DiscoveryDataBase::pdp_to_send()
{
    // lock(sharing mode) mutex locally
    std::lock_guard<share_mutex_t> lock(sh_mtx_);
    return pdp_to_send_;
}

void DiscoveryDataBase::clear_pdp_to_send()
{
    // lock(exclusive mode) mutex locally
    std::lock_guard<share_mutex_t> lock(sh_mtx_);
    pdp_to_send_.clear();
}

const std::vector<CacheChange_t*> DiscoveryDataBase::edp_publications_to_send()
{
    // lock(sharing mode) mutex locally
    std::lock_guard<share_mutex_t> lock(sh_mtx_);
    return edp_publications_to_send_;
}

void DiscoveryDataBase::clear_edp_publications_to_send()
{
    // lock(exclusive mode) mutex locally
    std::lock_guard<share_mutex_t> lock(sh_mtx_);
    edp_publications_to_send_.clear();
}

const std::vector<CacheChange_t*> DiscoveryDataBase::edp_subscriptions_to_send()
{
    // lock(sharing mode) mutex locally
    std::lock_guard<share_mutex_t> lock(sh_mtx_);
    return edp_subscriptions_to_send_;
}

void DiscoveryDataBase::clear_edp_subscriptions_to_send()
{
    // lock(exclusive mode) mutex locally
    std::lock_guard<share_mutex_t> lock(sh_mtx_);
    edp_subscriptions_to_send_.clear();
}

const std::vector<CacheChange_t*> DiscoveryDataBase::changes_to_release()
{
    // lock(sharing mode) mutex locally
    std::lock_guard<share_mutex_t> lock(sh_mtx_);
    return changes_to_release_;
}

void DiscoveryDataBase::clear_changes_to_release()
{
    // lock(exclusive mode) mutex locally
    std::lock_guard<share_mutex_t> lock(sh_mtx_);
    changes_to_release_.clear();
}

////////////
// Functions to process_data_queue()
bool DiscoveryDataBase::process_data_queue()
{
    bool is_dirty_topic = false;

    // This method will be always called from the worker thread, that is, not two simultaneous calls could ever take
    // place. We swap the queue before locking in order to avoid ABBA locks by avoiding taking DBQueue mutexes and the
    // database one simultaneously.

    // Swap DATA queues
    data_queue_.Swap();

    CacheChange_t* change;
    std::string topic_name;

    // Process all messages in the queque
    while (!data_queue_.Empty())
    {
        // Process each message with Front()
        DiscoveryDataQueueInfo data_queue_info = data_queue_.Front();
        change = data_queue_info.change();
        topic_name = data_queue_info.topic();

        // If the change is a DATA(p|w|r)
        if (change->kind == ALIVE)
        {
            logInfo(DISCOVERY_DATABASE, "ALIVE change received from: " << change->instanceHandle);
            // DATA(p) case
            if (is_participant(change))
            {
                // Update participants map
                logInfo(DISCOVERY_DATABASE, "DATA(p) received from: " << change->instanceHandle);
                create_participant_from_change(change);
            }
            // DATA(w) case
            else if (is_writer(change))
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

            // Lock(exclusive mode) mutex locally
            std::lock_guard<share_mutex_t> lock(sh_mtx_);

            // Update set of dirty_topics
            // In case of Data(p), topic name is empty, so no topic should be added to dirty_topics_
            if ((topic_name != "") &&
                    std::find(dirty_topics_.begin(), dirty_topics_.end(),
                    topic_name) == dirty_topics_.end())
            {
                logInfo(DISCOVERY_DATABASE, "Setting topic " << topic_name << " as dirty");
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
        CacheChange_t* ch)
{
    std::lock_guard<share_mutex_t> lock(sh_mtx_);

    DiscoveryParticipantInfo part(ch);
    std::pair<std::map<GuidPrefix_t, DiscoveryParticipantInfo>::iterator, bool> ret =
            participants_.insert(std::make_pair(guid_from_change(ch).guidPrefix, part));

    // If insert returns false, means that the participant already existed (DATA(p) is an update). In that case
    // we need to update the change related to the participant and return the old change to the pool
    if (!ret.second)
    {
        // Add old change to changes_to_release_
        // Note the relevant participants collection is not cleared. That list is composed based on topic matching and
        // should not change by a DATA(p) update.
        logInfo(DISCOVERY_DATABASE, "Participant updating. Marking old change to release");
        changes_to_release_.push_back(ret.first->second.set_change_and_unmatch(ch));
    }
    else
    {
        // In this case the relevant participants collection of the newbie is empty.
        GUID_t change_guid = guid_from_change(ch);
        logInfo(DISCOVERY_DATABASE, "New participant added: " << change_guid.guidPrefix);
        if (change_guid.guidPrefix == server_guid_prefix_)
        {
            // DiscoveryDataBase would only received own server DATA announcement on updates, that is, it cannot be on
            // the current pdp_to_send_ array
            pdp_to_send_.push_back(ch);

#if (defined(__INTERNALDEBUG) || defined(_INTERNALDEBUG))
            if (std::find(
                        pdp_to_send_.begin(),
                        pdp_to_send_.end(),
                        ch) != pdp_to_send_.end())
            {
                logError(DISCOVERY_DATABASE, "Server DATA(p) "
                        << ch->instanceHandle << " processed twice.");
            }
#endif
        }
    }

}

void DiscoveryDataBase::create_writers_from_change(
        CacheChange_t* ch,
        const std::string& topic_name)
{
    std::lock_guard<share_mutex_t> lock(sh_mtx_);

    const GUID_t writer_guid = guid_from_change(ch);

    // Update participants_::writers. The participant Discovery ancillary should be there and updated
    std::map<GuidPrefix_t, DiscoveryParticipantInfo>::iterator w_pit =
            participants_.find(writer_guid.guidPrefix);
    if (w_pit != participants_.end())
    {
        // The collection of relevant participants for this DiscoveryParticipantInfo will be populated below
        w_pit->second.add_writer(writer_guid);
    }
    else
    {
        // the participant should be there because DATA(p)s always precede DATA(r|w) in the design either by builtin
        // endpoint matching like in clients our using the relevance mechanism like in servers.
        logError(DISCOVERY_DATABASE, "Processing DATA(w) " << writer_guid << " before corresponding DATA(p)");
        return;
    }

    // Update writers_
    DiscoveryEndpointInfo tmp_writer(ch, topic_name);
    std::pair<std::map<GUID_t, DiscoveryEndpointInfo>::iterator, bool> ret =
            writers_.insert(std::make_pair(writer_guid, tmp_writer));

    // If the publisher info does not exists, create the Discovery ancillary and update the matching state.
    // The readers suitable for matching can be retrieved using the per topic reader collection.
    // Note that we must update relevance for:
    // - suitable subscribers DATA(r) and it's corresponding participants DATA(p)
    // - new publisher DATA(w) and it's corresponding participant DATA(p)
    if (ret.second)
    {
        std::map<GUID_t, DiscoveryEndpointInfo>::iterator writer_it =
                writers_.find(writer_guid);

        std::map<std::string, std::vector<GUID_t>>::iterator readers_it =
                readers_by_topic_.find(topic_name);

        if (readers_it != readers_by_topic_.end())
        {
            for (GUID_t reader_it: readers_it->second)
            {
                // New Publisher DATA(w):
                // Each participant of a suitable subscriber must acknowledge the new publisher's info:
                // the DiscoveryEndpointInfo corresponding to writer_guid must have false entries for the suitable
                // subscribers.
                writer_it->second.add_or_update_ack_participant(reader_it.guidPrefix);

                // Update the participant ack status list from readers_
                std::map<GUID_t, DiscoveryEndpointInfo>::iterator rit =
                        readers_.find(reader_it);
                if (rit != readers_.end())
                {
                    // Suitable subscribers DATA(r):
                    // The new publisher's participant must acknowledge the suitable subscribers' info:
                    // the DiscoveryEndpointInfo corresponding to each subscriber must have an entry for the
                    // writer_guid.guidPrefix participant if not relevant yet.
                    if (!rit->second.is_matched(writer_guid.guidPrefix))
                    {
                        rit->second.add_or_update_ack_participant(writer_guid.guidPrefix);
                    }
                }

                // Update the participant ack status list from participants_
                std::map<GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
                        participants_.find(reader_it.guidPrefix);
                if (pit != participants_.end())
                {
                    // Suitable subscribers' participant DATA(p):
                    // Each participant of a suitable subscriber must be acknowledge by the new publisher's participant.
                    // the DiscoveryParticipantInfo corresponding to each subcriber's participant must have an entry for
                    // the writer_guid.guidPrefix participant if not there yet.
                    if (!pit->second.is_matched(writer_guid.guidPrefix))
                    {
                        pit->second.add_or_update_ack_participant(writer_guid.guidPrefix);
                    }
                }

                // New Publisher's participant DATA(p):
                // Each participant of a suitable subscriber must acknowledge the new publisher's participant info:
                // the DiscoveryParticipantInfo corresponding to writer_guid.guidPrefix must have an entry for each
                // suitable subscriber participant if not yet.
                if (!w_pit->second.is_matched(reader_it.guidPrefix))
                {
                    if (!w_pit->second.is_matched(reader_it.guidPrefix))
                    {
                        w_pit->second.add_or_update_ack_participant(reader_it.guidPrefix);
                    }
                }
            }
        }

        // Update writers_by_topic
        std::map<std::string, std::vector<GUID_t>>::iterator topic_it =
                writers_by_topic_.find(topic_name);

        if (topic_it != writers_by_topic_.end())
        {
#if (defined(__INTERNALDEBUG) || defined(_INTERNALDEBUG))
            std::vector<GUID_t>::iterator writer_by_topic_it =
                    std::find(topic_it->second.begin(), topic_it->second.end(), writer_guid);
            if (writer_by_topic_it != topic_it->second.end())
            {
                logError(DISCOVERY_DATABASE, "The publisher " << writer_guid << " was already in topic " << topic_name
                        << " collection.");
                return;
            }
#endif
            // add the new publisher to this topic list
            topic_it->second.push_back(writer_guid);
       }
        // This is the first writer in the topic
        else
        {
            std::vector<GUID_t> writers_in_topic = {writer_guid};
            auto topic_iterator = writers_by_topic_.insert(
                std::pair<std::string, std::vector<GUID_t>>(topic_name, writers_in_topic));
            if (!topic_iterator.second)
            {
                logError(DISCOVERY_DATABASE, "Could not insert writer " << writer_guid << " in topic " << topic_name);
            }
        }
    }
    // If writer exists, update the change and set all participant ack status to false
    // In this case only the DATA(w) data changes all other DATA(r) and DATA(p)s are already processed
    else
    {
        changes_to_release_.push_back(ret.first->second.set_change_and_unmatch(ch));
    }

}

void DiscoveryDataBase::create_readers_from_change(
        CacheChange_t* ch,
        const std::string& topic_name)
{
    std::lock_guard<share_mutex_t> lock(sh_mtx_);

    const GUID_t reader_guid = guid_from_change(ch);

    // Update participants_::readers. The participant Discovery ancillary should be there and updated
    std::map<GuidPrefix_t, DiscoveryParticipantInfo>::iterator r_pit =
            participants_.find(reader_guid.guidPrefix);
    if (r_pit != participants_.end())
    {
        // The collection of relevant participants for this DiscoveryParticipantInfo will be populated below
        r_pit->second.add_reader(reader_guid);
    }
    else
    {
        // the participant should be there because DATA(p)s always precede DATA(r|w) in the design either by builtin
        // endpoint matching like in clients our using the relevance mechanism like in servers.
        logError(DISCOVERY_DATABASE, "Processing DATA(r) " << reader_guid << " before corresponding DATA(p)");
        return;
    }

    DiscoveryEndpointInfo tmp_reader(ch, topic_name);
    std::pair<std::map<GUID_t, DiscoveryEndpointInfo>::iterator, bool> ret =
            readers_.insert(std::make_pair(reader_guid, tmp_reader));

    // If the subscriber info does not exists, create the Discovery ancillary and update the matching state.
    // The writers suitable for matching can be retrieved using the per topic writer collection.
    // Note that we must update relevance for:
    // - suitable publishers DATA(w) and it's corresponding participants DATA(p)
    // - new subscriber DATA(r) and it's corresponding participant DATA(p)
    if (ret.second)
    {
        std::map<GUID_t, DiscoveryEndpointInfo>::iterator reader_it =
                readers_.find(reader_guid);

        std::map<std::string, std::vector<GUID_t>>::iterator writers_it =
                writers_by_topic_.find(topic_name);

        if (writers_it != writers_by_topic_.end())
        {
            for (GUID_t writer_it: writers_it->second)
            {
                // New Subscriber DATA(r):
                // Each participant of a suitable publisher must acknowledge the new subscriber's info:
                // the DiscoveryEndpointInfo corresponding to reader_guid must have false entries for the suitable
                // publishers.
                reader_it->second.add_or_update_ack_participant(writer_it.guidPrefix);

                // Update the participant ack status list from writers_
                std::map<GUID_t, DiscoveryEndpointInfo>::iterator wit =
                        writers_.find(writer_it);
                if (wit != writers_.end())
                {
                    // Suitable publishers DATA(w):
                    // The new subscriber's participant must acknowledge the suitable publishers' info:
                    // the DiscoveryEndpointInfo corresponding to each publisher must have an entry for the
                    // reader_guid.guidPrefix participant if not relevant yet.
                    if (!wit->second.is_matched(reader_guid.guidPrefix))
                    {
                        wit->second.add_or_update_ack_participant(reader_guid.guidPrefix);
                    }
                }

                // Update the participant ack status list from participants_
                std::map<GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
                        participants_.find(writer_it.guidPrefix);
                if (pit != participants_.end())
                {
                    // Suitable publishers' participant DATA(p):
                    // Each participant of a suitable publisher must be acknowledge by the new subscriber's participant.
                    // the DiscoveryParticipantInfo corresponding to each publisher's participant must have an entry for
                    // the reader_guid.guidPrefix participant if not there yet.
                    if (!pit->second.is_matched(reader_guid.guidPrefix))
                    {
                        pit->second.add_or_update_ack_participant(reader_guid.guidPrefix);
                    }
                }

                // New Subscriber's participant DATA(p):
                // Each participant of a suitable publisher must acknowledge the new subscriber's participant info:
                // the DiscoveryParticipantInfo corresponding to reader_guid.guidPrefix must have an entry for each
                // suitable publisher participant if not yet.
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
        std::map<std::string, std::vector<GUID_t>>::iterator topic_it =
                readers_by_topic_.find(topic_name);

        if (topic_it != readers_by_topic_.end())
        {
#if (defined(__INTERNALDEBUG) || defined(_INTERNALDEBUG))
            std::vector<GUID_t>::iterator reader_by_topic_it =
                    std::find(topic_it->second.begin(), topic_it->second.end(), reader_guid);
            if (reader_by_topic_it != topic_it->second.end())
            {
                logError(DISCOVERY_DATABASE, "The subscriber " << reader_guid << " was already in topic " << topic_name
                        << " collection.");
                return;
            }
#endif
            // add the new publisher to this topic list
            topic_it->second.push_back(reader_guid);
        }
        // This is the first reader in the topic
        else
        {
            std::vector<GUID_t> readers_in_topic = {reader_guid};
            auto topic_iterator = readers_by_topic_.insert(
                std::pair<std::string, std::vector<GUID_t>>(topic_name, readers_in_topic));
            if (!topic_iterator.second)
            {
                logError(DISCOVERY_DATABASE, "Could not insert reader " << reader_guid << " in topic " << topic_name);
            }
        }
    }
    // If reader exists, update the change and set all participant ack status to false
    // In this case only the DATA(r) data changes all other DATA(w) and DATA(p)s are already processed
    else
    {
        changes_to_release_.push_back(ret.first->second.set_change_and_unmatch(ch));
    }
}

void DiscoveryDataBase::process_dispose_participant(
        CacheChange_t* ch)
{
    std::lock_guard<share_mutex_t> lock(sh_mtx_);

    const GUID_t participant_guid = guid_from_change(ch);

    // Change DATA(p) with DATA(Up) in participants map
    std::map<GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
            participants_.find(participant_guid.guidPrefix);
    if (pit != participants_.end())
    {
        changes_to_release_.push_back(pit->second.set_change_and_unmatch(ch));
    }

    // Delete entries from writers_ belonging to the participant
    for (auto wit = writers_.begin(); wit != writers_.end();)
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
        CacheChange_t* ch,
        const std::string& topic_name)
{
    std::lock_guard<share_mutex_t> lock(sh_mtx_);

    const GUID_t writer_guid = guid_from_change(ch);

    // Change DATA(w) with DATA(Uw)
    std::map<GUID_t, DiscoveryEndpointInfo>::iterator wit = writers_.find(writer_guid);
    if (wit != writers_.end())
    {
        changes_to_release_.push_back(wit->second.set_change_and_unmatch(ch));
    }

    // Update own entry participants_::writers
    std::map<GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
            participants_.find(writer_guid.guidPrefix);
    if (pit != participants_.end())
    {
        pit->second.remove_writer(writer_guid);
    }

    // Update own entry writers_by_topic_
    std::map<std::string, std::vector<GUID_t>>::iterator tit =
            writers_by_topic_.find(topic_name);
    if (tit != writers_by_topic_.end())
    {
        for (std::vector<GUID_t>::iterator writer_it = tit->second.begin();
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
        CacheChange_t* ch,
        const std::string& topic_name)
{
    std::lock_guard<share_mutex_t> lock(sh_mtx_);

    const GUID_t reader_guid = guid_from_change(ch);

    // Change DATA(r) with DATA(Ur)
    std::map<GUID_t, DiscoveryEndpointInfo>::iterator rit = readers_.find(reader_guid);
    if (rit != readers_.end())
    {
        changes_to_release_.push_back(rit->second.set_change_and_unmatch(ch));
    }

    // Update own entry participants_::readers
    std::map<GuidPrefix_t, DiscoveryParticipantInfo>::iterator pit =
            participants_.find(reader_guid.guidPrefix);
    if (pit != participants_.end())
    {
        pit->second.remove_reader(reader_guid);
    }

    // Update own entry readers_by_topic_
    std::map<std::string, std::vector<GUID_t>>::iterator tit =
            readers_by_topic_.find(topic_name);
    if (tit != readers_by_topic_.end())
    {
        for (std::vector<GUID_t>::iterator reader_it = tit->second.begin();
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
    std::lock_guard<share_mutex_t> lock(sh_mtx_);

    // Iterator objects are declared here because they are reused in each iteration of the loops
    std::map<GuidPrefix_t, DiscoveryParticipantInfo>::iterator parts_reader_it;
    std::map<GuidPrefix_t, DiscoveryParticipantInfo>::iterator parts_writer_it;
    std::map<GUID_t, DiscoveryEndpointInfo>::iterator readers_it;
    std::map<GUID_t, DiscoveryEndpointInfo>::iterator writers_it;

    // Iterate over dirty_topics_
    for (auto topic_it = dirty_topics_.begin(); topic_it != dirty_topics_.end();)
    {
        logInfo(DISCOVERY_DATABASE, "Processing topic: " << *topic_it);
        // Flag to store whether a topic can be cleared.
        bool is_clearable = true;

        // Get all the endpoints in the topic
        auto wret = writers_by_topic_.find(*topic_it);
        auto rret = readers_by_topic_.find(*topic_it);

        // If there cannot be match skip
        if ( wret == writers_by_topic_.end()
                || rret == readers_by_topic_.end() )
        {
            ++topic_it;
            continue;
        }

        // Traverse the collections assessing which DATAs should be add to the server's builtin writers
        std::vector<GUID_t>& writers = wret->second;
        std::vector<GUID_t>& readers = rret->second;

        for (GUID_t& writer: writers)
        // Iterate over writers in the topic:
        {
            logInfo(DISCOVERY_DATABASE, "[" << *topic_it << "]" << " Processing writer: " << writer);
            // Iterate over readers in the topic:
            for (GUID_t& reader : readers)
            {
                logInfo(DISCOVERY_DATABASE, "[" << *topic_it << "]" << " Processing reader: " << reader);
                // Find participants with writer info and participant with reader info in participants_
                parts_reader_it = participants_.find(reader.guidPrefix);
                parts_writer_it = participants_.find(writer.guidPrefix);
                // Find reader info in readers_
                readers_it = readers_.find(reader);
                // Find writer info in writers_
                writers_it = writers_.find(writer);

                if ( parts_reader_it == participants_.end()
                        || parts_writer_it == participants_.end()
                        || readers_it == readers_.end()
                        || writers_it == writers_.end() )
                {
                    // The relationship between the different collections should not be broken. If they are it hints
                    // possible synchronization issues
                    logError(DISCOVERY_DATABASE, "process_dirty_topics found incoherent database state.");
                    continue;
                }

                // Check if the writer's participant knows alreay the reader's participant. If not, keep or add the
                // reader's participant DATA(p) in the server's PDP writer
                if (parts_reader_it->second.is_matched(writer.guidPrefix))
                {
                    // If writer's participant knows reader's one check if the reader is know to writer's participant
                    // If it's not acknowledge we add or keep the DATA(r) in the server's EDP sub writer
                    if (!readers_it->second.is_matched(writer.guidPrefix))
                    {
                        if (std::find(
                                    edp_subscriptions_to_send_.begin(),
                                    edp_subscriptions_to_send_.end(),
                                    readers_it->second.change()) == edp_subscriptions_to_send_.end())
                        {
                            logInfo(DISCOVERY_DATABASE, "Addind DATA(r) to send: "
                                    << readers_it->second.change()->instanceHandle);
                            edp_subscriptions_to_send_.push_back(readers_it->second.change());
                        }
                        // pending acknowledgements on this topic
                        is_clearable = false;
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

                    // pending acknowledgements on this topic
                    is_clearable = false;
                }

                // Check if the reader's participant knows alreay the writers's participant. If not, keep or add the
                // writer's participant DATA(p) in the server's PDP writer
                if (parts_writer_it->second.is_matched(reader.guidPrefix))
                {
                   // If reader's participant knows writer's one check if the writer is know to reader's participant
                   // If it's not acknowledge we add or keep the DATA(w) in the server's EDP pub writer
                   if (!writers_it->second.is_matched(reader.guidPrefix))
                    {
                        if (std::find(
                                    edp_publications_to_send_.begin(),
                                    edp_publications_to_send_.end(),
                                    writers_it->second.change()) == edp_publications_to_send_.end())
                        {
                            logInfo(DISCOVERY_DATABASE, "Addind DATA(w) to send: "
                                    << writers_it->second.change()->instanceHandle);
                            edp_publications_to_send_.push_back(writers_it->second.change());
                        }
                        // pending acknowledgements on this topic
                        is_clearable = false;
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
                    // pending acknowledgements on this topic
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
        CacheChange_t* change)
{
    // Lock(exclusive mode) mutex locally
    std::lock_guard<share_mutex_t> lock(sh_mtx_);

    if (change->kind == ChangeKind_t::ALIVE)
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

bool DiscoveryDataBase::is_participant(
        const CacheChange_t* ch)
{
    return c_EntityId_RTPSParticipant == guid_from_change(ch).entityId;
}

bool DiscoveryDataBase::is_writer(
        const CacheChange_t* ch)
{
    if (is_participant(ch))
    {
        return false;
    }
    constexpr uint8_t entity_id_is_writer_bit = 0x03;
    return ((guid_from_change(ch).entityId.value[3] & entity_id_is_writer_bit) != 0);
}

bool DiscoveryDataBase::is_reader(
        const CacheChange_t* ch)
{
    if (is_participant(ch))
    {
        return false;
    }
    constexpr uint8_t entity_id_is_reader_bit = 0x04;
    return ((guid_from_change(ch).entityId.value[3] & entity_id_is_reader_bit) != 0);
}

GUID_t DiscoveryDataBase::guid_from_change(
        const CacheChange_t* ch)
{
    return iHandle2GUID(ch->instanceHandle);
}

CacheChange_t* DiscoveryDataBase::cache_change_own_participant()
{
    auto part_it = participants_.find(server_guid_prefix_);
    if (part_it != participants_.end())
    {
        return part_it->second.change();
    }
    return nullptr;
}

const std::vector<GuidPrefix_t> DiscoveryDataBase::remote_participants()
{
    std::vector<GuidPrefix_t> remote_participants;
    // Iterate over participants to add the remote ones
    for (auto participant: participants_)
    {
        // Only add participants other than the sever
        if (server_guid_prefix_ != participant.first)
        {
            remote_participants.push_back(participant.first);
        }
    }
    return remote_participants;
}

DiscoveryDataBase::AckedFunctor DiscoveryDataBase::functor(
        CacheChange_t* change)
{
    return DiscoveryDataBase::AckedFunctor(this, change);
}

DiscoveryDataBase::AckedFunctor::AckedFunctor(
        DiscoveryDataBase* db,
        CacheChange_t* change)
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
        const ReaderProxy* reader_proxy)
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
