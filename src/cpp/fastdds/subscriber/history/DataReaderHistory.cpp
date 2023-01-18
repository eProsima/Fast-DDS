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

    if (qos.history().kind != KEEP_ALL_HISTORY_QOS)
    {
        max_samples = qos.history().depth;
        if (type->m_isGetKeyDefined)
        {
            max_samples *= qos.resource_limits().max_instances;
        }

        initial_samples = std::min(initial_samples, max_samples);
    }

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

        instances_.emplace(c_InstanceHandle_Unknown,
                std::make_shared<DataReaderInstance>(key_changes_allocation_, key_writers_allocation_));
        data_available_instances_[c_InstanceHandle_Unknown] = instances_[c_InstanceHandle_Unknown];
    }

    using std::placeholders::_1;
    using std::placeholders::_2;

    receive_fn_ = qos.history().kind == KEEP_ALL_HISTORY_QOS ?
            std::bind(&DataReaderHistory::received_change_keep_all, this, _1, _2) :
            std::bind(&DataReaderHistory::received_change_keep_last, this, _1, _2);

    complete_fn_ = qos.history().kind == KEEP_ALL_HISTORY_QOS ?
            std::bind(&DataReaderHistory::completed_change_keep_all, this, _1, _2) :
            std::bind(&DataReaderHistory::completed_change_keep_last, this, _1, _2);

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
        CacheChange_t* a_change,
        size_t unknown_missing_changes_up_to)
{
    if (mp_reader == nullptr || mp_mutex == nullptr)
    {
        logError(SUBSCRIBER, "You need to create a Reader with this History before using it");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    return receive_fn_(a_change, unknown_missing_changes_up_to);
}

bool DataReaderHistory::received_change_keep_all(
        CacheChange_t* a_change,
        size_t unknown_missing_changes_up_to)
{
    if (!compute_key_for_change_fn_(a_change))
    {
        // Store the sample temporally only in ReaderHistory. When completed it will be stored in SubscriberHistory too.
        return add_to_reader_history_if_not_full(a_change);
    }

    InstanceCollection::iterator vit;
    if (find_key(a_change->instanceHandle, vit))
    {
        DataReaderInstance::ChangeCollection& instance_changes = vit->second->cache_changes;
        size_t total_size = instance_changes.size() + unknown_missing_changes_up_to;
        if (total_size < static_cast<size_t>(resource_limited_qos_.max_samples_per_instance))
        {
            return add_received_change_with_key(a_change, *vit->second);
        }

        logInfo(SUBSCRIBER, "Change not added due to maximum number of samples per instance");
    }

    return false;
}

bool DataReaderHistory::received_change_keep_last(
        CacheChange_t* a_change,
        size_t /* unknown_missing_changes_up_to */)
{
    if (!compute_key_for_change_fn_(a_change))
    {
        // Store the sample temporally only in ReaderHistory. When completed it will be stored in SubscriberHistory too.
        return add_to_reader_history_if_not_full(a_change);
    }

    InstanceCollection::iterator vit;
    if (find_key(a_change->instanceHandle, vit))
    {
        bool add = false;
        DataReaderInstance::ChangeCollection& instance_changes = vit->second->cache_changes;
        if (instance_changes.size() < static_cast<size_t>(history_qos_.depth))
        {
            add = true;
        }
        else
        {
            // Try to substitute the oldest sample.
            CacheChange_t* first_change = instance_changes.at(0);
            if (a_change->sourceTimestamp < first_change->sourceTimestamp)
            {
                // Received change is older than oldest, and should be discarded
                return true;
            }

            // As the instance is ordered by source timestamp, we can always remove the first one.
            add = remove_change_sub(first_change);
        }

        if (add)
        {
            return add_received_change_with_key(a_change, *vit->second);
        }
    }

    return false;
}

bool DataReaderHistory::add_received_change_with_key(
        CacheChange_t* a_change,
        DataReaderInstance& instance)
{
    if (add_to_reader_history_if_not_full(a_change))
    {
        add_to_instance(a_change, instance);
        return true;
    }

    return false;
}

bool DataReaderHistory::add_to_reader_history_if_not_full(
        CacheChange_t* a_change)
{
    if (m_isHistoryFull)
    {
        // Discarding the sample.
        logWarning(SUBSCRIBER, "Attempting to add Data to Full ReaderHistory: " << type_name_);
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
    data_available_instances_[a_change->instanceHandle] = instances_[a_change->instanceHandle];

    logInfo(SUBSCRIBER, mp_reader->getGuid().entityId
            << ": Change " << a_change->sequenceNumber << " added from: "
            << a_change->writerGUID << " with KEY: " << a_change->instanceHandle; );
}

bool DataReaderHistory::get_first_untaken_info(
        SampleInfo& info)
{
    std::lock_guard<RecursiveTimedMutex> lock(*mp_mutex);

    for (auto& it : data_available_instances_)
    {
        auto& instance_changes = it.second->cache_changes;
        if (!instance_changes.empty())
        {
            ReadTakeCommand::generate_info(info, *(it.second), instance_changes.front());
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
        if (vit->second->cache_changes.size() == 0)
        {
            data_available_instances_.erase(vit->first);
            instances_.erase(vit);
            vit_out = instances_.emplace(handle,
                            std::make_shared<DataReaderInstance>(key_changes_allocation_,
                            key_writers_allocation_)).first;
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
        for (auto chit = vit->second->cache_changes.begin(); chit != vit->second->cache_changes.end(); ++chit)
        {
            if ((*chit)->sequenceNumber == change->sequenceNumber &&
                    (*chit)->writerGUID == change->writerGUID)
            {
                vit->second->cache_changes.erase(chit);
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
        for (auto chit = vit->second->cache_changes.begin(); chit != vit->second->cache_changes.end(); ++chit)
        {
            if ((*chit)->sequenceNumber == change->sequenceNumber &&
                    (*chit)->writerGUID == change->writerGUID)
            {
                assert(it == chit);
                it = vit->second->cache_changes.erase(chit);
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
    auto it = instances_.find(handle);
    if (it == instances_.end())
    {
        return false;
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
        logError(SUBSCRIBER, "You need to create a Reader with this History before using it");
        return false;
    }
    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
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
    return mp_reader->get_unread_count(mark_as_read);
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
        // NO_KEY topics can only return the ficticious instance.
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
    if (instance_info->second->cache_changes.empty())
    {
        if ((InstanceStateKind::ALIVE_INSTANCE_STATE != instance_info->second->instance_state) &&
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
        CacheChange_t* p_sample = *removal;

        if (!has_keys_ || p_sample->is_fully_assembled())
        {
            // clean any references to this CacheChange in the key state collection
            auto it = instances_.find(p_sample->instanceHandle);

            // if keyed and in history must be in the map
            assert(it != instances_.end());

            auto& c = it->second->cache_changes;
            c.erase(std::remove(c.begin(), c.end(), p_sample), c.end());
        }
    }

    // call the base class
    return ReaderHistory::remove_change_nts(removal, release);
}

bool DataReaderHistory::completed_change(
        CacheChange_t* change)
{
    bool ret_value = true;

    if (!change->instanceHandle.isDefined())
    {
        InstanceCollection::iterator vit;
        ret_value = compute_key_for_change_fn_(change) && find_key(change->instanceHandle, vit);
        if (ret_value)
        {
            ret_value = !change->instanceHandle.isDefined() || complete_fn_(change, *vit->second);
        }

        if (!ret_value)
        {
            const_iterator chit = find_change_nts(change);
            if (chit != changesEnd())
            {
                m_isHistoryFull = false;
                remove_change_nts(chit);
            }
            else
            {
                logError(SUBSCRIBER, "Change should exist but didn't find it");
            }
        }
    }

    return ret_value;
}

bool DataReaderHistory::completed_change_keep_all(
        CacheChange_t* change,
        DataReaderInstance& instance)
{
    DataReaderInstance::ChangeCollection& instance_changes = instance.cache_changes;
    if (instance_changes.size() < static_cast<size_t>(resource_limited_qos_.max_samples_per_instance))
    {
        add_to_instance(change, instance);
        return true;
    }

    logWarning(SUBSCRIBER, "Change not added due to maximum number of samples per instance");
    return false;
}

bool DataReaderHistory::completed_change_keep_last(
        CacheChange_t* change,
        DataReaderInstance& instance)
{
    bool add = false;
    DataReaderInstance::ChangeCollection& instance_changes = instance.cache_changes;
    if (instance_changes.size() < static_cast<size_t>(history_qos_.depth))
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
        add_to_instance(change, instance);
        return true;
    }

    return false;
}

void DataReaderHistory::update_instance_nts(
        CacheChange_t* const change)
{
    InstanceCollection::iterator vit;
    vit = instances_.find(change->instanceHandle);

    assert(vit != instances_.end());
    vit->second->update_state(change->kind, change->writerGUID);
    change->reader_info.disposed_generation_count = vit->second->disposed_generation_count;
    change->reader_info.no_writers_generation_count = vit->second->no_writers_generation_count;
}

void DataReaderHistory::writer_not_alive(
        const fastrtps::rtps::GUID_t& writer_guid)
{
    for (auto& it : instances_)
    {
        it.second->writer_removed(writer_guid);
    }
}

} // namespace detail
} // namsepace dds
} // namespace fastdds
} // namsepace eprosima
