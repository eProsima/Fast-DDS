// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of DataWriterHistory: replaces src/cpp/fastdds/publisher/DataWriterHistory.cpp.
// wait_for_acknowledgement_last_change() forwards to the DataWriterHistoryMock companion of the instance. The rest is a
// simplified history: deadlines and removals of instances always succeed and inline QoS adds no overhead.

#include <fastdds/publisher/DataWriterHistory.hpp>

#include <algorithm>
#include <chrono>
#include <mutex>

#include <fastdds/dds/log/Log.hpp>

#include <mock/fastdds/publisher/DataWriterHistoryMock.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

using namespace eprosima::fastdds::rtps;

using Companion = mock::MockCompanion<DataWriterHistory>;

HistoryAttributes DataWriterHistory::to_history_attributes(
        const HistoryQosPolicy& history_qos,
        const ResourceLimitsQosPolicy& resource_limits_qos,
        const TopicKind_t& topic_kind,
        uint32_t payloadMaxSize,
        MemoryManagementPolicy_t mempolicy)
{
    auto initial_samples = resource_limits_qos.allocated_samples;
    auto max_samples = resource_limits_qos.max_samples;
    auto extra_samples = resource_limits_qos.extra_samples;

    if (history_qos.kind != KEEP_ALL_HISTORY_QOS)
    {
        max_samples = history_qos.depth;
        if (topic_kind != NO_KEY)
        {
            if (0 < resource_limits_qos.max_instances)
            {
                max_samples *= resource_limits_qos.max_instances;
            }
            else
            {
                max_samples = -1;
            }
        }

        initial_samples = std::min(initial_samples, max_samples);
    }

    return HistoryAttributes(mempolicy, payloadMaxSize, initial_samples, max_samples, extra_samples);
}

DataWriterHistory::DataWriterHistory(
        const std::shared_ptr<IPayloadPool>& payload_pool,
        const std::shared_ptr<IChangePool>& change_pool,
        const HistoryQosPolicy& history_qos,
        const ResourceLimitsQosPolicy& resource_limits_qos,
        const TopicKind_t& topic_kind,
        uint32_t payloadMaxSize,
        MemoryManagementPolicy_t mempolicy,
        std::function<void (const InstanceHandle_t&)> unack_sample_remove_functor)
    : WriterHistory(to_history_attributes(history_qos, resource_limits_qos, topic_kind, payloadMaxSize,
            mempolicy), payload_pool, change_pool)
    , history_qos_(history_qos)
    , resource_limited_qos_(resource_limits_qos)
    , topic_kind_(topic_kind)
    , unacknowledged_sample_removed_functor_(unack_sample_remove_functor)
{
    if (resource_limited_qos_.max_samples <= 0)
    {
        resource_limited_qos_.max_samples = -1;
    }

    if (resource_limited_qos_.max_instances <= 0)
    {
        resource_limited_qos_.max_instances = -1;
    }

    if (resource_limited_qos_.max_samples_per_instance <= 0)
    {
        resource_limited_qos_.max_samples_per_instance = -1;
    }

    Companion::attach(this);
}

DataWriterHistory::~DataWriterHistory()
{
    Companion::detach(this);
}

bool DataWriterHistory::wait_for_acknowledgement_last_change(
        const InstanceHandle_t& handle,
        std::unique_lock<RecursiveTimedMutex>& lock,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    return Companion::get(this).wait_for_acknowledgement_last_change(handle, lock, max_blocking_time);
}

void DataWriterHistory::rebuild_instances()
{
}

bool DataWriterHistory::register_instance(
        const InstanceHandle_t& instance_handle,
        std::unique_lock<RecursiveTimedMutex>&,
        const std::chrono::time_point<std::chrono::steady_clock>&,
        SerializedPayload_t*& payload)
{
    payload = nullptr;

    /// Preconditions
    if (topic_kind_ == NO_KEY)
    {
        return false;
    }

    t_m_Inst_Caches::iterator vit;
    bool result = find_or_add_key(instance_handle, SerializedPayload_t(), &vit);
    if (result)
    {
        payload = &vit->second.key_payload;
    }
    return result;
}

bool DataWriterHistory::is_key_registered(
        const InstanceHandle_t& handle)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_HISTORY, "You need to create a Writer with this History before using it");
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

SerializedPayload_t* DataWriterHistory::get_key_value(
        const InstanceHandle_t& handle)
{
    t_m_Inst_Caches::iterator vit = keyed_changes_.find(handle);
    if (vit != keyed_changes_.end() && vit->second.is_registered())
    {
        return &vit->second.key_payload;
    }
    return nullptr;
}

bool DataWriterHistory::add_pub_change(
        CacheChange_t* change,
        WriteParams& wparams,
        std::unique_lock<RecursiveTimedMutex>& lock,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    bool returnedValue = false;

    if (prepare_change(change, lock, max_blocking_time))
    {
        if (this->add_change_(change, wparams))
        {
            returnedValue = true;
        }
    }
    return returnedValue;
}

// Called by add_pub_change_with_commit_hook(), defined in the header, before adding the change.
bool DataWriterHistory::prepare_change(
        CacheChange_t* change,
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

    // For NO_KEY we can directly add the change
    bool add = (topic_kind_ == NO_KEY);
    if (topic_kind_ == WITH_KEY)
    {
        t_m_Inst_Caches::iterator vit;

        if (find_or_add_key(change->instanceHandle, change->serializedPayload, &vit))
        {
            vit->second.cache_changes.push_back(change);
            add = true;
        }
    }

    return add;
}

bool DataWriterHistory::set_next_deadline(
        const InstanceHandle_t&,
        const std::chrono::steady_clock::time_point&)
{
    return true;
}

bool DataWriterHistory::removeMinChange()
{
    return true;
}

bool DataWriterHistory::removeAllChange(
        size_t*)
{
    return true;
}

bool DataWriterHistory::remove_instance_changes(
        const InstanceHandle_t&,
        const SequenceNumber_t&)
{
    return true;
}

bool DataWriterHistory::get_next_deadline(
        InstanceHandle_t&,
        std::chrono::steady_clock::time_point&)
{
    return true;
}

bool DataWriterHistory::remove_change_pub(
        CacheChange_t* change)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*this->mp_mutex);
    if (topic_kind_ == NO_KEY)
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
        if (!this->find_or_add_key(change->instanceHandle, change->serializedPayload, &vit))
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

// The previous mock did not override remove_change_g(), so the WriterHistory ones were used.
bool DataWriterHistory::remove_change_g(
        CacheChange_t* a_change)
{
    return WriterHistory::remove_change_g(a_change);
}

bool DataWriterHistory::remove_change_g(
        CacheChange_t* a_change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    return WriterHistory::remove_change_g(a_change, max_blocking_time);
}

bool DataWriterHistory::find_or_add_key(
        const InstanceHandle_t& instance_handle,
        const SerializedPayload_t&,
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
        *vit_out = keyed_changes_.insert(std::make_pair(instance_handle, detail::DataWriterInstance())).first;
    }
    return true;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
