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
 * @file StatefulReader.hpp
 */

#ifndef FASTDDS_RTPS_READER__STATEFULREADER_HPP
#define FASTDDS_RTPS_READER__STATEFULREADER_HPP

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <mutex>
#include <vector>

#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

#include <rtps/reader/BaseReader.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class WriterProxy;
class RTPSMessageSenderInterface;

/**
 * Class StatefulReader, specialization of BaseReader that stores the state of the matched writers.
 * @ingroup READER_MODULE
 */
class StatefulReader : public fastdds::rtps::BaseReader
{
public:

    friend class RTPSParticipantImpl;

    virtual ~StatefulReader();

protected:

    StatefulReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            ReaderHistory* hist,
            ReaderListener* listen = nullptr);

    StatefulReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            ReaderHistory* hist,
            ReaderListener* listen = nullptr);

    StatefulReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            const std::shared_ptr<IChangePool>& change_pool,
            ReaderHistory* hist,
            ReaderListener* listen = nullptr);

public:

    /**
     * Add a matched writer represented by its attributes.
     * @param wdata Attributes of the writer to add.
     * @return True if correctly added.
     */
    bool matched_writer_add_edp(
            const WriterProxyData& wdata) override;

    /**
     * Remove a WriterProxyData from the matached writers.
     * @param writer_guid GUID of the writer to remove.
     * @param removed_by_lease true it the writer was removed due to lease duration.
     * @return True if correct.
     */
    bool matched_writer_remove(
            const GUID_t& writer_guid,
            bool removed_by_lease = false) override;

    /**
     * Tells us if a specific Writer is matched against this reader.
     * @param writer_guid GUID of the writer to check.
     * @return True if it is matched.
     */
    bool matched_writer_is_matched(
            const GUID_t& writer_guid) override;

    /**
     * Look for a specific WriterProxy.
     * @param writerGUID GUID_t of the writer we are looking for.
     * @param WP Pointer to pointer to a WriterProxy.
     * @return True if found.
     */
    bool matched_writer_lookup(
            const GUID_t& writerGUID,
            WriterProxy** WP);

    /**
     * Processes a new DATA message.
     * @param change Pointer to the CacheChange_t.
     * @return true if the reader accepts messages.
     */
    bool process_data_msg(
            CacheChange_t* change) override;

    /**
     * Processes a new DATA FRAG message.
     *
     * @param change Pointer to the CacheChange_t.
     * @param sampleSize Size of the complete, assembled message.
     * @param fragmentStartingNum Starting number of this particular message.
     * @param fragmentsInSubmessage Number of fragments on this particular message.
     * @return true if the reader accepts message.
     */
    bool process_data_frag_msg(
            CacheChange_t* change,
            uint32_t sampleSize,
            uint32_t fragmentStartingNum,
            uint16_t fragmentsInSubmessage) override;

    /**
     * Processes a new HEARTBEAT message.
     *
     * @return true if the reader accepts messages.
     */
    bool process_heartbeat_msg(
            const GUID_t& writerGUID,
            uint32_t hbCount,
            const SequenceNumber_t& firstSN,
            const SequenceNumber_t& lastSN,
            bool finalFlag,
            bool livelinessFlag,
            fastdds::rtps::VendorId_t origin_vendor_id = c_VendorId_Unknown) override;

    bool process_gap_msg(
            const GUID_t& writerGUID,
            const SequenceNumber_t& gapStart,
            const SequenceNumberSet_t& gapList,
            fastdds::rtps::VendorId_t origin_vendor_id = c_VendorId_Unknown) override;

    /**
     * Method to indicate the reader that some change has been removed due to HistoryQos requirements.
     * @param change Pointer to the CacheChange_t.
     * @param prox Pointer to the WriterProxy.
     * @return True if correctly removed.
     */
    bool change_removed_by_history(
            CacheChange_t* change) override;

    /**
     * This method is called when a new change is received. This method calls the received_change of the History
     * and depending on the implementation performs different actions.
     * @param a_change Pointer of the change to add.
     * @param prox Pointer to the WriterProxy that adds the Change.
     * @param unknown_missing_changes_up_to The number of changes from the same writer with a lower sequence number that
     *                                      could potentially be received in the future.
     * @return True if added.
     */
    bool change_received(
            CacheChange_t* a_change,
            WriterProxy* prox,
            size_t unknown_missing_changes_up_to);

    /**
     * Get the RTPS participant
     * @return Associated RTPS participant
     */
    inline RTPSParticipantImpl* getRTPSParticipant() const
    {
        return mp_RTPSParticipant;
    }

    /**
     * Get reference to associated RTPS partiicipant's \c ResourceEvent
     * @return Reference to associated RTPS partiicipant's \c ResourceEvent
     */
    ResourceEvent& getEventResource() const;

    CacheChange_t* next_unread_cache() override;

    CacheChange_t* next_untaken_cache() override;

    /**
     * Update the times parameters of the Reader.
     * @param times ReaderTimes reference.
     * @return True if correctly updated.
     */
    bool updateTimes(
            const ReaderTimes& times);

    /**
     *
     * @return Reference to the ReaderTimes.
     */
    inline ReaderTimes& getTimes()
    {
        return times_;
    }

    /**
     * Get the number of matched writers
     * @return Number of matched writers
     */
    inline size_t getMatchedWritersSize() const
    {
        return matched_writers_.size();
    }

    /*!
     * @brief Returns there is a clean state with all Writers.
     * It occurs when the Reader received all samples sent by Writers. In other words,
     * its WriterProxies are up to date.
     * @return There is a clean state with all Writers.
     */
    bool is_in_clean_state() override;

    /**
     * Sends an acknack message from this reader.
     * @param writer Pointer to the info of the remote writer.
     * @param sns Sequence number bitmap with the acknack information.
     * @param sender Message sender interface.
     * @param is_final Value for final flag.
     */
    void send_acknack(
            const WriterProxy* writer,
            const SequenceNumberSet_t& sns,
            RTPSMessageSenderInterface* sender,
            bool is_final);

    /**
     * Sends an acknack message from this reader in response to a heartbeat.
     * @param writer Pointer to the proxy representing the writer to send the acknack to.
     * @param sender Message sender interface.
     * @param heartbeat_was_final Final flag of the last received heartbeat.
     */
    void send_acknack(
            const WriterProxy* writer,
            RTPSMessageSenderInterface* sender,
            bool heartbeat_was_final);

    /**
     * Use the participant of this reader to send a message to certain locator.
     * @param buffers Vector of buffers to send.
     * @param total_bytes Total number of bytes to send.
     * @param locators_begin Destination locators iterator begin.
     * @param locators_end Destination locators iterator end.
     * @param max_blocking_time_point Future time point where any blocking should end.
     */
    bool send_sync_nts(
            const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
            const uint32_t& total_bytes,
            const Locators& locators_begin,
            const Locators& locators_end,
            std::chrono::steady_clock::time_point& max_blocking_time_point);

    /**
     * Assert the livelines of a matched writer.
     * @param writer GUID of the writer to assert.
     */
    void assert_writer_liveliness(
            const GUID_t& writer) override;

    /**
     * Called just before a change is going to be deserialized.
     * @param [in]  change            Pointer to the change being accessed.
     * @param [out] writer            Writer proxy the @c change belongs to.
     * @param [out] is_future_change  Whether the change is in the future (i.e. there are
     *                                earlier unreceived changes from the same writer).
     *
     * @return Whether the change is still valid or not.
     */
    bool begin_sample_access_nts(
            CacheChange_t* change,
            WriterProxy*& writer,
            bool& is_future_change) override;

    /**
     * Called after the change has been deserialized.
     * @param [in] change        Pointer to the change being accessed.
     * @param [in] writer        Writer proxy the @c change belongs to.
     * @param [in] mark_as_read  Whether the @c change should be marked as read or not.
     */
    void end_sample_access_nts(
            CacheChange_t* change,
            WriterProxy*& writer,
            bool mark_as_read) override;

    /**
     * @brief Fills the provided vector with the GUIDs of the matched writers.
     *
     * @param[out] guids Vector to be filled with the GUIDs of the matched writers.
     * @return True if the operation was successful.
     */
    bool matched_writers_guids(
            std::vector<GUID_t>& guids) const final;

