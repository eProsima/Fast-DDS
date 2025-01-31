// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataWriterHistory.cpp
 */
#include <fastdds/publisher/DataWriterHistory.hpp>

#include <chrono>
#include <limits>
#include <mutex>

#include <fastdds/dds/common/InstanceHandle.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <rtps/writer/BaseWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

using namespace eprosima::fastdds::rtps;

HistoryAttributes DataWriterHistory::to_history_attributes(
        const HistoryQosPolicy& history_qos,
        const ResourceLimitsQosPolicy& resource_limits_qos,
        const rtps::TopicKind_t& topic_kind,
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
            max_samples *= resource_limits_qos.max_instances;
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
        const rtps::TopicKind_t& topic_kind,
        uint32_t payloadMaxSize,
        MemoryManagementPolicy_t mempolicy,
        std::function<void (const fastdds::rtps::InstanceHandle_t&)> unack_sample_remove_functor)
    : WriterHistory(to_history_attributes(history_qos, resource_limits_qos, topic_kind, payloadMaxSize,
            mempolicy), payload_pool, change_pool)
    , history_qos_(history_qos)
    , resource_limited_qos_(resource_limits_qos)
    , topic_kind_(topic_kind)
    , unacknowledged_sample_removed_functor_(unack_sample_remove_functor)
{
    if (resource_limited_qos_.max_samples <= 0)
    {
        resource_limited_qos_.max_samples = std::numeric_limits<int32_t>::max();
    }

    if (resource_limited_qos_.max_instances <= 0)
    {
        resource_limited_qos_.max_instances = std::numeric_limits<int32_t>::max();
    }

    if (resource_limited_qos_.max_samples_per_instance <= 0)
    {
        resource_limited_qos_.max_samples_per_instance = std::numeric_limits<int32_t>::max();
    }
}

DataWriterHistory::~DataWriterHistory()
{
}

void DataWriterHistory::rebuild_instances()
{
    if (topic_kind_ == WITH_KEY)
    {
        for (CacheChange_t* change : m_changes)
        {
            t_m_Inst_Caches::iterator vit;
            if (find_or_add_key(change->instanceHandle, change->serializedPayload, &vit))
            {
                vit->second.cache_changes.push_back(change);
            }
        }
    }
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
    bool result = find_or_add_key(instance_handle, {}, &vit);
    if (result)
    {
        payload = &vit->second.key_payload;
    }
    return result;
}

fastdds::rtps::SerializedPayload_t* DataWriterHistory::get_key_value(
        const fastdds::rtps::InstanceHandle_t& handle)
{
    t_m_Inst_Caches::iterator vit = keyed_changes_.find(handle);
    if (vit != keyed_changes_.end() && vit->second.is_registered())
    {
        return &vit->second.key_payload;
    }
    return nullptr;
}

bool DataWriterHistory::prepare_change(
        CacheChange_t* change,
        std::unique_lock<RecursiveTimedMutex>& lock,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    if (m_isHistoryFull)
    {
        bool ret = false;
        bool is_acked = change_is_acked_or_fully_delivered(m_changes.front());
        InstanceHandle_t instance = topic_kind_ == NO_KEY ?
                HANDLE_NIL : m_changes.front()->instanceHandle;

        if (history_qos_.kind == KEEP_ALL_HISTORY_QOS)
        {
            ret = this->mp_writer->try_remove_change(max_blocking_time, lock);
            // If change was removed (ret == 1) in KeepAllHistory, it must have been acked
            is_acked = ret;
        }
        else if (history_qos_.kind == KEEP_LAST_HISTORY_QOS)
        {
            ret = this->remove_min_change(max_blocking_time);
        }

        // Notify if change has been removed unacknowledged
        if (ret && !is_acked)
        {
            unacknowledged_sample_removed_functor_(instance);
        }
        else if (!ret)
        {
            EPROSIMA_LOG_WARNING(RTPS_HISTORY,
                    "Attempting to add Data to Full WriterCache.");
            return false;
        }
    }

    assert(!m_isHistoryFull);

    // For NO_KEY we can directly add the change
    bool add = (topic_kind_ == NO_KEY);
    if (topic_kind_ == WITH_KEY)
    {
        t_m_Inst_Caches::iterator vit;

        // For WITH_KEY, we take into account the limits on the instance
        // In case we wait for a sequence to be acknowledged, we try several times
        // until we reach the max blocking timepoint
        while (!add)
        {
            // We should have the instance
            if (!find_or_add_key(change->instanceHandle, change->serializedPayload, &vit))
            {
                break;
            }

            if (history_qos_.kind == KEEP_LAST_HISTORY_QOS)
            {
                if (vit->second.cache_changes.size() < static_cast<size_t>(history_qos_.depth))
                {
                    add = true;
                }
                else
                {
                    bool is_acked = change_is_acked_or_fully_delivered(vit->second.cache_changes.front());
                    InstanceHandle_t instance = change->instanceHandle;
                    add = remove_change_pub(vit->second.cache_changes.front());
                    // Notify if removed unacknowledged
                    if (add && !is_acked)
                    {
                        unacknowledged_sample_removed_functor_(instance);
                    }
                }
            }
            else if (history_qos_.kind == KEEP_ALL_HISTORY_QOS)
            {
                if (vit->second.cache_changes.size() <
                        static_cast<size_t>(resource_limited_qos_.max_samples_per_instance))
                {
                    add = true;
                }
                else
                {
                    SequenceNumber_t seq_to_remove = vit->second.cache_changes.front()->sequenceNumber;
                    if (!mp_writer->wait_for_acknowledgement(seq_to_remove, max_blocking_time, lock))
                    {
                        // Timeout waiting. Will not add change to history.
                        break;
                    }

                    // vit may have been invalidated
                    if (!find_or_add_key(change->instanceHandle, change->serializedPayload, &vit))
                    {
                        break;
                    }

                    // If the change we were trying to remove was already removed, try again
                    if (vit->second.cache_changes.empty() ||
                            vit->second.cache_changes.front()->sequenceNumber != seq_to_remove)
                    {
                        continue;
                    }

                    // Remove change if still present
                    add = remove_change_pub(vit->second.cache_changes.front());
                }
            }
        }

        if (add)
        {
            vit->second.cache_changes.push_back(change);
        }
    }

    return add;
}

