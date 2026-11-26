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

#ifndef FASTDDS_TEST_MOCK_RTPS_READER__STATEFULREADERMOCK_HPP
#define FASTDDS_TEST_MOCK_RTPS_READER__STATEFULREADERMOCK_HPP

#include <cstdint>

#include <gmock/gmock.h>

#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <mock/MockCompanion.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/resources/ResourceEvent.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class WriterProxy;

/**
 * Expectations of a StatefulReader created with the mock implementation, reached with mock::mock_of().
 * No writer is ever matched, no data is processed and no message is sent. getEventResource() returns, by default, an
 * event resource owned by the companion, whose thread is not started.
 */
class StatefulReaderMock
{
public:

    StatefulReaderMock()
    {
        ON_CALL(*this, getEventResource()).WillByDefault(testing::ReturnRef(service_));
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD1(matched_writer_add_edp, bool(const WriterProxyData& wdata));

    MOCK_METHOD2(matched_writer_remove, bool(const GUID_t& writer_guid, bool removed_by_lease));

    MOCK_METHOD1(matched_writer_is_matched, bool(const GUID_t& writer_guid));

    MOCK_METHOD1(assert_writer_liveliness, void(const GUID_t& writer_guid));

    MOCK_METHOD0(is_in_clean_state, bool());

    MOCK_METHOD1(change_removed_by_history, bool(CacheChange_t* change));

    MOCK_METHOD3(change_received, bool(CacheChange_t* a_change, WriterProxy* prox, size_t unknown_missing_changes));

    MOCK_METHOD0(getEventResource, ResourceEvent& ());

    //! Called when an ACKNACK with a sequence number set is sent to a writer.
    MOCK_METHOD1(simp_send_acknack, void(const SequenceNumberSet_t& sns));
    // *INDENT-ON*

private:

    ResourceEvent service_;
};

/**
 * StatefulReader with public constructors, for tests needing a stateful reader. As the code using a stateful reader
 * expects the caller to hold its mutex, the mutex is held while the instance exists. Each instance gets its own GUID,
 * with an increasing entity identifier.
 */
class TestStatefulReader : public StatefulReader
{
public:

    TestStatefulReader()
        : TestStatefulReader(
                nullptr)
    {
    }

    explicit TestStatefulReader(
            ReaderHistory* history)
        : StatefulReader(nullptr, next_guid(), ReaderAttributes(), history)
    {
        getMutex().lock();
    }

    ~TestStatefulReader()
    {
        getMutex().unlock();
    }

    using RTPSReader::history_;
    using BaseReader::listener_;

private:

    static GUID_t next_guid()
    {
        static uint32_t entity_id = 0;
        GUID_t guid;
        uint32_t id = ++entity_id;
        guid.entityId.value[0] = static_cast<octet>(id >> 16);
        guid.entityId.value[1] = static_cast<octet>(id >> 8);
        guid.entityId.value[2] = static_cast<octet>(id);
        guid.entityId.value[3] = 0x07;
        return guid;
    }

};

} // namespace rtps

namespace mock {

template<>
struct MockOf<rtps::StatefulReader>
{
    using type = testing::NiceMock<rtps::StatefulReaderMock>;
};

/**
 * Get the companion of a TestStatefulReader, which is the one of its StatefulReader base.
 * @param reader Instance created with the mock implementation.
 * @return Companion holding the expectations of the instance.
 */
inline MockOf<rtps::StatefulReader>::type& mock_of(
        const rtps::TestStatefulReader* reader)
{
    return MockCompanion<rtps::StatefulReader>::get(reader);
}

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_RTPS_READER__STATEFULREADERMOCK_HPP
