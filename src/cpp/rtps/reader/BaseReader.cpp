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

/*
 * BaseReader.cpp
 */

#include <rtps/reader/BaseReader.hpp>

#include <cassert>
#include <cstdint>
#include <mutex>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/Endpoint.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>

#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/DataSharing/DataSharingListener.hpp>
#include <rtps/DataSharing/DataSharingNotification.hpp>
#include <rtps/DataSharing/DataSharingPayloadPool.hpp>
#include <rtps/history/BasicPayloadPool.hpp>
#include <rtps/history/CacheChangePool.h>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/ReaderHistoryState.hpp>
#include <statistics/rtps/StatisticsBase.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

BaseReader::BaseReader(
        fastdds::rtps::RTPSParticipantImpl* pimpl,
        const fastdds::rtps::GUID_t& guid,
        const fastdds::rtps::ReaderAttributes& att,
        fastdds::rtps::ReaderHistory* hist,
        fastdds::rtps::ReaderListener* listen)
    : fastdds::rtps::RTPSReader(pimpl, guid, att, hist)
    , listener_(listen)
    , accept_messages_from_unkown_writers_(att.accept_messages_from_unkown_writers)
    , expects_inline_qos_(att.expects_inline_qos)
    , history_state_(new fastdds::rtps::ReaderHistoryState(att.matched_writers_allocation.initial))
    , liveliness_kind_(att.liveliness_kind)
    , liveliness_lease_duration_(att.liveliness_lease_duration)
{
    PoolConfig cfg = PoolConfig::from_history_attributes(hist->m_att);
    std::shared_ptr<IChangePool> change_pool;
    std::shared_ptr<IPayloadPool> payload_pool;
    payload_pool = BasicPayloadPool::get(cfg, change_pool);

    init(payload_pool, change_pool);
    setup_datasharing(att);
}

BaseReader::BaseReader(
        fastdds::rtps::RTPSParticipantImpl* pimpl,
        const fastdds::rtps::GUID_t& guid,
        const fastdds::rtps::ReaderAttributes& att,
        const std::shared_ptr<fastdds::rtps::IPayloadPool>& payload_pool,
        fastdds::rtps::ReaderHistory* hist,
        fastdds::rtps::ReaderListener* listen)
    : BaseReader(
        pimpl, guid, att, payload_pool,
        std::make_shared<CacheChangePool>(PoolConfig::from_history_attributes(hist->m_att)),
        hist, listen)
{
}

BaseReader::BaseReader(
        fastdds::rtps::RTPSParticipantImpl* pimpl,
        const fastdds::rtps::GUID_t& guid,
        const fastdds::rtps::ReaderAttributes& att,
        const std::shared_ptr<fastdds::rtps::IPayloadPool>& payload_pool,
        const std::shared_ptr<fastdds::rtps::IChangePool>& change_pool,
        fastdds::rtps::ReaderHistory* hist,
        fastdds::rtps::ReaderListener* listen)
    : fastdds::rtps::RTPSReader(pimpl, guid, att, hist)
    , listener_(listen)
    , accept_messages_from_unkown_writers_(att.accept_messages_from_unkown_writers)
    , expects_inline_qos_(att.expects_inline_qos)
    , history_state_(new fastdds::rtps::ReaderHistoryState(att.matched_writers_allocation.initial))
    , liveliness_kind_(att.liveliness_kind)
    , liveliness_lease_duration_(att.liveliness_lease_duration)
{
    init(payload_pool, change_pool);
    setup_datasharing(att);
}

void BaseReader::local_actions_on_reader_removed()
{
    local_ptr_->deactivate();
}

BaseReader::~BaseReader()
{
    EPROSIMA_LOG_INFO(RTPS_READER, "Removing reader " << this->getGuid().entityId);

    for (auto it = history_->changesBegin(); it != history_->changesEnd(); ++it)
    {
        release_cache(*it);
    }

    delete history_state_;

    // As releasing the change pool will delete the cache changes it owns,
    // the payload pool may be called to release their payloads, so we should
    // ensure that the payload pool is destroyed after the change pool.
    change_pool_.reset();
    payload_pool_.reset();
}

bool BaseReader::matched_writer_add(
        const PublicationBuiltinTopicData& info)
{
    const auto& alloc = mp_RTPSParticipant->get_attributes().allocation;
    WriterProxyData wdata(alloc.data_limits, info);

    return matched_writer_add_edp(wdata);
}

