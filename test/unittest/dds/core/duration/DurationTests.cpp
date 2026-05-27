// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <cstdint>

#include <gtest/gtest.h>

#include <fastdds/dds/core/Time_t.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/* Test the constructor with default values and with user values */
TEST(DurationTests, duration_constructor)
{
    Duration_t duration_default;
    EXPECT_EQ(duration_default.seconds, 0);
    EXPECT_EQ(duration_default.nanosec, 0);

    Duration_t duration_user(1, 500000000);
    EXPECT_EQ(duration_user.seconds, 1);
    EXPECT_EQ(duration_user.nanosec, 500000000);

    Duration_t duration_float(1.5);
    EXPECT_EQ(duration_float.seconds, 1);
    EXPECT_EQ(duration_float.nanosec, 500000000);
}

TEST(DurationTests, conversion_methods)
{
    {
        Duration_t duration(2, 250000000);
        EXPECT_EQ(duration.to_ns(), 2250000000);
        EXPECT_EQ(duration.fraction(), 1073741824);  // 1/4 of a second in fractions
    }

    {
        Duration_t duration(1, 1000000000);
        EXPECT_EQ(duration.to_ns(), 2000000000);
        EXPECT_EQ(duration.fraction(), c_TimeInfinite.fraction());
    }

    {
        Duration_t duration(1, 999999999);
        EXPECT_EQ(duration.to_ns(), 1999999999);
        EXPECT_EQ(duration.fraction(), 4294967292);
    }
}

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
