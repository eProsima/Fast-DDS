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

#ifndef FASTDDS_RTPS_READER__BASEREADER_HPP
#define FASTDDS_RTPS_READER__BASEREADER_HPP

#include <cstdint>
#include <memory>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/status/LivelinessChangedStatus.hpp>
#include <fastdds/rtps/Endpoint.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>
#include <fastdds/rtps/history/History.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/statistics/IListeners.hpp>
#include <fastdds/statistics/rtps/StatisticsCommon.hpp>
#include <fastdds/utils/TimedConditionVariable.hpp>

#include <rtps/reader/LocalReaderPointer.hpp>

namespace eprosima {

namespace fastdds {
namespace rtps {

struct CacheChange_t;
class IDataSharingListener;
struct ReaderHistoryState;
class ReaderListener;
class RTPSParticipantImpl;
class WriterProxy;
class WriterProxyData;

class BaseReader
    : public fastdds::rtps::RTPSReader
    , public fastdds::statistics::StatisticsReaderImpl
{

public:

    bool matched_writer_add(
            const PublicationBuiltinTopicData& info) final;

    /**
     * @brief Add a matched writer represented by its attributes.
     *
     * @param wdata  Discovery information regarding the writer to add.
     *
     * @return True if correctly added.
     */
    virtual bool matched_writer_add_edp(
            const WriterProxyData& wdata) = 0;

    fastdds::rtps::ReaderListener* get_listener() const override;

    void set_listener(
            fastdds::rtps::ReaderListener* listener) override;

    bool expects_inline_qos() const override;

    fastdds::rtps::ReaderHistory* get_history() const override;

    IReaderDataFilter* get_content_filter() const override;

    void set_content_filter(
            IReaderDataFilter* filter) override;

    uint64_t get_unread_count() const override;

    uint64_t get_unread_count(
            bool mark_as_read) override;

    bool wait_for_unread_cache(
            const fastdds::dds::Duration_t& timeout) override;

    bool is_sample_valid(
            const void* data,
            const fastdds::rtps::GUID_t& writer,
            const fastdds::rtps::SequenceNumber_t& sn) const override;

    /**
     * @brief Get a pointer to a BaseReader object from a RTPSReader pointer.
     *
     * @param reader  Pointer to the RTPSReader object.
     *
     * @return Pointer to the BaseReader object.
     */
    static BaseReader* downcast(
            fastdds::rtps::RTPSReader* reader);

    /**
     * @brief Get a pointer to a BaseReader object from a Endpoint pointer.
     *
     * @param endpoint  Pointer to the Endpoint object.
     *
     * @return Pointer to the BaseReader object.
     */
    static BaseReader* downcast(
            fastdds::rtps::Endpoint* endpoint);

    /**
     * @brief Set the entity ID of the trusted writer.
     *
     * @param writer  Entity ID of the trusted writer.
     */
    void set_trusted_writer(
            const fastdds::rtps::EntityId_t& writer)
    {
        accept_messages_from_unkown_writers_ = false;
        trusted_writer_entity_id_ = writer;
    }

    /**
     * @brief Allow reception ALIVE changes from non-matched writers.
     */
    void allow_unknown_writers();

    /**
     * @return The liveliness kind of this reader
     */
    fastdds::dds::LivelinessQosPolicyKind liveliness_kind() const
    {
        return liveliness_kind_;
    }

    /**
     * @return The liveliness lease duration of this reader
     */
    fastdds::dds::Duration_t liveliness_lease_duration() const
    {
        return liveliness_lease_duration_;
    }

    /**
     * @return the datasharing listener associated with this reader.
     */
    const std::unique_ptr<fastdds::rtps::IDataSharingListener>& datasharing_listener() const
    {
        return datasharing_listener_;
    }

    /**
     * @brief Retrieves the local pointer to this reader
     * to be used by other local entities.
     *
     * @return Local pointer to this reader.
     */
    std::shared_ptr<LocalReaderPointer> get_local_pointer();

    /**
     * @brief Reserve a CacheChange_t.
     *
     * @param [in]  cdr_payload_size  Size of the received payload.
     * @param [out] change            Pointer to the reserved change.
     *
     * @return True if correctly reserved.
     */
    bool reserve_cache(
            uint32_t cdr_payload_size,
            fastdds::rtps::CacheChange_t*& change);

    /**
     * @brief Release a CacheChange_t.
     *
     * @param change  Pointer to the change to release.
     */
    void release_cache(
            fastdds::rtps::CacheChange_t* change);

    /**
     * @brief Method to notify the reader that a change has been removed from its history.
     *
     * @param change  Pointer to the CacheChange_t that was removed from the history.
     *
     * @return True if correctly removed.
     */
    virtual bool change_removed_by_history(
            fastdds::rtps::CacheChange_t* change) = 0;

    /**
     * @brief Called just before a change is going to be deserialized.
     *
     * @param [in]  change            Pointer to the change being accessed.
     * @param [out] writer            Writer proxy the @c change belongs to.
     * @param [out] is_future_change  Whether the change is in the future (i.e. there are
     *                                earlier unreceived changes from the same writer).
     *
     * @return Whether the change is still valid or not.
     */
    virtual bool begin_sample_access_nts(
            fastdds::rtps::CacheChange_t* change,
            fastdds::rtps::WriterProxy*& writer,
            bool& is_future_change) = 0;

    /**
     * @brief Called after the change has been deserialized.
     *
     * @param [in] change        Pointer to the change being accessed.
     * @param [in] writer        Writer proxy the @c change belongs to.
     * @param [in] mark_as_read  Whether the @c change should be marked as read or not.
     */
    virtual void end_sample_access_nts(
            fastdds::rtps::CacheChange_t* change,
            fastdds::rtps::WriterProxy*& writer,
            bool mark_as_read) = 0;

    /**
     * @brief A method to update the liveliness changed status of the reader
     *
     * @param writer            The writer changing liveliness, specified by its guid
     * @param alive_change      The change requested for alive count. Should be -1, 0 or +1
     * @param not_alive_change  The change requested for not alive count. Should be -1, 0 or +1
     */
    void update_liveliness_changed_status(
            const fastdds::rtps::GUID_t& writer,
            int32_t alive_change,
            int32_t not_alive_change);

    /**
     * @brief Process an incoming DATA message.
     *
     * @param change  Pointer to the incoming CacheChange_t.
     *
     * @return true if the reader accepts message.
     */
    virtual bool process_data_msg(
            fastdds::rtps::CacheChange_t* change) = 0;

    /**
     * @brief Process an incoming DATA_FRAG message.
     *
     * @param change                 Pointer to the incoming CacheChange_t.
     * @param sampleSize             Size of the complete, assembled message.
     * @param fragmentStartingNum    Starting number of this particular message.
     * @param fragmentsInSubmessage  Number of fragments on this particular message.
     *
     * @return true if the reader accepts message.
     */
    virtual bool process_data_frag_msg(
            fastdds::rtps::CacheChange_t* change,
            uint32_t sampleSize,
            uint32_t fragmentStartingNum,
            uint16_t fragmentsInSubmessage) = 0;

    /**
     * @brief Process an incoming HEARTBEAT message.
     *
     * @param writerGUID
     * @param hbCount
     * @param firstSN
     * @param lastSN
     * @param finalFlag
     * @param livelinessFlag
     * @param origin_vendor_id
     *
     * @return true if the reader accepts message.
     */
    virtual bool process_heartbeat_msg(
            const fastdds::rtps::GUID_t& writerGUID,
            uint32_t hbCount,
            const fastdds::rtps::SequenceNumber_t& firstSN,
            const fastdds::rtps::SequenceNumber_t& lastSN,
            bool finalFlag,
            bool livelinessFlag,
            VendorId_t origin_vendor_id = c_VendorId_Unknown) = 0;

    /**
     * @brief Process an incoming GAP message.
     *
     * @param writerGUID
     * @param gapStart
     * @param gapList
     * @param origin_vendor_id
     *
     * @return true if the reader accepts message.
     */
    virtual bool process_gap_msg(
            const fastdds::rtps::GUID_t& writerGUID,
            const fastdds::rtps::SequenceNumber_t& gapStart,
            const fastdds::rtps::SequenceNumberSet_t& gapList,
            VendorId_t origin_vendor_id = c_VendorId_Unknown) = 0;

    /**
     * @brief Waits for not being referenced/used by any other entity.
     */
    virtual void local_actions_on_reader_removed();

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
            fastdds::rtps::RTPSParticipantImpl* pimpl,
            const fastdds::rtps::GUID_t& guid,
            const fastdds::rtps::ReaderAttributes& att,
            fastdds::rtps::ReaderHistory* hist,
            fastdds::rtps::ReaderListener* listen);