ReaderListener* BaseReader::get_listener() const
{
    std::lock_guard<decltype(mp_mutex)> lock(mp_mutex);
    return listener_;
}

void BaseReader::set_listener(
        ReaderListener* target)
{
    std::lock_guard<decltype(mp_mutex)> lock(mp_mutex);
    listener_ = target;
}

bool BaseReader::expects_inline_qos() const
{
    return expects_inline_qos_;
}

ReaderHistory* BaseReader::get_history() const
{
    return history_;
}

//! @return The content filter associated to this reader.
IReaderDataFilter* BaseReader::get_content_filter() const
{
    std::lock_guard<decltype(mp_mutex)> lock(mp_mutex);
    return data_filter_;
}

//! Set the content filter associated to this reader.
//! @param filter Pointer to the content filter to associate to this reader.
void BaseReader::set_content_filter(
        IReaderDataFilter* filter)
{
    std::lock_guard<decltype(mp_mutex)> lock(mp_mutex);
    data_filter_ = filter;
}

uint64_t BaseReader::get_unread_count() const
{
    std::lock_guard<decltype(mp_mutex)> lock(mp_mutex);
    return total_unread_;
}

uint64_t BaseReader::get_unread_count(
        bool mark_as_read)
{
    std::lock_guard<decltype(mp_mutex)> lock(mp_mutex);
    uint64_t ret_val = total_unread_;

    if (mark_as_read)
    {
        for (auto it = history_->changesBegin(); 0 < total_unread_ && it != history_->changesEnd(); ++it)
        {
            fastdds::rtps::CacheChange_t* change = *it;
            if (!change->isRead && get_last_notified(change->writerGUID) >= change->sequenceNumber)
            {
                change->isRead = true;
                assert(0 < total_unread_);
                --total_unread_;
            }
        }
        assert(0 == total_unread_);
    }
    return ret_val;
}

bool BaseReader::wait_for_unread_cache(
        const eprosima::fastdds::dds::Duration_t& timeout)
{
    auto time_out = std::chrono::steady_clock::now() + std::chrono::seconds(timeout.seconds) +
            std::chrono::nanoseconds(timeout.nanosec);

#if HAVE_STRICT_REALTIME
    std::unique_lock<decltype(mp_mutex)> lock(mp_mutex, std::defer_lock);
    if (lock.try_lock_until(time_out))
#else
    std::unique_lock<decltype(mp_mutex)> lock(mp_mutex);
#endif  // HAVE_STRICT_REALTIME
    {
        if (new_notification_cv_.wait_until(
                    lock, time_out,
                    [&]()
                    {
                        return total_unread_ > 0;
                    }))
        {
            return true;
        }
    }

    return false;
}

bool BaseReader::is_sample_valid(
        const void* data,
        const fastdds::rtps::GUID_t& writer,
        const fastdds::rtps::SequenceNumber_t& sn) const
{
    if (is_datasharing_compatible_ && datasharing_listener_->writer_is_matched(writer))
    {
        // Check if the payload is dirty
        // Note the Payloads used in loans include a mandatory RTPS 2.3 extra header
        auto payload = static_cast<const fastdds::rtps::octet*>(data);
        payload -= fastdds::rtps::SerializedPayload_t::representation_header_size;
        if (!fastdds::rtps::DataSharingPayloadPool::check_sequence_number(payload, sn))
        {
            return false;
        }
    }
    return true;
}

BaseReader* BaseReader::downcast(
        fastdds::rtps::RTPSReader* reader)
{
    assert(nullptr != dynamic_cast<BaseReader*>(reader));
    return static_cast<BaseReader*>(reader);
}

BaseReader* BaseReader::downcast(
        fastdds::rtps::Endpoint* endpoint)
{
    assert(nullptr != dynamic_cast<BaseReader*>(endpoint));
    return static_cast<BaseReader*>(endpoint);
}

void BaseReader::allow_unknown_writers()
{
    assert(fastdds::rtps::EntityId_t::unknown() != trusted_writer_entity_id_);
    accept_messages_from_unkown_writers_ = true;
}

std::shared_ptr<LocalReaderPointer> BaseReader::get_local_pointer()
{
    return local_ptr_;
}

