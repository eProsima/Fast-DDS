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

#include <fastdds/rtps/common/Guid.hpp>
#include <rtps/common/GuidUtils.hpp>

#include <climits>
#include <iostream>

#include <gtest/gtest.h>

using GUID_t = eprosima::fastdds::rtps::GUID_t;
using GuidUtils = eprosima::fastdds::rtps::GuidUtils;

/*!
 * @fn TEST(GuidUtilities, prefix_create)
 * @brief This test checks the GUID prefix generator on the same process.
 */
TEST(GuidUtilities, prefix_create)
{
    GUID_t guid_1;
    GUID_t guid_2;

    // Calling twice with the same participant id should return the same prefix
    GuidUtils::instance().guid_prefix_create(1, guid_1.guidPrefix);
    GuidUtils::instance().guid_prefix_create(1, guid_2.guidPrefix);
    EXPECT_EQ(guid_1.guidPrefix, guid_2.guidPrefix);

    // Calling with a different participant id should return different prefixes but be
    // considered on the same process
    guid_2 = GUID_t::unknown();
    GuidUtils::instance().guid_prefix_create(2, guid_2.guidPrefix);
    EXPECT_NE(guid_1.guidPrefix, guid_2.guidPrefix);
    EXPECT_TRUE(guid_1.is_on_same_process_as(guid_2));
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
