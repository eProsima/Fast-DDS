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

#include <algorithm>
#include <forward_list>
#include <thread>

#include <fastdds/dds/subscriber/ReadCondition.hpp>
#include <fastdds/dds/core/StackAllocatedSequence.hpp>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

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

    for (int i = 0; i < 10; ++i )
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

    for (int i = 0; i < 10; ++i )
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

    for (int i = 0; i < 10; ++i )
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

TEST(ReadConditions, wait_on_SampleStateMask)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .init();
    ASSERT_TRUE(reader.isInitialized());
    writer.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .init();
    ASSERT_TRUE(writer.isInitialized());

    DataReader& data_reader = reader.get_native_reader();

    // Condition masks
    ViewStateMask view_states = ANY_VIEW_STATE;
    InstanceStateMask instance_states = ANY_INSTANCE_STATE;

    // Create the read conditions
    ReadCondition* read_cond = data_reader.create_readcondition(READ_SAMPLE_STATE, view_states, instance_states);
    EXPECT_NE(read_cond, nullptr);

    ReadCondition* not_read_cond = data_reader.create_readcondition(NOT_READ_SAMPLE_STATE, view_states, instance_states);
    EXPECT_NE(not_read_cond, nullptr);

    ReadCondition* any_read_cond = data_reader.create_readcondition(ANY_SAMPLE_STATE, view_states, instance_states);
    EXPECT_NE(any_read_cond, nullptr);

    // Create the waitset and associate
    WaitSet ws;
    EXPECT_EQ(ws.attach_condition(*read_cond), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(ws.attach_condition(*not_read_cond), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(ws.attach_condition(*any_read_cond), ReturnCode_t::RETCODE_OK);

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // 1- Check NOT_READ_SAMPLE_STATE
    // Send sample from a background thread
    std::string test_message("Testing sample state");
    std::thread bw([&writer, &test_message]
            {
                HelloWorldPubSubType::type msg;
                msg.index(1);
                msg.message(test_message);

                // Allow main thread entering wait state, before sending
                std::this_thread::sleep_for(std::chrono::seconds(1));
                writer.send_sample(msg);
            });

    ConditionSeq triggered;
    EXPECT_EQ(ws.wait(triggered, 2.0), ReturnCode_t::RETCODE_OK);
    bw.join();

    // Check the data is there
    EXPECT_EQ(reader.get_native_reader().get_unread_count(), 1);

    // Check the conditions triggered where the expected ones
    ASSERT_FALSE(read_cond->get_trigger_value());
    EXPECT_EQ(std::find(triggered.begin(), triggered.end(), read_cond), triggered.end());
    ASSERT_TRUE(not_read_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), not_read_cond), triggered.end());
    ASSERT_TRUE(any_read_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), any_read_cond), triggered.end());

    // 2- Check READ_SAMPLE_STATE
    // Read sample from a background thread
    FASTDDS_SEQUENCE(DataSeq, HelloWorldPubSubType::type);
    DataSeq datas;
    SampleInfoSeq infos;

    bw = std::thread([&]
                    {
                        EXPECT_EQ(reader.get_native_reader().read_w_condition(
                                    datas,
                                    infos,
                                    1,
                                    not_read_cond), ReturnCode_t::RETCODE_OK);
                    });

    triggered.clear();
    bw.join(); // the read must be performed before the wait, otherwise the wait returns at once
               // with the previous result
    EXPECT_EQ(ws.wait(triggered, 2.0), ReturnCode_t::RETCODE_OK);

    // Check data is good
    EXPECT_EQ(datas[0].index(), 1);
    EXPECT_EQ(datas[0].message(), test_message);

    // Check the conditions triggered where the expected ones
    ASSERT_TRUE(read_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), read_cond), triggered.end());
    ASSERT_FALSE(not_read_cond->get_trigger_value());
    EXPECT_EQ(std::find(triggered.begin(), triggered.end(), not_read_cond), triggered.end());
    ASSERT_TRUE(any_read_cond->get_trigger_value());
    EXPECT_NE(std::find(triggered.begin(), triggered.end(), any_read_cond), triggered.end());

    // take the sample to check the API works
    datas.unloan();
    infos.unloan();
    EXPECT_EQ(reader.get_native_reader().take_w_condition(
                datas,
                infos,
                1,
                read_cond), ReturnCode_t::RETCODE_OK);

    // Check data is good
    EXPECT_EQ(datas[0].index(), 1);
    EXPECT_EQ(datas[0].message(), test_message);

    // Detach conditions & destroy
    EXPECT_EQ(ws.detach_condition(*read_cond), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(data_reader.delete_readcondition(read_cond), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(ws.detach_condition(*not_read_cond), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(data_reader.delete_readcondition(not_read_cond), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(ws.detach_condition(*any_read_cond), ReturnCode_t::RETCODE_OK);
    EXPECT_EQ(data_reader.delete_readcondition(any_read_cond), ReturnCode_t::RETCODE_OK);
}
