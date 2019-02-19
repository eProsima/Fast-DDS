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

#include <fastrtps/TopicDataType.h>
#include <fastrtps/log/Log.h>

#include <mutex>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

inline bool sort_ReaderHistoryCache(CacheChange_t*c1,CacheChange_t*c2)
{
    return c1->sequenceNumber < c2->sequenceNumber;
}

SubscriberHistory::SubscriberHistory(
        SubscriberImpl* simpl,
        uint32_t payloadMaxSize,
        const HistoryQosPolicy& history,
        const ResourceLimitsQosPolicy& resource,
        MemoryManagementPolicy_t mempolicy)
    : ReaderHistory(HistoryAttributes(mempolicy, payloadMaxSize,
                history.kind == KEEP_ALL_HISTORY_QOS ?
                        resource.allocated_samples :
                        simpl->getAttributes().topic.getTopicKind() == NO_KEY ?
                            std::min(resource.allocated_samples, history.depth) :
                            std::min(resource.allocated_samples, history.depth * resource.max_instances),
                history.kind == KEEP_ALL_HISTORY_QOS ?
                        resource.max_samples :
                        simpl->getAttributes().topic.getTopicKind() == NO_KEY ?
                            history.depth :
                            history.depth * resource.max_instances))
    , m_unreadCacheCount(0)
    , m_historyQos(history)
    , m_resourceLimitsQos(resource)
    , mp_subImpl(simpl)
    , mp_getKeyObject(nullptr)
{
    if (mp_subImpl->getType()->m_isGetKeyDefined)
    {
        mp_getKeyObject = mp_subImpl->getType()->createData();
    }
}

