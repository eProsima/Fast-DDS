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
#include <future>
#include <thread>

#include <gtest/gtest.h>

// Include mocks first
#include <fastdds/core/condition/ConditionNotifier.hpp>

// Include UUT
#include <fastdds/core/condition/WaitSetImpl.hpp>

// Other includes
#include <fastdds/dds/core/condition/Condition.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::detail;
using ::testing::_;

class TestCondition : public Condition
{
public:

    volatile bool trigger_value = false;

    bool get_trigger_value() const override
    {
        return trigger_value;
    }

};

TEST(WaitSetImplTests, condition_management)
{
    TestCondition condition;
    ConditionSeq conditions;
    WaitSetImpl wait_set;

    // The condition is attached, detached, attached again and then deleted.
    // The following calls to the notifier are expected
    auto notifier = condition.get_notifier();
    EXPECT_CALL(*notifier, attach_to(_)).Times(2);
    EXPECT_CALL(*notifier, detach_from(_)).Times(1);
    EXPECT_CALL(*notifier, will_be_deleted(_)).Times(1);

    // WaitSetImpl should be created without conditions
    EXPECT_EQ(RETCODE_OK, wait_set.get_conditions(conditions));
    EXPECT_TRUE(conditions.empty());

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

    // Calling will_be_deleted should detach the condition
    wait_set.will_be_deleted(condition);
    EXPECT_EQ(RETCODE_OK, wait_set.get_conditions(conditions));
    EXPECT_TRUE(conditions.empty());
}

TEST(WaitSetImplTests, wait)
{
    const eprosima::fastdds::dds::Duration_t timeout{ 1, 0 };

    TestCondition condition;

    {
        ConditionSeq conditions;
        WaitSetImpl wait_set;

        // Expecting calls on the notifier of triggered_condition
        auto notifier = condition.get_notifier();
        EXPECT_CALL(*notifier, attach_to(_)).Times(1);
        EXPECT_CALL(*notifier, will_be_deleted(_)).Times(1);

        // Waiting on empty wait set should timeout
        EXPECT_EQ(RETCODE_TIMEOUT, wait_set.wait(conditions, timeout));
        EXPECT_TRUE(conditions.empty());

        // Attach condition
        EXPECT_EQ(RETCODE_OK, wait_set.attach_condition(condition));

        // Waiting on untriggered condition should timeout
        EXPECT_EQ(RETCODE_TIMEOUT, wait_set.wait(conditions, timeout));
        EXPECT_TRUE(conditions.empty());

        // Waiting on already triggered condition should inmediately return condition
        condition.trigger_value = true;
        EXPECT_EQ(RETCODE_OK, wait_set.wait(conditions, timeout));
        EXPECT_EQ(1u, conditions.size());
        EXPECT_NE(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &condition));

        // A wake_up without a trigger should timeout
        {
            condition.trigger_value = false;
            std::thread notify_without_trigger([&]()
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        wait_set.wake_up();
                    });
            EXPECT_EQ(RETCODE_TIMEOUT, wait_set.wait(conditions, timeout));
            EXPECT_TRUE(conditions.empty());
            notify_without_trigger.join();
        }

        // A wake_up with a trigger should return the condition
        {
            std::thread trigger_and_notify([&]()
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        condition.trigger_value = true;
                        wait_set.wake_up();
                    });
            EXPECT_EQ(RETCODE_OK, wait_set.wait(conditions, timeout));
            EXPECT_EQ(1u, conditions.size());
            EXPECT_NE(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &condition));
            trigger_and_notify.join();
        }

        // Two threads are not allowed to wait at the same time
        {
            std::thread second_wait_thread([&wait_set, &timeout]()
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        ConditionSeq conds;
                        EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, wait_set.wait(conds, timeout));
                        EXPECT_TRUE(conds.empty());
                    });

            condition.trigger_value = false;
            EXPECT_EQ(RETCODE_TIMEOUT, wait_set.wait(conditions, timeout));
            EXPECT_TRUE(conditions.empty());
            second_wait_thread.join();
        }

        // Waiting forever and adding a triggered condition should wake and only return the added condition
        {
            TestCondition triggered_condition;
            triggered_condition.trigger_value = true;

            // Expecting calls on the notifier of triggered_condition
            notifier = triggered_condition.get_notifier();
            EXPECT_CALL(*notifier, attach_to(_)).Times(1);
            EXPECT_CALL(*notifier, will_be_deleted(_)).Times(1);

            std::thread add_triggered_condition([&]()
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        wait_set.attach_condition(triggered_condition);
                    });

            EXPECT_EQ(RETCODE_OK, wait_set.wait(conditions, eprosima::fastdds::dds::c_TimeInfinite));
            EXPECT_EQ(1u, conditions.size());
            EXPECT_EQ(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &condition));
            EXPECT_NE(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &triggered_condition));
            add_triggered_condition.join();

            wait_set.will_be_deleted(triggered_condition);
        }

        wait_set.will_be_deleted(condition);
    }
}

TEST(WaitSetImplTests, fix_wait_notification_lost)
{
    ConditionSeq conditions;
    WaitSetImpl wait_set;

    // Waiting should return the added connection after the trigger value is updated and the wait_set waken.
    {
        TestCondition triggered_condition;

        // Expecting calls on the notifier of triggered_condition.
        auto notifier = triggered_condition.get_notifier();
        EXPECT_CALL(*notifier, attach_to(_)).Times(1);
        EXPECT_CALL(*notifier, will_be_deleted(_)).Times(1);

        class AnotherTestCondition : public Condition
        {
        public:

            bool get_trigger_value() const override
            {
                // Time to simulate thread context switch or something else
                std::this_thread::sleep_for(std::chrono::seconds(2));
                return false;
            }

        }
        second_simulator_condition;

        // Expecting calls on the notifier of second_simulator_condition.
        notifier = second_simulator_condition.get_notifier();
        EXPECT_CALL(*notifier, attach_to(_)).Times(1);
        EXPECT_CALL(*notifier, will_be_deleted(_)).Times(1);

        wait_set.attach_condition(triggered_condition);
        wait_set.attach_condition(second_simulator_condition);

        std::promise<void> promise;
        std::future<void> future = promise.get_future();
        ReturnCode_t ret = RETCODE_ERROR;
        std::thread wait_conditions([&]()
                {
                    // Not to use `WaitSetImpl::wait` with a timeout value, because the
                    // `condition_variable::wait_for` could call _Predicate function again.
                    ret = wait_set.wait(conditions, eprosima::fastdds::dds::c_TimeInfinite);
                    promise.set_value();
                });

        // One second sleep to make the `wait_set.wait` check `triggered_condition` in the above thread
        std::this_thread::sleep_for(std::chrono::seconds(1));
        triggered_condition.trigger_value = true;
        wait_set.wake_up();

        // Expecting get notification after wake_up, otherwise output error within 5 seconds.
        future.wait_for(std::chrono::seconds(5));
        EXPECT_EQ(RETCODE_OK, ret);
        EXPECT_EQ(1u, conditions.size());
        EXPECT_NE(conditions.cend(), std::find(conditions.cbegin(), conditions.cend(), &triggered_condition));

        // Wake up the `wait_set` to make sure the thread exit
        wait_set.wake_up();
        wait_conditions.join();

        wait_set.will_be_deleted(triggered_condition);
        wait_set.will_be_deleted(second_simulator_condition);
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
