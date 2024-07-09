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
 * @file ReaderHistory.hpp
 *
 */

#ifndef FASTDDS_RTPS_HISTORY__READERHISTORY_HPP
#define FASTDDS_RTPS_HISTORY__READERHISTORY_HPP

#include <fastdds/rtps/history/History.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/dds/core/status/SampleRejectedStatus.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class WriterProxy;
class RTPSReader;

/**
 * Class ReaderHistory, container of the different CacheChanges of a reader
 * @ingroup READER_MODULE
 */
class ReaderHistory : public History
{
    friend class RTPSReader;

    ReaderHistory(
            ReaderHistory&&) = delete;
    ReaderHistory& operator =(
            ReaderHistory&&) = delete;

public:

    /**
     * Constructor of the ReaderHistory. It needs a HistoryAttributes.
     */
    FASTDDS_EXPORTED_API ReaderHistory(
            const HistoryAttributes& att);
    FASTDDS_EXPORTED_API ~ReaderHistory() override;

    /**
     * Check if a new change can be added to this history.
     *
     * @param [in]  writer_guid                    GUID of the writer where the change came from.
     * @param [in]  total_payload_size             Total payload size of the incoming change.
     * @param [in]  unknown_missing_changes_up_to  The number of changes from the same writer with a lower sequence
     *                                             number that could potentially be received in the future.
     * @param [out] will_never_be_accepted         When the method returns @c false, this parameter will inform
     *                                             whether the change could be accepted in the future or not.
     *
     * @pre change should not be present in the history
     *
     * @return Whether a call to received_change will succeed when called with the same arguments.
     */
    FASTDDS_EXPORTED_API virtual bool can_change_be_added_nts(
            const GUID_t& writer_guid,
            uint32_t total_payload_size,
            size_t unknown_missing_changes_up_to,
            bool& will_never_be_accepted) const;

    /**
     * Virtual method that is called when a new change is received.
     * In this implementation this method just calls add_change. The user can overload this method in case
     * he needs to perform additional checks before adding the change.
     * @param change Pointer to the change
     * @param unknown_missing_changes_up_to The number of changes from the same writer with a lower sequence number that
     *                                      could potentially be received in the future.
     * @return True if added.
     */
    FASTDDS_EXPORTED_API virtual bool received_change(
            CacheChange_t* change,
            size_t unknown_missing_changes_up_to);

    /**
     * Virtual method that is called when a new change is received.
     * In this implementation this method just calls add_change. The user can overload this method in case
     * he needs to perform additional checks before adding the change.
     * @param [in] change Pointer to the change
     * @param [in] unknown_missing_changes_up_to The number of changes from the same writer with a lower sequence number that
     *                                      could potentially be received in the future.
     * @param [out] rejection_reason In case of been rejected the sample, it will contain the reason of the rejection.
     * @return True if added.
     */
    FASTDDS_EXPORTED_API virtual bool received_change(
            CacheChange_t* change,
            size_t unknown_missing_changes_up_to,
            fastdds::dds::SampleRejectedStatusKind& rejection_reason)
    {
        rejection_reason = fastdds::dds::NOT_REJECTED;
        return received_change(change, unknown_missing_changes_up_to);
    }

    /**
     * Called when a fragmented change is received completely by the Subscriber. Will find its instance and store it.
     * @pre Change should be already present in the history.
     * @param [in] change The received change
     * @return
     */
    FASTDDS_EXPORTED_API bool virtual completed_change(
            rtps::CacheChange_t* change)
    {
        (void)change;
        return true;
    }

    /**
     * Called when a fragmented change is received completely by the Subscriber. Will find its instance and store it.
     * @pre Change should be already present in the history.
     * @param [in] change The received change
     * @param [in] unknown_missing_changes_up_to Number of missing changes before this one
     * @param [out] rejection_reason In case of been rejected the sample, it will contain the reason of the rejection.
     * @return
     */
    FASTDDS_EXPORTED_API virtual bool completed_change(
            CacheChange_t* change,
            size_t unknown_missing_changes_up_to,
            fastdds::dds::SampleRejectedStatusKind& rejection_reason)
    {
        (void)change;
        (void)unknown_missing_changes_up_to;
        (void)rejection_reason;
        return true;
    }

