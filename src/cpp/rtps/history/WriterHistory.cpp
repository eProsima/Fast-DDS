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
 * @file WriterHistory.cpp
 *
 */

#include <fastdds/rtps/history/WriterHistory.hpp>

#include <cassert>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <utility>

#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/ChangeKind_t.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/SampleIdentity.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>
#include <fastdds/rtps/common/WriteParams.hpp>
#include <fastdds/rtps/history/History.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/utils/TimedMutex.hpp>

#include <fastdds/core/policy/ParameterSerializer.hpp>
#include <rtps/history/BasicPayloadPool.hpp>
#include <rtps/history/CacheChangePool.h>
#include <rtps/history/PoolConfig.h>
#include <rtps/messages/RTPSMessageGroup.hpp>
#include <rtps/writer/BaseWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

static CacheChange_t* initialize_change(
        CacheChange_t* reserved_change,
        ChangeKind_t change_kind,
        InstanceHandle_t handle,
        RTPSWriter* writer)
{
    reserved_change->kind = change_kind;
    if ((WITH_KEY == writer->getAttributes().topicKind) && !handle.isDefined())
    {
        EPROSIMA_LOG_WARNING(RTPS_WRITER, "Changes in KEYED Writers need a valid instanceHandle");
    }
    reserved_change->instanceHandle = handle;
    reserved_change->writerGUID = writer->getGuid();
    reserved_change->writer_info.previous = nullptr;
    reserved_change->writer_info.next = nullptr;
    reserved_change->writer_info.num_sent_submessages = 0;
    reserved_change->vendor_id = c_VendorId_eProsima;
    return reserved_change;
}

WriteParams WriteParams::WRITE_PARAM_DEFAULT;

WriterHistory::WriterHistory(
        const HistoryAttributes& att)
    : History(att)
{
    PoolConfig cfg = PoolConfig::from_history_attributes(att);
    payload_pool_ = BasicPayloadPool::get(cfg, change_pool_);
}

WriterHistory::WriterHistory(
        const HistoryAttributes& att,
        const std::shared_ptr<IPayloadPool>& payload_pool)
    : History(att)
    , change_pool_(std::make_shared<CacheChangePool>(PoolConfig::from_history_attributes(att)))
    , payload_pool_(payload_pool)
{
}

WriterHistory::WriterHistory(
        const HistoryAttributes& att,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        const std::shared_ptr<IChangePool>& change_pool)
    : History(att)
    , change_pool_(change_pool)
    , payload_pool_(payload_pool)
{
}

WriterHistory::~WriterHistory()
{
    // As releasing the change pool will delete the cache changes it owns,
    // the payload pool may be called to release their payloads, so we should
    // ensure that the payload pool is destroyed after the change pool.
    change_pool_.reset();
    payload_pool_.reset();
}

const std::shared_ptr<IPayloadPool>& WriterHistory::get_payload_pool() const
{
    return payload_pool_;
}

const std::shared_ptr<IChangePool>& WriterHistory::get_change_pool() const
{
    return change_pool_;
}

bool WriterHistory::add_change(
        CacheChange_t* a_change)
{
    WriteParams wparams;
    return add_change_(a_change, wparams);
}

bool WriterHistory::add_change(
        CacheChange_t* a_change,
        WriteParams& wparams)
{
    return add_change_(a_change, wparams);
}

bool WriterHistory::prepare_and_add_change(
        CacheChange_t* a_change,
        WriteParams& wparams)
{
    if (a_change->writerGUID != mp_writer->getGuid())
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER_HISTORY,
                "Change writerGUID " << a_change->writerGUID << " different than Writer GUID " <<
                mp_writer->getGuid());
        return false;
    }
    if ((m_att.memoryPolicy == PREALLOCATED_MEMORY_MODE) && a_change->serializedPayload.length > m_att.payloadMaxSize)
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER_HISTORY,
                "Change payload size of '" << a_change->serializedPayload.length <<
                "' bytes is larger than the history payload size of '" << m_att.payloadMaxSize <<
                "' bytes and cannot be resized.");
        return false;
    }

    if (m_isHistoryFull)
    {
        EPROSIMA_LOG_WARNING(RTPS_WRITER_HISTORY, "History full for writer " << a_change->writerGUID);
        return false;
    }

    ++m_lastCacheChangeSeqNum;
    a_change->sequenceNumber = m_lastCacheChangeSeqNum;
    if (wparams.source_timestamp().seconds() < 0)
    {
        Time_t::now(a_change->sourceTimestamp);
    }
    else
    {
        a_change->sourceTimestamp = wparams.source_timestamp();
    }
    a_change->writer_info.num_sent_submessages = 0;

    a_change->write_params = wparams;
    // Updated sample and related sample identities on the user's write params
    wparams.sample_identity().writer_guid(a_change->writerGUID);
    wparams.sample_identity().sequence_number(a_change->sequenceNumber);
    wparams.related_sample_identity(wparams.sample_identity());
    set_fragments(a_change);

    m_changes.push_back(a_change);

    if (static_cast<int32_t>(m_changes.size()) == m_att.maximumReservedCaches)
    {
        m_isHistoryFull = true;
    }

    EPROSIMA_LOG_INFO(RTPS_WRITER_HISTORY,
            "Change " << a_change->sequenceNumber << " added with " << a_change->serializedPayload.length << " bytes");

    return true;
}

