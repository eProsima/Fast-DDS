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

#ifndef FASTDDS_TEST_MOCK_RTPS_WRITER__STATEFULWRITERMOCK_HPP
#define FASTDDS_TEST_MOCK_RTPS_WRITER__STATEFULWRITERMOCK_HPP

#include <chrono>
#include <cstdint>
#include <memory>

#include <gmock/gmock.h>

#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <mock/MockCompanion.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/writer/DeliveryRetCode.hpp>
#include <rtps/writer/LocatorSelectorSender.hpp>
#include <rtps/writer/StatefulWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class ReaderProxy;
class RTPSMessageGroup;
class RTPSParticipantImpl;

/**
 * Expectations of a StatefulWriter created with the mock implementation, reached with mock::mock_of().
 * No reader is ever matched, no message is sent and no event is scheduled.
 */
class StatefulWriterMock
{
public:

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD1(matched_reader_add_edp, bool(const ReaderProxyData& data));

    MOCK_METHOD1(matched_reader_remove, bool(const GUID_t& reader_guid));

    MOCK_METHOD1(matched_reader_is_matched, bool(const GUID_t& reader_guid));

    MOCK_METHOD1(unsent_change_added_to_history, void(CacheChange_t* change));

    MOCK_METHOD4(deliver_sample_nts, DeliveryRetCode(
            CacheChange_t*,
            RTPSMessageGroup&,
            LocatorSelectorSender&,
            const std::chrono::time_point<std::chrono::steady_clock>&));

    MOCK_METHOD2(intraprocess_delivery, bool(CacheChange_t* change, ReaderProxy* reader_proxy));

    MOCK_METHOD3(intraprocess_gap, bool(
            ReaderProxy* reader_proxy,
            const SequenceNumber_t& first_seq,
            const SequenceNumber_t& last_seq));

    MOCK_METHOD3(intraprocess_heartbeat, bool(ReaderProxy* reader_proxy, bool liveliness, bool gap_preceded));

    MOCK_METHOD2(send_periodic_heartbeat, bool(bool final, bool liveliness));

    MOCK_METHOD1(perform_nack_supression, void(const GUID_t& reader_guid));
    // *INDENT-ON*
};

namespace detail {

//! Owner of the history of a TestStatefulWriter created without one, constructed before the writer.
struct TestStatefulWriterHistory
{
    explicit TestStatefulWriterHistory(
            WriterHistory* history)
    {
        if (nullptr == history)
        {
            HistoryAttributes attributes;
            attributes.initialReservedCaches = 0;
            owned_history.reset(new WriterHistory(attributes));
        }
    }

    std::unique_ptr<WriterHistory> owned_history;
};

} // namespace detail

/**
 * StatefulWriter for tests needing a stateful writer, not registered in any flow controller. Without a given history,
 * it uses one of its own. Each instance gets its own GUID, with an increasing entity identifier.
 */
class TestStatefulWriter : private detail::TestStatefulWriterHistory, public StatefulWriter
{
public:

    explicit TestStatefulWriter(
            RTPSParticipantImpl* participant = nullptr,
            WriterHistory* history = nullptr)
        : detail::TestStatefulWriterHistory(history)
        , StatefulWriter(participant, next_guid(), WriterAttributes(), nullptr,
                nullptr != history ? history : owned_history.get(), nullptr, nullptr)
    {
    }

    using BaseWriter::history_;
    using BaseWriter::listener_;

private:

    static GUID_t next_guid()
    {
        static uint32_t entity_id = 0;
        GUID_t guid;
        uint32_t id = ++entity_id;
        guid.entityId.value[0] = static_cast<octet>(id >> 16);
        guid.entityId.value[1] = static_cast<octet>(id >> 8);
        guid.entityId.value[2] = static_cast<octet>(id);
        guid.entityId.value[3] = 0x02;
        return guid;
    }

};

} // namespace rtps

namespace mock {

template<>
struct MockOf<rtps::StatefulWriter>
{
    using type = testing::NiceMock<rtps::StatefulWriterMock>;
};

/**
 * Get the companion of a TestStatefulWriter, which is the one of its StatefulWriter base.
 * @param writer Instance created with the mock implementation.
 * @return Companion holding the expectations of the instance.
 */
inline MockOf<rtps::StatefulWriter>::type& mock_of(
        const rtps::TestStatefulWriter* writer)
{
    return MockCompanion<rtps::StatefulWriter>::get(writer);
}

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_RTPS_WRITER__STATEFULWRITERMOCK_HPP
