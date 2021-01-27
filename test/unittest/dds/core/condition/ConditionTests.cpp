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

#include "../../../logging/mock/MockConsumer.h"
#include <fastdds/dds/log/Log.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/dds/core/condition/GuardCondition.hpp>
#include <fastdds/dds/core/condition/StatusCondition.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>
#include <fastdds/rtps/common/Time_t.h>
#include <fastrtps/types/TypesBase.h>

using eprosima::fastrtps::types::ReturnCode_t;

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
    Condition cond;

    ASSERT_FALSE(cond.get_trigger_value());

    HELPER_WaitForEntries(1);
}

TEST_F(ConditionTests, unsupported_wait_set_methods)
{
    WaitSet ws;
    Condition aux_cond;
    ConditionSeq aux_cond_seq;
    eprosima::fastrtps::Duration_t timeout(1, 0u);

    ASSERT_EQ(ws.attach_condition(aux_cond), ReturnCode_t::RETCODE_UNSUPPORTED);
    ASSERT_EQ(ws.detach_condition(aux_cond), ReturnCode_t::RETCODE_UNSUPPORTED);
    ASSERT_EQ(ws.get_conditions(aux_cond_seq), ReturnCode_t::RETCODE_UNSUPPORTED);
    ASSERT_EQ(ws.wait(aux_cond_seq, timeout), ReturnCode_t::RETCODE_UNSUPPORTED);
}

TEST_F(ConditionTests, unsupported_guard_condition_methods)
{
    GuardCondition cond;

    ASSERT_EQ(cond.set_trigger_value(true), ReturnCode_t::RETCODE_UNSUPPORTED);
}

TEST_F(ConditionTests, unsupported_status_condition_methods)
{
    StatusCondition cond;

    ASSERT_EQ(cond.set_enabled_statuses(StatusMask()), ReturnCode_t::RETCODE_UNSUPPORTED);
    ASSERT_EQ(cond.get_enabled_statuses().to_string(), StatusMask().to_string());
    ASSERT_EQ(cond.get_entity(), nullptr);

    HELPER_WaitForEntries(2);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
