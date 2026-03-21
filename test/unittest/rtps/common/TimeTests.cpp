// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/rtps/common/Time_t.hpp>

using namespace eprosima::fastdds::rtps;

/*
 * Test << operator
 * NOTE: The conversion from double to Time_t is not equitative, so the string conversion is not exact to double
 *  i.e: Time_t(0.3) = 0.299999999
 */
TEST(TimeTest, serialize_operator)
{
    std::stringstream ss;

    Time_t t1;
    ss << t1;
    ASSERT_EQ(ss.str(), "0.0");
    ss.str("");
    ss.clear();

    // Set sec and nanosec
    Time_t t2(1, 0);
    t2.nanosec(1);
    ss << t2;
    ASSERT_EQ(ss.str(), "1.1");
    ss.str("");
    ss.clear();

    Time_t t3(4346, 0);
    t3.nanosec(415672901);
    ss << t3;
    ASSERT_EQ(ss.str(), "4346.415672901");
    ss.str("");
    ss.clear();

    // double constructor
    Time_t t4(2.0);
    ss << t4;
    ASSERT_EQ(ss.str(), "2.0");
    ss.str("");
    ss.clear();

    //! numeric converison when fraction precision
    Time_t t5(0.000002);
    ss << t5;
    ASSERT_EQ(ss.str(), "0.1999");
    ss.str("");
    ss.clear();

    // sec & frac constructor
    Time_t t6(4346, 5);
    ss << t6;
    ASSERT_EQ(ss.str(), "4346.1");
    ss.str("");
    ss.clear();
}

/*
 * Test >> operator
 */
TEST(TimeTest, deserialize_operator)
{
    Time_t t;

    // This does not work due to double precision
    // std::stringstream st1("1.02");
    // st1 >> t;
    // ASSERT_EQ(t.seconds(), 1);
    // ASSERT_EQ(t.nanosec(), 20000000u);

    std::stringstream st1("1.002");
    st1 >> t;
    ASSERT_EQ(t.seconds(), 1);
    ASSERT_EQ(t.nanosec(), 2u);

    std::stringstream st2("0.000003");
    st2 >> t;
    ASSERT_EQ(t.seconds(), 0);
    ASSERT_EQ(t.nanosec(), 3u);

    // Non nanosecs
    std::stringstream st3("12");
    st3 >> t;
    ASSERT_EQ(t.seconds(), 12);
    ASSERT_EQ(t.nanosec(), 0u);

    // Lowest time
    std::stringstream st4("0.000000001");
    st4 >> t;
    ASSERT_EQ(t.seconds(), 0);
    ASSERT_EQ(t.nanosec(), 1u);

    // Check double precision
    std::stringstream st5("4346.415672901");
    st5 >> t;
    ASSERT_EQ(t.seconds(), 4346);
    ASSERT_EQ(t.nanosec(), 415672901u);

    std::stringstream st6("123.456789");
    st6 >> t;
    ASSERT_EQ(t.seconds(), 123);
    ASSERT_EQ(t.nanosec(), 456789u);
}

/*
 * Test >> operator with bad format
 */
TEST(TimeTest, bad_format_deserialize_operator)
{
    Time_t t;

    // Check more than 9 digits
    {
        std::stringstream st("1.1231231231");
        st >> t;
        ASSERT_EQ(t.seconds(), 1);
        ASSERT_EQ(t.nanosec(), 231231231u);
    }

    {
        std::stringstream st("2.789789789789");
        st >> t;
        ASSERT_EQ(t.seconds(), 2);
        ASSERT_EQ(t.nanosec(), 294967295u);
    }

    {
        std::stringstream st("1234.999999999999");
        st >> t;
        ASSERT_EQ(t.seconds(), 1234);
        ASSERT_EQ(t.nanosec(), 294967295u);
    }

    {
        std::stringstream st("55.1000000001");
        st >> t;
        ASSERT_EQ(t.seconds(), 55);
        ASSERT_EQ(t.nanosec(), 1u);
    }

    // Check a negative case
    {
        std::stringstream st("-3");
        st >> t;
        ASSERT_EQ(t.seconds(), -3);
        ASSERT_EQ(t.nanosec(), 0u);
    }

    // One second from nanosecond case, set to zero time
    {
        std::stringstream st("0.1000000000");
        st >> t;
        ASSERT_EQ(t.seconds(), 0);
        ASSERT_EQ(t.nanosec(), 0u);
    }

    // Non numeric case
    {
        std::stringstream st("non_number");
        st >> t;
        ASSERT_EQ(t.seconds(), 0);
        ASSERT_EQ(t.nanosec(), 0u);
    }

    // Non numeric case after number
    {
        std::stringstream st("2non_number");
        st >> t;
        ASSERT_EQ(t.seconds(), 2);
        ASSERT_EQ(t.nanosec(), 0u);
    }

    {
        std::stringstream st("9.3non_number4");
        st >> t;
        ASSERT_EQ(t.seconds(), 9);
        ASSERT_EQ(t.nanosec(), 3u);
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
