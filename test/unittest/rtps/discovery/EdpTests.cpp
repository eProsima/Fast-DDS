// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

    EDPMock(
            PDP* pdp,
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

};

class EdpTests : public ::testing::Test
{
protected:

    void SetUp() override
    {
        wdata = new ::testing::NiceMock<WriterProxyData>(1, 1);
        rdata = new ::testing::NiceMock<ReaderProxyData>(1, 1);

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

    void check_expectations(
            bool valid_matching)
    {
        EDP::MatchingFailureMask no_match_reason;
        fastdds::dds::PolicyMask incompatible_qos;

        EXPECT_EQ(edp->valid_matching(wdata, rdata, no_match_reason, incompatible_qos), valid_matching);
        EXPECT_EQ(no_match_reason.none(), valid_matching);
        EXPECT_FALSE(no_match_reason.test(EDP::MatchingFailureMask::incompatible_qos));
        EXPECT_TRUE(incompatible_qos.none());

        EXPECT_EQ(edp->valid_matching(rdata, wdata, no_match_reason, incompatible_qos), valid_matching);
        EXPECT_EQ(no_match_reason.none(), valid_matching);
        EXPECT_FALSE(no_match_reason.test(EDP::MatchingFailureMask::incompatible_qos));
        EXPECT_TRUE(incompatible_qos.none());
    }

    void check_expectations(
            bool valid_matching,
            fastdds::dds::QosPolicyId_t policy)
    {
        EDP::MatchingFailureMask no_match_reason;
        fastdds::dds::PolicyMask incompatible_qos;

        EXPECT_EQ(edp->valid_matching(wdata, rdata, no_match_reason, incompatible_qos), valid_matching);
        EXPECT_EQ(no_match_reason.none(), valid_matching);
        EXPECT_TRUE(no_match_reason.test(EDP::MatchingFailureMask::incompatible_qos));
        EXPECT_TRUE(incompatible_qos.test(policy));
        incompatible_qos.reset(policy);
        EXPECT_TRUE(incompatible_qos.none());

        EXPECT_EQ(edp->valid_matching(rdata, wdata, no_match_reason, incompatible_qos), valid_matching);
        EXPECT_EQ(no_match_reason.none(), valid_matching);
        EXPECT_TRUE(no_match_reason.test(EDP::MatchingFailureMask::incompatible_qos));
        EXPECT_TRUE(incompatible_qos.test(policy));
        incompatible_qos.reset(policy);
        EXPECT_TRUE(incompatible_qos.none());
    }

    template <typename T>
    struct QosTestingCase
    {
        T offered_qos;
        T requested_qos;
        fastdds::dds::QosPolicyId_t failed_qos;
    };

    void check_expectations(
            fastdds::dds::QosPolicyId_t failed_qos)
    {
        if (failed_qos == fastdds::dds::INVALID_QOS_POLICY_ID)
        {
            check_expectations(true);
        }
        else
        {
            check_expectations(false, failed_qos);
        }
    }

    ::testing::NiceMock<PDP> pdp_;
    ::testing::NiceMock<RTPSParticipantImpl> participant_;
    ::testing::NiceMock<WriterProxyData>* wdata;
    ::testing::NiceMock<ReaderProxyData>* rdata;
    EDPMock* edp;
};


TEST_F(EdpTests, CompleteCompatibility)
{
    check_expectations(true);
}

TEST_F(EdpTests, IncompatibleTopic)
{
    set_incompatible_topic();
    check_expectations(false);
}

TEST_F(EdpTests, IncompatibleTopicKind)
{
    set_incompatible_topic_kind();
    check_expectations(false);
}

TEST_F(EdpTests, IncompatibleType)
{
    set_incompatible_type();
    check_expectations(false);
}


TEST_F(EdpTests, CheckPartitionCompatibility)
{
    // Start with the same partitions
    wdata->m_qos.m_partition.push_back("Partition1");
    rdata->m_qos.m_partition.push_back("Partition1");
    check_expectations(true);

    // Add new (different) partitions. They still match on the previous one
    wdata->m_qos.m_partition.push_back("Partition2");
    rdata->m_qos.m_partition.push_back("Partition3");
    check_expectations(true);

    // Clear and add different partitions
    wdata->m_qos.m_partition.clear();
    wdata->m_qos.m_partition.push_back("Partition10");
    rdata->m_qos.m_partition.clear();
    rdata->m_qos.m_partition.push_back("Partition20");
    check_expectations(false);

    // Wildcard matching
    wdata->m_qos.m_partition.clear();
    wdata->m_qos.m_partition.push_back("Part*");
    rdata->m_qos.m_partition.clear();
    rdata->m_qos.m_partition.push_back("Partition");
    check_expectations(true);
}

TEST_F(EdpTests, CheckDurabilityCompatibility)
{
    std::vector<QosTestingCase<DurabilityQosPolicyKind> > testing_cases{
        { PERSISTENT_DURABILITY_QOS, PERSISTENT_DURABILITY_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { PERSISTENT_DURABILITY_QOS, TRANSIENT_DURABILITY_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { PERSISTENT_DURABILITY_QOS, TRANSIENT_LOCAL_DURABILITY_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { PERSISTENT_DURABILITY_QOS, VOLATILE_DURABILITY_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { TRANSIENT_DURABILITY_QOS, PERSISTENT_DURABILITY_QOS, fastdds::dds::DURABILITY_QOS_POLICY_ID},
        { TRANSIENT_DURABILITY_QOS, TRANSIENT_DURABILITY_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { TRANSIENT_DURABILITY_QOS, TRANSIENT_LOCAL_DURABILITY_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { TRANSIENT_DURABILITY_QOS, VOLATILE_DURABILITY_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { TRANSIENT_LOCAL_DURABILITY_QOS, PERSISTENT_DURABILITY_QOS, fastdds::dds::DURABILITY_QOS_POLICY_ID},
        { TRANSIENT_LOCAL_DURABILITY_QOS, TRANSIENT_DURABILITY_QOS, fastdds::dds::DURABILITY_QOS_POLICY_ID},
        { TRANSIENT_LOCAL_DURABILITY_QOS, TRANSIENT_LOCAL_DURABILITY_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { TRANSIENT_LOCAL_DURABILITY_QOS, VOLATILE_DURABILITY_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { VOLATILE_DURABILITY_QOS, PERSISTENT_DURABILITY_QOS, fastdds::dds::DURABILITY_QOS_POLICY_ID},
        { VOLATILE_DURABILITY_QOS, TRANSIENT_DURABILITY_QOS, fastdds::dds::DURABILITY_QOS_POLICY_ID},
        { VOLATILE_DURABILITY_QOS, TRANSIENT_LOCAL_DURABILITY_QOS, fastdds::dds::DURABILITY_QOS_POLICY_ID},
        { VOLATILE_DURABILITY_QOS, VOLATILE_DURABILITY_QOS, fastdds::dds::INVALID_QOS_POLICY_ID}
    };

    for (auto testing_case : testing_cases)
    {
        wdata->m_qos.m_durability.kind = testing_case.offered_qos;
        rdata->m_qos.m_durability.kind = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST_F(EdpTests, CheckDeadlineCompatibility)
{
    std::vector<QosTestingCase<unsigned int> > testing_cases{
        { 5, 5, fastdds::dds::INVALID_QOS_POLICY_ID},
        { 5, 10, fastdds::dds::INVALID_QOS_POLICY_ID},
        { 10, 5, fastdds::dds::DEADLINE_QOS_POLICY_ID}
    };

    for (auto testing_case : testing_cases)
    {
        wdata->m_qos.m_deadline.period = testing_case.offered_qos;
        rdata->m_qos.m_deadline.period = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST_F(EdpTests, CheckOwnershipCompatibility)
{
    std::vector<QosTestingCase<OwnershipQosPolicyKind> > testing_cases{
        { SHARED_OWNERSHIP_QOS, SHARED_OWNERSHIP_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { SHARED_OWNERSHIP_QOS, EXCLUSIVE_OWNERSHIP_QOS, fastdds::dds::OWNERSHIP_QOS_POLICY_ID},
        { EXCLUSIVE_OWNERSHIP_QOS, SHARED_OWNERSHIP_QOS, fastdds::dds::OWNERSHIP_QOS_POLICY_ID},
        { EXCLUSIVE_OWNERSHIP_QOS, EXCLUSIVE_OWNERSHIP_QOS, fastdds::dds::INVALID_QOS_POLICY_ID}
    };

    for (auto testing_case : testing_cases)
    {
        wdata->m_qos.m_ownership.kind = testing_case.offered_qos;
        rdata->m_qos.m_ownership.kind = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST_F(EdpTests, CheckLivelinessKindCompatibility)
{
    std::vector<QosTestingCase<LivelinessQosPolicyKind> > testing_cases{
        { MANUAL_BY_TOPIC_LIVELINESS_QOS, MANUAL_BY_TOPIC_LIVELINESS_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { MANUAL_BY_TOPIC_LIVELINESS_QOS, MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { MANUAL_BY_TOPIC_LIVELINESS_QOS, AUTOMATIC_LIVELINESS_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, MANUAL_BY_TOPIC_LIVELINESS_QOS, fastdds::dds::LIVELINESS_QOS_POLICY_ID},
        { MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, AUTOMATIC_LIVELINESS_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { AUTOMATIC_LIVELINESS_QOS, MANUAL_BY_TOPIC_LIVELINESS_QOS, fastdds::dds::LIVELINESS_QOS_POLICY_ID},
        { AUTOMATIC_LIVELINESS_QOS, MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, fastdds::dds::LIVELINESS_QOS_POLICY_ID},
        { AUTOMATIC_LIVELINESS_QOS, AUTOMATIC_LIVELINESS_QOS, fastdds::dds::INVALID_QOS_POLICY_ID}
    };

    for (auto testing_case : testing_cases)
    {
        wdata->m_qos.m_liveliness.kind = testing_case.offered_qos;
        rdata->m_qos.m_liveliness.kind = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST_F(EdpTests, CheckLeaseDurationCompatibility)
{
    std::vector<QosTestingCase<unsigned int> > testing_cases{
        { 5, 5, fastdds::dds::INVALID_QOS_POLICY_ID},
        { 5, 10, fastdds::dds::INVALID_QOS_POLICY_ID},
        { 10, 5, fastdds::dds::LIVELINESS_QOS_POLICY_ID},
    };

    for (auto testing_case : testing_cases)
    {
        wdata->m_qos.m_liveliness.lease_duration = testing_case.offered_qos;
        rdata->m_qos.m_liveliness.lease_duration = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST_F(EdpTests, CheckReliabilityCompatibility)
{
    std::vector<QosTestingCase<ReliabilityQosPolicyKind> > testing_cases{
        { RELIABLE_RELIABILITY_QOS, RELIABLE_RELIABILITY_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { RELIABLE_RELIABILITY_QOS, BEST_EFFORT_RELIABILITY_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { BEST_EFFORT_RELIABILITY_QOS, RELIABLE_RELIABILITY_QOS, fastdds::dds::RELIABILITY_QOS_POLICY_ID},
        { BEST_EFFORT_RELIABILITY_QOS, BEST_EFFORT_RELIABILITY_QOS, fastdds::dds::INVALID_QOS_POLICY_ID}
    };

    for (auto testing_case : testing_cases)
    {
        wdata->m_qos.m_reliability.kind = testing_case.offered_qos;
        rdata->m_qos.m_reliability.kind = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST_F(EdpTests, CheckPositiveAckCompatibility)
{
    std::vector<QosTestingCase<bool> > testing_cases{
        { true, true, fastdds::dds::INVALID_QOS_POLICY_ID},
        { true, false, fastdds::dds::INVALID_QOS_POLICY_ID},
        { false, true, fastdds::dds::DISABLEPOSITIVEACKS_QOS_POLICY_ID},
        { false, false, fastdds::dds::INVALID_QOS_POLICY_ID}
    };

    for (auto testing_case : testing_cases)
    {
        wdata->m_qos.m_disablePositiveACKs.enabled = testing_case.offered_qos;
        rdata->m_qos.m_disablePositiveACKs.enabled = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST_F(EdpTests, CheckDataSharingCompatibility)
{
    // None is datasharing
    wdata->m_qos.data_sharing.disable();
    rdata->m_qos.data_sharing.disable();
    check_expectations(true);

    // One is datasharing, the other one is not
    wdata->m_qos.data_sharing.force("path");
    rdata->m_qos.data_sharing.disable();
    check_expectations(false, fastdds::dds::DATASHARING_QOS_POLICY_ID);

    rdata->m_qos.data_sharing.force("path");
    wdata->m_qos.data_sharing.disable();
    check_expectations(false, fastdds::dds::DATASHARING_QOS_POLICY_ID);

    // Both are datasharing with no common ID
    std::vector<uint16_t> wdomain_ids;
    wdomain_ids.push_back(10);
    wdata->m_qos.data_sharing.force("path", wdomain_ids);
    std::vector<uint16_t> rdomain_ids;
    rdomain_ids.push_back(20);
    rdata->m_qos.data_sharing.force("path", rdomain_ids);
    check_expectations(false, fastdds::dds::DATASHARING_QOS_POLICY_ID);

    // Add a common ID
    wdomain_ids.push_back(30);
    wdata->m_qos.data_sharing.force("path", wdomain_ids);
    rdomain_ids.push_back(30);
    rdata->m_qos.data_sharing.force("path", rdomain_ids);
    check_expectations(true);
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
