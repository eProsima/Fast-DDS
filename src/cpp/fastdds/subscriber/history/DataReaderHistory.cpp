// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataReaderHistory.cpp
 */

#include <limits>
#include <mutex>

#include "DataReaderHistory.hpp"

#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/reader/RTPSReader.h>

#include <fastdds/subscriber/DataReaderImpl/ReadTakeCommand.hpp>
#include <rtps/reader/WriterProxy.h>
#include <utils/collections/sorted_vector_insert.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

using namespace eprosima::fastrtps::rtps;

using eprosima::fastrtps::RecursiveTimedMutex;

static HistoryAttributes to_history_attributes(
        const TypeSupport& type,
        const DataReaderQos& qos)
{
    auto initial_samples = qos.resource_limits().allocated_samples;
    auto max_samples = qos.resource_limits().max_samples;

    auto mempolicy = qos.endpoint().history_memory_policy;
    auto payloadMaxSize = type->m_typeSize + 3; // possible alignment

    return HistoryAttributes(mempolicy, payloadMaxSize, initial_samples, max_samples);
}

DataReaderHistory::DataReaderHistory(
        const TypeSupport& type,
        const TopicDescription& topic,
        const DataReaderQos& qos)
    : ReaderHistory(to_history_attributes(type, qos))
    , key_writers_allocation_(qos.reader_resource_limits().matched_publisher_allocation)
    , history_qos_(qos.history())
    , resource_limited_qos_(qos.resource_limits())
    , topic_name_(topic.get_name())
    , type_name_(topic.get_type_name())
    , has_keys_(type->m_isGetKeyDefined)
    , type_(type.get())
    , get_key_object_(nullptr)
{
    if (resource_limited_qos_.max_samples == 0)
    {
        resource_limited_qos_.max_samples = std::numeric_limits<int32_t>::max();
    }

    if (resource_limited_qos_.max_instances == 0)
    {
        resource_limited_qos_.max_instances = std::numeric_limits<int32_t>::max();
    }

    if (resource_limited_qos_.max_samples_per_instance == 0)
    {
        resource_limited_qos_.max_samples_per_instance = std::numeric_limits<int32_t>::max();
    }

    if (type_->m_isGetKeyDefined)
    {
        get_key_object_ = type_->createData();

        if (resource_limited_qos_.max_samples_per_instance < std::numeric_limits<int32_t>::max())
        {
            key_changes_allocation_.maximum = resource_limited_qos_.max_samples_per_instance;
        }
    }
    else
    {
        resource_limited_qos_.max_instances = 1;
        resource_limited_qos_.max_samples_per_instance = resource_limited_qos_.max_samples;
        key_changes_allocation_.initial = resource_limited_qos_.allocated_samples;
        key_changes_allocation_.maximum = resource_limited_qos_.max_samples;

        keyed_changes_.emplace(c_InstanceHandle_Unknown,
                DataReaderInstance{ key_changes_allocation_, key_writers_allocation_ });
    }

    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    using std::placeholders::_4;

    receive_fn_ = qos.history().kind == KEEP_ALL_HISTORY_QOS ?
            std::bind(&DataReaderHistory::received_change_keep_all, this, _1, _2, _3) :
            std::bind(&DataReaderHistory::received_change_keep_last, this, _1, _2, _3);

    complete_fn_ = qos.history().kind == KEEP_ALL_HISTORY_QOS ?
            std::bind(&DataReaderHistory::completed_change_keep_all, this, _1, _2, _3, _4) :
            std::bind(&DataReaderHistory::completed_change_keep_last, this, _1, _2, _3, _4);

    if (!has_keys_)
    {
        compute_key_for_change_fn = [](CacheChange_t* change)
                {
                    change->instanceHandle = c_InstanceHandle_Unknown;
                    return true;
                };
    }
    else
    {
        compute_key_for_change_fn =
                [this](CacheChange_t* a_change)
                {
                    if (a_change->instanceHandle.isDefined())
                    {
                        return true;
                    }

                    if (!a_change->is_fully_assembled())
                    {
                        return false;
                    }

                    if (type_ != nullptr)
                    {
                        logInfo(SUBSCRIBER, "Getting Key of change with no Key transmitted");
                        type_->deserialize(&a_change->serializedPayload, get_key_object_);
                        bool is_key_protected = false;
#if HAVE_SECURITY
                        is_key_protected = mp_reader->getAttributes().security_attributes().is_key_protected;
#endif // if HAVE_SECURITY
                        return type_->getKey(get_key_object_, &a_change->instanceHandle, is_key_protected);
                    }

                    logWarning(SUBSCRIBER, "NO KEY in topic: " << topic_name_
                                                               << " and no method to obtain it"; );
                    return false;
                };
    }
}