void WriterHistory::notify_writer(
        CacheChange_t* a_change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    mp_writer->unsent_change_added_to_history(a_change, max_blocking_time);
}

bool WriterHistory::add_change_(
        CacheChange_t* a_change,
        WriteParams& wparams,
        std::chrono::time_point<std::chrono::steady_clock> max_blocking_time)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER_HISTORY,
                "You need to create a Writer with this History before adding any changes");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    if (!prepare_and_add_change(a_change, wparams))
    {
        return false;
    }

    notify_writer(a_change, max_blocking_time);

    return true;
}

bool WriterHistory::matches_change(
        const CacheChange_t* inner_change,
        CacheChange_t* outer_change)
{
    if (nullptr == outer_change
            || nullptr == inner_change)
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER_HISTORY, "Pointer is not valid");
        return false;
    }

    if (outer_change->writerGUID != mp_writer->getGuid())
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER_HISTORY,
                "Change writerGUID " << outer_change->writerGUID << " different than Writer GUID " <<
                mp_writer->getGuid());
        return false;
    }

    return inner_change->sequenceNumber == outer_change->sequenceNumber;
}

History::iterator WriterHistory::remove_change_nts(
        const_iterator removal,
        bool release)
{
    return WriterHistory::remove_change_nts(removal, std::chrono::steady_clock::now() + std::chrono::hours(24),
                   release);
}

History::iterator WriterHistory::remove_change_nts(
        const_iterator removal,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time,
        bool release)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER_HISTORY,
                "You need to create a Writer with this History before removing any changes");
        return remove_iterator_constness(removal);
    }

    if (removal == changesEnd())
    {
        EPROSIMA_LOG_INFO(RTPS_WRITER_HISTORY, "Trying to remove without a proper CacheChange_t referenced");
        return changesEnd();
    }

    CacheChange_t* change = *removal;

    // Inform writer
    if (mp_writer->change_removed_by_history(change, max_blocking_time))
    {
        // Remove from history
        auto ret_val = m_changes.erase(removal);
        m_isHistoryFull = false;

        // Release from pools
        if ( release )
        {
            release_change(change);
        }

        return ret_val;
    }

    EPROSIMA_LOG_INFO(RTPS_WRITER_HISTORY,
            "Failed to inform the writer that a change is going to be removed by the history");
    return remove_iterator_constness(removal);
}

bool WriterHistory::remove_change_g(
        CacheChange_t* a_change)
{
    return remove_change(a_change, std::chrono::steady_clock::now() + std::chrono::hours(24));
}

bool WriterHistory::remove_change_g(
        CacheChange_t* a_change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    return remove_change(a_change, max_blocking_time);
}

bool WriterHistory::remove_change(
        const SequenceNumber_t& sequence_number)
{
    CacheChange_t* p = remove_change_and_reuse(sequence_number);

    if (nullptr != p )
    {
        release_change(p);
        return true;
    }

    return false;
}

CacheChange_t* WriterHistory::remove_change_and_reuse(
        const SequenceNumber_t& sequence_number)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER_HISTORY,
                "You need to create a Writer with this History before removing any changes");
        return nullptr;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);

    // Create a temporary reference change associated to the sequence number
    CacheChange_t ch;
    ch.sequenceNumber = sequence_number;
    ch.writerGUID = mp_writer->getGuid();

    auto it = find_change(&ch);

    if ( it == changesEnd())
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER_HISTORY, "Sequence number provided doesn't match any change in history");
        return nullptr;
    }

    CacheChange_t* removal = *it;
    remove_change(it, false);

    return removal;
}

bool WriterHistory::remove_min_change()
{
    return remove_min_change(std::chrono::steady_clock::now() + std::chrono::hours(24));
}

bool WriterHistory::remove_min_change(
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{

    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER_HISTORY,
                "You need to create a Writer with this History before removing any changes");
        return false;
    }

