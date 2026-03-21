// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/rtps/common/VendorId_t.hpp>

using VendorId_t = eprosima::fastdds::rtps::VendorId_t;

TEST(VendorIdTests, ostream_operator)
{
    VendorId_t vendor_id = {0x01, 0x0F};
    std::stringstream ss;
    ss << vendor_id;
    EXPECT_EQ(ss.str(), "0x01 0x0f");
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
