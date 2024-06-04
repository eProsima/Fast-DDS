// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BaseReader.hpp
 */

#ifndef RTPS_READER__BASEREADER_HPP
#define RTPS_READER__BASEREADER_HPP

#include <cstdint>
#include <memory>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/status/LivelinessChangedStatus.hpp>
#include <fastdds/rtps/Endpoint.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/SequenceNumber.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/rtps/common/VendorId_t.hpp>
#include <fastdds/rtps/history/History.h>
#include <fastdds/rtps/history/IChangePool.h>
#include <fastdds/rtps/history/IPayloadPool.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/statistics/IListeners.hpp>
#include <fastdds/statistics/rtps/StatisticsCommon.hpp>
#include <fastdds/utils/TimedConditionVariable.hpp>

namespace eprosima {

namespace fastrtps {
namespace rtps {

struct CacheChange_t;
class IDataSharingListener;
struct ReaderHistoryState;
class ReaderListener;
class RTPSParticipantImpl;
class WriterProxy;
class WriterProxyData;

} // namespace rtps
} // namespace fastrtps

namespace fastdds {
namespace rtps {

class BaseReader
    : public fastrtps::rtps::RTPSReader
    , public fastdds::statistics::StatisticsReaderImpl
{

public:

    /**
     * Get the associated listener, secondary attached Listener in case it is of compound type
     * @return Pointer to the associated reader listener.
     */
    fastrtps::rtps::ReaderListener* get_listener() const override;

    /**
     * Switch the ReaderListener kind for the Reader.
     * If the RTPSReader does not belong to the built-in protocols it switches out the old one.
     * If it belongs to the built-in protocols, it sets the new ReaderListener callbacks to be called after the
     * built-in ReaderListener ones.
     * @param target Pointed to ReaderLister to attach
     * @return True is correctly set.
     */
    bool set_listener(
            fastrtps::rtps::ReaderListener* target) override;

    /**
     * @return True if the reader expects Inline QOS.
     */
    bool expects_inline_qos() const override;

    //! Returns a pointer to the associated History.
    fastrtps::rtps::ReaderHistory* get_history() const override;

    //! @return The content filter associated to this reader.
    IReaderDataFilter* get_content_filter() const override;

    //! Set the content filter associated to this reader.
    //! @param filter Pointer to the content filter to associate to this reader.
    void set_content_filter(
            IReaderDataFilter* filter) override;

    uint64_t get_unread_count() const override;

    uint64_t get_unread_count(
            bool mark_as_read) override;

    bool wait_for_unread_cache(
            const fastrtps::Duration_t& timeout) override;

    bool is_sample_valid(
            const void* data,
            const fastrtps::rtps::GUID_t& writer,
            const fastrtps::rtps::SequenceNumber_t& sn) const override;

    static BaseReader* downcast(
            fastrtps::rtps::RTPSReader* reader);

    static BaseReader* downcast(
            fastrtps::rtps::Endpoint* endpoint);

    void set_trusted_writer(
            const fastrtps::rtps::EntityId_t& writer)
    {
        accept_messages_from_unkown_writers_ = false;
        trusted_writer_entity_id_ = writer;
    }

    /**
     * Whether the reader accepts messages directed to unknown readers.
     *
     * @return true if the reader accepts messages directed to unknown readers, false otherwise.
     */
    bool accept_messages_to_unknown_readers() const
    {
        return accept_messages_to_unknown_readers_;
    }

    //! @return The liveliness kind of this reader
    fastdds::dds::LivelinessQosPolicyKind liveliness_kind() const
    {
        return liveliness_kind_;
    }

    //! @return The liveliness lease duration of this reader
    fastrtps::Duration_t liveliness_lease_duration() const
    {
        return liveliness_lease_duration_;
    }

    const std::unique_ptr<fastrtps::rtps::IDataSharingListener>& datasharing_listener() const
    {
        return datasharing_listener_;
    }

    /**
     * Reserve a CacheChange_t.
     * @param change Pointer to pointer to the Cache.
     * @param dataCdrSerializedSize Size of the Cache.
     * @return True if correctly reserved.
     */
    bool reserve_cache(
            fastrtps::rtps::CacheChange_t** change,
            uint32_t dataCdrSerializedSize);

    /**
     * Release a cacheChange.
     */
    void release_cache(
            fastrtps::rtps::CacheChange_t* change);

    /**
     * Method to indicate the reader that some change has been removed due to HistoryQos requirements.
     * @param change Pointer to the CacheChange_t.
     * @param prox Pointer to the WriterProxy.
     * @return True if correctly removed.
     */
    virtual bool change_removed_by_history(
            fastrtps::rtps::CacheChange_t* change,
            fastrtps::rtps::WriterProxy* prox = nullptr) = 0;

    /**
     * Called just before a change is going to be deserialized.
     * @param [in]  change            Pointer to the change being accessed.
     * @param [out] writer            Writer proxy the @c change belongs to.
     * @param [out] is_future_change  Whether the change is in the future (i.e. there are
     *                                earlier unreceived changes from the same writer).
     *
     * @return Whether the change is still valid or not.
     */
    virtual bool begin_sample_access_nts(
            fastrtps::rtps::CacheChange_t* change,
            fastrtps::rtps::WriterProxy*& writer,
            bool& is_future_change) = 0;

    /**
     * Called after the change has been deserialized.
     * @param [in] change        Pointer to the change being accessed.
     * @param [in] writer        Writer proxy the @c change belongs to.
     * @param [in] mark_as_read  Whether the @c change should be marked as read or not.
     */
    virtual void end_sample_access_nts(
            fastrtps::rtps::CacheChange_t* change,
            fastrtps::rtps::WriterProxy*& writer,
            bool mark_as_read) = 0;

    /**
     * @brief A method to update the liveliness changed status of the reader
     * @param writer The writer changing liveliness, specified by its guid
     * @param alive_change The change requested for alive count. Should be -1, 0 or +1
     * @param not_alive_change The change requested for not alive count. Should be -1, 0 or +1
     */
    void update_liveliness_changed_status(
            fastrtps::rtps::GUID_t writer,
            int32_t alive_change,
            int32_t not_alive_change);

    /**
     * Processes a new DATA message. Previously the message must have been accepted by function acceptMsgDirectedTo.
     *
     * @param change Pointer to the CacheChange_t.
     * @return true if the reader accepts messages from the.
     */
    virtual bool process_data_msg(
            fastrtps::rtps::CacheChange_t* change) = 0;

    /**
     * Processes a new DATA FRAG message.
     *
     * @param change Pointer to the CacheChange_t.
     * @param sampleSize Size of the complete, assembled message.
     * @param fragmentStartingNum Starting number of this particular message.
     * @param fragmentsInSubmessage Number of fragments on this particular message.
     * @return true if the reader accepts message.
     */
    virtual bool process_data_frag_msg(
            fastrtps::rtps::CacheChange_t* change,
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
     * @param origin_vendor_id
     * @return true if the reader accepts messages from the.
     */
    virtual bool process_heartbeat_msg(
            const fastrtps::rtps::GUID_t& writerGUID,
            uint32_t hbCount,
            const fastrtps::rtps::SequenceNumber_t& firstSN,
            const fastrtps::rtps::SequenceNumber_t& lastSN,
            bool finalFlag,
            bool livelinessFlag,
            VendorId_t origin_vendor_id = c_VendorId_Unknown) = 0;

    /**
     * Processes a new GAP message.
     * @param writerGUID
     * @param gapStart
     * @param gapList
     * @param origin_vendor_id
     * @return true if the reader accepts messages from the.
     */
    virtual bool process_gap_msg(
            const fastrtps::rtps::GUID_t& writerGUID,
            const fastrtps::rtps::SequenceNumber_t& gapStart,
            const fastrtps::rtps::SequenceNumberSet_t& gapList,
            VendorId_t origin_vendor_id = c_VendorId_Unknown) = 0;

#ifdef FASTDDS_STATISTICS

    bool add_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener) override;

    bool remove_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener) override;

