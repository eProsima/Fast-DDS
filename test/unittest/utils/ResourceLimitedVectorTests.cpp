// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/utils/collections/ResourceLimitedVector.hpp>
#include <gtest/gtest.h>

using namespace eprosima::fastdds;

// Power of two has been chosen to allow for ASSERT_EQ on capacity, as
// some implementations of std::vector would enforce power of two capacities
constexpr size_t NUM_ITEMS = 32;

class ResourceLimitedVectorTests : public ::testing::Test
{
public:

    const int testbed[NUM_ITEMS] =
    {
        1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
    };
};

TEST_F(ResourceLimitedVectorTests, default_constructor)
{
    ResourceLimitedVector<int> uut;

    // Should be empty and non-allocated
    ASSERT_TRUE(uut.empty());
    ASSERT_EQ(uut.capacity(), 0u);

    // Add all items and check no errors
    for (int i : testbed)
    {
        ASSERT_NE (uut.push_back(i), nullptr);
    }

    // Vector should be filled
    ASSERT_EQ(uut.size(), NUM_ITEMS);
    ASSERT_EQ(uut.capacity(), NUM_ITEMS);

    // Remove all items and check no errors
    for (int i : testbed)
    {
        ASSERT_TRUE(uut.remove(i));
        ASSERT_FALSE(uut.remove(i));
    }

    // Should be empty
    ASSERT_TRUE(uut.empty());
    ASSERT_EQ(uut.capacity(), NUM_ITEMS);

    // Add all items using insert
    auto it = uut.cbegin();
    for (int i : testbed)
    {
        it = uut.insert(it, i);
        ASSERT_NE(it, uut.cend());
        it = uut.insert(it, std::move(i));
        ASSERT_NE(it, uut.cend());
    }

    // Vector should be filled
    ASSERT_EQ(uut.size(), 2 * NUM_ITEMS);
    ASSERT_EQ(uut.capacity(), 2 * NUM_ITEMS);

    uut.clear();

    // Should be empty but allocated
    ASSERT_TRUE(uut.empty());
    ASSERT_EQ(uut.capacity(), 2 * NUM_ITEMS);
}

TEST_F(ResourceLimitedVectorTests, static_config)
{
    ResourceLimitedVector<int> uut(ResourceLimitedContainerConfig::fixed_size_configuration(NUM_ITEMS));

    // Capacity should have been reserved
    ASSERT_TRUE(uut.empty());
    ASSERT_EQ(uut.capacity(), NUM_ITEMS);

    // Values should be correctly added
    for (int i : testbed)
    {
        ASSERT_NE(uut.push_back(i), nullptr);
    }

    // Vector should be filled
    ASSERT_EQ(uut.size(), NUM_ITEMS);
    ASSERT_EQ(uut.capacity(), NUM_ITEMS);

    // Adding more values should return error
    for (int i : testbed)
    {
        ASSERT_EQ(uut.push_back(i), nullptr);
    }

    // Size and capacity should not have changed
    ASSERT_EQ(uut.size(), NUM_ITEMS);
    ASSERT_EQ(uut.capacity(), NUM_ITEMS);

    // Remove all items and check no errors
    for (int i : testbed)
    {
        ASSERT_TRUE(uut.remove(i));
        ASSERT_FALSE(uut.remove(i));
    }

    // Should be empty
    ASSERT_TRUE(uut.empty());
    ASSERT_EQ(uut.capacity(), NUM_ITEMS);
}

