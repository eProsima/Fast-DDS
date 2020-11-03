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
 * @file RTPSReader.h
 */

#ifndef _FASTDDS_RTPS_READER_RTPSREADER_H_
#define _FASTDDS_RTPS_READER_RTPSREADER_H_

#include <fastdds/rtps/Endpoint.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/common/SequenceNumber.h>
#include <fastrtps/qos/LivelinessChangedStatus.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/utils/TimedConditionVariable.hpp>
#include "../history/ReaderHistory.h"

namespace eprosima {
namespace fastrtps {
namespace rtps {

// Forward declarations
class LivelinessManager;
class ReaderListener;
class WriterProxy;
struct CacheChange_t;
struct ReaderHistoryState;
class WriterProxyData;

/**
 * Class RTPSReader, manages the reception of data from its matched writers.
 * @ingroup READER_MODULE
 */
class RTPSReader : public Endpoint
{
    friend class ReaderHistory;
    friend class RTPSParticipantImpl;
    friend class MessageReceiver;
    friend class EDP;
    friend class WLP;

protected:

    RTPSReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            ReaderHistory* hist,
            ReaderListener* listen = nullptr);

    RTPSReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            ReaderHistory* hist,
            ReaderListener* listen = nullptr);

    RTPSReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            const std::shared_ptr<IChangePool>& change_pool,
            ReaderHistory* hist,
            ReaderListener* listen = nullptr);

    virtual ~RTPSReader();

public:

    /**
     * Add a matched writer represented by its attributes.
     * @param wdata Attributes of the writer to add.
     * @return True if correctly added.
     */
    RTPS_DllAPI virtual bool matched_writer_add(
            const WriterProxyData& wdata) = 0;

    /**
     * Remove a writer represented by its attributes from the matched writers.
     * @param writer_guid GUID of the writer to remove.
     * @param removed_by_lease Whether the writer is being unmatched due to a participant drop.
     * @return True if correctly removed.
     */
    RTPS_DllAPI virtual bool matched_writer_remove(
            const GUID_t& writer_guid,
            bool removed_by_lease = false) = 0;

    /**
     * Tells us if a specific Writer is matched against this reader.
     * @param writer_guid GUID of the writer to check.
     * @return True if it is matched.
     */
    RTPS_DllAPI virtual bool matched_writer_is_matched(
            const GUID_t& writer_guid) = 0;

    /**
     * Processes a new DATA message. Previously the message must have been accepted by function acceptMsgDirectedTo.
     *
     * @param change Pointer to the CacheChange_t.
     * @return true if the reader accepts messages from the.
     */
    RTPS_DllAPI virtual bool processDataMsg(
            CacheChange_t* change) = 0;

    /**
     * Processes a new DATA FRAG message.
     *
     * @param change Pointer to the CacheChange_t.
     * @param sampleSize Size of the complete, assembled message.
     * @param fragmentStartingNum Starting number of this particular message.
     * @param fragmentsInSubmessage Number of fragments on this particular message.
     * @return true if the reader accepts message.
     */
    RTPS_DllAPI virtual bool processDataFragMsg(
            CacheChange_t* change,
            uint32_t sampleSize,
            uint32_t fragmentStartingNum,
            uint16_t fragmentsInSubmessage) = 0;

    /**
     * Processes a new HEARTBEAT message.
     * @param writerGUID
     * @param hbCount
     * @param firstSN
     * @param lastSN
     * @param finalFlag
     * @param livelinessFlag
     * @return true if the reader accepts messages from the.
     */
    RTPS_DllAPI virtual bool processHeartbeatMsg(
            const GUID_t& writerGUID,
            uint32_t hbCount,
            const SequenceNumber_t& firstSN,
            const SequenceNumber_t& lastSN,
            bool finalFlag,
            bool livelinessFlag) = 0;

    /**
     * Processes a new GAP message.
     * @param writerGUID
     * @param gapStart
     * @param gapList
     * @return true if the reader accepts messages from the.
     */
    RTPS_DllAPI virtual bool processGapMsg(
            const GUID_t& writerGUID,
            const SequenceNumber_t& gapStart,
            const SequenceNumberSet_t& gapList) = 0;

    /**
     * Method to indicate the reader that some change has been removed due to HistoryQos requirements.
     * @param change Pointer to the CacheChange_t.
     * @param prox Pointer to the WriterProxy.
     * @return True if correctly removed.
     */
    RTPS_DllAPI virtual bool change_removed_by_history(
            CacheChange_t* change,
            WriterProxy* prox = nullptr) = 0;

    /**
     * Get the associated listener, secondary attached Listener in case it is of coumpound type
     * @return Pointer to the associated reader listener.
     */
    RTPS_DllAPI ReaderListener* getListener() const;

    /**
     * Switch the ReaderListener kind for the Reader.
     * If the RTPSReader does not belong to the built-in protocols it switches out the old one.
     * If it belongs to the built-in protocols, it sets the new ReaderListener callbacks to be called after the
     * built-in ReaderListener ones.
     * @param target Pointed to ReaderLister to attach
     * @return True is correctly set.
     */
    RTPS_DllAPI bool setListener(
            ReaderListener* target);

    /**
     * Reserve a CacheChange_t.
     * @param change Pointer to pointer to the Cache.
     * @param dataCdrSerializedSize Size of the Cache.
     * @return True if correctly reserved.
     */
    RTPS_DllAPI bool reserveCache(
            CacheChange_t** change,
            uint32_t dataCdrSerializedSize);

    /**
     * Release a cacheChange.
     */
    RTPS_DllAPI void releaseCache(
            CacheChange_t* change);

