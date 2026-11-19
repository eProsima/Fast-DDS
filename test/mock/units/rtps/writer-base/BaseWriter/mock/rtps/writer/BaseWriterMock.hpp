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

#ifndef FASTDDS_TEST_MOCK_RTPS_WRITER__BASEWRITERMOCK_HPP
#define FASTDDS_TEST_MOCK_RTPS_WRITER__BASEWRITERMOCK_HPP

#include <chrono>
#include <cstdint>
#include <mutex>
#include <vector>

#include <gmock/gmock.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/attributes/ResourceManagement.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/transport/NetworkBuffer.hpp>
#include <fastdds/utils/TimedMutex.hpp>
#include <mock/MockCompanion.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/writer/BaseWriter.hpp>
#include <rtps/writer/DeliveryRetCode.hpp>
#include <rtps/writer/LocatorSelectorSender.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class IReaderDataFilter;
class RTPSMessageGroup;
class RTPSParticipantImpl;
class WriterHistory;

/**
 * Expectations on the non-virtual methods of a BaseWriter created with the mock implementation, reached with
 * mock::mock_of(). The virtual methods are mocked by TestBaseWriter. The writer reports no changes in its history.
 */
class BaseWriterMock
{
public:

    BaseWriterMock()
    {
        // A writer without changes in its history
        ON_CALL(*this, get_seq_num_min()).WillByDefault(testing::Return(c_SequenceNumber_Unknown));
        ON_CALL(*this, get_seq_num_max()).WillByDefault(testing::Return(c_SequenceNumber_Unknown));
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD0(get_max_allowed_payload_size, uint32_t());

    MOCK_METHOD0(get_liveliness_kind, const fastdds::dds::LivelinessQosPolicyKind& ());

    MOCK_METHOD0(get_liveliness_lease_duration, const dds::Duration_t& ());

    MOCK_METHOD0(get_liveliness_announcement_period, const dds::Duration_t& ());

    MOCK_METHOD0(is_datasharing_compatible, bool());

    MOCK_METHOD1(is_datasharing_compatible_with, bool(const dds::DataSharingQosPolicy& qos));

    MOCK_METHOD0(get_seq_num_min, SequenceNumber_t());

    MOCK_METHOD0(get_seq_num_max, SequenceNumber_t());

    MOCK_METHOD4(send_nts, bool(
            const std::vector<eprosima::fastdds::rtps::NetworkBuffer>&,
            const uint32_t&,
            const LocatorSelectorSender&,
            std::chrono::steady_clock::time_point&));
    // *INDENT-ON*
};

/**
 * BaseWriter implementing every pure virtual method, for tests needing a writer. RTPSWriter, BaseWriter and
 * StatisticsWriterImpl use their mock implementations, so the writer is not registered in any flow controller and
 * sends nothing. Each instance gets its own GUID, with an increasing entity identifier.
 */
class TestBaseWriter : public BaseWriter
{
public:

    TestBaseWriter()
        : TestBaseWriter(
                nullptr)
    {
    }

    explicit TestBaseWriter(
            RTPSParticipantImpl* participant,
            WriterHistory* history = nullptr)
        : BaseWriter(participant, next_guid(), WriterAttributes(), nullptr, history)
        , general_locator_selector_(*this, ResourceLimitedContainerConfig())
        , async_locator_selector_(*this, ResourceLimitedContainerConfig())
    {
    }

    /**
     * Change the history of the writer, as the participant mock does when the writer is created.
     * @param history History of the writer.
     */
    void set_history(
            WriterHistory* history)
    {
        history_ = history;
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD1(reader_data_filter, void(IReaderDataFilter* filter));

    MOCK_METHOD4(deliver_sample_nts, DeliveryRetCode(
            CacheChange_t*,
            RTPSMessageGroup&,
            LocatorSelectorSender&,
            const std::chrono::time_point<std::chrono::steady_clock>&));
    // *INDENT-ON*

    const IReaderDataFilter* reader_data_filter() const override
    {
        return nullptr;
    }

    bool matched_reader_remove(
            const GUID_t&) override
    {
        return false;
    }

    bool matched_reader_is_matched(
            const GUID_t&) override
    {
        return false;
    }

    bool has_been_fully_delivered(
            const SequenceNumber_t&) const override
    {
        return false;
    }

    bool is_acked_by_all(
            const SequenceNumber_t&) const override
    {
        return false;
    }

    bool wait_for_all_acked(
            const dds::Duration_t&) override
    {
        return true;
    }

    bool get_disable_positive_acks() const override
    {
        return false;
    }

    bool matched_readers_guids(
            std::vector<GUID_t>&) const override
    {
        return false;
    }

    bool get_connections(
            fastdds::statistics::rtps::ConnectionList&) override
    {
        return true;
    }

    bool matched_reader_add_edp(
            const ReaderProxyData&) override
    {
        return false;
    }

    void unsent_change_added_to_history(
            CacheChange_t*,
            const std::chrono::time_point<std::chrono::steady_clock>&) override
    {
    }

    bool change_removed_by_history(
            CacheChange_t*,
            const std::chrono::time_point<std::chrono::steady_clock>&) override
    {
        return true;
    }

    LocatorSelectorSender& get_general_locator_selector() override
    {
        return general_locator_selector_;
    }

    LocatorSelectorSender& get_async_locator_selector() override
    {
        return async_locator_selector_;
    }

    bool process_acknack(
            const GUID_t&,
            const GUID_t&,
            uint32_t,
            const SequenceNumberSet_t&,
            bool,
            bool& result,
            fastdds::rtps::VendorId_t) override
    {
        result = false;
        return true;
    }

    bool process_nack_frag(
            const GUID_t& writer_guid,
            const GUID_t&,
            uint32_t,
            const SequenceNumber_t&,
            const FragmentNumberSet_t&,
            bool& result,
            fastdds::rtps::VendorId_t) override
    {
        result = false;
        return writer_guid == getGuid();
    }

    bool try_remove_change(
            const std::chrono::steady_clock::time_point&,
            std::unique_lock<RecursiveTimedMutex>&) override
    {
        return true;
    }

    bool wait_for_acknowledgement(
            const SequenceNumber_t&,
            const std::chrono::steady_clock::time_point&,
            std::unique_lock<RecursiveTimedMutex>&) override
    {
        return true;
    }

    using BaseWriter::history_;
    using BaseWriter::listener_;
    using BaseWriter::liveliness_lost_status_;
    using Endpoint::mp_RTPSParticipant;

    LocatorSelectorSender general_locator_selector_;
    LocatorSelectorSender async_locator_selector_;

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
struct MockOf<rtps::BaseWriter>
{
    using type = testing::NiceMock<rtps::BaseWriterMock>;
};

/**
 * Get the companion of a TestBaseWriter, which is the one of its BaseWriter base.
 * @param writer Instance created with the mock implementation.
 * @return Companion holding the expectations of the instance.
 */
inline MockOf<rtps::BaseWriter>::type& mock_of(
        const rtps::TestBaseWriter* writer)
{
    return MockCompanion<rtps::BaseWriter>::get(writer);
}

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_RTPS_WRITER__BASEWRITERMOCK_HPP
