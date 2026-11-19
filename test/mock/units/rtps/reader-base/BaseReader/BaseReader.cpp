// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of BaseReader: replaces src/cpp/rtps/reader/BaseReader.cpp. The reader has no pools, no
// datasharing and no persistence, collects no statistics, and forwards the reservation and release of changes to the
// BaseReaderMock companion of the instance.

#include <rtps/reader/BaseReader.hpp>

#include <memory>

#include <mock/rtps/reader/BaseReaderMock.hpp>
#include <rtps/DataSharing/IDataSharingListener.hpp>
#include <statistics/rtps/StatisticsBase.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Companion = mock::MockCompanion<BaseReader>;

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
    , history_state_(nullptr)
    , liveliness_kind_(att.liveliness_kind)
    , liveliness_lease_duration_(att.liveliness_lease_duration)
{
    local_ptr_ = std::make_shared<LocalReaderPointer>(this);
    Companion::attach(this);
}

BaseReader::BaseReader(
        fastdds::rtps::RTPSParticipantImpl* pimpl,
        const fastdds::rtps::GUID_t& guid,
        const fastdds::rtps::ReaderAttributes& att,
        const std::shared_ptr<fastdds::rtps::IPayloadPool>& payload_pool,
        fastdds::rtps::ReaderHistory* hist,
        fastdds::rtps::ReaderListener* listen)
    : BaseReader(pimpl, guid, att, hist, listen)
{
    payload_pool_ = payload_pool;
}

BaseReader::BaseReader(
        fastdds::rtps::RTPSParticipantImpl* pimpl,
        const fastdds::rtps::GUID_t& guid,
        const fastdds::rtps::ReaderAttributes& att,
        const std::shared_ptr<fastdds::rtps::IPayloadPool>& payload_pool,
        const std::shared_ptr<fastdds::rtps::IChangePool>& change_pool,
        fastdds::rtps::ReaderHistory* hist,
        fastdds::rtps::ReaderListener* listen)
    : BaseReader(pimpl, guid, att, hist, listen)
{
    payload_pool_ = payload_pool;
    change_pool_ = change_pool;
}

BaseReader::~BaseReader()
{
    local_ptr_->deactivate();
    Companion::detach(this);
}

bool BaseReader::matched_writer_add(
        const PublicationBuiltinTopicData&)
{
    return false;
}

fastdds::rtps::ReaderListener* BaseReader::get_listener() const
{
    return listener_;
}

void BaseReader::set_listener(
        fastdds::rtps::ReaderListener* listener)
{
    listener_ = listener;
}

bool BaseReader::expects_inline_qos() const
{
    return expects_inline_qos_;
}

fastdds::rtps::ReaderHistory* BaseReader::get_history() const
{
    return history_;
}

IReaderDataFilter* BaseReader::get_content_filter() const
{
    return data_filter_;
}

void BaseReader::set_content_filter(
        IReaderDataFilter* filter)
{
    data_filter_ = filter;
}

uint64_t BaseReader::get_unread_count() const
{
    return total_unread_;
}

uint64_t BaseReader::get_unread_count(
        bool mark_as_read)
{
    uint64_t ret = total_unread_;
    if (mark_as_read)
    {
        total_unread_ = 0;
    }
    return ret;
}

bool BaseReader::wait_for_unread_cache(
        const fastdds::dds::Duration_t&)
{
    return 0 < total_unread_;
}

bool BaseReader::is_sample_valid(
        const void*,
        const fastdds::rtps::GUID_t&,
        const fastdds::rtps::SequenceNumber_t&) const
{
    return true;
}

BaseReader* BaseReader::downcast(
        fastdds::rtps::RTPSReader* reader)
{
    return static_cast<BaseReader*>(reader);
}

BaseReader* BaseReader::downcast(
        fastdds::rtps::Endpoint* endpoint)
{
    return static_cast<BaseReader*>(endpoint);
}

void BaseReader::allow_unknown_writers()
{
    accept_messages_from_unkown_writers_ = true;
}

std::shared_ptr<LocalReaderPointer> BaseReader::get_local_pointer()
{
    return local_ptr_;
}

bool BaseReader::reserve_cache(
        uint32_t cdr_payload_size,
        uint16_t fragment_size,
        fastdds::rtps::CacheChange_t*& change)
{
    return Companion::get(this).reserve_cache(cdr_payload_size, fragment_size, change);
}

void BaseReader::release_cache(
        fastdds::rtps::CacheChange_t* change)
{
    Companion::get(this).release_cache(change);
}

void BaseReader::update_liveliness_changed_status(
        const fastdds::rtps::GUID_t&,
        int32_t,
        int32_t)
{
}

void BaseReader::local_actions_on_reader_removed()
{
}

bool BaseReader::add_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener>)
{
    return true;
}

bool BaseReader::remove_statistics_listener(
        std::shared_ptr<fastdds::statistics::IListener>)
{
    return true;
}

void BaseReader::set_enabled_statistics_writers_mask(
        uint32_t)
{
}

bool BaseReader::may_remove_history_record(
        bool)
{
    return true;
}

void BaseReader::add_persistence_guid(
        const fastdds::rtps::GUID_t&,
        const fastdds::rtps::GUID_t&)
{
}

void BaseReader::remove_persistence_guid(
        const fastdds::rtps::GUID_t&,
        const fastdds::rtps::GUID_t&,
        bool)
{
}

fastdds::rtps::SequenceNumber_t BaseReader::get_last_notified(
        const fastdds::rtps::GUID_t&)
{
    return SequenceNumber_t();
}

fastdds::rtps::SequenceNumber_t BaseReader::update_last_notified(
        const fastdds::rtps::GUID_t&,
        const fastdds::rtps::SequenceNumber_t&)
{
    return SequenceNumber_t();
}

void BaseReader::persist_last_notified_nts(
        const fastdds::rtps::GUID_t&,
        const fastdds::rtps::SequenceNumber_t&)
{
}

bool BaseReader::is_datasharing_compatible_with(
        const fastdds::rtps::WriterProxyData&)
{
    return false;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