    BaseReader(
            fastdds::rtps::RTPSParticipantImpl* pimpl,
            const fastdds::rtps::GUID_t& guid,
            const fastdds::rtps::ReaderAttributes& att,
            const std::shared_ptr<fastdds::rtps::IPayloadPool>& payload_pool,
            fastdds::rtps::ReaderHistory* hist,
            fastdds::rtps::ReaderListener* listen);

    BaseReader(
            fastdds::rtps::RTPSParticipantImpl* pimpl,
            const fastdds::rtps::GUID_t& guid,
            const fastdds::rtps::ReaderAttributes& att,
            const std::shared_ptr<fastdds::rtps::IPayloadPool>& payload_pool,
            const std::shared_ptr<fastdds::rtps::IChangePool>& change_pool,
            fastdds::rtps::ReaderHistory* hist,
            fastdds::rtps::ReaderListener* listen);

    /**
     * @brief Whether a history record may be removed.
     *
     * @param removed_by_lease  Whether the history record is to be removed due to a participant drop.
     *
     * @return Whether the history record may be removed.
     */
    virtual bool may_remove_history_record(
            bool removed_by_lease);

    /**
     * @brief Add a remote writer to the persistence_guid map.
     *
     * @param guid              GUID of the remote writer.
     * @param persistence_guid  Persistence GUID of the remote writer.
     */
    void add_persistence_guid(
            const fastdds::rtps::GUID_t& guid,
            const fastdds::rtps::GUID_t& persistence_guid);

