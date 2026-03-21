// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <condition_variable>
#include <thread>

#include <asio.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/rtps/common/Time_t.hpp>
#include <rtps/resources/ResourceEvent.h>
#include <rtps/writer/LivelinessManager.hpp>

class LivelinessManagerTests : public ::testing::Test
{
public:

    LivelinessManagerTests()
        : service_()
    {
    }

    void SetUp()
    {
        service_.init_thread();

        writer_losing_liveliness = eprosima::fastdds::rtps::GUID_t();
        writer_recovering_liveliness = eprosima::fastdds::rtps::GUID_t();
        num_writers_lost = 0;
        num_writers_recovered = 0;
    }

    void TearDown()
    {
    }

    void run()
    {
    }

    eprosima::fastdds::rtps::ResourceEvent service_;

    // Callback to test the liveliness manager

    void liveliness_changed(
            eprosima::fastdds::rtps::GUID_t guid,
            const eprosima::fastdds::dds::LivelinessQosPolicyKind&,
            const eprosima::fastdds::dds::Duration_t&,
            int alive_change,
            int not_alive_change)
    {
        if (alive_change > 0)
        {
            std::unique_lock<std::mutex> lock(liveliness_recovered_mutex_);
            writer_recovering_liveliness = guid;
            num_writers_recovered++;
            liveliness_recovered_cv_.notify_one();
        }
        else if (not_alive_change > 0)
        {
            std::unique_lock<std::mutex> lock(liveliness_lost_mutex_);
            writer_losing_liveliness = guid;
            num_writers_lost++;
            liveliness_lost_cv_.notify_one();
        }
    }

    void wait_liveliness_lost(
            unsigned int num_lost)
    {
        std::unique_lock<std::mutex> lock(liveliness_lost_mutex_);
        liveliness_lost_cv_.wait(lock, [&]()
                {
                    return num_writers_lost == num_lost;
                });
    }

    void wait_liveliness_recovered(
            unsigned int num_recovered)
    {
        std::unique_lock<std::mutex> lock(liveliness_recovered_mutex_);
        liveliness_recovered_cv_.wait(lock, [&]()
                {
                    return num_writers_recovered == num_recovered;
                });
    }

    eprosima::fastdds::rtps::GUID_t writer_losing_liveliness;
    eprosima::fastdds::rtps::GUID_t writer_recovering_liveliness;
    unsigned int num_writers_lost;
    unsigned int num_writers_recovered;

    std::mutex liveliness_lost_mutex_;
    std::condition_variable liveliness_lost_cv_;
    std::mutex liveliness_recovered_mutex_;
    std::condition_variable liveliness_recovered_cv_;
};