#ifdef FASTDDS_STATISTICS
    bool get_connections(
            fastdds::statistics::rtps::ConnectionList& connection_list) override;
#endif // ifdef FASTDDS_STATISTICS

private:

    void init(
            RTPSParticipantImpl* pimpl,
            const ReaderAttributes& att);

    bool acceptMsgFrom(
            const GUID_t& entityGUID,
            WriterProxy** wp) const;

    /*!
     * @brief Search for an incomplete (i.e. fragments pending) change, given its sequence number and writer's GUID.
     *
     * @param sequence_number [in] Sequence number of the change to search.
     * @param writer_guid [in]     Writer's GUID of the change to search.
     * @param change [out]         Pointer to the incomplete change if found, nullptr otherwise.
     * @param hint [in]            Iterator to start searching from. Used to improve the search.
     *
     * @return Iterator pointing to the position were the change was found.
     *         It can be used to improve the following call to this same method.
     */
    fastdds::rtps::History::const_iterator find_cache_in_fragmented_process(
            const fastdds::rtps::SequenceNumber_t& sequence_number,
            const fastdds::rtps::GUID_t& writer_guid,
            fastdds::rtps::CacheChange_t*& change,
            fastdds::rtps::History::const_iterator hint) const;

    /*!
     * @remarks Non thread-safe.
     */
    bool findWriterProxy(
            const GUID_t& writerGUID,
            WriterProxy** wp) const;

    void NotifyChanges(
            WriterProxy* wp);

    void remove_changes_from(
            const GUID_t& writerGUID,
            bool is_payload_pool_lost = false);

    //! Acknack Count
    uint32_t acknack_count_;
    //! NACKFRAG Count
    uint32_t nackfrag_count_;
    //!ReaderTimes of the StatefulReader.
    ReaderTimes times_;
    //! Vector containing pointers to all the active WriterProxies.
    ResourceLimitedVector<WriterProxy*> matched_writers_;
    //! Vector containing pointers to all the inactive, ready for reuse, WriterProxies.
    ResourceLimitedVector<WriterProxy*> matched_writers_pool_;
    //!
    ResourceLimitedContainerConfig proxy_changes_config_;
    //! True to disable positive ACKs
    bool disable_positive_acks_;
    //! False when being destroyed
    bool is_alive_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif // FASTDDS_RTPS_READER__STATEFULREADER_HPP