DataReaderHistory::~DataReaderHistory()
{
    if (type_->m_isGetKeyDefined)
    {
        type_->deleteData(get_key_object_);
    }
}

bool DataReaderHistory::can_change_be_added_nts(
        const GUID_t& writer_guid,
        uint32_t total_payload_size,
        size_t unknown_missing_changes_up_to,
        bool& will_never_be_accepted) const
{
    if (!ReaderHistory::can_change_be_added_nts(writer_guid, total_payload_size, unknown_missing_changes_up_to,
            will_never_be_accepted))
    {
        return false;
    }

    will_never_be_accepted = false;
    return (0 == unknown_missing_changes_up_to) ||
           (m_changes.size() + unknown_missing_changes_up_to < static_cast<size_t>(resource_limited_qos_.max_samples));
}

bool DataReaderHistory::received_change(
        CacheChange_t* change,
        size_t unknown_missing_changes_up_to)
{
    SampleRejectedStatusKind rejection_reason;
    return received_change(change, unknown_missing_changes_up_to, rejection_reason);
}

bool DataReaderHistory::received_change(
        CacheChange_t* a_change,
        size_t unknown_missing_changes_up_to,
        SampleRejectedStatusKind& rejection_reason)
{
    rejection_reason = NOT_REJECTED;

    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(SUBSCRIBER, "You need to create a Reader with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    return receive_fn_(a_change, unknown_missing_changes_up_to, rejection_reason);
}

bool DataReaderHistory::received_change_keep_all(
        CacheChange_t* a_change,
        size_t unknown_missing_changes_up_to,
        SampleRejectedStatusKind& rejection_reason)
{
    if (!compute_key_for_change_fn(a_change))
    {
        // Store the sample temporally only in ReaderHistory. When completed it will be stored in DataReaderHistory too.
        return add_to_reader_history_if_not_full(a_change, rejection_reason);
    }

    bool ret_value = false;
    InstanceCollection::iterator vit;
    if (find_key(a_change->instanceHandle, vit))
    {
        DataReaderInstance::ChangeCollection& instance_changes = vit->second.cache_changes;
        size_t total_size = instance_changes.size() + unknown_missing_changes_up_to;
        if (total_size < static_cast<size_t>(resource_limited_qos_.max_samples_per_instance))
        {
            ret_value =  add_received_change_with_key(a_change, vit->second, rejection_reason);
        }
        else
        {
            logInfo(SUBSCRIBER, "Change not added due to maximum number of samples per instance");
            rejection_reason = REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
        }

    }
    else
    {
        rejection_reason = REJECTED_BY_INSTANCES_LIMIT;
    }

    return ret_value;
}

bool DataReaderHistory::received_change_keep_last(
        CacheChange_t* a_change,
        size_t,
        SampleRejectedStatusKind& rejection_reason)
{
    if (!compute_key_for_change_fn(a_change))
    {
        // Store the sample temporally only in ReaderHistory. When completed it will be stored in SubscriberHistory too.
        return add_to_reader_history_if_not_full(a_change, rejection_reason);
    }

    bool ret_value = false;
    InstanceCollection::iterator vit;
    if (find_key(a_change->instanceHandle, vit))
    {
        DataReaderInstance::ChangeCollection& instance_changes = vit->second.cache_changes;
        if (instance_changes.size() < static_cast<size_t>(history_qos_.depth))
        {
            ret_value = true;
        }
        else
        {
            // Try to substitute the oldest sample.
            CacheChange_t* first_change = instance_changes.at(0);
            if (a_change->sourceTimestamp >= first_change->sourceTimestamp)
            {
                // As the instance is ordered by source timestamp, we can always remove the first one.
                ret_value = remove_change_sub(first_change);
            }
            else
            {
                // Received change is older than oldest, and should be discarded
                rejection_reason = REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
                return true;
            }
        }

        if (ret_value)
        {
            ret_value = add_received_change_with_key(a_change, vit->second, rejection_reason);
        }
    }
    else
    {
        rejection_reason = REJECTED_BY_INSTANCES_LIMIT;
    }

    return ret_value;
}

bool DataReaderHistory::add_received_change_with_key(
        CacheChange_t* a_change,
        DataReaderInstance& instance,
        SampleRejectedStatusKind& rejection_reason)
{
    bool ret_value = add_to_reader_history_if_not_full(a_change, rejection_reason);

    if (ret_value)
    {
        add_to_instance(a_change, instance);
    }

    return ret_value;
}

bool DataReaderHistory::add_to_reader_history_if_not_full(
        CacheChange_t* a_change,
        SampleRejectedStatusKind& rejection_reason)
{
    if (m_isHistoryFull)
    {
        // Discarding the sample.
        logWarning(SUBSCRIBER, "Attempting to add Data to Full ReaderHistory: " << type_name_);
        rejection_reason = REJECTED_BY_SAMPLES_LIMIT;
        return false;
    }

    bool ret_value = add_change(a_change);
    if (m_changes.size() == static_cast<size_t>(m_att.maximumReservedCaches))
    {
        m_isHistoryFull = true;
    }
    return ret_value;
}

void DataReaderHistory::add_to_instance(
        CacheChange_t* a_change,
        DataReaderInstance& instance)
{
    // ADD TO KEY VECTOR
    DataReaderCacheChange item = a_change;
    eprosima::utilities::collections::sorted_vector_insert(instance.cache_changes, item,
            [](const DataReaderCacheChange& lhs, const DataReaderCacheChange& rhs)
            {
                return lhs->sourceTimestamp < rhs->sourceTimestamp;
            });

    logInfo(SUBSCRIBER, mp_reader->getGuid().entityId
            << ": Change " << a_change->sequenceNumber << " added from: "
            << a_change->writerGUID << " with KEY: " << a_change->instanceHandle; );
}

bool DataReaderHistory::get_first_untaken_info(
        SampleInfo& info)
{
    std::lock_guard<RecursiveTimedMutex> lock(*mp_mutex);

    CacheChange_t* change = nullptr;
    WriterProxy* wp = nullptr;
    if (mp_reader->nextUntakenCache(&change, &wp))
    {
        auto it = keyed_changes_.find(change->instanceHandle);
        assert(it != keyed_changes_.end());
        auto& instance_changes = it->second.cache_changes;
        auto item =
                std::find_if(instance_changes.cbegin(), instance_changes.cend(),
                        [change](const DataReaderCacheChange& v)
                        {
                            return v == change;
                        });
        ReadTakeCommand::generate_info(info, it->second, *item);
        mp_reader->change_read_by_user(change, wp, false);
        return true;
    }

    return false;
}

bool DataReaderHistory::find_key(
        const InstanceHandle_t& handle,
        InstanceCollection::iterator& vit_out)
{
    InstanceCollection::iterator vit;
    vit = keyed_changes_.find(handle);
    if (vit != keyed_changes_.end())
    {
        vit_out = vit;
        return true;
    }

    if (keyed_changes_.size() < static_cast<size_t>(resource_limited_qos_.max_instances))
    {
        vit_out = keyed_changes_.emplace(handle,
                        DataReaderInstance{key_changes_allocation_, key_writers_allocation_}).first;
        return true;
    }

    for (vit = keyed_changes_.begin(); vit != keyed_changes_.end(); ++vit)
    {
        if (InstanceStateKind::ALIVE_INSTANCE_STATE != vit->second.instance_state)
        {
            keyed_changes_.erase(vit);
            vit_out = keyed_changes_.emplace(handle,
                            DataReaderInstance{ key_changes_allocation_, key_writers_allocation_ }).first;
            return true;
        }
    }

    logWarning(SUBSCRIBER, "History has reached the maximum number of instances");
    return false;
}

void DataReaderHistory::writer_unmatched(
        const GUID_t& writer_guid,
        const SequenceNumber_t& last_notified_seq)
{
    remove_changes_with_pred(
        [&writer_guid, &last_notified_seq](CacheChange_t* ch)
        {
            return (writer_guid == ch->writerGUID) && (last_notified_seq < ch->sequenceNumber);
        });
}

bool DataReaderHistory::remove_change_sub(
        CacheChange_t* change)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(SUBSCRIBER, "You need to create a Reader with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    bool found = false;
    InstanceCollection::iterator vit;
    if (find_key(change->instanceHandle, vit))
    {
        for (auto chit = vit->second.cache_changes.begin(); chit != vit->second.cache_changes.end(); ++chit)
        {
            if ((*chit)->sequenceNumber == change->sequenceNumber &&
                    (*chit)->writerGUID == change->writerGUID)
            {
                vit->second.cache_changes.erase(chit);
                found = true;
                break;
            }
        }
    }
    if (!found)
    {
        logError(SUBSCRIBER, "Change not found on this key, something is wrong");
    }

    if (remove_change(change))
    {
        m_isHistoryFull = false;
        return true;
    }

    return false;
}

bool DataReaderHistory::remove_change_sub(
        CacheChange_t* change,
        DataReaderInstance::ChangeCollection::iterator& it)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(SUBSCRIBER, "You need to create a Reader with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    bool found = false;
    InstanceCollection::iterator vit;
    if (find_key(change->instanceHandle, vit))
    {
        for (auto chit = vit->second.cache_changes.begin(); chit != vit->second.cache_changes.end(); ++chit)
        {
            if ((*chit)->sequenceNumber == change->sequenceNumber &&
                    (*chit)->writerGUID == change->writerGUID)
            {
                assert(it == chit);
                it = vit->second.cache_changes.erase(chit);
                found = true;
                break;
            }
        }
    }
    if (!found)
    {
        logError(SUBSCRIBER, "Change not found on this key, something is wrong");
    }

    const_iterator chit = find_change_nts(change);
    if (chit == changesEnd())
    {
        logInfo(RTPS_WRITER_HISTORY, "Trying to remove a change not in history");
        return false;
    }

    m_isHistoryFull = false;
    ReaderHistory::remove_change_nts(chit);

    return true;
}

bool DataReaderHistory::set_next_deadline(
        const InstanceHandle_t& handle,
        const std::chrono::steady_clock::time_point& next_deadline_us)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(SUBSCRIBER, "You need to create a Reader with this History before using it");
        return false;
    }
    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    auto it = keyed_changes_.find(handle);
    if (it == keyed_changes_.end())
    {
        return false;
    }

    it->second.next_deadline_us = next_deadline_us;
    return true;
}

