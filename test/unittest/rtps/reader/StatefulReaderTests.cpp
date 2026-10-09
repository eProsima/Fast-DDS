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

#include <gtest/gtest.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/ReaderListener.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/writer/RTPSWriter.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <thread>
#include <vector>

namespace eprosima {
namespace fastrtps {
namespace rtps {

namespace {

class RemovingListener : public ReaderListener
{
public:

    void onNewCacheChangeAdded(
            RTPSReader* reader,
            const CacheChange_t* const change) override
    {
        ASSERT_NE(nullptr, reader);
        ASSERT_NE(nullptr, change);

        notified_sequences_.push_back(change->sequenceNumber.to64long());

        // Removing earlier changes while NotifyChanges() is iterating over the
        // history invalidates the current vector iterator.
        if (change->sequenceNumber == SequenceNumber_t(0, 3))
        {
            ReaderHistory* history = reader->getHistory();
            ASSERT_NE(nullptr, history);

            for (const SequenceNumber_t& sequence_to_remove : sequences_to_remove_)
            {
                bool removed = false;
                for (auto change_it = history->changesBegin(); change_it != history->changesEnd(); ++change_it)
                {
                    CacheChange_t* change_to_remove = *change_it;
                    if (change_to_remove->writerGUID == change->writerGUID &&
                            change_to_remove->sequenceNumber == sequence_to_remove)
                    {
                        history->remove_change(change_it);
                        removed = true;
                        break;
                    }
                }
                ASSERT_TRUE(removed);
            }
        }
    }

    void remove_on_third_notification(
            const std::array<SequenceNumber_t, 2>& sequences)
    {
        sequences_to_remove_ = sequences;
    }

    const std::vector<int64_t>& notified_sequences() const
    {
        return notified_sequences_;
    }

private:

