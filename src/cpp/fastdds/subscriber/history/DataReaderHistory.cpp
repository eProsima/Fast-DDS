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
#include <memory>
#include <mutex>

#include "DataReaderHistory.hpp"

#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>

#include <fastdds/subscriber/DataReaderImpl/ReadTakeCommand.hpp>

#include <rtps/common/ChangeComparison.hpp>
#include <rtps/reader/BaseReader.hpp>
#include <rtps/reader/WriterProxy.h>
#include <utils/collections/sorted_vector_insert.hpp>

using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

using fastdds::RecursiveTimedMutex;

static HistoryAttributes to_history_attributes(
        const TypeSupport& type,
        const DataReaderQos& qos)
{
    auto initial_samples = qos.resource_limits().allocated_samples;
    auto max_samples = qos.resource_limits().max_samples;

    auto mempolicy = qos.endpoint().history_memory_policy;
    auto payloadMaxSize = type->max_serialized_type_size + 3; // possible alignment

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
    , has_keys_(type->is_compute_key_provided)
    , type_(type.get())
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

    if (type_->is_compute_key_provided)
    {
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

        instances_.emplace(c_InstanceHandle_Unknown,
                std::make_shared<DataReaderInstance>(key_changes_allocation_, key_writers_allocation_));
        data_available_instances_[c_InstanceHandle_Unknown] = instances_[c_InstanceHandle_Unknown];
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
        compute_key_for_change_fn_ = [](CacheChange_t* change)
                {
                    change->instanceHandle = c_InstanceHandle_Unknown;
                    return true;
                };
    }
    else
    {
        compute_key_for_change_fn_ =
                [this](CacheChange_t* a_change)
                {
                    if (!a_change->is_fully_assembled())
                    {
                        return false;
                    }

                    if (a_change->instanceHandle.isDefined())
                    {
                        return true;
                    }

                    if (type_ != nullptr)
                    {
                        EPROSIMA_LOG_INFO(SUBSCRIBER, "Getting Key of change with no Key transmitted");
                        bool is_key_protected = false;
#if HAVE_SECURITY
                        is_key_protected = mp_reader->getAttributes().security_attributes().is_key_protected;
#endif // if HAVE_SECURITY
                        return type_->compute_key(a_change->serializedPayload, a_change->instanceHandle,
                                       is_key_protected);
                    }

                    EPROSIMA_LOG_WARNING(SUBSCRIBER, "NO KEY in topic: " << topic_name_
                                                                         << " and no method to obtain it"; );
                    return false;
                };
    }
}

DataReaderHistory::~DataReaderHistory()
{
}

bool DataReaderHistory::can_change_be_added_nts(
        const GUID_t& writer_guid,
        uint32_t total_payload_size,
        size_t unknown_missing_changes_up_to,
        bool& will_never_be_accepted) const
{
    return ReaderHistory::can_change_be_added_nts(writer_guid, total_payload_size, unknown_missing_changes_up_to,
                   will_never_be_accepted);
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
    bool ret_value = false;
    rejection_reason = NOT_REJECTED;

    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(SUBSCRIBER, "You need to create a Reader with this History before using it");
        return false;
    }

    if ((0 == unknown_missing_changes_up_to) ||
            (m_changes.size() + unknown_missing_changes_up_to < static_cast<size_t>(resource_limited_qos_.max_samples)))
    {
        std::lock_guard<RecursiveTimedMutex> guard(*getMutex());
        ret_value =  receive_fn_(a_change, unknown_missing_changes_up_to, rejection_reason);
    }
    else
    {
        rejection_reason = REJECTED_BY_SAMPLES_LIMIT;
    }

    return ret_value;
}

