// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <limits>
#include <thread>

// Header for the unit we are testing
#include <rtps/reader/StatefulReader.hpp>

#include <fastdds/config.hpp>
#include <fastdds/dds/common/InstanceHandle.hpp>

#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/TopicDescription.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <rtps/builtin/data/WriterProxyData.hpp>

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
namespace dds {

const InstanceHandle_t HANDLE_NIL;

} // namespace dds

namespace rtps {

/* Regression Test for improving gaps processing
 * https://github.com/eProsima/Fast-DDS/pull/3343
 */
TEST(StatefulReaderTests, RTPSCorrectGAPProcessing)
{
    RTPSParticipantAttributes part_attrs;
    RTPSParticipant* part = RTPSDomain::createParticipant(0, false, part_attrs, nullptr);

    HistoryAttributes hatt{};
    ReaderHistory reader_history(hatt);
    WriterHistory writer_history(hatt);

    ReaderAttributes reader_att{};
    reader_att.endpoint.endpointKind = READER;
    reader_att.endpoint.reliabilityKind = RELIABLE;
    reader_att.endpoint.durabilityKind = TRANSIENT_LOCAL;

    RTPSReader* reader = RTPSDomain::createRTPSReader(part, reader_att, &reader_history, nullptr);
    StatefulReader* uut = dynamic_cast<StatefulReader*>(reader);
    ASSERT_NE(uut, nullptr);

    WriterAttributes writer_att{};
    writer_att.endpoint.endpointKind = WRITER;
    writer_att.endpoint.reliabilityKind = RELIABLE;
    writer_att.endpoint.durabilityKind = TRANSIENT_LOCAL;

    RTPSWriter* writer = RTPSDomain::createRTPSWriter(part, writer_att, &writer_history, nullptr);
    ASSERT_NE(writer, nullptr);

    // Register both endpoints
    TopicDescription topic_desc;
    topic_desc.type_name = "string";
    topic_desc.topic_name = "topic";
    part->register_reader(reader, topic_desc, fastdds::dds::ReaderQos());
    part->register_writer(writer, topic_desc, fastdds::dds::WriterQos());

    // After registration, the writer should be matched
    auto writer_guid = writer->getGuid();
    EXPECT_TRUE(uut->matched_writer_is_matched(writer_guid));

    // Send a wrong GAP
    SequenceNumberSet_t seq_set(SequenceNumber_t(0, 0));
    ASSERT_NO_FATAL_FAILURE(uut->process_gap_msg(writer_guid, {0, 0}, seq_set));

    // Destroy the writer
    RTPSDomain::removeRTPSWriter(writer);

    // Destroy the reader
    RTPSDomain::removeRTPSReader(reader);
}

/* Regression test for: https://github.com/eProsima/Fast-DDS/pull/6217
   Test that checks that non empty changes (dispose, unregister, ...) with empty
   payloads are processed.
 */
TEST(StatefulReaderTests, EmptyPayloadUnregisterDisposeProcessing)
{
    RTPSParticipantAttributes part_attrs;
    RTPSParticipant* part = RTPSDomain::createParticipant(0, false, part_attrs, nullptr);

    HistoryAttributes hatt{};
    ReaderHistory reader_history(hatt);
    WriterHistory writer_history(hatt);

    ReaderAttributes reader_att{};
    reader_att.endpoint.endpointKind = READER;
    reader_att.endpoint.reliabilityKind = RELIABLE;
    reader_att.endpoint.durabilityKind = TRANSIENT_LOCAL;

    RTPSReader* reader = RTPSDomain::createRTPSReader(part, reader_att, &reader_history, nullptr);
    StatefulReader* uut = dynamic_cast<StatefulReader*>(reader);
    ASSERT_NE(uut, nullptr);

    WriterAttributes writer_att{};
    writer_att.endpoint.endpointKind = WRITER;
    writer_att.endpoint.reliabilityKind = RELIABLE;
    writer_att.endpoint.durabilityKind = TRANSIENT_LOCAL;

    RTPSWriter* writer = RTPSDomain::createRTPSWriter(part, writer_att, &writer_history, nullptr);
    ASSERT_NE(writer, nullptr);

    // Register both endpoints
    TopicDescription topic_desc;
    topic_desc.type_name = "string";
    topic_desc.topic_name = "topic";
    part->register_reader(reader, topic_desc, fastdds::dds::ReaderQos());
    part->register_writer(writer, topic_desc, fastdds::dds::WriterQos());

    // After registration, the writer should be matched
    auto writer_guid = writer->getGuid();
    EXPECT_TRUE(uut->matched_writer_is_matched(writer_guid));

    CacheChange_t change;
    change.writerGUID = writer_guid;
    change.sequenceNumber = {0, 1};
    change.serializedPayload.length = 0;
    change.instanceHandle.value[0] = 1;
    // Alive change is not actually processed, but the method returns true because
    // the sample is considered non relevant
    change.kind = ChangeKind_t::ALIVE;
    EXPECT_TRUE(uut->process_data_msg(&change));
    change.sequenceNumber++;
    // Dispose is processed
    change.kind = ChangeKind_t::NOT_ALIVE_DISPOSED;
    EXPECT_TRUE(uut->process_data_msg(&change));
    change.sequenceNumber++;
    // Unregistered is processed
    change.kind = ChangeKind_t::NOT_ALIVE_UNREGISTERED;
    EXPECT_TRUE(uut->process_data_msg(&change));
    change.sequenceNumber++;
    // Diposed and unregistered is processed
    change.kind = ChangeKind_t::NOT_ALIVE_DISPOSED_UNREGISTERED;
    EXPECT_TRUE(uut->process_data_msg(&change));
    change.sequenceNumber++;

    // Destroy the writer
    RTPSDomain::removeRTPSWriter(writer);
    // Destroy the reader
    RTPSDomain::removeRTPSReader(reader);
}

/* Vulnerability reproducer for GHSA-f4hw-9j9g-j8xj: an excessive-iteration CPU-exhaustion DoS in
 * WriterProxy::process_gap.
 *
 * It measures the time taken to process a GAP starting at low_mark+1 (special case) with a gap_list.base() of
 * 2e7 vs 2e8.
 * Without the patch, the second call takes ~10x longer than the first, because of the for-loop in process_gap_msg.
 */
TEST(StatefulReaderTests, GapUnboundedSequenceRangeIteration)
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

