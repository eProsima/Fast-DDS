// Copyright 220Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
/*
#define TEST_FRIENDS \
    FRIEND_TEST(WriterProxyTests, MissingChangesUpdate); \
    FRIEND_TEST(WriterProxyTests, LostChangesUpdate); \
    FRIEND_TEST(WriterProxyTests, ReceivedChangeSet); \
    FRIEND_TEST(WriterProxyTests, IrrelevantChangeSet);
*/
#include <fastdds/rtps/builtin/discovery/endpoint/EDP.h>
#include <fastdds/rtps/builtin/discovery/participant/PDP.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <rtps/participant/RTPSParticipantImpl.h>


namespace eprosima {
namespace fastrtps {
namespace rtps {

using ::testing::Return;
using ::testing::ReturnRef;

class EDPMock : public EDP
{
public:
    EDPMock(PDP* pdp,
            RTPSParticipantImpl* part)
    : EDP(pdp, part)
    {
    }

    virtual ~EDPMock()
    {
    }

    bool initEDP(
            BuiltinAttributes& /*attributes*/) override
    {
        return true;
    }

    void assignRemoteEndpoints(
            const ParticipantProxyData& /*pdata*/) override
    {
    }

    bool removeLocalReader(
            RTPSReader* /*R*/) override
    {
        return true;
    }

    bool removeLocalWriter(
            RTPSWriter* /*W*/) override
    {
        return true;
    }

    bool processLocalReaderProxyData(
            RTPSReader* /*reader*/,
            ReaderProxyData* /*rdata*/) override
    {
        return true;
    }

    bool processLocalWriterProxyData(
            RTPSWriter* /*writer*/,
            WriterProxyData* /*wdata*/) override
    {
        return true;
    }

    fastdds::dds::PolicyMask reader_incompatible_qos_policies(GUID_t rguid)
    {
        return reader_status_[rguid].requested_incompatible_qos;
    }

    fastdds::dds::PolicyMask writer_incompatible_qos_policies(GUID_t wguid)
    {
        return writer_status_[wguid].offered_incompatible_qos;
    }

};

class EdpTests : public ::testing::Test
{
protected:

    void SetUp() override
    {
        wdata = new ::testing::NiceMock<WriterProxyData>(1,1);
        rdata = new ::testing::NiceMock<ReaderProxyData>(1,1);

        //Same Topic
        wdata->topicName("Topic");
        rdata->topicName("Topic");

        //Same Topic Kind
        wdata->topicKind(TopicKind_t::NO_KEY);
        rdata->topicKind(TopicKind_t::NO_KEY);

        //With no type information, only Type name is compared
        rdata->m_qos.type_consistency.m_force_type_validation = false;
        wdata->typeName("TypeName");
        rdata->typeName("TypeName");

        rdata->isAlive(true);
        wdata->isAlive(true);

        edp = new EDPMock(&pdp_, &participant_);
    }

    void TearDown() override
    {
        delete edp;
        delete wdata;
        delete rdata;
    }

    void set_incompatible_topic()
    {
        rdata->topicName("AnotherTopic");
    }

    void set_incompatible_topic_kind()
    {
        rdata->topicKind(TopicKind_t::WITH_KEY);
    }

    void set_incompatible_type()
    {
        rdata->typeName("AnotherTypeName");
    }


    ::testing::NiceMock<PDP> pdp_;
    ::testing::NiceMock<RTPSParticipantImpl> participant_;
    ::testing::NiceMock<WriterProxyData>* wdata;
    ::testing::NiceMock<ReaderProxyData>* rdata;
    EDPMock* edp;
};


TEST_F(EdpTests, CompleteCompatibility)
{
    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());
}

TEST_F(EdpTests, IncompatibleTopic)
{
    set_incompatible_topic();
    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());
}

TEST_F(EdpTests, IncompatibleTopicKind)
{
    set_incompatible_topic_kind();
    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());
}

TEST_F(EdpTests, IncompatibleType)
{
    set_incompatible_type();
    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());
}

TEST_F(EdpTests, CheckDurabilityCompatibility)
{
    wdata->m_qos.m_durability.kind = PERSISTENT_DURABILITY_QOS;
    rdata->m_qos.m_durability.kind = TRANSIENT_DURABILITY_QOS;

    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

    wdata->m_qos.m_durability.kind = TRANSIENT_DURABILITY_QOS;
    rdata->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;

    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

    wdata->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    rdata->m_qos.m_durability.kind = VOLATILE_DURABILITY_QOS;

    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

    wdata->m_qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    rdata->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;

    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).test(fastdds::dds::DURABILITY_QOS_POLICY_ID));

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).test(fastdds::dds::DURABILITY_QOS_POLICY_ID));

    wdata->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    rdata->m_qos.m_durability.kind = TRANSIENT_DURABILITY_QOS;

    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).test(fastdds::dds::DURABILITY_QOS_POLICY_ID));

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).test(fastdds::dds::DURABILITY_QOS_POLICY_ID));

    wdata->m_qos.m_durability.kind = TRANSIENT_DURABILITY_QOS;
    rdata->m_qos.m_durability.kind = PERSISTENT_DURABILITY_QOS;

    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).test(fastdds::dds::DURABILITY_QOS_POLICY_ID));

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).test(fastdds::dds::DURABILITY_QOS_POLICY_ID));
}

