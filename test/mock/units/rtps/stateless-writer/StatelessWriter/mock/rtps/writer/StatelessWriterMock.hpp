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

#ifndef FASTDDS_TEST_MOCK_RTPS_WRITER__STATELESSWRITERMOCK_HPP
#define FASTDDS_TEST_MOCK_RTPS_WRITER__STATELESSWRITERMOCK_HPP

#include <chrono>
#include <cstdint>
#include <vector>

#include <gmock/gmock.h>

#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/transport/NetworkBuffer.hpp>
#include <mock/MockCompanion.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/writer/DeliveryRetCode.hpp>
#include <rtps/writer/LocatorSelectorSender.hpp>
#include <rtps/writer/StatelessWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSMessageGroup;
class RTPSParticipantImpl;

/**
 * Expectations of a StatelessWriter created with the mock implementation, reached with mock::mock_of().
 * No reader is ever matched and nothing is sent.
 */
class StatelessWriterMock
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
    // *INDENT-ON*
};

/**
 * StatelessWriter for tests needing a stateless writer, not registered in any flow controller. Each instance gets its
 * own GUID, with an increasing entity identifier.
 */
class TestStatelessWriter : public StatelessWriter
{
public:

    explicit TestStatelessWriter(
            RTPSParticipantImpl* participant = nullptr)
        : StatelessWriter(participant, next_guid(), WriterAttributes(), nullptr, nullptr)
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
        guid.entityId.value[3] = 0x03;
        return guid;
    }

};

} // namespace rtps

namespace mock {

template<>
struct MockOf<rtps::StatelessWriter>
{
    using type = testing::NiceMock<rtps::StatelessWriterMock>;
};

/**
 * Get the companion of a TestStatelessWriter, which is the one of its StatelessWriter base.
 * @param writer Instance created with the mock implementation.
 * @return Companion holding the expectations of the instance.
 */
inline MockOf<rtps::StatelessWriter>::type& mock_of(
        const rtps::TestStatelessWriter* writer)
{
    return MockCompanion<rtps::StatelessWriter>::get(writer);
}

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_RTPS_WRITER__STATELESSWRITERMOCK_HPP
