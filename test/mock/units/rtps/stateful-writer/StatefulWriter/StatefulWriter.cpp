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

// Link-seam implementation of StatefulWriter: replaces src/cpp/rtps/writer/StatefulWriter.cpp. No reader is ever
// matched, no message is sent, no event is scheduled and every change counts as acknowledged, and the matching, the
// notification of new changes, the delivery of samples and the messages to readers are forwarded to the
// StatefulWriterMock companion of the instance.

#include <rtps/writer/StatefulWriter.hpp>

#include <fastdds/rtps/history/WriterHistory.hpp>
#include <mock/rtps/writer/StatefulWriterMock.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Companion = mock::MockCompanion<StatefulWriter>;

StatefulWriter::StatefulWriter(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const WriterAttributes& att,
        FlowController* flow_controller,
        WriterHistory* history,
        WriterListener* listener,
        StatefulWriterListener* stateful_listener)
    : BaseWriter(pimpl, guid, att, flow_controller, history, listener)
    , periodic_hb_event_(nullptr)
    , nack_response_event_(nullptr)
    , ack_event_(nullptr)
    , heartbeat_count_(0)
    , times_(att.times)
    , matched_remote_readers_(att.matched_readers_allocation)
    , matched_readers_pool_(att.matched_readers_allocation)
    , next_all_acked_notify_sequence_(0, 1)
    , all_acked_(false)
    , may_remove_change_cond_()
    , may_remove_change_(0)
    , disable_heartbeat_piggyback_(att.disable_heartbeat_piggyback)
    , disable_positive_acks_(att.disable_positive_acks)
    , keep_duration_(att.keep_duration)
    , last_sequence_number_()
    , biggest_removed_sequence_number_()
    , matched_local_readers_(att.matched_readers_allocation)
    , matched_datasharing_readers_(att.matched_readers_allocation)
    , locator_selector_general_(*this, att.matched_readers_allocation)
    , locator_selector_async_(*this, att.matched_readers_allocation)
    , stateful_writer_listener_(stateful_listener)
{
    Companion::attach(this);
}

StatefulWriter::~StatefulWriter()
{
    Companion::detach(this);
}

void StatefulWriter::local_actions_on_writer_removed()
{
}

bool StatefulWriter::matched_reader_add_edp(
        const ReaderProxyData& data)
{
    return Companion::get(this).matched_reader_add_edp(data);
}

bool StatefulWriter::matched_reader_remove(
        const GUID_t& reader_guid)
{
    return Companion::get(this).matched_reader_remove(reader_guid);
}

bool StatefulWriter::matched_reader_is_matched(
        const GUID_t& reader_guid)
{
    return Companion::get(this).matched_reader_is_matched(reader_guid);
}

void StatefulWriter::reader_data_filter(
        fastdds::rtps::IReaderDataFilter* filter)
{
    reader_data_filter_ = filter;
}

const fastdds::rtps::IReaderDataFilter* StatefulWriter::reader_data_filter() const
{
    return reader_data_filter_;
}

bool StatefulWriter::has_been_fully_delivered(
        const SequenceNumber_t&) const
{
    return false;
}

bool StatefulWriter::is_acked_by_all(
        const SequenceNumber_t&) const
{
    return false;
}

bool StatefulWriter::wait_for_all_acked(
        const dds::Duration_t&)
{
    return true;
}

void StatefulWriter::update_attributes(
        const WriterAttributes& att)
{
    BaseWriter::update_attributes(att);
}

bool StatefulWriter::matched_readers_guids(
        std::vector<GUID_t>&) const
{
    return false;
}

bool StatefulWriter::get_connections(
        fastdds::statistics::rtps::ConnectionList&)
{
    return true;
}

void StatefulWriter::unsent_change_added_to_history(
        CacheChange_t* change,
        const std::chrono::time_point<std::chrono::steady_clock>&)
{
    Companion::get(this).unsent_change_added_to_history(change);
}

bool StatefulWriter::change_removed_by_history(
        CacheChange_t*,
        const std::chrono::time_point<std::chrono::steady_clock>&)
{
    return true;
}

DeliveryRetCode StatefulWriter::deliver_sample_nts(
        CacheChange_t* cache_change,
        RTPSMessageGroup& group,
        LocatorSelectorSender& locator_selector,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    return Companion::get(this).deliver_sample_nts(cache_change, group, locator_selector, max_blocking_time);
}

bool StatefulWriter::process_acknack(
        const GUID_t&,
        const GUID_t&,
        uint32_t,
        const SequenceNumberSet_t&,
        bool,
        bool& result,
        fastdds::rtps::VendorId_t)
{
    result = false;
    return true;
}

bool StatefulWriter::process_nack_frag(
        const GUID_t& writer_guid,
        const GUID_t&,
        uint32_t,
        const SequenceNumber_t&,
        const FragmentNumberSet_t&,
        bool& result,
        fastdds::rtps::VendorId_t)
{
    result = false;
    return writer_guid == m_guid;
}

bool StatefulWriter::try_remove_change(
        const std::chrono::steady_clock::time_point&,
        std::unique_lock<RecursiveTimedMutex>&)
{
    return true;
}

bool StatefulWriter::wait_for_acknowledgement(
        const SequenceNumber_t&,
        const std::chrono::steady_clock::time_point&,
        std::unique_lock<RecursiveTimedMutex>&)
{
    return true;
}

bool StatefulWriter::intraprocess_delivery(
        CacheChange_t* change,
        ReaderProxy* reader_proxy)
{
    return Companion::get(this).intraprocess_delivery(change, reader_proxy);
}

bool StatefulWriter::intraprocess_gap(
        ReaderProxy* reader_proxy,
        const SequenceNumber_t& first_seq,
        const SequenceNumber_t& last_seq)
{
    return Companion::get(this).intraprocess_gap(reader_proxy, first_seq, last_seq);
}

bool StatefulWriter::intraprocess_heartbeat(
        ReaderProxy* reader_proxy,
        bool liveliness,
        bool gap_preceded)
{
    return Companion::get(this).intraprocess_heartbeat(reader_proxy, liveliness, gap_preceded);
}

bool StatefulWriter::all_readers_updated()
{
    return true;
}

bool StatefulWriter::matched_reader_lookup(
        GUID_t&,
        ReaderProxy**)
{
    return false;
}

void StatefulWriter::update_times(
        const WriterTimes& times)
{
    times_ = times;
}

void StatefulWriter::update_positive_acks_times(
        const WriterAttributes&)
{
}

SequenceNumber_t StatefulWriter::next_sequence_number() const
{
    return history_->next_sequence_number();
}

bool StatefulWriter::send_periodic_heartbeat(
        bool final,
        bool liveliness)
{
    return Companion::get(this).send_periodic_heartbeat(final, liveliness);
}

void StatefulWriter::send_heartbeat_to_nts(
        ReaderProxy&,
        bool,
        bool)
{
}

void StatefulWriter::perform_nack_response()
{
}

void StatefulWriter::perform_nack_supression(
        const GUID_t& reader_guid)
{
    Companion::get(this).perform_nack_supression(reader_guid);
}

void StatefulWriter::rebuild_status_after_load()
{
}

void StatefulWriter::print_inconsistent_acknack(
        const GUID_t&,
        const GUID_t&,
        const SequenceNumber_t&,
        const SequenceNumber_t&,
        const SequenceNumber_t&)
{
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
