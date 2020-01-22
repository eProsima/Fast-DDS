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
 * @file PublisherHistory.cpp
 *
 */

#include <mutex>

#include <fastrtps/publisher/PublisherHistory.h>

#include <fastrtps_deprecated/publisher/PublisherImpl.h>

#include <fastdds/rtps/writer/RTPSWriter.h>

#include <fastrtps/log/Log.h>

#include <mutex>

extern eprosima::fastrtps::rtps::WriteParams WRITE_PARAM_DEFAULT;

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

PublisherHistory::PublisherHistory(
        const TopicAttributes& topic_att,
        uint32_t payloadMaxSize,
        MemoryManagementPolicy_t mempolicy)
    : WriterHistory(HistoryAttributes(mempolicy, payloadMaxSize,
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
{
}

PublisherHistory::~PublisherHistory()
{
}


bool PublisherHistory::add_pub_change(
        CacheChange_t* change,
        WriteParams &wparams,
        std::unique_lock<RecursiveTimedMutex>& lock,
        std::chrono::time_point<std::chrono::steady_clock> max_blocking_time)
{
    if (m_isHistoryFull)
    {
        bool ret = false;

        if (history_qos_.kind == KEEP_ALL_HISTORY_QOS)
        {
            ret = this->mp_writer->try_remove_change(max_blocking_time, lock);
        }
        else if (history_qos_.kind == KEEP_LAST_HISTORY_QOS)
        {
            ret = this->remove_min_change();
        }

        if (!ret)
        {
            logWarning(RTPS_HISTORY, "Attempting to add Data to Full WriterCache: " << topic_att_.getTopicDataType());
            return false;
        }
    }

    assert(!m_isHistoryFull);

    bool returnedValue = false;

    //NO KEY HISTORY
    if (topic_att_.getTopicKind() == NO_KEY)
    {
#if HAVE_STRICT_REALTIME
        if (this->add_change_(change, wparams, max_blocking_time))
#else
        if (this->add_change_(change, wparams))
#endif
        {
            returnedValue = true;
        }
    }
    //HISTORY WITH KEY
    else if (topic_att_.getTopicKind() == WITH_KEY)
    {
        t_m_Inst_Caches::iterator vit;
        if (find_key(change, &vit))
        {
            logInfo(RTPS_HISTORY, "Found key: " << vit->first);
            bool add = false;
            if (history_qos_.kind == KEEP_ALL_HISTORY_QOS)
            {
                if (static_cast<int32_t>(vit->second.cache_changes.size()) <
                        resource_limited_qos_.max_samples_per_instance)
                {
                    add = true;
                }
                else
                {
                    logWarning(RTPS_HISTORY,"Change not added due to maximum number of samples per instance"<<endl;);
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
                    if (remove_change_pub(vit->second.cache_changes.front()))
                    {
                        add = true;
                    }
                }
            }

            if (add)
            {
                vit->second.cache_changes.push_back(change);
#if HAVE_STRICT_REALTIME
                if (this->add_change_(change, wparams, max_blocking_time))
#else
                if (this->add_change_(change, wparams))
#endif
                {
                    logInfo(RTPS_HISTORY, topic_att_.getTopicDataType() << " Change "
                            << change->sequenceNumber << " added with key: " << change->instanceHandle
                            << " and " << change->serializedPayload.length << " bytes");
                    returnedValue =  true;
                }
            }
        }
    }


    return returnedValue;
}

bool PublisherHistory::find_key(
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
        for (vit = keyed_changes_.begin(); vit != keyed_changes_.end(); ++vit)
        {
            if (vit->second.cache_changes.size() == 0)
            {
                keyed_changes_.erase(vit);
                *vit_out = keyed_changes_.insert(std::make_pair(a_change->instanceHandle, KeyedChanges())).first;
                return true;
            }
        }
        logWarning(PUBLISHER, "History has reached the maximum number of instances" << endl;)
    }
    return false;
}


bool PublisherHistory::removeAllChange(size_t* removed)
{

    size_t rem = 0;
    std::lock_guard<RecursiveTimedMutex> guard(*this->mp_mutex);

    while (m_changes.size() > 0)
    {
        if (remove_change_pub(m_changes.front()))
        {
            ++rem;
        }
        else
        {
            break;
        }
    }
    if (removed != nullptr)
    {
        *removed = rem;
    }
    if (rem > 0)
    {
        return true;
    }
    return false;
}


bool PublisherHistory::removeMinChange()
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Writer with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*this->mp_mutex);
    if (m_changes.size() > 0)
    {
        return remove_change_pub(m_changes.front());
    }
    return false;
}

bool PublisherHistory::remove_change_pub(CacheChange_t* change)
{

    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Writer with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*this->mp_mutex);
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
        if (!this->find_key(change,&vit))
        {
            return false;
        }

        for(auto chit = vit->second.cache_changes.begin(); chit!= vit->second.cache_changes.end(); ++chit)
        {
            if (((*chit)->sequenceNumber == change->sequenceNumber) && ((*chit)->writerGUID == change->writerGUID))
            {
                if (remove_change(change))
                {
                    vit->second.cache_changes.erase(chit);
                    m_isHistoryFull = false;
                    return true;
                }
            }
        }
        logError(PUBLISHER, "Change not found, something is wrong");
    }
    return false;
}

bool PublisherHistory::remove_change_g(CacheChange_t* a_change)
{
    return remove_change_pub(a_change);
}

bool PublisherHistory::set_next_deadline(
        const InstanceHandle_t& handle,
        const std::chrono::steady_clock::time_point& next_deadline_us)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Writer with this History before using it");
        return false;
    }
    std::lock_guard<RecursiveTimedMutex> guard(*this->mp_mutex);

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

bool PublisherHistory::get_next_deadline(
        InstanceHandle_t &handle,
        std::chrono::steady_clock::time_point &next_deadline_us)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Writer with this History before using it");
        return false;
    }
    std::lock_guard<RecursiveTimedMutex> guard(*this->mp_mutex);

    if (topic_att_.getTopicKind() == WITH_KEY)
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
    else if (topic_att_.getTopicKind() == NO_KEY)
    {
        next_deadline_us = next_deadline_us_;
        return true;
    }

    return false;
}
