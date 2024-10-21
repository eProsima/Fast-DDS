// Copyright 2016-2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ReaderProxy.hpp
 */
#ifndef RTPS_WRITER__READERPROXY_HPP
#define RTPS_WRITER__READERPROXY_HPP

#include <algorithm>
#include <atomic>
#include <mutex>
#include <set>

#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/FragmentNumber.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/writer/ChangeForReader.hpp>
#include <rtps/writer/ReaderLocator.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class BaseReader;
class StatefulWriter;
class TimedEvent;
class RTPSReader;
class IDataSharingNotifier;
class RTPSGapBuilder;

/**
 * ReaderProxy class that helps to keep the state of a specific Reader with respect to the RTPSWriter.
 * @ingroup WRITER_MODULE
 */
class ReaderProxy
{
public:

    ~ReaderProxy();

    /**
     * Constructor.
     * @param times WriterTimes to use in the ReaderProxy.
     * @param loc_alloc Maximum number of remote locators to keep in the ReaderProxy.
     * @param writer Pointer to the StatefulWriter creating the reader proxy.
     */
    ReaderProxy(
            const WriterTimes& times,
            const RemoteLocatorsAllocationAttributes& loc_alloc,
            StatefulWriter* writer);

    /**
     * Activate this proxy associating it to a remote reader.
     * @param reader_attributes ReaderProxyData of the reader for which to keep state.
     * @param is_datasharing whether the reader is datasharing compatible with the writer or not.
     */
    void start(
            const ReaderProxyData& reader_attributes,
            bool is_datasharing = false);

    /**
     * Update information about the remote reader.
     * @param reader_attributes ReaderProxyData with updated information of the reader.
     * @return true if data was modified, false otherwise.
     */
    bool update(
            const ReaderProxyData& reader_attributes);

    /**
     * Disable this proxy.
     */
    void stop();

    /**
     * Called when a change is added to the writer's history.
     * @param change Information regarding the change added.
     * @param is_relevant Specify if change is relevant for this remote reader.
     * @param restart_nack_supression Whether nack-supression event should be restarted.
     */
    void add_change(
            const ChangeForReader_t& change,
            bool is_relevant,
            bool restart_nack_supression);

    void add_change(
            const ChangeForReader_t& change,
            bool is_relevant,
            bool restart_nack_supression,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time);

    /**
     * Check if there are changes pending for this reader.
     * @return true when there are pending changes, false otherwise.
     */
    bool has_changes() const;

    /**
     * Check if a specific change has been already acknowledged for this reader.
     * @param seq_num Sequence number of the change to be checked.
     * @return true when the change is irrelevant or has been already acknowledged, false otherwise.
     */
    bool change_is_acked(
            const SequenceNumber_t& seq_num) const;

    /**
     * Check if a specific change is marked to be sent to this reader.
     *
     * @param [in]  seq_num Sequence number of the change to be checked.
     * @param [out] next_unsent_frag Return next fragment to be sent.
     * @param [out] gap_seq Return, when it is its first delivery (should be relevant seq_num), the sequence number of
     * the first sequence of the gap [first, seq_num). Otherwise return SequenceNumber_t::unknown().
     * @param [in]  min_seq Minimum sequence number managed by the History. It could be SequenceNumber_t::unknown() if
     * history is empty.
     * @param [out] need_reactivate_periodic_heartbeat Indicates if the heartbeat period event has to be restarted.
     *
     * @return true if the change is marked to be sent. False otherwise.
     */
    bool change_is_unsent(
            const SequenceNumber_t& seq_num,
            FragmentNumber_t& next_unsent_frag,
            SequenceNumber_t& gap_seq,
            const SequenceNumber_t& min_seq,
            bool& need_reactivate_periodic_heartbeat) const;

    /**
     * Mark all changes up to the one indicated by seq_num as Acknowledged.
     * For instance, when seq_num is 30, changes 1-29 are marked as acknowledged.
     * @param seq_num Sequence number of the first change not to be marked as acknowledged.
     */
    void acked_changes_set(
            const SequenceNumber_t& seq_num);

    /**
     * Mark all changes in the vector as requested.
     * @param seq_num_set Bitmap of sequence numbers.
     * @param gap_builder RTPSGapBuilder reference uses for adding  each requested change that is irrelevant for the
     * requester.
     * @param [in] min_seq_in_history Minimum SequenceNumber_t in the writer's history. If writer's history is empty,
     * SequenceNumber_t::unknown() is expected.
     * @return true if at least one change has been marked as REQUESTED, false otherwise.
     */
    bool requested_changes_set(
            const SequenceNumberSet_t& seq_num_set,
            RTPSGapBuilder& gap_builder,
            const SequenceNumber_t& min_seq_in_history);

    /**
     * Performs processing of preemptive acknack
     * @param func functor called, if the requester is a local reader, for each changes moved to UNSENT status.
     * @return true if a heartbeat should be sent, false otherwise.
     */
    bool process_initial_acknack(
            const std::function<void(ChangeForReader_t& change)>& func);

