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

// Link-seam implementation of StatelessWriter: replaces src/cpp/rtps/writer/StatelessWriter.cpp. No reader is ever
// matched, nothing is sent and every change counts as acknowledged, and the matching, the notification of new changes
// and the delivery of samples are forwarded to the StatelessWriterMock companion of the instance.

#include <rtps/writer/StatelessWriter.hpp>

#include <mock/rtps/writer/StatelessWriterMock.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Companion = mock::MockCompanion<StatelessWriter>;

StatelessWriter::StatelessWriter(
        RTPSParticipantImpl* impl,
        const GUID_t& guid,
        const WriterAttributes& attributes,
        FlowController* flow_controller,
        WriterHistory* history,
        WriterListener* listener)
    : BaseWriter(impl, guid, attributes, flow_controller, history, listener)
    , matched_remote_readers_(attributes.matched_readers_allocation)
    , matched_local_readers_(attributes.matched_readers_allocation)
    , matched_datasharing_readers_(attributes.matched_readers_allocation)
    , matched_readers_pool_(attributes.matched_readers_allocation)
    , locator_selector_(*this, attributes.matched_readers_allocation)
{
    Companion::attach(this);
}

StatelessWriter::~StatelessWriter()
{
    Companion::detach(this);
}

void StatelessWriter::local_actions_on_writer_removed()
{
}

bool StatelessWriter::matched_reader_add_edp(
        const ReaderProxyData& data)
{
    return Companion::get(this).matched_reader_add_edp(data);
}

bool StatelessWriter::matched_reader_remove(
        const GUID_t& reader_guid)
{
    return Companion::get(this).matched_reader_remove(reader_guid);
}

bool StatelessWriter::matched_reader_is_matched(
        const GUID_t& reader_guid)
{
    return Companion::get(this).matched_reader_is_matched(reader_guid);
}

bool StatelessWriter::has_been_fully_delivered(
        const SequenceNumber_t&) const
{
    return false;
}

bool StatelessWriter::is_acked_by_all(
        const SequenceNumber_t&) const
{
    return false;
}

bool StatelessWriter::wait_for_all_acked(
        const dds::Duration_t&)
{
    return true;
}

bool StatelessWriter::get_disable_positive_acks() const
{
    return false;
}

bool StatelessWriter::matched_readers_guids(
        std::vector<GUID_t>&) const
{
    return false;
}

bool StatelessWriter::get_connections(
        fastdds::statistics::rtps::ConnectionList&)
{
    return true;
}

void StatelessWriter::unsent_change_added_to_history(
        CacheChange_t* change,
        const std::chrono::time_point<std::chrono::steady_clock>&)
{
    Companion::get(this).unsent_change_added_to_history(change);
}

bool StatelessWriter::change_removed_by_history(
        CacheChange_t*,
        const std::chrono::time_point<std::chrono::steady_clock>&)
{
    return true;
}

DeliveryRetCode StatelessWriter::deliver_sample_nts(
        CacheChange_t* cache_change,
        RTPSMessageGroup& group,
        LocatorSelectorSender& locator_selector,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    return Companion::get(this).deliver_sample_nts(cache_change, group, locator_selector, max_blocking_time);
}

bool StatelessWriter::send_nts(
        const std::vector<NetworkBuffer>&,
        const uint32_t&,
        const LocatorSelectorSender&,
        std::chrono::steady_clock::time_point&) const
{
    return true;
}

bool StatelessWriter::process_acknack(
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

bool StatelessWriter::process_nack_frag(
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

bool StatelessWriter::try_remove_change(
        const std::chrono::steady_clock::time_point&,
        std::unique_lock<RecursiveTimedMutex>&)
{
    return true;
}

bool StatelessWriter::wait_for_acknowledgement(
        const SequenceNumber_t&,
        const std::chrono::steady_clock::time_point&,
        std::unique_lock<RecursiveTimedMutex>&)
{
    return true;
}

bool StatelessWriter::send_to_fixed_locators(
        const std::vector<eprosima::fastdds::rtps::NetworkBuffer>&,
        const uint32_t&,
        std::chrono::steady_clock::time_point&) const
{
    return true;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
