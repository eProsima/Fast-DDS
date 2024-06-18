#include <fastdds/subscriber/history/DataReaderInstance.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

/*!
 * \test DDS-OWN-INST-01 Tests instance's owner changes successfully when the current owner's instance changes its
 * ownership strength.
 */
TEST(DataReaderInstance, writer_update_its_ownership_strength)
{
    // Prepare test input.
    eprosima::fastdds::dds::detail::DataReaderInstance instance({}, {});
    const eprosima::fastdds::rtps::GUID_t dw1_guid({}, 1);
    const eprosima::fastdds::rtps::GUID_t dw2_guid({}, 2);
    const eprosima::fastdds::rtps::GUID_t dw3_guid({}, 3);
    const eprosima::fastdds::rtps::GUID_t another_guid({}, 4);

    instance.alive_writers.push_back({dw1_guid, 1});
    instance.alive_writers.push_back({dw2_guid, 10});
    instance.alive_writers.push_back({dw3_guid, 5});
    instance.current_owner = {dw2_guid, 10};


    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing DW2's strength to 11.
    instance.writer_update_its_ownership_strength(dw2_guid, 11);
    // DW2 changes strength to 11 and still is the owner.
    ASSERT_EQ(dw2_guid, instance.current_owner.first);
    ASSERT_EQ(11u, instance.current_owner.second);

    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing DW1's strength to 4.
    instance.writer_update_its_ownership_strength(dw1_guid, 4);
    // DW1 changes strength to 4 and the owner doesn't changes.
    ASSERT_EQ(dw2_guid, instance.current_owner.first);
    ASSERT_EQ(11u, instance.current_owner.second);

    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing DW1's strength to 12.
    instance.writer_update_its_ownership_strength(dw1_guid, 12);
    // DW1 changes strength to 12 and the owner doesn't changes.
    ASSERT_EQ(dw2_guid, instance.current_owner.first);
    ASSERT_EQ(11u, instance.current_owner.second);

    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing DW2's strength to 13.
    instance.writer_update_its_ownership_strength(dw2_guid, 13);
    // DW2 changes strength to 13 and still is the new owner.
    ASSERT_EQ(dw2_guid, instance.current_owner.first);
    ASSERT_EQ(13u, instance.current_owner.second);

    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing DW3's strength to 6.
    instance.writer_update_its_ownership_strength(dw3_guid, 6);
    // DW3 changes strength to 6 and the owner doesn't changes.
    ASSERT_EQ(dw2_guid, instance.current_owner.first);
    ASSERT_EQ(13u, instance.current_owner.second);

    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing DW2's strength to 4.
    instance.writer_update_its_ownership_strength(dw2_guid, 4);
    // DW2 changes strength to 6 and losses the ownership. DW3 is the new owner.
    ASSERT_EQ(dw3_guid, instance.current_owner.first);
    ASSERT_EQ(5u, instance.current_owner.second);

    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing DW3's strength to 14.
    instance.writer_update_its_ownership_strength(dw3_guid, 14);
    // DW3 changes strength to 14 and doesn't change the owner
    ASSERT_EQ(dw3_guid, instance.current_owner.first);
    ASSERT_EQ(14u, instance.current_owner.second);

    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing no registered DataWriter's strength
    // to 20.
    instance.writer_update_its_ownership_strength(another_guid, 20);
    // Anything changes because it is not an "alive" writer of the instance.
    ASSERT_EQ(dw3_guid, instance.current_owner.first);
    ASSERT_EQ(14u, instance.current_owner.second);
    ASSERT_EQ(3u, instance.alive_writers.size());
}

/*!
 * \test DDS-OWN-INST-02 Tests instance's owner changes successfully when instance is notified with a new "alive"
 * change.
 */
