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

#ifndef FASTDDS_TEST_MOCK_RTPS_READER__BASEREADERMOCK_HPP
#define FASTDDS_TEST_MOCK_RTPS_READER__BASEREADERMOCK_HPP

#include <cstdint>
#include <vector>

#include <gmock/gmock.h>

#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <mock/MockCompanion.hpp>
#include <rtps/reader/BaseReader.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipantImpl;
class WriterProxy;
class WriterProxyData;

/**
 * Expectations on the non-virtual methods of a BaseReader created with the mock implementation, reached with
 * mock::mock_of(). The virtual methods are mocked by TestBaseReader.
 */
class BaseReaderMock
{
public:

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD3(reserve_cache, bool(uint32_t cdr_payload_size, uint16_t fragment_size, CacheChange_t*& change));

    MOCK_METHOD1(release_cache, void(CacheChange_t* change));
    // *INDENT-ON*
};

/**
 * BaseReader implementing every pure virtual method, for tests needing a reader. RTPSReader, BaseReader and
 * StatisticsReaderImpl use their mock implementations, so the reader receives nothing and collects no statistics. Each
 * instance gets its own GUID, with an increasing entity identifier.
 */
class TestBaseReader : public BaseReader
{
public:

    TestBaseReader()
        : TestBaseReader(
                nullptr)
    {
    }

    explicit TestBaseReader(
            ReaderHistory* history,
            RTPSParticipantImpl* participant = nullptr)
        : BaseReader(participant, next_guid(), ReaderAttributes(), history, nullptr)
    {
    }

    /**
     * Change the history of the reader, as the participant mock does when the reader is created.
     * @param history History of the reader.
     */
    void set_history(
            ReaderHistory* history)
    {
        history_ = history;
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD(bool, expects_inline_qos, (), (const, override));

    MOCK_METHOD(IReaderDataFilter*, get_content_filter, (), (const, override));

    MOCK_METHOD(void, set_content_filter, (IReaderDataFilter* filter), (override));

    MOCK_METHOD(bool, wait_for_unread_cache, (const fastdds::dds::Duration_t& timeout), (override));

    MOCK_METHOD(uint64_t, get_unread_count, (), (const, override));

    MOCK_METHOD(uint64_t, get_unread_count, (bool mark_as_read), (override));

    MOCK_METHOD(bool, change_removed_by_history, (CacheChange_t* change), (override));
    // *INDENT-ON*

    bool matched_writer_add_edp(
            const WriterProxyData&) override
    {
        return false;
    }

    bool matched_writer_remove(
            const GUID_t&,
            bool) override
    {
        return false;
    }

    bool matched_writer_is_matched(
            const GUID_t&) override
    {
        return false;
    }

    void assert_writer_liveliness(
            const GUID_t&) override
    {
    }

    bool is_in_clean_state() override
    {
        return true;
    }

    bool matched_writers_guids(
            std::vector<GUID_t>&) const override
    {
        return false;
    }

    CacheChange_t* next_unread_cache() override
    {
        return nullptr;
    }

    CacheChange_t* next_untaken_cache() override
    {
        return nullptr;
    }

    bool get_connections(
            fastdds::statistics::rtps::ConnectionList&) override
    {
        return true;
    }

    bool begin_sample_access_nts(
            CacheChange_t*,
            WriterProxy*&,
            bool&) override
    {
        return true;
    }

    void end_sample_access_nts(
            CacheChange_t*,
            WriterProxy*&,
            bool,
            bool) override
    {
    }

    bool process_data_msg(
            CacheChange_t*) override
    {
        return true;
    }

    bool process_data_frag_msg(
            CacheChange_t*,
            uint32_t,
            uint32_t,
            uint16_t) override
    {
        return true;
    }

    bool process_heartbeat_msg(
            const GUID_t&,
            uint32_t,
            const SequenceNumber_t&,
            const SequenceNumber_t&,
            bool,
            bool,
            VendorId_t) override
    {
        return true;
    }

    bool process_gap_msg(
            const GUID_t&,
            const SequenceNumber_t&,
            const SequenceNumberSet_t&,
            VendorId_t) override
    {
        return true;
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
struct MockOf<rtps::BaseReader>
{
    using type = rtps::BaseReaderMock;
};

/**
 * Get the companion of a TestBaseReader, which is the one of its BaseReader base.
 * @param reader Instance created with the mock implementation.
 * @return Companion holding the expectations of the instance.
 */
inline MockOf<rtps::BaseReader>::type& mock_of(
        const rtps::TestBaseReader* reader)
{
    return MockCompanion<rtps::BaseReader>::get(reader);
}

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_RTPS_READER__BASEREADERMOCK_HPP
