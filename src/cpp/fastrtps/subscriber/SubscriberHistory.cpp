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
#include "SubscriberImpl.h"

#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/reader/WriterProxy.h>

#include <fastrtps/topic/TopicDataType.h>
#include <fastrtps/log/Log.h>

#include <mutex>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

inline bool sort_ReaderHistoryCache(CacheChange_t*c1,CacheChange_t*c2)
{
    return c1->sequenceNumber < c2->sequenceNumber;
}

SubscriberHistory::SubscriberHistory(
        SubscriberAttributes& satt,
        TopicDataType* type,
        const ReaderQos& qos,
        uint32_t payloadMaxSize,
        const HistoryQosPolicy& history,
        const ResourceLimitsQosPolicy& resource,
        MemoryManagementPolicy_t mempolicy)
    : ReaderHistory(HistoryAttributes(mempolicy, payloadMaxSize,
                history.kind == KEEP_ALL_HISTORY_QOS ?
                        resource.allocated_samples :
                        type->m_isGetKeyDefined ?
                            std::min(resource.allocated_samples, history.depth) :
                            std::min(resource.allocated_samples, history.depth * resource.max_instances),
                history.kind == KEEP_ALL_HISTORY_QOS ?
                        resource.max_samples :
                        type->m_isGetKeyDefined ?
                            history.depth :
                            history.depth * resource.max_instances))
    , m_unreadCacheCount(0)
    , m_historyQos(history)
    , m_resourceLimitsQos(resource)
    , sub_att_(satt)
    , type_(type)
    , qos_(qos)
    , mp_getKeyObject(nullptr)
{
    if (type_->m_isGetKeyDefined)
    {
        mp_getKeyObject = type_->createData();
    }
}

