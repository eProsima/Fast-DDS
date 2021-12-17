// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima)
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
// See the License for the specific language governing permissiones and
// limitations under the License.

#ifndef PUBLISHERHISTORY_H_
#define PUBLISHERHISTORY_H_

#include <chrono>
#include <mutex>

#include <gmock/gmock.h>

#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/common/ChangeKind_t.hpp>
#include <fastdds/rtps/common/InstanceHandle.h>
#include <fastdds/rtps/common/SequenceNumber.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/resources/ResourceManagement.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/common/KeyedChanges.h>
#include <fastrtps/utils/TimedMutex.hpp>

namespace eprosima {
namespace fastrtps {

using namespace eprosima::fastrtps::rtps;

static HistoryAttributes to_history_attributes(
        const TopicAttributes& topic_att,
        uint32_t payloadMaxSize,
        MemoryManagementPolicy_t mempolicy)
{
    auto initial_samples = topic_att.resourceLimitsQos.allocated_samples;
    auto max_samples = topic_att.resourceLimitsQos.max_samples;
    auto extra_samples = topic_att.resourceLimitsQos.extra_samples;

    if (topic_att.historyQos.kind != KEEP_ALL_HISTORY_QOS)
    {
        max_samples = topic_att.historyQos.depth;
        if (topic_att.getTopicKind() != NO_KEY)
        {
            max_samples *= topic_att.resourceLimitsQos.max_instances;
        }

        initial_samples = std::min(initial_samples, max_samples);
    }

    return HistoryAttributes(mempolicy, payloadMaxSize, initial_samples, max_samples, extra_samples);
}

class PublisherHistory : public WriterHistory
{
public:

    PublisherHistory(
            const TopicAttributes& topic_att,
            uint32_t payloadMaxSize,
            MemoryManagementPolicy_t mempolicy)
        : WriterHistory(to_history_attributes(topic_att, payloadMaxSize, mempolicy))
        , history_qos_(topic_att.historyQos)
        , resource_limited_qos_(topic_att.resourceLimitsQos)
        , topic_att_(topic_att)
    {
        if (resource_limited_qos_.max_instances == 0)
        {
            resource_limited_qos_.max_instances = std::numeric_limits<int32_t>::max();
        }

        if (resource_limited_qos_.max_samples_per_instance == 0)
        {
            resource_limited_qos_.max_samples_per_instance = std::numeric_limits<int32_t>::max();
        }
    }

    PublisherHistory(
            const PublisherHistory&)
        : WriterHistory(HistoryAttributes())
    {
    }

    ~PublisherHistory() = default;

    MOCK_METHOD3(wait_for_acknowledgement_last_change, bool(
                const InstanceHandle_t&,
                std::unique_lock<RecursiveTimedMutex>& lock,
                const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time));

    void rebuild_instances()
    {
    }

    bool register_instance(
            const InstanceHandle_t& instance_handle,
            std::unique_lock<RecursiveTimedMutex>&,
            const std::chrono::time_point<std::chrono::steady_clock>&)
    {
        /// Preconditions
        if (topic_att_.getTopicKind() == NO_KEY)
        {
            return false;
        }

        t_m_Inst_Caches::iterator vit;
        return find_or_add_key(instance_handle, &vit);
    }

    bool is_key_registered(
            const InstanceHandle_t& handle)
    {
        if (mp_writer == nullptr || mp_mutex == nullptr)
        {
            logError(RTPS_HISTORY, "You need to create a Writer with this History before using it");
            return false;
        }
        std::lock_guard<RecursiveTimedMutex> guard(*this->mp_mutex);
        t_m_Inst_Caches::iterator vit;
        vit = keyed_changes_.find(handle);
        return (vit != keyed_changes_.end() &&
               (vit->second.cache_changes.empty() ||
               (NOT_ALIVE_UNREGISTERED != vit->second.cache_changes.back()->kind &&
               NOT_ALIVE_DISPOSED_UNREGISTERED != vit->second.cache_changes.back()->kind
               )
               )
               );
    }

    bool add_pub_change(
            CacheChange_t* change,
            WriteParams& wparams,
            std::unique_lock<RecursiveTimedMutex>&,
            const std::chrono::time_point<std::chrono::steady_clock>&)
    {
        if (m_isHistoryFull)
        {
            if (!this->remove_min_change())
            {
                return false;
            }
        }

        bool returnedValue = false;

        // For NO_KEY we can directly add the change
        bool add = (topic_att_.getTopicKind() == NO_KEY);
        if (topic_att_.getTopicKind() == WITH_KEY)
        {
            t_m_Inst_Caches::iterator vit;

            if (find_or_add_key(change->instanceHandle, &vit))
            {
                vit->second.cache_changes.push_back(change);
                add = true;
            }
        }

        if (add)
        {
            if (this->add_change_(change, wparams))
            {
                returnedValue = true;
            }
        }
        return returnedValue;
    }

    bool set_next_deadline(
            const InstanceHandle_t&,
            const std::chrono::steady_clock::time_point&)
    {
        return true;
    }

    bool removeMinChange()
    {
        return true;
    }

    bool removeAllChange(
            size_t*)
    {
        return true;
    }

    bool remove_instance_changes(
            const InstanceHandle_t&,
            const SequenceNumber_t&)
    {
        return true;
    }

    bool get_next_deadline(
            InstanceHandle_t&,
            std::chrono::steady_clock::time_point&)
    {
        return true;
    }

    bool remove_change_pub(
            CacheChange_t* change)
    {
        if (mp_writer == nullptr || mp_mutex == nullptr)
        {
            return false;
        }

        std::lock_guard<RecursiveTimedMutex> guard(*this->mp_mutex);
        if (topic_att_.getTopicKind() == NO_KEY)
        {
            if (remove_change(change))
            {
                m_isHistoryFull = false;
                return true;
            }

            return false;
        }
        else
        {
            t_m_Inst_Caches::iterator vit;
            if (!this->find_or_add_key(change->instanceHandle, &vit))
            {
                return false;
            }

            for (auto chit = vit->second.cache_changes.begin(); chit != vit->second.cache_changes.end(); ++chit)
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
        }
        return false;
    }

private:

    typedef std::map<InstanceHandle_t, KeyedChanges> t_m_Inst_Caches;

    //!Map where keys are instance handles and values are vectors of cache changes associated
    t_m_Inst_Caches keyed_changes_;
    //!HistoryQosPolicy values.
    HistoryQosPolicy history_qos_;
    //!ResourceLimitsQosPolicy values.
    ResourceLimitsQosPolicy resource_limited_qos_;
    //!Topic Attributes
    TopicAttributes topic_att_;

    bool find_or_add_key(
            const InstanceHandle_t& instance_handle,
            t_m_Inst_Caches::iterator* vit_out)
    {
        t_m_Inst_Caches::iterator vit;
        vit = keyed_changes_.find(instance_handle);
        if (vit != keyed_changes_.end())
        {
            *vit_out = vit;
        }
        else
        {
            *vit_out = keyed_changes_.insert(std::make_pair(instance_handle, KeyedChanges())).first;
        }
        return true;
    }

};

} // fastrtps
} // eprosima

#endif // PUBLISHERHISTORY_H_
