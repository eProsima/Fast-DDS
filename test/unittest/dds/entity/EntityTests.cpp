// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/dds/core/Entity.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

TEST(EntityTests, CreateEntityProvidesEmtpyStatusChange)
{
    Entity entity;
    ASSERT_EQ(entity.get_status_changes(), StatusMask::none());
}

TEST(EntityTests, EnableEntity)
{
    Entity entity;
    ASSERT_FALSE(entity.is_enabled());
    entity.enable();
    ASSERT_TRUE(entity.is_enabled());
    entity.close();
    ASSERT_FALSE(entity.is_enabled());
}



} // namespace dds
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
