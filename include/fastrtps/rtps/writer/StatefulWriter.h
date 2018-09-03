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
 * @file StatefulWriter.h
 *
 */

#ifndef STATEFULWRITER_H_
#define STATEFULWRITER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "RTPSWriter.h"
#include "timedevent/PeriodicHeartbeat.h"
#include <condition_variable>
#include <mutex>

namespace eprosima
{
    namespace fastrtps
    {
        namespace rtps
        {
            class ReaderProxy;

            /**
             * Class StatefulWriter, specialization of RTPSWriter that maintains information of each matched Reader.
             * @ingroup WRITER_MODULE
             */
            class StatefulWriter: public RTPSWriter
            {
                friend class RTPSParticipantImpl;
                friend class ReaderProxy;
                public:

                //!Destructor
                virtual ~StatefulWriter();

                //!Timed Event to manage the periodic HB to the Reader.
                // TODO Change to public because a bug. Refactor.
                PeriodicHeartbeat* mp_periodicHB;

                protected:
                //!Constructor
                StatefulWriter(RTPSParticipantImpl*,GUID_t& guid,WriterAttributes& att,WriterHistory* hist,WriterListener* listen=nullptr);
                private:
                //!Count of the sent heartbeats.
                Count_t m_heartbeatCount;
                //!WriterTimes
                WriterTimes m_times;

                //! Vector containin all the associated ReaderProxies.
                std::vector<ReaderProxy*> matched_readers;
                //!EntityId used to send the HB.(only for builtin types performance)
                EntityId_t m_HBReaderEntityId;
                // TODO Join this mutex when main mutex would not be recursive.
                std::mutex all_acked_mutex_;
                std::condition_variable all_acked_cond_;
                // TODO Also remove when main mutex not recursive.
                bool all_acked_;
                std::mutex may_remove_change_mutex_;
                std::condition_variable may_remove_change_cond_;
                unsigned int may_remove_change_;

                public:
                /**
                 * Add a specific change to all ReaderLocators.
                 * @param p Pointer to the change.
                 */
                void unsent_change_added_to_history(CacheChange_t* p) override;
                /**
                 * Indicate the writer that a change has been removed by the history due to some HistoryQos requirement.
                 * @param a_change Pointer to the change that is going to be removed.
                 * @return True if removed correctly.
                 */
                bool change_removed_by_history(CacheChange_t* a_change) override;
                /**
                 * Method to indicate that there are changes not sent in some of all ReaderProxy.
                 */
                void send_any_unsent_changes() override;

                //!Increment the HB count.
                inline void incrementHBCount(){ ++m_heartbeatCount; };
                /**
                 * Add a matched reader.
                 * @param ratt Attributes of the reader to add.
                 * @return True if added.
                 */
                bool matched_reader_add(const RemoteReaderAttributes& ratt) override;
                /**
                 * Remove a matched reader.
                 * @param ratt Attributes of the reader to remove.
                 * @return True if removed.
                 */
                bool matched_reader_remove(const RemoteReaderAttributes& ratt) override;
                /**
                 * Tells us if a specific Reader is matched against this writer
                 * @param ratt Attributes of the reader to remove.
                 * @return True if it was matched.
                 */
                bool matched_reader_is_matched(const RemoteReaderAttributes& ratt) override;

                bool is_acked_by_all(const CacheChange_t* a_change) const override;

                bool wait_for_all_acked(const Duration_t& max_wait) override;

                /**
                * Remove the change with the minimum SequenceNumber
                * @return True if removed.
                */
                bool try_remove_change(std::chrono::microseconds& microseconds,
                        std::unique_lock<std::recursive_mutex>& lock) override;

                /**
                 * Update the Attributes of the Writer.
                 * @param att New attributes
                 */
                void updateAttributes(WriterAttributes& att) override;

                /**
                 * Find a Reader Proxy in this writer.
                 * @param[in] readerGuid The GUID_t of the reader.
                 * @param[out] RP Pointer to pointer to return the ReaderProxy.
                 * @return True if correct.
                 */
                bool matched_reader_lookup(GUID_t& readerGuid,ReaderProxy** RP);

                /** Get count of heartbeats
                 * @return count of heartbeats
                 */
                inline Count_t getHeartbeatCount() const {return this->m_heartbeatCount;};

                /** Get heartbeat reader entity id
                 * @return heartbeat reader entity id
                 */
                inline EntityId_t getHBReaderEntityId() {return this->m_HBReaderEntityId;};

                /**
                 * Get the begin of the matched readers
                 * @return A vector iterator pointing to de begin of the matched readers list
                 */
                inline std::vector<ReaderProxy*>::iterator matchedReadersBegin(){return this->matched_readers.begin();};

                /**
                 * Get the end of the matched readers
                 * @return A vector iterator pointing to de end of the matched readers list
                 */
                inline std::vector<ReaderProxy*>::iterator matchedReadersEnd(){return this->matched_readers.end();};

                /**
                 * Get the RTPS participant
                 * @return RTPS participant
                 */
                inline RTPSParticipantImpl* getRTPSParticipant() const {return mp_RTPSParticipant;}

                /**
                 * Get the number of matched readers
                 * @return Number of the matched readers
                 */
                inline size_t getMatchedReadersSize() const {return matched_readers.size();};

                /**
                 * Update the WriterTimes attributes of all associated ReaderProxy.
                 * @param times WriterTimes parameter.
                 */
                void updateTimes(WriterTimes& times);

                void add_flow_controller(std::unique_ptr<FlowController> controller) override;

                SequenceNumber_t next_sequence_number() const;

                /*!
                 * @brief Sends a heartbeat to a remote reader.
                 * @remarks This function is non thread-safe.
                 */
                void send_heartbeat_to_nts(ReaderProxy& remoteReaderProxy, bool final = false,
                        bool send_empty_history_info = false);

                void process_acknack(const GUID_t reader_guid, uint32_t ack_count,
                        const SequenceNumberSet_t& sn_set, bool final_flag);

                private:

                void send_heartbeat_nts_(const std::vector<GUID_t>& remote_readers, const LocatorList_t& locators,
                        RTPSMessageGroup& message_group, bool final = false, bool send_empty_history_info = false);

                void check_acked_status();

                bool disableHeartbeatPiggyback_;

                const uint32_t sendBufferSize_;

                int32_t currentUsageSendBufferSize_;

                std::vector<std::unique_ptr<FlowController> > m_controllers;

                StatefulWriter& operator=(const StatefulWriter&) = delete;
            };
        } /* namespace rtps */
    } /* namespace fastrtps */
} /* namespace eprosima */
#endif
#endif /* STATEFULWRITER_H_ */