    /*!
     * @brief Sets a change to a particular status (if present in the ReaderProxy)
     * @param seq_num Sequence number of the change to update.
     * @param status Status to apply.
     * @param restart_nack_supression Whether nack supression event should be restarted or not.
     * @param delivered true if change was able to be delivered to its addressees. false otherwise.
     */
    void from_unsent_to_status(
            const SequenceNumber_t& seq_num,
            ChangeForReaderStatus_t status,
            bool restart_nack_supression,
            bool delivered = true);

    /**
     * @brief Mark a particular fragment as sent.
     * @param [in]  seq_num Sequence number of the change to update.
     * @param [in]  frag_num Fragment number to mark as sent.
     * @param [out] was_last_fragment Indicates if the fragment was the last one pending.
     * @return true when the change was found, false otherwise.
     */
    bool mark_fragment_as_sent_for_change(
            const SequenceNumber_t& seq_num,
            FragmentNumber_t frag_num,
            bool& was_last_fragment);

    /**
     * Turns all UNDERWAY changes into UNACKNOWLEDGED.
     *
     * @return true if at least one change changed its status, false otherwise.
     */
    bool perform_nack_supression();

    /**
     * Turns all REQUESTED changes into UNSENT.
     *
     * @param func Function executed for each change which changes its status.
     * @return the number of changes that changed its status.
     */
    uint32_t perform_acknack_response(
            const std::function<void(ChangeForReader_t& change)>& func);

    /**
     * Call this to inform a change was removed from history.
     * @param seq_num Sequence number of the removed change.
     */
    void change_has_been_removed(
            const SequenceNumber_t& seq_num);

    /*!
     * @brief Returns there is some UNACKNOWLEDGED change.
     * @param first_seq_in_history Minimum sequence number in the writer history.
     * @return There is some UNACKNOWLEDGED change.
     */
    bool has_unacknowledged(
            const SequenceNumber_t& first_seq_in_history) const;

    /**
     * Get the GUID of the reader represented by this proxy.
     * @return the GUID of the reader represented by this proxy.
     */
    inline const GUID_t& guid() const
    {
        return locator_info_.remote_guid();
    }

    /**
     * Get the durability of the reader represented by this proxy.
     * @return the durability of the reader represented by this proxy.
     */
    inline DurabilityKind_t durability_kind() const
    {
        return durability_kind_;
    }

    /**
     * Check if the reader represented by this proxy expexts inline QOS to be received.
     * @return true if the reader represented by this proxy expexts inline QOS to be received.
     */
    inline bool expects_inline_qos() const
    {
        return expects_inline_qos_;
    }

    /**
     * Check if the reader represented by this proxy is reliable.
     * @return true if the reader represented by this proxy is reliable.
     */
    inline bool is_reliable() const
    {
        return is_reliable_;
    }

    inline bool disable_positive_acks() const
    {
        return disable_positive_acks_;
    }

    /**
     * Check if the reader represented by this proxy is remote and reliable.
     * @return true if the reader represented by this proxy is remote and reliable.
     */
    inline bool is_remote_and_reliable() const
    {
        return !locator_info_.is_local_reader() && !locator_info_.is_datasharing_reader() && is_reliable_;
    }

    /**
     * Check if the reader is on the same process.
     * @return true if the reader is no the same process.
     */
    inline bool is_local_reader()
    {
        return locator_info_.is_local_reader();
    }

    /**
     * Get the local reader on the same process (if any).
     * @return The local reader on the same process.
     */
    inline LocalReaderPointer::Instance local_reader()
    {
        return locator_info_.local_reader();
    }

    /**
     * Called when an ACKNACK is received to set a new value for the minimum count accepted for following received
     * ACKNACKs.
     *
     * @param acknack_count The count of the received ACKNACK.
     * @return true if internal count changed (i.e. received ACKNACK is accepted)
     */
    bool check_and_set_acknack_count(
            uint32_t acknack_count)
    {
        if (acknack_count >= next_expected_acknack_count_)
        {
            next_expected_acknack_count_ = acknack_count;
            ++next_expected_acknack_count_;
            return true;
        }

        return false;
    }

    /**
     * Process an incoming NACKFRAG submessage.
     * @param reader_guid Destination guid of the submessage.
     * @param nack_count Counter field of the submessage.
     * @param seq_num Sequence number field of the submessage.
     * @param fragments_state Bitmap indicating the requested fragments.
     * @return true if a change was modified, false otherwise.
     */
    bool process_nack_frag(
            const GUID_t& reader_guid,
            uint32_t nack_count,
            const SequenceNumber_t& seq_num,
            const FragmentNumberSet_t& fragments_state);

    /**
     * Filter a CacheChange_t using the StatefulWriter's IReaderDataFilter.
     * @param change
     * @return true if the change is relevant, false otherwise.
     */
    bool rtps_is_relevant(
            CacheChange_t* change) const;