    TopicDescription topic_desc;
    topic_desc.type_name = "string";
    topic_desc.topic_name = "topic";
    part->register_reader(reader, topic_desc, fastdds::dds::ReaderQos());

    // measure_gap_us(base, gap_start_low): fresh writer proxy (changes_from_writer_low_mark_ ==
    // max_sequence_number_ == 0 on match), then a single GAP with gap_list.base() == base and
    // gapStart == {0, gap_start_low}. Returns elapsed microseconds for the process_gap_msg call.
    auto measure_gap_us = [&](uint64_t base, uint32_t gap_start_low) -> int64_t
            {
                HistoryAttributes whatt{};
                WriterHistory writer_history(whatt);
                WriterAttributes writer_att{};
                writer_att.endpoint.endpointKind = WRITER;
                writer_att.endpoint.reliabilityKind = RELIABLE;
                writer_att.endpoint.durabilityKind = TRANSIENT_LOCAL;

                RTPSWriter* writer = RTPSDomain::createRTPSWriter(part, writer_att, &writer_history, nullptr);
                EXPECT_NE(writer, nullptr);
                part->register_writer(writer, topic_desc, fastdds::dds::WriterQos());
                auto writer_guid = writer->getGuid();
                EXPECT_TRUE(uut->matched_writer_is_matched(writer_guid));

                // Attacker-forged GAP: small gap_start + huge gap_list.base(). No upper-bound check on
                // gap_list.base() in StatefulReader::process_gap_msg / WriterProxy::process_gap.
                SequenceNumber_t gap_start(0, gap_start_low);
                SequenceNumberSet_t gap_list(SequenceNumber_t(static_cast<uint64_t>(base)));

                auto t0 = std::chrono::steady_clock::now();
                uut->process_gap_msg(writer_guid, gap_start, gap_list);
                auto t1 = std::chrono::steady_clock::now();

                RTPSDomain::removeRTPSWriter(writer);
                return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
            };

    const uint64_t small_base = 20000000ULL;   // 2e7
    const uint64_t large_base = 200000000ULL;  // 2e8 (10x)

    // gap_start == low_mark + 1 (== {0,1}) : special case entered -> bound = gap_list.base()+256 -> loop O(base).
    int64_t small_us = measure_gap_us(small_base, 1);
    int64_t large_us = measure_gap_us(large_base, 1);
    double ratio = (small_us > 0 ? static_cast<double>(large_us) / static_cast<double>(small_us) : -1.0);

