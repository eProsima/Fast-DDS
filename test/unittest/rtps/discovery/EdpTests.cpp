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

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/builtin/discovery/endpoint/EDP.h>
#include <rtps/builtin/discovery/participant/PDP.h>
#include <rtps/participant/RTPSParticipantImpl.hpp>

#if HAVE_SECURITY
#include <rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#endif // if HAVE_SECURITY

namespace eprosima {
namespace fastdds {
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
            const ParticipantProxyData& /*pdata*/,
            bool /*assign_secure_endpoints*/) override
    {
    }

    bool remove_reader(
            RTPSReader* /*R*/) override
    {
        return true;
    }

    bool remove_writer(
            RTPSWriter* /*W*/) override
    {
        return true;
    }

    bool process_reader_proxy_data(
            RTPSReader* /*reader*/,
            ReaderProxyData* /*rdata*/) override
    {
        return true;
    }

    bool process_writer_proxy_data(
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
        rdata = new ::testing::NiceMock<ReaderProxyData>((size_t)1, (size_t)1);

        //Same Topic
        wdata->topic_name = "Topic";
        rdata->topic_name = "Topic";

        //Same Topic Kind
        wdata->topic_kind = TopicKind_t::NO_KEY;
        rdata->topic_kind = TopicKind_t::NO_KEY;

        //With no type information, only Type name is compared
        rdata->type_consistency.m_force_type_validation = false;
        wdata->type_name = "TypeName";
        rdata->type_name = "TypeName";

        rdata->is_alive(true);

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
        rdata->topic_name = "AnotherTopic";
    }

    void set_incompatible_topic_kind()
    {
        rdata->topic_kind = TopicKind_t::WITH_KEY;
    }