bool DataWriterHistory::add_pub_change(
        CacheChange_t* change,
        WriteParams& wparams,
        std::unique_lock<RecursiveTimedMutex>& lock,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    bool returnedValue = false;
    bool add = prepare_change(change, lock, max_blocking_time);

    if (add)
    {
#if HAVE_STRICT_REALTIME
        if (this->add_change_(change, wparams, max_blocking_time))
#else
        if (this->add_change_(change, wparams))
#endif // if HAVE_STRICT_REALTIME
        {
            EPROSIMA_LOG_INFO(RTPS_HISTORY,
                    " Change " << change->sequenceNumber << " added with key: " << change->instanceHandle
                               << " and " << change->serializedPayload.length << " bytes");
            returnedValue = true;
        }
    }

    return returnedValue;
}

bool DataWriterHistory::find_or_add_key(
        const InstanceHandle_t& instance_handle,
        const SerializedPayload_t& payload,
        t_m_Inst_Caches::iterator* vit_out)
{
    static_cast<void>(payload);

    t_m_Inst_Caches::iterator vit;
    vit = keyed_changes_.find(instance_handle);
    if (vit != keyed_changes_.end())
    {
        *vit_out = vit;
        return true;
    }

    if (static_cast<int>(keyed_changes_.size()) < resource_limited_qos_.max_instances)
    {
        vit = keyed_changes_.insert(std::make_pair(instance_handle, detail::DataWriterInstance())).first;
        vit->second.key_payload.copy(&payload, false);
        *vit_out = vit;
        return true;
    }

    return false;
}

bool DataWriterHistory::removeAllChange(
        size_t* removed)
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

bool DataWriterHistory::removeMinChange()
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_HISTORY, "You need to create a Writer with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*this->mp_mutex);
    if (m_changes.size() > 0)
    {
        return remove_change_pub(m_changes.front());
    }
    return false;
}

bool DataWriterHistory::remove_change_pub(
        CacheChange_t* change)
{
    return DataWriterHistory::remove_change_pub(change, std::chrono::steady_clock::now() + std::chrono::hours(24));
}

bool DataWriterHistory::remove_change_pub(
        CacheChange_t* change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_HISTORY, "You need to create a Writer with this History before using it");
        return false;
    }

#if HAVE_STRICT_REALTIME
    std::unique_lock<RecursiveTimedMutex> lock(*this->mp_mutex, std::defer_lock);
    if (!lock.try_lock_until(max_blocking_time))
    {
        EPROSIMA_LOG_ERROR(PUBLISHER, "Cannot lock the DataWriterHistory mutex");
        return false;
    }
#else
    std::lock_guard<RecursiveTimedMutex> guard(*this->mp_mutex);
