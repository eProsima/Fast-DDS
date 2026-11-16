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

// Link-seam implementation of StatelessReader: replaces src/cpp/rtps/reader/StatelessReader.cpp. No data is processed
// and no sample is ever available, and the matching and removal of changes are forwarded to the StatelessReaderMock
// companion of the instance.

#include <rtps/reader/StatelessReader.hpp>

#include <mock/rtps/reader/StatelessReaderMock.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Companion = mock::MockCompanion<StatelessReader>;

StatelessReader::StatelessReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        ReaderHistory* hist,
        ReaderListener* listen)
    : BaseReader(pimpl, guid, att, hist, listen)
    , matched_writers_(att.matched_writers_allocation)
{
    Companion::attach(this);
}

StatelessReader::StatelessReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        ReaderHistory* hist,
        ReaderListener* listen)
    : BaseReader(pimpl, guid, att, payload_pool, hist, listen)
    , matched_writers_(att.matched_writers_allocation)
{
    Companion::attach(this);
}

StatelessReader::StatelessReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        const std::shared_ptr<IChangePool>& change_pool,
        ReaderHistory* hist,
        ReaderListener* listen)
    : BaseReader(pimpl, guid, att, payload_pool, change_pool, hist, listen)
    , matched_writers_(att.matched_writers_allocation)
{
    Companion::attach(this);
}

StatelessReader::~StatelessReader()
{
    Companion::detach(this);
}

bool StatelessReader::matched_writer_add_edp(
        const WriterProxyData& wdata)
{
    return Companion::get(this).matched_writer_add_edp(wdata);
}

bool StatelessReader::matched_writer_remove(
        const GUID_t& writer_guid,
        bool removed_by_lease)
{
    return Companion::get(this).matched_writer_remove(writer_guid, removed_by_lease);
}

bool StatelessReader::matched_writer_is_matched(
        const GUID_t& writer_guid)
{
    return Companion::get(this).matched_writer_is_matched(writer_guid);
}

bool StatelessReader::change_removed_by_history(
        CacheChange_t* change)
{
    return Companion::get(this).change_removed_by_history(change);
}

bool StatelessReader::process_data_msg(
        CacheChange_t*)
{
    return true;
}

bool StatelessReader::process_data_frag_msg(
        CacheChange_t*,
        uint32_t,
        uint32_t,
        uint16_t)
{
    return true;
}

bool StatelessReader::process_heartbeat_msg(
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

bool StatelessReader::process_gap_msg(
        const GUID_t&,
        const SequenceNumber_t&,
        const SequenceNumberSet_t&,
        fastdds::rtps::VendorId_t)
{
    return true;
}

bool StatelessReader::change_received(
        CacheChange_t*)
{
    return true;
}

CacheChange_t* StatelessReader::next_unread_cache()
{
    return nullptr;
}

CacheChange_t* StatelessReader::next_untaken_cache()
{
    return nullptr;
}

void StatelessReader::assert_writer_liveliness(
        const GUID_t& guid)
{
    Companion::get(this).assert_writer_liveliness(guid);
}

bool StatelessReader::begin_sample_access_nts(
        CacheChange_t*,
        WriterProxy*& writer,
        bool& is_future_change)
{
    writer = nullptr;
    is_future_change = false;
    return true;
}

void StatelessReader::end_sample_access_nts(
        CacheChange_t*,
        WriterProxy*&,
        bool,
        bool)
{
}

bool StatelessReader::matched_writers_guids(
        std::vector<GUID_t>&) const
{
    return false;
}

bool StatelessReader::get_connections(
        fastdds::statistics::rtps::ConnectionList&)
{
    return true;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
