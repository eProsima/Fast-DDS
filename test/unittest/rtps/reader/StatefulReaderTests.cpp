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
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/ReaderListener.h>
#include <fastdds/rtps/reader/RTPSReader.h>

#include <array>
#include <cstdint>
#include <cstring>
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
