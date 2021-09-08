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
    , history_qos_(qos.history())
    , resource_limited_qos_(qos.resource_limits())
    , topic_name_(topic.get_name())
    , type_name_(topic.get_type_name())
    , has_keys_(type->m_isGetKeyDefined)
    , type_(type.get())
    , get_key_object_(nullptr)
{
    if (type_->m_isGetKeyDefined)
    {
        get_key_object_ = type_->createData();
    }

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

    using std::placeholders::_1;
    using std::placeholders::_2;

    if (!has_keys_)
    {
        receive_fn_ = qos.history().kind == KEEP_ALL_HISTORY_QOS ?
                std::bind(&DataReaderHistory::received_change_keep_all_no_key, this, _1, _2) :
                std::bind(&DataReaderHistory::received_change_keep_last_no_key, this, _1, _2);
    }
    else
    {
        receive_fn_ = qos.history().kind == KEEP_ALL_HISTORY_QOS ?
                std::bind(&DataReaderHistory::received_change_keep_all_with_key, this, _1, _2) :
                std::bind(&DataReaderHistory::received_change_keep_last_with_key, this, _1, _2);
    }
}

DataReaderHistory::~DataReaderHistory()
{
    if (type_->m_isGetKeyDefined)
    {
        type_->deleteData(get_key_object_);
    }
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

bool DataReaderHistory::received_change_keep_all_no_key(
        CacheChange_t* a_change,
        size_t unknown_missing_changes_up_to)
{
    // TODO(Ricardo) Check
    if (m_changes.size() + unknown_missing_changes_up_to < static_cast<size_t>(resource_limited_qos_.max_samples))
    {
        return add_received_change(a_change);
    }

    return false;
}

bool DataReaderHistory::received_change_keep_last_no_key(
        CacheChange_t* a_change,
        size_t /* unknown_missing_changes_up_to */ )
{
    bool add = false;
    if (m_changes.size() < static_cast<size_t>(history_qos_.depth))
    {
        add = true;
    }
    else
    {
        // Try to substitute the oldest sample.
        CacheChange_t* first_change = m_changes.at(0);
        if (a_change->sourceTimestamp < first_change->sourceTimestamp)
        {
            // Received change is older than oldest, and should be discarded
            return true;
        }

        // As the history is ordered by source timestamp, we can always remove the first one.
        add = remove_change_sub(first_change);
    }

    if (add)
    {
        return add_received_change(a_change);
    }

    return false;
}

bool DataReaderHistory::received_change_keep_all_with_key(
        CacheChange_t* a_change,
        size_t unknown_missing_changes_up_to)
{
    InstanceCollection::iterator vit;
    if (find_key_for_change(a_change, vit))
    {
        DataReaderInstance::ChangeCollection& instance_changes = vit->second.cache_changes;
        size_t total_size = instance_changes.size() + unknown_missing_changes_up_to;
        if (total_size < static_cast<size_t>(resource_limited_qos_.max_samples_per_instance))
        {
            return add_received_change_with_key(a_change, vit->second);
        }

        logInfo(SUBSCRIBER, "Change not added due to maximum number of samples per instance");
    }

    return false;
}

bool DataReaderHistory::received_change_keep_last_with_key(
        CacheChange_t* a_change,
        size_t /* unknown_missing_changes_up_to */)
{
    InstanceCollection::iterator vit;
    if (find_key_for_change(a_change, vit))
    {
        bool add = false;
        DataReaderInstance::ChangeCollection& instance_changes = vit->second.cache_changes;
        if (instance_changes.size() < static_cast<size_t>(history_qos_.depth))
        {
            add = true;
        }
        else
        {
            // Try to substitute the oldest sample.
            CacheChange_t* first_change = instance_changes.at(0).change;
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
            return add_received_change_with_key(a_change, vit->second);
        }
    }

    return false;
}

bool DataReaderHistory::add_received_change(
        CacheChange_t* a_change)
{
    return add_received_change_with_key(a_change, keyed_changes_[c_InstanceHandle_Unknown]);
}

bool DataReaderHistory::add_received_change_with_key(
        CacheChange_t* a_change,
        DataReaderInstance& instance)
{
    if (m_isHistoryFull)
    {
        // Discarting the sample.
        logWarning(SUBSCRIBER, "Attempting to add Data to Full ReaderHistory: " << type_name_);
        return false;
    }

    if (add_change(a_change))
    {
        if (m_changes.size() == static_cast<size_t>(m_att.maximumReservedCaches))
        {
            m_isHistoryFull = true;
        }

        //ADD TO KEY VECTOR
        DataReaderCacheChange item{ a_change, instance.disposed_generation_count,
                                    instance.no_writers_generation_count };
        eprosima::utilities::collections::sorted_vector_insert(instance.cache_changes, item,
                [](const DataReaderCacheChange& lhs, const DataReaderCacheChange& rhs)
                {
                    return lhs.change->sourceTimestamp < rhs.change->sourceTimestamp;
                });

        logInfo(SUBSCRIBER, mp_reader->getGuid().entityId
                << ": Change " << a_change->sequenceNumber << " added from: "
                << a_change->writerGUID << " with KEY: " << a_change->instanceHandle; );

        return true;
    }

    return false;
}

bool DataReaderHistory::find_key_for_change(
        CacheChange_t* a_change,
        InstanceCollection::iterator& map_it)
{
    if (!a_change->instanceHandle.isDefined() && type_ != nullptr)
    {
        logInfo(SUBSCRIBER, "Getting Key of change with no Key transmitted");
        type_->deserialize(&a_change->serializedPayload, get_key_object_);
        bool is_key_protected = false;
#if HAVE_SECURITY
        is_key_protected = mp_reader->getAttributes().security_attributes().is_key_protected;
#endif // if HAVE_SECURITY
        if (!type_->getKey(get_key_object_, &a_change->instanceHandle, is_key_protected))
        {
            return false;
        }
    }
    else if (!a_change->instanceHandle.isDefined())
    {
        logWarning(SUBSCRIBER, "NO KEY in topic: " << topic_name_
                                                   << " and no method to obtain it"; );
        return false;
    }

    return find_key(a_change, &map_it);
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
                            return v.change == change;
                        });
        ReadTakeCommand::generate_info(info, it->second, *item);
        mp_reader->change_read_by_user(change, wp, false);
        return true;
    }

    return false;
}

