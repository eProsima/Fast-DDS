// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/rtps/common/LocatorWithMask.hpp>
#include <fastrtps/utils/IPLocator.h>

#include <rtps/network/ExternalLocatorsProcessor.hpp>

using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastrtps::rtps;

static bool operator == (
        const eprosima::fastrtps::ResourceLimitedVector<Locator>& lhs,
        const eprosima::fastdds::rtps::LocatorList& rhs)
{
    LocatorList left_list;

    for (const Locator_t& loc : lhs)
    {
        left_list.push_back(loc);
    }

    return left_list == rhs;
}

static bool operator == (
        const eprosima::fastrtps::ResourceLimitedVector<Locator>& lhs,
        const eprosima::fastrtps::ResourceLimitedVector<Locator>& rhs)
{
    LocatorList right_list;

    for (const Locator_t& loc : rhs)
    {
        right_list.push_back(loc);
    }

    return lhs == right_list;
}

static bool operator == (
        const eprosima::fastrtps::rtps::RemoteLocatorList& lhs,
        const eprosima::fastrtps::rtps::RemoteLocatorList& rhs)
{
    return lhs.multicast == rhs.multicast && lhs.unicast == rhs.unicast;
}

void single_participant_check(
        ParticipantProxyData& pdata,
        const ExternalLocators& def_ext_locators,
        const ExternalLocators& meta_ext_locators,
        const RemoteLocatorList& def_check_locators,
        const RemoteLocatorList& meta_check_locators)
{
    ExternalLocatorsProcessor::add_external_locators(pdata, def_ext_locators, meta_ext_locators);
    ASSERT_TRUE(pdata.default_locators == def_check_locators);
    ASSERT_TRUE(pdata.metatraffic_locators == meta_check_locators);
}

TEST(ExternalLocatorsProcessorTests, add_external_locators_participant)
{
    ExternalLocators empty_locators;
    RemoteLocatorList empty_test_list;
    RemoteLocatorList test_list;
    LocatorWithMask test_locator;
    std::stringstream stream("1.1.1.1:9999");
    stream >> test_locator;
    test_list.add_unicast_locator(test_locator);

    ParticipantProxyData working_data({});

    ASSERT_TRUE(working_data.metatraffic_locators == empty_test_list);
    ASSERT_TRUE(working_data.default_locators == empty_test_list);

    // Adding empty external locators should leave working_data untouched
    single_participant_check(working_data, empty_locators, empty_locators, empty_test_list, empty_test_list);

    // Adding empty external locators should leave working_data untouched
    {
        ExternalLocators accum_locators;

        for (uint8_t externality = std::numeric_limits<uint8_t>::max(); externality > 0; --externality)
        {
            for (uint8_t cost = 0; cost < std::numeric_limits<uint8_t>::max(); ++cost)
            {
                for (uint8_t mask = 0; mask < 32; ++mask)
                {
                    ExternalLocators single_locator;
                    test_locator.mask(mask);
                    single_locator[externality][cost].emplace_back(test_locator);
                    accum_locators[externality][cost].emplace_back(test_locator);

                    working_data.default_locators.unicast.clear();
                    working_data.metatraffic_locators.unicast.clear();
                    single_participant_check(working_data, accum_locators, empty_locators, test_list, empty_test_list);
                    single_participant_check(working_data, single_locator, empty_locators, test_list, empty_test_list);

                    working_data.default_locators.unicast.clear();
                    working_data.metatraffic_locators.unicast.clear();
                    single_participant_check(working_data, single_locator, empty_locators, test_list, empty_test_list);
                    single_participant_check(working_data, accum_locators, empty_locators, test_list, empty_test_list);

                    working_data.default_locators.unicast.clear();
                    working_data.metatraffic_locators.unicast.clear();
                    single_participant_check(working_data, empty_locators, accum_locators, empty_test_list, test_list);
                    single_participant_check(working_data, empty_locators, single_locator, empty_test_list, test_list);

                    working_data.default_locators.unicast.clear();
                    working_data.metatraffic_locators.unicast.clear();
                    single_participant_check(working_data, empty_locators, single_locator, empty_test_list, test_list);
                    single_participant_check(working_data, empty_locators, accum_locators, empty_test_list, test_list);

                    working_data.default_locators.unicast.clear();
                    working_data.metatraffic_locators.unicast.clear();
                    single_participant_check(working_data, accum_locators, single_locator, test_list, test_list);
                    single_participant_check(working_data, single_locator, accum_locators, test_list, test_list);

                    working_data.default_locators.unicast.clear();
                    working_data.metatraffic_locators.unicast.clear();
                    single_participant_check(working_data, single_locator, accum_locators, test_list, test_list);
                    single_participant_check(working_data, accum_locators, single_locator, test_list, test_list);
                }
            }
        }
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
