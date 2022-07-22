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

#include <forward_list>

#include <fastdds/dds/subscriber/ReadCondition.hpp>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"

#include <gtest/gtest.h>

using namespace eprosima::fastdds::dds;
using eprosima::fastrtps::types::ReturnCode_t;

TEST(ReadConditions, read_conditions_management)
{
    PubSubReader<HelloWorldPubSubType> manager(TEST_TOPIC_NAME);
    manager.init();
    ASSERT_TRUE(manager.isInitialized());
    DataReader& reader = manager.get_native_reader();

    // Condition masks
    SampleStateMask sample_states = 0;
    ViewStateMask view_states = 0;
    InstanceStateMask instance_states = 0;

    // Create and destroy testing

    // 1- cannot create a ReadConditon that cannot be triggered
    ReadCondition* cond = reader.create_readcondition( sample_states, view_states, instance_states);
    EXPECT_EQ(cond, nullptr);

    // 2- create a ReadCondition and destroy it
    sample_states = ANY_SAMPLE_STATE;
    cond = reader.create_readcondition( sample_states, view_states, instance_states);
    EXPECT_NE(cond, nullptr);
    ReturnCode_t res = reader.delete_readcondition(cond);
    EXPECT_EQ(res, ReturnCode_t::RETCODE_OK);

    // 3- Create several ReadConditions associated to the same masks (share implementation)
    std::forward_list<ReadCondition*> conds;

    for (int i = 0; i < 10 ; ++i )
    {
        conds.push_front(reader.create_readcondition( sample_states, view_states, instance_states));
    }

    for (ReadCondition* c : conds)
    {
        EXPECT_EQ(reader.delete_readcondition(c), ReturnCode_t::RETCODE_OK);
    }
    conds.clear();

    // 4- Create several ReadConditions associated to different same masks
    sample_states = 0;
    view_states = 0;
    instance_states = 0;

    for (int i = 0; i < 10 ; ++i )
    {
        conds.push_front(reader.create_readcondition( ++sample_states, ++view_states, ++instance_states));
    }

    for (ReadCondition* c : conds)
    {
        EXPECT_EQ(reader.delete_readcondition(c), ReturnCode_t::RETCODE_OK);
    }
    conds.clear();

    // 5- Create several ReadConditions and destroy them using delete_contained_entities
    sample_states = 0;
    view_states = 0;
    instance_states = 0;

    for (int i = 0; i < 10 ; ++i )
    {
        conds.push_front(reader.create_readcondition( ++sample_states, ++view_states, ++instance_states));
    }

    EXPECT_EQ(reader.delete_contained_entities(), ReturnCode_t::RETCODE_OK);
    conds.clear();

    // 6- Check a DataReader only handles its own ReadConditions
    PubSubReader<HelloWorldPubSubType> another_manager(TEST_TOPIC_NAME);
    another_manager.init();
    ASSERT_TRUE(another_manager.isInitialized());
    DataReader& another_reader = another_manager.get_native_reader();
    cond = another_reader.create_readcondition(sample_states, view_states, instance_states);
    EXPECT_NE(cond, nullptr);
    EXPECT_EQ(reader.delete_readcondition(cond), ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);

    // 7- Check the DataReader cannot be deleted with outstanding conditions
    EXPECT_EQ(another_manager.get_native_subscriber().delete_contained_entities(),
            ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);

    EXPECT_EQ(another_reader.delete_contained_entities(), ReturnCode_t::RETCODE_OK);
}