TEST_F(EdpTests, CheckDeadlineCompatibility)
{
    wdata->m_qos.m_deadline.period = 5;
    rdata->m_qos.m_deadline.period = 10;

    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

    wdata->m_qos.m_deadline.period = 10;
    rdata->m_qos.m_deadline.period = 5;

    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).test(fastdds::dds::DEADLINE_QOS_POLICY_ID));

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).test(fastdds::dds::DEADLINE_QOS_POLICY_ID));
}

TEST_F(EdpTests, CheckOwnershipCompatibility)
{
    wdata->m_qos.m_ownership.kind = SHARED_OWNERSHIP_QOS;
    rdata->m_qos.m_ownership.kind = SHARED_OWNERSHIP_QOS;

    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

    wdata->m_qos.m_ownership.kind = EXCLUSIVE_OWNERSHIP_QOS;
    rdata->m_qos.m_ownership.kind = EXCLUSIVE_OWNERSHIP_QOS;

    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

    wdata->m_qos.m_ownership.kind = SHARED_OWNERSHIP_QOS;
    rdata->m_qos.m_ownership.kind = EXCLUSIVE_OWNERSHIP_QOS;

    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).test(fastdds::dds::OWNERSHIP_QOS_POLICY_ID));

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).test(fastdds::dds::OWNERSHIP_QOS_POLICY_ID));

    wdata->m_qos.m_ownership.kind = EXCLUSIVE_OWNERSHIP_QOS;
    rdata->m_qos.m_ownership.kind = SHARED_OWNERSHIP_QOS;

    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).test(fastdds::dds::OWNERSHIP_QOS_POLICY_ID));

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).test(fastdds::dds::OWNERSHIP_QOS_POLICY_ID));
}

TEST_F(EdpTests, CheckLivelinessKindCompatibility)
{
    wdata->m_qos.m_liveliness.kind = MANUAL_BY_TOPIC_LIVELINESS_QOS;
    rdata->m_qos.m_liveliness.kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;

    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

    wdata->m_qos.m_liveliness.kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
    rdata->m_qos.m_liveliness.kind = AUTOMATIC_LIVELINESS_QOS;

    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

    wdata->m_qos.m_liveliness.kind = AUTOMATIC_LIVELINESS_QOS;
    rdata->m_qos.m_liveliness.kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;

    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).test(fastdds::dds::LIVELINESS_QOS_POLICY_ID));

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).test(fastdds::dds::LIVELINESS_QOS_POLICY_ID));

    wdata->m_qos.m_liveliness.kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
    rdata->m_qos.m_liveliness.kind = MANUAL_BY_TOPIC_LIVELINESS_QOS;

    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).test(fastdds::dds::LIVELINESS_QOS_POLICY_ID));

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).test(fastdds::dds::LIVELINESS_QOS_POLICY_ID));
}

TEST_F(EdpTests, CheckLeaseDurationCompatibility)
{
    wdata->m_qos.m_liveliness.lease_duration = 5;
    rdata->m_qos.m_liveliness.lease_duration = 10;

    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

    wdata->m_qos.m_liveliness.lease_duration = 10;
    rdata->m_qos.m_liveliness.lease_duration = 5;

    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).test(fastdds::dds::LIVELINESS_QOS_POLICY_ID));

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).test(fastdds::dds::LIVELINESS_QOS_POLICY_ID));
}

TEST_F(EdpTests, CheckReliabilityCompatibility)
{
    wdata->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    rdata->m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;

    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

    wdata->m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    rdata->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).test(fastdds::dds::RELIABILITY_QOS_POLICY_ID));

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).test(fastdds::dds::RELIABILITY_QOS_POLICY_ID));
}

TEST_F(EdpTests, CheckPositiveAckCompatibility)
{
    wdata->m_qos.m_disablePositiveACKs.enabled = true;
    rdata->m_qos.m_disablePositiveACKs.enabled = false;

    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

    wdata->m_qos.m_disablePositiveACKs.enabled = false;
    rdata->m_qos.m_disablePositiveACKs.enabled = true;

    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).test(fastdds::dds::DISABLEPOSITIVEACKS_QOS_POLICY_ID));

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).test(fastdds::dds::DISABLEPOSITIVEACKS_QOS_POLICY_ID));
}

TEST_F(EdpTests, CheckPartitionCompatibility)
{
    // Start with the same partitions
    wdata->m_qos.m_partition.push_back("Partition1");
    rdata->m_qos.m_partition.push_back("Partition1");

    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

    // Add new (different) partitions. They still match on the previous one
    wdata->m_qos.m_partition.push_back("Partition2");
    rdata->m_qos.m_partition.push_back("Partition3");

    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

    // Clear and add different partitions
    wdata->m_qos.m_partition.clear();
    wdata->m_qos.m_partition.push_back("Partition10");
    rdata->m_qos.m_partition.clear();
    rdata->m_qos.m_partition.push_back("Partition20");

    EXPECT_FALSE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_FALSE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

    // Wildcard matching
    wdata->m_qos.m_partition.clear();
    wdata->m_qos.m_partition.push_back("Part*");
    rdata->m_qos.m_partition.clear();
    rdata->m_qos.m_partition.push_back("Partition");

    EXPECT_TRUE(edp->validMatching(wdata, rdata));
    EXPECT_TRUE(edp->writer_incompatible_qos_policies(wdata->guid()).none());

    EXPECT_TRUE(edp->validMatching(rdata, wdata));
    EXPECT_TRUE(edp->reader_incompatible_qos_policies(rdata->guid()).none());

}


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

int main(int argc, char **argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}