    void set_incompatible_type()
    {
        rdata->type_name = "AnotherTypeName";
        rdata->type_information.assigned(false);
        wdata->type_information.assigned(false);
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
    wdata->partition.push_back("Partition1");
    rdata->partition.push_back("Partition1");
    check_expectations(true);

    // Add new (different) partitions. They still match on the previous one
    wdata->partition.push_back("Partition2");
    rdata->partition.push_back("Partition3");
    check_expectations(true);

    // Clear and add different partitions
    wdata->partition.clear();
    wdata->partition.push_back("Partition10");
    rdata->partition.clear();
    rdata->partition.push_back("Partition20");
    check_expectations(false);

    // Wildcard matching
    wdata->partition.clear();
    wdata->partition.push_back("Part*");
    rdata->partition.clear();
    rdata->partition.push_back("Partition");
    check_expectations(true);

    // Matching empty list against empty partition
    wdata->partition.clear();
    rdata->partition.clear();
    rdata->partition.push_back("");
    check_expectations(true);
    wdata->partition.clear();
    wdata->partition.push_back("");
    rdata->partition.clear();
    check_expectations(true);
}

TEST_F(EdpTests, CheckDurabilityCompatibility)
{
    std::vector<QosTestingCase<fastdds::dds::DurabilityQosPolicyKind>> testing_cases{
        { fastdds::dds::PERSISTENT_DURABILITY_QOS, fastdds::dds::PERSISTENT_DURABILITY_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::PERSISTENT_DURABILITY_QOS, fastdds::dds::TRANSIENT_DURABILITY_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::PERSISTENT_DURABILITY_QOS, fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::PERSISTENT_DURABILITY_QOS, fastdds::dds::VOLATILE_DURABILITY_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::TRANSIENT_DURABILITY_QOS, fastdds::dds::PERSISTENT_DURABILITY_QOS,
          fastdds::dds::DURABILITY_QOS_POLICY_ID},
        { fastdds::dds::TRANSIENT_DURABILITY_QOS, fastdds::dds::TRANSIENT_DURABILITY_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::TRANSIENT_DURABILITY_QOS, fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::TRANSIENT_DURABILITY_QOS, fastdds::dds::VOLATILE_DURABILITY_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS, fastdds::dds::PERSISTENT_DURABILITY_QOS,
          fastdds::dds::DURABILITY_QOS_POLICY_ID},
        { fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS, fastdds::dds::TRANSIENT_DURABILITY_QOS,
          fastdds::dds::DURABILITY_QOS_POLICY_ID},
        { fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS, fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS, fastdds::dds::VOLATILE_DURABILITY_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::VOLATILE_DURABILITY_QOS, fastdds::dds::PERSISTENT_DURABILITY_QOS,
          fastdds::dds::DURABILITY_QOS_POLICY_ID},
        { fastdds::dds::VOLATILE_DURABILITY_QOS, fastdds::dds::TRANSIENT_DURABILITY_QOS,
          fastdds::dds::DURABILITY_QOS_POLICY_ID},
        { fastdds::dds::VOLATILE_DURABILITY_QOS, fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS,
          fastdds::dds::DURABILITY_QOS_POLICY_ID},
        { fastdds::dds::VOLATILE_DURABILITY_QOS, fastdds::dds::VOLATILE_DURABILITY_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID}
    };

    for (auto testing_case : testing_cases)
    {
        wdata->durability.kind = testing_case.offered_qos;
        rdata->durability.kind = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST_F(EdpTests, CheckDeadlineCompatibility)
{
    std::vector<QosTestingCase<unsigned int>> testing_cases{
        { 5, 5, fastdds::dds::INVALID_QOS_POLICY_ID},
        { 5, 10, fastdds::dds::INVALID_QOS_POLICY_ID},
        { 10, 5, fastdds::dds::DEADLINE_QOS_POLICY_ID}
    };

    for (auto testing_case : testing_cases)
    {
        wdata->deadline.period = testing_case.offered_qos;
        rdata->deadline.period = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST_F(EdpTests, CheckOwnershipCompatibility)
{
    std::vector<QosTestingCase<fastdds::dds::OwnershipQosPolicyKind>> testing_cases{
        { fastdds::dds::SHARED_OWNERSHIP_QOS, fastdds::dds::SHARED_OWNERSHIP_QOS, fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::SHARED_OWNERSHIP_QOS, fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS,
          fastdds::dds::OWNERSHIP_QOS_POLICY_ID},
        { fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS, fastdds::dds::SHARED_OWNERSHIP_QOS,
          fastdds::dds::OWNERSHIP_QOS_POLICY_ID},
        { fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS, fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID}
    };

    for (auto testing_case : testing_cases)
    {
        wdata->ownership.kind = testing_case.offered_qos;
        rdata->ownership.kind = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST_F(EdpTests, CheckLivelinessKindCompatibility)
{
    std::vector<QosTestingCase<fastdds::dds::LivelinessQosPolicyKind>> testing_cases{
        { fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS, fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS, fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS, fastdds::dds::AUTOMATIC_LIVELINESS_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS,
          fastdds::dds::LIVELINESS_QOS_POLICY_ID},
        { fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, fastdds::dds::AUTOMATIC_LIVELINESS_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::AUTOMATIC_LIVELINESS_QOS, fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS,
          fastdds::dds::LIVELINESS_QOS_POLICY_ID},
        { fastdds::dds::AUTOMATIC_LIVELINESS_QOS, fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
          fastdds::dds::LIVELINESS_QOS_POLICY_ID},
        { fastdds::dds::AUTOMATIC_LIVELINESS_QOS, fastdds::dds::AUTOMATIC_LIVELINESS_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID}
    };

    for (auto testing_case : testing_cases)
    {
        wdata->liveliness.kind = testing_case.offered_qos;
        rdata->liveliness.kind = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST_F(EdpTests, CheckLeaseDurationCompatibility)
{
    std::vector<QosTestingCase<unsigned int>> testing_cases{
        { 5, 5, fastdds::dds::INVALID_QOS_POLICY_ID},
        { 5, 10, fastdds::dds::INVALID_QOS_POLICY_ID},
        { 10, 5, fastdds::dds::LIVELINESS_QOS_POLICY_ID},
    };

    for (auto testing_case : testing_cases)
    {
        wdata->liveliness.lease_duration = testing_case.offered_qos;
        rdata->liveliness.lease_duration = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST_F(EdpTests, CheckReliabilityCompatibility)
{
    std::vector<QosTestingCase<fastdds::dds::ReliabilityQosPolicyKind>> testing_cases{
        { fastdds::dds::RELIABLE_RELIABILITY_QOS, fastdds::dds::RELIABLE_RELIABILITY_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::RELIABLE_RELIABILITY_QOS, fastdds::dds::BEST_EFFORT_RELIABILITY_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID},
        { fastdds::dds::BEST_EFFORT_RELIABILITY_QOS, fastdds::dds::RELIABLE_RELIABILITY_QOS,
          fastdds::dds::RELIABILITY_QOS_POLICY_ID},
        { fastdds::dds::BEST_EFFORT_RELIABILITY_QOS, fastdds::dds::BEST_EFFORT_RELIABILITY_QOS,
          fastdds::dds::INVALID_QOS_POLICY_ID}
    };

    for (auto testing_case : testing_cases)
    {
        wdata->reliability.kind = testing_case.offered_qos;
        rdata->reliability.kind = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST_F(EdpTests, CheckPositiveAckCompatibility)
{
    std::vector<QosTestingCase<bool>> testing_cases{
        { true, true, fastdds::dds::INVALID_QOS_POLICY_ID},
        { true, false, fastdds::dds::INVALID_QOS_POLICY_ID},
        { false, true, fastdds::dds::DISABLEPOSITIVEACKS_QOS_POLICY_ID},
        { false, false, fastdds::dds::INVALID_QOS_POLICY_ID}
    };

    for (auto testing_case : testing_cases)
    {
        wdata->disable_positive_acks.enabled = testing_case.offered_qos;
        rdata->disable_positive_acks.enabled = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST_F(EdpTests, CheckDataRepresentationCompatibility)
{
    using DataRepresentationQosVector = std::vector<fastdds::dds::DataRepresentationId>;
    std::vector<QosTestingCase<DataRepresentationQosVector>> testing_cases{
        { {}, {}, fastdds::dds::INVALID_QOS_POLICY_ID},
        { {}, {fastdds::dds::DataRepresentationId::XCDR_DATA_REPRESENTATION}, fastdds::dds::INVALID_QOS_POLICY_ID},
        { {},
            {fastdds::dds::DataRepresentationId::XCDR_DATA_REPRESENTATION,
             fastdds::dds::DataRepresentationId::XCDR2_DATA_REPRESENTATION},
            fastdds::dds::INVALID_QOS_POLICY_ID},
        { {}, {fastdds::dds::DataRepresentationId::XCDR2_DATA_REPRESENTATION},
            fastdds::dds::DATAREPRESENTATION_QOS_POLICY_ID},
        { {fastdds::dds::DataRepresentationId::XCDR_DATA_REPRESENTATION}, {}, fastdds::dds::INVALID_QOS_POLICY_ID},
        { {fastdds::dds::DataRepresentationId::XCDR_DATA_REPRESENTATION},
            {fastdds::dds::DataRepresentationId::XCDR_DATA_REPRESENTATION},
            fastdds::dds::INVALID_QOS_POLICY_ID},
        { {fastdds::dds::DataRepresentationId::XCDR_DATA_REPRESENTATION},
            {fastdds::dds::DataRepresentationId::XCDR_DATA_REPRESENTATION,
             fastdds::dds::DataRepresentationId::XCDR2_DATA_REPRESENTATION},
            fastdds::dds::INVALID_QOS_POLICY_ID},
        { {fastdds::dds::DataRepresentationId::XCDR_DATA_REPRESENTATION},
            {fastdds::dds::DataRepresentationId::XCDR2_DATA_REPRESENTATION},
            fastdds::dds::DATAREPRESENTATION_QOS_POLICY_ID},
        { {fastdds::dds::DataRepresentationId::XCDR2_DATA_REPRESENTATION}, {},
            fastdds::dds::DATAREPRESENTATION_QOS_POLICY_ID},
        { {fastdds::dds::DataRepresentationId::XCDR2_DATA_REPRESENTATION},
            {fastdds::dds::DataRepresentationId::XCDR_DATA_REPRESENTATION},
            fastdds::dds::DATAREPRESENTATION_QOS_POLICY_ID},
        { {fastdds::dds::DataRepresentationId::XCDR2_DATA_REPRESENTATION},
            {fastdds::dds::DataRepresentationId::XCDR_DATA_REPRESENTATION,
             fastdds::dds::DataRepresentationId::XCDR2_DATA_REPRESENTATION},
            fastdds::dds::INVALID_QOS_POLICY_ID},
        { {fastdds::dds::DataRepresentationId::XCDR2_DATA_REPRESENTATION},
            {fastdds::dds::DataRepresentationId::XCDR2_DATA_REPRESENTATION},
            fastdds::dds::INVALID_QOS_POLICY_ID}
    };

    for (auto testing_case : testing_cases)
    {
        wdata->representation.m_value = testing_case.offered_qos;
        rdata->representation.m_value = testing_case.requested_qos;
        check_expectations(testing_case.failed_qos);
    }
}

TEST(MatchingFailureMask, matching_failure_mask_overflow)
{
    EDP::MatchingFailureMask mask;

    mask.set(EDP::MatchingFailureMask::different_topic);
    EXPECT_TRUE(mask.test(EDP::MatchingFailureMask::different_topic));

    mask.set(EDP::MatchingFailureMask::inconsistent_topic);
    EXPECT_TRUE(mask.test(EDP::MatchingFailureMask::inconsistent_topic));

    mask.set(EDP::MatchingFailureMask::incompatible_qos);
    EXPECT_TRUE(mask.test(EDP::MatchingFailureMask::incompatible_qos));

    mask.set(EDP::MatchingFailureMask::partitions);
    EXPECT_TRUE(mask.test(EDP::MatchingFailureMask::partitions));

    mask.set(EDP::MatchingFailureMask::different_typeinfo);
    EXPECT_TRUE(mask.test(EDP::MatchingFailureMask::different_typeinfo));
}

TEST_F(EdpTests, CheckTypeIdentifierComparation)
{
    dds::xtypes::TypeIdentifier minimal;
    minimal.equivalence_hash({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14});
    minimal._d(dds::xtypes::EK_MINIMAL);
    dds::xtypes::TypeIdentifier complete;
    complete.equivalence_hash({2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
    complete._d(dds::xtypes::EK_COMPLETE);

    wdata->type_information.assigned(true);
    rdata->type_information.assigned(true);

    wdata->type_information.type_information.complete().typeid_with_size().type_id(complete);
    wdata->type_information.type_information.minimal().typeid_with_size().type_id(minimal);
    rdata->type_information.type_information.complete().typeid_with_size().type_id(complete);
    rdata->type_information.type_information.minimal().typeid_with_size().type_id(minimal);
    check_expectations(true);

    wdata->type_information.type_information.complete().typeid_with_size().type_id(complete);
    wdata->type_information.type_information.minimal().typeid_with_size().type_id(minimal);
    rdata->type_information.type_information.complete().typeid_with_size().type_id(complete);
    rdata->type_information.type_information.minimal().typeid_with_size().type_id().no_value({});
    check_expectations(true);

    wdata->type_information.type_information.complete().typeid_with_size().type_id(complete);
    wdata->type_information.type_information.minimal().typeid_with_size().type_id(minimal);
    rdata->type_information.type_information.complete().typeid_with_size().type_id().no_value({});
    rdata->type_information.type_information.minimal().typeid_with_size().type_id(minimal);
    check_expectations(true);

    wdata->type_information.type_information.complete().typeid_with_size().type_id(complete);
    wdata->type_information.type_information.minimal().typeid_with_size().type_id().no_value({});
    rdata->type_information.type_information.complete().typeid_with_size().type_id(complete);
    rdata->type_information.type_information.minimal().typeid_with_size().type_id(minimal);
    check_expectations(true);

    wdata->type_information.type_information.complete().typeid_with_size().type_id().no_value({});
    wdata->type_information.type_information.minimal().typeid_with_size().type_id(minimal);
    rdata->type_information.type_information.complete().typeid_with_size().type_id(complete);
    rdata->type_information.type_information.minimal().typeid_with_size().type_id(minimal);
    check_expectations(true);

    wdata->type_information.type_information.complete().typeid_with_size().type_id().no_value({});
    wdata->type_information.type_information.minimal().typeid_with_size().type_id().no_value({});
    rdata->type_information.type_information.complete().typeid_with_size().type_id(complete);
    rdata->type_information.type_information.minimal().typeid_with_size().type_id(minimal);
    check_expectations(false);

    wdata->type_information.type_information.complete().typeid_with_size().type_id(complete);
    wdata->type_information.type_information.minimal().typeid_with_size().type_id(minimal);
    rdata->type_information.type_information.complete().typeid_with_size().type_id().no_value({});
    rdata->type_information.type_information.minimal().typeid_with_size().type_id().no_value({});
    check_expectations(false);
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
