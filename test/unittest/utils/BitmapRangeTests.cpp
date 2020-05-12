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
    ValueType min;
    ValueType max;
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

        if (!uut.empty())
        {
            ValueType base = uut.base();
            if (uut.max() != (base + max) || uut.min() != (base + min))
            {
                return false;
            }
        }

        return std::equal(bitmap.cbegin(), bitmap.cbegin() + num_longs, check.bitmap.cbegin());
    }
};

struct TestInputAdd
{
    uint32_t offset;

    bool perform_input(ValueType base, TestType& uut) const
    {
        return uut.add(base + offset);
    }
};

struct TestInputAddRange
{
    uint32_t offset_from;
    uint32_t offset_to;

    bool perform_input(ValueType base, TestType& uut) const
    {
        uut.add_range(base + offset_from, base + offset_to);
        return true;
    }
};

struct TestInputRemove
{
    uint32_t offset_begin;
    uint32_t offset_end;

    bool perform_input(ValueType base, TestType& uut) const
    {
        for (uint32_t offset = offset_begin; offset < offset_end; ++offset)
        {
            uut.remove(base + offset);
        }
        return true;
    }
};

template<
        typename InputType>
struct TestStep
{
    InputType input;
    TestResult expected_result;
};

template<
        typename InputType>
struct TestCase
{
    TestResult initialization;
    std::vector<TestStep<InputType>> steps;

    void Test(ValueType base, TestType& uut) const
    {
        ASSERT_TRUE(initialization.Check(initialization.result, uut));
        
        for (auto step : steps)
        {
            bool result = step.input.perform_input(base, uut);
            ASSERT_TRUE(step.expected_result.Check(result, uut));
            ASSERT_EQ(base + step.expected_result.num_bits - 1, uut.max());
        }
    }
};

class BitmapRangeTests: public ::testing::Test
{
    public:
        const ValueType explicit_base = 123UL;
        const ValueType sliding_base = 513UL;

        const TestCase<TestInputAdd> test0 =
        {
            // initialization
            {
                true, 0, 0, 0, 0, {0,0,0,0,0,0,0,0}
            },
            // steps
            {
                // Adding base
                {
                    {0},
                    {
                        true, 0, 0, 1, 1, {0x80000000UL,0,0,0,0,0,0,0}
                    }
                },
                // Adding base again
                {
                    {0},
                    {
                        true, 0, 0, 1, 1, {0x80000000UL,0,0,0,0,0,0,0}
                    }
                },
                // Adding out of range
                {
                    {256},
                    {
                        false, 0, 0, 1, 1, {0x80000000UL,0,0,0,0,0,0,0}
                    }
                },
                // Middle of first word
                {
                    {16},
                    {
                        true, 0, 16, 17, 1, {0x80008000UL,0,0,0,0,0,0,0}
                    }
                },
                // Before previous one
                {
                    {15},
                    {
                        true, 0, 16, 17, 1, {0x80018000UL,0,0,0,0,0,0,0}
                    }
                },
                // On third word
                {
                    {67},
                    {
                        true, 0, 67, 68, 3, {0x80018000UL,0,0x10000000UL,0,0,0,0,0}
                    }
                },
                // Before last on third word
                {
                    {94},
                    {
                        true, 0, 94, 95, 3, {0x80018000UL,0,0x10000002UL,0,0,0,0,0}
                    }
                },
                // Last on third word
                {
                    {95},
                    {
                        true, 0, 95, 96, 3, {0x80018000UL,0,0x10000003UL,0,0,0,0,0}
                    }
                },
                // Last possible item
                {
                    {255},
                    {
                        true, 0, 255, 256, 8, {0x80018000UL,0,0x10000003UL,0,0,0,0,0x00000001UL}
                    }
                }
            }
        };

        const TestResult all_ones =
        {
            true,
            0UL,
            255UL,
            256UL,
            8UL,
            { 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL }
        };

