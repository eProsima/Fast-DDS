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

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/common/FragmentNumber.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>

#include <rtps/domain/RTPSDomainImpl.hpp>
#include <rtps/flowcontrol/FlowController.hpp>
#include <rtps/messages/MessageReceiver.h>
#include <rtps/messages/RTPSMessageCreator.hpp>
#include <rtps/reader/StatelessReader.hpp>
#include <rtps/writer/BaseWriter.hpp>
#include <rtps/writer/DeliveryRetCode.hpp>
#include <rtps/writer/LocatorSelectorSender.hpp>

#ifdef FASTDDS_STATISTICS

void register_monitorservice_types_type_objects()
{
}

void register_types_type_objects()
{
}

#endif  // FASTDDS_STATISTICS

namespace eprosima {
namespace fastdds {
namespace rtps {

//! Flow controller doing nothing, needed to build a writer outside of the participant.
class NullFlowController : public FlowController
{
public:

    void init() override
    {
    }

    void register_writer(
            BaseWriter*) override
    {
    }

    void unregister_writer(
            BaseWriter*) override
    {
    }

    bool add_new_sample(
            BaseWriter*,
            CacheChange_t*,
            const std::chrono::time_point<std::chrono::steady_clock>&) override
    {
        return true;
    }

    bool add_old_sample(
            BaseWriter*,
            CacheChange_t*) override
    {
        return true;
    }

    bool remove_change(
            CacheChange_t*,
            const std::chrono::time_point<std::chrono::steady_clock>&) override
    {
        return true;
    }

    uint32_t get_max_payload() override
    {
        return 65000u;
    }

};

//! Reader counting the submessages the MessageReceiver forwards to it.
class SpyReader : public StatelessReader
{
public:

    SpyReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            ReaderHistory* history)
        : StatelessReader(pimpl, guid, att, history, nullptr)
    {
    }

    bool process_data_msg(
            CacheChange_t*) override
    {
        ++data_count;
        return true;
    }

    bool process_data_frag_msg(
            CacheChange_t*,
            uint32_t,
            uint32_t,
            uint16_t) override
    {
        ++data_frag_count;
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
        ++heartbeat_count;
        return true;
    }

    bool process_gap_msg(
            const GUID_t&,
            const SequenceNumber_t&,
            const SequenceNumberSet_t&,
            VendorId_t) override
    {
        ++gap_count;
        return true;
    }

    unsigned int data_count = 0;
    unsigned int data_frag_count = 0;
    unsigned int heartbeat_count = 0;
    unsigned int gap_count = 0;
};

//! Writer counting the submessages the MessageReceiver forwards to it.
//! It derives from BaseWriter because the concrete writers mark the processing methods as final.
class SpyWriter : public BaseWriter
{
public:

    SpyWriter(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const WriterAttributes& att,
            FlowController* controller,
            WriterHistory* history)
        : BaseWriter(pimpl, guid, att, controller, history, nullptr)
        , locator_selector_(*this, att.matched_readers_allocation)
    {
    }

    bool process_acknack(
            const GUID_t& writer_guid,
            const GUID_t&,
            uint32_t,
            const SequenceNumberSet_t&,
            bool,
            bool& result,
            VendorId_t) override
    {
        ++acknack_count;
        result = true;
        return writer_guid == m_guid;
    }

    bool process_nack_frag(
            const GUID_t& writer_guid,
            const GUID_t&,
            uint32_t,
            const SequenceNumber_t&,
            const FragmentNumberSet_t&,
            bool& result,
            VendorId_t) override
    {
        ++nack_frag_count;
        result = true;
        return writer_guid == m_guid;
    }

    unsigned int acknack_count = 0;
    unsigned int nack_frag_count = 0;

    // Remaining pure virtual methods are not exercised by the MessageReceiver.

    bool matched_reader_remove(
            const GUID_t&) override
    {
        return true;
    }

    bool matched_reader_is_matched(
            const GUID_t&) override
    {
        return false;
    }