bool DataReaderHistory::get_next_deadline(
        InstanceHandle_t& handle,
        std::chrono::steady_clock::time_point& next_deadline_us)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(SUBSCRIBER, "You need to create a Reader with this History before using it");
        return false;
    }
    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    auto min = std::min_element(keyed_changes_.begin(),
                    keyed_changes_.end(),
                    [](
                        const std::pair<InstanceHandle_t, DataReaderInstance>& lhs,
                        const std::pair<InstanceHandle_t, DataReaderInstance>& rhs)
                    {
                        return lhs.second.next_deadline_us < rhs.second.next_deadline_us;
                    });
    handle = min->first;
    next_deadline_us = min->second.next_deadline_us;
    return true;
}

std::pair<bool, DataReaderHistory::instance_info> DataReaderHistory::lookup_instance(
        const InstanceHandle_t& handle,
        bool exact) const
{
    if (!has_keys_)
    {
        if (handle.isDefined())
        {
            // NO_KEY topics can only return the ficticious instance.
            // Execution can only get here for two reasons:
            // - Looking for a specific instance (exact = true)
            // - Looking for the next instance to the ficticious one (exact = false)
            // In both cases, no instance should be returned
            return { false, {InstanceHandle_t(), nullptr} };
        }

        if (exact)
        {
            // Looking for HANDLE_NIL, nothing to return
            return { false, {InstanceHandle_t(), nullptr} };
        }

        // Looking for the first instance, return the ficticious one containing all changes
        InstanceHandle_t tmp;
        tmp.value[0] = 1;
        return { true, {tmp, const_cast<DataReaderInstance*>(&keyed_changes_.begin()->second)} };
    }

    InstanceCollection::const_iterator it;

    if (exact)
    {
        it = keyed_changes_.find(handle);
    }
    else
    {
        auto comp = [](const InstanceHandle_t& h, const std::pair<InstanceHandle_t, DataReaderInstance>& it)
                {
                    return h < it.first;
                };
        it = std::upper_bound(keyed_changes_.begin(), keyed_changes_.end(), handle, comp);
    }

    if (it != keyed_changes_.end())
    {
        return { true, {it->first, const_cast<DataReaderInstance*>(&(it->second))} };
    }
    return { false, {InstanceHandle_t(), nullptr} };
}

