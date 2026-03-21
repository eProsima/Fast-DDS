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

#include <thread>

#include <gtest/gtest.h>

#include <fastdds/core/condition/StatusConditionImpl.hpp>
#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/dds/core/condition/GuardCondition.hpp>
#include <fastdds/dds/core/condition/StatusCondition.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>
#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/Time_t.hpp>

#include "../../../logging/mock/MockConsumer.h"

using namespace eprosima::fastdds::dds;
using namespace std;

class ConditionTests : public ::testing::Test
{
public:

    ConditionTests()
    {
        Reset();
    }

    ~ConditionTests()
    {
        Log::Reset();
        Log::KillThread();
    }

    void Reset()
    {
        Log::ClearConsumers();
        mockConsumer = new MockConsumer();
        Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mockConsumer));
        Log::SetVerbosity(Log::Warning);
    }

    MockConsumer* mockConsumer;

    const uint32_t AsyncTries = 5;
    const uint32_t AsyncWaitMs = 25;

    void HELPER_WaitForEntries(
            uint32_t amount)
    {
        size_t entries = 0;
        for (uint32_t i = 0; i != AsyncTries; i++)
        {
            entries = mockConsumer->ConsumedEntries().size();
            if (entries == amount)
            {
                break;
            }
            this_thread::sleep_for(chrono::milliseconds(AsyncWaitMs));
        }

        ASSERT_EQ(amount, mockConsumer->ConsumedEntries().size());
    }

};

TEST_F(ConditionTests, unsupported_condition_methods)
{
    class TestCondition : public Condition
    {
    }
    cond;

    ASSERT_FALSE(cond.get_trigger_value());

    HELPER_WaitForEntries(1);
}

TEST_F(ConditionTests, waitset_condition_management)
{
    ConditionSeq conditions;
    WaitSet wait_set;

    // WaitSet should be created without conditions
    EXPECT_EQ(RETCODE_OK, wait_set.get_conditions(conditions));
    EXPECT_TRUE(conditions.empty());

    // This scope allows checking the wait_set behavior when the condition is destroyed
    {
        GuardCondition condition;

        // Trying to detach without having attached
        EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, wait_set.detach_condition(condition));

        // Adding the same condition several times should always succeed and keep the list with a single condition
        for (int i = 0; i < 2; ++i)
        {
            EXPECT_EQ(RETCODE_OK, wait_set.attach_condition(condition));
            EXPECT_EQ(RETCODE_OK, wait_set.get_conditions(conditions));
            EXPECT_EQ(1u, conditions.size());
            EXPECT_NE(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &condition));
        }

        // Detaching the condition once should succeed
        EXPECT_EQ(RETCODE_OK, wait_set.detach_condition(condition));
        EXPECT_EQ(RETCODE_OK, wait_set.get_conditions(conditions));
        EXPECT_TRUE(conditions.empty());

        // Detaching a second time should fail
        EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, wait_set.detach_condition(condition));
        EXPECT_EQ(RETCODE_OK, wait_set.get_conditions(conditions));
        EXPECT_TRUE(conditions.empty());

        // Attach the condition again
        EXPECT_EQ(RETCODE_OK, wait_set.attach_condition(condition));
        EXPECT_EQ(RETCODE_OK, wait_set.get_conditions(conditions));
        EXPECT_EQ(1u, conditions.size());
        EXPECT_NE(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &condition));
    }

    // After the condition is destroyed, the wait_set should be empty
    EXPECT_EQ(RETCODE_OK, wait_set.get_conditions(conditions));
    EXPECT_TRUE(conditions.empty());
}