    // CONTROL: identical huge base, gap_start == low_mark + 2 (== {0,2}) : special case NOT entered
    // -> max_allowed_gap = low_mark + 256 -> finalSN = min(base, 256) = 256 -> ~254 iterations.
    int64_t capped_us = measure_gap_us(large_base, 2);

    std::cout << "small_base=" << small_base << " -> " << small_us << " us" << std::endl;
    std::cout << "large_base=" << large_base << " (10x) -> " << large_us << " us" << std::endl;
    std::cout << "ratio=" << ratio << std::endl;
    std::cout << "CONTROL capped (gap_start=low_mark+2, same base) -> " << capped_us << " us" << std::endl;

    // A patched, O(stored changes) implementation would make all three calls return in comparable
    // (near-zero) time regardless of base. The vulnerable O(sequence-number range) loop makes
    // large_us ~10x small_us, and both dwarf the (correctly 256-capped) control.
    if (small_us < large_us)
    {
        EXPECT_LT(ratio, 5.0);
    }

    RTPSDomain::removeRTPSReader(reader);
}

/* Variation of GapUnboundedSequenceRangeIteration that uses a gap_start below the low mark, which shall be treated
 * the same as gap_start == low_mark + 1 (special case) and thus also triggers the O(base) loop.
 */
TEST(StatefulReaderTests, GapUnboundedSequenceRangeIterationLessThanLowMark)
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

    TopicDescription topic_desc;
    topic_desc.type_name = "string";
    topic_desc.topic_name = "topic";
    part->register_reader(reader, topic_desc, fastdds::dds::ReaderQos());

    // measure_gap_us(base, gap_start_low): fresh writer proxy (changes_from_writer_low_mark_ ==
    // max_sequence_number_ == 0 on match), then a single GAP with gap_list.base() == base and
    // gapStart == {0, gap_start_low}. Returns elapsed microseconds for the process_gap_msg call.
    auto measure_gap_us = [&](uint64_t base, uint32_t gap_start_low, uint32_t low_mark) -> int64_t
            {
                HistoryAttributes whatt{};
                WriterHistory writer_history(whatt);
                WriterAttributes writer_att{};
                writer_att.endpoint.endpointKind = WRITER;
                writer_att.endpoint.reliabilityKind = RELIABLE;
                writer_att.endpoint.durabilityKind = TRANSIENT_LOCAL;

                RTPSWriter* writer = RTPSDomain::createRTPSWriter(part, writer_att, &writer_history, nullptr);
                EXPECT_NE(writer, nullptr);
                part->register_writer(writer, topic_desc, fastdds::dds::WriterQos());
                auto writer_guid = writer->getGuid();
                EXPECT_TRUE(uut->matched_writer_is_matched(writer_guid));

                // Set the low mark to a non-zero value, so that gap_start < low_mark triggers the special case.
                SequenceNumber_t low_mark_sn(0, low_mark);
                uut->process_heartbeat_msg(writer_guid, 1u, low_mark_sn + 1u, low_mark_sn, true, false);

                // Attacker-forged GAP: small gap_start + huge gap_list.base(). No upper-bound check on
                // gap_list.base() in StatefulReader::process_gap_msg / WriterProxy::process_gap.
                SequenceNumber_t gap_start(0, gap_start_low);
                SequenceNumberSet_t gap_list(SequenceNumber_t(static_cast<uint64_t>(base)));

                auto t0 = std::chrono::steady_clock::now();
                uut->process_gap_msg(writer_guid, gap_start, gap_list);
                auto t1 = std::chrono::steady_clock::now();

                RTPSDomain::removeRTPSWriter(writer);
                return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
            };

    const uint64_t small_base = 20000000ULL;   // 2e7
    const uint64_t large_base = 200000000ULL;  // 2e8 (10x)

    // gap_start < low_mark : special case entered -> bound = gap_list.base()+256 -> loop O(base).
    int64_t small_us = measure_gap_us(small_base, 91, 100);
    int64_t large_us = measure_gap_us(large_base, 91, 100);
    double ratio = (small_us > 0 ? static_cast<double>(large_us) / static_cast<double>(small_us) : -1.0);

    // CONTROL: identical huge base, gap_start == low_mark + 2 (== {0,2}) : special case NOT entered
    // -> max_allowed_gap = low_mark + 256 -> finalSN = min(base, 256) = 256 -> ~254 iterations.
    int64_t capped_us = measure_gap_us(large_base, 102, 100);

    std::cout << "small_base=" << small_base << " -> " << small_us << " us" << std::endl;
    std::cout << "large_base=" << large_base << " (10x) -> " << large_us << " us" << std::endl;
    std::cout << "ratio=" << ratio << std::endl;
    std::cout << "CONTROL capped (gap_start=low_mark+2, same base) -> " << capped_us << " us" << std::endl;

    // A patched, O(stored changes) implementation would make all three calls return in comparable
    // (near-zero) time regardless of base. The vulnerable O(sequence-number range) loop makes
    // large_us ~10x small_us, and both dwarf the (correctly 256-capped) control.
    if (small_us < large_us)
    {
        EXPECT_LT(ratio, 5.0);
    }

    RTPSDomain::removeRTPSReader(reader);
}