bool DataReaderHistory::received_change_keep_all(
        CacheChange_t* a_change,
        size_t unknown_missing_changes_up_to,
        SampleRejectedStatusKind& rejection_reason)
{
    if (!compute_key_for_change_fn_(a_change))
    {
        // Store the sample temporally only in ReaderHistory. When completed it will be stored in DataReaderHistory too.
        return add_to_reader_history_if_not_full(a_change, rejection_reason);
    }

    bool ret_value = false;
    InstanceCollection::iterator vit;
    if (find_key(a_change->instanceHandle, vit))
    {
        DataReaderInstance::ChangeCollection& instance_changes = vit->second->cache_changes;
        size_t total_size = instance_changes.size() + unknown_missing_changes_up_to;
        if (total_size < static_cast<size_t>(resource_limited_qos_.max_samples_per_instance))
        {
            ret_value =  add_received_change_with_key(a_change, *vit->second, rejection_reason);
        }
        else
        {
            EPROSIMA_LOG_INFO(SUBSCRIBER, "Change not added due to maximum number of samples per instance");
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
    if (!compute_key_for_change_fn_(a_change))
    {
        // Store the sample temporally only in ReaderHistory. When completed it will be stored in SubscriberHistory too.
        return add_to_reader_history_if_not_full(a_change, rejection_reason);
    }

    bool ret_value = false;
    InstanceCollection::iterator vit;
    if (find_key(a_change->instanceHandle, vit))
    {
        DataReaderInstance::ChangeCollection& instance_changes = vit->second->cache_changes;
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
                return true;
            }
        }

        if (ret_value)
        {
            ret_value = add_received_change_with_key(a_change, *vit->second, rejection_reason);
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
        EPROSIMA_LOG_WARNING(SUBSCRIBER, "Attempting to add Data to Full ReaderHistory: " << type_name_);
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
    eprosima::utilities::collections::sorted_vector_insert(instance.cache_changes, item, rtps::history_order_cmp);
    data_available_instances_[a_change->instanceHandle] = instances_[a_change->instanceHandle];

    EPROSIMA_LOG_INFO(SUBSCRIBER, mp_reader->getGuid().entityId
            << ": Change " << a_change->sequenceNumber << " added from: "
            << a_change->writerGUID << " with KEY: " << a_change->instanceHandle; );
}

bool DataReaderHistory::get_first_untaken_info(
        SampleInfo& info)
{
    std::lock_guard<RecursiveTimedMutex> lock(*getMutex());

    for (auto& it : data_available_instances_)
    {
        auto& instance_changes = it.second->cache_changes;
        for (auto& instance_change : instance_changes)
        {
            WriterProxy* wp = nullptr;
            bool is_future_change = false;

            auto base_reader = rtps::BaseReader::downcast(mp_reader);
            if (base_reader->begin_sample_access_nts(instance_change, wp, is_future_change))
            {
                base_reader->end_sample_access_nts(instance_change, wp, false);
                if (is_future_change)
                {
                    continue;
                }
            }

            ReadTakeCommand::generate_info(info, *(it.second), instance_change);
            return true;
        }
    }

    return false;
}

bool DataReaderHistory::find_key(
        const InstanceHandle_t& handle,
        InstanceCollection::iterator& vit_out)
{
    InstanceCollection::iterator vit;
    vit = instances_.find(handle);
    if (vit != instances_.end())
    {
        vit_out = vit;
        return true;
    }

    if (instances_.size() < static_cast<size_t>(resource_limited_qos_.max_instances))
    {
        vit_out = instances_.emplace(handle,
                        std::make_shared<DataReaderInstance>(key_changes_allocation_, key_writers_allocation_)).first;
        return true;
    }

    for (vit = instances_.begin(); vit != instances_.end(); ++vit)
    {
        if (InstanceStateKind::ALIVE_INSTANCE_STATE != vit->second->instance_state)
        {
            data_available_instances_.erase(vit->first);
            instances_.erase(vit);
            vit_out = instances_.emplace(handle,
                            std::make_shared<DataReaderInstance>(key_changes_allocation_,
                            key_writers_allocation_)).first;
            return true;
        }
    }

    EPROSIMA_LOG_WARNING(SUBSCRIBER, "History has reached the maximum number of instances");
    return false;
}

void DataReaderHistory::writer_unmatched(
        const GUID_t& writer_guid,
        const SequenceNumber_t& last_notified_seq)
{
    // Remove all future changes from the unmatched writer
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
        EPROSIMA_LOG_ERROR(SUBSCRIBER, "You need to create a Reader with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*getMutex());
    bool found = false;
    InstanceCollection::iterator vit;
    if (find_key(change->instanceHandle, vit))
    {
        for (auto chit = vit->second->cache_changes.begin(); chit != vit->second->cache_changes.end(); ++chit)
        {
            if ((*chit)->sequenceNumber == change->sequenceNumber &&
                    (*chit)->writerGUID == change->writerGUID)
            {
                vit->second->cache_changes.erase(chit);
                found = true;

                if (change->isRead)
                {
                    --counters_.samples_read;
                }
                break;
            }
        }
    }
    if (!found)
    {
        EPROSIMA_LOG_ERROR(SUBSCRIBER, "Change not found on this key, something is wrong");
    }

    if (remove_change(change))
    {
        m_isHistoryFull = false;
        counters_.samples_unread = mp_reader->get_unread_count();
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
        EPROSIMA_LOG_ERROR(SUBSCRIBER, "You need to create a Reader with this History before using it");
        return false;
    }

    CacheChange_t dummy_change;
    dummy_change.instanceHandle = change->instanceHandle;
    dummy_change.isRead = change->isRead;
    dummy_change.sequenceNumber = change->sequenceNumber;
    dummy_change.writerGUID = change->writerGUID;

    std::lock_guard<RecursiveTimedMutex> guard(*getMutex());

    const_iterator chit = find_change_nts(change);
    if (chit == changesEnd())
    {
        EPROSIMA_LOG_INFO(RTPS_WRITER_HISTORY, "Trying to remove a change not in history");
        return false;
    }

    auto new_it = ReaderHistory::remove_change_nts(chit);

    if (new_it == changesEnd() || !matches_change(&dummy_change, *new_it)) // Change was successfully removed.
    {
        InstanceCollection::iterator vit;
        if (find_key(dummy_change.instanceHandle, vit))
        {
            auto in_it = std::find(vit->second->cache_changes.begin(), vit->second->cache_changes.end(), change);

            if (vit->second->cache_changes.end() != in_it)
            {
                assert(it == in_it);
                it = vit->second->cache_changes.erase(in_it);
                if (dummy_change.isRead)
                {
                    --counters_.samples_read;
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(SUBSCRIBER, "Change not found on this key, something is wrong");
            }
        }

        m_isHistoryFull = false;
        counters_.samples_unread = mp_reader->get_unread_count();
        return true;
    }

    return false;
}

bool DataReaderHistory::set_next_deadline(
        const InstanceHandle_t& handle,
        const std::chrono::steady_clock::time_point& next_deadline_us,
        bool deadline_missed)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(SUBSCRIBER, "You need to create a Reader with this History before using it");
        return false;
    }
    std::lock_guard<RecursiveTimedMutex> guard(*getMutex());
    auto it = instances_.find(handle);
    if (it == instances_.end())
    {
        return false;
    }

    if (deadline_missed)
    {
        it->second->deadline_missed();
    }
    it->second->next_deadline_us = next_deadline_us;
    return true;
}

bool DataReaderHistory::get_next_deadline(
        InstanceHandle_t& handle,
        std::chrono::steady_clock::time_point& next_deadline_us)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(SUBSCRIBER, "You need to create a Reader with this History before using it");
        return false;
    }
    std::lock_guard<RecursiveTimedMutex> guard(*getMutex());
    auto min = std::min_element(instances_.begin(),
                    instances_.end(),
                    [](
                        const InstanceCollection::value_type& lhs,
                        const InstanceCollection::value_type& rhs)
                    {
                        return lhs.second->next_deadline_us < rhs.second->next_deadline_us;
                    });
    handle = min->first;
    next_deadline_us = min->second->next_deadline_us;
    return true;
}

