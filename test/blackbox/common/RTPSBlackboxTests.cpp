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

#include <cstdlib>
#include <memory>
#include <string>
#include <thread>

#include <fastdds/dds/builtin/topic/BuiltinTopicKey.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "RTPSAsSocketReader.hpp"
#include "RTPSAsSocketWriter.hpp"
#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

//#define cout "Use Log instead!"

uint16_t global_port = 0;
//bool enable_datasharing;

uint16_t get_port()
{
    uint16_t port = static_cast<uint16_t>(GET_PID());

    if (5000 > port)
    {
        port += 5000;
    }

    std::cout << "Generating port " << port << std::endl;
    return port;
}

class BlackboxEnvironment : public ::testing::Environment
{
public:

    void SetUp()
    {
        global_port = get_port();

        // Blackbox tests were designed with the assumption that intraprocess
        // and datasharing are both disabled. Most of them use TEST_P in order to
        // test with and without intraprocess and datasharing, but those who test
        // conditions related to network packets being lost should not use intraprocessr
        // nor datasharing. Setting it off here ensures that intraprocess and
        // datasharing are only tested when required.
        eprosima::fastdds::LibrarySettings att;
        att.intraprocess_delivery = eprosima::fastdds::INTRAPROCESS_OFF;
        eprosima::fastdds::rtps::RTPSDomain::set_library_settings(att);
        //enable_datasharing = false;

        //Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);
        //Log::SetCategoryFilter(std::regex("(SECURITY)"));
    }

    void TearDown()
    {
        //Log::Reset();
        eprosima::fastdds::dds::Log::KillThread();
        eprosima::fastdds::rtps::RTPSDomain::stopAll();
    }

};

void entity_id_to_builtin_topic_key(
        eprosima::fastdds::rtps::BuiltinTopicKey_t& bt_key,
        const eprosima::fastdds::rtps::EntityId_t& entity_id)
{
    bt_key.value[0] = 0;
    bt_key.value[1] = 0;
    bt_key.value[2] = static_cast<uint32_t>(entity_id.value[0]) << 24
            | static_cast<uint32_t>(entity_id.value[1]) << 16
            | static_cast<uint32_t>(entity_id.value[2]) << 8
            | static_cast<uint32_t>(entity_id.value[3]);
}

void guid_prefix_to_builtin_topic_key(
        eprosima::fastdds::rtps::BuiltinTopicKey_t& bt_key,
        const eprosima::fastdds::rtps::GuidPrefix_t& guid_prefix)
{
    bt_key.value[0] = static_cast<uint32_t>(guid_prefix.value[0]) << 24
            | static_cast<uint32_t>(guid_prefix.value[1]) << 16
            | static_cast<uint32_t>(guid_prefix.value[2]) << 8
            | static_cast<uint32_t>(guid_prefix.value[3]);
    bt_key.value[1] = static_cast<uint32_t>(guid_prefix.value[4]) << 24
            | static_cast<uint32_t>(guid_prefix.value[5]) << 16
            | static_cast<uint32_t>(guid_prefix.value[6]) << 8
            | static_cast<uint32_t>(guid_prefix.value[7]);
    bt_key.value[2] = static_cast<uint32_t>(guid_prefix.value[8]) << 24
            | static_cast<uint32_t>(guid_prefix.value[9]) << 16
            | static_cast<uint32_t>(guid_prefix.value[10]) << 8
            | static_cast<uint32_t>(guid_prefix.value[11]);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new BlackboxEnvironment);

    return RUN_ALL_TESTS();
}
