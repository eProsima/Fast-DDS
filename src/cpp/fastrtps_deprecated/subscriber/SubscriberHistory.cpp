// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SubscriberHistory.cpp
 *
 */

#include <fastrtps/subscriber/SubscriberHistory.h>
#include <fastrtps_deprecated/subscriber/SubscriberImpl.h>

#include <fastdds/rtps/reader/RTPSReader.h>
#include <rtps/reader/WriterProxy.h>

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastrtps/log/Log.h>

#include <mutex>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

using eprosima::fastdds::dds::TopicDataType;

inline bool sort_ReaderHistoryCache(CacheChange_t* c1, CacheChange_t* c2)
{
    return c1->sequenceNumber < c2->sequenceNumber;
}

SubscriberHistory::SubscriberHistory(
        const TopicAttributes& topic_att,
        TopicDataType* type,
        const ReaderQos& qos,
        uint32_t payloadMaxSize,
        MemoryManagementPolicy_t mempolicy)
    : ReaderHistory(HistoryAttributes(mempolicy, payloadMaxSize,
                topic_att.historyQos.kind == KEEP_ALL_HISTORY_QOS ?
                        topic_att.resourceLimitsQos.allocated_samples :
                        topic_att.getTopicKind() == NO_KEY ?
                            std::min(topic_att.resourceLimitsQos.allocated_samples, topic_att.historyQos.depth) :
                            std::min(topic_att.resourceLimitsQos.allocated_samples, topic_att.historyQos.depth
                                     * topic_att.resourceLimitsQos.max_instances),
                topic_att.historyQos.kind == KEEP_ALL_HISTORY_QOS ?
                        topic_att.resourceLimitsQos.max_samples :
                        topic_att.getTopicKind() == NO_KEY ?
                            topic_att.historyQos.depth :
                            topic_att.historyQos.depth * topic_att.resourceLimitsQos.max_instances))
    , history_qos_(topic_att.historyQos)
    , resource_limited_qos_(topic_att.resourceLimitsQos)
    , topic_att_(topic_att)
    , type_(type)
    , qos_(qos)
    , get_key_object_(nullptr)
{
    if (type_->m_isGetKeyDefined)
    {
        get_key_object_ = type_->createData();
    }
}

SubscriberHistory::~SubscriberHistory()
{
    if (type_->m_isGetKeyDefined)
    {
        type_->deleteData(get_key_object_);
    }
}