uint64_t DataReaderHistory::get_unread_count(
        bool mark_as_read)
{
    std::lock_guard<RecursiveTimedMutex> guard(*getMutex());
    uint64_t ret_val = mp_reader->get_unread_count(mark_as_read);
    assert(ret_val == counters_.samples_unread);
    if (mark_as_read)
    {
        counters_.samples_read += ret_val;
        counters_.samples_unread = 0;
    }
    return ret_val;
}

bool DataReaderHistory::is_instance_present(
        const InstanceHandle_t& handle) const
{
    return has_keys_ && instances_.find(handle) != instances_.end();
}

std::pair<bool, DataReaderHistory::instance_info> DataReaderHistory::lookup_available_instance(
        const InstanceHandle_t& handle,
        bool exact)
{
    InstanceCollection::iterator it = data_available_instances_.end();

    if (!has_keys_)
    {
        // NO_KEY topics can only return the fictitious instance.
        // Execution can only get here for two reasons:
        // - Looking for a specific instance (exact = true)
        // - Looking for the next instance to the ficticious one (exact = false)
        // In both cases, no instance should be returned
        if (!handle.isDefined() && !exact)
        {
            // Looking for the first instance, return the ficticious one containing all changes
            it = data_available_instances_.begin();
        }
    }
    else
    {
        if (exact)
        {
            // Looking for a specific instance on a topic with key
            it = data_available_instances_.find(handle);
        }
        else
        {
            if (!handle.isDefined())
            {
                // Looking for the first instance on a topic with key
                it = data_available_instances_.begin();
            }
            else
            {
                // Looking for an instance with a handle greater than the one on the input
                auto comp = [](const InstanceHandle_t& h, const InstanceCollection::value_type& it)
                        {
                            return h < it.first;
                        };
                it = std::upper_bound(data_available_instances_.begin(), data_available_instances_.end(), handle, comp);
            }
        }
    }

    return { it != data_available_instances_.end(), it };
}

