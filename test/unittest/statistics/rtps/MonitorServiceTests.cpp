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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/statistics/rtps/monitor_service/Interfaces.hpp>

#include <statistics/rtps/monitor-service/MonitorService.hpp>
#include <statistics/rtps/monitor-service/MonitorServiceListener.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

struct MockStatusQueryable : public IStatusQueryable
{
    MOCK_METHOD2(get_incompatible_qos_status, bool (
                const fastrtps::rtps::GUID_t& guid,
                dds::IncompatibleQosStatus& status));

    MOCK_METHOD2(get_inconsistent_topic_status, bool(
                const fastrtps::rtps::GUID_t& guid,
                dds::InconsistentTopicStatus& status));

    MOCK_METHOD2(get_liveliness_lost_status, bool(
                const fastrtps::rtps::GUID_t& guid,
                dds::LivelinessLostStatus& status));

    MOCK_METHOD2(get_liveliness_changed_status, bool(
                const fastrtps::rtps::GUID_t& guid,
                dds::LivelinessChangedStatus& status));

    MOCK_METHOD2(get_deadline_missed_status, bool(
                const fastrtps::rtps::GUID_t& guid,
                dds::DeadlineMissedStatus& status));

    MOCK_METHOD2(get_sample_lost_status, bool(
                const fastrtps::rtps::GUID_t& guid,
                dds::SampleLostStatus& status));

};

struct MockConnectionsQueryable : public IConnectionsQueryable
{
    MOCK_METHOD1(get_entity_connections, ConnectionList(
                const fastrtps::rtps::GUID_t& guid));
};

struct MockProxyQueryable : public IProxyQueryable
{

    MOCK_METHOD1(get_all_local_proxies, bool(
                std::vector<fastrtps::rtps::GUID_t>& guids));

    MOCK_METHOD2(get_serialized_proxy, bool(
                const fastrtps::rtps::GUID_t& guid,
                fastrtps::rtps::CDRMessage_t* msg));
};


class MonitorServiceTests : public ::testing::Test
{

public:

    MonitorServiceTests()
        : monitor_srv_(
            fastrtps::rtps::GUID_t(),
            &mock_proxy_q_,
            &mock_conns_q_,
            mock_status_q_)
        , listener_(&monitor_srv_)
        , n_local_entities(5)
    {

    }

    void SetUp() override
    {
        mock_guids.reserve(n_local_entities);

        ON_CALL(mock_proxy_q_, get_all_local_proxies(::testing::_)).WillByDefault(testing::Invoke(
                    [this](std::vector<fastrtps::rtps::GUID_t>& guids)
                    {
                        guids.reserve(n_local_entities);
                        mock_guids.reserve(n_local_entities);

                        for (size_t i = 1; i <= n_local_entities; i++)
                        {
                            fastrtps::rtps::GUID_t guid;
                            guid.entityId.value[3] = i;
                            guids.push_back(guid);
                            mock_guids.push_back(guid);
                        }
                        return true;
                    }));
    }

    void TearDown() override
    {

    }

protected:

    MockConnectionsQueryable mock_conns_q_;
    MockStatusQueryable mock_status_q_;
    MockProxyQueryable mock_proxy_q_;

    MonitorService monitor_srv_;
    MonitorServiceListener listener_;
    size_t n_local_entities;
    std::vector<fastrtps::rtps::GUID_t> mock_guids;
};

TEST_F(MonitorServiceTests, enabling_monitor_service_routine)
{
    //! At the startup, the service should collect the already existing
    //! local entities
    EXPECT_CALL(mock_proxy_q_, get_all_local_proxies(::testing::_)).Times(1);
    EXPECT_CALL(mock_proxy_q_, get_serialized_proxy(::testing::_, ::testing::_)).Times(n_local_entities);
    EXPECT_CALL(mock_conns_q_, get_entity_connections(::testing::_)).Times(n_local_entities);

    //! Enable the service
    ASSERT_FALSE(monitor_srv_.disable_monitor_service());
    ASSERT_TRUE(monitor_srv_.enable_monitor_service());
    ASSERT_TRUE(monitor_srv_.is_enabled());

    //! Verify expectations
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ::testing::Mock::VerifyAndClearExpectations(&mock_conns_q_);
    ::testing::Mock::VerifyAndClearExpectations(&mock_proxy_q_);

}