bool BaseReader::reserve_cache(
        uint32_t cdr_payload_size,
        fastdds::rtps::CacheChange_t*& change)
{
    std::lock_guard<decltype(mp_mutex)> guard(mp_mutex);

    change = nullptr;

    fastdds::rtps::CacheChange_t* reserved_change = nullptr;
    if (!change_pool_->reserve_cache(reserved_change))
    {
        EPROSIMA_LOG_WARNING(RTPS_READER, "Problem reserving cache from pool");
        return false;
    }

    uint32_t payload_size = fixed_payload_size_ ? fixed_payload_size_ : cdr_payload_size;
    if (!payload_pool_->get_payload(payload_size, reserved_change->serializedPayload))
    {
        change_pool_->release_cache(reserved_change);
        EPROSIMA_LOG_WARNING(RTPS_READER, "Problem reserving payload from pool");
        return false;
    }

    change = reserved_change;
    return true;
}

void BaseReader::release_cache(
        fastdds::rtps::CacheChange_t* change)
{
    std::lock_guard<decltype(mp_mutex)> guard(mp_mutex);

    fastdds::rtps::IPayloadPool* pool = change->serializedPayload.payload_owner;
    if (pool)
    {
        pool->release_payload(change->serializedPayload);
    }
    change_pool_->release_cache(change);
}

void BaseReader::update_liveliness_changed_status(
        const fastdds::rtps::GUID_t& writer,
        int32_t alive_change,
        int32_t not_alive_change)
{
    std::lock_guard<decltype(mp_mutex)> lock(mp_mutex);

    liveliness_changed_status_.alive_count += alive_change;
    liveliness_changed_status_.alive_count_change += alive_change;
    liveliness_changed_status_.not_alive_count += not_alive_change;
    liveliness_changed_status_.not_alive_count_change += not_alive_change;
    liveliness_changed_status_.last_publication_handle = writer;

    if (nullptr != listener_)
    {
        listener_->on_liveliness_changed(this, liveliness_changed_status_);

        liveliness_changed_status_.alive_count_change = 0;
        liveliness_changed_status_.not_alive_count_change = 0;
    }
}

#ifdef FASTDDS_STATISTICS

bool BaseReader::add_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener> listener)
{
    return add_statistics_listener_impl(listener);
}

bool BaseReader::remove_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener> listener)
{
    return remove_statistics_listener_impl(listener);
}

void BaseReader::set_enabled_statistics_writers_mask(
        uint32_t enabled_writers)
{
    set_enabled_statistics_writers_mask_impl(enabled_writers);
}

#endif // FASTDDS_STATISTICS

bool BaseReader::may_remove_history_record(
        bool removed_by_lease)
{
    return !removed_by_lease;
}

void BaseReader::add_persistence_guid(
        const fastdds::rtps::GUID_t& guid,
        const fastdds::rtps::GUID_t& persistence_guid)
{
    if (fastdds::rtps::c_Guid_Unknown == persistence_guid || persistence_guid == guid)
    {
        std::lock_guard<decltype(mp_mutex)> guard(mp_mutex);
        history_state_->persistence_guid_map[guid] = guid;
        history_state_->persistence_guid_count[guid]++;
    }
    else
    {
        std::lock_guard<decltype(mp_mutex)> guard(mp_mutex);
        history_state_->persistence_guid_map[guid] = persistence_guid;
        history_state_->persistence_guid_count[persistence_guid]++;

        // Could happen that a value has already been stored in the record with the guid and not the persistence guid
        // This is because received_change is called before Proxy is created
        // In this case, we substitute the guid for the persistence (in case they are not equal)
        auto spourious_record = history_state_->history_record.find(guid);
        if (spourious_record != history_state_->history_record.end())
        {
            EPROSIMA_LOG_INFO(RTPS_READER, "Sporious record found, changing guid "
                    << guid << " for persistence guid " << persistence_guid);
            update_last_notified(guid, spourious_record->second);
            history_state_->history_record.erase(spourious_record);
        }
    }
}

void BaseReader::remove_persistence_guid(
        const fastdds::rtps::GUID_t& guid,
        const fastdds::rtps::GUID_t& persistence_guid,
        bool removed_by_lease)
{
    std::lock_guard<decltype(mp_mutex)> guard(mp_mutex);
    auto persistence_guid_stored = (fastdds::rtps::c_Guid_Unknown == persistence_guid) ? guid : persistence_guid;
    history_state_->persistence_guid_map.erase(guid);
    auto count = --history_state_->persistence_guid_count[persistence_guid_stored];
    if (count <= 0 && may_remove_history_record(removed_by_lease))
    {
        history_state_->history_record.erase(persistence_guid_stored);
        history_state_->persistence_guid_count.erase(persistence_guid_stored);
    }
}

