// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/rtps/writer/LivelinessManager.h>
#include <asio.hpp>
#include <thread>
#include <gtest/gtest.h>

class TimedEventEnvironment : public ::testing::Environment
{
    public:

        TimedEventEnvironment() : work_(service_) {}

        void SetUp()
        {
            thread_ = new std::thread(&TimedEventEnvironment::run, this);

            writer_losing_liveliness = eprosima::fastrtps::rtps::GUID_t();
            writer_recovering_liveliness = eprosima::fastrtps::rtps::GUID_t();
            num_writers_lost = 0;
            num_writers_recovered = 0;
        }

        void TearDown()
        {
            service_.stop();
            thread_->join();
            delete thread_;
        }

        void run()
        {
            service_.run();
        }

        asio::io_service service_;
        asio::io_service::work work_;
        std::thread *thread_;

        // Callbacks to test the liveliness manager

        void liveliness_lost(eprosima::fastrtps::rtps::GUID_t guid)
        {
            writer_losing_liveliness = guid;
            num_writers_lost++;
        }

        void liveliness_recovered(eprosima::fastrtps::rtps::GUID_t guid)
        {
            writer_recovering_liveliness = guid;
            num_writers_recovered++;
        }

        eprosima::fastrtps::rtps::GUID_t writer_losing_liveliness;
        eprosima::fastrtps::rtps::GUID_t writer_recovering_liveliness;
        unsigned int num_writers_lost;
        unsigned int num_writers_recovered;
};

TimedEventEnvironment* const env = dynamic_cast<TimedEventEnvironment*>(testing::AddGlobalTestEnvironment(new TimedEventEnvironment));