TEST_F(ConditionTests, waitset_wait)
{
    GuardCondition condition;
    ConditionSeq conditions;
    WaitSet wait_set;
    const eprosima::fastdds::dds::Duration_t timeout{ 1, 0 };

    // Waiting on empty wait set should timeout
    EXPECT_EQ(RETCODE_TIMEOUT, wait_set.wait(conditions, timeout));
    EXPECT_TRUE(conditions.empty());

    // Attach condition
    EXPECT_EQ(RETCODE_OK, wait_set.attach_condition(condition));

    // Waiting on untriggered condition should timeout
    EXPECT_EQ(RETCODE_TIMEOUT, wait_set.wait(conditions, timeout));
    EXPECT_TRUE(conditions.empty());

    // Waiting on already triggered condition should inmediately return condition
    EXPECT_EQ(RETCODE_OK, condition.set_trigger_value(true));
    EXPECT_EQ(RETCODE_OK, wait_set.wait(conditions, timeout));
    EXPECT_EQ(1u, conditions.size());
    EXPECT_NE(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &condition));

    // Adding a non-triggered condition while waiting should timeout
    EXPECT_EQ(RETCODE_OK, condition.set_trigger_value(false));
    {
        GuardCondition non_triggered_condition;
        std::thread thr_add_non_triggered([&]()
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    EXPECT_EQ(RETCODE_OK, wait_set.attach_condition(non_triggered_condition));
                });

        EXPECT_EQ(RETCODE_TIMEOUT, wait_set.wait(conditions, timeout));
        EXPECT_TRUE(conditions.empty());
        thr_add_non_triggered.join();
    }

    // Setting the trigger while waiting should return the condition
    {
        std::thread thr_set_trigger([&]()
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    EXPECT_EQ(RETCODE_OK, condition.set_trigger_value(true));
                });

        EXPECT_EQ(RETCODE_OK, wait_set.wait(conditions, timeout));
        EXPECT_EQ(1u, conditions.size());
        EXPECT_NE(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &condition));
        thr_set_trigger.join();
    }

    // Two threads are not allowed to wait at the same time
    EXPECT_EQ(RETCODE_OK, condition.set_trigger_value(false));
    {
        std::thread thr_second_wait([&wait_set, &timeout]()
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    ConditionSeq conds;
                    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, wait_set.wait(conds, timeout));
                    EXPECT_TRUE(conds.empty());
                });

        EXPECT_EQ(RETCODE_TIMEOUT, wait_set.wait(conditions, timeout));
        EXPECT_TRUE(conditions.empty());
        thr_second_wait.join();
    }

    // Waiting forever and adding a triggered condition should wake and only return the added condition
    {
        GuardCondition triggered_condition;
        EXPECT_EQ(RETCODE_OK, triggered_condition.set_trigger_value(true));

        std::thread thr_add_triggered([&]()
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    wait_set.attach_condition(triggered_condition);
                });

        EXPECT_EQ(RETCODE_OK, wait_set.wait(conditions, eprosima::fastdds::dds::c_TimeInfinite));
        EXPECT_EQ(1u, conditions.size());
        EXPECT_EQ(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &condition));
        EXPECT_NE(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &triggered_condition));
        thr_add_triggered.join();
    }
}

TEST_F(ConditionTests, guard_condition_methods)
{
    GuardCondition cond;

    EXPECT_FALSE(cond.get_trigger_value());
    EXPECT_EQ(RETCODE_OK, cond.set_trigger_value(true));
    EXPECT_TRUE(cond.get_trigger_value());
    EXPECT_EQ(RETCODE_OK, cond.set_trigger_value(false));
    EXPECT_FALSE(cond.get_trigger_value());
}

TEST_F(ConditionTests, status_condition_methods)
{
    Entity entity;
    StatusCondition& cond = entity.get_statuscondition();

    EXPECT_EQ(&entity, cond.get_entity());
    EXPECT_FALSE(cond.get_trigger_value());

    StatusMask mask_none = StatusMask::none();
    StatusMask mask_all = StatusMask::all();
    StatusMask mask_single = StatusMask::inconsistent_topic();

    // According to the DDS standard, StatusCondition should start with all statuses enabled
    EXPECT_EQ(mask_all.to_string(), cond.get_enabled_statuses().to_string());
    EXPECT_EQ(RETCODE_OK, cond.set_enabled_statuses(mask_single));
    EXPECT_EQ(mask_single.to_string(), cond.get_enabled_statuses().to_string());
    EXPECT_EQ(RETCODE_OK, cond.set_enabled_statuses(mask_none));
    EXPECT_EQ(mask_none.to_string(), cond.get_enabled_statuses().to_string());
    EXPECT_EQ(RETCODE_OK, cond.set_enabled_statuses(mask_all));
    EXPECT_EQ(mask_all.to_string(), cond.get_enabled_statuses().to_string());
}

