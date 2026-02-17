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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/builtin/topic/ParticipantBuiltinTopicData.hpp>
#include "BlackboxTests.hpp"
#include "PubSubParticipant.hpp"

using namespace eprosima::fastdds;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class UserDataQos : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = false;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

/**
 * This test checks that the user data updates once the participant is initialized are correctly applied
 * In order to check that the user data is correctly updated, two participants are created and the discovery info is
 * checked.
 */
TEST_P(UserDataQos, update_user_data_qos)
{
    PubSubParticipant<HelloWorldPubSubType> participant_1(0u, 0u, 0u, 0u);
    ASSERT_TRUE(participant_1.user_data({'a', 'b', 'c', 'd', 'e'}).init_participant());

    PubSubParticipant<HelloWorldPubSubType> participant_2(0u, 0u, 0u, 0u);

    participant_2.set_on_discovery_function([&](const rtps::ParticipantBuiltinTopicData& info) -> bool
            {
                std::cout << "Received USER_DATA: ";
                for (auto i : info.user_data)
                {
                    std::cout << i << ' ';
                }
                std::cout << std::endl;
                return info.user_data == std::vector<rtps::octet>({'a', 'b', 'c', 'd', 'e'});
            });
    participant_2.set_on_participant_qos_update_function([&](const rtps::ParticipantBuiltinTopicData& info) -> bool
            {
                std::cout << "Received USER_DATA: ";
                for (auto i : info.user_data)
                {
                    std::cout << i << ' ';
                }
                std::cout << std::endl;
                return info.user_data == std::vector<rtps::octet>({'f', 'g'});
            });

    ASSERT_TRUE(participant_2.init_participant());

    participant_1.wait_discovery();
    participant_2.wait_discovery_result();

    // Update user data
    ASSERT_TRUE(participant_1.update_user_data({'f', 'g'}));

    participant_2.wait_qos_update();
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(UserDataQos,
        UserDataQos,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<UserDataQos::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case DATASHARING:
                    return "Datasharing";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }
        });
