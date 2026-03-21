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

#include <fastdds/rtps/common/EntityId_t.hpp>

using EntityId = eprosima::fastdds::rtps::EntityId_t;

namespace test {

/**
 * @brief Get a manually sorted entityid vector object
 *
 * @note To test more entities just add to this vector new entities sorted manually.
 */
std::vector<EntityId> get_sorted_entityid_vector()
{
    std::vector<EntityId> result_vector;

    // Create entities with different values to compare them afterwards
    // All zeros
    {
        EntityId entity; // Default constructor
        result_vector.push_back(entity);
    }

    // Last value one
    {
        EntityId entity; // Default constructor
        entity.value[entity.size - 1] = 0x01;
        result_vector.push_back(entity);
    }

    // Last value ff
    {
        EntityId entity; // Default constructor
        entity.value[entity.size - 1] = 0xff;
        result_vector.push_back(entity);
    }

    // 2 value 0x66
    {
        EntityId entity; // Default constructor
        entity.value[2] = 0x66;
        result_vector.push_back(entity);
    }

    // First value one
    {
        EntityId entity; // Default constructor
        entity.value[0] = 0x01;
        result_vector.push_back(entity);
    }

    // First value ff
    {
        EntityId entity; // Default constructor
        entity.value[0] = 0xff;
        result_vector.push_back(entity);
    }

    // Create a vector that is sorted
    return result_vector;
}

} // namespace test

/**
 * @brief This test checks Entity Id compare \c cmp method.
 *
 * Uses a manually sorted vector of entities to check cmp result.
 *
 * @note this method checks cmp sign, it does not use memcmp as testing the function with itself would not give info.
 */
TEST(EntityIdTests, cmp)
{
    auto manually_sorted_entities = test::get_sorted_entityid_vector();

    for (std::size_t i = 0; i < manually_sorted_entities.size(); ++i)
    {
        for (std::size_t j = 0; j < manually_sorted_entities.size(); ++j)
        {
            int result = EntityId::cmp(manually_sorted_entities[i], manually_sorted_entities[j]);

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
 * @brief This test checks Entity Id compare \c operator< method.
 *
 * Uses a manually sorted vector of entities to check operator< result.
 *
 * @note this method checks cmp sign, it does not use memcmp as testing the function with itself would not give info.
 */
TEST(EntityIdTests, minor_opertor)
{
    auto manually_sorted_entities = test::get_sorted_entityid_vector();

    for (std::size_t i = 0; i < manually_sorted_entities.size(); ++i)
    {
        for (std::size_t j = 0; j < manually_sorted_entities.size(); ++j)
        {
            bool result = manually_sorted_entities[i] < manually_sorted_entities[j];

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
 * @brief This test checks Entity Id compare \c operator== and \c operator!= method.
 *
 * Uses a manually sorted vector of entities to check cmp result.
 *
 * @note this method checks cmp sign, it does not use memcmp as testing the function with itself would not give info.
 */
TEST(EntityIdTests, equality)
{
    auto manually_sorted_entities = test::get_sorted_entityid_vector();

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