TEST_F(ConditionTests, status_condition_trigger)
{
    WaitSet wait_set;
    ConditionSeq conditions;
    const eprosima::fastdds::dds::Duration_t timeout{ 1, 0 };

    Entity entity;
    StatusCondition& cond = entity.get_statuscondition();

    StatusMask mask_all = StatusMask::all();
    StatusMask one_mask = StatusMask::inconsistent_topic();
    StatusMask other_mask = StatusMask::data_on_readers();

    auto wait_for_trigger = [&]()
            {
                EXPECT_EQ(RETCODE_OK, wait_set.wait(conditions, eprosima::fastdds::dds::c_TimeInfinite));
                EXPECT_EQ(1u, conditions.size());
                EXPECT_EQ(&cond, conditions[0]);
                EXPECT_TRUE(cond.get_trigger_value());
            };

    auto expect_no_trigger = [&]()
            {
                EXPECT_FALSE(cond.get_trigger_value());
                EXPECT_EQ(RETCODE_TIMEOUT, wait_set.wait(conditions, timeout));
                EXPECT_TRUE(conditions.empty());
            };

    ASSERT_NE(nullptr, cond.get_impl());

    EXPECT_EQ(RETCODE_OK, wait_set.attach_condition(cond));

    // Condition should be untriggered upon creation
    EXPECT_EQ(mask_all.to_string(), cond.get_enabled_statuses().to_string());
    expect_no_trigger();

    // Triggering other_mask should trigger
    {
        std::thread wait_thr(wait_for_trigger);
        cond.get_impl()->set_status(other_mask, true);
        EXPECT_TRUE(cond.get_trigger_value());
        wait_thr.join();
    }

    // Setting mask to one_mask should untrigger
    EXPECT_EQ(RETCODE_OK, cond.set_enabled_statuses(one_mask));
    EXPECT_EQ(one_mask.to_string(), cond.get_enabled_statuses().to_string());
    expect_no_trigger();

    // Triggering one_mask should trigger
    {
        std::thread wait_thr(wait_for_trigger);
        cond.get_impl()->set_status(one_mask, true);
        EXPECT_TRUE(cond.get_trigger_value());
        wait_thr.join();
    }

    // Triggering twice should not affect trigger
    cond.get_impl()->set_status(one_mask, true);
    wait_for_trigger();

    // Untriggering other_mask should not affect trigger
    cond.get_impl()->set_status(other_mask, false);
    wait_for_trigger();

    // Triggering other_mask should not affect trigger
    cond.get_impl()->set_status(other_mask, true);
    wait_for_trigger();

    // Untriggering one_mask should untrigger
    cond.get_impl()->set_status(one_mask, false);
    expect_no_trigger();

    // Untriggering other_mask should not trigger
    cond.get_impl()->set_status(other_mask, false);
    expect_no_trigger();

    // Triggering other_mask should not trigger
    cond.get_impl()->set_status(other_mask, true);
    expect_no_trigger();

    // Setting mask to other_mask should trigger
    {
        std::thread wait_thr(wait_for_trigger);
        EXPECT_EQ(RETCODE_OK, cond.set_enabled_statuses(other_mask));
        EXPECT_EQ(other_mask.to_string(), cond.get_enabled_statuses().to_string());
        wait_thr.join();
    }

    // Triggering one_mask should not affect trigger
    cond.get_impl()->set_status(one_mask, true);
    wait_for_trigger();

    // Setting mask to one_mask should not affect trigger
    EXPECT_EQ(RETCODE_OK, cond.set_enabled_statuses(one_mask));
    EXPECT_EQ(one_mask.to_string(), cond.get_enabled_statuses().to_string());
    wait_for_trigger();

    // Untriggering other_mask should not affect trigger
    cond.get_impl()->set_status(other_mask, false);
    wait_for_trigger();

    // Setting mask to other_mask should untrigger
    EXPECT_EQ(RETCODE_OK, cond.set_enabled_statuses(other_mask));
    EXPECT_EQ(other_mask.to_string(), cond.get_enabled_statuses().to_string());
    expect_no_trigger();

    // Setting mask to one_mask should trigger
    EXPECT_EQ(RETCODE_OK, cond.set_enabled_statuses(one_mask));
    EXPECT_EQ(one_mask.to_string(), cond.get_enabled_statuses().to_string());
    EXPECT_TRUE(cond.get_trigger_value());
    wait_for_trigger();
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