bool SubscriberHistory::received_change(
        CacheChange_t* a_change,
        size_t unknown_missing_changes_up_to)
{

    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Reader with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);

    //NO KEY HISTORY
    if (topic_att_.getTopicKind() == NO_KEY)
    {
        bool add = false;
        if (history_qos_.kind == KEEP_ALL_HISTORY_QOS)
        {
            // TODO(Ricardo) Check
            if (m_changes.size() + unknown_missing_changes_up_to <
                    static_cast<size_t>(resource_limited_qos_.max_samples))
            {
                add = true;
            }
        }
        else if (history_qos_.kind == KEEP_LAST_HISTORY_QOS)
        {
            if (m_changes.size() < static_cast<size_t>(history_qos_.depth))
            {
                add = true;
            }
            else
            {
                // TODO (Ricardo) Older samples should be selected by sourcetimestamp.

                // Try to substitute a older samples.
                CacheChange_t* older = nullptr;

                for (auto it = m_changes.begin(); it != m_changes.end(); ++it)
                {
                    if ((*it)->writerGUID == a_change->writerGUID &&
                        (*it)->sequenceNumber < a_change->sequenceNumber)
                    {
                        older = *it;
                        break;
                    }
                }

                if (older != nullptr)
                {
                    if (this->remove_change_sub(older))
                    {
                        add = true;
                    }
                }
            }
        }

        if (add)
        {
            if (m_isHistoryFull)
            {
                // Discarting the sample.
                logWarning(SUBSCRIBER, "Attempting to add Data to Full ReaderHistory: " << topic_att_.getTopicDataType());
                return false;
            }

            if (this->add_change(a_change))
            {
                if (static_cast<int32_t>(m_changes.size()) == resource_limited_qos_.max_samples)
                    m_isHistoryFull = true;
                logInfo(SUBSCRIBER, topic_att_.getTopicDataType()
                    << ": Change " << a_change->sequenceNumber << " added from: "
                    << a_change->writerGUID;);

                return true;
            }
        }
    }
    //HISTORY WITH KEY
    else if (topic_att_.getTopicKind() == WITH_KEY)
    {
        if (!a_change->instanceHandle.isDefined() && type_ != nullptr)
        {
            logInfo(RTPS_HISTORY, "Getting Key of change with no Key transmitted")
                type_->deserialize(&a_change->serializedPayload, get_key_object_);
            bool is_key_protected = false;
#if HAVE_SECURITY
            is_key_protected = mp_reader->getAttributes().security_attributes().is_key_protected;
#endif
            if(!type_->getKey(get_key_object_, &a_change->instanceHandle, is_key_protected))
            {
                return false;
            }
        }
        else if (!a_change->instanceHandle.isDefined())
        {
            logWarning(RTPS_HISTORY, "NO KEY in topic: " << this->topic_att_.topicName
                << " and no method to obtain it";);
            return false;
        }
        t_m_Inst_Caches::iterator vit;
        if (find_key(a_change, &vit))
        {
            bool add = false;
            if (history_qos_.kind == KEEP_ALL_HISTORY_QOS)
            {
                if (static_cast<int32_t>(vit->second.cache_changes.size())
                        < resource_limited_qos_.max_samples_per_instance)
                {
                    add = true;
                }
                else
                {
                    logWarning(SUBSCRIBER, "Change not added due to maximum number of samples per instance";);
                    return false;
                }
            }
            else if (history_qos_.kind == KEEP_LAST_HISTORY_QOS)
            {
                if (vit->second.cache_changes.size() < static_cast<size_t>(history_qos_.depth))
                {
                    add = true;
                }
                else
                {
                    // Try to substitute the oldest sample with the same key
                    auto older_sample = vit->second.cache_changes.rend();
                    for (auto it = vit->second.cache_changes.rbegin(); it != vit->second.cache_changes.rend(); ++it)
                    {

                        if ((*it)->writerGUID == a_change->writerGUID)
                        {
                            if ((*it)->sequenceNumber < a_change->sequenceNumber)
                            {
                                older_sample = it;
                            }
                            // Already received
                            else if ((*it)->sequenceNumber == a_change->sequenceNumber)
                            {
                                return false;
                            }
                        }
                    }

                    if (older_sample != vit->second.cache_changes.rend())
                    {
                        if (this->remove_change_sub(*older_sample))
                        {
                            add = true;
                        }
                    }
                }
            }

            if (add)
            {
                if (m_isHistoryFull)
                {
                    // Discarting the sample.
                    logWarning(SUBSCRIBER, "Attempting to add Data to Full ReaderHistory: " << topic_att_.getTopicDataType());
                    return false;
                }

                if (this->add_change(a_change))
                {
                    if (static_cast<int32_t>(m_changes.size()) == resource_limited_qos_.max_samples)
                    {
                        m_isHistoryFull = true;
                    }
                    //ADD TO KEY VECTOR
                    if (vit->second.cache_changes.size() == 0)
                    {
                        vit->second.cache_changes.push_back(a_change);
                    }
                    else if (vit->second.cache_changes.back()->sequenceNumber < a_change->sequenceNumber)
                    {
                        vit->second.cache_changes.push_back(a_change);
                    }
                    else
                    {
                        vit->second.cache_changes.push_back(a_change);
                        std::sort(vit->second.cache_changes.begin(),
                                  vit->second.cache_changes.end(),
                                  sort_ReaderHistoryCache);
                    }

                    logInfo(SUBSCRIBER, this->mp_reader->getGuid().entityId
                        << ": Change " << a_change->sequenceNumber << " added from: "
                        << a_change->writerGUID << " with KEY: " << a_change->instanceHandle;);

                    return true;
                }
            }
        }
    }

    return false;
}

bool SubscriberHistory::readNextBuffer(SerializedPayload_t* data, SampleInfo_t* info)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Reader with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    CacheChange_t* change;
    WriterProxy * wp;
    if (this->mp_reader->nextUnreadCache(&change, &wp))
    {
        logInfo(SUBSCRIBER, this->mp_reader->getGuid().entityId << ": reading " << change->sequenceNumber);
        if (change->kind == ALIVE)
        {
            data->reserve(change->serializedPayload.length);
            change->serializedPayload.copy(data);
        }
        if (info != nullptr)
        {
            info->sampleKind = change->kind;
            info->sample_identity.writer_guid(change->writerGUID);
            info->sample_identity.sequence_number(change->sequenceNumber);
            info->sourceTimestamp = change->sourceTimestamp;
            if (qos_.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
            {
                info->ownershipStrength = wp->ownership_strength();
            }
            if (this->topic_att_.topicKind == WITH_KEY &&
                change->instanceHandle == c_InstanceHandle_Unknown && change->kind == ALIVE)
            {
                this->type_->getKey(data, &change->instanceHandle);
            }
            info->iHandle = change->instanceHandle;
            info->related_sample_identity = change->write_params.sample_identity();
        }
        return true;
    }
    return false;
}

