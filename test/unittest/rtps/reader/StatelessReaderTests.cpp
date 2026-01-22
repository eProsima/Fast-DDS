// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Header for the unit we are testing
#include <fastdds/rtps/reader/StatelessReader.h>

#include <fastdds/dds/common/InstanceHandle.hpp>

#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/SequenceNumber.h>
#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/writer/RTPSWriter.h>

#include <fastdds/rtps/builtin/data/WriterProxyData.h>

#ifdef FASTDDS_STATISTICS

void register_monitorservice_types_type_objects()
{
}

void register_types_type_objects()
{
}

#endif  // FASTDDS_STATISTICS

namespace eprosima {

namespace fastrtps {

namespace rtps {

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastdds::dds;

/* Regression test for: https://github.com/eProsima/Fast-DDS/pull/6217
   Test that checks that non empty changes (dispose, unregister, ...) with empty
   payloads are processed.
 */
TEST(StatelessReaderTests, EmptyPayloadUnregisterDisposeProcessing)
{
    RTPSParticipantAttributes part_attrs;
    RTPSParticipant* part = RTPSDomain::createParticipant(0, false, part_attrs, nullptr);

    HistoryAttributes hatt{};
    ReaderHistory reader_history(hatt);
    WriterHistory writer_history(hatt);

    ReaderAttributes reader_att{};
    reader_att.endpoint.endpointKind = READER;
    reader_att.endpoint.reliabilityKind = BEST_EFFORT;
    reader_att.endpoint.durabilityKind = VOLATILE;

    RTPSReader* reader = RTPSDomain::createRTPSReader(part, reader_att, &reader_history, nullptr);
    StatelessReader* uut = dynamic_cast<StatelessReader*>(reader);
    ASSERT_NE(uut, nullptr);

    WriterAttributes writer_att{};
    writer_att.endpoint.endpointKind = WRITER;
    writer_att.endpoint.reliabilityKind = RELIABLE;
    writer_att.endpoint.durabilityKind = TRANSIENT_LOCAL;

    RTPSWriter* writer = RTPSDomain::createRTPSWriter(part, writer_att, &writer_history, nullptr);
    ASSERT_NE(writer, nullptr);

    // Register both endpoints
    TopicAttributes topic_desc;
    topic_desc.topicKind = rtps::WITH_KEY;
    topic_desc.topicName = "topic";
    topic_desc.topicDataType = "string";
    part->registerReader(reader, topic_desc, fastdds::dds::ReaderQos());
    part->registerWriter(writer, topic_desc, fastdds::dds::WriterQos());

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
    EXPECT_TRUE(uut->processDataMsg(&change));
    change.sequenceNumber++;
    // Dispose is processed
    change.kind = ChangeKind_t::NOT_ALIVE_DISPOSED;
    EXPECT_TRUE(uut->processDataMsg(&change));
    change.sequenceNumber++;
    // Unregistered is processed
    change.kind = ChangeKind_t::NOT_ALIVE_UNREGISTERED;
    EXPECT_TRUE(uut->processDataMsg(&change));
    change.sequenceNumber++;
    // Diposed and unregistered is processed
    change.kind = ChangeKind_t::NOT_ALIVE_DISPOSED_UNREGISTERED;
    EXPECT_TRUE(uut->processDataMsg(&change));
    change.sequenceNumber++;

    // Destroy the writer
    RTPSDomain::removeRTPSWriter(writer);
    // Destroy the reader
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
