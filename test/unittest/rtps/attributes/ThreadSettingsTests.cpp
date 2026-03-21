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

#include <fastdds/rtps/attributes/ThreadSettings.hpp>

TEST(ThreadSettingsTests, EqualOperators)
{
    eprosima::fastdds::rtps::ThreadSettings settings_1;
    eprosima::fastdds::rtps::ThreadSettings settings_2;

    ASSERT_TRUE(settings_1 == settings_2);
    ASSERT_FALSE(settings_1 != settings_2);

    // Fixed scheduling_policy cases
    settings_2.scheduling_policy = settings_1.scheduling_policy;
    settings_2.priority = settings_1.priority + 1;
    settings_2.affinity = settings_1.affinity;
    settings_2.stack_size = settings_1.stack_size;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    settings_2.scheduling_policy = settings_1.scheduling_policy;
    settings_2.priority = settings_1.priority;
    settings_2.affinity = settings_1.affinity + 1;
    settings_2.stack_size = settings_1.stack_size;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    settings_2.scheduling_policy = settings_1.scheduling_policy;
    settings_2.priority = settings_1.priority;
    settings_2.affinity = settings_1.affinity;
    settings_2.stack_size = settings_1.stack_size + 1;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    settings_2.scheduling_policy = settings_1.scheduling_policy;
    settings_2.priority = settings_1.priority + 1;
    settings_2.affinity = settings_1.affinity + 1;
    settings_2.stack_size = settings_1.stack_size;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    settings_2.scheduling_policy = settings_1.scheduling_policy;
    settings_2.priority = settings_1.priority;
    settings_2.affinity = settings_1.affinity + 1;
    settings_2.stack_size = settings_1.stack_size + 1;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    settings_2.scheduling_policy = settings_1.scheduling_policy;
    settings_2.priority = settings_1.priority + 1;
    settings_2.affinity = settings_1.affinity;
    settings_2.stack_size = settings_1.stack_size + 1;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    settings_2.scheduling_policy = settings_1.scheduling_policy;
    settings_2.priority = settings_1.priority + 1;
    settings_2.affinity = settings_1.affinity + 1;
    settings_2.stack_size = settings_1.stack_size + 1;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    // Fixed priority cases (not already covered)
    settings_2.scheduling_policy = settings_1.scheduling_policy + 1;
    settings_2.priority = settings_1.priority;
    settings_2.affinity = settings_1.affinity;
    settings_2.stack_size = settings_1.stack_size;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    settings_2.scheduling_policy = settings_1.scheduling_policy + 1;
    settings_2.priority = settings_1.priority;
    settings_2.affinity = settings_1.affinity + 1;
    settings_2.stack_size = settings_1.stack_size;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    settings_2.scheduling_policy = settings_1.scheduling_policy + 1;
    settings_2.priority = settings_1.priority;
    settings_2.affinity = settings_1.affinity;
    settings_2.stack_size = settings_1.stack_size + 1;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    settings_2.scheduling_policy = settings_1.scheduling_policy + 1;
    settings_2.priority = settings_1.priority;
    settings_2.affinity = settings_1.affinity + 1;
    settings_2.stack_size = settings_1.stack_size + 1;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    // Fixed affinity cases (not already covered)
    settings_2.scheduling_policy = settings_1.scheduling_policy + 1;
    settings_2.priority = settings_1.priority + 1;
    settings_2.affinity = settings_1.affinity;
    settings_2.stack_size = settings_1.stack_size;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    settings_2.scheduling_policy = settings_1.scheduling_policy + 1;
    settings_2.priority = settings_1.priority + 1;
    settings_2.affinity = settings_1.affinity;
    settings_2.stack_size = settings_1.stack_size + 1;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    // Fixed stack_size cases (not already covered)
    settings_2.scheduling_policy = settings_1.scheduling_policy + 1;
    settings_2.priority = settings_1.priority + 1;
    settings_2.affinity = settings_1.affinity + 1;
    settings_2.stack_size = settings_1.stack_size;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    settings_2.scheduling_policy = settings_1.scheduling_policy + 1;
    settings_2.priority = settings_1.priority + 1;
    settings_2.affinity = settings_1.affinity;
    settings_2.stack_size = settings_1.stack_size + 1;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);

    // All different
    settings_2.scheduling_policy = settings_1.scheduling_policy + 1;
    settings_2.priority = settings_1.priority + 1;
    settings_2.affinity = settings_1.affinity + 1;
    settings_2.stack_size = settings_1.stack_size + 1;
    ASSERT_FALSE(settings_1 == settings_2);
    ASSERT_TRUE(settings_1 != settings_2);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