bool SubscriberHistory::takeNextBuffer(SerializedPayload_t* data, SampleInfo_t* info)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Reader with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    CacheChange_t* change;
    WriterProxy * wp;
    if (this->mp_reader->nextUntakenCache(&change, &wp))
    {
        logInfo(SUBSCRIBER, this->mp_reader->getGuid().entityId << ": taking seqNum" << change->sequenceNumber <<
            " from writer: " << change->writerGUID);
        if (change->kind == ALIVE)
        {
            this->type_->deserialize(&change->serializedPayload, data);
        }
        if (info != nullptr)
        {
            info->sampleKind = change->kind;
            info->sample_identity.writer_guid(change->writerGUID);
            info->sample_identity.sequence_number(change->sequenceNumber);
            info->sourceTimestamp = change->sourceTimestamp;
            if (qos_.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
            {
                info->ownershipStrength = wp->ownership_strength();
            }
            if (this->topic_att_.topicKind == WITH_KEY &&
                change->instanceHandle == c_InstanceHandle_Unknown && change->kind == ALIVE)
            {
                data->reserve(change->serializedPayload.length);
                change->serializedPayload.copy(data);
            }
            info->iHandle = change->instanceHandle;
            info->related_sample_identity = change->write_params.sample_identity();
        }
        this->remove_change_sub(change);
        return true;
    }

    return false;
}

bool SubscriberHistory::readNextData(
        void* data,
        SampleInfo_t* info,
        std::chrono::steady_clock::time_point& max_blocking_time)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Reader with this History before using it");
        return false;
    }

    std::unique_lock<RecursiveTimedMutex> lock(*mp_mutex, std::defer_lock);

    if(lock.try_lock_until(max_blocking_time))
    {
        CacheChange_t* change;
        WriterProxy * wp;
        if (this->mp_reader->nextUnreadCache(&change, &wp))
        {
            logInfo(SUBSCRIBER, this->mp_reader->getGuid().entityId << ": reading " << change->sequenceNumber);
            if (change->kind == ALIVE)
            {
                this->type_->deserialize(&change->serializedPayload, data);
            }
            if (info != nullptr)
            {
                info->sampleKind = change->kind;
                info->sample_identity.writer_guid(change->writerGUID);
                info->sample_identity.sequence_number(change->sequenceNumber);
                info->sourceTimestamp = change->sourceTimestamp;
                if (qos_.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
                {
                    info->ownershipStrength = wp->ownership_strength();
                }
                if (this->topic_att_.topicKind == WITH_KEY &&
                    change->instanceHandle == c_InstanceHandle_Unknown && change->kind == ALIVE)
                {
                    bool is_key_protected = false;
#if HAVE_SECURITY
                    is_key_protected = mp_reader->getAttributes().security_attributes().is_key_protected;
#endif
                    this->type_->getKey(data, &change->instanceHandle, is_key_protected);
                }
                info->iHandle = change->instanceHandle;
                info->related_sample_identity = change->write_params.sample_identity();
            }
            return true;
        }
    }
    return false;
}


bool SubscriberHistory::takeNextData(
        void* data,
        SampleInfo_t* info,
        std::chrono::steady_clock::time_point& max_blocking_time)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Reader with this History before using it");
        return false;
    }

    std::unique_lock<RecursiveTimedMutex> lock(*mp_mutex, std::defer_lock);

    if(lock.try_lock_until(max_blocking_time))
    {
        CacheChange_t* change;
        WriterProxy * wp;
        if (this->mp_reader->nextUntakenCache(&change, &wp))
        {
            logInfo(SUBSCRIBER, this->mp_reader->getGuid().entityId << ": taking seqNum" << change->sequenceNumber <<
                    " from writer: " << change->writerGUID);
            if (change->kind == ALIVE)
            {
                this->type_->deserialize(&change->serializedPayload, data);
            }
            if (info != nullptr)
            {
                info->sampleKind = change->kind;
                info->sample_identity.writer_guid(change->writerGUID);
                info->sample_identity.sequence_number(change->sequenceNumber);
                info->sourceTimestamp = change->sourceTimestamp;
                if (qos_.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
                {
                    info->ownershipStrength = wp->ownership_strength();
                }
                if (this->topic_att_.topicKind == WITH_KEY &&
                        change->instanceHandle == c_InstanceHandle_Unknown && change->kind == ALIVE)
                {
                    bool is_key_protected = false;
#if HAVE_SECURITY
                    is_key_protected = mp_reader->getAttributes().security_attributes().is_key_protected;
#endif
                    this->type_->getKey(data, &change->instanceHandle, is_key_protected);
                }
                info->iHandle = change->instanceHandle;
                info->related_sample_identity = change->write_params.sample_identity();
            }
            this->remove_change_sub(change);
            return true;
        }
    }

    return false;
}

