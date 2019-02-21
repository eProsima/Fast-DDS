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
 * @file StatefulReader.h
 */

#ifndef FASTRTPS_RTPS_READER_STATEFULREADER_H_
#define FASTRTPS_RTPS_READER_STATEFULREADER_H_

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "RTPSReader.h"
#include "../../utils/collections/ResourceLimitedVector.hpp"

#include <mutex>

namespace eprosima {
namespace fastrtps{
namespace rtps {

class WriterProxy;
class RTPSMessageGroup_t;

/**
 * Class StatefulReader, specialization of RTPSReader than stores the state of the matched writers.
 * @ingroup READER_MODULE
 */
class StatefulReader : public RTPSReader
{
    public:

        friend class RTPSParticipantImpl;

        virtual ~StatefulReader();

    protected:

        StatefulReader(
                RTPSParticipantImpl*,
                const GUID_t& guid,
                const ReaderAttributes& att,
                ReaderHistory* hist,
                ReaderListener* listen = nullptr);

    public:

        /**
         * Add a matched writer represented by a WriterProxyData object.
         * @param wdata Pointer to the WPD object to add.
         * @return True if correctly added.
         */
        bool matched_writer_add(const RemoteWriterAttributes& wdata) override;

        /**
         * Remove a WriterProxyData from the matached writers due to liveliness expiration.
         * @param writer_guid GUID of the writer proxy for which livelines expired.
         * @return True if correct.
         */
        bool liveliness_expired(const GUID_t& writer_guid);

        /**
         * Remove a WriterProxyData from the matached writers.
         * @param wdata Pointer to the WPD object.
         * @return True if correct.
         */
        bool matched_writer_remove(const RemoteWriterAttributes& wdata) override;

        /**
         * Tells us if a specific Writer is matched against this reader
         * @param wdata Pointer to the WriterProxyData object
         * @return True if it is matched.
         */
        bool matched_writer_is_matched(const RemoteWriterAttributes& wdata) const override;

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
         * Processes a new DATA message. Previously the message must have been accepted by function acceptMsgDirectedTo.
         * @param change Pointer to the CacheChange_t.
         * @return true if the reader accepts messages.
         */
        bool processDataMsg(CacheChange_t* change) override;

        /**
         * Processes a new DATA FRAG message. Previously the message must have been accepted by function acceptMsgDirectedTo.
         * @param change Pointer to the CacheChange_t.
         * @param sampleSize Size of the complete assembled message.
         * @param fragmentStartingNum fragment number of this particular fragment.
         * @return true if the reader accepts messages.
         */
        bool processDataFragMsg(
                CacheChange_t* change, 
                uint32_t sampleSize, 
                uint32_t fragmentStartingNum) override;

        /**
         * Processes a new HEARTBEAT message. Previously the message must have been accepted by function acceptMsgDirectedTo.
         *
         * @return true if the reader accepts messages.
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
         * Method to indicate the reader that some change has been removed due to HistoryQos requirements.
         * @param change Pointer to the CacheChange_t.
         * @param prox Pointer to the WriterProxy.
         * @return True if correctly removed.
         */
        bool change_removed_by_history(
                CacheChange_t* change,
                WriterProxy* prox = nullptr) override;

        /**
         * This method is called when a new change is received. This method calls the received_change of the History
         * and depending on the implementation performs different actions.
         * @param a_change Pointer of the change to add.
         * @param prox Pointer to the WriterProxy that adds the Change.
         * @param lock mutex protecting the StatefulReader.
         * @return True if added.
         */
        bool change_received(
                CacheChange_t* a_change, 
                WriterProxy* prox);

        /**
         * Get the RTPS participant
         * @return Associated RTPS participant
         */
        inline RTPSParticipantImpl* getRTPSParticipant() const 
        {
            return mp_RTPSParticipant;
        }

        /**
         * Read the next unread CacheChange_t from the history
         * @param change Pointer to pointer of CacheChange_t
         * @param wpout Pointer to pointer the matched writer proxy
         * @return True if read.
         */
        bool nextUnreadCache(
                CacheChange_t** change,
                WriterProxy** wpout = nullptr) override;

        /**
         * Take the next CacheChange_t from the history;
         * @param change Pointer to pointer of CacheChange_t
         * @param wpout Pointer to pointer the matched writer proxy
         * @return True if read.
         */
        bool nextUntakenCache(
                CacheChange_t** change,
                WriterProxy** wpout = nullptr) override;

        /**
         * Update the times parameters of the Reader.
         * @param times ReaderTimes reference.
         * @return True if correctly updated.
         */
        bool updateTimes(const ReaderTimes& times);

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
        bool isInCleanState() const override;

        /**
         * Sends an acknack message from this reader.
         * @param sns Sequence number bitmap with the acknack information.
         * @param buffer Message buffer to use for serialization.
         * @param locators List of destination locators.
         * @param guids List of destination writer GUIDs.
         * @param is_final Value for final flag.
         */
        void send_acknack(
                const SequenceNumberSet_t& sns,
                RTPSMessageGroup_t& buffer,
                const LocatorList_t& locators,
                const std::vector<GUID_t>& guids,
                bool is_final);

        /**
         * Sends an acknack message from this reader in response to a heartbeat.
         * @param writer Pointer to the proxy representing the writer to send the acknack to.
         * @param buffer Message buffer to use for serialization.
         * @param locators List of destination locators.
         * @param guids List of destination writer GUIDs.
         * @param heartbeat_was_final Final flag of the last received heartbeat.
         */
        void send_acknack(
                const WriterProxy* writer,
                RTPSMessageGroup_t& buffer,
                const LocatorList_t& locators,
                const std::vector<GUID_t>& guids,
                bool heartbeat_was_final);

    private:

        bool acceptMsgFrom(
                const GUID_t& entityGUID,
                WriterProxy** wp) const;

        /*!
         * @remarks Nn thread-safe.
         */
        bool findWriterProxy(
                const GUID_t& writerGUID, 
                WriterProxy** wp) const;

        void NotifyChanges(WriterProxy* wp);

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
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif

#endif /* FASTRTPS_RTPS_READER_STATEFULREADER_H_ */
