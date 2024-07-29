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
#include <thread>

#include <fastdds/dds/log/Log.hpp>
#include <rtps/resources/ResourceEvent.h>

#include <statistics/rtps/monitor-service/Interfaces.hpp>
#include <statistics/rtps/monitor-service/MonitorService.hpp>
#include <statistics/rtps/monitor-service/MonitorServiceListener.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

struct MockStatusQueryable : public IStatusQueryable
{
    MOCK_METHOD2(get_monitoring_status, bool (
                const fastdds::rtps::GUID_t& guid,
                MonitorServiceData & status));
};

struct MockConnectionsQueryable : public IConnectionsQueryable
{
    MOCK_METHOD2(get_entity_connections, bool(
                const fastdds::rtps::GUID_t& guid,
                ConnectionList & conns_list));
};

struct MockProxyQueryable : public IProxyQueryable
{

    MOCK_METHOD1(get_all_local_proxies, bool(
                std::vector<fastdds::rtps::GUID_t>& guids));

    MOCK_METHOD2(get_serialized_proxy, bool(
                const fastdds::rtps::GUID_t& guid,
                fastdds::rtps::CDRMessage_t* msg));
};


class MonitorServiceTests : public ::testing::Test
{

public:

    MonitorServiceTests()
        : listener_(&monitor_srv_)
        , n_local_entities(5)
        , monitor_srv_(
            fastdds::rtps::GUID_t(),
            &mock_proxy_q_,
            &mock_conns_q_,
            mock_status_q_,
            [&](fastdds::rtps::RTPSWriter**,
            fastdds::rtps::WriterAttributes&,
            fastdds::rtps::WriterHistory*,
            fastdds::rtps::WriterListener*,
            const fastdds::rtps::EntityId_t&,
            bool)->bool
            {
                return true;
            },
            [&](
                fastdds::rtps::RTPSWriter*,
                const ::eprosima::fastdds::rtps::TopicDescription&,
                const ::eprosima::fastdds::dds::WriterQos&)->bool
            {
                return true;
            },
            mock_event_resource_)
    {
        monitor_srv_.set_writer(&writer);
        mock_event_resource_.init_thread();
    }

    void SetUp() override
    {
        mock_guids.reserve(n_local_entities);

        ON_CALL(mock_proxy_q_, get_all_local_proxies(::testing::_)).WillByDefault(testing::Invoke(
                    [this](std::vector<fastdds::rtps::GUID_t>& guids)
                    {
                        guids.reserve(n_local_entities);
                        mock_guids.reserve(n_local_entities);

                        for (size_t i = 1; i <= static_cast<size_t>(n_local_entities); i++)
                        {
                            fastdds::rtps::GUID_t guid;
                            guid.entityId.value[3] = (fastdds::rtps::octet)i;
                            guids.push_back(guid);
                            mock_guids.push_back(guid);
                        }
                        return true;
                    }));

        ON_CALL(mock_proxy_q_, get_serialized_proxy(::testing::_, ::testing::_)).WillByDefault(testing::Invoke(
                    [](const fastdds::rtps::GUID_t&,
                    fastdds::rtps::CDRMessage_t*)
                    {
                        return true;
                    }));

        ON_CALL(mock_conns_q_, get_entity_connections(::testing::_, ::testing::_)).WillByDefault(testing::Invoke(
                    [](const fastdds::rtps::GUID_t&,
                    ConnectionList&)
                    {
                        return true;
                    }));
    }

    void TearDown() override
    {

    }

protected:

