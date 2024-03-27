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

#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>

#include <utils/StringMatching.hpp>

using namespace eprosima::fastdds::rtps;

class StringMatchingTests : public ::testing::Test
{
public:

    StringMatchingTests()
    {
    }

    ~StringMatchingTests()
    {
    }

    char const* path = "foo/bar/baz";
    char const* pattern0 = "foo/bar/baz";
    char const* pattern1 = "foo*";
    char const* pattern2 = "*baz";
    char const* pattern3 = "foo/*/baz";
    char const* pattern4 = "foo/bar/ba?";
    char const* pattern5 = "*ba?*";
    char const* pattern6 = "foo\\bar\\baz";
    char const* pattern7 = "*bar";
    char const* pattern8 = "*";
    char const* pattern9 = "foo/bar/qux";
    char const* pattern10 = "FOO/BAR/QUX";
};



TEST_F(StringMatchingTests, patterns_with_wildcards)
{
    ASSERT_TRUE(StringMatching::matchString(path, pattern0));
    ASSERT_TRUE(StringMatching::matchString(path, pattern1));
    ASSERT_TRUE(StringMatching::matchString(path, pattern2));
    ASSERT_TRUE(StringMatching::matchString(path, pattern3));
    ASSERT_TRUE(StringMatching::matchString(path, pattern4));
    ASSERT_TRUE(StringMatching::matchString(path, pattern5));
    ASSERT_FALSE(StringMatching::matchString(path, pattern6));
    ASSERT_FALSE(StringMatching::matchString(path, pattern7));
    ASSERT_TRUE(StringMatching::matchString(path, pattern8));
    ASSERT_FALSE(StringMatching::matchString(path, pattern9));
}


int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
