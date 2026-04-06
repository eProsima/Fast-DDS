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

#include <fastdds/rtps/history/WriterHistory.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/common/WriteParams.h>
#include <fastdds/rtps/messages/CDRMessage.h>
#include <fastdds/core/policy/ParameterSerializer.hpp>

#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace rtps {

namespace {

constexpr uint16_t pid_standard_rpc_related_sample_identity = 0x0083;

/*!
 * @brief Append the standard RPC-over-DDS related_sample_identity PID to inline QoS.
 *
 * The extra PID is emitted only for non-builtin writers and only when a
 * related sample identity is present, preserving the existing builtin and
 * legacy/custom behavior.
 *
 * @param change Cache change whose inline QoS may be extended with the
 *               standard related_sample_identity PID.
 * @return true if the PID was appended successfully, or if no append was
 *         needed for the current change.
 */
bool append_standard_related_sample_identity_inline_qos(
        CacheChange_t& change)
{
    if (change.writerGUID.is_builtin() ||
            change.write_params.related_sample_identity() == SampleIdentity::unknown())
    {
        return true;
    }

    change.inline_qos.reserve(
        change.inline_qos.length +
        fastdds::dds::ParameterSerializer<Parameter_t>::PARAMETER_SAMPLE_IDENTITY_SIZE);

    CDRMessage_t msg(change.inline_qos);
    const SampleIdentity& sample_id = change.write_params.related_sample_identity();

    bool valid = true;
    valid &= CDRMessage::addUInt16(&msg, pid_standard_rpc_related_sample_identity);
    valid &= CDRMessage::addUInt16(&msg, 24);
    valid &= CDRMessage::addData(
        &msg,
        sample_id.writer_guid().guidPrefix.value,
        GuidPrefix_t::size);
    valid &= CDRMessage::addData(
        &msg,
        sample_id.writer_guid().entityId.value,
        EntityId_t::size);
    valid &= CDRMessage::addInt32(&msg, sample_id.sequence_number().high);
    valid &= CDRMessage::addUInt32(&msg, sample_id.sequence_number().low);

    if (valid)
    {
        change.inline_qos.length = msg.length;
        change.inline_qos.pos = msg.pos;
    }

    return valid;
}

} // namespace

WriteParams WriteParams::WRITE_PARAM_DEFAULT;

WriterHistory::WriterHistory(
        const HistoryAttributes& att)
    : History(att)
    , mp_writer(nullptr)
{

}

WriterHistory::~WriterHistory()
{
    // TODO Auto-generated destructor stub
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
        logError(RTPS_WRITER_HISTORY,
                "Change writerGUID " << a_change->writerGUID << " different than Writer GUID " <<
                mp_writer->getGuid());
        return false;
    }
    if ((m_att.memoryPolicy == PREALLOCATED_MEMORY_MODE) && a_change->serializedPayload.length > m_att.payloadMaxSize)
    {
        logError(RTPS_WRITER_HISTORY,
                "Change payload size of '" << a_change->serializedPayload.length <<
                "' bytes is larger than the history payload size of '" << m_att.payloadMaxSize <<
                "' bytes and cannot be resized.");
        return false;
    }

    if (m_isHistoryFull)
    {
        logWarning(RTPS_WRITER_HISTORY, "History full for writer " << a_change->writerGUID);
        return false;
    }

    ++m_lastCacheChangeSeqNum;
    a_change->sequenceNumber = m_lastCacheChangeSeqNum;
    Time_t::now(a_change->sourceTimestamp);
    a_change->writer_info.num_sent_submessages = 0;

    a_change->write_params = wparams;
    // Updated sample and related sample identities on the user's write params
    wparams.sample_identity().writer_guid(a_change->writerGUID);
    wparams.sample_identity().sequence_number(a_change->sequenceNumber);
    wparams.related_sample_identity(wparams.sample_identity());
    if (!append_standard_related_sample_identity_inline_qos(*a_change))
    {
        logError(RTPS_WRITER_HISTORY, "Failed to append standard related_sample_identity inline QoS");
        return false;
    }
    set_fragments(a_change);

    m_changes.push_back(a_change);

    if (static_cast<int32_t>(m_changes.size()) == m_att.maximumReservedCaches)
    {
        m_isHistoryFull = true;
    }

    logInfo(RTPS_WRITER_HISTORY,
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
        logError(RTPS_WRITER_HISTORY, "You need to create a Writer with this History before adding any changes");
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
        logError(RTPS_WRITER_HISTORY, "Pointer is not valid");
        return false;
    }

    if (outer_change->writerGUID != mp_writer->getGuid())
    {
        logError(RTPS_WRITER_HISTORY,
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
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_WRITER_HISTORY, "You need to create a Writer with this History before removing any changes");
        return changesEnd();
    }

    if ( removal == changesEnd())
    {
        logInfo(RTPS_WRITER_HISTORY, "Trying to remove without a proper CacheChange_t referenced");
        return changesEnd();
    }

    // Remove from history
    CacheChange_t* change = *removal;
    auto ret_val = m_changes.erase(removal);
    m_isHistoryFull = false;

    // Inform writer
    mp_writer->change_removed_by_history(change);

    // Release from pools
    if ( release )
    {
        mp_writer->release_change(change);
    }

    return ret_val;
}

