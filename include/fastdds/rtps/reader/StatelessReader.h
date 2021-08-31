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
 * @file StatelessReader.h
 */


#ifndef _FASTDDS_RTPS_READER_STATELESSREADER_H_
#define _FASTDDS_RTPS_READER_STATELESSREADER_H_

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastrtps/utils/collections/ResourceLimitedVector.hpp>

#include <mutex>
#include <map>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * Class StatelessReader, specialization of the RTPSReader for Best Effort Readers.
 * @ingroup READER_MODULE
 */
class StatelessReader : public RTPSReader
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
    bool matched_writer_add(
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
            CacheChange_t* change,
            WriterProxy* prox = nullptr) override;

    /**
     * Processes a new DATA message.
     *
     * @param change Pointer to the CacheChange_t.
     * @return true if the reader accepts messages from the.
     */
    bool processDataMsg(
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
    bool processDataFragMsg(
            CacheChange_t* change,
            uint32_t sampleSize,
            uint32_t fragmentStartingNum,
            uint16_t fragmentsInSubmessage) override;

    /**
     * Processes a new HEARTBEAT message.
     *
     * @return true if the reader accepts messages from the.
     */
    bool processHeartbeatMsg(
            const GUID_t& writerGUID,
            uint32_t hbCount,
            const SequenceNumber_t& firstSN,
            const SequenceNumber_t& lastSN,
            bool finalFlag,
            bool livelinessFlag) override;

    bool processGapMsg(
            const GUID_t& writerGUID,
            const SequenceNumber_t& gapStart,
            const SequenceNumberSet_t& gapList) override;

    /**
     * This method is called when a new change is received. This method calls the received_change of the History
     * and depending on the implementation performs different actions.
     * @param a_change Pointer of the change to add.
     * @return True if added.
     */
    bool change_received(
            CacheChange_t* a_change);

    /**
     * Read the next unread CacheChange_t from the history
     * @param change Pointer to pointer of CacheChange_t
     * @param wpout Pointer to pointer of the matched writer proxy
     * @return True if read.
     */
    bool nextUnreadCache(
            CacheChange_t** change,
            WriterProxy** wpout = nullptr) override;

    /**
     * Take the next CacheChange_t from the history;
     * @param change Pointer to pointer of CacheChange_t
     * @param wpout Pointer to pointer of the matched writer proxy
     * @return True if read.
     */
    bool nextUntakenCache(
            CacheChange_t** change,
            WriterProxy** wpout = nullptr) override;

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
    bool isInCleanState() override
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
     * @param [out] wp                Writer proxy the @c change belongs to.
     * @param [out] is_future_change  Whether the change is in the future (i.e. there are
     *                                earlier unreceived changes from the same writer).
     *
     * @return Whether the change is still valid or not.
     */
    bool begin_sample_access_nts(
            CacheChange_t* change,
            WriterProxy*& wp,
            bool& is_future_change) override;

    /**
     * Called after the change has been deserialized.
     * @param [in] change        Pointer to the change being accessed.
     * @param [in] wp            Writer proxy the @c change belongs to.
     * @param [in] mark_as_read  Whether the @c change should be marked as read or not.
     */
    void end_sample_access_nts(
            CacheChange_t* change,
            WriterProxy*& wp,
            bool mark_as_read) override;

    /**
     * Called when the user has retrieved a change from the history.
     * @param change Pointer to the change to ACK
     * @param writer Writer proxy of the \c change.
     * @param mark_as_read Whether the \c change should be marked as read or not
     */
    void change_read_by_user(
            CacheChange_t* change,
            WriterProxy* writer,
            bool mark_as_read = true) override;

private:

    struct RemoteWriterInfo_t
    {
        GUID_t guid;
        GUID_t persistence_guid;
        bool has_manual_topic_liveliness = false;
        CacheChange_t* fragmented_change = nullptr;
        bool is_datasharing = false;
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

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif /* _FASTDDS_RTPS_READER_STATELESSREADER_H_ */
