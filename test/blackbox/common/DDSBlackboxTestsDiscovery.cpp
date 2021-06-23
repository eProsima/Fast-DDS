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

#include <gtest/gtest.h>

#include "BlackboxTests.hpp"

#include "PubSubReader.hpp"

// Regression test for redmine issue 11857
TEST(DDSDiscovery, IgnoreParticipantFlags)
{
    // This participant is created with:
    // - ignoreParticipantFlags = FILTER_SAME_PROCESS (will avoid discovery of p2)
    // - metatrafficUnicastLocatorList = 127.0.0.1:7399, 127.0.0.1:7398 (to ensure two listening threads are created)
    PubSubReader<HelloWorldType> p1(TEST_TOPIC_NAME);
    p1.set_xml_filename("discovery_participant_flags.xml");
    p1.set_participant_profile("participant_1");
    p1.init();
    EXPECT_TRUE(p1.isInitialized());

    // This participant is created with initialPeersList = 127.0.0.1:7399
    // When the announcements of this participant arrive to p1, they will be ignored, and thus p1 will not
    // announce itself back to p2.
    PubSubReader<HelloWorldType> p2(TEST_TOPIC_NAME);
    p2.set_xml_filename("discovery_participant_flags.xml");
    p2.set_participant_profile("participant_2");
    p2.init();
    EXPECT_TRUE(p2.isInitialized());
    EXPECT_FALSE(p2.wait_participant_discovery(1, std::chrono::seconds(1)));
    EXPECT_FALSE(p1.wait_participant_discovery(1, std::chrono::seconds(1)));

    // This participant is created with:
    // - initialPeersList = 127.0.0.1:7398
    // - a custom guid prefix
    // The announcements of this participant will arrive to p1 on a different listening thread.
    // Due to the custom prefix, they should not be ignored, and mutual discovery should happen
    PubSubReader<HelloWorldType> p3(TEST_TOPIC_NAME);
    p3.set_xml_filename("discovery_participant_flags.xml");
    p3.set_participant_profile("participant_3");
    p3.init();
    EXPECT_TRUE(p1.wait_participant_discovery());
    EXPECT_TRUE(p3.wait_participant_discovery());
}