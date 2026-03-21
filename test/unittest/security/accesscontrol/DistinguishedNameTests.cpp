// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <list>

#include <security/accesscontrol/DistinguishedName.h>

using namespace eprosima::fastdds::rtps::security;

namespace test {

/*
 * This struct join multiple DistinguishedNames in string format that refer to the same entity.
 * This entity is different for the different sets.
 *
 * Each of the sets is though to check a specific functionality of the compare function.
 */
static const std::vector<std::vector<DistinguishedName>> DIFFERENT_DISTINGUISHED_NAMES =
{
    // ROS2 case (check trim spaces)
    {
        "CN=/pub",
        "CN = /pub",
        "CN= /pub",
        "CN =/pub",
        "  CN  =  /pub  ",
    },

    // Documentation case (check different order)
    {
        "emailAddress=example@eprosima.com, CN=DomainParticipantName, O=eProsima, ST=MA, C=ES",
        "C=ES, ST=MA, O=eProsima, CN=DomainParticipantName, emailAddress=example@eprosima.com",
    },

    // RFC 2253 example (check scaped comma)
    {
        "CN=L. Eagle,O=Sue\\, Grabbit and Runn,C=GB",
        "CN=L. Eagle,O=\"Sue, Grabbit and Runn\",C=GB",
    },

    // Some other values to verify negative compare works
    {
        "CN=/o_pub",
        "CN=\"/o_pub\"",
    },

    {
        "CN=/pub,O=ROS",
        "CN=/pub, O=ROS",
    },
};

} // namespace test

/*
 * Test rfc2253_string_compare function by giving known equal values.
 */
TEST(DistinguishedNameTests, rfc2253_string_compare_positive)
{
    // For each collection of equals
    for (const auto& equal_collection : test::DIFFERENT_DISTINGUISHED_NAMES)
    {
        // For each name
        for (const auto& name_to_compare : equal_collection)
        {
            // For each equal name
            for (const auto& other_equal_name : equal_collection)
            {
                // std::cout << "COMPARING: " << name_to_compare << " and " << other_equal_name << std::endl;
                ASSERT_TRUE(rfc2253_string_compare(name_to_compare, other_equal_name))
                    << name_to_compare << " != " << other_equal_name;

                ASSERT_TRUE(rfc2253_string_compare(other_equal_name, name_to_compare))
                    << " inverse: " << other_equal_name << " != " << name_to_compare;
            }
        }
    }
}

/*
 * Test rfc2253_string_compare function by giving known not equal values.
 */
