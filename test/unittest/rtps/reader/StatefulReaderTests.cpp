// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <rtps/reader/StatefulReader.hpp>

#include <fastdds/config.h>
#include <fastdds/dds/common/InstanceHandle.hpp>

#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/SequenceNumber.h>
#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/writer/RTPSWriter.h>

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
} // namespace fastdds

namespace fastrtps {
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
    TopicAttributes topic_att;
    topic_att.topicKind = NO_KEY;
    topic_att.topicDataType = "string";
    topic_att.topicName = "topic";
    part->registerReader(reader, topic_att, {});
    part->registerWriter(writer, topic_att, {});

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

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
