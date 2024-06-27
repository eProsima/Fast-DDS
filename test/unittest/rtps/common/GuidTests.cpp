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

#include <fastdds/rtps/common/Guid.hpp>

using Guid = eprosima::fastdds::rtps::GUID_t;

namespace test {

/**
 * @brief Get a manually sorted guid vector object
 *
 * @note To test more guids just add to this vector new guids sorted manually.
 */
std::vector<Guid> get_sorted_guid_vector()
{
    std::vector<Guid> result_vector;

    // Create guids with different values to compare them afterwards
    // All zeros
    {
        Guid guid; // Default constructor
        result_vector.push_back(guid);
    }

    // Last value of entity one
    {
        Guid guid; // Default constructor
        guid.entityId.value[guid.entityId.size - 1] = 0x01;
        result_vector.push_back(guid);
    }

    // Last value of entity ff
    {
        Guid guid; // Default constructor
        guid.entityId.value[guid.entityId.size - 1] = 0xff;
        result_vector.push_back(guid);
    }

    // Value in Entity 2 to 0x66
    {
        Guid guid; // Default constructor
        guid.entityId.value[2] = 0x66;
        result_vector.push_back(guid);
    }

    // Last value of prefix one
    {
        Guid guid; // Default constructor
        guid.guidPrefix.value[guid.guidPrefix.size - 1] = 0x01;
        result_vector.push_back(guid);
    }

    // Last value of prefix one and last value of entity ff
    {
        Guid guid; // Default constructor
        guid.guidPrefix.value[guid.guidPrefix.size - 1] = 0x01;
        guid.entityId.value[guid.entityId.size - 1] = 0xff;
        result_vector.push_back(guid);
    }

    // Value in Prefix 2 to 0x66
    {
        Guid guid; // Default constructor
        guid.guidPrefix.value[6] = 0x66;
        result_vector.push_back(guid);
    }

    // First value one
    {
        Guid guid; // Default constructor
        guid.guidPrefix.value[0] = 0x01;
        result_vector.push_back(guid);
    }

    // First value ff
    {
        Guid guid; // Default constructor
        guid.guidPrefix.value[0] = 0xff;
        result_vector.push_back(guid);
    }

    // Create a vector that is sorted
    return result_vector;
}

} // namespace test

/**
 * @brief This test checks Guid compare \c operator< method.
 *
 * Uses a manually sorted vector of guids to check operator< result.
 *
 * @note this method checks cmp sign, it does not use memcmp as testing the function with itself would not give info.
 */
TEST(GuidTests, minor_opertor)
{
    auto manually_sorted_guids = test::get_sorted_guid_vector();

    for (std::size_t i = 0; i < manually_sorted_guids.size(); ++i)
    {
        for (std::size_t j = 0; j < manually_sorted_guids.size(); ++j)
        {
            bool result = manually_sorted_guids[i] < manually_sorted_guids[j];

            if (i == j)
            {
                // [i] == [j] so false
                ASSERT_FALSE(result) <<
                    "[" << i << "] " << manually_sorted_guids[i] << " < " <<
                    "[" << j << "] " << manually_sorted_guids[j];
            }
            else if (i < j)
            {
                // [i] < [j] so true
                ASSERT_TRUE(result) <<
                    "[" << i << "] " << manually_sorted_guids[i] << " < " <<
                    "[" << j << "] " << manually_sorted_guids[j];
            }
            else
            {
                // [i] > [j] so false
                ASSERT_FALSE(result) <<
                    "[" << i << "] " << manually_sorted_guids[i] << " < " <<
                    "[" << j << "] " << manually_sorted_guids[j];
            }
        }
    }
}

/**
 * @brief This test checks Guid compare \c operator== and \c operator!= method.
 *
 * Uses a manually sorted vector of entities to check cmp result.
 *
 * @note this method checks cmp sign, it does not use memcmp as testing the function with itself would not give info.
 */
TEST(GuidTests, equality)
{
    auto manually_sorted_entities = test::get_sorted_guid_vector();

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