fastdds::rtps::SequenceNumber_t BaseReader::get_last_notified(
        const fastdds::rtps::GUID_t& guid)
{
    fastdds::rtps::SequenceNumber_t ret_val;
    std::lock_guard<decltype(mp_mutex)> guard(mp_mutex);
    fastdds::rtps::GUID_t guid_to_look = guid;
    auto p_guid = history_state_->persistence_guid_map.find(guid);
    if (p_guid != history_state_->persistence_guid_map.end())
    {
        guid_to_look = p_guid->second;
    }

    auto p_seq = history_state_->history_record.find(guid_to_look);
    if (p_seq != history_state_->history_record.end())
    {
        ret_val = p_seq->second;
    }

    return ret_val;
}

fastdds::rtps::SequenceNumber_t BaseReader::update_last_notified(
        const fastdds::rtps::GUID_t& guid,
        const fastdds::rtps::SequenceNumber_t& seq)
{
    fastdds::rtps::SequenceNumber_t ret_val;
    std::lock_guard<decltype(mp_mutex)> guard(mp_mutex);
    fastdds::rtps::GUID_t guid_to_look = guid;
    auto p_guid = history_state_->persistence_guid_map.find(guid);
    if (p_guid != history_state_->persistence_guid_map.end())
    {
        guid_to_look = p_guid->second;
    }

    auto p_seq = history_state_->history_record.find(guid_to_look);
    if (p_seq != history_state_->history_record.end())
    {
        ret_val = p_seq->second;
    }

    if (ret_val < seq)
    {
        history_state_->history_record[guid_to_look] = seq;
        persist_last_notified_nts(guid_to_look, seq);
        new_notification_cv_.notify_all();
    }

    return ret_val;
}

void BaseReader::persist_last_notified_nts(
        const fastdds::rtps::GUID_t& peristence_guid,
        const fastdds::rtps::SequenceNumber_t& seq)
{
    // Empty base implementation since base behavior is to not persist data
    static_cast<void>(peristence_guid);
    static_cast<void>(seq);
}

bool BaseReader::is_datasharing_compatible_with(
        const fastdds::rtps::WriterProxyData& wdata)
{
    if (!is_datasharing_compatible_ ||
            wdata.data_sharing.kind() == fastdds::dds::DataSharingKind::OFF)
    {
        return false;
    }

    for (auto id : wdata.data_sharing.domain_ids())
    {
        if (std::find(m_att.data_sharing_configuration().domain_ids().begin(),
                m_att.data_sharing_configuration().domain_ids().end(), id)
                != m_att.data_sharing_configuration().domain_ids().end())
        {
            return true;
        }
    }

    return false;
}

void BaseReader::init(
        const std::shared_ptr<IPayloadPool>& payload_pool,
        const std::shared_ptr<IChangePool>& change_pool)
{
    payload_pool_ = payload_pool;
    change_pool_ = change_pool;
    fixed_payload_size_ = 0;
    if (history_->m_att.memoryPolicy == PREALLOCATED_MEMORY_MODE)
    {
        fixed_payload_size_ = history_->m_att.payloadMaxSize;
    }

    local_ptr_ = std::make_shared<LocalReaderPointer>(this);

    EPROSIMA_LOG_INFO(RTPS_READER, "RTPSReader created correctly");
}

void BaseReader::setup_datasharing(
        const fastdds::rtps::ReaderAttributes& att)
{

    if (att.endpoint.data_sharing_configuration().kind() != fastdds::dds::DataSharingKind::OFF)
    {
        using std::placeholders::_1;
        std::shared_ptr<DataSharingNotification> notification = DataSharingNotification::create_notification(
            getGuid(), att.endpoint.data_sharing_configuration().shm_directory());
        if (notification)
        {
            is_datasharing_compatible_ = true;
            datasharing_listener_.reset(new DataSharingListener(
                        notification,
                        att.endpoint.data_sharing_configuration().shm_directory(),
                        att.data_sharing_listener_thread,
                        att.matched_writers_allocation,
                        this));

            // We can start the listener here, as no writer can be matched already,
            // so no notification will occur until the non-virtual instance is constructed.
            // But we need to stop the listener in the non-virtual instance destructor.
            datasharing_listener_->start();
        }
    }
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
