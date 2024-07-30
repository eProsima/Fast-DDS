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
    part->register_reader(reader, topic_desc, {});
    part->register_writer(writer, topic_desc, {});

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
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
