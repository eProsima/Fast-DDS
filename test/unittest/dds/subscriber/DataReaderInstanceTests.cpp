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
    const eprosima::fastrtps::rtps::GUID_t dw1_guid({}, 1);
    const eprosima::fastrtps::rtps::GUID_t dw2_guid({}, 2);
    const eprosima::fastrtps::rtps::GUID_t dw3_guid({}, 3);
    const eprosima::fastrtps::rtps::GUID_t another_guid({}, 4);

    instance.alive_writers.push_back({dw1_guid, 1});
    instance.alive_writers.push_back({dw2_guid, 10});
    instance.alive_writers.push_back({dw3_guid, 5});
    instance.current_owner = {dw2_guid, 10};


    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing DW2's strength to 11.
    instance.writer_update_its_ownership_strength(dw2_guid, 11);
    // DW2 changes strength to 11 and still is the owner.
    ASSERT_EQ(dw2_guid, instance.current_owner.first);
    ASSERT_EQ(11, instance.current_owner.second);

    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing DW1's strength to 4.
    instance.writer_update_its_ownership_strength(dw1_guid, 4);
    // DW1 changes strength to 4 and the owner doesn't changes.
    ASSERT_EQ(dw2_guid, instance.current_owner.first);
    ASSERT_EQ(11, instance.current_owner.second);

    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing DW1's strength to 12.
    instance.writer_update_its_ownership_strength(dw1_guid, 12);
    // DW1 changes strength to 12 and it is the new owner.
    ASSERT_EQ(dw1_guid, instance.current_owner.first);
    ASSERT_EQ(12, instance.current_owner.second);

    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing DW2's strength to 13.
    instance.writer_update_its_ownership_strength(dw2_guid, 13);
    // DW2 changes strength to 13 and it is the new owner.
    ASSERT_EQ(dw2_guid, instance.current_owner.first);
    ASSERT_EQ(13, instance.current_owner.second);

    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing DW3's strength to 6.
    instance.writer_update_its_ownership_strength(dw3_guid, 6);
    // DW3 changes strength to 6 and the owner doesn't changes.
    ASSERT_EQ(dw2_guid, instance.current_owner.first);
    ASSERT_EQ(13, instance.current_owner.second);

    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing DW2's strength to 6.
    instance.writer_update_its_ownership_strength(dw2_guid, 6);
    // DW2 changes strength to 6 and losses the ownership. DW1 is the new owner.
    ASSERT_EQ(dw1_guid, instance.current_owner.first);
    ASSERT_EQ(12, instance.current_owner.second);

    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing DW3's strength to 14.
    instance.writer_update_its_ownership_strength(dw3_guid, 14);
    // DW3 changes strength to 14 and it is the new owner.
    ASSERT_EQ(dw3_guid, instance.current_owner.first);
    ASSERT_EQ(14, instance.current_owner.second);


    // Calls `DataReaderInstance::writer_update_its_ownership_strength()` changing no registered DataWriter's strength
    // to 20.
    instance.writer_update_its_ownership_strength(another_guid, 20);
    // Anything changes because it is not an "alive" writer of the instance.
    ASSERT_EQ(dw3_guid, instance.current_owner.first);
    ASSERT_EQ(14, instance.current_owner.second);
    ASSERT_EQ(3, instance.alive_writers.size());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
