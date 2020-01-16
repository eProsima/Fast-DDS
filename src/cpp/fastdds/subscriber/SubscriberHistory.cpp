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

#include <fastdds/dds/subscriber/SubscriberHistory.hpp>
#include <fastdds/topic/DataReaderImpl.hpp>

#include <fastdds/rtps/reader/RTPSReader.h>
#include <rtps/reader/WriterProxy.h>

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/core/status/SampleRejectedStatus.hpp>
#include <fastrtps/log/Log.h>

#include <mutex>

namespace eprosima {
namespace fastdds {
namespace dds {

using namespace fastrtps::rtps;

SubscriberHistory::SubscriberHistory(
        const fastrtps::TopicAttributes& topic_att,
        TopicDataType* type,
        const DataReaderQos& qos,
        uint32_t payloadMaxSize,
        MemoryManagementPolicy_t mempolicy)
    : ReaderHistory(HistoryAttributes(mempolicy, payloadMaxSize,
            qos.history.kind == KEEP_ALL_HISTORY_QOS ?
            qos.resource_limits.allocated_samples :
            topic_att.getTopicKind() == NO_KEY ?
            std::min(qos.resource_limits.allocated_samples, qos.history.depth) :
            std::min(qos.resource_limits.allocated_samples, qos.history.depth
            * qos.resource_limits.max_instances),
            qos.history.kind == KEEP_ALL_HISTORY_QOS ?
            qos.resource_limits.max_samples :
            topic_att.getTopicKind() == NO_KEY ?
            qos.history.depth :
            qos.history.depth * qos.resource_limits.max_instances))
    , history_qos_(qos.history)
    , resource_limited_qos_(qos.resource_limits)
    , topic_att_(topic_att)
    , type_(type)
    , qos_(qos)
    , get_key_object_(nullptr)
{
    if (type_->m_isGetKeyDefined)
    {
        get_key_object_ = type_->createData();
    }

    using std::placeholders::_1;
    using std::placeholders::_2;

    if (topic_att.getTopicKind() == NO_KEY)
    {
        receive_fn_ = qos.history.kind == KEEP_ALL_HISTORY_QOS ?
                std::bind(&SubscriberHistory::received_change_keep_all_no_key, this, _1, _2) :
                std::bind(&SubscriberHistory::received_change_keep_last_no_key, this, _1, _2);
    }
    else
    {
        receive_fn_ = qos.history.kind == KEEP_ALL_HISTORY_QOS ?
                std::bind(&SubscriberHistory::received_change_keep_all_with_key, this, _1, _2) :
                std::bind(&SubscriberHistory::received_change_keep_last_with_key, this, _1, _2);
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

    std::lock_guard<fastrtps::RecursiveTimedMutex> guard(*mp_mutex);
    return receive_fn_(a_change, unknown_missing_changes_up_to);
}

bool SubscriberHistory::received_change_keep_all_no_key(
        CacheChange_t* a_change,
        size_t unknown_missing_changes_up_to)
{
    // TODO(Ricardo) Check
    if (m_changes.size() + unknown_missing_changes_up_to < static_cast<size_t>(resource_limited_qos_.max_samples) )
    {
        return add_received_change(a_change);
    }

    if (a_change->sequenceNumber != mp_reader->sample_rejected_status_.last_seq_num)
    {
        mp_reader->sample_rejected_status_.total_count++;
        mp_reader->sample_rejected_status_.total_count_change++;
        mp_reader->sample_rejected_status_.last_reason = fastdds::dds::REJECTED_BY_SAMPLES_LIMIT;
        mp_reader->sample_rejected_status_.last_instance_handle = a_change->instanceHandle;
        mp_reader->sample_rejected_status_.last_seq_num = a_change->sequenceNumber;
        mp_reader->getListener()->on_sample_rejected(mp_reader, mp_reader->sample_rejected_status_);
    }

    return false;
}

bool SubscriberHistory::received_change_keep_last_no_key(
        CacheChange_t* a_change,
        size_t /* unknown_missing_changes_up_to */ )
{
    bool add = false;
    if (m_changes.size() < static_cast<size_t>(history_qos_.depth) )
    {
        add = true;
    }
    else
    {
        // Try to substitute the oldest sample.

        // As the history should be ordered following the presentation QoS, we can always remove the first one.
        add = remove_change_sub(m_changes.at(0));
    }

    if (add)
    {
        return add_received_change(a_change);
    }

    return false;
}

bool SubscriberHistory::received_change_keep_all_with_key(
        CacheChange_t* a_change,
        size_t /* unknown_missing_changes_up_to */ )
{
    // TODO(Miguel C): Should we check unknown_missing_changes_up_to as it is done in received_change_keep_all_no_key?

    t_m_Inst_Caches::iterator vit;
    if (find_key_for_change(a_change, vit))
    {
        std::vector<CacheChange_t*>& instance_changes = vit->second.cache_changes;
        if (instance_changes.size() < static_cast<size_t>(resource_limited_qos_.max_samples_per_instance) )
        {
            return add_received_change_with_key(a_change, vit->second.cache_changes);
        }

        if (a_change->sequenceNumber != mp_reader->sample_rejected_status_.last_seq_num)
        {
            mp_reader->sample_rejected_status_.total_count++;
            mp_reader->sample_rejected_status_.total_count_change++;
            mp_reader->sample_rejected_status_.last_reason = fastdds::dds::REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
            mp_reader->sample_rejected_status_.last_instance_handle = a_change->instanceHandle;
            mp_reader->sample_rejected_status_.last_seq_num = a_change->sequenceNumber;
            mp_reader->getListener()->on_sample_rejected(mp_reader, mp_reader->sample_rejected_status_);
        }
        logWarning(SUBSCRIBER, "Change not added due to maximum number of samples per instance");
    }

    return false;
}

bool SubscriberHistory::received_change_keep_last_with_key(
        CacheChange_t* a_change,
        size_t /* unknown_missing_changes_up_to */)
{
    t_m_Inst_Caches::iterator vit;
    if (find_key_for_change(a_change, vit))
    {
        bool add = false;
        std::vector<CacheChange_t*>& instance_changes = vit->second.cache_changes;
        if (instance_changes.size() < static_cast<size_t>(history_qos_.depth) )
        {
            add = true;
        }
        else
        {
            // Try to substitute the oldest sample.

            // As the instance should be ordered following the presentation QoS, we can always remove the first one.
            add = remove_change_sub(instance_changes.at(0));
        }

        if (add)
        {
            return add_received_change_with_key(a_change, instance_changes);
        }
    }

    return false;
}

bool SubscriberHistory::add_received_change(
        CacheChange_t* a_change)
{
    if (m_isHistoryFull)
    {
        // Discarding the sample.
        if (a_change->sequenceNumber != mp_reader->sample_rejected_status_.last_seq_num)
        {
            mp_reader->sample_rejected_status_.total_count++;
            mp_reader->sample_rejected_status_.total_count_change++;
            mp_reader->sample_rejected_status_.last_reason = fastdds::dds::REJECTED_BY_SAMPLES_LIMIT;
            mp_reader->sample_rejected_status_.last_instance_handle = a_change->instanceHandle;
            mp_reader->sample_rejected_status_.last_seq_num = a_change->sequenceNumber;
            mp_reader->getListener()->on_sample_rejected(mp_reader, mp_reader->sample_rejected_status_);
        }
        logWarning(SUBSCRIBER, "Attempting to add Data to Full ReaderHistory: " << topic_att_.getTopicDataType());
        return false;
    }

    if (add_change(a_change))
    {
        if (m_changes.size() == static_cast<size_t>(resource_limited_qos_.max_samples) )
        {
            m_isHistoryFull = true;
        }

        logInfo(SUBSCRIBER, topic_att_.getTopicDataType()
                << ": Change " << a_change->sequenceNumber << " added from: "
                << a_change->writerGUID; );

        return true;
    }

    return false;
}

bool SubscriberHistory::add_received_change_with_key(
        CacheChange_t* a_change,
        std::vector<CacheChange_t*>& instance_changes)
{
    if (m_isHistoryFull)
    {
        // Discarting the sample.
        if (a_change->sequenceNumber != mp_reader->sample_rejected_status_.last_seq_num)
        {
            mp_reader->sample_rejected_status_.total_count++;
            mp_reader->sample_rejected_status_.total_count_change++;
            mp_reader->sample_rejected_status_.last_reason = fastdds::dds::REJECTED_BY_SAMPLES_LIMIT;
            mp_reader->sample_rejected_status_.last_instance_handle = a_change->instanceHandle;
            mp_reader->sample_rejected_status_.last_seq_num = a_change->sequenceNumber;
            mp_reader->getListener()->on_sample_rejected(mp_reader, mp_reader->sample_rejected_status_);
        }
        logWarning(SUBSCRIBER, "Attempting to add Data to Full ReaderHistory: " << topic_att_.getTopicDataType());
        return false;
    }

    if (add_change(a_change))
    {
        if (m_changes.size() == static_cast<size_t>(resource_limited_qos_.max_samples))
        {
            m_isHistoryFull = true;
        }

        auto it = most_recent_sample_.find(a_change->instanceHandle);
        if (it != most_recent_sample_.end())
        {
            if (it->second.instance_state == ::dds::sub::status::InstanceState::not_alive_disposed() &&
                    a_change->kind == fastrtps::rtps::ChangeKind_t::ALIVE)
            {
                it->second.disposed_generation_count++;
            }
            else if (it->second.instance_state == ::dds::sub::status::InstanceState::not_alive_no_writers() &&
                    a_change->kind == fastrtps::rtps::ChangeKind_t::ALIVE)
            {
                it->second.no_writers_generation_count++;
            }
            it->second.instance_state = a_change->kind;

        }
        else
        {
            SampleInfo_t sample;
            sample.instance_state = a_change->kind;
            most_recent_sample_[a_change->instanceHandle] = sample;
        }

        //ADD TO KEY VECTOR

        // As the instance should be ordered following the presentation QoS, and
        // we only support ordering by reception timestamp, we can always add at the end.
        instance_changes.push_back(a_change);

        logInfo(SUBSCRIBER, mp_reader->getGuid().entityId
                << ": Change " << a_change->sequenceNumber << " added from: "
                << a_change->writerGUID << " with KEY: " << a_change->instanceHandle; );

        return true;
    }

    return false;
}

bool SubscriberHistory::find_key_for_change(
        CacheChange_t* a_change,
        t_m_Inst_Caches::iterator& map_it)
{
    if (!a_change->instanceHandle.isDefined() && type_ != nullptr)
    {
        logInfo(RTPS_HISTORY, "Getting Key of change with no Key transmitted")
        type_->deserialize(&a_change->serializedPayload, get_key_object_);
        bool is_key_protected = false;
#if HAVE_SECURITY
        is_key_protected = mp_reader->getAttributes().security_attributes().is_key_protected;
#endif
        if (!type_->getKey(get_key_object_, &a_change->instanceHandle, is_key_protected))
        {
            return false;
        }
    }
    else if (!a_change->instanceHandle.isDefined())
    {
        logWarning(RTPS_HISTORY, "NO KEY in topic: " << topic_att_.topicName
                                                     << " and no method to obtain it"; );
        return false;
    }

    return find_key(a_change, &map_it);
}

void SubscriberHistory::deserialize_change(
        CacheChange_t* change,
        uint32_t /*ownership_strength*/,
        void* data,
        SampleInfo_t* info)
{
    if (change->kind == ALIVE)
    {
        type_->deserialize(&change->serializedPayload, data);
    }

    if (info != nullptr)
    {
        info->sample_state = ::dds::sub::status::SampleState::not_read();
        info->view_state = ::dds::sub::status::ViewState::new_view();
        info->publication_handle = change->writerGUID;
        info->source_timestamp = change->sourceTimestamp;
        info->sample_rank = static_cast<uint32_t>(change->sequenceNumber.to64long());
        info->valid_data = false;
        auto instance_it = instance_info_.find(change->instanceHandle);
        if (change->kind == fastrtps::rtps::ChangeKind_t::ALIVE)
        {
            if (instance_it != instance_info_.end())
            {
                info->view_state = instance_it->second.view_state;
                if (instance_it->second.instance_state == ::dds::sub::status::InstanceState::not_alive_disposed())
                {
                    instance_it->second.disposed_generation_count++;
                }
                else if (instance_it->second.instance_state ==
                        ::dds::sub::status::InstanceState::not_alive_no_writers())
                {
                    instance_it->second.no_writers_generation_count++;
                }
            }
            info->instance_state = ::dds::sub::status::InstanceState::alive();
            info->valid_data = true;
        }
        else if (change->kind == fastrtps::rtps::ChangeKind_t::NOT_ALIVE_DISPOSED)
        {
            info->instance_state = ::dds::sub::status::InstanceState::not_alive_disposed();
        }
        else if (change->kind == fastrtps::rtps::ChangeKind_t::NOT_ALIVE_UNREGISTERED)
        {
            info->instance_state = ::dds::sub::status::InstanceState::not_alive_no_writers();
        }
        else if (change->kind == fastrtps::rtps::ChangeKind_t::NOT_ALIVE_DISPOSED_UNREGISTERED)
        {
            info->instance_state = ::dds::sub::status::InstanceState::not_alive_mask();
        }
        instance_it->second.instance_state = info->instance_state;
        info->generation_rank = 0;
        auto MRS = most_recent_sample_.find(change->instanceHandle);
        if (instance_it != instance_info_.end())
        {
            info->disposed_generation_count = instance_it->second.disposed_generation_count;
            info->no_writers_generation_count = instance_it->second.no_writers_generation_count;
        }
        if (MRS != most_recent_sample_.end())
        {
            info->absolute_generation_rank =
                    (MRS->second.disposed_generation_count + MRS->second.no_writers_generation_count) -
                    (info->disposed_generation_count + info->no_writers_generation_count);
        }

        if (topic_att_.topicKind == WITH_KEY &&
                change->instanceHandle == c_InstanceHandle_Unknown &&
                change->kind == ALIVE)
        {
            bool is_key_protected = false;
#if HAVE_SECURITY
            is_key_protected = mp_reader->getAttributes().security_attributes().is_key_protected;
#endif
            type_->getKey(data, &change->instanceHandle, is_key_protected);
        }
        info->instance_handle = change->instanceHandle;
    }
}

void SubscriberHistory::notify_not_new(
        SampleInfo_t* info)
{
    auto it = instance_info_.find(info->instance_handle);
    if (it != instance_info_.end())
    {
        it->second.view_state = ::dds::sub::status::ViewState::not_new_view();
    }
    else
    {
        SampleInfo_t s_info;
        s_info.view_state = ::dds::sub::status::ViewState::not_new_view();
        instance_info_[info->instance_handle] = s_info;
    }
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

    std::unique_lock<fastrtps::RecursiveTimedMutex> lock(*mp_mutex, std::defer_lock);

    if (lock.try_lock_until(max_blocking_time))
    {
        CacheChange_t* change;
        WriterProxy* wp;
        if (mp_reader->nextUnreadCache(&change, &wp))
        {
            logInfo(SUBSCRIBER, mp_reader->getGuid().entityId << ": reading " << change->sequenceNumber);
            uint32_t ownership = wp && qos_.ownership.kind == EXCLUSIVE_OWNERSHIP_QOS ?
                    wp->ownership_strength() : 0;
            deserialize_change(change, ownership, data, info);
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

    std::unique_lock<fastrtps::RecursiveTimedMutex> lock(*mp_mutex, std::defer_lock);

    if (lock.try_lock_until(max_blocking_time))
    {
        CacheChange_t* change = nullptr;
        WriterProxy* wp = nullptr;
        if (mp_reader->nextUntakenCache(&change, &wp))
        {
            logInfo(SUBSCRIBER, mp_reader->getGuid().entityId << ": taking seqNum" << change->sequenceNumber <<
                    " from writer: " << change->writerGUID);
            uint32_t ownership = wp && qos_.ownership.kind == EXCLUSIVE_OWNERSHIP_QOS ?
                    wp->ownership_strength() : 0;
            deserialize_change(change, ownership, data, info);
            remove_change_sub(change);
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

    if (keyed_changes_.size() < static_cast<size_t>(resource_limited_qos_.max_instances))
    {
        *vit_out = keyed_changes_.insert(std::make_pair(a_change->instanceHandle, fastrtps::KeyedChanges())).first;
        return true;
    }
    else
    {
        for (vit = keyed_changes_.begin(); vit != keyed_changes_.end(); ++vit)
        {
            if (vit->second.cache_changes.size() == 0)
            {
                keyed_changes_.erase(vit);
                *vit_out =
                        keyed_changes_.insert(std::make_pair(a_change->instanceHandle,
                                fastrtps::KeyedChanges())).first;
                return true;
            }
        }
        if (a_change->sequenceNumber != mp_reader->sample_rejected_status_.last_seq_num)
        {
            mp_reader->sample_rejected_status_.total_count++;
            mp_reader->sample_rejected_status_.total_count_change++;
            mp_reader->sample_rejected_status_.last_reason = fastdds::dds::REJECTED_BY_INSTANCES_LIMIT;
            mp_reader->sample_rejected_status_.last_instance_handle = a_change->instanceHandle;
            mp_reader->sample_rejected_status_.last_seq_num = a_change->sequenceNumber;
            mp_reader->getListener()->on_sample_rejected(mp_reader, mp_reader->sample_rejected_status_);
        }

        logWarning(SUBSCRIBER, "History has reached the maximum number of instances");
    }
    return false;
}

bool SubscriberHistory::remove_change_sub(
        CacheChange_t* change)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Reader with this History before using it");
        return false;
    }

    std::lock_guard<fastrtps::RecursiveTimedMutex> guard(*mp_mutex);
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
        if (!find_key(change, &vit))
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
    std::lock_guard<fastrtps::RecursiveTimedMutex> guard(*mp_mutex);

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
        InstanceHandle_t& handle,
        std::chrono::steady_clock::time_point& next_deadline_us)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_HISTORY, "You need to create a Reader with this History before using it");
        return false;
    }
    std::lock_guard<fastrtps::RecursiveTimedMutex> guard(*mp_mutex);

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
                            const std::pair<InstanceHandle_t, fastrtps::KeyedChanges>& lhs,
                            const std::pair<InstanceHandle_t, fastrtps::KeyedChanges>& rhs)
                    {
                        return lhs.second.next_deadline_us < rhs.second.next_deadline_us;
                    });
        handle = min->first;
        next_deadline_us = min->second.next_deadline_us;
        return true;
    }

    return false;
}

} // namespace dds
} // namespace fastdds
} // namsepace eprosima
