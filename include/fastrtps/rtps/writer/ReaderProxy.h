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
 * @file ReaderProxy.h
 */
#ifndef FASTRTPS_RTPS_WRITER_READERPROXY_H_
#define FASTRTPS_RTPS_WRITER_READERPROXY_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <algorithm>
#include <mutex>
#include <set>
#include "../common/Types.h"
#include "../common/Locator.h"
#include "../common/SequenceNumber.h"
#include "../common/CacheChange.h"
#include "../common/FragmentNumber.h"
#include "../attributes/WriterAttributes.h"

#include <set>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class StatefulWriter;
class NackResponseDelay;
class NackSupressionDuration;

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
     * @param reader_attributes RemoteReaderAttributes of the reader for which to keep state.
     * @param times WriterTimes to use in the ReaderProxy.
     * @param writer Pointer to the StatefulWriter creating the reader proxy.
     */
    ReaderProxy(
            const RemoteReaderAttributes& reader_attributes, 
            const WriterTimes& times, 
            StatefulWriter* writer);

    void destroy_timers();

    void add_change(
            const ChangeForReader_t& change,
            bool restart_nack_supression);

    bool has_changes() const;

    bool change_is_acked(const SequenceNumber_t& seq_num) const;

    /**
     * Mark all changes up to the one indicated by seq_num as Acknowledged.
     * For instance, when seq_num is 30, changes 1-29 are marked as acknowledged.
     * @param seq_num Sequence number of the first change not to be marked as acknowledged.
     */
    void acked_changes_set(const SequenceNumber_t& seq_num);

    /**
     * Mark all changes in the vector as requested.
     * @param seq_num_set Bitmap of sequence numbers.
     * @return true if at least one change has been marked as REQUESTED, false otherwise.
     */
    bool requested_changes_set(const SequenceNumberSet_t& seq_num_set);

    /**
    * Applies the given function object to every unsent change.
    * @param f Function to apply
    */
    template <class UnaryFunction>
    void for_each_unsent_change(UnaryFunction f) const
    {
        for (auto &change_for_reader : m_changesForReader)
        {
            if (change_for_reader.getStatus() == UNSENT)
            {
                f(&change_for_reader);
            }
        }
    }

    /*!
     * @brief Sets a change to a particular status (if present in the ReaderProxy)
     * @param seq_num Sequence number of the change to update.
     * @param status Status to apply.
     * @param restart_nack_supression Whether nack supression event should be restarted or not.
     * @return true when a status has changed, false otherwise.
     */
    bool set_change_to_status(
            const SequenceNumber_t& seq_num, 
            ChangeForReaderStatus_t status,
            bool restart_nack_supression);

    /**
     * @brief Mark a particular fragment as sent.
     * @param[in]  seq_num Sequence number of the change to update.
     * @param[in]  frag_num Fragment number to mark as sent.
     * @param[out] was_last_fragment Indicates if the fragment was the last one pending.
     * @return true when the change was found, false otherwise.
     */
    bool mark_fragment_as_sent_for_change(
            const SequenceNumber_t& seq_num,
            FragmentNumber_t frag_num,
            bool& was_last_fragment);

    /*
     * Converts all changes with a given status to a different status.
     * @param previous Status to change.
     * @param next Status to adopt.
     * @return true when at least one change has been modified, false otherwise.
     */
    bool convert_status_on_all_changes(
            ChangeForReaderStatus_t previous, 
            ChangeForReaderStatus_t next);

    void change_has_been_removed(const SequenceNumber_t& seq_num);

    /*!
     * @brief Returns there is some UNACKNOWLEDGED change.
     * @return There is some UNACKNOWLEDGED change.
     */
    bool has_unacknowledged() const;

    inline const GUID_t& guid() const
    {
        return reader_attributes_.guid;
    }

    inline DurabilityKind_t durability_kind() const
    {
        return reader_attributes_.endpoint.durabilityKind;
    }

    inline bool expects_inline_qos() const
    {
        return reader_attributes_.expectsInlineQos;
    }

    inline bool is_reliable() const
    {
        return reader_attributes_.endpoint.reliabilityKind == RELIABLE;
    }

    inline const RemoteReaderAttributes& reader_attributes() const
    {
        return reader_attributes_;
    }

    inline const LocatorList_t& remote_locators() const
    {
        return reader_attributes_.endpoint.remoteLocatorList;
    }

    bool check_and_set_acknack_count(uint32_t acknack_count)
    {
        if (last_acknack_count_ < acknack_count)
        {
            last_acknack_count_ = acknack_count;
            return true;
        }

        return false;
    }

    bool process_nack_frag(
            const GUID_t& reader_guid, 
            uint32_t nack_count,
            const SequenceNumber_t& seq_num,
            const FragmentNumberSet_t& fragments_state);

    /**
     * Filter a CacheChange_t, in this version always returns true.
     * @param change
     * @return
     */
    inline bool rtps_is_relevant(CacheChange_t* change)
    {
        (void)change; return true;
    };

    SequenceNumber_t get_low_mark() const
    {
        return changes_low_mark_;
    }

    void update_nack_response_interval(const Duration_t& interval);

    void update_nack_supression_interval(const Duration_t& interval);

private:

    //!Attributes of the Remote Reader
    RemoteReaderAttributes reader_attributes_;
    //!Pointer to the associated StatefulWriter.
    StatefulWriter* writer_;
    //!Set of the changes and its state.
    std::set<ChangeForReader_t, ChangeForReaderCmp> m_changesForReader;
    //! Timed Event to manage the Acknack response delay.
    NackResponseDelay* nack_response_event_;
    //! Timed Event to manage the delay to mark a change as UNACKED after sending it.
    NackSupressionDuration* nack_supression_event_;
    //! Last ack/nack count
    uint32_t last_acknack_count_;
    //! Last  NACKFRAG count.
    uint32_t last_nackfrag_count_;

    SequenceNumber_t changes_low_mark_;

    /*!
     * @brief Adds requested fragments. These fragments will be sent in next NackResponseDelay.
     * @param[in] seq_num Sequence number to be paired with the requested fragments.
     * @param[in] frag_set set containing the requested fragments to be sent.
     * @return True if there is at least one requested fragment. False in other case.
     */
    bool requested_fragment_set(
            const SequenceNumber_t& seq_num, 
            const FragmentNumberSet_t& frag_set);

};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
#endif
#endif /* FASTRTPS_RTPS_WRITER_READERPROXY_H_ */