    /**
     * @brief Remove a remote writer from the persistence_guid map.
     *
     * @param guid              GUID of the remote writer.
     * @param persistence_guid  Persistence GUID of the remote writer.
     * @param removed_by_lease  Whether the GUIDs are being removed due to a participant drop.
     */
    void remove_persistence_guid(
            const fastdds::rtps::GUID_t& guid,
            const fastdds::rtps::GUID_t& persistence_guid,
            bool removed_by_lease);

    /**
     * @brief Get the last notified sequence for a writer's GUID.
     *
     * @param guid  The writer GUID to query.
     *
     * @return Last notified sequence number for input guid.
     * @remarks Takes persistence_guid into consideration.
     */
    fastdds::rtps::SequenceNumber_t get_last_notified(
            const fastdds::rtps::GUID_t& guid);

    /**
     * @brief Update the last notified sequence for a writer's GUID.
     *
     * @param guid  The GUID of the writer.
     * @param seq   Max sequence number available on writer.
     *
     * @return Previous value of last notified sequence number for input GUID.
     * @remarks Takes persistence_guid into consideration.
     */
    fastdds::rtps::SequenceNumber_t update_last_notified(
            const fastdds::rtps::GUID_t& guid,
            const fastdds::rtps::SequenceNumber_t& seq);

    /**
     * @brief Persist the last notified sequence for a persistence guid.
     * This method is called inside update_last_notified just after updating the last notified sequence for a writer
     * and gives persistent readers the opportunity to write the new sequence number to the database.
     *
     * @param persistence_guid  The persistence guid to update.
     * @param seq               Sequence number to set for input guid.
     */
    virtual void persist_last_notified_nts(
            const fastdds::rtps::GUID_t& persistence_guid,
            const fastdds::rtps::SequenceNumber_t& seq);

    /**
     * @brief Check if a writer can communicate with this reader using data-sharing.
     *
     * @param wdata  Discovery information of the writer to check.
     *
     * @return Whether the writer is datasharing compatible with this reader or not.
     */
    bool is_datasharing_compatible_with(
            const fastdds::rtps::WriterProxyData& wdata);

    /// Pool of serialized payloads.
    std::shared_ptr<IPayloadPool> payload_pool_;

    /// Pool of cache changes.
    std::shared_ptr<IChangePool> change_pool_;

    /// Pointer to the listener associated with this reader.
    fastdds::rtps::ReaderListener* listener_;
    /// Whether the reader accepts messages from unmatched writers.
    bool accept_messages_from_unkown_writers_;
    /// Whether the reader expects inline QoS.
    bool expects_inline_qos_;

    /// The data filter associated with this reader.
    IReaderDataFilter* data_filter_ = nullptr;

    /// The history record associated with this reader.
    fastdds::rtps::ReaderHistoryState* history_state_;

    /// Total number of unread samples in the history.
    uint64_t total_unread_ = 0;
    /// Condition variable to wait for unread samples.
    fastdds::TimedConditionVariable new_notification_cv_;

    /// The liveliness kind of this reader.
    fastdds::dds::LivelinessQosPolicyKind liveliness_kind_;
    /// The liveliness lease duration of this reader.
    fastdds::dds::Duration_t liveliness_lease_duration_;

    /// Whether the reader is datasharing compatible.
    bool is_datasharing_compatible_ = false;
    /// The listener for the datasharing notifications.
    std::unique_ptr<fastdds::rtps::IDataSharingListener> datasharing_listener_;

    /// The liveliness changed status struct as defined in the DDS standard.
    fastdds::dds::LivelinessChangedStatus liveliness_changed_status_;

    /// Trusted writer (for Builtin)
    fastdds::rtps::EntityId_t trusted_writer_entity_id_;

    /// RefCountedPointer of this instance.
    std::shared_ptr<LocalReaderPointer> local_ptr_;

private:

    /**
     * @brief Perform pools related setup.
     * This method is called from the constructor to perform the necessary setup for the payload and change pools.
     *
     * @param payload_pool  Payload pool to use.
     * @param change_pool   Change pool to use.
     */
    void init(
            const std::shared_ptr<fastdds::rtps::IPayloadPool>& payload_pool,
            const std::shared_ptr<fastdds::rtps::IChangePool>& change_pool);

    /**
     * @brief Perform datasharing related setup.
     * This method is called from the constructor to perform the necessary setup for datasharing.
     * If datasharing is enabled in the reader attributes, the method will create the necessary notification
     * segment, along with the corresponding listener.
     *
     * @param att  Attributes of the reader.
     */
    void setup_datasharing(
            const fastdds::rtps::ReaderAttributes& att);

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif // FASTDDS_RTPS_READER__BASEREADER_HPP