bool SubscriberHistory::find_key(
        CacheChange_t* a_change,
        t_m_Inst_Caches::iterator* vit_out)
{
    t_m_Inst_Caches::iterator vit;
    vit = keyed_changes_.find(a_change->instanceHandle);
    if (vit != keyed_changes_.end())
    {
        *vit_out = vit;
        return true;
    }

    if (static_cast<int>(keyed_changes_.size()) < resource_limited_qos_.max_instances)
    {
        *vit_out = keyed_changes_.insert(std::make_pair(a_change->instanceHandle, KeyedChanges())).first;
        return true;
    }
    else
    {
        for (vit = keyed_changes_.begin(); vit!= keyed_changes_.end(); ++vit)
        {
            if (vit->second.cache_changes.size() == 0)
            {
                keyed_changes_.erase(vit);
                *vit_out = keyed_changes_.insert(std::make_pair(a_change->instanceHandle, KeyedChanges())).first;
                return true;
            }
        }
        logWarning(SUBSCRIBER, "History has reached the maximum number of instances");
    }
    return false;
}


bool SubscriberHistory::remove_change_sub(CacheChange_t* change)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Reader with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    if (topic_att_.getTopicKind() == NO_KEY)
    {
        if (this->remove_change(change))
        {
            m_isHistoryFull = false;
            return true;
        }
        return false;
    }
    else
    {
        t_m_Inst_Caches::iterator vit;
        if (!this->find_key(change, &vit))
        {
            return false;
        }

        for (auto chit = vit->second.cache_changes.begin(); chit != vit->second.cache_changes.end(); ++chit)
        {
            if ((*chit)->sequenceNumber == change->sequenceNumber && (*chit)->writerGUID == change->writerGUID)
            {
                if (remove_change(change))
                {
                    vit->second.cache_changes.erase(chit);
                    m_isHistoryFull = false;
                    return true;
                }
            }
        }
        logError(SUBSCRIBER, "Change not found, something is wrong");
    }
    return false;
}

bool SubscriberHistory::set_next_deadline(
        const InstanceHandle_t& handle,
        const std::chrono::steady_clock::time_point& next_deadline_us)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Reader with this History before using it");
        return false;
    }
    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);

    if (topic_att_.getTopicKind() == NO_KEY)
    {
        next_deadline_us_ = next_deadline_us;
        return true;
    }
    else if (topic_att_.getTopicKind() == WITH_KEY)
    {
        if (keyed_changes_.find(handle) == keyed_changes_.end())
        {
            return false;
        }

        keyed_changes_[handle].next_deadline_us = next_deadline_us;
        return true;
    }

    return false;
}

bool SubscriberHistory::get_next_deadline(
        InstanceHandle_t &handle,
        std::chrono::steady_clock::time_point &next_deadline_us)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Reader with this History before using it");
        return false;
    }
    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);

    if (topic_att_.getTopicKind() == NO_KEY)
    {
        next_deadline_us = next_deadline_us_;
        return true;
    }
    else if (topic_att_.getTopicKind() == WITH_KEY)
    {
        auto min = std::min_element(keyed_changes_.begin(),
                                    keyed_changes_.end(),
                                    [](
                                        const std::pair<InstanceHandle_t, KeyedChanges> &lhs,
                                        const std::pair<InstanceHandle_t, KeyedChanges> &rhs)
                                        {
                                            return lhs.second.next_deadline_us < rhs.second.next_deadline_us;
                                        });
        handle = min->first;
        next_deadline_us = min->second.next_deadline_us;
        return true;
    }

    return false;
}
