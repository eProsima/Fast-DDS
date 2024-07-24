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

#include <string>
#include <utility>
#include <vector>

#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/utils/TypePropagation.hpp>

#include "../../common/utils.hpp"

#include <gtest/gtest.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::utils;

/**
 * Test for the to_type_propagation function.
 * This test checks that the function returns the expected TypePropagation value
 * for a given PropertyPolicyQos.
 */
TEST(TypePropagationTests, to_type_propagation)
{
    /* Prepare test cases */
    // Generate random string for testing the case where the property is not found
    std::string random_property_name = generate_random_string(generate_random_number(0, 254));

    // Just in case the generated string equals the property name
    if (parameter_policy_type_propagation == random_property_name)
    {
        random_property_name += "a";
    }

    // Generate random string for testing the TYPEPROPAGATION_UNKNOWN case
    std::string random_property_value = generate_random_string(generate_random_number(0, 254));

    // Just in case the generated string is a valid value for the property
    if (("enabled" == random_property_value) ||
            ("disabled" == random_property_value) ||
            ("minimal_bandwidth" == random_property_value) ||
            ("registration_only" == random_property_value))
    {
        random_property_value += "a";
    }

    // Define correct values for the property
    const std::string enabled_property_value = "enabled";
    const std::string disabled_property_value = "disabled";
    const std::string minimal_bandwidth_property_value = "minimal_bandwidth";
    const std::string registration_only_property_value = "registration_only";

    using TestCase = std::pair<std::pair<std::string, std::string>, TypePropagation>;

    const std::vector<TestCase> test_cases
    {
        {{random_property_name, random_property_value}, TypePropagation::TYPEPROPAGATION_ENABLED},
        {{random_property_name, disabled_property_value}, TypePropagation::TYPEPROPAGATION_ENABLED},
        {{random_property_name, enabled_property_value}, TypePropagation::TYPEPROPAGATION_ENABLED},
        {{random_property_name, minimal_bandwidth_property_value}, TypePropagation::TYPEPROPAGATION_ENABLED},
        {{random_property_name, registration_only_property_value}, TypePropagation::TYPEPROPAGATION_ENABLED},
        {{parameter_policy_type_propagation, random_property_value}, TypePropagation::TYPEPROPAGATION_UNKNOWN},
        {{parameter_policy_type_propagation, disabled_property_value}, TypePropagation::TYPEPROPAGATION_DISABLED},
        {{parameter_policy_type_propagation, enabled_property_value}, TypePropagation::TYPEPROPAGATION_ENABLED},
        {{parameter_policy_type_propagation, minimal_bandwidth_property_value},
            TypePropagation::TYPEPROPAGATION_MINIMAL_BANDWIDTH},
        {{parameter_policy_type_propagation, registration_only_property_value},
            TypePropagation::TYPEPROPAGATION_REGISTRATION_ONLY}
    };

    /* Test lambda */
    auto to_type_propagation_test = [](
        const std::string& property_name,
        const std::string& property_value,
        TypePropagation type_propagation) -> void
            {
                PropertyPolicyQos property_qos;
                property_qos.properties().emplace_back(property_name, property_value);

                EXPECT_EQ(type_propagation, to_type_propagation(property_qos));
            };

    /* Run test cases */
    for (const auto& test_case : test_cases)
    {
        const std::string& property_name = test_case.first.first;
        const std::string& property_value = test_case.first.second;
        const TypePropagation type_propagation = test_case.second;

        std::cout << "Checking test_case: ('" << property_name << "', '" << property_value << "') -> '" <<
            type_propagation << "'" << std::endl;
        to_type_propagation_test(property_name, property_value, type_propagation);
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