void DataReaderHistory::check_and_remove_instance(
        DataReaderHistory::instance_info& instance_info)
{
    DataReaderInstance* instance = instance_info.second;
    if (instance->cache_changes.empty() &&
            (InstanceStateKind::ALIVE_INSTANCE_STATE != instance->instance_state) &&
            instance_info.first.isDefined())
    {
        keyed_changes_.erase(instance_info.first);
        instance_info.second = nullptr;
    }
}

ReaderHistory::iterator DataReaderHistory::remove_change_nts(
        ReaderHistory::const_iterator removal,
        bool release)
{
    if (removal != changesEnd())
    {
        CacheChange_t* p_sample = *removal;

        if (!has_keys_ || p_sample->is_fully_assembled())
        {
            // clean any references to this CacheChange in the key state collection
            auto it = keyed_changes_.find(p_sample->instanceHandle);

            // if keyed and in history must be in the map
            // There is a case when the sample could not be in the keyed map. The first received fragment of a
            // fragmented sample is stored in the history, and when it is completed it is stored in the keyed map.
            // But it can occur it is rejected when the sample is completed and removed without being stored in the
            // keyed map.
            if (it != keyed_changes_.end())
            {
                auto& c = it->second.cache_changes;
                c.erase(std::remove(c.begin(), c.end(), p_sample), c.end());
            }
        }
    }

    // call the base class
    return ReaderHistory::remove_change_nts(removal, release);
}