#endif // if HAVE_STRICT_REALTIME

    if (topic_kind_ == NO_KEY)
    {
        if (remove_change(change, max_blocking_time))
        {
            m_isHistoryFull = false;
            return true;
        }

        return false;
    }
    else
    {
        t_m_Inst_Caches::iterator vit;
        vit = keyed_changes_.find(change->instanceHandle);
        if (vit == keyed_changes_.end())
        {
            return false;
        }

        for (auto chit = vit->second.cache_changes.begin(); chit != vit->second.cache_changes.end(); ++chit)
        {
            if (((*chit)->sequenceNumber == change->sequenceNumber) && ((*chit)->writerGUID == change->writerGUID))
            {
                if (remove_change(change, max_blocking_time))
                {
                    vit->second.cache_changes.erase(chit);
                    m_isHistoryFull = false;
                    return true;
                }
            }
        }
        EPROSIMA_LOG_ERROR(PUBLISHER, "Change not found, something is wrong");
    }
    return false;
}

bool DataWriterHistory::remove_change_g(
        CacheChange_t* a_change)
{
    return remove_change_pub(a_change, std::chrono::steady_clock::now() + std::chrono::hours(24));
}

bool DataWriterHistory::remove_change_g(
        CacheChange_t* a_change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    return remove_change_pub(a_change, max_blocking_time);
}

bool DataWriterHistory::remove_instance_changes(
        const InstanceHandle_t& handle,
        const SequenceNumber_t& seq_up_to)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_HISTORY, "You need to create a Writer with this History before using it");
        return false;
    }

    if (topic_kind_ == NO_KEY)
    {
        EPROSIMA_LOG_ERROR(RTPS_HISTORY, "Cannot be removed instance changes of a NO_KEY DataType");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*this->mp_mutex);
    t_m_Inst_Caches::iterator vit;
    vit = keyed_changes_.find(handle);
    if (vit == keyed_changes_.end())
    {
        return false;
    }

    auto chit = vit->second.cache_changes.begin();

    for (; chit != vit->second.cache_changes.end() && (*chit)->sequenceNumber <= seq_up_to; ++chit)
    {
        if (remove_change(*chit))
        {
            m_isHistoryFull = false;
        }
    }

    vit->second.cache_changes.erase(vit->second.cache_changes.begin(), chit);

    if (vit->second.cache_changes.empty())
    {
        keyed_changes_.erase(vit);
    }

    return true;
}

bool DataWriterHistory::set_next_deadline(
        const InstanceHandle_t& handle,
        const std::chrono::steady_clock::time_point& next_deadline_us)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_HISTORY, "You need to create a Writer with this History before using it");
        return false;
    }
    std::lock_guard<RecursiveTimedMutex> guard(*this->mp_mutex);

    if (topic_kind_ == NO_KEY)
    {
        next_deadline_us_ = next_deadline_us;
        return true;
    }
    else if (topic_kind_ == WITH_KEY)
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

bool DataWriterHistory::get_next_deadline(
        InstanceHandle_t& handle,
        std::chrono::steady_clock::time_point& next_deadline_us)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_HISTORY, "You need to create a Writer with this History before using it");
        return false;
    }
    std::lock_guard<RecursiveTimedMutex> guard(*this->mp_mutex);

    if (topic_kind_ == WITH_KEY)
    {
        auto min = std::min_element(
            keyed_changes_.begin(),
            keyed_changes_.end(),
            [](
                const t_m_Inst_Caches::value_type& lhs,
                const t_m_Inst_Caches::value_type& rhs)
            {
                return lhs.second.next_deadline_us < rhs.second.next_deadline_us;
            });

        handle = min->first;
        next_deadline_us = min->second.next_deadline_us;
        return true;
    }
    else if (topic_kind_ == NO_KEY)
    {
        next_deadline_us = next_deadline_us_;
        return true;
    }

    return false;
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
    return vit != keyed_changes_.end() && vit->second.is_registered();
}

bool DataWriterHistory::wait_for_acknowledgement_last_change(
        const InstanceHandle_t& handle,
        std::unique_lock<RecursiveTimedMutex>& lock,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    if (WITH_KEY == topic_kind_)
    {
        // Find the instance
        t_m_Inst_Caches::iterator vit = keyed_changes_.find(handle);
        if (vit != keyed_changes_.end())
        {
            SequenceNumber_t seq = vit->second.cache_changes.back()->sequenceNumber;
            return mp_writer->wait_for_acknowledgement(seq, max_blocking_time, lock);
        }
    }
    return false;
}

bool DataWriterHistory::change_is_acked_or_fully_delivered(
        const CacheChange_t* change)
{
    bool is_acked = false;
    if (mp_writer->get_disable_positive_acks())
    {
        is_acked = mp_writer->has_been_fully_delivered(change->sequenceNumber);
    }
    else
    {
        is_acked = mp_writer->is_acked_by_all(change->sequenceNumber);
    }
    return is_acked;
}

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