std::pair<bool, DataReaderHistory::instance_info> DataReaderHistory::next_available_instance_nts(
        const InstanceHandle_t& handle,
        const DataReaderHistory::instance_info& current_info)
{
    if (current_info == data_available_instances_.end())
    {
        return { false, current_info };
    }
    instance_info it = current_info;
    if (it->first == handle)
    {
        ++it;
    }

    return { it != data_available_instances_.end(), it };
}

void DataReaderHistory::check_and_remove_instance(
        DataReaderHistory::instance_info& instance_info)
{
    DataReaderInstance* instance = instance_info->second.get();

    if (instance->cache_changes.empty())
    {
        if (InstanceStateKind::ALIVE_INSTANCE_STATE != instance->instance_state &&
                instance->alive_writers.empty() &&
                instance_info->first.isDefined())
        {
            instances_.erase(instance_info->first);
        }

        instance_info = data_available_instances_.erase(instance_info);
    }
}

ReaderHistory::iterator DataReaderHistory::remove_change_nts(
        ReaderHistory::const_iterator removal,
        bool release)
{
    if (removal != changesEnd())
    {
        CacheChange_t* change_ptr = *removal;
        CacheChange_t dummy_change;
        bool is_fully_assembled = (*removal)->is_fully_assembled();
        dummy_change.instanceHandle = (*removal)->instanceHandle;
        dummy_change.isRead = (*removal)->isRead;
        dummy_change.sequenceNumber = (*removal)->sequenceNumber;
        dummy_change.writerGUID = (*removal)->writerGUID;

        // call the base class
        auto ret_val = ReaderHistory::remove_change_nts(removal, release);

        if (ret_val == changesEnd() || !matches_change(&dummy_change, *ret_val)) // Change was successfully removed.
        {
            if (!has_keys_ || is_fully_assembled)
            {
                // clean any references to this CacheChange in the key state collection
                auto it = instances_.find(dummy_change.instanceHandle);

                // if keyed and in history must be in the map
                // There is a case when the sample could not be in the keyed map. The first received fragment of a
                // fragmented sample is stored in the history, and when it is completed it is stored in the keyed map.
                // But it can occur it is rejected when the sample is completed and removed without being stored in the
                // keyed map.
                if (it != instances_.end())
                {
                    it->second->cache_changes.remove(change_ptr);
                    if (dummy_change.isRead)
                    {
                        --counters_.samples_read;
                    }
                }
            }

            counters_.samples_unread = mp_reader->get_unread_count();
            return ret_val;
        }

        return remove_iterator_constness(removal);
    }

    return changesEnd();
}

ReaderHistory::iterator DataReaderHistory::remove_change_nts(
        ReaderHistory::const_iterator removal,
        const std::chrono::time_point<std::chrono::steady_clock>&,
        bool release)
{
    return DataReaderHistory::remove_change_nts(removal, release);
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
    bool ret_value = false;
    rejection_reason = REJECTED_BY_INSTANCES_LIMIT;

    if (compute_key_for_change_fn_(change))
    {
        InstanceCollection::iterator vit;
        if (find_key(change->instanceHandle, vit))
        {
            ret_value = !change->instanceHandle.isDefined() ||
                    complete_fn_(change, *vit->second, unknown_missing_changes_up_to, rejection_reason);
        }
    }

    if (ret_value)
    {
        rejection_reason = NOT_REJECTED;
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
        EPROSIMA_LOG_WARNING(SUBSCRIBER, "Change not added due to maximum number of samples per instance");
        rejection_reason = REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
    }

    return ret_value;
}