    void set_enabled_statistics_writers_mask(
            uint32_t enabled_writers) override;

#endif // FASTDDS_STATISTICS

    virtual ~BaseReader();

protected:

    BaseReader(
            fastrtps::rtps::RTPSParticipantImpl* pimpl,
            const fastrtps::rtps::GUID_t& guid,
            const fastrtps::rtps::ReaderAttributes& att,
            fastrtps::rtps::ReaderHistory* hist,
            fastrtps::rtps::ReaderListener* listen);

    BaseReader(
            fastrtps::rtps::RTPSParticipantImpl* pimpl,
            const fastrtps::rtps::GUID_t& guid,
            const fastrtps::rtps::ReaderAttributes& att,
            const std::shared_ptr<fastrtps::rtps::IPayloadPool>& payload_pool,
            fastrtps::rtps::ReaderHistory* hist,
            fastrtps::rtps::ReaderListener* listen);

    BaseReader(
            fastrtps::rtps::RTPSParticipantImpl* pimpl,
            const fastrtps::rtps::GUID_t& guid,
            const fastrtps::rtps::ReaderAttributes& att,
            const std::shared_ptr<fastrtps::rtps::IPayloadPool>& payload_pool,
            const std::shared_ptr<fastrtps::rtps::IChangePool>& change_pool,
            fastrtps::rtps::ReaderHistory* hist,
            fastrtps::rtps::ReaderListener* listen);

