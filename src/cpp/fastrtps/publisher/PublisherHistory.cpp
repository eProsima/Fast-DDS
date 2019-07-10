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
#include <fastrtps/topic/TopicDataType.h>

#include "PublisherImpl.h"

#include <fastrtps/rtps/writer/RTPSWriter.h>

#include <fastrtps/log/Log.h>

#include <mutex>

extern eprosima::fastrtps::rtps::WriteParams WRITE_PARAM_DEFAULT;

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

PublisherHistory::PublisherHistory(
        TopicDataType* type,
        uint32_t payloadMaxSize,
        const HistoryQosPolicy& history,
        const ResourceLimitsQosPolicy& resource,
        MemoryManagementPolicy_t mempolicy)
    : WriterHistory(HistoryAttributes(mempolicy, payloadMaxSize,
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
    , m_historyQos(history)
    , m_resourceLimitsQos(resource)
    , type_(type)
{
    // TODO Auto-generated constructor stub
}

PublisherHistory::~PublisherHistory()
{
}


bool PublisherHistory::add_pub_change(
        CacheChange_t* change,
        WriteParams &wparams,
        std::unique_lock<std::recursive_timed_mutex>& lock,
        std::chrono::time_point<std::chrono::steady_clock> max_blocking_time)
{
    if(m_isHistoryFull)
    {
        bool ret = false;

        if(m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
        {
            ret = this->mp_writer->try_remove_change(max_blocking_time, lock);
        }
        else if(m_historyQos.kind == KEEP_LAST_HISTORY_QOS)
        {
            ret = this->remove_min_change();
        }

        if(!ret)
        {
            logWarning(RTPS_HISTORY,"Attempting to add Data to Full WriterCache: "<< type_->getName());
            return false;
        }
    }

    assert(!m_isHistoryFull);

    bool returnedValue = false;

    //NO KEY HISTORY
    if (!type_->m_isGetKeyDefined)
    {
        if(this->add_change_(change, wparams, max_blocking_time))
        {
            returnedValue = true;
        }
    }
    //HISTORY WITH KEY
    else if (type_->m_isGetKeyDefined)
    {
        t_m_Inst_Caches::iterator vit;
        if(find_key(change,&vit))
        {
            logInfo(RTPS_HISTORY,"Found key: "<< vit->first);
            bool add = false;
            if(m_historyQos.kind == KEEP_ALL_HISTORY_QOS)
            {
                if((int32_t)vit->second.cache_changes.size() < m_resourceLimitsQos.max_samples_per_instance)
                {
                    add = true;
                }
                else
                {
                    logWarning(RTPS_HISTORY,"Change not added due to maximum number of samples per instance"<<endl;);
                }
            }
            else if (m_historyQos.kind == KEEP_LAST_HISTORY_QOS)
            {
                if(vit->second.cache_changes.size() < (size_t)m_historyQos.depth)
                {
                    add = true;
                }
                else
                {
                    if(remove_change_pub(vit->second.cache_changes.front()))
                    {
                        add = true;
                    }
                }
            }

            if(add)
            {
                vit->second.cache_changes.push_back(change);
                if(this->add_change_(change, wparams, max_blocking_time))
                {
                    logInfo(RTPS_HISTORY, type_->getName() <<" Change "
                            << change->sequenceNumber << " added with key: "<<change->instanceHandle
                            << " and "<<change->serializedPayload.length<< " bytes");
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

    if ((int)keyed_changes_.size() < m_resourceLimitsQos.max_instances)
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
    std::lock_guard<std::recursive_timed_mutex> guard(*this->mp_mutex);

    while(m_changes.size()>0)
    {
        if(remove_change_pub(m_changes.front()))
            ++rem;
        else
            break;
    }
    if(removed!=nullptr)
    {
        *removed = rem;
    }
    if (rem>0)
    {
        return true;
    }
    return false;
}


bool PublisherHistory::removeMinChange()
{
    if(mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY,"You need to create a Writer with this History before using it");
        return false;
    }

    std::lock_guard<std::recursive_timed_mutex> guard(*this->mp_mutex);
    if(m_changes.size()>0)
        return remove_change_pub(m_changes.front());
    return false;
}

bool PublisherHistory::remove_change_pub(CacheChange_t* change)
{

    if(mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY,"You need to create a Writer with this History before using it");
        return false;
    }

    std::lock_guard<std::recursive_timed_mutex> guard(*this->mp_mutex);
    if (!type_->m_isGetKeyDefined)
    {
        if(this->remove_change(change))
        {
            m_isHistoryFull = false;
            return true;
        }

        return false;
    }
    else
    {
        t_m_Inst_Caches::iterator vit;
        if(!this->find_key(change,&vit))
        {
            return false;
        }

        for(auto chit = vit->second.cache_changes.begin(); chit!= vit->second.cache_changes.end(); ++chit)
        {
            if( ((*chit)->sequenceNumber == change->sequenceNumber) && ((*chit)->writerGUID == change->writerGUID) )
            {
                if(remove_change(change))
                {
                    vit->second.cache_changes.erase(chit);
                    m_isHistoryFull = false;
                    return true;
                }
            }
        }
        logError(PUBLISHER,"Change not found, something is wrong");
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
    if(mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY,"You need to create a Writer with this History before using it");
        return false;
    }
    std::lock_guard<std::recursive_timed_mutex> guard(*this->mp_mutex);

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

bool PublisherHistory::get_next_deadline(
        InstanceHandle_t &handle,
        std::chrono::steady_clock::time_point &next_deadline_us)
{
    if(mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY,"You need to create a Writer with this History before using it");
        return false;
    }
    std::lock_guard<std::recursive_timed_mutex> guard(*this->mp_mutex);

    if (!type_->m_isGetKeyDefined)
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
    else if (type_->m_isGetKeyDefined)
    {
        next_deadline_us = next_deadline_us_;
        return true;
    }

    return false;
}