#if HAVE_STRICT_REALTIME
    std::unique_lock<RecursiveTimedMutex> lock(*mp_mutex, std::defer_lock);
    if (!lock.try_lock_until(max_blocking_time))
    {
        EPROSIMA_LOG_ERROR(PUBLISHER, "Cannot lock the DataWriterHistory mutex");
        return false;
    }
#else
    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
#endif // if HAVE_STRICT_REALTIME

    if (m_changes.size() > 0 && remove_change_g(m_changes.front(), max_blocking_time))
    {
        return true;
    }
    else
    {
        return false;
    }
}

//TODO Hacer metodos de remove_all_changes. y hacer los metodos correspondientes en los writers y publishers.

CacheChange_t* WriterHistory::create_change(
        ChangeKind_t changeKind,
        InstanceHandle_t handle)
{
    EPROSIMA_LOG_INFO(RTPS_WRITER, "Creating new change");

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    CacheChange_t* reserved_change = nullptr;
    if (!change_pool_->reserve_cache(reserved_change))
    {
        EPROSIMA_LOG_WARNING(RTPS_WRITER, "Problem reserving cache from pool");
        return nullptr;
    }

    return initialize_change(reserved_change, changeKind, handle, mp_writer);
}

CacheChange_t* WriterHistory::create_change(
        uint32_t payload_size,
        ChangeKind_t changeKind,
        InstanceHandle_t handle)
{
    EPROSIMA_LOG_INFO(RTPS_WRITER, "Creating new change");

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    CacheChange_t* reserved_change = nullptr;
    if (!change_pool_->reserve_cache(reserved_change))
    {
        EPROSIMA_LOG_WARNING(RTPS_WRITER, "Problem reserving cache from pool");
        return nullptr;
    }

    if (!payload_pool_->get_payload(payload_size, reserved_change->serializedPayload))
    {
        change_pool_->release_cache(reserved_change);
        EPROSIMA_LOG_WARNING(RTPS_WRITER, "Problem reserving payload from pool");
        return nullptr;
    }

    return initialize_change(reserved_change, changeKind, handle, mp_writer);
}

bool WriterHistory::release_change(
        CacheChange_t* ch)
{
    // Asserting preconditions
    assert(mp_writer != nullptr);
    assert(ch != nullptr);
    assert(ch->writerGUID == mp_writer->getGuid());

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);

    IPayloadPool* pool = ch->serializedPayload.payload_owner;
    if (pool)
    {
        pool->release_payload(ch->serializedPayload);
    }
    return change_pool_->release_cache(ch);
}

void WriterHistory::do_release_cache(
        CacheChange_t* ch)
{
    release_change(ch);
}

void WriterHistory::set_fragments(
        CacheChange_t* change)
{
    // Fragment if necessary
    if (high_mark_for_frag_ == 0)
    {
        high_mark_for_frag_ = mp_writer->get_max_allowed_payload_size();
    }

    uint32_t final_high_mark_for_frag = high_mark_for_frag_;

    // Calc additional size for inline QoS
    uint32_t inline_qos_size = change->inline_qos.length;
    if (change->write_params.related_sample_identity() != SampleIdentity::unknown())
    {
        inline_qos_size += (2 * fastdds::dds::ParameterSerializer<Parameter_t>::PARAMETER_SAMPLE_IDENTITY_SIZE);
    }
    if (ChangeKind_t::ALIVE != change->kind && TopicKind_t::WITH_KEY == mp_writer->getAttributes().topicKind)
    {
        inline_qos_size += fastdds::dds::ParameterSerializer<Parameter_t>::PARAMETER_KEY_SIZE;
        inline_qos_size += fastdds::dds::ParameterSerializer<Parameter_t>::PARAMETER_STATUS_SIZE;
    }

    // If inlineqos for related_sample_identity is required, then remove its size from the final fragment size.
    if (0 < inline_qos_size)
    {
        uint32_t overhead = fastdds::dds::ParameterSerializer<Parameter_t>::PARAMETER_SENTINEL_SIZE + inline_qos_size;
        constexpr uint32_t min_fragment_size = 4;
        if (final_high_mark_for_frag < (overhead + min_fragment_size))
        {
            final_high_mark_for_frag = min_fragment_size;
        }
        else
        {
            final_high_mark_for_frag -= overhead;
        }
    }

    // If it is big data, fragment it.
    if (change->serializedPayload.length > final_high_mark_for_frag)
    {
        // Fragment the data.
        // Set the fragment size to the cachechange.
        change->setFragmentSize(static_cast<uint16_t>(
                    (std::min)(final_high_mark_for_frag, RTPSMessageGroup::get_max_fragment_payload_size())));
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