bool DataReaderHistory::completed_change_keep_last(
        CacheChange_t* change,
        DataReaderInstance& instance,
        size_t,
        SampleRejectedStatusKind&)
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
            return true;
        }
    }

    if (ret_value)
    {
        add_to_instance(change, instance);
    }

    return ret_value;
}

void DataReaderHistory::change_was_processed_nts(
        CacheChange_t* const change,
        bool is_going_to_be_mark_as_read)
{
    if (!change->isRead && is_going_to_be_mark_as_read)
    {
        ++counters_.samples_read;
        --counters_.samples_unread;
    }
}

void DataReaderHistory::instance_viewed_nts(
        const InstanceCollection::mapped_type& instance)
{
    if (ViewStateKind::NEW_VIEW_STATE == instance->view_state)
    {
        instance->view_state = ViewStateKind::NOT_NEW_VIEW_STATE;
        --counters_.instances_new;
        ++counters_.instances_not_new;
    }
}

bool DataReaderHistory::update_instance_nts(
        CacheChange_t* const change)
{
    InstanceCollection::iterator vit;
    vit = instances_.find(change->instanceHandle);

    assert(vit != instances_.end());
    assert(false == change->isRead);
    auto previous_owner = vit->second->current_owner.first;
    ++counters_.samples_unread;
    bool ret =
            vit->second->update_state(counters_, change->kind, change->writerGUID,
                    change->reader_info.writer_ownership_strength);
    change->reader_info.disposed_generation_count = vit->second->disposed_generation_count;
    change->reader_info.no_writers_generation_count = vit->second->no_writers_generation_count;

    auto current_owner = vit->second->current_owner.first;
    if ((current_owner != previous_owner) && (current_owner == change->writerGUID))
    {
        // Remove all changes from different owners after the change.
        DataReaderInstance::ChangeCollection& changes = vit->second->cache_changes;
        auto it = std::lower_bound(changes.begin(), changes.end(), change, rtps::history_order_cmp);
        assert(it != changes.end());
        assert(*it == change);
        ++it;
        while (it != changes.end())
        {
            if ((*it)->writerGUID != current_owner)
            {
                // Remove from history
                remove_change_sub(*it, it);

                // Current iterator will point to change next to the one removed. Avoid incrementing.
                continue;
            }
            ++it;
        }
    }

    return ret;
}

void DataReaderHistory::writer_not_alive(
        const GUID_t& writer_guid)
{
    for (auto& it : instances_)
    {
        it.second->writer_removed(counters_, writer_guid);
    }
}

StateFilter DataReaderHistory::get_mask_status() const noexcept
{
    std::lock_guard<RecursiveTimedMutex> guard(*getMutex());

    return {
        static_cast<SampleStateMask>(
            (counters_.samples_read ? READ_SAMPLE_STATE : 0) |
            (counters_.samples_unread ? NOT_READ_SAMPLE_STATE : 0)),
        static_cast<ViewStateMask>(
            (counters_.instances_not_new ? NOT_NEW_VIEW_STATE : 0) |
            (counters_.instances_new ? NEW_VIEW_STATE : 0)),
        static_cast<InstanceStateMask>(
            (counters_.instances_alive ? ALIVE_INSTANCE_STATE : 0) |
            (counters_.instances_disposed ? NOT_ALIVE_DISPOSED_INSTANCE_STATE : 0) |
            (counters_.instances_no_writers ? NOT_ALIVE_NO_WRITERS_INSTANCE_STATE : 0))
    };
}

void DataReaderHistory::writer_update_its_ownership_strength_nts(
        const GUID_t& writer_guid,
        const uint32_t ownership_strength)
{
    for (auto& instance : instances_)
    {
        instance.second->writer_update_its_ownership_strength(writer_guid, ownership_strength);
    }
}

} // namespace detail
} // namsepace dds
} // namespace fastdds
} // namsepace eprosima
