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

#include <gtest/gtest.h>

// Include mocks first
#include <fastdds/core/condition/WaitSetImpl.hpp>

// Include UUT
#include <fastdds/core/condition/ConditionNotifier.hpp>

// Other includes
#include <fastdds/dds/core/condition/Condition.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::detail;
using ::testing::_;

class TestCondition : public Condition
{
};

TEST(ConditionNotifierTests, basic_test)
{
    WaitSetImpl wait_set;
    ConditionNotifier notifier;
    TestCondition condition;

    auto test_steps = [&]()
            {
                // This should not call wake_up, as the wait_set has not been attached to the notifier (ncalls = 0/1)
                notifier.notify();
                notifier.will_be_deleted(condition);

                // Waitset should be called after being attached (ncalls = 1/2)
                notifier.attach_to(&wait_set);
                notifier.notify();
                notifier.will_be_deleted(condition);

                // Attaching nullptr should not fail and the other waitset should be called (ncalls = 2/3)
                notifier.attach_to(nullptr);
                notifier.notify();
                notifier.will_be_deleted(condition);

                // Attaching same waitset should not duplicate calls (ncalls = 3/4)
                notifier.attach_to(&wait_set);
                notifier.notify();
                notifier.will_be_deleted(condition);

                // Detaching nullptr should not fail and the other waitset should still be called (ncalls = 4/5)
                notifier.detach_from(nullptr);
                notifier.notify();
                notifier.will_be_deleted(condition);

                // Waitset should not be called after being detached (ncalls = 4/6)
                notifier.detach_from(&wait_set);
                notifier.notify();
                notifier.will_be_deleted(condition);

                // Waitset is allowed to be removed twice (ncalls = 4/7)
                notifier.detach_from(&wait_set);
                notifier.notify();
                notifier.will_be_deleted(condition);
            };

    EXPECT_CALL(wait_set, wake_up()).Times(4);
    EXPECT_CALL(wait_set, will_be_deleted(_)).Times(4);
    test_steps();
    testing::Mock::VerifyAndClearExpectations(&wait_set);

    WaitSetImpl other_waitset;
    notifier.attach_to(&other_waitset);

    EXPECT_CALL(wait_set, wake_up()).Times(4);
    EXPECT_CALL(wait_set, will_be_deleted(_)).Times(4);
    EXPECT_CALL(other_waitset, wake_up()).Times(7);
    EXPECT_CALL(other_waitset, will_be_deleted(_)).Times(7);
    test_steps();
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
