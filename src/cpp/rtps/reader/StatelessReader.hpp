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
 * @file StatelessReader.hpp
 */


#ifndef FASTDDS_RTPS_READER__STATELESSREADER_HPP
#define FASTDDS_RTPS_READER__STATELESSREADER_HPP

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <map>
#include <mutex>

#include <fastdds/rtps/common/VendorId_t.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

#include <rtps/reader/BaseReader.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class StatelessReader, specialization of the RTPSReader for Best Effort Readers.
 * @ingroup READER_MODULE
 */
class StatelessReader : public fastdds::rtps::BaseReader
{
    friend class RTPSParticipantImpl;

public:

    virtual ~StatelessReader();

protected:

    StatelessReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            ReaderHistory* hist,
            ReaderListener* listen = nullptr);

    StatelessReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            ReaderHistory* hist,
            ReaderListener* listen = nullptr);

    StatelessReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            const std::shared_ptr<IChangePool>& change_pool,
            ReaderHistory* hist,
            ReaderListener* listen = nullptr);

public:

    /**
     * Add a matched writer represented by a WriterProxyData object.
     * @param wdata Pointer to the WPD object to add.
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
     * Method to indicate the reader that some change has been removed due to HistoryQos requirements.
     * @param change Pointer to the CacheChange_t.
     * @param prox Pointer to the WriterProxy.
     * @return True if correctly removed.
     */
    bool change_removed_by_history(
            CacheChange_t* change) override;

    /**
     * Processes a new DATA message.
     *
     * @param change Pointer to the CacheChange_t.
     * @return true if the reader accepts messages from the.
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
     * @return true if the reader accepts messages from the.
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
     * This method is called when a new change is received. This method calls the received_change of the History
     * and depending on the implementation performs different actions.
     * @param a_change Pointer of the change to add.
     * @return True if added.
     */
    bool change_received(
            CacheChange_t* a_change);

    CacheChange_t* next_unread_cache() override;

    CacheChange_t* next_untaken_cache() override;

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
     * StatelessReader allways return true;
     * @return true
     */
    bool is_in_clean_state() override
    {
        return true;
    }

    /**
     * Get the RTPS participant
     * @return Associated RTPS participant
     */
    inline RTPSParticipantImpl* getRTPSParticipant() const
    {
        return mp_RTPSParticipant;
    }

    /**
     * @brief Assert liveliness of remote writer
     * @param guid The guid of the remote writer
     */
    void assert_writer_liveliness(
            const GUID_t& guid) override;

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

    struct RemoteWriterInfo_t
    {
        GUID_t guid;
        GUID_t persistence_guid;
        bool has_manual_topic_liveliness = false;
        CacheChange_t* fragmented_change = nullptr;
        bool is_datasharing = false;
        uint32_t ownership_strength;
    };

    bool acceptMsgFrom(
            const GUID_t& entityId,
            ChangeKind_t change_kind);

    bool thereIsUpperRecordOf(
            const GUID_t& guid,
            const SequenceNumber_t& seq);

    /**
     * @brief A method to check if a matched writer has manual_by_topic liveliness
     * @param guid The guid of the remote writer
     * @return True if writer has manual_by_topic livelinesss
     */
    bool writer_has_manual_liveliness(
            const GUID_t& guid);

    void remove_changes_from(
            const GUID_t& writerGUID,
            bool is_payload_pool_lost = false);


    //!List of GUID_t os matched writers.
    //!Is only used in the Discovery, to correctly notify the user using SubscriptionListener::onSubscriptionMatched();
    ResourceLimitedVector<RemoteWriterInfo_t> matched_writers_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif // FASTDDS_RTPS_READER__STATELESSREADER_HPP
