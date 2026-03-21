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
#include <fastdds/rtps/RTPSDomain.hpp>

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

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