bool DataReaderHistory::find_key(
        CacheChange_t* a_change,
        InstanceCollection::iterator* vit_out)
{
    InstanceCollection::iterator vit;
    vit = keyed_changes_.find(a_change->instanceHandle);
    if (vit != keyed_changes_.end())
    {
        *vit_out = vit;
        return true;
    }

    if (keyed_changes_.size() < static_cast<size_t>(resource_limited_qos_.max_instances))
    {
        *vit_out = keyed_changes_.insert(std::make_pair(a_change->instanceHandle, DataReaderInstance())).first;
        return true;
    }
    else
    {
        for (vit = keyed_changes_.begin(); vit != keyed_changes_.end(); ++vit)
        {
            if (vit->second.cache_changes.size() == 0)
            {
                keyed_changes_.erase(vit);
                *vit_out = keyed_changes_.insert(std::make_pair(a_change->instanceHandle, DataReaderInstance())).first;
                return true;
            }
        }
        logWarning(SUBSCRIBER, "History has reached the maximum number of instances");
    }
    return false;
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
    if (find_key(change, &vit))
    {
        for (auto chit = vit->second.cache_changes.begin(); chit != vit->second.cache_changes.end(); ++chit)
        {
            if (chit->change->sequenceNumber == change->sequenceNumber &&
                    chit->change->writerGUID == change->writerGUID)
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
    if (find_key(change, &vit))
    {
        for (auto chit = vit->second.cache_changes.begin(); chit != vit->second.cache_changes.end(); ++chit)
        {
            if (chit->change->sequenceNumber == change->sequenceNumber &&
                    chit->change->writerGUID == change->writerGUID)
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
    if (keyed_changes_.find(handle) == keyed_changes_.end())
    {
        return false;
    }

    keyed_changes_[handle].next_deadline_us = next_deadline_us;
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
        bool exact)
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
        return { true, {tmp, &keyed_changes_.begin()->second} };
    }

    InstanceCollection::iterator it;

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
        return { true, {it->first, &(it->second)} };
    }
    return { false, {InstanceHandle_t(), nullptr} };
}

ReaderHistory::iterator DataReaderHistory::remove_change_nts(
        ReaderHistory::const_iterator removal,
        bool release)
{
    if (removal != changesEnd())
    {
        CacheChange_t* p_sample = *removal;

        // clean any references to this CacheChange in the key state collection
        auto it = keyed_changes_.find(p_sample->instanceHandle);

        // if keyed and in history must be in the map
        assert(it != keyed_changes_.end());

        auto& c = it->second.cache_changes;
        c.erase(std::remove_if(c.begin(), c.end(), [p_sample](DataReaderCacheChange& elem)
                {
                    return elem.change == p_sample;
                }), c.end());
    }

    // call the base class
    return ReaderHistory::remove_change_nts(removal, release);
}

} // namespace detail
} // namsepace dds
} // namespace fastdds
} // namsepace eprosima