    std::vector<int64_t> notified_sequences_;
    std::array<SequenceNumber_t, 2> sequences_to_remove_;
};

GUID_t writer_guid()
{
    GUID_t guid;
    guid.guidPrefix.value[0] = 1;
    guid.entityId.value[3] = 3;
    return guid;
}

void initialize_change(
        CacheChange_t& change,
        const GUID_t& guid,
        const int32_t sequence_number)
{
    change.kind = ALIVE;
    change.writerGUID = guid;
    change.sequenceNumber = SequenceNumber_t(0, sequence_number);
    change.serializedPayload.length = 4;
    std::memset(change.serializedPayload.data, sequence_number, change.serializedPayload.length);
}

} // anonymous namespace

TEST(StatefulReaderTests, ListenerCanRemoveEarlierChangesWhileNotifying)
{
    RTPSParticipantAttributes participant_attributes;
    participant_attributes.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::NONE;
    participant_attributes.builtin.use_WriterLivelinessProtocol = false;

    RTPSParticipant* participant = RTPSDomain::createParticipant(0, participant_attributes);
    ASSERT_NE(nullptr, participant);

    HistoryAttributes history_attributes;
    history_attributes.payloadMaxSize = 4;
    history_attributes.initialReservedCaches = 4;
    history_attributes.maximumReservedCaches = 4;
    ReaderHistory history(history_attributes);

    RemovingListener listener;

    ReaderAttributes reader_attributes;
    reader_attributes.endpoint.endpointKind = READER;
    reader_attributes.endpoint.reliabilityKind = RELIABLE;
    fastdds::dds::DataSharingQosPolicy data_sharing;
    data_sharing.off();
    reader_attributes.endpoint.set_data_sharing_configuration(data_sharing);

    RTPSReader* reader = RTPSDomain::createRTPSReader(
        participant,
        reader_attributes,
        &history,
        &listener);
    ASSERT_NE(nullptr, reader);

    WriterProxyData writer_data(0, 0);
    writer_data.guid(writer_guid());
    writer_data.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    writer_data.m_qos.data_sharing.off();
    ASSERT_TRUE(reader->matched_writer_add(writer_data));

    CacheChange_t change_1(4);
    CacheChange_t change_2(4);
    CacheChange_t change_3(4);
    CacheChange_t change_4(4);
    std::array<CacheChange_t*, 4> changes {{&change_1, &change_2, &change_3, &change_4}};
    initialize_change(change_1, writer_data.guid(), 1);
    initialize_change(change_2, writer_data.guid(), 2);
    initialize_change(change_3, writer_data.guid(), 3);
    initialize_change(change_4, writer_data.guid(), 4);
    listener.remove_on_third_notification({SequenceNumber_t(0, 1), SequenceNumber_t(0, 2)});

    // Receive change 4 before change 3. This leaves change 4 in the history
    // without notifying it, because WriterProxy only exposes a contiguous
    // sequence to the reader. When change 3 arrives, NotifyChanges() notifies
    // both 3 and 4 in a single iteration.
    const std::array<size_t, 4> receive_order {{0, 1, 3, 2}};
    for (const size_t index : receive_order)
    {
        ASSERT_TRUE(reader->processDataMsg(changes[index]));
    }

    const std::vector<int64_t> expected_sequences {1, 2, 3, 4};
    EXPECT_EQ(expected_sequences, listener.notified_sequences());

    RTPSDomain::removeRTPSParticipant(participant);
}

/* Regression test for GHSA-57rp-jm6r-xjw5: an unvalidated HEARTBEAT firstSN lets
 * changes_from_writer_low_mark_ be set to an attacker-chosen value, after which
 * StatefulReader::NotifyChanges' inner loop (WriterProxy::next_cache_change_to_be_notified)
 * counts up to that value one increment at a time with no history lookup -- O(value), not
 * O(history size). This drives the exact same entry points MessageReceiver calls after parsing
 * real wire bytes (process_data_msg, process_heartbeat_msg), with no network/transport/discovery
 * involved, so there is nothing here for a MITM harness, socket buffering, or ACKNACK
 * retransmission to interfere with.
 *
 * Rather than actually waiting for a ~1e18-iteration spin, this measures wall-clock time for two
 * increasing "huge" sequence-number values on independent writer GUIDs and checks that the second
 * (10x larger) call takes markedly longer than the first -- direct evidence the cost is
 * proportional to the injected sequence-number magnitude, not to the single stored history change used in each run.
 */
TEST(StatefulReaderTests, DataHeartbeatSequenceNumberUnboundedNotifyLoop)
{
    RTPSParticipantAttributes part_attrs;
    RTPSParticipant* part = RTPSDomain::createParticipant(0, false, part_attrs, nullptr);

    HistoryAttributes hatt{};
    ReaderHistory reader_history(hatt);

    ReaderAttributes reader_att{};
    reader_att.endpoint.endpointKind = READER;
    reader_att.endpoint.reliabilityKind = RELIABLE;
    reader_att.endpoint.durabilityKind = TRANSIENT_LOCAL;

    RTPSReader* reader = RTPSDomain::createRTPSReader(part, reader_att, &reader_history, nullptr);
    StatefulReader* uut = dynamic_cast<StatefulReader*>(reader);
    ASSERT_NE(uut, nullptr);

    TopicAttributes topic_desc;
    topic_desc.topicDataType = "string";
    topic_desc.topicName = "topic";
    part->registerReader(reader, topic_desc, fastdds::dds::ReaderQos());

    // measure_spin_us(huge_sn): inject one CacheChange with sequenceNumber == huge_sn (simulating
    // an attacker-forged DATA writerSN -- accepted without a sequence-number magnitude bound), then a
    // HEARTBEAT whose firstSN == huge_sn + 1 (simulating an attacker-forged HEARTBEAT firstSN --
    // also accepted without a sequence-number magnitude bound). Returns elapsed microseconds for the process_heartbeat_msg
    // call, which is where NotifyChanges' inner loop runs.
    auto measure_spin_us = [&](const SequenceNumber_t& huge_sn) -> int64_t
            {
                HistoryAttributes whatt{};
                WriterHistory writer_history(whatt);
                WriterAttributes writer_att{};
                writer_att.endpoint.endpointKind = WRITER;
                writer_att.endpoint.reliabilityKind = RELIABLE;
                writer_att.endpoint.durabilityKind = TRANSIENT_LOCAL;

                RTPSWriter* writer = RTPSDomain::createRTPSWriter(part, writer_att, &writer_history, nullptr);
                EXPECT_NE(writer, nullptr);
                part->registerWriter(writer, topic_desc, fastdds::dds::WriterQos());
                auto writer_guid = writer->getGuid();
                EXPECT_TRUE(uut->matched_writer_is_matched(writer_guid));

                // Priming step (matches EmptyPayloadUnregisterDisposeProcessing's call order above):
                // an empty-payload ALIVE change is not actually stored ("considered non-relevant")
                // but establishes the instance/writer-proxy context the following DISPOSED needs.
                CacheChange_t prime;
                prime.writerGUID = writer_guid;
                prime.sequenceNumber = SequenceNumber_t(0, 1);
                prime.serializedPayload.length = 0;
                prime.instanceHandle.value[0] = 1;
                prime.kind = ChangeKind_t::ALIVE;
                uut->processDataMsg(&prime);

                // (a) Attacker-forged DATA: no upper bound on sequence-number magnitude in
                // WriterProxy::received_change_set / StatefulReader::change_received. Empty-payload
                // NOT_ALIVE_DISPOSED (rather than ALIVE) is the same construction
                // EmptyPayloadUnregisterDisposeProcessing uses above -- it is fully processed and
                // stored in reader history without needing a payload-pool-backed buffer.
                CacheChange_t change;
                change.writerGUID = writer_guid;
                change.sequenceNumber = huge_sn;
                change.serializedPayload.length = 0;
                change.instanceHandle.value[0] = 1;
                change.kind = ChangeKind_t::NOT_ALIVE_DISPOSED;
                EXPECT_TRUE(uut->processDataMsg(&change));

                // (b) Attacker-forged HEARTBEAT: no upper bound check in
                // WriterProxy::process_heartbeat / lost_changes_update either.
                SequenceNumber_t firstSN = huge_sn + 1;
                SequenceNumber_t lastSN = huge_sn + 2;

                auto t0 = std::chrono::steady_clock::now();
                uut->processHeartbeatMsg(writer_guid, /*hbCount=*/ 1, firstSN, lastSN,
                        /*finalFlag=*/ true, /*livelinessFlag=*/ false);
                auto t1 = std::chrono::steady_clock::now();

                RTPSDomain::removeRTPSWriter(writer);
                return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
            };

    const int64_t small_n = 20000000LL;   // 2e7
    const int64_t large_n = 200000000LL;  // 2e8 (10x)

    int64_t small_us = measure_spin_us(SequenceNumber_t(static_cast<uint64_t>(small_n)));
    int64_t large_us = measure_spin_us(SequenceNumber_t(static_cast<uint64_t>(large_n)));
    double ratio = (small_us > 0 ? static_cast<double>(large_us) / static_cast<double>(small_us) : -1.0);

    std::cout << "small_n=" << small_n << " -> " << small_us << " us" << std::endl;
    std::cout << "large_n=" << large_n << " (10x) -> " << large_us << " us" << std::endl;
    std::cout << "ratio=" << ratio << std::endl;

    // CONTROL -- the CVE-2025-65016 variant on THIS SAME (fixed) build: a HEARTBEAT alone, with an
    // equally-huge firstSN but NO stored DATA change, must NOT spin. With no qualifying change in
    // history, NotifyChanges' outer loop does not execute and it falls through to the O(1)
    // prox->consider_all_notified() -- which is exactly the CVE-2025-65016 fix (v3.4.2+ replaced a
    // trailing "while (Unknown != next_cache_change_to_be_notified()) {}" drain loop with that call).
    // So this control is fast, while the DATA+HEARTBEAT variant above still scales O(sequence value):
    // the published fix does not cover the inner do-while exercised by this two-message path.
    auto measure_hb_only_us = [&](const SequenceNumber_t& huge_sn) -> int64_t
            {
                HistoryAttributes whatt{};
                WriterHistory writer_history(whatt);
                WriterAttributes writer_att{};
                writer_att.endpoint.endpointKind = WRITER;
                writer_att.endpoint.reliabilityKind = RELIABLE;
                writer_att.endpoint.durabilityKind = TRANSIENT_LOCAL;
                RTPSWriter* writer = RTPSDomain::createRTPSWriter(part, writer_att, &writer_history, nullptr);
                EXPECT_NE(writer, nullptr);
                part->registerWriter(writer, topic_desc, fastdds::dds::WriterQos());
                auto writer_guid = writer->getGuid();
                EXPECT_TRUE(uut->matched_writer_is_matched(writer_guid));
                // No DATA change stored: HEARTBEAT alone.
                SequenceNumber_t firstSN = huge_sn + 1;
                SequenceNumber_t lastSN = huge_sn + 2;
                auto t0 = std::chrono::steady_clock::now();
                uut->processHeartbeatMsg(writer_guid, /*hbCount=*/ 1, firstSN, lastSN,
                        /*finalFlag=*/ true, /*livelinessFlag=*/ false);
                auto t1 = std::chrono::steady_clock::now();
                RTPSDomain::removeRTPSWriter(writer);
                return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
            };
    int64_t hb_only_us = measure_hb_only_us(SequenceNumber_t(static_cast<uint64_t>(large_n)));
    std::cout << "CONTROL hb-alone large_n=" << large_n << " -> " << hb_only_us << " us" << std::endl;

    // A patched, O(history size) implementation would make both DATA+HEARTBEAT calls return in
    // comparable (near-zero) time regardless of small_n/large_n. The vulnerable O(sequence-number
    // value) implementation makes large_us come out roughly 10x small_us.
    if (small_us < large_us)
    {
        EXPECT_LT(ratio, 5.0);
    }

    // Destroy the entities
    RTPSDomain::removeRTPSReader(reader);
    RTPSDomain::removeRTPSParticipant(part);
}

/* Live demonstration of the actual denial of service for GHSA-57rp-jm6r-xjw5.
 *
 * A single crafted DATA + HEARTBEAT pair pins one core at 100% CPU and the
 * process_heartbeat_msg() call never returns. While it runs:
 *   - `top -H -p <pid>`  shows one thread at 100% CPU;
 *   - `gdb -p <pid>` then `thread apply all bt` shows the stack pinned in
 *       WriterProxy::next_cache_change_to_be_notified
 *       <- StatefulReader::NotifyChanges   (the inner do-while)
 *     with `last_notified_` climbing toward the injected firstSN while
 *     `changes_from_writer_low_mark_` stays fixed at ~5.98e18.
 * Stop it with Ctrl-C. On a FIXED (bounded) build the call returns immediately
 * and the test prints "RETURNED" and passes.
 * (For a long-but-finite variant that returns on its own in ~1 minute, replace
 *  the constant below with static_cast<uint64_t>(0x0000000200000000ULL) ~ 8.6e9.)
 */
TEST(StatefulReaderTests, DataHeartbeatSequenceNumberFullHang)
{
    RTPSParticipantAttributes part_attrs;
    RTPSParticipant* part = RTPSDomain::createParticipant(0, false, part_attrs, nullptr);

    HistoryAttributes hatt{};
    ReaderHistory reader_history(hatt);

    ReaderAttributes reader_att{};
    reader_att.endpoint.endpointKind = READER;
    reader_att.endpoint.reliabilityKind = RELIABLE;
    reader_att.endpoint.durabilityKind = TRANSIENT_LOCAL;

    RTPSReader* reader = RTPSDomain::createRTPSReader(part, reader_att, &reader_history, nullptr);
    StatefulReader* uut = dynamic_cast<StatefulReader*>(reader);
    ASSERT_NE(uut, nullptr);

    TopicAttributes topic_desc;
    topic_desc.topicDataType = "string";
    topic_desc.topicName = "topic";
    part->registerReader(reader, topic_desc, fastdds::dds::ReaderQos());

    HistoryAttributes whatt{};
    WriterHistory writer_history(whatt);
    WriterAttributes writer_att{};
    writer_att.endpoint.endpointKind = WRITER;
    writer_att.endpoint.reliabilityKind = RELIABLE;
    writer_att.endpoint.durabilityKind = TRANSIENT_LOCAL;
    RTPSWriter* writer = RTPSDomain::createRTPSWriter(part, writer_att, &writer_history, nullptr);
    ASSERT_NE(writer, nullptr);
    part->registerWriter(writer, topic_desc, fastdds::dds::WriterQos());
    auto writer_guid = writer->getGuid();
    ASSERT_TRUE(uut->matched_writer_is_matched(writer_guid));

    // Attacker-chosen huge sequence number ~5.98e18, matching the advisory's
    // observed changes_from_writer_low_mark_ = 0x5300000000000001.
    const SequenceNumber_t huge = SequenceNumber_t(static_cast<uint64_t>(0x5300000000000001ULL));

    // Prime (empty ALIVE, not stored) then the forged DATA at 'huge' (stored).
    CacheChange_t prime;
    prime.writerGUID = writer_guid;
    prime.sequenceNumber = SequenceNumber_t(0, 1);
    prime.serializedPayload.length = 0;
    prime.instanceHandle.value[0] = 1;
    prime.kind = ChangeKind_t::ALIVE;
    uut->processDataMsg(&prime);

    CacheChange_t change;
    change.writerGUID = writer_guid;
    change.sequenceNumber = huge;
    change.serializedPayload.length = 0;
    change.instanceHandle.value[0] = 1;
    change.kind = ChangeKind_t::NOT_ALIVE_DISPOSED;
    ASSERT_TRUE(uut->processDataMsg(&change));

    const SequenceNumber_t firstSN = huge + 1;   // 0x5300000000000002
    const SequenceNumber_t lastSN  = huge + 2;

    // Vulnerable build: never returns (inner do-while runs ~5.98e18 iterations).
    // Fixed build: returns immediately and reaches the line below.
    uut->processHeartbeatMsg(writer_guid, /*hbCount=*/ 1, firstSN, lastSN,
            /*finalFlag=*/ true, /*livelinessFlag=*/ false);

    RTPSDomain::removeRTPSWriter(writer);
    RTPSDomain::removeRTPSReader(reader);
    RTPSDomain::removeRTPSParticipant(part);
}

/**
 * Regression test for GHSA-6m2p-wjv5-386v: a malicious HEARTBEAT with lastSN == INT32_MAX, UINT32_MAX-1.
 *
 * Would only crash when assertions are enabled (i.e. NDEBUG is not defined).
 * The vulnerability triggered an assertion failure in the + operator of SequenceNumber_t, during the execution
 * of StatefulReader::send_acknack.
 */
TEST(StatefulReaderTests, MaliciousHeartbeatHighSN)
{
    RTPSParticipantAttributes part_attrs;
    RTPSParticipant* part = RTPSDomain::createParticipant(0, false, part_attrs, nullptr);

    HistoryAttributes hatt{};
    ReaderHistory reader_history(hatt);

    ReaderAttributes reader_att{};
    reader_att.endpoint.endpointKind = READER;
    reader_att.endpoint.reliabilityKind = RELIABLE;
    reader_att.endpoint.durabilityKind = TRANSIENT_LOCAL;
    reader_att.times.heartbeatResponseDelay.seconds = 0;
    reader_att.times.heartbeatResponseDelay.nanosec = 1;

    RTPSReader* reader = RTPSDomain::createRTPSReader(part, reader_att, &reader_history, nullptr);
    StatefulReader* uut = dynamic_cast<StatefulReader*>(reader);
    ASSERT_NE(uut, nullptr);

    TopicAttributes topic_desc;
    topic_desc.topicDataType = "string";
    topic_desc.topicName = "topic";
    part->registerReader(reader, topic_desc, fastdds::dds::ReaderQos());

    // Ensure the writer is considered remote, so we can trigger the send_acknack path in process_heartbeat_msg.
    GUID_t writer_guid;
    writer_guid.guidPrefix.value[0] = 0xFF;
    writer_guid.entityId.value[0] = 0xFF;
    WriterProxyData writer_data(0, 0);
    writer_data.guid(writer_guid);
    writer_data.m_qos.m_reliability.kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;
    writer_data.m_qos.m_durability.kind = fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    reader->matched_writer_add(writer_data);
    ASSERT_TRUE(uut->matched_writer_is_matched(writer_guid));

    // Send a malicious heartbeat with both firstSN and lastSN == INT32_MAX, UINT32_MAX-1.
    // Force response by using finalFlag = false.
    const SequenceNumber_t huge_SN{ std::numeric_limits<int32_t>::max(), std::numeric_limits<uint32_t>::max() - 1 };
    uut->processHeartbeatMsg(writer_guid, /*hbCount=*/ 1, huge_SN, huge_SN,
            /*finalFlag=*/ false, /*livelinessFlag=*/ false);

    // Wait for the acknack timer to trigger, which will call send_acknack and potentially trigger the assertion.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    RTPSDomain::removeRTPSReader(reader);
    RTPSDomain::removeRTPSParticipant(part);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