    void reader_data_filter(
            IReaderDataFilter*) override
    {
    }

    const IReaderDataFilter* reader_data_filter() const override
    {
        return nullptr;
    }

    bool has_been_fully_delivered(
            const SequenceNumber_t&) const override
    {
        return true;
    }

    bool is_acked_by_all(
            const SequenceNumber_t&) const override
    {
        return true;
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
        return true;
    }

#ifdef FASTDDS_STATISTICS
    bool get_connections(
            fastdds::statistics::rtps::ConnectionList&) override
    {
        return true;
    }

#endif // ifdef FASTDDS_STATISTICS

    bool matched_reader_add_edp(
            const ReaderProxyData&) override
    {
        return true;
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

    DeliveryRetCode deliver_sample_nts(
            CacheChange_t*,
            RTPSMessageGroup&,
            LocatorSelectorSender&,
            const std::chrono::time_point<std::chrono::steady_clock>&) override
    {
        return DeliveryRetCode::DELIVERED;
    }

    LocatorSelectorSender& get_general_locator_selector() override
    {
        return locator_selector_;
    }

    LocatorSelectorSender& get_async_locator_selector() override
    {
        return locator_selector_;
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

private:

    LocatorSelectorSender locator_selector_;
};

/**
 * Fixture feeding crafted RTPS messages to a real MessageReceiver with a local reader and
 * a local writer associated to it.
 */
class MessageReceiverTests : public ::testing::Test
{
protected:

    void SetUp() override
    {
        RTPSParticipantAttributes part_attrs;
        participant_ = RTPSDomain::createParticipant(0, false, part_attrs, nullptr);
        ASSERT_NE(nullptr, participant_);

        participant_impl_ = RTPSDomainImpl::get_instance()->find_participant(participant_->getGuid());
        ASSERT_NE(nullptr, participant_impl_);

        local_prefix_ = participant_->getGuid().guidPrefix;
        remote_prefix_ = local_prefix_;
        remote_prefix_.value[0] = static_cast<octet>(local_prefix_.value[0] + 1);

        ReaderAttributes reader_att;
        reader_att.endpoint.endpointKind = READER;
        reader_att.endpoint.reliabilityKind = BEST_EFFORT;
        reader_att.endpoint.durabilityKind = VOLATILE;
        reader_.reset(new SpyReader(participant_impl_, GUID_t(local_prefix_, local_reader_id_), reader_att,
                &reader_history_));

        WriterAttributes writer_att;
        writer_att.endpoint.endpointKind = WRITER;
        writer_att.endpoint.reliabilityKind = RELIABLE;
        writer_att.endpoint.durabilityKind = VOLATILE;
        writer_.reset(new SpyWriter(participant_impl_, GUID_t(local_prefix_, local_writer_id_), writer_att,
                &flow_controller_, &writer_history_));

        receiver_.reset(new MessageReceiver(participant_impl_, 65536u));
        receiver_->associateEndpoint(reader_.get());
        receiver_->associateEndpoint(writer_.get());
    }

    void TearDown() override
    {
        if (receiver_)
        {
            receiver_->removeEndpoint(reader_.get());
            receiver_->removeEndpoint(writer_.get());
            receiver_.reset();
        }
        writer_.reset();
        reader_.reset();

        if (nullptr != participant_)
        {
            RTPSDomain::removeRTPSParticipant(participant_);
        }
    }

    //! Feed a built message to the MessageReceiver under test.
    void process(
            CDRMessage_t& msg)
    {
        msg.length = msg.pos;
        Locator_t locator;
        receiver_->processCDRMsg(locator, locator, &msg);
    }

    //! Build a complete RTPS message holding a DATA submessage sent by @c writer_id.
    void build_data_msg(
            CDRMessage_t& msg,
            const EntityId_t& writer_id)
    {
        CacheChange_t change;
        change.kind = ALIVE;
        change.writerGUID = GUID_t(remote_prefix_, writer_id);
        change.sequenceNumber = {0, 1};
        change.serializedPayload.reserve(4u);
        change.serializedPayload.length = 4u;

        RTPSMessageCreator::addMessageData(&msg, remote_prefix_, &change, NO_KEY, local_reader_id_, false, nullptr);
    }

    //! Build a complete RTPS message holding a DATA_FRAG submessage sent by @c writer_id.
    void build_data_frag_msg(
            CDRMessage_t& msg,
            const EntityId_t& writer_id)
    {
        CacheChange_t change;
        change.kind = ALIVE;
        change.writerGUID = GUID_t(remote_prefix_, writer_id);
        change.sequenceNumber = {0, 1};
        change.serializedPayload.reserve(8u);
        change.serializedPayload.length = 8u;
        change.setFragmentSize(4u);

        RTPSMessageCreator::addMessageDataFrag(&msg, remote_prefix_, &change, 1u, NO_KEY, local_reader_id_, false,
                nullptr);
    }

    //! Build a complete RTPS message holding a HEARTBEAT submessage sent by @c writer_id.
    void build_heartbeat_msg(
            CDRMessage_t& msg,
            const EntityId_t& writer_id)
    {
        RTPSMessageCreator::addMessageHeartbeat(&msg, remote_prefix_, local_prefix_, local_reader_id_, writer_id,
                SequenceNumber_t(0, 1), SequenceNumber_t(0, 1), 1, true, false);
    }

    //! Build a complete RTPS message holding a GAP submessage sent by @c writer_id.
    void build_gap_msg(
            CDRMessage_t& msg,
            const EntityId_t& writer_id)
    {
        SequenceNumberSet_t gap_list(SequenceNumber_t(0, 2));
        RTPSMessageCreator::addMessageGap(&msg, remote_prefix_, local_prefix_, SequenceNumber_t(0, 1), gap_list,
                local_reader_id_, writer_id);
    }

    //! Build a complete RTPS message holding an ACKNACK submessage sent by @c reader_id.
    void build_acknack_msg(
            CDRMessage_t& msg,
            const EntityId_t& reader_id)
    {
        SequenceNumberSet_t sn_set(SequenceNumber_t(0, 1));
        RTPSMessageCreator::addMessageAcknack(&msg, remote_prefix_, local_prefix_, reader_id, local_writer_id_,
                sn_set, 1, true);
    }

    //! Build a complete RTPS message holding a NACK_FRAG submessage sent by @c reader_id.
    void build_nack_frag_msg(
            CDRMessage_t& msg,
            const EntityId_t& reader_id)
    {
        SequenceNumber_t writer_sn(0, 1);
        FragmentNumberSet_t fn_set(1u);
        RTPSMessageCreator::addMessageNackFrag(&msg, remote_prefix_, local_prefix_, reader_id, local_writer_id_,
                writer_sn, fn_set, 1);
    }

    const EntityId_t local_reader_id_ {0x00000107};
    const EntityId_t local_writer_id_ {0x00000102};
    const EntityId_t remote_writer_id_ {0x00000202};
    const EntityId_t remote_reader_id_ {0x00000207};

    RTPSParticipant* participant_ = nullptr;
    RTPSParticipantImpl* participant_impl_ = nullptr;
    GuidPrefix_t local_prefix_;
    GuidPrefix_t remote_prefix_;

    HistoryAttributes history_att_;
    ReaderHistory reader_history_ {history_att_};
    WriterHistory writer_history_ {history_att_};
    NullFlowController flow_controller_;

    std::unique_ptr<SpyReader> reader_;
    std::unique_ptr<SpyWriter> writer_;
    std::unique_ptr<MessageReceiver> receiver_;
};

/**
 * @test Regression for Redmine issue #25677.
 *
 * A DATA submessage whose writerId is ENTITYID_UNKNOWN must not be forwarded to any reader,
 * while the same submessage with a valid writerId must be.
 */
TEST_F(MessageReceiverTests, data_with_unknown_writer_id_is_ignored)
{
    {
        CDRMessage_t msg(1000);
        build_data_msg(msg, c_EntityId_Unknown);
        process(msg);
    }
    EXPECT_EQ(0u, reader_->data_count);

    {
        CDRMessage_t msg(1000);
        build_data_msg(msg, remote_writer_id_);
        process(msg);
    }
    EXPECT_EQ(1u, reader_->data_count);
}

/**
 * @test Regression for Redmine issue #25677.
 *
 * A DATA_FRAG submessage whose writerId is ENTITYID_UNKNOWN must not be forwarded to any reader,
 * while the same submessage with a valid writerId must be.
 */
TEST_F(MessageReceiverTests, data_frag_with_unknown_writer_id_is_ignored)
{
    {
        CDRMessage_t msg(1000);
        build_data_frag_msg(msg, c_EntityId_Unknown);
        process(msg);
    }
    EXPECT_EQ(0u, reader_->data_frag_count);

    {
        CDRMessage_t msg(1000);
        build_data_frag_msg(msg, remote_writer_id_);
        process(msg);
    }
    EXPECT_EQ(1u, reader_->data_frag_count);
}

/**
 * @test Regression for Redmine issue #25677.
 *
 * A HEARTBEAT submessage whose writerId is ENTITYID_UNKNOWN must not be forwarded to any reader,
 * while the same submessage with a valid writerId must be.
 */
TEST_F(MessageReceiverTests, heartbeat_with_unknown_writer_id_is_ignored)
{
    {
        CDRMessage_t msg(1000);
        build_heartbeat_msg(msg, c_EntityId_Unknown);
        process(msg);
    }
    EXPECT_EQ(0u, reader_->heartbeat_count);

    {
        CDRMessage_t msg(1000);
        build_heartbeat_msg(msg, remote_writer_id_);
        process(msg);
    }
    EXPECT_EQ(1u, reader_->heartbeat_count);
}

/**
 * @test Regression for Redmine issue #25677.
 *
 * A GAP submessage whose writerId is ENTITYID_UNKNOWN must not be forwarded to any reader,
 * while the same submessage with a valid writerId must be.
 */
TEST_F(MessageReceiverTests, gap_with_unknown_writer_id_is_ignored)
{
    {
        CDRMessage_t msg(1000);
        build_gap_msg(msg, c_EntityId_Unknown);
        process(msg);
    }
    EXPECT_EQ(0u, reader_->gap_count);

    {
        CDRMessage_t msg(1000);
        build_gap_msg(msg, remote_writer_id_);
        process(msg);
    }
    EXPECT_EQ(1u, reader_->gap_count);
}

/**
 * @test Regression for Redmine issue #25677.
 *
 * An ACKNACK submessage whose readerId is ENTITYID_UNKNOWN must not be forwarded to any writer,
 * while the same submessage with a valid readerId must be.
 */
TEST_F(MessageReceiverTests, acknack_with_unknown_reader_id_is_ignored)
{
    {
        CDRMessage_t msg(1000);
        build_acknack_msg(msg, c_EntityId_Unknown);
        process(msg);
    }
    EXPECT_EQ(0u, writer_->acknack_count);

    {
        CDRMessage_t msg(1000);
        build_acknack_msg(msg, remote_reader_id_);
        process(msg);
    }
    EXPECT_EQ(1u, writer_->acknack_count);
}

/**
 * @test Regression for Redmine issue #25677.
 *
 * A NACK_FRAG submessage whose readerId is ENTITYID_UNKNOWN must not be forwarded to any writer,
 * while the same submessage with a valid readerId must be.
 */
TEST_F(MessageReceiverTests, nack_frag_with_unknown_reader_id_is_ignored)
{
    {
        CDRMessage_t msg(1000);
        build_nack_frag_msg(msg, c_EntityId_Unknown);
        process(msg);
    }
    EXPECT_EQ(0u, writer_->nack_frag_count);

    {
        CDRMessage_t msg(1000);
        build_nack_frag_msg(msg, remote_reader_id_);
        process(msg);
    }
    EXPECT_EQ(1u, writer_->nack_frag_count);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
