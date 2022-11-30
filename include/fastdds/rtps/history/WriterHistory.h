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
 * @file WriterHistory.h
 *
 */

#ifndef _FASTDDS_RTPS_WRITERHISTORY_H_
#define _FASTDDS_RTPS_WRITERHISTORY_H_

#include <fastdds/rtps/history/History.h>
#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSWriter;
class WriteParams;

/**
 * Class WriterHistory, container of the different CacheChanges of a writer
 * @ingroup WRITER_MODULE
 */
class WriterHistory : public rtps::History
{
    friend class RTPSWriter;
    friend class PersistentWriter;
    friend class IPersistenceService;

    WriterHistory(
            WriterHistory&&) = delete;
    WriterHistory& operator =(
            WriterHistory&&) = delete;

public:

    /**
     * Constructor of the WriterHistory.
     */
    RTPS_DllAPI WriterHistory(
            const HistoryAttributes&  att);
    RTPS_DllAPI virtual ~WriterHistory() override;

    /**
     * Add a CacheChange_t to the WriterHistory.
     * @param a_change Pointer to the CacheChange_t to be added.
     * @return True if added.
     */
    RTPS_DllAPI bool add_change(
            CacheChange_t* a_change);

    /**
     * Add a CacheChange_t to the WriterHistory.
     * @param a_change Pointer to the CacheChange_t to be added.
     * @param wparams Extra write parameters.
     * @return True if added.
     */
    RTPS_DllAPI bool add_change(
            CacheChange_t* a_change,
            WriteParams& wparams);

    /**
     * Remove a specific change from the history.
     * No Thread Safe
     * @param removal iterator to the change for removal
     * @param release specifies if the change should be return to the pool
     * @return iterator to the next change if any
     */
    RTPS_DllAPI iterator remove_change_nts(
            const_iterator removal,
            bool release = true) override;

    /**
     * Criteria to search a specific CacheChange_t on history
     * @param inner change to compare
     * @param outer change for comparison
     * @return true if inner matches outer criteria
     */
    RTPS_DllAPI bool matches_change(
            const CacheChange_t* inner,
            CacheChange_t* outer) override;

    //! Introduce base class method into scope
    using History::remove_change;

    RTPS_DllAPI virtual bool remove_change_g(
            CacheChange_t* a_change);

    RTPS_DllAPI bool remove_change(
            const SequenceNumber_t& sequence_number);

    RTPS_DllAPI CacheChange_t* remove_change_and_reuse(
            const SequenceNumber_t& sequence_number);

    /**
     * Remove the CacheChange_t with the minimum sequenceNumber.
     * @return True if correctly removed.
     */
    RTPS_DllAPI bool remove_min_change();

    RTPS_DllAPI SequenceNumber_t next_sequence_number() const
    {
        return m_lastCacheChangeSeqNum + 1;
    }

protected:

    RTPS_DllAPI bool do_reserve_cache(
            CacheChange_t** change,
            uint32_t size) override;

    RTPS_DllAPI void do_release_cache(
            CacheChange_t* ch) override;

    /**
     * Introduce a change into the history, and let the associated writer send it.
     *
     * @param [in,out] a_change       The change to be added.
     *                                Its @c sequenceNumber and sourceTimestamp will be filled by this method.
     *                                Its @c wparams will be filled from parameter @c wparams.
     * @param [in,out] wparams        On input, it holds the WriteParams to be copied into @c a_change.
     *                                On output, will be filled with the sample identity assigned to @c a_change.
     * @param [in] max_blocking_time  Maximum time point the writer is allowed to be blocked till the change is put
     *                                into the wire or the sending queue.
     *
     * @return whether @c a_change could be added to the history.
     */
    bool add_change_(
            CacheChange_t* a_change,
            WriteParams& wparams,
            std::chrono::time_point<std::chrono::steady_clock> max_blocking_time
            = std::chrono::steady_clock::now() + std::chrono::hours(24));

    /**
     * Introduce a change into the history, and let the associated writer send it.
     *
     * @param [in,out] a_change       The change to be added.
     *                                Its @c sequenceNumber and sourceTimestamp will be filled by this method.
     *                                Its @c wparams will be filled from parameter @c wparams.
     * @param [in,out] wparams        On input, it holds the WriteParams to be copied into @c a_change.
     *                                On output, will be filled with the sample identity assigned to @c a_change.
     * @param [in] pre_commit         Functor called after @c a_change has been added to the history, and its
     *                                information has been filled, but before the writer is notified of the insertion.
     * @param [in] max_blocking_time  Maximum time point the writer is allowed to be blocked till the change is put
     *                                into the wire or the sending queue.
     *
     * @return whether @c a_change could be added to the history.
     */
    template<typename PreCommitHook>
    bool add_change_with_commit_hook(
            CacheChange_t* a_change,
            WriteParams& wparams,
            PreCommitHook pre_commit,
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

        pre_commit(*a_change);
        notify_writer(a_change, max_blocking_time);

        return true;
    }

    //!Last CacheChange Sequence Number added to the History.
    SequenceNumber_t m_lastCacheChangeSeqNum;
    //!Pointer to the associated RTPSWriter;
    RTPSWriter* mp_writer;

    uint32_t high_mark_for_frag_ = 0;

private:

    /**
     * Introduce a change into the history.
     *
     * @param [in,out] a_change  The change to be added.
     *                           Its @c sequenceNumber and sourceTimestamp will be filled by this method.
     *                           Its @c wparams will be filled from parameter @c wparams.
     * @param [in,out] wparams   On input, it holds the WriteParams to be copied into @c a_change.
     *                           On output, will be filled with the sample identity assigned to @c a_change.
     *
     * @return whether @c a_change could be added to the history.
     */
    bool prepare_and_add_change(
            CacheChange_t* a_change,
            WriteParams& wparams);

    /**
     * Notifies the RTPS writer associated with this history that a change has been added.
     * Depending on the publish mode, it will be directly sent to the wire, or put into a sending queue.
     *
     * @param [in] a_change           The change that has just been added to the history.
     * @param [in] max_blocking_time  Maximum time point the writer is allowed to be blocked till the change is put
     *                                into the wire or the sending queue.
     */
    void notify_writer(
            CacheChange_t* a_change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time);

    void set_fragments(
            CacheChange_t* change);
};

} // namespace rtps
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_WRITERHISTORY_H_ */