    /**
     * Add a CacheChange_t to the ReaderHistory.
     * @param a_change Pointer to the CacheChange to add.
     * @return True if added.
     */
    FASTDDS_EXPORTED_API bool add_change(
            CacheChange_t* a_change);

    /**
     * Remove a specific change from the history.
     * No Thread Safe
     * @param removal iterator to the change for removal
     * @param release specifies if the change must be returned to the pool
     * @return iterator to the next change if any
     */
    FASTDDS_EXPORTED_API iterator remove_change_nts(
            const_iterator removal,
            bool release = true) override;

    /**
     * Remove a specific change from the history.
     * No Thread Safe
     * @param removal iterator to the change for removal
     * @param [in] max_blocking_time Maximum time this method has to complete the task.
     * @param release specifies if the change must be returned to the pool
     * @return iterator to the next change if any
     */
    FASTDDS_EXPORTED_API iterator remove_change_nts(
            const_iterator removal,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time,
            bool release = true) override;

    /**
     * Criteria to search a specific CacheChange_t on history
     * @param inner change to compare
     * @param outer change for comparison
     * @return true if inner matches outer criteria
     */
    FASTDDS_EXPORTED_API bool matches_change(
            const CacheChange_t* inner,
            CacheChange_t* outer) override;

    //! Introduce base class method into scope
    using History::remove_change;

    /**
     * Remove all changes from the History that have a certain guid.
     * @param a_guid Pointer to the target guid to search for.
     * @return True if successful, even if no changes have been removed.
     * */
    FASTDDS_EXPORTED_API bool remove_changes_with_guid(
            const GUID_t& a_guid);

    /**
     * Remove all fragmented changes from certain writer up to certain sequence number.
     * @param seq_num First SequenceNumber_t not to be removed.
     * @param writer_guid GUID of the writer for which changes should be looked for.
     * @return True if successful, even if no changes have been removed.
     */
    bool remove_fragmented_changes_until(
            const SequenceNumber_t& seq_num,
            const GUID_t& writer_guid);

    FASTDDS_EXPORTED_API bool get_min_change_from(
            CacheChange_t** min_change,
            const GUID_t& writerGuid);

    /**
     * Called when a writer is unmatched from the reader holding this history.
     *
     * This method will remove all the changes on the history that came from the writer being unmatched and which have
     * not yet been notified to the user.
     *
     * @param writer_guid        GUID of the writer being unmatched.
     * @param last_notified_seq  Last sequence number from the specified writer that was notified to the user.
     */
    FASTDDS_EXPORTED_API virtual void writer_unmatched(
            const GUID_t& writer_guid,
            const SequenceNumber_t& last_notified_seq);

    /*!
     * @brief This function should be called by reader if a writer updates its ownership strength.
     *
     * @param [in] writer_guid Guid of the writer which changes its ownership strength.
     * @param [out] ownership_strength New value of the writer's Ownership strength.
     */
    FASTDDS_EXPORTED_API virtual void writer_update_its_ownership_strength_nts(
            const GUID_t& writer_guid,
            const uint32_t ownership_strength)
    {
        static_cast<void>(writer_guid);
        static_cast<void>(ownership_strength);
    }

protected:

    FASTDDS_EXPORTED_API void do_release_cache(
            CacheChange_t* ch) override;

    template<typename Pred>
    inline void remove_changes_with_pred(
            Pred pred)
    {
        assert(nullptr != mp_reader);
        assert(nullptr != mp_mutex);

        std::lock_guard<RecursiveTimedMutex> guard(*mp_mutex);
        std::vector<CacheChange_t*>::iterator chit = m_changes.begin();
        while (chit != m_changes.end())
        {
            if (pred(*chit))
            {
                chit = remove_change_nts(chit);
            }
            else
            {
                ++chit;
            }
        }
    }

    //!Pointer to the reader
    RTPSReader* mp_reader;

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif // FASTDDS_RTPS_HISTORY__READERHISTORY_HPP
