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

#include <fastdds/dds/core/conditions/GuardCondition.hpp>
#include <dds/core/cond/GuardCondition.hpp>
#include <fastdds/dds/core/Entity.hpp>
#include <dds/core/Entity.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

TEST(ConditionsTests, GuardConditionActivation)
{
    GuardCondition cond;
    ASSERT_FALSE(cond.get_trigger_value());
    ASSERT_EQ(cond.set_trigger_value(true), ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(cond.get_trigger_value());
    ASSERT_EQ(cond.set_trigger_value(false), ReturnCode_t::RETCODE_OK);
    ASSERT_FALSE(cond.get_trigger_value());
}

TEST(ConditionsTests, PSMGuardConditionActivation)
{
    ::dds::core::cond::GuardCondition cond;
    ASSERT_FALSE(cond.trigger_value());
    ASSERT_NO_THROW(cond.trigger_value(true));
    ASSERT_TRUE(cond.trigger_value());
    ASSERT_NO_THROW(cond.trigger_value(false));
    ASSERT_FALSE(cond.trigger_value());
}

TEST(ConditionsTests, StatusCondition)
{
    Entity entity;
    StatusCondition& cond = entity.get_statuscondition();
    ASSERT_FALSE(cond.get_trigger_value());
    ASSERT_EQ(cond.get_enabled_statuses(), StatusMask::all());

    StatusMask mask = StatusMask::data_available();
    ASSERT_NO_THROW(cond.set_enabled_statuses(mask));

    StatusCondition& cond2 = entity.get_statuscondition();
    ASSERT_EQ(cond2.get_enabled_statuses(), StatusMask::data_available());
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
