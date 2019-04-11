// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/utils/fixed_size_bitmap.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <vector>
#include <set>

using ValueType = uint32_t;
using TestType = eprosima::fastrtps::BitmapRange<ValueType>;

struct TestResult
{
    bool result;
    uint32_t num_bits;
    uint32_t num_longs;
    TestType::bitmap_type bitmap;

    bool Check(bool ret_val, TestType& uut) const
    {
        if (result != ret_val)
        {
            return false;
        }

        TestResult check;
        uut.bitmap_get(check.num_bits, check.bitmap, check.num_longs);
        if (num_bits != check.num_bits || num_longs != check.num_longs)
        {
            return false;
        }
        return std::equal(bitmap.cbegin(), bitmap.cbegin() + num_longs, check.bitmap.cbegin());
    }
};

struct TestStep
{
    uint32_t input_offset;
    TestResult expected_result;
};

struct TestCase
{
    TestResult initialization;
    std::vector<TestStep> steps;

    void Test(ValueType base, TestType& uut) const
    {
        ASSERT_TRUE(initialization.Check(initialization.result, uut));

        
        for (auto step : steps)
        {
            bool result = uut.add(base + step.input_offset);
            ASSERT_TRUE(step.expected_result.Check(result, uut));
            ASSERT_EQ(base + step.expected_result.num_bits - 1, uut.max());
        }
    }
};

class BitmapRangeTests: public ::testing::Test
{
    public:
        const ValueType explicit_base = 123UL;

        const TestCase test0 =
        {
            // initialization
            {
                true, 0, 0, {0,0,0,0,0,0,0,0}
            },
            // steps
            {
                // Adding base
                {
                    0,
                    {
                        true, 1, 1, {0x80000000UL,0,0,0,0,0,0,0}
                    }
                },
                // Adding base again
                {
                    0,
                    {
                        true, 1, 1, {0x80000000UL,0,0,0,0,0,0,0}
                    }
                },
                // Adding out of range
                {
                    256,
                    {
                        false, 1, 1, {0x80000000UL,0,0,0,0,0,0,0}
                    }
                },
                // Middle of first word
                {
                    16,
                    {
                        true, 17, 1, {0x80008000UL,0,0,0,0,0,0,0}
                    }
                },
                // Before previous one
                {
                    15,
                    {
                        true, 17, 1, {0x80018000UL,0,0,0,0,0,0,0}
                    }
                },
                // On third word
                {
                    67,
                    {
                        true, 68, 3, {0x80018000UL,0,0x10000000UL,0,0,0,0,0}
                    }
                },
                // Last on third word
                {
                    94,
                    {
                        true, 95, 3, {0x80018000UL,0,0x10000002UL,0,0,0,0,0}
                    }
                },
                // Last on third word
                {
                    95,
                    {
                        true, 96, 3, {0x80018000UL,0,0x10000003UL,0,0,0,0,0}
                    }
                },
                // Last possible item
                {
                    255,
                    {
                        true, 256, 8, {0x80018000UL,0,0x10000003UL,0,0,0,0,0x00000001UL}
                    }
                }
            }
        };

        const TestResult all_ones =
        {
            true,
            256UL,
            8UL,
            { 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL }
        };
};

TEST_F(BitmapRangeTests, default_constructor)
{
    TestType uut;
    test0.Test(ValueType(), uut);
}

TEST_F(BitmapRangeTests, explicit_constructor)
{
    TestType uut(explicit_base);
    test0.Test(explicit_base, uut);
}

TEST_F(BitmapRangeTests, change_base)
{
    // Test with default constructor
    TestType uut;
    test0.Test(ValueType(), uut);

    // Change base and test again
    uut.base(explicit_base);
    test0.Test(explicit_base, uut);
}

TEST_F(BitmapRangeTests, full_range)
{
    TestType uut(explicit_base);

    // Add all possible items in range
    for (ValueType item = explicit_base, last = explicit_base + 256UL; item < last; item++)
    {
        ASSERT_TRUE(uut.add(item));
    }

    all_ones.Check(all_ones.result, uut);
}

TEST_F(BitmapRangeTests, serialization)
{
    uint32_t num_bits;
    uint32_t num_longs;
    TestType::bitmap_type bitmap;

    // Populate the range using the test case
    TestType uut(explicit_base);
    test0.Test(explicit_base, uut);

    // Get bitmap serialization and set it again
    uut.bitmap_get(num_bits, bitmap, num_longs);
    uut.bitmap_set(num_bits, bitmap.data());

    // Bitmap should be equal to the one of the last result
    auto last_result = test0.steps.rbegin()->expected_result;
    last_result.Check(last_result.result, uut);
}

TEST_F(BitmapRangeTests, traversal)
{
    // Populate the range using the test case
    TestType uut(explicit_base);
    test0.Test(explicit_base, uut);

    // Collect the items that should be processed
    std::set<ValueType> items;
    for (auto step : test0.steps)
    {
        if (step.expected_result.result)
        {
            items.insert(explicit_base + step.input_offset);
        }
    }

    // Functor should only be called for items in the set, which are removed
    uut.for_each([&](const ValueType& t)
    {
        ASSERT_NE(items.find(t), items.end());
        items.erase(t);
    });

    // All items should have been processed
    ASSERT_TRUE(items.empty());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