TEST_F(ResourceLimitedVectorTests, prealocated_growing_1_config)
{
    ResourceLimitedVector<int> uut(ResourceLimitedContainerConfig{ NUM_ITEMS, NUM_ITEMS * 2, 1});

    ASSERT_TRUE(uut.empty());

    // Capacity should have been reserved
    ASSERT_EQ(uut.size(), 0u);
    ASSERT_EQ(uut.capacity(), NUM_ITEMS);

    // Values should be correctly added
    for (int i : testbed)
    {
        ASSERT_NE(uut.push_back(i), nullptr);
    }

    // Vector should be filled
    ASSERT_EQ(uut.size(), NUM_ITEMS);
    ASSERT_EQ(uut.capacity(), NUM_ITEMS);

    // More values should be correctly added
    for (int i : testbed)
    {
        ASSERT_NE(uut.push_back(i), nullptr);
        ASSERT_EQ(uut.size(), NUM_ITEMS + i);
        ASSERT_GE(uut.capacity(), NUM_ITEMS + i);
    }

    // Vector should be filled
    ASSERT_EQ(uut.size(), NUM_ITEMS * 2);
    ASSERT_EQ(uut.capacity(), NUM_ITEMS * 2);

    // Adding more values should return error
    for (int i : testbed)
    {
        ASSERT_EQ(uut.push_back(i), nullptr);
    }

    // Size and capacity should not have changed
    ASSERT_EQ(uut.size(), NUM_ITEMS * 2);
    ASSERT_EQ(uut.capacity(), NUM_ITEMS * 2);

    // Remove all items and check no errors
    for (int i : testbed)
    {
        ASSERT_TRUE(uut.remove(i));
        ASSERT_TRUE(uut.remove(i));
        ASSERT_FALSE(uut.remove(i));
    }

    // Should be empty
    ASSERT_TRUE(uut.empty());
    ASSERT_EQ(uut.capacity(), NUM_ITEMS * 2);
}

TEST_F(ResourceLimitedVectorTests, prealocated_growing_n_config)
{
    ResourceLimitedVector<int> uut(ResourceLimitedContainerConfig{ NUM_ITEMS, NUM_ITEMS * 2, NUM_ITEMS });

    // Capacity should have been reserved
    ASSERT_TRUE(uut.empty());
    ASSERT_EQ(uut.capacity(), NUM_ITEMS);

    // Values should be correctly added
    for (int i : testbed)
    {
        ASSERT_NE(uut.push_back(i), nullptr);
    }

    // Vector should be filled
    ASSERT_EQ(uut.size(), NUM_ITEMS);
    ASSERT_EQ(uut.capacity(), NUM_ITEMS);

    // More values should be correctly added
    for (int i : testbed)
    {
        ASSERT_NE(uut.push_back(i), nullptr);
        ASSERT_EQ(uut.size(), NUM_ITEMS + i);
        ASSERT_EQ(uut.capacity(), NUM_ITEMS * 2);
    }

    // Vector should be filled
    ASSERT_EQ(uut.size(), NUM_ITEMS * 2);
    ASSERT_EQ(uut.capacity(), NUM_ITEMS * 2);

    // Adding more values should return error
    for (int i : testbed)
    {
        ASSERT_EQ(uut.push_back(i), nullptr);
    }

    // Size and capacity should not have changed
    ASSERT_EQ(uut.size(), NUM_ITEMS * 2);
    ASSERT_EQ(uut.capacity(), NUM_ITEMS * 2);

    // Remove all items and check no errors
    for (int i : testbed)
    {
        ASSERT_TRUE(uut.remove(i));
        ASSERT_TRUE(uut.remove(i));
        ASSERT_FALSE(uut.remove(i));
    }

    // Should be empty
    ASSERT_TRUE(uut.empty());
    ASSERT_EQ(uut.capacity(), NUM_ITEMS * 2);
}

TEST_F(ResourceLimitedVectorTests, remove_if)
{
    ResourceLimitedVector<int> uut(ResourceLimitedContainerConfig::fixed_size_configuration(NUM_ITEMS));

    // Capacity should have been reserved
    ASSERT_TRUE(uut.empty());
    ASSERT_EQ(uut.capacity(), NUM_ITEMS);

    // Add all items and check no errors
    for (int i : testbed)
    {
        ASSERT_NE(uut.push_back(i), nullptr);
    }

    // Vector should be filled
    ASSERT_EQ(uut.size(), NUM_ITEMS);
    ASSERT_EQ(uut.capacity(), NUM_ITEMS);

    auto is_odd = [](int i)
            {
                return (i & 1) != 0;
            };

    // Remove all odd items and check no errors
    for (size_t i = 0; i < NUM_ITEMS / 2; i++)
    {
        ASSERT_TRUE(uut.remove_if(is_odd));
    }

    // Trying to remove odd items should give errors
    for (size_t i = 0; i < NUM_ITEMS / 2; i++)
    {
        ASSERT_FALSE(uut.remove_if(is_odd));
    }

    // Should be half-empty
    ASSERT_EQ(uut.size(), NUM_ITEMS / 2);
    ASSERT_EQ(uut.capacity(), NUM_ITEMS);
}


int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
