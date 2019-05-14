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

#include <fastrtps/utils/fixed_size_string.hpp>
#include <gtest/gtest.h>

using namespace eprosima::fastrtps;

constexpr size_t MAX_CHARS = 255;
constexpr size_t OTHER_MAX_CHARS = 127;

class FixedSizeStringTests: public ::testing::Test
{
    public:
        char const *pattern0 = "foo/bar/baz";
        char const *long_pattern =
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
            "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";

        size_t pattern0_len = strlen(pattern0);
};

TEST_F(FixedSizeStringTests, default_constructor)
{
    std::string std_s;
    fixed_string<MAX_CHARS> fixed_s;

    ASSERT_EQ(fixed_s.size(), 0u);
    ASSERT_EQ(fixed_s, "");
    ASSERT_EQ(fixed_s, std_s);
}

TEST_F(FixedSizeStringTests, construct_with_empty_c_string)
{
    std::string std_s;
    fixed_string<MAX_CHARS> fixed_s("");

    ASSERT_EQ(fixed_s.size(), 0u);
    ASSERT_EQ(fixed_s, "");
    ASSERT_EQ(fixed_s, std_s);
}

TEST_F(FixedSizeStringTests, construct_with_empty_std_string)
{
    std::string std_s;
    fixed_string<MAX_CHARS> fixed_s(std_s);

    ASSERT_EQ(fixed_s.size(), 0u);
    ASSERT_EQ(fixed_s, "");
    ASSERT_EQ(fixed_s, std_s);
}

TEST_F(FixedSizeStringTests, construct_with_c_string)
{
    fixed_string<MAX_CHARS> fixed_s(pattern0);

    ASSERT_EQ(fixed_s.size(), pattern0_len);
    ASSERT_EQ(fixed_s, pattern0);
}

TEST_F(FixedSizeStringTests, construct_with_std_string)
{
    std::string std_string(pattern0);
    fixed_string<MAX_CHARS> fixed_s(std_string);

    ASSERT_EQ(fixed_s.size(), pattern0_len);
    ASSERT_EQ(fixed_s, std_string);
}

TEST_F(FixedSizeStringTests, construct_with_long_c_string)
{
    fixed_string<MAX_CHARS> fixed_s(long_pattern);

    ASSERT_EQ(fixed_s.size(), MAX_CHARS);
    ASSERT_EQ(fixed_s, long_pattern);
}

TEST_F(FixedSizeStringTests, construct_with_long_std_string)
{
    std::string std_string(long_pattern);
    fixed_string<MAX_CHARS> fixed_s(std_string);

    ASSERT_EQ(fixed_s.size(), MAX_CHARS);
    ASSERT_EQ(fixed_s, std_string);
}

TEST_F(FixedSizeStringTests, assign_operators_and_inequality)
{
    fixed_string<MAX_CHARS> fixed_s;

    std::string std_s_empty;
    std::string std_s(pattern0);
    std::string std_s_long(long_pattern);

    fixed_s = std_s_long;
    ASSERT_EQ(fixed_s.size(), MAX_CHARS);
    ASSERT_EQ(fixed_s, long_pattern);
    ASSERT_NE(fixed_s, pattern0);
    ASSERT_NE(fixed_s, "");

    fixed_s = pattern0;
    ASSERT_EQ(fixed_s.size(), pattern0_len);
    ASSERT_EQ(fixed_s, std_s);
    ASSERT_NE(fixed_s, std_s_empty);
    ASSERT_NE(fixed_s, std_s_long);

    fixed_s = std_s_empty;
    ASSERT_EQ(fixed_s.size(), 0u);
    ASSERT_EQ(fixed_s, "");
    ASSERT_NE(fixed_s, long_pattern);
    ASSERT_NE(fixed_s, pattern0);

    fixed_s = long_pattern;
    ASSERT_EQ(fixed_s.size(), MAX_CHARS);
    ASSERT_EQ(fixed_s, std_s_long);
    ASSERT_NE(fixed_s, std_s);
    ASSERT_NE(fixed_s, std_s_empty);

    fixed_s = std_s;
    ASSERT_EQ(fixed_s.size(), pattern0_len);
    ASSERT_EQ(fixed_s, pattern0);
    ASSERT_NE(fixed_s, "");
    ASSERT_NE(fixed_s, long_pattern);

    fixed_s = "";
    ASSERT_EQ(fixed_s.size(), 0u);
    ASSERT_EQ(fixed_s, std_s_empty);
    ASSERT_NE(fixed_s, std_s_long);
    ASSERT_NE(fixed_s, std_s);
}

TEST_F(FixedSizeStringTests, different_fixed_sizes)
{
    fixed_string<MAX_CHARS> s1;
    fixed_string<OTHER_MAX_CHARS> s2;

    ASSERT_EQ(s1, s2);

    s1 = long_pattern;
    ASSERT_NE(s1, s2);

    s2 = s1;
    ASSERT_EQ(s2, s1);

    s2 = pattern0;
    ASSERT_NE(s2, s1);

    s1 = s2;
    ASSERT_EQ(s1, s2);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