bool DataReaderHistory::completed_change(
        CacheChange_t* change)
{
    SampleRejectedStatusKind reason;
    return completed_change(change, 0, reason);
}

bool DataReaderHistory::completed_change(
        CacheChange_t* change,
        size_t unknown_missing_changes_up_to,
        SampleRejectedStatusKind& rejection_reason)
{
    bool ret_value = true;
    rejection_reason = NOT_REJECTED;

    if (!change->instanceHandle.isDefined())
    {
        ret_value = false;
        if (compute_key_for_change_fn(change))
        {
            InstanceCollection::iterator vit;
            if (find_key(change->instanceHandle, vit))
            {
                ret_value = !change->instanceHandle.isDefined() ||
                        complete_fn_(change, vit->second, unknown_missing_changes_up_to, rejection_reason);
            }
            else
            {
                rejection_reason = REJECTED_BY_INSTANCES_LIMIT;
            }
        }
    }

    return ret_value;
}

bool DataReaderHistory::completed_change_keep_all(
        CacheChange_t* change,
        DataReaderInstance& instance,
        size_t unknown_missing_changes_up_to,
        SampleRejectedStatusKind& rejection_reason)
{
    bool ret_value = false;
    DataReaderInstance::ChangeCollection& instance_changes = instance.cache_changes;
    if (instance_changes.size() + unknown_missing_changes_up_to <
            static_cast<size_t>(resource_limited_qos_.max_samples_per_instance))
    {
        add_to_instance(change, instance);
        ret_value = true;
    }
    else
    {
        logWarning(SUBSCRIBER, "Change not added due to maximum number of samples per instance");
        rejection_reason = REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
    }

    return ret_value;
}

bool DataReaderHistory::completed_change_keep_last(
        CacheChange_t* change,
        DataReaderInstance& instance,
        size_t,
        SampleRejectedStatusKind& rejection_reason)
{
    bool ret_value = false;
    DataReaderInstance::ChangeCollection& instance_changes = instance.cache_changes;
    if (instance_changes.size() < static_cast<size_t>(history_qos_.depth))
    {
        ret_value = true;
    }
    else
    {
        // Try to substitute the oldest sample.
        CacheChange_t* first_change = instance_changes.at(0);
        if (change->sourceTimestamp >= first_change->sourceTimestamp)
        {
            // As the instance is ordered by source timestamp, we can always remove the first one.
            ret_value = remove_change_sub(first_change);
        }
        else
        {
            // Received change is older than oldest, and should be discarded
            rejection_reason = REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
            return true;
        }
    }

    if (ret_value)
    {
        add_to_instance(change, instance);
    }

    return ret_value;
}

void DataReaderHistory::update_instance_nts(
        CacheChange_t* const change)
{
    InstanceCollection::iterator vit;
    vit = keyed_changes_.find(change->instanceHandle);

    assert(vit != keyed_changes_.end());
    vit->second.update_state(change->kind, change->writerGUID);
    change->reader_info.disposed_generation_count = vit->second.disposed_generation_count;
    change->reader_info.no_writers_generation_count = vit->second.no_writers_generation_count;
}

void DataReaderHistory::writer_not_alive(
        const fastrtps::rtps::GUID_t& writer_guid)
{
    for (auto& it : keyed_changes_)
    {
        it.second.writer_removed(writer_guid);
    }
}

} // namespace detail
} // namsepace dds
} // namespace fastdds
} // namsepace eprosima