bool WriterHistory::remove_change_g(
        CacheChange_t* a_change)
{
    return remove_change(a_change);
}

bool WriterHistory::remove_change(
        const SequenceNumber_t& sequence_number)
{
    CacheChange_t* p = remove_change_and_reuse(sequence_number);

    if (nullptr != p )
    {
        mp_writer->release_change(p);
        return true;
    }

    return false;
}

CacheChange_t* WriterHistory::remove_change_and_reuse(
        const SequenceNumber_t& sequence_number)
{
    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_WRITER_HISTORY, "You need to create a Writer with this History before removing any changes");
        return nullptr;
    }

    // Create a temporary reference change associated to the sequence number
    CacheChange_t ch;
    ch.sequenceNumber = sequence_number;
    ch.writerGUID = mp_writer->getGuid();

    auto it = find_change(&ch);

    if ( it == changesEnd())
    {
        logError(RTPS_WRITER_HISTORY, "Sequence number provided doesn't match any change in history");
        return nullptr;
    }

    CacheChange_t* removal = *it;
    remove_change(it, false);

    return removal;
}

bool WriterHistory::remove_min_change()
{

    if (mp_writer == nullptr || mp_mutex == nullptr)
    {
        logError(RTPS_WRITER_HISTORY, "You need to create a Writer with this History before removing any changes");
        return false;
    }

    std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
    if (m_changes.size() > 0 && remove_change_g(m_changes.front()))
    {
        return true;
    }
    else
    {
        return false;
    }
}

//TODO Hacer metodos de remove_all_changes. y hacer los metodos correspondientes en los writers y publishers.

bool WriterHistory::do_reserve_cache(
        CacheChange_t** change,
        uint32_t size)
{
    *change = mp_writer->new_change(
        [size]()
        {
            return size;
        }, ALIVE);
    return *change != nullptr;
}

void WriterHistory::do_release_cache(
        CacheChange_t* ch)
{
    mp_writer->release_change(ch);
}

void WriterHistory::set_fragments(
        CacheChange_t* change)
{
    // Fragment if necessary
    if (high_mark_for_frag_ == 0)
    {
        high_mark_for_frag_ = mp_writer->getMaxDataSize();
    }

    uint32_t final_high_mark_for_frag = high_mark_for_frag_;

    // Calc additional size for inline QoS
    uint32_t inline_qos_size = change->inline_qos.length;
    if (change->write_params.related_sample_identity() != SampleIdentity::unknown())
    {
        inline_qos_size += fastdds::dds::ParameterSerializer<Parameter_t>::PARAMETER_SAMPLE_IDENTITY_SIZE;
    }
    if (ChangeKind_t::ALIVE != change->kind && TopicKind_t::WITH_KEY == mp_writer->m_att.topicKind)
    {
        inline_qos_size += fastdds::dds::ParameterSerializer<Parameter_t>::PARAMETER_KEY_SIZE;
        inline_qos_size += fastdds::dds::ParameterSerializer<Parameter_t>::PARAMETER_STATUS_SIZE;
    }

    // If inlineqos for related_sample_identity is required, then remove its size from the final fragment size.
    if (0 < inline_qos_size)
    {
        final_high_mark_for_frag -= (
            fastdds::dds::ParameterSerializer<Parameter_t>::PARAMETER_SENTINEL_SIZE +
            inline_qos_size);
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
} // namespace fastrtps
} // namespace eprosima