SubscriberHistory::~SubscriberHistory()
{
    if (type_->m_isGetKeyDefined)
    {
        type_->deleteData(mp_getKeyObject);
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

    std::lock_guard<std::recursive_timed_mutex> guard(*mp_mutex);

    //NO KEY HISTORY
    if (!type_->m_isGetKeyDefined)
    {
        bool add = false;
        if (m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
        {
            // TODO(Ricardo) Check
            if (m_changes.size() + unknown_missing_changes_up_to < (size_t)m_resourceLimitsQos.max_samples)
            {
                add = true;
            }
        }
        else if (m_historyQos.kind == KEEP_LAST_HISTORY_QOS)
        {
            if (m_changes.size() < (size_t)m_historyQos.depth)
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
                    bool read = older->isRead;

                    if (this->remove_change_sub(older))
                    {
                        if (!read)
                        {
                            this->decreaseUnreadCount();
                        }
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
                logWarning(SUBSCRIBER, "Attempting to add Data to Full ReaderHistory: " << sub_att_.getEntityID());
                return false;
            }

            if (this->add_change(a_change))
            {
                increaseUnreadCount();
                if ((int32_t)m_changes.size() == m_resourceLimitsQos.max_samples)
                    m_isHistoryFull = true;
                logInfo(SUBSCRIBER, sub_att_.getEntityID()
                    << ": Change " << a_change->sequenceNumber << " added from: "
                    << a_change->writerGUID;);

                return true;
            }
        }
    }
    //HISTORY WITH KEY
    else if (type_->m_isGetKeyDefined)
    {
        if (!a_change->instanceHandle.isDefined())
        {
            logInfo(RTPS_HISTORY, "Getting Key of change with no Key transmitted")
                type_->deserialize(&a_change->serializedPayload, mp_getKeyObject);
            bool is_key_protected = false;
#if HAVE_SECURITY
            is_key_protected = mp_reader->getAttributes().security_attributes().is_key_protected;
#endif
            if(!type_->getKey(mp_getKeyObject, &a_change->instanceHandle, is_key_protected))
                return false;

        }
        else if (!a_change->instanceHandle.isDefined())
        {
            logWarning(RTPS_HISTORY, "NO KEY in type: " << type_->getName()
                << " and no method to obtain it";);
            return false;
        }
        t_m_Inst_Caches::iterator vit;
        if (find_key(a_change, &vit))
        {
            bool add = false;
            if (m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
            {
                if ((int32_t)vit->second.cache_changes.size() < m_resourceLimitsQos.max_samples_per_instance)
                {
                    add = true;
                }
                else
                {
                    logWarning(SUBSCRIBER, "Change not added due to maximum number of samples per instance";);
                    return false;
                }
            }
            else if (m_historyQos.kind == KEEP_LAST_HISTORY_QOS)
            {
                if (vit->second.cache_changes.size() < (size_t)m_historyQos.depth)
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
                                older_sample = it;
                            // Already received
                            else if ((*it)->sequenceNumber == a_change->sequenceNumber)
                                return false;
                        }
                    }

                    if (older_sample != vit->second.cache_changes.rend())
                    {
                        bool read = (*older_sample)->isRead;

                        if (this->remove_change_sub(*older_sample))
                        {
                            if (!read)
                            {
                                this->decreaseUnreadCount();
                            }
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
                    logWarning(SUBSCRIBER, "Attempting to add Data to Full ReaderHistory: " << sub_att_.getEntityID());
                    return false;
                }

                if (this->add_change(a_change))
                {
                    increaseUnreadCount();
                    if ((int32_t)m_changes.size() == m_resourceLimitsQos.max_samples)
                        m_isHistoryFull = true;
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

    std::lock_guard<std::recursive_timed_mutex> guard(*mp_mutex);
    CacheChange_t* change;
    WriterProxy * wp;
    if (this->mp_reader->nextUnreadCache(&change, &wp))
    {
        change->isRead = true;
        this->decreaseUnreadCount();
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
                info->ownershipStrength = wp->m_att.ownershipStrength;
            }

            if (type_->m_isGetKeyDefined &&
                change->instanceHandle == c_InstanceHandle_Unknown && change->kind == ALIVE)
            {
                type_->getKey(data, &change->instanceHandle);
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

    std::lock_guard<std::recursive_timed_mutex> guard(*mp_mutex);
    CacheChange_t* change;
    WriterProxy * wp;
    if (this->mp_reader->nextUntakenCache(&change, &wp))
    {
        if (!change->isRead)
        {
            this->decreaseUnreadCount();
        }
        change->isRead = true;
        logInfo(SUBSCRIBER, this->mp_reader->getGuid().entityId << ": taking seqNum" << change->sequenceNumber <<
            " from writer: " << change->writerGUID);
        if (change->kind == ALIVE)
        {
            type_->deserialize(&change->serializedPayload, data);
        }
        if (info != nullptr)
        {
            info->sampleKind = change->kind;
            info->sample_identity.writer_guid(change->writerGUID);
            info->sample_identity.sequence_number(change->sequenceNumber);
            info->sourceTimestamp = change->sourceTimestamp;
            if (qos_.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
            {
                info->ownershipStrength = wp->m_att.ownershipStrength;
            }
            if (type_->m_isGetKeyDefined &&
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

bool SubscriberHistory::readNextData(void* data, SampleInfo_t* info)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Reader with this History before using it");
        return false;
    }

    std::lock_guard<std::recursive_timed_mutex> guard(*mp_mutex);
    CacheChange_t* change;
    WriterProxy * wp;
    if (this->mp_reader->nextUnreadCache(&change, &wp))
    {
        change->isRead = true;
        this->decreaseUnreadCount();
        logInfo(SUBSCRIBER, this->mp_reader->getGuid().entityId << ": reading " << change->sequenceNumber);
        if (change->kind == ALIVE)
        {
            type_->deserialize(&change->serializedPayload, data);
        }
        if (info != nullptr)
        {
            info->sampleKind = change->kind;
            info->sample_identity.writer_guid(change->writerGUID);
            info->sample_identity.sequence_number(change->sequenceNumber);
            info->sourceTimestamp = change->sourceTimestamp;
            if (qos_.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
            {
                info->ownershipStrength = wp->m_att.ownershipStrength;
            }
            if (type_->m_isGetKeyDefined &&
                change->instanceHandle == c_InstanceHandle_Unknown && change->kind == ALIVE)
            {
                bool is_key_protected = false;
#if HAVE_SECURITY
                is_key_protected = mp_reader->getAttributes().security_attributes().is_key_protected;
#endif
                type_->getKey(data, &change->instanceHandle, is_key_protected);
            }
            info->iHandle = change->instanceHandle;
            info->related_sample_identity = change->write_params.sample_identity();
        }
        return true;
    }
    return false;
}


bool SubscriberHistory::takeNextData(void* data, SampleInfo_t* info)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Reader with this History before using it");
        return false;
    }

    std::lock_guard<std::recursive_timed_mutex> guard(*mp_mutex);
    CacheChange_t* change;
    WriterProxy * wp;
    if (this->mp_reader->nextUntakenCache(&change, &wp))
    {
        if (!change->isRead)
        {
            this->decreaseUnreadCount();
        }
        change->isRead = true;
        logInfo(SUBSCRIBER, this->mp_reader->getGuid().entityId << ": taking seqNum" << change->sequenceNumber <<
            " from writer: " << change->writerGUID);
        if (change->kind == ALIVE)
        {
            type_->deserialize(&change->serializedPayload, data);
        }
        if (info != nullptr)
        {
            info->sampleKind = change->kind;
            info->sample_identity.writer_guid(change->writerGUID);
            info->sample_identity.sequence_number(change->sequenceNumber);
            info->sourceTimestamp = change->sourceTimestamp;
            if (qos_.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
            {
                info->ownershipStrength = wp->m_att.ownershipStrength;
            }
            if (type_->m_isGetKeyDefined &&
                change->instanceHandle == c_InstanceHandle_Unknown && change->kind == ALIVE)
            {
                bool is_key_protected = false;
#if HAVE_SECURITY
                is_key_protected = mp_reader->getAttributes().security_attributes().is_key_protected;
#endif
                type_->getKey(data, &change->instanceHandle, is_key_protected);
            }
            info->iHandle = change->instanceHandle;
            info->related_sample_identity = change->write_params.sample_identity();
        }
        this->remove_change_sub(change);
        return true;
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

    if ((int)keyed_changes_.size() < m_resourceLimitsQos.max_instances)
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

    std::lock_guard<std::recursive_timed_mutex> guard(*mp_mutex);
    if (!type_->m_isGetKeyDefined)
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
    std::lock_guard<std::recursive_timed_mutex> guard(*mp_mutex);

    if (!type_->m_isGetKeyDefined)
    {
        next_deadline_us_ = next_deadline_us;
        return true;
    }
    else if (type_->m_isGetKeyDefined)
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
    std::lock_guard<std::recursive_timed_mutex> guard(*mp_mutex);

    if (!type_->m_isGetKeyDefined)
    {
        next_deadline_us = next_deadline_us_;
        return true;
    }
    else if (type_->m_isGetKeyDefined)
    {
        auto min = std::min_element(keyed_changes_.begin(),
                                    keyed_changes_.end(),
                                    [](
                                    const std::pair<InstanceHandle_t, KeyedChanges> &lhs,
                                    const std::pair<InstanceHandle_t, KeyedChanges> &rhs)
        { return lhs.second.next_deadline_us < rhs.second.next_deadline_us; });
        handle = min->first;
        next_deadline_us = min->second.next_deadline_us;
        return true;
    }

    return false;
}