TEST(DistinguishedNameTests, rfc2253_string_compare_negative)
{
    // For each collection of equals
    for (size_t i = 0; i < test::DIFFERENT_DISTINGUISHED_NAMES.size(); i++)
    {
        auto& collection_to_compare = test::DIFFERENT_DISTINGUISHED_NAMES[i];

        // For each name
        for (const auto& name_to_compare : collection_to_compare)
        {
            // For each different collection of equals
            for (size_t j = 0; j < test::DIFFERENT_DISTINGUISHED_NAMES.size(); j++)
            {
                if (i == j)
                {
                    continue;
                }

                auto& other_collection_to_compare = test::DIFFERENT_DISTINGUISHED_NAMES[j];

                // For each not equal name
                for (const auto& other_name_to_compare : other_collection_to_compare)
                {
                    // std::cout << "COMPARING: " << name_to_compare << " and " << other_name_to_compare << std::endl;
                    ASSERT_FALSE(rfc2253_string_compare(name_to_compare, other_name_to_compare))
                        << name_to_compare << " == " << other_name_to_compare;

                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// details functions

/*
 * Test Attribute operator==
 */
TEST(DistinguishedNameTests, attribute_compare)
{
    // Positive cases
    {
        // Same string
        {
            const char* cc1 = "Hello";
            detail::Attribute att1(cc1);
            detail::Attribute att2(cc1);
            ASSERT_EQ(att1, att2)
                << att1.value << " != " << att2.value;
        }

        // With spaces
        {
            const char* cc1 = " He  llo   ";
            const char* cc2 = " He  llo   ";
            detail::Attribute att1(cc1);
            detail::Attribute att2(cc2);
            ASSERT_EQ(att1, att2)
                << att1.value << " != " << att2.value;
        }

        // With scaped chars
        {
            const char* cc1 = "Hello,";
            const char* cc2 = "Hello\\,";
            detail::Attribute att1(cc1);
            detail::Attribute att2(cc2);
            ASSERT_EQ(att1, att2)
                << att1.value << " != " << att2.value;
        }

        // With quotes chars
        {
            const char* cc1 = "\"Hello\"";
            const char* cc2 = "Hello";
            detail::Attribute att1(cc1);
            detail::Attribute att2(cc2);
            ASSERT_EQ(att1, att2)
                << att1.value << " != " << att2.value;
        }
    }

    // Negative cases
    {
        {
            const char* cc1 = "/pub";
            const char* cc2 = "/sub";
            detail::Attribute att1(cc1);
            detail::Attribute att2(cc2);
            ASSERT_NE(att1, att2)
                << att1.value << " == " << att2.value;
        }

        {
            const char* cc1 = "/pub";
            const char* cc2 = " /pub";
            detail::Attribute att1(cc1);
            detail::Attribute att2(cc2);
            ASSERT_NE(att1, att2)
                << att1.value << " == " << att2.value;
        }

        {
            const char* cc1 = "Hello\\.";
            const char* cc2 = "Hello\\,";
            detail::Attribute att1(cc1);
            detail::Attribute att2(cc2);
            ASSERT_NE(att1, att2)
                << att1.value << " == " << att2.value;
        }
    }
}

/*
 * Test Attribute trim_blank_spaces function
 */
TEST(DistinguishedNameTests, trim_blank_spaces)
{
    // No spaces
    {
        const char* cc1 = "Hello";
        const char* cc2 = "Hello";
        detail::Attribute att1(cc1);
        att1.trim_blank_spaces();
        detail::Attribute att2(cc2);
        ASSERT_EQ(att1, att2)
            << att1.value << " != " << att2.value;
    }

    // One space
    {
        const char* cc1 = " Hello";
        const char* cc2 = "Hello";
        detail::Attribute att1(cc1);
        att1.trim_blank_spaces();
        detail::Attribute att2(cc2);
        ASSERT_EQ(att1, att2)
            << att1.value << " != " << att2.value;
    }

    // Multiple spaces
    {
        const char* cc1 = "   Hello";
        const char* cc2 = "Hello";
        detail::Attribute att1(cc1);
        att1.trim_blank_spaces();
        detail::Attribute att2(cc2);
        ASSERT_EQ(att1, att2)
            << att1.value << " != " << att2.value;
    }
}

/*
 * Test Attribute trim_back_blank_spaces function
 */
TEST(DistinguishedNameTests, trim_back_blank_spaces)
{
    // No spaces
    {
        const char* cc1 = "Hello";
        const char* cc2 = "Hello";
        detail::Attribute att1(cc1);
        att1.trim_back_blank_spaces();
        detail::Attribute att2(cc2);
        ASSERT_EQ(att1, att2)
            << att1.value << " != " << att2.value;
    }

    // One space
    {
        const char* cc1 = "Hello ";
        const char* cc2 = "Hello";
        detail::Attribute att1(cc1);
        att1.trim_back_blank_spaces();
        detail::Attribute att2(cc2);
        ASSERT_EQ(att1, att2)
            << att1.value << " != " << att2.value;
    }

    // Multiple spaces
    {
        const char* cc1 = "Hello   ";
        const char* cc2 = "Hello";
        detail::Attribute att1(cc1);
        att1.trim_back_blank_spaces();
        detail::Attribute att2(cc2);
        ASSERT_EQ(att1, att2)
            << att1.value << " != " << att2.value;
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
