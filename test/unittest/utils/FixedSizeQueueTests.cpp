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

using namespace eprosima::fastdds;

constexpr size_t CAPACITY = 32;

class FixedSizeQueueTests : public ::testing::Test
{
public:

    const int testbed[CAPACITY] =
    {
        1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
    };

    bool back_insertion;

    bool push(
            FixedSizeQueue<int>& uut,
            int value)
    {
        if (back_insertion)
        {
            return uut.push_back(value);
        }
        else
        {
            return uut.push_front(value);
        }
    }

    bool pop(
            FixedSizeQueue<int>& uut)
    {
        if (back_insertion)
        {
            return uut.pop_front();
        }
        else
        {
            return uut.pop_back();
        }
    }

    int get(
            FixedSizeQueue<int>& uut)
    {
        if (back_insertion)
        {
            return uut.front();
        }
        else
        {
            return uut.back();
        }
    }

    void test_limits(
            FixedSizeQueue<int>& uut)
    {
        // Should be empty and allocated
        ASSERT_TRUE(uut.empty());
        ASSERT_EQ(uut.size(), 0u);
        ASSERT_EQ(uut.capacity(), CAPACITY);

        // Add all items and check no errors
        for (int i : testbed)
        {
            ASSERT_EQ (push(uut, i), true);
        }

        // Queue should be filled
        ASSERT_TRUE(uut.full());
        ASSERT_EQ(uut.size(), CAPACITY);
        ASSERT_EQ(uut.capacity(), CAPACITY);

        // Try to add one more fails
        ASSERT_EQ (push(uut, CAPACITY + 1), false);

        // Remove all items and check no errors
        for (int i : testbed)
        {
            ASSERT_EQ(get(uut), i);
            ASSERT_EQ(pop(uut), true);
        }

        // Should be empty
        ASSERT_TRUE(uut.empty());
        ASSERT_EQ(uut.size(), 0u);
        ASSERT_EQ(uut.capacity(), CAPACITY);

        // Try to remove one more fails
        ASSERT_EQ (pop(uut), false);

        // Add some data to force overflow on buffer
        size_t c = CAPACITY / 2;
        while (--c)
        {
            ASSERT_EQ (push(uut, -1), true);
        }

        // Add the testbed, popping if needed
        for (int i : testbed)
        {
            if (!push(uut, i))
            {
                ASSERT_EQ(pop(uut), true);
                ASSERT_EQ (push(uut, i), true);
            }
        }

        // Queue should be filled
        ASSERT_TRUE(uut.full());
        ASSERT_EQ(uut.size(), CAPACITY);
        ASSERT_EQ(uut.capacity(), CAPACITY);

        // Try to add one more fails
        ASSERT_EQ (push(uut, CAPACITY + 1), false);

        // Iterate on the values
        int i = 0;
        if (back_insertion)
        {
            for (auto it = uut.begin(); it != uut.end(); ++it)
            {
                ASSERT_EQ(*it, ++i);
            }
        }
        else
        {
            for (auto it = uut.rbegin(); it != uut.rend(); ++it)
            {
                ASSERT_EQ(*it, ++i);
            }
        }

        // Remove all items
        uut.clear();

        // Should be empty
        ASSERT_TRUE(uut.empty());
        ASSERT_EQ(uut.size(), 0u);
        ASSERT_EQ(uut.capacity(), CAPACITY);

        // Try to remove one more fails
        ASSERT_EQ (pop(uut), false);
    }

};

TEST_F(FixedSizeQueueTests, default_construct_and_initialize)
{
    FixedSizeQueue<int> uut;

    // Should be empty and non-allocated
    ASSERT_TRUE(uut.empty());
    ASSERT_EQ(uut.size(), 0u);
    ASSERT_EQ(uut.capacity(), 0u);

    // Initialize
    uut.init(CAPACITY);

    back_insertion = true;
    test_limits(uut);

    back_insertion = false;
    test_limits(uut);
}

TEST_F(FixedSizeQueueTests, initializer_constructor)
{
    FixedSizeQueue<int> uut(CAPACITY);

    back_insertion = true;
    test_limits(uut);

    back_insertion = false;
    test_limits(uut);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
