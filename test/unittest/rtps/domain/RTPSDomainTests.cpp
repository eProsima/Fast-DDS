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

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/RTPSDomain.h>

/**
 * This test checks the getter and setter for the library settings in the RTPS layer.
 */
TEST(RTPSDomainTests, library_settings_test)
{
    eprosima::fastdds::LibrarySettings library_settings;
    EXPECT_TRUE(eprosima::fastdds::rtps::RTPSDomain::get_library_settings(library_settings));
    // Get LibrarySettings default values
#if HAVE_STRICT_REALTIME
    EXPECT_EQ(eprosima::fastdds::INTRAPROCESS_OFF, library_settings.intraprocess_delivery);
#else
    EXPECT_EQ(eprosima::fastdds::INTRAPROCESS_FULL, library_settings.intraprocess_delivery);
#endif // if HAVE_STRICT_REALTIME
    library_settings.intraprocess_delivery = eprosima::fastdds::INTRAPROCESS_USER_DATA_ONLY;
    // Setting the library settings within an empty RTPSDomain shall return true
    EXPECT_TRUE(eprosima::fastdds::rtps::RTPSDomain::set_library_settings(library_settings));
    EXPECT_TRUE(eprosima::fastdds::rtps::RTPSDomain::get_library_settings(library_settings));
    EXPECT_EQ(eprosima::fastdds::INTRAPROCESS_USER_DATA_ONLY, library_settings.intraprocess_delivery);
    // Create RTPSParticipant
    eprosima::fastdds::rtps::RTPSParticipantAttributes part_attr;
    eprosima::fastdds::rtps::RTPSParticipant* participant =
            eprosima::fastdds::rtps::RTPSDomain::createParticipant(0, part_attr);
    ASSERT_NE(nullptr, participant);
    library_settings.intraprocess_delivery = eprosima::fastdds::INTRAPROCESS_OFF;
    // Setting LibrarySettings with any RTPSParticipant shall fail
    EXPECT_FALSE(eprosima::fastdds::rtps::RTPSDomain::set_library_settings(library_settings));
    EXPECT_TRUE(eprosima::fastdds::rtps::RTPSDomain::get_library_settings(library_settings));
    EXPECT_EQ(eprosima::fastdds::INTRAPROCESS_USER_DATA_ONLY, library_settings.intraprocess_delivery);
    // Remove RTPSParticipant
    EXPECT_TRUE(eprosima::fastdds::rtps::RTPSDomain::removeRTPSParticipant(participant));
    library_settings.intraprocess_delivery = eprosima::fastdds::INTRAPROCESS_OFF;
    // Setting LibrarySettings with no participants shall suceed
    EXPECT_TRUE(eprosima::fastdds::rtps::RTPSDomain::set_library_settings(library_settings));
    EXPECT_TRUE(eprosima::fastdds::rtps::RTPSDomain::get_library_settings(library_settings));
    EXPECT_EQ(eprosima::fastdds::INTRAPROCESS_OFF, library_settings.intraprocess_delivery);
    eprosima::fastdds::rtps::RTPSDomain::stopAll();
}

/**
 * This test checks get_topic_attributes_from_profile API.
 */
TEST(RTPSDomainTests, get_topic_attributes_from_profile_test)
{
    std::string profile_name = "test_profile_name";
    eprosima::fastdds::TopicAttributes topic_att;
    EXPECT_FALSE(eprosima::fastdds::rtps::RTPSDomain::get_topic_attributes_from_profile(profile_name, topic_att));

    const std::string xml =
            R"(<profiles>
    <topic profile_name="test_profile_name">
        <name>Test</name>
        <dataType>DataTest</dataType>
        <historyQos>
            <kind>KEEP_LAST</kind>
            <depth>20</depth>
        </historyQos>
        <resourceLimitsQos>
            <max_samples>5</max_samples>
            <max_instances>2</max_instances>
            <max_samples_per_instance>1</max_samples_per_instance>
            <allocated_samples>20</allocated_samples>
            <extra_samples>10</extra_samples>
        </resourceLimitsQos>
    </topic>
</profiles>)";

    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_string(xml.c_str(),
            xml.length()), eprosima::fastdds::dds::RETCODE_OK);
    EXPECT_TRUE(eprosima::fastdds::rtps::RTPSDomain::get_topic_attributes_from_profile(profile_name, topic_att));
    EXPECT_EQ(topic_att.topicName, "Test");
    EXPECT_EQ(topic_att.topicDataType, "DataTest");
    EXPECT_EQ(topic_att.historyQos.kind, eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS);
    EXPECT_EQ(topic_att.historyQos.depth, 20);
    EXPECT_EQ(topic_att.resourceLimitsQos.max_samples, 5);
    EXPECT_EQ(topic_att.resourceLimitsQos.max_instances, 2);
    EXPECT_EQ(topic_att.resourceLimitsQos.max_samples_per_instance, 1);
    EXPECT_EQ(topic_att.resourceLimitsQos.allocated_samples, 20);
    EXPECT_EQ(topic_att.resourceLimitsQos.extra_samples, 10);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
