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

#include <fastdds/rtps/common/GuidPrefix_t.hpp>

using GuidPrefix = eprosima::fastdds::rtps::GuidPrefix_t;

namespace test {

/**
 * @brief Get a manually sorted guidprefix vector object
 *
 * @note To test more prefixes just add to this vector new prefixes sorted manually.
 */
std::vector<GuidPrefix> get_sorted_guidprefix_vector()
{
    std::vector<GuidPrefix> result_vector;

    // Create prefixes with different values to compare them afterwards
    // All zeros
    {
        GuidPrefix prefix; // Default constructor
        result_vector.push_back(prefix);
    }

    // Last value one
    {
        GuidPrefix prefix; // Default constructor
        prefix.value[prefix.size - 1] = 0x01;
        result_vector.push_back(prefix);
    }

    // Last value ff
    {
        GuidPrefix prefix; // Default constructor
        prefix.value[prefix.size - 1] = 0xff;
        result_vector.push_back(prefix);
    }

    // 6 value 0x66
    {
        GuidPrefix prefix; // Default constructor
        prefix.value[6] = 0x66;
        result_vector.push_back(prefix);
    }

    // First value one
    {
        GuidPrefix prefix; // Default constructor
        prefix.value[0] = 0x01;
        result_vector.push_back(prefix);
    }

    // First value ff
    {
        GuidPrefix prefix; // Default constructor
        prefix.value[0] = 0xff;
        result_vector.push_back(prefix);
    }

    // Create a vector that is sorted
    return result_vector;
}

} // namespace test

/**
 * @brief This test checks Guid Prefix compare \c cmp method.
 *
 * Uses a manually sorted vector of prefixes to check cmp result.
 *
 * @note this method checks cmp sign, it does not use memcmp as testing the function with itself would not give info.
 */
TEST(GuidPrefixTests, cmp)
{
    auto manually_sorted_prefixes = test::get_sorted_guidprefix_vector();

    for (std::size_t i = 0; i < manually_sorted_prefixes.size(); ++i)
    {
        for (std::size_t j = 0; j < manually_sorted_prefixes.size(); ++j)
        {
            int result = GuidPrefix::cmp(manually_sorted_prefixes[i], manually_sorted_prefixes[j]);

            if (i == j)
            {
                // [i] == [j] so cmp should be 0
                ASSERT_EQ(result, 0);
            }
            else if (i < j)
            {
                // [i] < [j] so cmp should be negative
                ASSERT_LT(result, 0);
            }
            else
            {
                // [i] > [j] so cmp should be negative
                ASSERT_GT(result, 0);
            }
        }
    }
}

/**
 * @brief This test checks Guid Prefix compare \c operator< method.
 *
 * Uses a manually sorted vector of prefixes to check operator< result.
 *
 * @note this method checks cmp sign, it does not use memcmp as testing the function with itself would not give info.
 */
TEST(GuidPrefixTests, minor_opertor)
{
    auto manually_sorted_prefixes = test::get_sorted_guidprefix_vector();

    for (std::size_t i = 0; i < manually_sorted_prefixes.size(); ++i)
    {
        for (std::size_t j = 0; j < manually_sorted_prefixes.size(); ++j)
        {
            bool result = manually_sorted_prefixes[i] < manually_sorted_prefixes[j];

            if (i == j)
            {
                // [i] == [j] so false
                ASSERT_FALSE(result);
            }
            else if (i < j)
            {
                // [i] < [j] so true
                ASSERT_TRUE(result);
            }
            else
            {
                // [i] > [j] so false
                ASSERT_FALSE(result);
            }
        }
    }
}

/**
 * @brief This test checks Guid Prefix compare \c operator== and \c operator!= method.
 *
 * Uses a manually sorted vector of entities to check cmp result.
 *
 * @note this method checks cmp sign, it does not use memcmp as testing the function with itself would not give info.
 */
TEST(GuidPrefixTests, equality)
{
    auto manually_sorted_entities = test::get_sorted_guidprefix_vector();

    for (std::size_t i = 0; i < manually_sorted_entities.size(); ++i)
    {
        for (std::size_t j = 0; j < manually_sorted_entities.size(); ++j)
        {
            bool result_equal = manually_sorted_entities[i] == manually_sorted_entities[j];
            bool result_non_equal = manually_sorted_entities[i] != manually_sorted_entities[j];

            if (i == j)
            {
                ASSERT_TRUE(result_equal);
                ASSERT_FALSE(result_non_equal);
            }
            else
            {
                ASSERT_FALSE(result_equal);
                ASSERT_TRUE(result_non_equal);
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