    /**
     * Get the highest fully acknowledged sequence number.
     * @return the highest fully acknowledged sequence number.
     */
    SequenceNumber_t changes_low_mark() const
    {
        return changes_low_mark_;
    }

    /**
     * Change the interval of nack-supression event.
     * @param interval Time from data sending to acknack processing.
     */
    void update_nack_supression_interval(
            const dds::Duration_t& interval);

    LocatorSelectorEntry* general_locator_selector_entry()
    {
        return locator_info_.general_locator_selector_entry();
    }

    LocatorSelectorEntry* async_locator_selector_entry()
    {
        return locator_info_.async_locator_selector_entry();
    }

    RTPSMessageSenderInterface* message_sender()
    {
        return &locator_info_;
    }

    bool is_datasharing_reader() const
    {
        return locator_info_.is_datasharing_reader();
    }

    IDataSharingNotifier* datasharing_notifier()
    {
        return locator_info_.datasharing_notifier();
    }

    const IDataSharingNotifier* datasharing_notifier() const
    {
        return locator_info_.datasharing_notifier();
    }

    void datasharing_notify()
    {
        locator_info_.datasharing_notify();
    }

    size_t locators_size() const
    {
        return locator_info_.locators_size();
    }

    bool active() const
    {
        return active_;
    }

    void active(
            bool active)
    {
        active_ = active;
    }

    /**
     * @brief Check if the sequence number given has been delivered at least once to the transport layer.
     *
     * @param seq_number Sequence number of the change to check.
     * @param found The sequence number has been found in the list of changes pending to be sent/ack.
     *              This flag allows to differentiate the case when the change is not found from the one that is found
     *              but it has not been delivered yet.
     * @return true if the change has been delivered.
     * @return false otherwise.
     */
    bool has_been_delivered(
            const SequenceNumber_t& seq_number,
            bool& found) const;

private:

    //!Is this proxy active? I.e. does it have a remote reader associated?
    bool is_active_;
    //!Reader locator information
    ReaderLocator locator_info_;
    //!Taken from QoS
    DurabilityKind_t durability_kind_;
    //!Taken from QoS
    bool expects_inline_qos_;
    //!Taken from QoS
    bool is_reliable_;
    //!Taken from QoS
    bool disable_positive_acks_;
    //!Pointer to the associated StatefulWriter.
    StatefulWriter* writer_;
    //!Set of the changes and its state.
    ResourceLimitedVector<ChangeForReader_t, std::true_type> changes_for_reader_;
    //! Timed Event to manage the delay to mark a change as UNACKED after sending it.
    TimedEvent* nack_supression_event_;
    TimedEvent* initial_heartbeat_event_;
    //! Are timed events enabled?
    std::atomic_bool timers_enabled_;
    //! Next expected ack/nack count
    uint32_t next_expected_acknack_count_;
    //! Last  NACKFRAG count.
    uint32_t last_nackfrag_count_;

    SequenceNumber_t changes_low_mark_;

    bool active_ = false;

    using ChangeIterator = ResourceLimitedVector<ChangeForReader_t, std::true_type>::iterator;
    using ChangeConstIterator = ResourceLimitedVector<ChangeForReader_t, std::true_type>::const_iterator;

    void disable_timers();

    /*
     * Converts all changes with a given status to a different status.
     * @param previous Status to change.
     * @param next Status to adopt.
     * @param func Function executed for each change which changes its status.
     * @return the number of changes that have been modified.
     */
    uint32_t convert_status_on_all_changes(
            ChangeForReaderStatus_t previous,
            ChangeForReaderStatus_t next,
            const std::function<void(ChangeForReader_t& change)>& func = {});

    /*!
     * @brief Adds requested fragments. These fragments will be sent in next NackResponseDelay.
     * @param [in] seq_num Sequence number to be paired with the requested fragments.
     * @param [in] frag_set set containing the requested fragments to be sent.
     * @return True if there is at least one requested fragment. False in other case.
     */
    bool requested_fragment_set(
            const SequenceNumber_t& seq_num,
            const FragmentNumberSet_t& frag_set);

    void add_change(
            const ChangeForReader_t& change,
            bool is_relevant);

    /**
     * @brief Find a change with the specified sequence number.
     * @param seq_num Sequence number to find.
     * @param exact When false, the first change with a sequence number not less than seq_num will be returned.
     * When true, the change with a sequence number value of seq_num will be returned.
     * @return Iterator pointing to the change, changes_for_reader_.end() if not found.
     */
    ChangeIterator find_change(
            const SequenceNumber_t& seq_num,
            bool exact);

    /**
     * @brief Find a change with the specified sequence number.
     * @param seq_num Sequence number to find.
     * @return Iterator pointing to the change, changes_for_reader_.end() if not found.
     */
    ChangeConstIterator find_change(
            const SequenceNumber_t& seq_num) const;
};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // RTPS_WRITER__READERPROXY_HPP