        const TestCase<TestInputAddRange> test_range0 =
        {
            // initialization
            {
                true, 0, 0, 0, 0, {0,0,0,0,0,0,0,0}
            },
            // steps
            {
                // Empty input
                {
                    {0, 0},
                    {
                        true, 0, 0, 0, 0, {0,0,0,0,0,0,0,0}
                    }
                },
                // Adding base
                {
                    {0, 1},
                    {
                        true, 0, 0, 1, 1, {0x80000000UL,0,0,0,0,0,0,0}
                    }
                },
                // Wrong order params
                {
                    {10, 1},
                    {
                        true, 0, 0, 1, 1, {0x80000000UL,0,0,0,0,0,0,0}
                    }
                },
                // Adding out of range
                {
                    {256, 257},
                    {
                        true, 0, 0, 1, 1, {0x80000000UL,0,0,0,0,0,0,0}
                    }
                },
                // Middle of first word
                {
                    {15, 17},
                    {
                        true, 0, 16, 17, 1, {0x80018000UL,0,0,0,0,0,0,0}
                    }
                },
                // On second and third word
                {
                    {35, 68},
                    {
                        true, 0, 67, 68, 3, {0x80018000UL,0x1FFFFFFF,0xF0000000,0,0,0,0,0}
                    }
                },
                // Crossing more than one word
                {
                    {94, 133},
                    {
                        true, 0, 132, 133, 5, {0x80018000UL,0x1FFFFFFF,0xF0000003,0xFFFFFFFF,0xF8000000,0,0,0}
                    }
                },
                // Exactly one word
                {
                    {64, 96},
                    {
                        true, 0, 132, 133, 5, {0x80018000UL,0x1FFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xF8000000,0,0,0}
                    }
                },
                // Exactly two words
                {
                    {128, 192},
                    {
                        true, 0, 191, 192, 6, {0x80018000UL,0x1FFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0,0}
                    }
                },
                // Full range
                {
                    {0, 512},
                    all_ones
                }
            }
        };

        const TestCase<TestInputRemove> test_remove0 =
        {
            // initialization (starts from full word)
            {
                true, 0, 31, 32, 1, {0xFFFFFFFFUL, 0, 0, 0, 0, 0, 0, 0}
            },
            // steps
            {
                // Removing out of range
                {
                    {32, 33},
                    {
                        true, 0, 31, 32, 1, {0xFFFFFFFFUL, 0, 0, 0, 0, 0, 0, 0}
                    }
                },
                // Removing single in the middle
                {
                    {5, 6},
                    {
                        true, 0, 31, 32, 1, {0xFBFFFFFFUL, 0, 0, 0, 0, 0, 0, 0}
                    }
                },
                // Removing several in the middle
                {
                    {6, 31},
                    {
                        true, 0, 31, 32, 1, {0xF8000001UL, 0, 0, 0, 0, 0, 0, 0}
                    }
                },
                // Removing last
                {
                    {31, 32},
                    {
                        true, 0, 4, 5, 1, {0xF8000000UL, 0, 0, 0, 0, 0, 0, 0}
                    }
                },
                // Removing first
                {
                    {0, 1},
                    {
                        true, 1, 4, 5, 1, {0x78000000UL, 0, 0, 0, 0, 0, 0, 0}
                    }
                },
                // Removing all except first and last
                {
                    {2, 4},
                    {
                        true, 1, 4, 5, 1, {0x48000000UL, 0, 0, 0, 0, 0, 0, 0}
                    }
                },
                // Removing last
                {
                    {4, 5},
                    {
                        true, 1, 1, 2, 1, {0x40000000UL, 0, 0, 0, 0, 0, 0, 0}
                    }
                },
                // Removing first
                {
                    {1, 2},
                    {
                        true, 0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}
                    }
                }
            }
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

TEST_F(BitmapRangeTests, range_default_constructor)
{
    TestType uut;
    test_range0.Test(ValueType(), uut);
}

TEST_F(BitmapRangeTests, range_explicit_constructor)
{
    TestType uut(explicit_base);
    test_range0.Test(explicit_base, uut);
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
            items.insert(explicit_base + step.input.offset);
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

TEST_F(BitmapRangeTests, sliding_window)
{
    TestType uut(sliding_base);
    uut.add(sliding_base);

    // Check shifting right and then left
    for (ValueType i = 0; i < 256UL; i++)
    {
        uut.base_update(sliding_base - i);
        ASSERT_EQ(uut.max(), sliding_base);
        uut.base_update(sliding_base);
        ASSERT_EQ(uut.max(), sliding_base);
    }

    // Check shifting left and then right
    for (ValueType i = 0; i < 256UL; i++)
    {
        uut.base_update(sliding_base - i);
        ASSERT_EQ(uut.max(), sliding_base);
        uut.base_update(sliding_base - 255UL);
        ASSERT_EQ(uut.max(), sliding_base);
    }

    // Check cases dropping the most significant bit
    uut.add(sliding_base - 100UL);
    uut.base_update(sliding_base - 256UL);
    ASSERT_EQ(uut.max(), sliding_base - 100UL);
    uut.base_update(0);
    ASSERT_TRUE(uut.empty());
}

TEST_F(BitmapRangeTests, remove)
{
    TestType uut(explicit_base);
    uut.add_range(explicit_base, explicit_base + 32UL);

    test_remove0.Test(explicit_base, uut);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
