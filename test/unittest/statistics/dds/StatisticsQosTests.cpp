// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <string>

#include <gtest/gtest.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.h>
#include <fastdds/rtps/flowcontrol/FlowControllerConsts.hpp>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/statistics/dds/subscriber/qos/DataReaderQos.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

<<<<<<< HEAD
=======
#ifdef FASTDDS_STATISTICS
namespace {

// Accessor to DomainParticipantImpl* from DomainParticipant
using DomainParticipantImplPtr = efd::DomainParticipantImpl * efd::DomainParticipant::*;

DomainParticipantImplPtr get_domain_participant_impl_ptr();

template<DomainParticipantImplPtr P>
struct DomainParticipantImplAccessor
{
    friend DomainParticipantImplPtr get_domain_participant_impl_ptr()
    {
        return P;
    }

};

template struct DomainParticipantImplAccessor<&efd::DomainParticipant::impl_>;

// Accessor to builtin_publisher_ from statistics DomainParticipantImpl
using BuiltinPublisherPtr =
        efd::Publisher * eprosima::fastdds::statistics::dds::DomainParticipantImpl::*;

BuiltinPublisherPtr get_builtin_publisher_ptr();

template<BuiltinPublisherPtr P>
struct BuiltinPublisherAccessor
{
    friend BuiltinPublisherPtr get_builtin_publisher_ptr()
    {
        return P;
    }

};

template struct BuiltinPublisherAccessor<
    & eprosima::fastdds::statistics::dds::DomainParticipantImpl::builtin_publisher_>;

} // namespace
#endif // ifdef FASTDDS_STATISTICS

class StatisticsFromXMLProfileTests : public ::testing::Test
{
public:

#ifdef FASTDDS_STATISTICS
    eprosima::fastdds::statistics::dds::DomainParticipant* statistics_participant = nullptr;
    eprosima::fastdds::dds::Publisher* statistics_publisher = nullptr;

    void test_setup_XMLConfigurationForStatisticsDataWritersQoS(
            eprosima::fastdds::statistics::dds::DomainParticipant*& _statistics_participant,
            eprosima::fastdds::dds::Publisher*& statistics_pub,
            const std::string& xml)
    {

        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_string(xml.c_str(),
                xml.size());

        // Create DomainParticipant. Builtin statistics DataWriters are also created, as auto-enable option is true by default.
        eprosima::fastdds::dds::DomainParticipant* participant =
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                        create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
        ASSERT_NE(participant, nullptr);

        // Obtain pointer to child class
        _statistics_participant =
                eprosima::fastdds::statistics::dds::DomainParticipant::narrow(participant);
        ASSERT_NE(_statistics_participant, nullptr);

        // Get DomainParticipantImpl via accessor (no UB downcast)
        efd::DomainParticipantImpl* base_impl =
                _statistics_participant->*get_domain_participant_impl_ptr();
        ASSERT_NE(base_impl, nullptr);

        eprosima::fastdds::statistics::dds::DomainParticipantImpl* domain_statistics_participant_impl =
                static_cast<eprosima::fastdds::statistics::dds::DomainParticipantImpl*>(base_impl);
        ASSERT_NE(domain_statistics_participant_impl, nullptr);

        // Get Publisher via accessor
        statistics_pub =
                domain_statistics_participant_impl->*get_builtin_publisher_ptr();
        ASSERT_NE(statistics_pub, nullptr);
    }

#endif // ifdef FASTDDS_STATISTICS
};

>>>>>>> 25a43a7c3 (Add UBSan workflow and solve its errors (#6386))
/*
 * This test checks that STATISTICS_DATAWRITER_QOS correctly sets the expected QoS.
 * 1. Reliability RELIABLE
 * 2. Durability TRANSIENT LOCAL
 * 3. Pull mode enabled
 * 4. Publication mode ASYNCHRONOUS with custom flow controller
 * 5. History kind KEEP LAST
 * 6. History depth 1
 */
TEST(StatisticsQosTests, StatisticsDataWriterQosTest)
{
    EXPECT_EQ(STATISTICS_DATAWRITER_QOS.reliability().kind, eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    EXPECT_EQ(STATISTICS_DATAWRITER_QOS.durability().kind, eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS);
    const std::string* pushMode_property = eprosima::fastrtps::rtps::PropertyPolicyHelper::find_property(
        STATISTICS_DATAWRITER_QOS.properties(), "fastdds.push_mode");
    ASSERT_NE(pushMode_property, nullptr);
    EXPECT_EQ(pushMode_property->compare("false"), 0);
    EXPECT_EQ(STATISTICS_DATAWRITER_QOS.publish_mode().kind, eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE);
    EXPECT_EQ(STATISTICS_DATAWRITER_QOS.publish_mode().flow_controller_name,
            eprosima::fastdds::rtps::FASTDDS_STATISTICS_FLOW_CONTROLLER_DEFAULT);
    EXPECT_EQ(STATISTICS_DATAWRITER_QOS.history().kind, eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS);
    EXPECT_EQ(STATISTICS_DATAWRITER_QOS.history().depth, 1);
}

/*
 * This test checks that STATISTICS_DATAREADER_QOS correctly sets the expected QoS.
 * 1. Reliability RELIABLE
 * 2. Durability TRANSIENT LOCAL
 * 3. History kind KEEP LAST
 * 4. History depth 100
 */
TEST(StatisticsQosTests, StatisticsDataReaderQosTest)
{
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.reliability().kind == eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.durability().kind == eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS);
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.history().kind == eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS);
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.history().depth == 100);
}

} // namespace dds
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Error);

    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

    eprosima::fastdds::dds::Log::KillThread();
    return ret;
}