SubscriberHistory::~SubscriberHistory()
{
    if (mp_subImpl->getType()->m_isGetKeyDefined)
    {
        mp_subImpl->getType()->deleteData(mp_getKeyObject);
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

    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    //NO KEY HISTORY
    if (mp_subImpl->getAttributes().topic.getTopicKind() == NO_KEY)
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
                logWarning(SUBSCRIBER, "Attempting to add Data to Full ReaderHistory: " << this->mp_subImpl->getGuid().entityId);
                return false;
            }

            if (this->add_change(a_change))
            {
                increaseUnreadCount();
                if ((int32_t)m_changes.size() == m_resourceLimitsQos.max_samples)
                    m_isHistoryFull = true;
                logInfo(SUBSCRIBER, this->mp_subImpl->getGuid().entityId
                    << ": Change " << a_change->sequenceNumber << " added from: "
                    << a_change->writerGUID;);
                //print_changes_seqNum();
                return true;
            }
        }
    }
    //HISTORY WITH KEY
    else if (mp_subImpl->getAttributes().topic.getTopicKind() == WITH_KEY)
    {
        if (!a_change->instanceHandle.isDefined() && mp_subImpl->getType() != nullptr)
        {
            logInfo(RTPS_HISTORY, "Getting Key of change with no Key transmitted")
                mp_subImpl->getType()->deserialize(&a_change->serializedPayload, mp_getKeyObject);
            bool is_key_protected = false;
#if HAVE_SECURITY
            is_key_protected = mp_reader->getAttributes().security_attributes().is_key_protected;
#endif
            if(!mp_subImpl->getType()->getKey(mp_getKeyObject, &a_change->instanceHandle, is_key_protected))
                return false;

        }
        else if (!a_change->instanceHandle.isDefined())
        {
            logWarning(RTPS_HISTORY, "NO KEY in topic: " << this->mp_subImpl->getAttributes().topic.topicName
                << " and no method to obtain it";);
            return false;
        }
        t_v_Inst_Caches::iterator vit;
        if (find_Key(a_change, &vit))
        {
            //logInfo(RTPS_EDP,"Trying to add change with KEY: "<< vit->first << endl;);
            bool add = false;
            if (m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
            {
                if ((int32_t)vit->second.size() < m_resourceLimitsQos.max_samples_per_instance)
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
                if (vit->second.size() < (size_t)m_historyQos.depth)
                {
                    add = true;
                }
                else
                {
                    // Try to substitude a older samples.
                    auto older_sample = m_changes.rend();
                    for (auto it = m_changes.rbegin(); it != m_changes.rend(); ++it)
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

                    if (older_sample != m_changes.rend())
                    {
                        bool read = (*older_sample)->isRead;

                        if (this->remove_change_sub(*older_sample, &vit))
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
                    logWarning(SUBSCRIBER, "Attempting to add Data to Full ReaderHistory: " << this->mp_subImpl->getGuid().entityId);
                    return false;
                }

                if (this->add_change(a_change))
                {
                    increaseUnreadCount();
                    if ((int32_t)m_changes.size() == m_resourceLimitsQos.max_samples)
                        m_isHistoryFull = true;
                    //ADD TO KEY VECTOR
                    if (vit->second.size() == 0)
                    {
                        vit->second.push_back(a_change);
                    }
                    else if (vit->second.back()->sequenceNumber < a_change->sequenceNumber)
                    {
                        vit->second.push_back(a_change);
                    }
                    else
                    {
                        vit->second.push_back(a_change);
                        std::sort(vit->second.begin(), vit->second.end(), sort_ReaderHistoryCache);
                    }
                    logInfo(SUBSCRIBER, this->mp_reader->getGuid().entityId
                        << ": Change " << a_change->sequenceNumber << " added from: "
                        << a_change->writerGUID << " with KEY: " << a_change->instanceHandle;);
                    //	print_changes_seqNum();
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

    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
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
            if (this->mp_subImpl->getAttributes().qos.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
            {
                info->ownershipStrength = wp->ownership_strength();
            }
            if (this->mp_subImpl->getAttributes().topic.topicKind == WITH_KEY &&
                change->instanceHandle == c_InstanceHandle_Unknown && change->kind == ALIVE)
            {
                this->mp_subImpl->getType()->getKey(data, &change->instanceHandle);
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

    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
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
            this->mp_subImpl->getType()->deserialize(&change->serializedPayload, data);
        }
        if (info != nullptr)
        {
            info->sampleKind = change->kind;
            info->sample_identity.writer_guid(change->writerGUID);
            info->sample_identity.sequence_number(change->sequenceNumber);
            info->sourceTimestamp = change->sourceTimestamp;
            if (this->mp_subImpl->getAttributes().qos.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
            {
                info->ownershipStrength = wp->ownership_strength();
            }
            if (this->mp_subImpl->getAttributes().topic.topicKind == WITH_KEY &&
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

    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    CacheChange_t* change;
    WriterProxy * wp;
    if (this->mp_reader->nextUnreadCache(&change, &wp))
    {
        change->isRead = true;
        this->decreaseUnreadCount();
        logInfo(SUBSCRIBER, this->mp_reader->getGuid().entityId << ": reading " << change->sequenceNumber);
        if (change->kind == ALIVE)
        {
            this->mp_subImpl->getType()->deserialize(&change->serializedPayload, data);
        }
        if (info != nullptr)
        {
            info->sampleKind = change->kind;
            info->sample_identity.writer_guid(change->writerGUID);
            info->sample_identity.sequence_number(change->sequenceNumber);
            info->sourceTimestamp = change->sourceTimestamp;
            if (this->mp_subImpl->getAttributes().qos.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
            {
                info->ownershipStrength = wp->ownership_strength();
            }
            if (this->mp_subImpl->getAttributes().topic.topicKind == WITH_KEY &&
                change->instanceHandle == c_InstanceHandle_Unknown && change->kind == ALIVE)
            {
                bool is_key_protected = false;
#if HAVE_SECURITY
                is_key_protected = mp_reader->getAttributes().security_attributes().is_key_protected;
#endif
                this->mp_subImpl->getType()->getKey(data, &change->instanceHandle, is_key_protected);
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

    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
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
            this->mp_subImpl->getType()->deserialize(&change->serializedPayload, data);
        }
        if (info != nullptr)
        {
            info->sampleKind = change->kind;
            info->sample_identity.writer_guid(change->writerGUID);
            info->sample_identity.sequence_number(change->sequenceNumber);
            info->sourceTimestamp = change->sourceTimestamp;
            if (this->mp_subImpl->getAttributes().qos.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
            {
                info->ownershipStrength = wp->ownership_strength();
            }
            if (this->mp_subImpl->getAttributes().topic.topicKind == WITH_KEY &&
                change->instanceHandle == c_InstanceHandle_Unknown && change->kind == ALIVE)
            {
                bool is_key_protected = false;
#if HAVE_SECURITY
                is_key_protected = mp_reader->getAttributes().security_attributes().is_key_protected;
#endif
                this->mp_subImpl->getType()->getKey(data, &change->instanceHandle, is_key_protected);
            }
            info->iHandle = change->instanceHandle;
            info->related_sample_identity = change->write_params.sample_identity();
        }
        this->remove_change_sub(change);
        return true;
    }

    return false;
}

bool SubscriberHistory::find_Key(CacheChange_t* a_change, t_v_Inst_Caches::iterator* vit_out)
{
    t_v_Inst_Caches::iterator vit;
    bool found = false;
    for (vit = m_keyedChanges.begin(); vit != m_keyedChanges.end(); ++vit)
    {
        if (a_change->instanceHandle == vit->first)
        {
            *vit_out = vit;
            return true;
        }
    }
    if (!found)
    {
        if ((int)m_keyedChanges.size() < m_resourceLimitsQos.max_instances)
        {
            t_p_I_Change newpair;
            newpair.first = a_change->instanceHandle;
            m_keyedChanges.push_back(newpair);
            *vit_out = m_keyedChanges.end() - 1;
            return true;
        }
        else
        {
            for (vit = m_keyedChanges.begin(); vit != m_keyedChanges.end(); ++vit)
            {
                if (vit->second.size() == 0)
                {
                    m_keyedChanges.erase(vit);
                    t_p_I_Change newpair;
                    newpair.first = a_change->instanceHandle;
                    m_keyedChanges.push_back(newpair);
                    *vit_out = m_keyedChanges.end() - 1;
                    return true;
                }
            }
            logWarning(SUBSCRIBER, "History has reached the maximum number of instances");
        }

    }
    return false;
}


bool SubscriberHistory::remove_change_sub(CacheChange_t* change, t_v_Inst_Caches::iterator* vit_in)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Reader with this History before using it");
        return false;
    }

    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    if (mp_subImpl->getAttributes().topic.getTopicKind() == NO_KEY)
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
        t_v_Inst_Caches::iterator vit;
        if (vit_in != nullptr)
        {
            vit = *vit_in;
        }
        else if (this->find_Key(change, &vit))
        {

        }
        else
        {
            return false;
        }
        for (auto chit = vit->second.begin(); chit != vit->second.end(); ++chit)
        {
            if ((*chit)->sequenceNumber == change->sequenceNumber && (*chit)->writerGUID == change->writerGUID)
            {
                if (remove_change(change))
                {
                    vit->second.erase(chit);
                    m_isHistoryFull = false;
                    return true;
                }
            }
        }
        logError(SUBSCRIBER, "Change not found, something is wrong");
    }
    return false;
}
