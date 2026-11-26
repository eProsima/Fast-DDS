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

// Link-seam implementation of StatefulReader: replaces src/cpp/rtps/reader/StatefulReader.cpp. No writer is ever
// matched, no data is processed and no sample is ever available. The matching, the removal of changes, the event
// resource and the ACKNACKs sent are forwarded to the StatefulReaderMock companion of the instance.

#include <rtps/reader/StatefulReader.hpp>

#include <mock/rtps/reader/StatefulReaderMock.hpp>
#include <rtps/history/HistoryAttributesExtension.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Companion = mock::MockCompanion<StatefulReader>;

StatefulReader::StatefulReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        ReaderHistory* hist,
        ReaderListener* listen)
    : BaseReader(pimpl, guid, att, hist, listen)
    , acknack_count_(0)
    , nackfrag_count_(0)
    , times_(att.times)
    , matched_writers_(att.matched_writers_allocation)
    , matched_writers_pool_(att.matched_writers_allocation)
    , proxy_changes_config_(nullptr != hist ? resource_limits_from_history(hist->m_att, 0) :
            ResourceLimitedContainerConfig())
    , disable_positive_acks_(att.disable_positive_acks)
    , is_alive_(true)
{
    Companion::attach(this);
}

StatefulReader::StatefulReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        ReaderHistory* hist,
        ReaderListener* listen)
    : BaseReader(pimpl, guid, att, payload_pool, hist, listen)
    , acknack_count_(0)
    , nackfrag_count_(0)
    , times_(att.times)
    , matched_writers_(att.matched_writers_allocation)
    , matched_writers_pool_(att.matched_writers_allocation)
    , proxy_changes_config_(nullptr != hist ? resource_limits_from_history(hist->m_att, 0) :
            ResourceLimitedContainerConfig())
    , disable_positive_acks_(att.disable_positive_acks)
    , is_alive_(true)
{
    Companion::attach(this);
}

StatefulReader::StatefulReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        const std::shared_ptr<IChangePool>& change_pool,
        ReaderHistory* hist,
        ReaderListener* listen)
    : BaseReader(pimpl, guid, att, payload_pool, change_pool, hist, listen)
    , acknack_count_(0)
    , nackfrag_count_(0)
    , times_(att.times)
    , matched_writers_(att.matched_writers_allocation)
    , matched_writers_pool_(att.matched_writers_allocation)
    , proxy_changes_config_(nullptr != hist ? resource_limits_from_history(hist->m_att, 0) :
            ResourceLimitedContainerConfig())
    , disable_positive_acks_(att.disable_positive_acks)
    , is_alive_(true)
{
    Companion::attach(this);
}

StatefulReader::~StatefulReader()
{
    Companion::detach(this);
}

bool StatefulReader::matched_writer_add_edp(
        const WriterProxyData& wdata)
{
    return Companion::get(this).matched_writer_add_edp(wdata);
}

bool StatefulReader::matched_writer_remove(
        const GUID_t& writer_guid,
        bool removed_by_lease)
{
    return Companion::get(this).matched_writer_remove(writer_guid, removed_by_lease);
}

bool StatefulReader::matched_writer_is_matched(
        const GUID_t& writer_guid)
{
    return Companion::get(this).matched_writer_is_matched(writer_guid);
}

bool StatefulReader::matched_writer_lookup(
        const GUID_t&,
        WriterProxy** WP)
{
    *WP = nullptr;
    return false;
}

bool StatefulReader::process_data_msg(
        CacheChange_t*)
{
    return true;
}

bool StatefulReader::process_data_frag_msg(
        CacheChange_t*,
        uint32_t,
        uint32_t,
        uint16_t)
{
    return true;
}

bool StatefulReader::process_heartbeat_msg(
        const GUID_t&,
        uint32_t,
        const SequenceNumber_t&,
        const SequenceNumber_t&,
        bool,
        bool,
        fastdds::rtps::VendorId_t)
{
    return true;
}

bool StatefulReader::process_gap_msg(
        const GUID_t&,
        const SequenceNumber_t&,
        const SequenceNumberSet_t&,
        fastdds::rtps::VendorId_t)
{
    return true;
}

bool StatefulReader::change_removed_by_history(
        CacheChange_t* change)
{
    return Companion::get(this).change_removed_by_history(change);
}

bool StatefulReader::change_received(
        CacheChange_t* a_change,
        WriterProxy* prox,
        size_t unknown_missing_changes_up_to,
        bool& will_never_be_accepted)
{
    will_never_be_accepted = false;
    return Companion::get(this).change_received(a_change, prox, unknown_missing_changes_up_to);
}

ResourceEvent& StatefulReader::getEventResource() const
{
    return Companion::get(this).getEventResource();
}

CacheChange_t* StatefulReader::next_unread_cache()
{
    return nullptr;
}

CacheChange_t* StatefulReader::next_untaken_cache()
{
    return nullptr;
}

bool StatefulReader::updateTimes(
        const ReaderTimes& times)
{
    times_ = times;
    return true;
}

bool StatefulReader::is_in_clean_state()
{
    return Companion::get(this).is_in_clean_state();
}

void StatefulReader::send_acknack(
        const WriterProxy*,
        const SequenceNumberSet_t& sns,
        RTPSMessageSenderInterface*,
        bool)
{
    Companion::get(this).simp_send_acknack(sns);
}

void StatefulReader::send_acknack(
        const WriterProxy*,
        RTPSMessageSenderInterface*,
        bool)
{
}

bool StatefulReader::send_sync_nts(
        const std::vector<eprosima::fastdds::rtps::NetworkBuffer>&,
        const uint32_t&,
        const Locators&,
        const Locators&,
        std::chrono::steady_clock::time_point&)
{
    return true;
}

void StatefulReader::assert_writer_liveliness(
        const GUID_t& writer)
{
    Companion::get(this).assert_writer_liveliness(writer);
}

bool StatefulReader::begin_sample_access_nts(
        CacheChange_t*,
        WriterProxy*& writer,
        bool& is_future_change)
{
    writer = nullptr;
    is_future_change = false;
    return true;
}

void StatefulReader::end_sample_access_nts(
        CacheChange_t*,
        WriterProxy*&,
        bool,
        bool)
{
}

bool StatefulReader::matched_writers_guids(
        std::vector<GUID_t>&) const
{
    return false;
}

bool StatefulReader::get_connections(
        fastdds::statistics::rtps::ConnectionList&)
{
    return true;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
