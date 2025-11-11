// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <climits>
#include <vector>

#include <gtest/gtest.h>

#include <fastdds/rtps/common/CacheChange.hpp>

using namespace eprosima::fastdds::rtps;

struct FragmentTestStep
{
    std::string fail_description;

    struct __Input
    {
        bool is_key_only;
        uint32_t initial_fragment;
        uint16_t num_fragments;
    }
    input;

    struct __Check
    {
        bool missing_fragments[10];
    }
    check;

    void do_test(
            CacheChange_t& uut) const
    {
        if (input.num_fragments > 0)
        {
            SerializedPayload_t payload(100);
            payload.is_serialized_key = input.is_key_only;
            payload.length = 100;
            uut.add_fragments(payload, input.initial_fragment, input.num_fragments);
        }

        FragmentNumberSet_t fns;
        uut.get_missing_fragments(fns);

        for (FragmentNumber_t i = 1; i <= 10; i++)
        {
            EXPECT_EQ(fns.is_set(i), check.missing_fragments[i - 1])
                << "  on: " << fail_description << std::endl << "  index: " << i - 1;
        }
    }

};

/*!
 * @fn TEST(CacheChange, FragmentManagement)
 * @brief This test checks the fragment management behavior of CacheChange_t.
 */
TEST(CacheChange, FragmentManagement)
{
    const FragmentTestStep test_steps[] =
    {
        {
            "initial state",
            {false, 0, 0},
            {true, true, true, true, true, true, true, true, true, true}
        },
        {
            "received out-of-bounds (11)",
            {false, 11, 1},
            {true, true, true, true, true, true, true, true, true, true}
        },
        {
            "received (2)",
            {false, 2, 1},
            {true, false, true, true, true, true, true, true, true, true}
        },
        {
            "received (2) again",
            {false, 2, 1},
            {true, false, true, true, true, true, true, true, true, true}
        },
        {
            "received (2, 3)",
            {false, 2, 2},
            {true, false, false, true, true, true, true, true, true, true}
        },
        {
            "received (1)",
            {false, 1, 1},
            {false, false, false, true, true, true, true, true, true, true}
        },
        {
            "received (1) again",
            {false, 1, 1},
            {false, false, false, true, true, true, true, true, true, true}
        },
        {
            "received (9)",
            {false, 9, 1},
            {false, false, false, true, true, true, true, true, false, true}
        },
        {
            "received (8, 9, 10, 11)",
            {false, 8, 4},
            {false, false, false, true, true, true, true, true, false, true}
        },
        {
            "received key-only (7)",
            {true, 7, 1},
            {false, false, false, true, true, true, true, true, false, true}
        },
        {
            "received (7)",
            {false, 7, 1},
            {false, false, false, true, true, true, false, true, false, true}
        },
        {
            "received (4, 5, 6)",
            {false, 4, 3},
            {false, false, false, false, false, false, false, true, false, true}
        }
    };

    CacheChange_t uut(90);
    uut.serializedPayload.length = 90;

    uut.setFragmentSize(9, true);
    for (const FragmentTestStep& step : test_steps)
    {
        step.do_test(uut);
    }
}

TEST(CacheChange, calculate_required_fragmented_payload_size)
{
    struct TestCase
    {
        uint32_t payload_size;
        uint16_t fragment_size;
        bool expected_result;
        uint32_t expected_min_required_size;
    };

    const std::vector<TestCase> test_cases =
    {
        // No fragmentation
        {100u, 0u, true, 100u},
        {50u, 100u, true, 50u},
        // Minimum fragment size
        {100u, 1u, false, 0u},
        {100u, 2u, false, 0u},
        {100u, 3u, false, 0u},
        // Fragmentation cases
        {100u, 20u, true, 100u},
        {101u, 20u, true, 104u},
        {102u, 20u, true, 104u},
        {103u, 20u, true, 104u},
        {50004u, 50003u, true, 50008u},
        {50003u, 50002u, true, 50008u},
        {50002u, 50001u, true, 50008u},
        {50001u, 50000u, true, 50004u},
        // Typical UDP max fragment size
        {1234567u, 64000u, true, 1234567u},
        {1234568u, 64000u, true, 1234568u},
        {1234569u, 64000u, true, 1234569u},
        {1234570u, 64000u, true, 1234570u},
        // Edge cases
        {5u, 4u, true, 8u},
        {6u, 4u, true, 8u},
        {7u, 4u, true, 8u},
        {8u, 4u, true, 8u},
        {
            std::numeric_limits<uint32_t>::max() - 4u - 3u,
            std::numeric_limits<uint16_t>::max(),
            true,
            std::numeric_limits<uint32_t>::max() - 4u - 3u
        },
        {
            std::numeric_limits<uint32_t>::max() - 4u - 3u,
            4u,
            true,
            std::numeric_limits<uint32_t>::max() - 4u - 3u
        }
    };

    for (const auto& test_case : test_cases)
    {
        uint32_t min_required_size = 0;
        bool result = CacheChange_t::calculate_required_fragmented_payload_size(
            test_case.payload_size,
            test_case.fragment_size,
            min_required_size);
        EXPECT_EQ(result, test_case.expected_result)
            << "Failed for payload_size=" << test_case.payload_size
            << ", fragment_size=" << test_case.fragment_size;
        if (result)
        {
            EXPECT_EQ(min_required_size, test_case.expected_min_required_size)
                << "Failed for payload_size=" << test_case.payload_size
                << ", fragment_size=" << test_case.fragment_size;
        }
    }

    // Oversized payload
    constexpr uint32_t MAX_PAYLOAD_SIZE = std::numeric_limits<uint32_t>::max() - 4u - 3u;
    for (uint32_t payload_size = std::numeric_limits<uint32_t>::max(); payload_size > MAX_PAYLOAD_SIZE; --payload_size)
    {
        uint32_t min_required_size = 0;
        EXPECT_FALSE(CacheChange_t::calculate_required_fragmented_payload_size(payload_size, 20, min_required_size));
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