namespace  eprosima {
namespace fastdds {

using rtps::LivelinessData;
using rtps::LivelinessManager;
using rtps::GuidPrefix_t;
using rtps::GUID_t;

TEST_F(LivelinessManagerTests, WriterCanAlwaysBeAdded)
{
    LivelinessManager liveliness_manager(
        nullptr,
        service_);

    GuidPrefix_t guidP;
    guidP.value[0] = 1;
    GUID_t guid(guidP, 0);

    // Writers with same Guid, liveliness kind and lease duration cannot be added

    EXPECT_EQ(liveliness_manager.add_writer(guid, fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(1)), true);

    // Same guid and different liveliness kind can be added
    EXPECT_EQ(liveliness_manager.add_writer(guid, fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, dds::Duration_t(
                1)), true);
    EXPECT_EQ(liveliness_manager.add_writer(guid, fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS, dds::Duration_t(
                1)), true);

    // Same guid and different lease duration can be added
    EXPECT_EQ(liveliness_manager.add_writer(guid, fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(2)), true);
    EXPECT_EQ(liveliness_manager.add_writer(guid, fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(3)), true);
    EXPECT_EQ(liveliness_manager.add_writer(guid, fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(4)), true);

    // Same guid, same kind, and same lease duration can also be added
    EXPECT_EQ(liveliness_manager.add_writer(guid, fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(1)), true);
}

TEST_F(LivelinessManagerTests, WriterCannotBeRemovedTwice)
{
    LivelinessManager liveliness_manager(
        nullptr,
        service_);

    GuidPrefix_t guidP;
    guidP.value[0] = 1;
    GUID_t guid(guidP, 0);
    LivelinessData::WriterStatus writer_status;

    EXPECT_EQ(liveliness_manager.add_writer(guid, fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(1)), true);
    EXPECT_EQ(liveliness_manager.remove_writer(guid, fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(1),
            writer_status), true);
    EXPECT_EQ(liveliness_manager.remove_writer(guid, fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(1),
            writer_status), false);

    EXPECT_EQ(liveliness_manager.add_writer(guid, fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, dds::Duration_t(
                1)), true);
    EXPECT_EQ(liveliness_manager.remove_writer(guid, fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
            dds::Duration_t(
                1), writer_status), true);
    EXPECT_EQ(liveliness_manager.remove_writer(guid, fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
            dds::Duration_t(
                1), writer_status), false);

    EXPECT_EQ(liveliness_manager.add_writer(guid, fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS, dds::Duration_t(
                1)), true);
    EXPECT_EQ(liveliness_manager.remove_writer(guid, fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS, dds::Duration_t(1),
            writer_status),
            true);
    EXPECT_EQ(liveliness_manager.remove_writer(guid, fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS, dds::Duration_t(1),
            writer_status),
            false);
}

//! Tests that the assert_liveliness() method that takes liveliness kind as argument sets the alive state and time
//! correctly
TEST_F(LivelinessManagerTests, AssertLivelinessByKind)
{
    LivelinessManager liveliness_manager(
        nullptr,
        service_);

    GuidPrefix_t guidP;
    guidP.value[0] = 1;

    liveliness_manager.add_writer(GUID_t(guidP, 1), fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(10));
    liveliness_manager.add_writer(GUID_t(guidP, 2), fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(10));
    liveliness_manager.add_writer(GUID_t(guidP, 3), fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
            dds::Duration_t(10));
    liveliness_manager.add_writer(GUID_t(guidP, 4), fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
            dds::Duration_t(10));
    liveliness_manager.add_writer(GUID_t(guidP, 5), fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS, dds::Duration_t(10));
    liveliness_manager.add_writer(GUID_t(guidP, 6), fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS, dds::Duration_t(10));

    // Assert liveliness of automatic writers (the rest should be unchanged)
    EXPECT_TRUE(liveliness_manager.assert_liveliness(fastdds::dds::AUTOMATIC_LIVELINESS_QOS, guidP));
    auto liveliness_data = liveliness_manager.get_liveliness_data();
    EXPECT_EQ(liveliness_data[0].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[1].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[2].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[3].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[4].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[5].status, LivelinessData::WriterStatus::NOT_ASSERTED);

    // Assert liveliness of manual by participant writers
    EXPECT_TRUE(liveliness_manager.assert_liveliness(fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, guidP));
    liveliness_data = liveliness_manager.get_liveliness_data();
    EXPECT_EQ(liveliness_data[0].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[1].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[2].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[3].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[4].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[5].status, LivelinessData::WriterStatus::NOT_ASSERTED);

    // Assert liveliness of manual by topic writers
    EXPECT_TRUE(liveliness_manager.assert_liveliness(fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS, guidP));
    liveliness_data = liveliness_manager.get_liveliness_data();
    EXPECT_EQ(liveliness_data[0].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[1].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[2].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[3].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[4].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[5].status, LivelinessData::WriterStatus::ALIVE);

    // Finally check that times were also updated
    EXPECT_GT(liveliness_data[0].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[1].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[2].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[3].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[4].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[5].time, std::chrono::steady_clock::now());
}

//! Tests that the assert_liveliness() method that takes a writer as an argument sets the alive state and time correctly
TEST_F(LivelinessManagerTests, AssertLivelinessByWriter)
{
    LivelinessManager liveliness_manager(
        nullptr,
        service_);

    GuidPrefix_t guidP;
    guidP.value[0] = 1;

    liveliness_manager.add_writer(GUID_t(guidP, 1), fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(1));
    liveliness_manager.add_writer(GUID_t(guidP, 2), fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(1));
    liveliness_manager.add_writer(GUID_t(guidP, 3), fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
            dds::Duration_t(1));
    liveliness_manager.add_writer(GUID_t(guidP, 4), fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
            dds::Duration_t(1));
    liveliness_manager.add_writer(GUID_t(guidP, 5), fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS, dds::Duration_t(1));
    liveliness_manager.add_writer(GUID_t(guidP, 6), fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS, dds::Duration_t(1));

    // If a manual by topic writer is asserted the other writers are unchanged
    EXPECT_TRUE(liveliness_manager.assert_liveliness(
                GUID_t(guidP, 6),
                fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS,
                dds::Duration_t(1)));
    auto liveliness_data = liveliness_manager.get_liveliness_data();
    EXPECT_EQ(liveliness_data[0].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[1].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[2].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[3].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[4].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[5].status, LivelinessData::WriterStatus::ALIVE);

    EXPECT_TRUE(liveliness_manager.assert_liveliness(
                GUID_t(guidP, 5),
                fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS,
                dds::Duration_t(1)));
    liveliness_data = liveliness_manager.get_liveliness_data();
    EXPECT_EQ(liveliness_data[0].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[1].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[2].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[3].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[4].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[5].status, LivelinessData::WriterStatus::ALIVE);

    // If an automatic writer is asserted all automatic writers are asserted as well
    EXPECT_TRUE(liveliness_manager.assert_liveliness(
                GUID_t(guidP, 1),
                fastdds::dds::AUTOMATIC_LIVELINESS_QOS,
                dds::Duration_t(1)));
    liveliness_data = liveliness_manager.get_liveliness_data();
    EXPECT_EQ(liveliness_data[0].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[1].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[2].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[3].status, LivelinessData::WriterStatus::NOT_ASSERTED);
    EXPECT_EQ(liveliness_data[4].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[5].status, LivelinessData::WriterStatus::ALIVE);

    // If a manual by participant writer is asserted all manual by participant writers are asserted as well
    EXPECT_TRUE(liveliness_manager.assert_liveliness(
                GUID_t(guidP, 4),
                fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
                dds::Duration_t(1)));
    liveliness_data = liveliness_manager.get_liveliness_data();
    EXPECT_EQ(liveliness_data[0].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[1].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[2].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[3].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[4].status, LivelinessData::WriterStatus::ALIVE);
    EXPECT_EQ(liveliness_data[5].status, LivelinessData::WriterStatus::ALIVE);

    // Finally check that times were also updated
    EXPECT_GT(liveliness_data[0].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[1].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[2].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[3].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[4].time, std::chrono::steady_clock::now());
    EXPECT_GT(liveliness_data[5].time, std::chrono::steady_clock::now());
}

//! Tests the case when the timer expires and liveliness manager is managing two automatic writers with different
//! lease durations
TEST_F(LivelinessManagerTests, TimerExpired_Automatic)
{
    LivelinessManager liveliness_manager(
        std::bind(&LivelinessManagerTests::liveliness_changed,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4,
        std::placeholders::_5),
        service_);

    GuidPrefix_t guidP;
    guidP.value[0] = 1;

    liveliness_manager.add_writer(GUID_t(guidP, 1), fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(0.1));
    liveliness_manager.add_writer(GUID_t(guidP, 2), fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(0.5));

    // Assert liveliness
    liveliness_manager.assert_liveliness(GUID_t(guidP, 2), fastdds::dds::AUTOMATIC_LIVELINESS_QOS,
            dds::Duration_t(0.5));
    num_writers_recovered = 0u;

    // Wait so that first writer loses liveliness
    wait_liveliness_lost(1u);
    EXPECT_EQ(writer_losing_liveliness, GUID_t(guidP, 1));

    // Wait a bit longer so that second writer loses liveliness
    wait_liveliness_lost(2u);
    EXPECT_EQ(writer_losing_liveliness, GUID_t(guidP, 2));

    // Assert first writer
    liveliness_manager.assert_liveliness(GUID_t(guidP, 1), fastdds::dds::AUTOMATIC_LIVELINESS_QOS,
            dds::Duration_t(0.1));
    wait_liveliness_recovered(2u);
    EXPECT_EQ(num_writers_recovered, 2u);
}

//! Tests the case when the timer expires and liveliness manager is managing two manual by participant writers
//! with different lease durations
TEST_F(LivelinessManagerTests, TimerExpired_ManualByParticipant)
{
    LivelinessManager liveliness_manager(
        std::bind(&LivelinessManagerTests::liveliness_changed,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4,
        std::placeholders::_5),
        service_);


    GuidPrefix_t guidP;
    guidP.value[0] = 1;

    liveliness_manager.add_writer(GUID_t(guidP, 1), fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
            dds::Duration_t(0.1));
    liveliness_manager.add_writer(GUID_t(guidP, 2), fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
            dds::Duration_t(0.5));

    // Assert liveliness
    liveliness_manager.assert_liveliness(GUID_t(guidP,
            2), fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
            dds::Duration_t(0.5));
    num_writers_recovered = 0u;

    // Wait so that first writer loses liveliness
    wait_liveliness_lost(1u);
    EXPECT_EQ(writer_losing_liveliness, GUID_t(guidP, 1));
    EXPECT_EQ(num_writers_lost, 1u);

    // Wait a bit longer so that second writer loses liveliness
    wait_liveliness_lost(2u);
    EXPECT_EQ(writer_losing_liveliness, GUID_t(guidP, 2));
    EXPECT_EQ(num_writers_lost, 2u);

    // Assert first writer
    liveliness_manager.assert_liveliness(GUID_t(guidP,
            1), fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
            dds::Duration_t(0.1));
    wait_liveliness_recovered(2u);
    EXPECT_EQ(num_writers_recovered, 2u);
}

//! Tests the case when the timer expires and liveliness manager is managing two manual by topic writers
//! with different lease durations
TEST_F(LivelinessManagerTests, TimerExpired_ManualByTopic)
{
    LivelinessManager liveliness_manager(
        std::bind(&LivelinessManagerTests::liveliness_changed,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4,
        std::placeholders::_5),
        service_);


    GuidPrefix_t guidP;
    guidP.value[0] = 1;

    liveliness_manager.add_writer(GUID_t(guidP, 1), fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS, dds::Duration_t(0.1));
    liveliness_manager.add_writer(GUID_t(guidP, 2), fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS, dds::Duration_t(0.2));

    // Assert first writer
    liveliness_manager.assert_liveliness(GUID_t(guidP, 1), fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS,
            dds::Duration_t(0.1));
    wait_liveliness_recovered(1u);

    // Wait so that first writer loses liveliness
    wait_liveliness_lost(1u);
    EXPECT_EQ(writer_losing_liveliness, GUID_t(guidP, 1));
    EXPECT_EQ(num_writers_lost, 1u);

    // Wait a bit longer and check that the second writer does not recover its liveliness
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(writer_losing_liveliness, GUID_t(guidP, 1));
    EXPECT_EQ(num_writers_lost, 1u);

    // Assert second writer
    liveliness_manager.assert_liveliness(GUID_t(guidP, 2), fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS,
            dds::Duration_t(0.2));
    wait_liveliness_recovered(2u);
    num_writers_lost = 0u;

    // Wait so that it loses liveliness
    wait_liveliness_lost(1u);
    EXPECT_EQ(writer_losing_liveliness, GUID_t(guidP, 2));
    EXPECT_EQ(num_writers_lost, 1u);
}

//! Tests that the timer owner is calculated correctly
//! This is tested indirectly by checking which writer lost liveliness last
TEST_F(LivelinessManagerTests, TimerOwnerCalculation)
{
    LivelinessManager liveliness_manager(
        std::bind(&LivelinessManagerTests::liveliness_changed,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4,
        std::placeholders::_5),
        service_);


    GuidPrefix_t guidP;
    guidP.value[0] = 1;

    liveliness_manager.add_writer(GUID_t(guidP, 1), fastdds::dds::AUTOMATIC_LIVELINESS_QOS,
            dds::Duration_t(100 * 1e-3));
    liveliness_manager.add_writer(GUID_t(guidP, 2), fastdds::dds::AUTOMATIC_LIVELINESS_QOS,
            dds::Duration_t(1000 * 1e-3));
    liveliness_manager.add_writer(GUID_t(guidP, 3), fastdds::dds::AUTOMATIC_LIVELINESS_QOS,
            dds::Duration_t(500 * 1e-3));

    liveliness_manager.assert_liveliness(fastdds::dds::AUTOMATIC_LIVELINESS_QOS, guidP);

    wait_liveliness_lost(1u);
    EXPECT_EQ(writer_losing_liveliness, GUID_t(guidP, 1));
    EXPECT_EQ(num_writers_lost, 1u);

    wait_liveliness_lost(2u);
    EXPECT_EQ(writer_losing_liveliness, GUID_t(guidP, 3));
    EXPECT_EQ(num_writers_lost, 2u);

    wait_liveliness_lost(3u);
    EXPECT_EQ(writer_losing_liveliness, GUID_t(guidP, 2));
    EXPECT_EQ(num_writers_lost, 3u);
}

//! Tests that the writer that is the current timer owner can be removed, and that the timer is restarted
//! for the next writer
TEST_F(LivelinessManagerTests, TimerOwnerRemoved)
{
    LivelinessManager liveliness_manager(
        std::bind(&LivelinessManagerTests::liveliness_changed,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4,
        std::placeholders::_5),
        service_);


    GuidPrefix_t guidP;
    guidP.value[0] = 1;
    LivelinessData::WriterStatus writer_status;

    liveliness_manager.add_writer(GUID_t(guidP, 1), fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(0.5));
    liveliness_manager.add_writer(GUID_t(guidP, 2), fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(1));

    liveliness_manager.assert_liveliness(GUID_t(guidP, 1), fastdds::dds::AUTOMATIC_LIVELINESS_QOS,
            dds::Duration_t(0.5));
    liveliness_manager.remove_writer(GUID_t(guidP, 1), fastdds::dds::AUTOMATIC_LIVELINESS_QOS, dds::Duration_t(
                0.5), writer_status);

    wait_liveliness_lost(1u);
    EXPECT_EQ(writer_losing_liveliness, GUID_t(guidP, 2));
    EXPECT_EQ(num_writers_lost, 1u);
}

} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
