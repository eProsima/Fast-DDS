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
 * @file RTPSWriter.h
 */

#ifndef RTPSWRITER_H_
#define RTPSWRITER_H_

#include "../Endpoint.h"
#include "../messages/RTPSMessageGroup.h"
#include "../attributes/WriterAttributes.h"
#include "../../utils/collections/ResourceLimitedVector.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <chrono>

namespace eprosima {
namespace fastrtps{
namespace rtps {

class WriterListener;
class WriterHistory;
class FlowController;
struct CacheChange_t;


/**
 * Class RTPSWriter, manages the sending of data to the readers. Is always associated with a HistoryCache.
 * @ingroup WRITER_MODULE
 */
class RTPSWriter : public Endpoint
{
    friend class WriterHistory;
    friend class RTPSParticipantImpl;
    friend class RTPSMessageGroup;
    protected:
    RTPSWriter(
            RTPSParticipantImpl*,
            const GUID_t& guid,
            const WriterAttributes& att,
            WriterHistory* hist,
            WriterListener* listen=nullptr);
    virtual ~RTPSWriter();

    public:

    /**
     * Create a new change based with the provided changeKind.
     * @param changeKind The type of change.
     * @param handle InstanceHandle to assign.
     * @return Pointer to the CacheChange or nullptr if incorrect.
     */
    template<typename T>
    CacheChange_t* new_change(T &data, ChangeKind_t changeKind, InstanceHandle_t handle = c_InstanceHandle_Unknown)
    {
        return new_change([data]() -> uint32_t {return (uint32_t)T::getCdrSerializedSize(data);}, changeKind, handle);
    }


    RTPS_DllAPI CacheChange_t* new_change(const std::function<uint32_t()>& dataCdrSerializedSize,
            ChangeKind_t changeKind, InstanceHandle_t handle = c_InstanceHandle_Unknown);

    /**
     * Add a matched reader.
     * @param data Pointer to the ReaderProxyData object added.
     * @return True if added.
     */
    RTPS_DllAPI virtual bool matched_reader_add(const ReaderProxyData& data) = 0;

    /**
     * Add a matched reader.
     * @param ratt Pointer to the ReaderProxyData object added.
     * @return True if added.
     */
    RTPS_DllAPI virtual bool matched_reader_add(RemoteReaderAttributes& ratt) = 0;

    /**
     * Remove a matched reader.
     * @param reader_guid GUID of the reader to remove.
     * @return True if removed.
     */
    RTPS_DllAPI virtual bool matched_reader_remove(const GUID_t& reader_guid) = 0;
    /**
     * Tells us if a specific Reader is matched against this writer.
     * @param reader_guid GUID of the reader to check.
     * @return True if it was matched.
     */
    RTPS_DllAPI virtual bool matched_reader_is_matched(const GUID_t& reader_guid) = 0;
    /**
    * Check if a specific change has been acknowledged by all Readers.
    * Is only useful in reliable Writer. In BE Writers returns false when pending to be sent.
    * @return True if acknowledged by all.
    */
    RTPS_DllAPI virtual bool is_acked_by_all(const CacheChange_t* /*a_change*/) const { return false; }

    RTPS_DllAPI virtual bool wait_for_all_acked(const Duration_t& /*max_wait*/){ return true; }

    /**
     * Update the Attributes of the Writer.
     * @param att New attributes
     */
    RTPS_DllAPI virtual void updateAttributes(const WriterAttributes& att) = 0;
    /**
     * This method triggers the send operation for unsent changes.
     * @return number of messages sent
     */
    RTPS_DllAPI virtual void send_any_unsent_changes() = 0;

    /**
     * Get Min Seq Num in History.
     * @return Minimum sequence number in history
     */
    RTPS_DllAPI SequenceNumber_t get_seq_num_min();

    /**
     * Get Max Seq Num in History.
     * @return Maximum sequence number in history
     */
    RTPS_DllAPI SequenceNumber_t get_seq_num_max();

    /**
     * Get maximum size of the serialized type
     * @return Maximum size of the serialized type
     */
    RTPS_DllAPI uint32_t getTypeMaxSerialized();

    uint32_t getMaxDataSize();

    uint32_t calculateMaxDataSize(uint32_t length);

    /**
     * Get listener
     * @return Listener
     */
    RTPS_DllAPI inline WriterListener* getListener(){ return mp_listener; };

    /**
     * Get the asserted liveliness
     * @return Asserted liveliness
     */
    RTPS_DllAPI inline bool getLivelinessAsserted() { return m_livelinessAsserted; };

    /**
     * Get the asserted liveliness
     * @return asserted liveliness
     */
    RTPS_DllAPI inline void setLivelinessAsserted(bool l){ m_livelinessAsserted = l; };

