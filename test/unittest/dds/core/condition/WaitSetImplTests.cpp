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

#include <algorithm>

#include <gtest/gtest.h>

// Include mocks first
#include <fastdds/core/condition/ConditionNotifier.hpp>

// Include UUT
#include <fastdds/core/condition/WaitSetImpl.hpp>

// Other includes
#include <fastdds/dds/core/condition/Condition.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::detail;

TEST(WaitSetImplTests, condition_management)
{
    Condition condition;
    ConditionSeq conditions;
    WaitSetImpl wait_set;

    // WaitSetImpl should be created without conditions
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, wait_set.get_conditions(conditions));
    EXPECT_TRUE(conditions.empty());

    // Trying to detach without having attached
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, wait_set.detach_condition(condition));

    // Adding the same condition several times should always succeed and keep the list with a single condition
    for (int i = 0; i < 2; ++i)
    {
        EXPECT_EQ(ReturnCode_t::RETCODE_OK, wait_set.attach_condition(condition));
        EXPECT_EQ(ReturnCode_t::RETCODE_OK, wait_set.get_conditions(conditions));
        EXPECT_EQ(1u, conditions.size());
        EXPECT_NE(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &condition));
    }

    // Detaching the condition once should succeed
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, wait_set.detach_condition(condition));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, wait_set.get_conditions(conditions));
    EXPECT_TRUE(conditions.empty());

    // Detaching a second time should fail
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, wait_set.detach_condition(condition));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, wait_set.get_conditions(conditions));
    EXPECT_TRUE(conditions.empty());

    // Attach the condition again
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, wait_set.attach_condition(condition));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, wait_set.get_conditions(conditions));
    EXPECT_EQ(1u, conditions.size());
    EXPECT_NE(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &condition));

    // Calling will_be_deleted should detach the condition
    wait_set.will_be_deleted(condition);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, wait_set.get_conditions(conditions));
    EXPECT_TRUE(conditions.empty());
}

TEST(WaitSetImplTests, wait)
{
    class TestCondition : public Condition
    {
    public:

        bool trigger_value = false;

        bool get_trigger_value() const override
        {
            return trigger_value;
        }

    };

    TestCondition condition;
    ConditionSeq conditions;
    WaitSetImpl wait_set;
    const eprosima::fastrtps::Duration_t timeout{ 1, 0 };

    // Waiting on empty wait set should inmediately return OK
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, wait_set.wait(conditions, timeout));
    EXPECT_TRUE(conditions.empty());

    // Attach condition
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, wait_set.attach_condition(condition));

    // Waiting on untriggered condition should timeout
    EXPECT_EQ(ReturnCode_t::RETCODE_TIMEOUT, wait_set.wait(conditions, timeout));
    EXPECT_TRUE(conditions.empty());

    // Waiting on already triggered condition should inmediately return condition
    condition.trigger_value = true;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, wait_set.wait(conditions, timeout));
    EXPECT_EQ(1u, conditions.size());
    EXPECT_NE(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &condition));

    // A wake_up without a trigger should timeout
    condition.trigger_value = false;
    std::thread notify_without_trigger([&]()
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                wait_set.wake_up();
            });
    EXPECT_EQ(ReturnCode_t::RETCODE_TIMEOUT, wait_set.wait(conditions, timeout));
    EXPECT_TRUE(conditions.empty());

    // A wake_up with a trigger should return the condition
    std::thread trigger_and_notify([&]()
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                condition.trigger_value = true;
                wait_set.wake_up();
            });
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, wait_set.wait(conditions, timeout));
    EXPECT_EQ(1u, conditions.size());
    EXPECT_NE(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &condition));

    // Two threads are not allowed to wait at the same time
    std::thread second_wait_thread([&wait_set, &timeout]()
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                ConditionSeq conds;
                EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, wait_set.wait(conds, timeout));
                EXPECT_TRUE(conds.empty());
            });

    condition.trigger_value = false;
    EXPECT_EQ(ReturnCode_t::RETCODE_TIMEOUT, wait_set.wait(conditions, timeout));
    EXPECT_TRUE(conditions.empty());

    // Waiting forever and adding a triggered condition should wake and only return the added condition
    TestCondition triggered_condition;
    triggered_condition.trigger_value = true;

    std::thread add_triggered_condition([&]()
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                wait_set.attach_condition(triggered_condition);
            });

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, wait_set.wait(conditions, eprosima::fastrtps::c_TimeInfinite));
    EXPECT_EQ(1u, conditions.size());
    EXPECT_EQ(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &condition));
    EXPECT_NE(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &triggered_condition));

    notify_without_trigger.join();
    trigger_and_notify.join();
    second_wait_thread.join();
    add_triggered_condition.join();
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