    /**
     * Read the next unread CacheChange_t from the history
     * @param change Pointer to pointer of CacheChange_t
     * @param wp Pointer to pointer to the WriterProxy
     * @return True if read.
     */
    RTPS_DllAPI virtual bool nextUnreadCache(
            CacheChange_t** change,
            WriterProxy** wp) = 0;

    /**
     * Get the next CacheChange_t from the history to take.
     * @param change Pointer to pointer of CacheChange_t.
     * @param wp Pointer to pointer to the WriterProxy.
     * @return True if read.
     */
    RTPS_DllAPI virtual bool nextUntakenCache(
            CacheChange_t** change,
            WriterProxy** wp) = 0;

    RTPS_DllAPI bool wait_for_unread_cache(
            const eprosima::fastrtps::Duration_t& timeout);

    RTPS_DllAPI uint64_t get_unread_count() const;

    /**
     * @return True if the reader expects Inline QOS.
     */
    RTPS_DllAPI inline bool expectsInlineQos()
    {
        return m_expectsInlineQos;
    }

    //! Returns a pointer to the associated History.
    RTPS_DllAPI inline ReaderHistory* getHistory()
    {
        return mp_history;
    }

    /*!
     * @brief Returns there is a clean state with all Writers.
     * It occurs when the Reader received all samples sent by Writers. In other words,
     * its WriterProxies are up to date.
     * @return There is a clean state with all Writers.
     */
    virtual bool isInCleanState() = 0;

    //! The liveliness changed status struct as defined in the DDS
    LivelinessChangedStatus liveliness_changed_status_;

    inline void enableMessagesFromUnkownWriters(
            bool enable)
    {
        m_acceptMessagesFromUnkownWriters = enable;
    }

    void setTrustedWriter(
            const EntityId_t& writer)
    {
        m_acceptMessagesFromUnkownWriters = false;
        m_trustedWriterEntityId = writer;
    }

protected:

    virtual bool may_remove_history_record(
            bool removed_by_lease);

    /*!
     * @brief Add a remote writer to the persistence_guid map
     * @param guid GUID of the remote writer
     * @param persistence_guid Persistence GUID of the remote writer
     */
    void add_persistence_guid(
            const GUID_t& guid,
            const GUID_t& persistence_guid);

    /*!
     * @brief Remove a remote writer from the persistence_guid map
     * @param guid GUID of the remote writer
     * @param persistence_guid Persistence GUID of the remote writer
     * @param removed_by_lease Whether the GUIDs are being removed due to a participant drop.
     */
    void remove_persistence_guid(
            const GUID_t& guid,
            const GUID_t& persistence_guid,
            bool removed_by_lease);

    /*!
     * @brief Get the last notified sequence for a RTPS guid
     * @param guid The RTPS guid to query
     * @return Last notified sequence number for input guid
     * @remarks Takes persistence_guid into consideration
     */
    SequenceNumber_t get_last_notified(
            const GUID_t& guid);

    /*!
     * @brief Update the last notified sequence for a RTPS guid
     * @param guid The RTPS guid of the writer
     * @param seq Max sequence number available on writer
     * @return Previous value of last notified sequence number for input guid
     * @remarks Takes persistence_guid into consideration
     */
    SequenceNumber_t update_last_notified(
            const GUID_t& guid,
            const SequenceNumber_t& seq);

    /*!
     * @brief Set the last notified sequence for a persistence guid
     * @param persistence_guid The persistence guid to update
     * @param seq Sequence number to set for input guid
     * @remarks Persistent readers will write to DB
     */
    virtual void set_last_notified(
            const GUID_t& persistence_guid,
            const SequenceNumber_t& seq);

    /*!
     * @brief Search if there is a CacheChange_t, giving SequenceNumber_t and writer GUID_t,
     * waiting to be completed because it is fragmented.
     * @param sequence_number SequenceNumber_t of the searched CacheChange_t.
     * @param writer_guid writer GUID_t of the searched CacheChange_t.
     * @param change If a CacheChange_t was found, this argument will fill with its pointer.
     * In other case nullptr is returned.
     * @param hint Iterator since the search will start.
     * Used to improve the search.
     * @return Iterator pointing to the position were CacheChange_t was found.
     * It can be used to improve next search.
     */
    History::const_iterator findCacheInFragmentedProcess(
            const SequenceNumber_t& sequence_number,
            const GUID_t& writer_guid,
            CacheChange_t** change,
            History::const_iterator hint) const;

    //!ReaderHistory
    ReaderHistory* mp_history;
    //!Listener
    ReaderListener* mp_listener;
    //!Accept msg to unknwon readers (default=true)
    bool m_acceptMessagesToUnknownReaders;
    //!Accept msg from unknwon writers (BE-true,RE-false)
    bool m_acceptMessagesFromUnkownWriters;
    //!Trusted writer (for Builtin)
    EntityId_t m_trustedWriterEntityId;
    //!Expects Inline Qos.
    bool m_expectsInlineQos;

    //!ReaderHistoryState
    ReaderHistoryState* history_state_;

    uint64_t total_unread_ = 0;

    TimedConditionVariable new_notification_cv_;

    //! The liveliness kind of this reader
    LivelinessQosPolicyKind liveliness_kind_;
    //! The liveliness lease duration of this reader
    Duration_t liveliness_lease_duration_;

private:

    RTPSReader& operator =(
            const RTPSReader&) = delete;

    void init(
            const std::shared_ptr<IPayloadPool>& payload_pool,
            const std::shared_ptr<IChangePool>& change_pool);

};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_READER_RTPSREADER_H_ */