    void block_until_idle()
    {
        while (monitor_srv_.is_processing())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    testing::NiceMock<MockConnectionsQueryable> mock_conns_q_;
    testing::NiceMock<MockStatusQueryable> mock_status_q_;
    testing::NiceMock<MockProxyQueryable> mock_proxy_q_;
    MonitorServiceListener listener_;
    int n_local_entities;
    std::vector<fastdds::rtps::GUID_t> mock_guids;
    fastdds::rtps::ResourceEvent mock_event_resource_;
    testing::NiceMock<fastdds::rtps::BaseWriter> writer;
    MonitorService monitor_srv_;
};

TEST_F(MonitorServiceTests, enabling_monitor_service_routine)
{
    //! At the startup, the service should collect the already existing
    //! local entities
    EXPECT_CALL(mock_proxy_q_, get_all_local_proxies(::testing::_)).Times(1);
    EXPECT_CALL(mock_proxy_q_, get_serialized_proxy(::testing::_, ::testing::_)).Times(n_local_entities);
    EXPECT_CALL(mock_conns_q_, get_entity_connections(::testing::_, ::testing::_)).Times(n_local_entities);

    //! Enable the service
    ASSERT_FALSE(monitor_srv_.disable_monitor_service());
    ASSERT_TRUE(monitor_srv_.enable_monitor_service());
    ASSERT_TRUE(monitor_srv_.is_enabled());

    //! Verify expectations
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&mock_conns_q_));
    ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&mock_proxy_q_));

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
    EXPECT_CALL(mock_conns_q_, get_entity_connections(::testing::_, ::testing::_)).
            Times(n_local_entities);

    //! Trigger statuses updates for each entity
    for (auto& entity : mock_guids)
    {
        listener_.on_local_entity_change(entity, true);
        listener_.on_local_entity_connections_change(entity);
    }

    //! Verify expectations
    std::this_thread::sleep_for(std::chrono::seconds(5));
    ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&mock_proxy_q_));
    ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&mock_conns_q_));
}

TEST_F(MonitorServiceTests, multiple_dds_status_updates)
{
    //! Enable the service
    ASSERT_FALSE(monitor_srv_.disable_monitor_service());
    ASSERT_TRUE(monitor_srv_.enable_monitor_service());
    ASSERT_TRUE(monitor_srv_.is_enabled());

    ON_CALL(mock_status_q_, get_monitoring_status(::testing::_, ::testing::_)).
            WillByDefault(testing::Return(true));

    //! Expect the getters for each status that is going to be updated
    EXPECT_CALL(mock_status_q_, get_monitoring_status(::testing::_, ::testing::_)).
            Times(n_local_entities * 5);//statuses * n_local_entities

    //! Trigger statuses updates for each entity
    for (auto& entity : mock_guids)
    {
        listener_.on_local_entity_status_change(entity, statistics::StatusKind::INCOMPATIBLE_QOS);
        listener_.on_local_entity_status_change(entity, statistics::StatusKind::LIVELINESS_CHANGED);
        listener_.on_local_entity_status_change(entity, statistics::StatusKind::LIVELINESS_LOST);
        listener_.on_local_entity_status_change(entity, statistics::StatusKind::DEADLINE_MISSED);
        listener_.on_local_entity_status_change(entity, statistics::StatusKind::SAMPLE_LOST);
    }

    //! Verify expectations
    std::this_thread::sleep_for(std::chrono::seconds(5));
    ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&mock_status_q_));
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

    ON_CALL(mock_proxy_q_, get_serialized_proxy(::testing::_, ::testing::_)).
            WillByDefault(testing::Return(true));
    ON_CALL(mock_conns_q_, get_entity_connections(::testing::_, ::testing::_)).
            WillByDefault(testing::Return(true));
    ON_CALL(mock_status_q_, get_monitoring_status(::testing::_, ::testing::_)).
            WillByDefault(testing::Return(true));

    //! Expect the creation 5 ones
    EXPECT_CALL(mock_proxy_q_, get_serialized_proxy(::testing::_, ::testing::_)).
            Times(5);
    EXPECT_CALL(mock_conns_q_, get_entity_connections(::testing::_, ::testing::_)).
            Times(5);

    //! Expect the getters for each status that is going to be updated
    EXPECT_CALL(mock_status_q_, get_monitoring_status(::testing::_, ::testing::_)).
            Times(5 * 5);

    //! Trigger statuses updates for each of the non-existent entity
    for (auto& entity : mock_guids)
    {
        listener_.on_local_entity_connections_change(entity);
        listener_.on_local_entity_status_change(entity, statistics::StatusKind::INCOMPATIBLE_QOS);
        listener_.on_local_entity_status_change(entity, statistics::StatusKind::LIVELINESS_CHANGED);
        listener_.on_local_entity_status_change(entity, statistics::StatusKind::LIVELINESS_LOST);
        listener_.on_local_entity_status_change(entity, statistics::StatusKind::DEADLINE_MISSED);
        listener_.on_local_entity_status_change(entity, statistics::StatusKind::SAMPLE_LOST);
    }

    //! Verify expectations
    block_until_idle();
    ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&mock_status_q_));
    ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&mock_proxy_q_));
    ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&mock_conns_q_));
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
