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

#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/dds/core/condition/GuardCondition.hpp>
#include <fastdds/dds/core/condition/StatusCondition.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/Time_t.hpp>

#include "../../../logging/mock/MockConsumer.h"
#include "mock/MockEntity.hpp"

using namespace eprosima::fastdds::dds;
using namespace std;

class EntityTests : public ::testing::Test
{
public:

    EntityTests()
    {
        Reset();
    }

    ~EntityTests()
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

/* Test the constructor with default values and with user values */
TEST_F(EntityTests, entity_constructor)
{
    // Defaut constructor
    Entity entity;

    ASSERT_EQ(entity.get_status_mask(), StatusMask::all());
    ASSERT_EQ(entity.get_status_changes(), StatusMask::none());

    // Constructor with status_mask paramenter
    StatusMask sm = StatusMask::all();
    sm.data_available();

    Entity entity2(sm);

    ASSERT_EQ(entity.get_status_mask(), sm);
    ASSERT_EQ(entity.get_status_changes(), StatusMask::none());
}

/* Test with is_enabled() method:
 *  1. Entity is created disabled
 *  2. enable() enables entity
 *  3. close() disables entity
 */
TEST_F(EntityTests, entity_enable)
{
    Entity entity;

    ASSERT_FALSE(entity.is_enabled());
    ASSERT_EQ(entity.enable(), RETCODE_OK);
    ASSERT_TRUE(entity.is_enabled());
    entity.close();
    ASSERT_FALSE(entity.is_enabled());
}

/* Test get_instance_handle and set_instance_handle methods
 * set_instance_handle is protected, so a mock is used */
TEST_F(EntityTests, entity_get_instance_handle)
{
    // Default instance handle
    Entity entity;
    ASSERT_EQ(entity.get_instance_handle(), InstanceHandle_t());

    // Set non default instance handle
    MockEntity entity2;
    InstanceHandle_t handle;
    handle.value[0] = 13;
    entity2.mock_set_instance_handle(handle);
    ASSERT_EQ(entity2.get_instance_handle(), handle);
    ASSERT_NE(entity.get_instance_handle(), entity2.get_instance_handle());
}

/* Test entity == operator */
TEST_F(EntityTests, entity_equal_operator)
{
    // Check two default entities are equal
    Entity entity1;
    Entity entity2;

    ASSERT_TRUE(entity1 == entity2);

    // Check default entities with different mask are equals
    StatusMask sm = StatusMask::all();
    sm.data_available();
    Entity entity3(sm);

    ASSERT_TRUE(entity1 == entity3);

    // Check entities with different InstanceHandle are different
    InstanceHandle_t handle;
    handle.value[0] = 13;
    MockEntity entity4;
    entity4.mock_set_instance_handle(handle);

    ASSERT_FALSE(entity1 == entity4);
}

TEST_F(EntityTests, get_statuscondition)
{
    Entity entity;

    StatusCondition& cond = entity.get_statuscondition();
    EXPECT_EQ(&entity, cond.get_entity());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
