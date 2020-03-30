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

#include <thread>
#include <condition_variable>

#include <fastdds/dds/core/conditions/WaitSet.hpp>
#include <dds/core/cond/WaitSet.hpp>
#include <fastdds/dds/core/conditions/GuardCondition.hpp>
#include <dds/core/cond/GuardCondition.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class WaitsetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        thread_ = nullptr;
        waiting_ = false;
        timeout_ = fastrtps::Duration_t(10, 0);
    }

    void TearDown() override
    {
        if (thread_->joinable())
        {
            thread_->join();
        }
        delete thread_;
    }

    void run ()
    {
        std::unique_lock<std::mutex> guard(mtx_);
        waiting_ = true;
        guard.unlock();
        cv_.notify_one();

        ws_.wait(active_conditions_, timeout_);

        guard.lock();
        waiting_ = false;
        guard.unlock();
        cv_.notify_one();
    }

    bool start_waitset_wait(std::chrono::milliseconds timeout = std::chrono::seconds(1))
    {
        std::unique_lock<std::mutex> guard(mtx_);
        if (waiting_)
        {
            return true;
        }
        thread_ = new std::thread(&WaitsetTest::run, this);
        return cv_.wait_for(guard, timeout, [&]()
                {
                    return waiting_;
                });
    }

    bool wait_for_waitset_exit(std::chrono::milliseconds timeout = std::chrono::seconds(1))
    {
        std::unique_lock<std::mutex> guard(mtx_);
        if (!waiting_)
        {
            return true;
        }

        return cv_.wait_for(guard, timeout, [&]()
                {
                    return !waiting_;
                });
    }

    bool waiting_;
    ConditionSeq active_conditions_;
    fastrtps::Duration_t timeout_;
    WaitSet ws_;
    std::condition_variable cv_;
    std::mutex mtx_;

private:
    std::thread* thread_;
};



TEST_F(WaitsetTest, AttachingTriggeredConditionExitsWaitStatus)
{
    GuardCondition false_cond;
    ASSERT_EQ(ws_.attach_condition(&false_cond), ReturnCode_t::RETCODE_OK);

    start_waitset_wait();

    GuardCondition true_cond;
    ASSERT_EQ(true_cond.set_trigger_value(true), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(ws_.attach_condition(&true_cond), ReturnCode_t::RETCODE_OK);

    if (!wait_for_waitset_exit())
    {
        ADD_FAILURE() << "Timeout waiting for the waitset to exit from its wait";
    }

    ASSERT_EQ(active_conditions_.size(), 1);
    ASSERT_EQ(active_conditions_[0], &true_cond);

    ASSERT_EQ(ws_.detach_condition(&true_cond), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(ws_.detach_condition(&false_cond), ReturnCode_t::RETCODE_OK);
}

TEST_F(WaitsetTest, StartingWaitWithTriggeredConditionExitsWaitStatus)
{
    GuardCondition false_cond;
    ASSERT_EQ(ws_.attach_condition(&false_cond), ReturnCode_t::RETCODE_OK);

    GuardCondition true_cond;
    ASSERT_EQ(true_cond.set_trigger_value(true), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(ws_.attach_condition(&true_cond), ReturnCode_t::RETCODE_OK);

    start_waitset_wait();
    if (!wait_for_waitset_exit())
    {
        ADD_FAILURE() << "Timeout waiting for the waitset to exit from its wait";
    }

    ASSERT_EQ(active_conditions_.size(), 1);
    ASSERT_EQ(active_conditions_[0], &true_cond);

    ASSERT_EQ(ws_.detach_condition(&true_cond), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(ws_.detach_condition(&false_cond), ReturnCode_t::RETCODE_OK);
}

TEST_F(WaitsetTest, WaitEndsAfterTimeout)
{
    GuardCondition false_cond;
    ASSERT_EQ(ws_.attach_condition(&false_cond), ReturnCode_t::RETCODE_OK);
    timeout_ = fastrtps::Duration_t(3, 0);

    start_waitset_wait();
    if (!wait_for_waitset_exit(std::chrono::seconds(10)))
    {
        ADD_FAILURE() << "Waitset ended wait before timeout";
    }

    ASSERT_TRUE(active_conditions_.empty());

    ASSERT_EQ(ws_.detach_condition(&false_cond), ReturnCode_t::RETCODE_OK);
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
