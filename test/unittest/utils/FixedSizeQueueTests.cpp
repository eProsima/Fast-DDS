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

#include <utils/collections/FixedSizeQueue.hpp>
#include <gtest/gtest.h>

using namespace eprosima::fastrtps;

constexpr size_t CAPACITY = 32;

class FixedSizeQueueTests: public ::testing::Test
{
public:
    const int testbed[CAPACITY] = 
    {
            1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
    };

    void test_limits(FixedSizeQueue<int> &uut)
    {
        // Should be empty and allocated
        ASSERT_TRUE(uut.empty());
        ASSERT_EQ(uut.size(), 0);
        ASSERT_EQ(uut.capacity(), CAPACITY);

        // Add all items and check no errors
        for (int i : testbed)
        {
            ASSERT_EQ (uut.push(i), true);
        }

        // Queue should be filled
        ASSERT_TRUE(uut.full());
        ASSERT_EQ(uut.size(), CAPACITY);
        ASSERT_EQ(uut.capacity(), CAPACITY);

        // Try to add one more fails
        ASSERT_EQ (uut.push(CAPACITY + 1), false);

        // Remove all items and check no errors
        for (int i : testbed)
        {
            ASSERT_EQ(uut.front(), i);
            ASSERT_EQ(uut.pop(), true);
        }

        // Should be empty
        ASSERT_TRUE(uut.empty());
        ASSERT_EQ(uut.size(), 0);
        ASSERT_EQ(uut.capacity(), CAPACITY);

        // Try to remove one more fails
        ASSERT_EQ (uut.pop(), false);

        // Add some data to force overflow on buffer
        size_t c = CAPACITY / 2;
        while(--c)
        {
            ASSERT_EQ (uut.push(-1), true);
        }

        // Add the testbed, popping if needed
        for (int i : testbed)
        {
            if (!uut.push(i))
            {
                ASSERT_EQ(uut.pop(), true);
                ASSERT_EQ (uut.push(i), true);
            }
        }

        // Queue should be filled
        ASSERT_TRUE(uut.full());
        ASSERT_EQ(uut.size(), CAPACITY);
        ASSERT_EQ(uut.capacity(), CAPACITY);

        // Try to add one more fails
        ASSERT_EQ (uut.push(CAPACITY + 1), false);

        // Iterate on the values
        int i = 0;
        for (auto it = uut.begin(); it != uut.end(); ++it)
        {
            ASSERT_EQ(*it, ++i);
        }

        // Remove all items
        uut.clear();

        // Should be empty
        ASSERT_TRUE(uut.empty());
        ASSERT_EQ(uut.size(), 0);
        ASSERT_EQ(uut.capacity(), CAPACITY);

        // Try to remove one more fails
        ASSERT_EQ (uut.pop(), false);
    }
};

TEST_F(FixedSizeQueueTests, default_construct_and_initialize)
{
    FixedSizeQueue<int> uut;

    // Should be empty and non-allocated
    ASSERT_TRUE(uut.empty());
    ASSERT_EQ(uut.size(), 0);
    ASSERT_EQ(uut.capacity(), 0);

    // Initialize
    uut.init(CAPACITY);
    test_limits(uut);
}

TEST_F(FixedSizeQueueTests, initializer_constructor)
{
    FixedSizeQueue<int> uut(CAPACITY);
    test_limits(uut);
}


TEST_F(FixedSizeQueueTests, buffer_constructor)
{
    int* buffer = new int[CAPACITY];
    FixedSizeQueue<int> uut(buffer, CAPACITY);
    test_limits(uut);
    delete[] buffer;
}


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