TEST_F(MonitorServiceTests, multiple_proxy_and_connection_updates)
{
    //! Enable the service
    ASSERT_FALSE(monitor_srv_.disable_monitor_service());
    ASSERT_TRUE(monitor_srv_.enable_monitor_service());
    ASSERT_TRUE(monitor_srv_.is_enabled());

    //! Skip initial transient
    std::this_thread::sleep_for(std::chrono::seconds(3));

    //! Expect the getters for each status that is going to be updated
    EXPECT_CALL(mock_proxy_q_, get_serialized_proxy(::testing::_, ::testing::_)).
            Times(n_local_entities);
    EXPECT_CALL(mock_conns_q_, get_entity_connections(::testing::_)).
            Times(n_local_entities);

    //! Trigger statuses updates for each entity
    for (auto& entity : mock_guids)
    {
        listener_.on_local_entity_change(entity, true);
        listener_.on_local_entity_connections_change(entity);
    }

    //! Verify expectations
    std::this_thread::sleep_for(std::chrono::seconds(5));
    ::testing::Mock::VerifyAndClearExpectations(&mock_status_q_);
}

TEST_F(MonitorServiceTests, multiple_dds_status_updates)
{
    //! Enable the service
    ASSERT_FALSE(monitor_srv_.disable_monitor_service());
    ASSERT_TRUE(monitor_srv_.enable_monitor_service());
    ASSERT_TRUE(monitor_srv_.is_enabled());

    //! Expect the getters for each status that is going to be updated
    EXPECT_CALL(mock_status_q_, get_incompatible_qos_status(::testing::_, ::testing::_)).
            Times(n_local_entities);
    EXPECT_CALL(mock_status_q_, get_liveliness_lost_status(::testing::_, ::testing::_)).
            Times(n_local_entities);
    EXPECT_CALL(mock_status_q_, get_liveliness_changed_status(::testing::_, ::testing::_)).
            Times(n_local_entities);
    EXPECT_CALL(mock_status_q_, get_deadline_missed_status(::testing::_, ::testing::_)).
            Times(n_local_entities);
    EXPECT_CALL(mock_status_q_, get_sample_lost_status(::testing::_, ::testing::_)).
            Times(n_local_entities);

    //! Trigger statuses updates for each entity
    for (auto& entity : mock_guids)
    {
        listener_.on_local_entity_status_change(entity, statistics::INCOMPATIBLE_QOS);
        listener_.on_local_entity_status_change(entity, statistics::LIVELINESS_CHANGED);
        listener_.on_local_entity_status_change(entity, statistics::LIVELINESS_LOST);
        listener_.on_local_entity_status_change(entity, statistics::DEADLINE_MISSED);
        listener_.on_local_entity_status_change(entity, statistics::SAMPLE_LOST);
    }

    //! Verify expectations
    std::this_thread::sleep_for(std::chrono::seconds(5));
    ::testing::Mock::VerifyAndClearExpectations(&mock_status_q_);
}

TEST_F(MonitorServiceTests, entity_removal_correctly_performs)
{
    //! Enable the service
    ASSERT_FALSE(monitor_srv_.disable_monitor_service());
    ASSERT_TRUE(monitor_srv_.enable_monitor_service());
    ASSERT_TRUE(monitor_srv_.is_enabled());

    //! Trigger entity deletion
    for (auto& entity : mock_guids)
    {
        listener_.on_local_entity_change(entity, false);
    }

    EXPECT_CALL(mock_proxy_q_, get_serialized_proxy(::testing::_, ::testing::_)).
            Times(0);
    EXPECT_CALL(mock_conns_q_, get_entity_connections(::testing::_)).
            Times(0);

    //! Expect the getters for each status that is going to be updated
    EXPECT_CALL(mock_status_q_, get_incompatible_qos_status(::testing::_, ::testing::_)).
            Times(0);
    EXPECT_CALL(mock_status_q_, get_liveliness_lost_status(::testing::_, ::testing::_)).
            Times(0);
    EXPECT_CALL(mock_status_q_, get_liveliness_changed_status(::testing::_, ::testing::_)).
            Times(0);
    EXPECT_CALL(mock_status_q_, get_deadline_missed_status(::testing::_, ::testing::_)).
            Times(0);
    EXPECT_CALL(mock_status_q_, get_sample_lost_status(::testing::_, ::testing::_)).
            Times(0);

    //! Trigger statuses updates for each of the non-existent entity
    for (auto& entity : mock_guids)
    {
        listener_.on_local_entity_connections_change(entity);
        listener_.on_local_entity_status_change(entity, statistics::INCOMPATIBLE_QOS);
        listener_.on_local_entity_status_change(entity, statistics::LIVELINESS_CHANGED);
        listener_.on_local_entity_status_change(entity, statistics::LIVELINESS_LOST);
        listener_.on_local_entity_status_change(entity, statistics::DEADLINE_MISSED);
        listener_.on_local_entity_status_change(entity, statistics::SAMPLE_LOST);
    }

    //! Verify expectations
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ::testing::Mock::VerifyAndClearExpectations(&mock_status_q_);
}

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Error);

    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

    eprosima::fastdds::dds::Log::Flush();
    return ret;
}