/* Live demonstration of the actual GAP denial of service for GHSA-f4hw-9j9g-j8xj.
 *
 * A single crafted GAP (small gap_start + huge gap_list.base()) pins one core at 100% CPU and
 * process_gap_msg() effectively hangs for any practical observation period.
 *
 * Designed to fail by timeout.
 * While it runs:
 *   - `top -H -p <pid>`  shows one thread at 100% CPU;
 *   - `gdb -p <pid>` then `thread apply all bt` shows the stack pinned in
 *       WriterProxy::received_change_set  <-  WriterProxy::process_gap  (the for-loop body)
 *       <-  StatefulReader::process_gap_msg
 *     with changes_from_writer_low_mark_ climbing toward gap_list.base() while RSS stays flat.
 */
TEST(StatefulReaderTests, GapProcessingFullHang)
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

    TopicDescription topic_desc;
    topic_desc.type_name = "string";
    topic_desc.topic_name = "topic";
    part->register_reader(reader, topic_desc, fastdds::dds::ReaderQos());

    HistoryAttributes whatt{};
    WriterHistory writer_history(whatt);
    WriterAttributes writer_att{};
    writer_att.endpoint.endpointKind = WRITER;
    writer_att.endpoint.reliabilityKind = RELIABLE;
    writer_att.endpoint.durabilityKind = TRANSIENT_LOCAL;
    RTPSWriter* writer = RTPSDomain::createRTPSWriter(part, writer_att, &writer_history, nullptr);
    ASSERT_NE(writer, nullptr);
    part->register_writer(writer, topic_desc, fastdds::dds::WriterQos());
    auto writer_guid = writer->getGuid();
    ASSERT_TRUE(uut->matched_writer_is_matched(writer_guid));

    // gap_start == low_mark + 1 (special case) + attacker-chosen huge gap_list.base() ~5.98e18,
    // near the RTPS 63-bit positive maximum (~9.22e18). This is far larger than the smaller inflated
    // low marks seen in the supplemental on-wire captures; it is chosen only to make the practical hang
    // obvious. (The 0x53 high byte is an artifact of the original fuzzing seed 'S' == 0x53.)
    SequenceNumber_t gap_start(0, 1);
    SequenceNumberSet_t gap_list(SequenceNumber_t(static_cast<uint64_t>(0x5300000000000001ULL)));

    // Vulnerable build: does not return within any practical observation window
    // (process_gap for-loop runs ~5.98e18 iterations).
    // Fixed build: returns promptly and reaches the line below.
    uut->process_gap_msg(writer_guid, gap_start, gap_list);

    RTPSDomain::removeRTPSWriter(writer);
    RTPSDomain::removeRTPSReader(reader);
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

    TopicDescription topic_desc;
    topic_desc.type_name = "string";
    topic_desc.topic_name = "topic";
    part->register_reader(reader, topic_desc, fastdds::dds::ReaderQos());

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
                part->register_writer(writer, topic_desc, fastdds::dds::WriterQos());
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
                uut->process_data_msg(&prime);

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
                EXPECT_TRUE(uut->process_data_msg(&change));

                // (b) Attacker-forged HEARTBEAT: no upper bound check in
                // WriterProxy::process_heartbeat / lost_changes_update either.
                SequenceNumber_t firstSN = huge_sn + 1;
                SequenceNumber_t lastSN = huge_sn + 2;

                auto t0 = std::chrono::steady_clock::now();
                uut->process_heartbeat_msg(writer_guid, /*hbCount=*/ 1, firstSN, lastSN,
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
                part->register_writer(writer, topic_desc, fastdds::dds::WriterQos());
                auto writer_guid = writer->getGuid();
                EXPECT_TRUE(uut->matched_writer_is_matched(writer_guid));
                // No DATA change stored: HEARTBEAT alone.
                SequenceNumber_t firstSN = huge_sn + 1;
                SequenceNumber_t lastSN = huge_sn + 2;
                auto t0 = std::chrono::steady_clock::now();
                uut->process_heartbeat_msg(writer_guid, /*hbCount=*/ 1, firstSN, lastSN,
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

    // Destroy the reader
    RTPSDomain::removeRTPSReader(reader);
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

    TopicDescription topic_desc;
    topic_desc.type_name = "string";
    topic_desc.topic_name = "topic";
    part->register_reader(reader, topic_desc, fastdds::dds::ReaderQos());

    HistoryAttributes whatt{};
    WriterHistory writer_history(whatt);
    WriterAttributes writer_att{};
    writer_att.endpoint.endpointKind = WRITER;
    writer_att.endpoint.reliabilityKind = RELIABLE;
    writer_att.endpoint.durabilityKind = TRANSIENT_LOCAL;
    RTPSWriter* writer = RTPSDomain::createRTPSWriter(part, writer_att, &writer_history, nullptr);
    ASSERT_NE(writer, nullptr);
    part->register_writer(writer, topic_desc, fastdds::dds::WriterQos());
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
    uut->process_data_msg(&prime);

    CacheChange_t change;
    change.writerGUID = writer_guid;
    change.sequenceNumber = huge;
    change.serializedPayload.length = 0;
    change.instanceHandle.value[0] = 1;
    change.kind = ChangeKind_t::NOT_ALIVE_DISPOSED;
    ASSERT_TRUE(uut->process_data_msg(&change));

    const SequenceNumber_t firstSN = huge + 1;   // 0x5300000000000002
    const SequenceNumber_t lastSN  = huge + 2;

    // Vulnerable build: never returns (inner do-while runs ~5.98e18 iterations).
    // Fixed build: returns immediately and reaches the line below.
    uut->process_heartbeat_msg(writer_guid, /*hbCount=*/ 1, firstSN, lastSN,
            /*finalFlag=*/ true, /*livelinessFlag=*/ false);

    RTPSDomain::removeRTPSWriter(writer);
    RTPSDomain::removeRTPSReader(reader);
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
    reader_att.times.heartbeat_response_delay.seconds = 0;
    reader_att.times.heartbeat_response_delay.nanosec = 1;

    RTPSReader* reader = RTPSDomain::createRTPSReader(part, reader_att, &reader_history, nullptr);
    StatefulReader* uut = dynamic_cast<StatefulReader*>(reader);
    ASSERT_NE(uut, nullptr);

    TopicDescription topic_desc;
    topic_desc.type_name = "string";
    topic_desc.topic_name = "topic";
    part->register_reader(reader, topic_desc, fastdds::dds::ReaderQos());

    // Ensure the writer is considered remote, so we can trigger the send_acknack path in process_heartbeat_msg.
    GUID_t writer_guid;
    writer_guid.guidPrefix.value[0] = 0xFF;
    writer_guid.entityId.value[0] = 0xFF;
    PublicationBuiltinTopicData writer_data;
    writer_data.guid = writer_guid;
    writer_data.reliability.kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;
    writer_data.durability.kind = fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    reader->matched_writer_add(writer_data);
    ASSERT_TRUE(uut->matched_writer_is_matched(writer_guid));

    // Send a malicious heartbeat with both firstSN and lastSN == INT32_MAX, UINT32_MAX-1.
    // Force response by using finalFlag = false.
    const SequenceNumber_t huge_SN{ std::numeric_limits<int32_t>::max(), std::numeric_limits<uint32_t>::max() - 1 };
    uut->process_heartbeat_msg(writer_guid, /*hbCount=*/ 1, huge_SN, huge_SN,
            /*finalFlag=*/ false, /*livelinessFlag=*/ false);

    // Wait for the acknack timer to trigger, which will call send_acknack and potentially trigger the assertion.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    RTPSDomain::removeRTPSReader(reader);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