    /**
     * Get the publication mode
     * @return publication mode
     */
    RTPS_DllAPI inline bool isAsync() const { return is_async_; };

    /**
     * Remove an specified max number of changes
     * @return at least one change has been removed
     */
    RTPS_DllAPI bool remove_older_changes(unsigned int max = 0);

    virtual bool try_remove_change(std::chrono::microseconds& microseconds, std::unique_lock<std::recursive_mutex>& lock) = 0;

    /*
     * Adds a flow controller that will apply to this writer exclusively.
     */
    virtual void add_flow_controller(std::unique_ptr<FlowController> controller) = 0;

    /**
     * Get RTPS participant
     * @return RTPS participant
     */
    inline RTPSParticipantImpl* getRTPSParticipant() const {return mp_RTPSParticipant;}

    /**
     * Enable or disable sending data to readers separately
     * NOTE: This will only work for synchronous writers
     * @param enable If separate sending should be enabled
     */
    void set_separate_sending (bool enable) { m_separateSendingEnabled = enable; }

    /**
     * Inform if data is sent to readers separatedly
     * @return true if separate sending is enabled
     */
    bool get_separate_sending () const { return m_separateSendingEnabled; }

    /**
     * Process an incoming ACKNACK submessage.
     * @param writer_guid[in]      GUID of the writer the submessage is directed to.
     * @param reader_guid[in]      GUID of the reader originating the submessage.
     * @param ack_count[in]        Count field of the submessage.
     * @param sn_set[in]           Sequence number bitmap field of the submessage.
     * @param final_flag[in]       Final flag field of the submessage.
     * @param result[out]          true if the writer could process the submessage. 
     *                             Only valid when returned value is true.
     * @return true when the submessage was destinated to this writer, false otherwise.
     */
    virtual bool process_acknack(
            const GUID_t& writer_guid, 
            const GUID_t& reader_guid,
            uint32_t ack_count,
            const SequenceNumberSet_t& sn_set,
            bool final_flag,
            bool &result)
    {
        (void)reader_guid; (void)ack_count; (void)sn_set; (void)final_flag;

        result = false;
        return writer_guid == m_guid;
    }

    /**
     * Process an incoming NACKFRAG submessage.
     * @param writer_guid[in]      GUID of the writer the submessage is directed to.
     * @param reader_guid[in]      GUID of the reader originating the submessage.
     * @param ack_count[in]        Count field of the submessage.
     * @param seq_num[in]          Sequence number field of the submessage.
     * @param fragments_state[in]  Fragment number bitmap field of the submessage.
     * @param result[out]          true if the writer could process the submessage. 
     *                             Only valid when returned value is true.
     * @return true when the submessage was destinated to this writer, false otherwise.
     */
    virtual bool process_nack_frag(
            const GUID_t& writer_guid, 
            const GUID_t& reader_guid,
            uint32_t ack_count, 
            const SequenceNumber_t& seq_num, 
            const FragmentNumberSet_t fragments_state, 
            bool& result)
    {
        (void)reader_guid; (void)ack_count; (void)seq_num; (void)fragments_state;

        result = false;
        return writer_guid == m_guid;
    }

    protected:

    //!Is the data sent directly or announced by HB and THEN send to the ones who ask for it?.
    bool m_pushMode;
    //!Group created to send messages more efficiently
    RTPSMessageGroup_t m_cdrmessages;
    //!INdicates if the liveliness has been asserted
    bool m_livelinessAsserted;
    //!WriterHistory
    WriterHistory* mp_history;
    //!Listener
    WriterListener* mp_listener;
    //!Asynchronous publication activated
    bool is_async_;
    //!Separate sending activated
    bool m_separateSendingEnabled;

    LocatorList_t mAllShrinkedLocatorList;

    ResourceLimitedVector<GUID_t> all_remote_readers_;

    void update_cached_info_nts(std::vector<LocatorList_t>& allLocatorLists);

    /**
     * Initialize the header of hte CDRMessages.
     */
    void init_header();

    /**
     * Add a change to the unsent list.
     * @param change Pointer to the change to add.
     */
    virtual void unsent_change_added_to_history(CacheChange_t* change)=0;

    /**
     * Indicate the writer that a change has been removed by the history due to some HistoryQos requirement.
     * @param a_change Pointer to the change that is going to be removed.
     * @return True if removed correctly.
     */
    virtual bool change_removed_by_history(CacheChange_t* a_change)=0;

#if HAVE_SECURITY
    SerializedPayload_t encrypt_payload_;

    bool encrypt_cachechange(CacheChange_t* change);
#endif

    private:

    RTPSWriter& operator=(const RTPSWriter&) = delete;
};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSWRITER_H_ */