namespace eprosima {
namespace fastrtps {
namespace rtps {

TEST(LivelinessManagerTests, WriterCannotBeAddedTwice)
{
    LivelinessManager liveliness_manager(
                nullptr,
                nullptr,
                env->service_,
                *env->thread_);

    GuidPrefix_t guidP;
    guidP.value[0] = 1;
    GUID_t guid(guidP, 0);

    EXPECT_EQ(liveliness_manager.add_writer(guid, AUTOMATIC_LIVELINESS_QOS, Duration_t(1)), true);

    // Different liveliness kind but same guid cannot be added
    EXPECT_EQ(liveliness_manager.add_writer(guid, AUTOMATIC_LIVELINESS_QOS, Duration_t(1)), false);
    EXPECT_EQ(liveliness_manager.add_writer(guid, MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, Duration_t(1)), false);
    EXPECT_EQ(liveliness_manager.add_writer(guid, MANUAL_BY_TOPIC_LIVELINESS_QOS, Duration_t(1)), false);

    // Different lease duration but same guid cannot be added
    EXPECT_EQ(liveliness_manager.add_writer(guid, AUTOMATIC_LIVELINESS_QOS, Duration_t(2)), false);
    EXPECT_EQ(liveliness_manager.add_writer(guid, AUTOMATIC_LIVELINESS_QOS, Duration_t(3)), false);
    EXPECT_EQ(liveliness_manager.add_writer(guid, AUTOMATIC_LIVELINESS_QOS, Duration_t(4)), false);
}

TEST(LivelinessManagerTests, WriterCannotBeRemovedTwice)
{
    LivelinessManager liveliness_manager(
                nullptr,
                nullptr,
                env->service_,
                *env->thread_);

    GuidPrefix_t guidP;
    guidP.value[0] = 1;
    GUID_t guid(guidP, 0);

    EXPECT_EQ(liveliness_manager.add_writer(guid, AUTOMATIC_LIVELINESS_QOS, Duration_t(1)), true);
    EXPECT_EQ(liveliness_manager.remove_writer(guid), true);
    EXPECT_EQ(liveliness_manager.remove_writer(guid), false);

    EXPECT_EQ(liveliness_manager.add_writer(guid, MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, Duration_t(1)), true);
    EXPECT_EQ(liveliness_manager.remove_writer(guid), true);
    EXPECT_EQ(liveliness_manager.remove_writer(guid), false);

    EXPECT_EQ(liveliness_manager.add_writer(guid, MANUAL_BY_TOPIC_LIVELINESS_QOS, Duration_t(1)), true);
    EXPECT_EQ(liveliness_manager.remove_writer(guid), true);
    EXPECT_EQ(liveliness_manager.remove_writer(guid), false);
}

//! Tests that the assert_liveliness() method that takes liveliness kind as argument sets the alive state and time
//! correctly
TEST(LivelinessManagerTests, AssertLivelinessByKind)
{
    LivelinessManager liveliness_manager(
                nullptr,
                nullptr,
                env->service_,
                *env->thread_);

    GuidPrefix_t guidP;
    guidP.value[0] = 1;

    liveliness_manager.add_writer(GUID_t(guidP, 1), AUTOMATIC_LIVELINESS_QOS, Duration_t(10));
    liveliness_manager.add_writer(GUID_t(guidP, 2), AUTOMATIC_LIVELINESS_QOS, Duration_t(10));
    liveliness_manager.add_writer(GUID_t(guidP, 3), MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, Duration_t(10));
    liveliness_manager.add_writer(GUID_t(guidP, 4), MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, Duration_t(10));
    liveliness_manager.add_writer(GUID_t(guidP, 5), MANUAL_BY_TOPIC_LIVELINESS_QOS, Duration_t(10));
    liveliness_manager.add_writer(GUID_t(guidP, 6), MANUAL_BY_TOPIC_LIVELINESS_QOS, Duration_t(10));

    // Assert liveliness of automatic writers (the rest should be unchanged)
    EXPECT_TRUE(liveliness_manager.assert_liveliness(AUTOMATIC_LIVELINESS_QOS));
    auto liveliness_data = liveliness_manager.get_liveliness_data();
    EXPECT_EQ(liveliness_data[0].alive, true);
    EXPECT_EQ(liveliness_data[1].alive, true);
    EXPECT_EQ(liveliness_data[2].alive, false);
    EXPECT_EQ(liveliness_data[3].alive, false);
    EXPECT_EQ(liveliness_data[4].alive, false);
    EXPECT_EQ(liveliness_data[5].alive, false);

    // Assert liveliness of manual by participant writers
    EXPECT_TRUE(liveliness_manager.assert_liveliness(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS));
    liveliness_data = liveliness_manager.get_liveliness_data();
    EXPECT_EQ(liveliness_data[0].alive, true);
    EXPECT_EQ(liveliness_data[1].alive, true);
    EXPECT_EQ(liveliness_data[2].alive, true);
    EXPECT_EQ(liveliness_data[3].alive, true);
    EXPECT_EQ(liveliness_data[4].alive, false);
    EXPECT_EQ(liveliness_data[5].alive, false);

    // Assert liveliness of manual by topic writers
    EXPECT_TRUE(liveliness_manager.assert_liveliness(MANUAL_BY_TOPIC_LIVELINESS_QOS));
    liveliness_data = liveliness_manager.get_liveliness_data();
    EXPECT_EQ(liveliness_data[0].alive, true);
    EXPECT_EQ(liveliness_data[1].alive, true);
    EXPECT_EQ(liveliness_data[2].alive, true);
    EXPECT_EQ(liveliness_data[3].alive, true);
    EXPECT_EQ(liveliness_data[4].alive, true);
    EXPECT_EQ(liveliness_data[5].alive, true);

    // Finally check that times were also updated
    EXPECT_GT(liveliness_data[0].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[1].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[2].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[3].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[4].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[5].time, std::chrono::steady_clock::now());
}

//! Tests that the assert_liveliness() method that takes a guid as an argument sets the alive state and time correctly
TEST(LivelinessManagerTests, AssertLivelinessByGuid)
{
    LivelinessManager liveliness_manager(
                nullptr,
                nullptr,
                env->service_,
                *env->thread_);

    GuidPrefix_t guidP;
    guidP.value[0] = 1;

    liveliness_manager.add_writer(GUID_t(guidP, 1), AUTOMATIC_LIVELINESS_QOS, Duration_t(1));
    liveliness_manager.add_writer(GUID_t(guidP, 2), AUTOMATIC_LIVELINESS_QOS, Duration_t(1));
    liveliness_manager.add_writer(GUID_t(guidP, 3), MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, Duration_t(1));
    liveliness_manager.add_writer(GUID_t(guidP, 4), MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, Duration_t(1));
    liveliness_manager.add_writer(GUID_t(guidP, 5), MANUAL_BY_TOPIC_LIVELINESS_QOS, Duration_t(1));
    liveliness_manager.add_writer(GUID_t(guidP, 6), MANUAL_BY_TOPIC_LIVELINESS_QOS, Duration_t(1));

    // If a manual by topic writer is asserted the other writers are unchanged
    EXPECT_TRUE(liveliness_manager.assert_liveliness(GUID_t(guidP, 6)));
    auto liveliness_data = liveliness_manager.get_liveliness_data();
    EXPECT_EQ(liveliness_data[0].alive, false);
    EXPECT_EQ(liveliness_data[1].alive, false);
    EXPECT_EQ(liveliness_data[2].alive, false);
    EXPECT_EQ(liveliness_data[3].alive, false);
    EXPECT_EQ(liveliness_data[4].alive, false);
    EXPECT_EQ(liveliness_data[5].alive, true);

    EXPECT_TRUE(liveliness_manager.assert_liveliness(GUID_t(guidP, 5)));
    liveliness_data = liveliness_manager.get_liveliness_data();
    EXPECT_EQ(liveliness_data[0].alive, false);
    EXPECT_EQ(liveliness_data[1].alive, false);
    EXPECT_EQ(liveliness_data[2].alive, false);
    EXPECT_EQ(liveliness_data[3].alive, false);
    EXPECT_EQ(liveliness_data[4].alive, true);
    EXPECT_EQ(liveliness_data[5].alive, true);

    // If an automatic writer is asserted all automatic writers are asserted as well
    EXPECT_TRUE(liveliness_manager.assert_liveliness(GUID_t(guidP, 1)));
    liveliness_data = liveliness_manager.get_liveliness_data();
    EXPECT_EQ(liveliness_data[0].alive, true);
    EXPECT_EQ(liveliness_data[1].alive, true);
    EXPECT_EQ(liveliness_data[2].alive, false);
    EXPECT_EQ(liveliness_data[3].alive, false);
    EXPECT_EQ(liveliness_data[4].alive, true);
    EXPECT_EQ(liveliness_data[5].alive, true);

    // If a manual by participant writer is asserted all manual by participant writers are asserted as well
    EXPECT_TRUE(liveliness_manager.assert_liveliness(GUID_t(guidP, 4)));
    liveliness_data = liveliness_manager.get_liveliness_data();
    EXPECT_EQ(liveliness_data[0].alive, true);
    EXPECT_EQ(liveliness_data[1].alive, true);
    EXPECT_EQ(liveliness_data[2].alive, true);
    EXPECT_EQ(liveliness_data[3].alive, true);
    EXPECT_EQ(liveliness_data[4].alive, true);
    EXPECT_EQ(liveliness_data[5].alive, true);

    // Finally check that times were also updated
    EXPECT_GT(liveliness_data[0].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[1].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[2].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[3].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[4].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[5].time, std::chrono::steady_clock::now());}

//! Tests the case when the timer expires and liveliness manager is managing two automatic writers with different
//! lease durations
TEST(LivelinessManagerTests, TimerExpired_Automatic)
{
    LivelinessManager liveliness_manager(
                std::bind(&TimedEventEnvironment::liveliness_lost, env, std::placeholders::_1),
                std::bind(&TimedEventEnvironment::liveliness_recovered, env, std::placeholders::_1),
                env->service_,
                *env->thread_);

    GuidPrefix_t guidP;
    guidP.value[0] = 1;

    liveliness_manager.add_writer(GUID_t(guidP, 1), AUTOMATIC_LIVELINESS_QOS, Duration_t(0.1));
    liveliness_manager.add_writer(GUID_t(guidP, 2), AUTOMATIC_LIVELINESS_QOS, Duration_t(0.5));

    // Assert liveliness
    liveliness_manager.assert_liveliness(GUID_t(guidP, 2));
    env->num_writers_recovered = 0u;

    // Wait so that first writer loses liveliness
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(env->writer_losing_liveliness, GUID_t(guidP, 1));

    // Wait a bit longer so that second writer loses liveliness
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_EQ(env->writer_losing_liveliness, GUID_t(guidP, 2));

    // Assert first writer
    liveliness_manager.assert_liveliness(GUID_t(guidP, 1));
    EXPECT_EQ(env->num_writers_recovered, 2u);
}

//! Tests the case when the timer expires and liveliness manager is managing two manual by participant writers
//! with different lease durations
TEST(LivelinessManagerTests, TimerExpired_ManualByParticipant)
{
    LivelinessManager liveliness_manager(
                std::bind(&TimedEventEnvironment::liveliness_lost, env, std::placeholders::_1),
                std::bind(&TimedEventEnvironment::liveliness_recovered, env, std::placeholders::_1),
                env->service_,
                *env->thread_);

    GuidPrefix_t guidP;
    guidP.value[0] = 1;

    liveliness_manager.add_writer(GUID_t(guidP, 1), MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, Duration_t(0.1));
    liveliness_manager.add_writer(GUID_t(guidP, 2), MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, Duration_t(0.5));

    // Assert liveliness
    liveliness_manager.assert_liveliness(GUID_t(guidP, 2));
    env->num_writers_recovered = 0u;

    // Wait so that first writer loses liveliness
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(env->writer_losing_liveliness, GUID_t(guidP, 1));
    EXPECT_EQ(env->num_writers_lost, 1u);

    // Wait a bit longer so that second writer loses liveliness
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_EQ(env->writer_losing_liveliness, GUID_t(guidP, 2));
    EXPECT_EQ(env->num_writers_lost, 2u);

    // Assert first writer
    liveliness_manager.assert_liveliness(GUID_t(guidP, 1));
    EXPECT_EQ(env->num_writers_recovered, 2u);
}

//! Tests the case when the timer expires and liveliness manager is managing two manual by topic writers
//! with different lease durations
TEST(LivelinessManagerTests, TimerExpired_ManualByTopic)
{
    LivelinessManager liveliness_manager(
                std::bind(&TimedEventEnvironment::liveliness_lost, env, std::placeholders::_1),
                std::bind(&TimedEventEnvironment::liveliness_recovered, env, std::placeholders::_1),
                env->service_,
                *env->thread_);

    GuidPrefix_t guidP;
    guidP.value[0] = 1;

    liveliness_manager.add_writer(GUID_t(guidP, 1), MANUAL_BY_TOPIC_LIVELINESS_QOS, Duration_t(0.1));
    liveliness_manager.add_writer(GUID_t(guidP, 2), MANUAL_BY_TOPIC_LIVELINESS_QOS, Duration_t(0.2));

    // Assert first writer
    liveliness_manager.assert_liveliness(GUID_t(guidP, 1));

    // Wait so that first writer loses liveliness
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(env->writer_losing_liveliness, GUID_t(guidP, 1));
    EXPECT_EQ(env->num_writers_lost, 1u);

    // Wait a bit longer and check that the second writer does not recover its liveliness
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(env->writer_losing_liveliness, GUID_t(guidP, 1));
    EXPECT_EQ(env->num_writers_lost, 1u);

    // Assert second writer
    liveliness_manager.assert_liveliness(GUID_t(guidP, 2));
    env->num_writers_lost = 0u;

    // Wait so that it loses liveliness
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    EXPECT_EQ(env->writer_losing_liveliness, GUID_t(guidP, 2));
    EXPECT_EQ(env->num_writers_lost, 1u);
}

//! Tests that the writer that is the current timer owner can be removed, and that the timer is restarted
//! for the next writer
TEST(LivelinessManagerTests, TimerOwnerRemoved)
{
    LivelinessManager liveliness_manager(
                std::bind(&TimedEventEnvironment::liveliness_lost, env, std::placeholders::_1),
                std::bind(&TimedEventEnvironment::liveliness_recovered, env, std::placeholders::_1),
                env->service_,
                *env->thread_);

    GuidPrefix_t guidP;
    guidP.value[0] = 1;

    liveliness_manager.add_writer(GUID_t(guidP, 1), AUTOMATIC_LIVELINESS_QOS, Duration_t(0.5));
    liveliness_manager.add_writer(GUID_t(guidP, 2), AUTOMATIC_LIVELINESS_QOS, Duration_t(1));

    liveliness_manager.assert_liveliness(GUID_t(guidP, 1));
    liveliness_manager.remove_writer(GUID_t(guidP, 1));

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    EXPECT_EQ(env->writer_losing_liveliness, GUID_t(guidP, 2));
    EXPECT_EQ(env->num_writers_lost, 1);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

int main(int argc, char **argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