TEST(DataReaderInstance, update_state_alive)
{
    // Prepare test input.
    eprosima::fastdds::dds::detail::DataReaderHistoryCounters counters;
    eprosima::fastdds::dds::detail::DataReaderInstance instance({}, {});
    const eprosima::fastdds::rtps::GUID_t dw1_guid({}, 1);
    const eprosima::fastdds::rtps::GUID_t dw2_guid({}, 2);
    const eprosima::fastdds::rtps::GUID_t dw3_guid({}, 3);
    const eprosima::fastdds::rtps::GUID_t another_guid({}, 4);

    // Calls `DataReaderInstance::update_state()` with a new DW1 writer with strength 2.
    instance.update_state(counters, eprosima::fastdds::rtps::ALIVE, dw1_guid, 2);
    // Instance has a new owner DW1 with strength 2.
    ASSERT_EQ(dw1_guid, instance.current_owner.first);
    ASSERT_EQ(2u, instance.current_owner.second);
    ASSERT_EQ(1u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(1u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(0u, counters.instances_no_writers);

    // Calls `DataReaderInstance::update_state()` with a new DW2 writer with strength 1.
    instance.update_state(counters, eprosima::fastdds::rtps::ALIVE, dw2_guid, 1);
    // Instance has a new "alive" writer DW2 with strength 1.
    ASSERT_EQ(dw1_guid, instance.current_owner.first);
    ASSERT_EQ(2u, instance.current_owner.second);
    ASSERT_EQ(2u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(1u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(0u, counters.instances_no_writers);

    // Calls `DataReaderInstance::update_state()` with a new DW3 writer with strength 6.
    instance.update_state(counters, eprosima::fastdds::rtps::ALIVE, dw3_guid, 6);
    // Instance has a new "alive" writer DW3 with strength 6 and  DW3 is the new owner.
    ASSERT_EQ(dw3_guid, instance.current_owner.first);
    ASSERT_EQ(6u, instance.current_owner.second);
    ASSERT_EQ(3u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(1u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(0u, counters.instances_no_writers);

    // Calls `DataReaderInstance::update_state()` with a new DW2 writer with strength 7.
    instance.update_state(counters, eprosima::fastdds::rtps::ALIVE, dw2_guid, 7);
    // Instance has a new owner DW2 with strength 7.
    ASSERT_EQ(dw2_guid, instance.current_owner.first);
    ASSERT_EQ(7u, instance.current_owner.second);
    ASSERT_EQ(3u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(1u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(0u, counters.instances_no_writers);

    // Calls `DataReaderInstance::update_state()` with a new DW2 writer with strength 2.
    instance.update_state(counters, eprosima::fastdds::rtps::ALIVE, dw2_guid, 2);
    // Instance maintain the owner DW2 with strength 2.
    ASSERT_EQ(dw2_guid, instance.current_owner.first);
    ASSERT_EQ(2u, instance.current_owner.second);
    ASSERT_EQ(3u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(1u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(0u, counters.instances_no_writers);

    // Calls `DataReaderInstance::update_state()` with a new DW1 writer with strength 5.
    instance.update_state(counters, eprosima::fastdds::rtps::ALIVE, dw1_guid, 5);
    // Instance has a new owner DW1 with strength 5.
    ASSERT_EQ(dw1_guid, instance.current_owner.first);
    ASSERT_EQ(5u, instance.current_owner.second);
    ASSERT_EQ(3u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(1u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(0u, counters.instances_no_writers);

    // Calls `DataReaderInstance::update_state()` with a new DW1 writer with strength 10.
    instance.update_state(counters, eprosima::fastdds::rtps::ALIVE, dw1_guid, 10);
    // Instance maintain the owner DW1 with strength 10.
    ASSERT_EQ(dw1_guid, instance.current_owner.first);
    ASSERT_EQ(10u, instance.current_owner.second);
    ASSERT_EQ(3u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(1u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(0u, counters.instances_no_writers);
}

/*!
 * \test DDS-OWN-INST-03 Tests instance's owner changes successfully when writer is unregistered from the instance.
 */
TEST(DataReaderInstance, update_state_unregister)
{
    // Prepare test input.
    eprosima::fastdds::dds::detail::DataReaderHistoryCounters counters;
    eprosima::fastdds::dds::detail::DataReaderInstance instance({}, {});
    const eprosima::fastdds::rtps::GUID_t dw1_guid({}, 1);
    const eprosima::fastdds::rtps::GUID_t dw2_guid({}, 2);
    const eprosima::fastdds::rtps::GUID_t dw3_guid({}, 3);
    const eprosima::fastdds::rtps::GUID_t dw4_guid({}, 4);
    const eprosima::fastdds::rtps::GUID_t another_guid({}, 10);

    instance.alive_writers.push_back({dw1_guid, 1});
    instance.alive_writers.push_back({dw2_guid, 10});
    instance.alive_writers.push_back({dw3_guid, 5});
    instance.alive_writers.push_back({dw4_guid, 10});
    instance.current_owner = {dw2_guid, 10};

    // Call `DataReaderInstance::update_state()` unregistering DW2.
    instance.update_state(counters, eprosima::fastdds::rtps::NOT_ALIVE_UNREGISTERED, dw2_guid, 10);
    // Instance's owner changes to DW4
    ASSERT_EQ(dw4_guid, instance.current_owner.first);
    ASSERT_EQ(10u, instance.current_owner.second);
    ASSERT_EQ(eprosima::fastdds::dds::ALIVE_INSTANCE_STATE, instance.instance_state);
    ASSERT_EQ(3u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(1u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(0u, counters.instances_no_writers);

    // Call `DataReaderInstance::update_state()` unregistering DW3.
    instance.update_state(counters, eprosima::fastdds::rtps::NOT_ALIVE_UNREGISTERED, dw3_guid, 5);
    // Instance's owner doesn't changes.
    ASSERT_EQ(dw4_guid, instance.current_owner.first);
    ASSERT_EQ(10u, instance.current_owner.second);
    ASSERT_EQ(eprosima::fastdds::dds::ALIVE_INSTANCE_STATE, instance.instance_state);
    ASSERT_EQ(2u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(1u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(0u, counters.instances_no_writers);

    // Call `DataReaderInstance::update_state()` unregistering DW4.
    instance.update_state(counters, eprosima::fastdds::rtps::NOT_ALIVE_UNREGISTERED, dw4_guid, 10);
    // Instance's owner changes to DW1
    ASSERT_EQ(dw1_guid, instance.current_owner.first);
    ASSERT_EQ(1u, instance.current_owner.second);
    ASSERT_EQ(eprosima::fastdds::dds::ALIVE_INSTANCE_STATE, instance.instance_state);
    ASSERT_EQ(1u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(1u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(0u, counters.instances_no_writers);

    // Call `DataReaderInstance::update_state()` with with an unknown writer.
    instance.update_state(counters, eprosima::fastdds::rtps::NOT_ALIVE_UNREGISTERED, another_guid, 10);
    // Nothing happens.
    ASSERT_EQ(dw1_guid, instance.current_owner.first);
    ASSERT_EQ(1u, instance.current_owner.second);
    ASSERT_EQ(eprosima::fastdds::dds::ALIVE_INSTANCE_STATE, instance.instance_state);
    ASSERT_EQ(1u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(1u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(0u, counters.instances_no_writers);

    // Call `DataReaderInstance::update_state()` unregistering DW1.
    instance.update_state(counters, eprosima::fastdds::rtps::NOT_ALIVE_UNREGISTERED, dw1_guid, 1);
    // No instance's owner.
    ASSERT_EQ(eprosima::fastdds::rtps::c_Guid_Unknown, instance.current_owner.first);
    ASSERT_EQ(0u, instance.current_owner.second);
    ASSERT_EQ(eprosima::fastdds::dds::NOT_ALIVE_NO_WRITERS_INSTANCE_STATE, instance.instance_state);
    ASSERT_EQ(0u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(0u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(1u, counters.instances_no_writers);
}

/*!
 * \test DDS-OWN-INST-04 Tests instance's owner changes successfully when the deadline is missed.
 */
TEST(DataReaderInstance, deadline_missed)
{
    // Prepare test input.
    eprosima::fastdds::dds::detail::DataReaderInstance instance({}, {});
    const eprosima::fastdds::rtps::GUID_t dw1_guid({}, 1);
    const eprosima::fastdds::rtps::GUID_t dw2_guid({}, 2);
    const eprosima::fastdds::rtps::GUID_t dw3_guid({}, 3);
    const eprosima::fastdds::rtps::GUID_t dw4_guid({}, 4);

    instance.alive_writers.push_back({dw1_guid, 1});
    instance.alive_writers.push_back({dw2_guid, 10});
    instance.alive_writers.push_back({dw3_guid, 5});
    instance.alive_writers.push_back({dw4_guid, 10});
    instance.current_owner = {dw2_guid, 10};

    // Call `DataReaderInstance::deadline_missed()`.
    instance.deadline_missed();
    // Instance's owner changes to DW4
    ASSERT_EQ(dw4_guid, instance.current_owner.first);
    ASSERT_EQ(10u, instance.current_owner.second);
    ASSERT_EQ(eprosima::fastdds::dds::ALIVE_INSTANCE_STATE, instance.instance_state);
    ASSERT_EQ(3u, instance.alive_writers.size());

    // Call `DataReaderInstance::deadline_missed()`.
    instance.deadline_missed();
    // Instance's owner changes to DW3
    ASSERT_EQ(dw3_guid, instance.current_owner.first);
    ASSERT_EQ(5u, instance.current_owner.second);
    ASSERT_EQ(eprosima::fastdds::dds::ALIVE_INSTANCE_STATE, instance.instance_state);
    ASSERT_EQ(2u, instance.alive_writers.size());

    // Call `DataReaderInstance::deadline_missed()`.
    instance.deadline_missed();
    // Instance's owner changes to DW1
    ASSERT_EQ(dw1_guid, instance.current_owner.first);
    ASSERT_EQ(1u, instance.current_owner.second);
    ASSERT_EQ(eprosima::fastdds::dds::ALIVE_INSTANCE_STATE, instance.instance_state);
    ASSERT_EQ(1u, instance.alive_writers.size());

    // Call `DataReaderInstance::deadline_missed()`.
    instance.deadline_missed();
    // No instance's owner.
    ASSERT_EQ(eprosima::fastdds::rtps::c_Guid_Unknown, instance.current_owner.first);
    ASSERT_EQ(0u, instance.current_owner.second);
    ASSERT_EQ(eprosima::fastdds::dds::NOT_ALIVE_NO_WRITERS_INSTANCE_STATE, instance.instance_state);
    ASSERT_EQ(0u, instance.alive_writers.size());

    // Ownership fallback when multiple writers have the same strength
    eprosima::fastdds::dds::detail::DataReaderInstance second_instance({}, {});

    const eprosima::fastdds::rtps::GUID_t dw2_2_guid({}, 3);
    second_instance.alive_writers.push_back({dw2_2_guid, 2});
    second_instance.alive_writers.push_back({dw2_guid, 2});
    second_instance.alive_writers.push_back({dw4_guid, 4});
    second_instance.current_owner = {dw4_guid, 4};
    second_instance.deadline_missed();
    ASSERT_EQ(dw2_guid, second_instance.current_owner.first);
    ASSERT_EQ(2u, second_instance.current_owner.second);
    ASSERT_EQ(eprosima::fastdds::dds::ALIVE_INSTANCE_STATE, second_instance.instance_state);
    ASSERT_EQ(2u, second_instance.alive_writers.size());

}

/*!
 * \test DDS-OWN-INST-05 Tests instance's owner changes successfully when there are several writer with same owner
 * strength.
 */
TEST(DataReaderInstance, same_ownership_strenght)
{
    // Prepare test input.
    eprosima::fastdds::dds::detail::DataReaderHistoryCounters counters;
    eprosima::fastdds::dds::detail::DataReaderInstance instance({}, {});
    const eprosima::fastdds::rtps::GUID_t dw1_guid({}, 2);
    const eprosima::fastdds::rtps::GUID_t dw2_guid({}, 3);
    const eprosima::fastdds::rtps::GUID_t dw3_guid({}, 1);

    // Calls `DataReaderInstance::update_state()` with a new DW1 writer with strength 10.
    instance.update_state(counters, eprosima::fastdds::rtps::ALIVE, dw1_guid, 10);
    // Instance has a new owner DW1 with strength 10.
    ASSERT_EQ(dw1_guid, instance.current_owner.first);
    ASSERT_EQ(10u, instance.current_owner.second);
    ASSERT_EQ(1u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(1u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(0u, counters.instances_no_writers);

    // Calls `DataReaderInstance::update_state()` with a new DW2 writer with strength 10 and greater GUID.
    instance.update_state(counters, eprosima::fastdds::rtps::ALIVE, dw2_guid, 10);
    // Instance has a new "alive" writer DW3 with strength 10 and  owner doesn't change.
    ASSERT_EQ(dw1_guid, instance.current_owner.first);
    ASSERT_EQ(10u, instance.current_owner.second);
    ASSERT_EQ(2u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(1u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(0u, counters.instances_no_writers);

    // Calls `DataReaderInstance::update_state()` with a new DW3 writer with strength 10.
    instance.update_state(counters, eprosima::fastdds::rtps::ALIVE, dw3_guid, 10);
    // Instance has a new "alive" writer DW3 with strength 10 and  DW3 is the new owner.
    ASSERT_EQ(dw3_guid, instance.current_owner.first);
    ASSERT_EQ(10u, instance.current_owner.second);
    ASSERT_EQ(3u, instance.alive_writers.size());
    ASSERT_EQ(1u, counters.instances_new);
    ASSERT_EQ(0u, counters.instances_not_new);
    ASSERT_EQ(1u, counters.instances_alive);
    ASSERT_EQ(0u, counters.instances_disposed);
    ASSERT_EQ(0u, counters.instances_no_writers);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