    /*!
     * @brief Whether a history record may be removed.
     * @param removed_by_lease Whether the GUIDs are being removed due to a participant drop.
     * @return Whether the history record may be removed.
     */
    virtual bool may_remove_history_record(
            bool removed_by_lease);

    /*!
     * @brief Add a remote writer to the persistence_guid map
     * @param guid GUID of the remote writer
     * @param persistence_guid Persistence GUID of the remote writer
     */
    void add_persistence_guid(
            const fastrtps::rtps::GUID_t& guid,
            const fastrtps::rtps::GUID_t& persistence_guid);

    /*!
     * @brief Remove a remote writer from the persistence_guid map
     * @param guid GUID of the remote writer
     * @param persistence_guid Persistence GUID of the remote writer
     * @param removed_by_lease Whether the GUIDs are being removed due to a participant drop.
     */
    void remove_persistence_guid(
            const fastrtps::rtps::GUID_t& guid,
            const fastrtps::rtps::GUID_t& persistence_guid,
            bool removed_by_lease);

    /*!
     * @brief Get the last notified sequence for a RTPS guid
     * @param guid The RTPS guid to query
     * @return Last notified sequence number for input guid
     * @remarks Takes persistence_guid into consideration
     */
    fastrtps::rtps::SequenceNumber_t get_last_notified(
            const fastrtps::rtps::GUID_t& guid);

    /*!
     * @brief Update the last notified sequence for a RTPS guid
     * @param guid The RTPS guid of the writer
     * @param seq Max sequence number available on writer
     * @return Previous value of last notified sequence number for input guid
     * @remarks Takes persistence_guid into consideration
     */
    fastrtps::rtps::SequenceNumber_t update_last_notified(
            const fastrtps::rtps::GUID_t& guid,
            const fastrtps::rtps::SequenceNumber_t& seq);

    /*!
     * @brief Persist the last notified sequence for a persistence guid
     * This method is called inside update_last_notified just after updating the last notified sequence for a writer
     * and gives persistent readers the opportunity to write the new sequence number to the database.
     *
     * @param persistence_guid The persistence guid to update
     * @param seq Sequence number to set for input guid
     */
    virtual void persist_last_notified_nts(
            const fastrtps::rtps::GUID_t& persistence_guid,
            const fastrtps::rtps::SequenceNumber_t& seq);

    bool is_datasharing_compatible_with(
            const fastrtps::rtps::WriterProxyData& wdata);

    //! Listener
    fastrtps::rtps::ReaderListener* listener_;
    //! Accept msg from unknown writers (BE-true,RE-false)
    bool accept_messages_from_unkown_writers_;
    //! Expects Inline Qos.
    bool expects_inline_qos_;

    IReaderDataFilter* data_filter_ = nullptr;

    //!ReaderHistoryState
    fastrtps::rtps::ReaderHistoryState* history_state_;

    uint64_t total_unread_ = 0;

    fastrtps::TimedConditionVariable new_notification_cv_;

    //! The liveliness kind of this reader
    fastdds::dds::LivelinessQosPolicyKind liveliness_kind_;
    //! The liveliness lease duration of this reader
    fastrtps::Duration_t liveliness_lease_duration_;

    //! Whether the writer is datasharing compatible or not
    bool is_datasharing_compatible_ = false;
    //! The listener for the datasharing notifications
    std::unique_ptr<fastrtps::rtps::IDataSharingListener> datasharing_listener_;

    //! The liveliness changed status struct as defined in the DDS
    fastdds::dds::LivelinessChangedStatus liveliness_changed_status_;

    //! Accept msg to unknwon readers
    bool accept_messages_to_unknown_readers_ = true;
    //! Trusted writer (for Builtin)
    fastrtps::rtps::EntityId_t trusted_writer_entity_id_;

private:

    void init(
            const std::shared_ptr<fastrtps::rtps::IPayloadPool>& payload_pool,
            const std::shared_ptr<fastrtps::rtps::IChangePool>& change_pool);

    void setup_datasharing(
            const fastrtps::rtps::ReaderAttributes& att);

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif /* RTPS_READER__BASEREADER_HPP */
