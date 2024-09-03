#include <fastdds/subscriber/history/DataReaderHistory.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/utils/TimedMutex.hpp>
#include <rtps/reader/StatelessReader.hpp>


#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::detail;

// Auxiliary class
class TestType : public TopicDataType
{
public:

    MOCK_METHOD(bool, serialize, (
                const void* const data,
                eprosima::fastdds::rtps::SerializedPayload_t& payload,
                DataRepresentationId_t data_representation),
            (override));

    MOCK_METHOD(bool, deserialize, (
                eprosima::fastdds::rtps::SerializedPayload_t& payload,
                void* data),
            (override));

    MOCK_METHOD(uint32_t, calculate_serialized_size, (
                const void* const data, DataRepresentationId_t data_representation),
            (override));

    MOCK_METHOD(void*, create_data, (), (override));

    MOCK_METHOD(void, delete_data, (
                void* data),
            (override));

    MOCK_METHOD(bool, compute_key, (
                eprosima::fastdds::rtps::SerializedPayload_t& payload,
                eprosima::fastdds::dds::InstanceHandle_t& ihandle,
                bool),
            (override));

    MOCK_METHOD(bool, compute_key, (
                const void* const data,
                eprosima::fastdds::dds::InstanceHandle_t& ihandle,
                bool),
            (override));
};

bool add_test_change(
        eprosima::fastdds::dds::detail::DataReaderHistory& history,
        eprosima::fastdds::rtps::CacheChange_t& change,
        std::vector<std::unique_ptr<eprosima::fastdds::rtps::CacheChange_t>>& test_changes)
{
    ++change.sequenceNumber;
    eprosima::fastdds::rtps::Time_t::now(change.sourceTimestamp);
    eprosima::fastdds::rtps::CacheChange_t* new_change = new eprosima::fastdds::rtps::CacheChange_t();
    new_change->copy(&change);
    new_change->reader_info.writer_ownership_strength = change.reader_info.writer_ownership_strength;

    EXPECT_TRUE(history.received_change(new_change, 0));
    bool ret = history.update_instance_nts(new_change);
    test_changes.push_back(std::unique_ptr<eprosima::fastdds::rtps::CacheChange_t>(new_change));
    return ret;
}

/*!
 * \test DDS-OWN-HIST-01 Tests `DataReaderInstance` handles successfully the reception of Non-Keyed samples with
 * different Ownership's strength.
 */
TEST(DataReaderHistory, exclusive_ownership_non_keyed_sample_reception)
{
    const TypeSupport type(new TestType());
    const Topic topic("test", "test");
    DataReaderQos qos;
    qos.ownership().kind = eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS;
    qos.history().kind = KEEP_ALL_HISTORY_QOS;
    DataReaderHistory history(type, topic, qos);
    eprosima::fastdds::RecursiveTimedMutex mutex;
    eprosima::fastdds::rtps::StatelessReader reader(&history, &mutex);
    std::vector<std::unique_ptr<eprosima::fastdds::rtps::CacheChange_t>> changes;

    eprosima::fastdds::rtps::CacheChange_t dw1_change;
    dw1_change.writerGUID = {{}, 1};
    dw1_change.reader_info.writer_ownership_strength = 1;
    eprosima::fastdds::rtps::CacheChange_t dw2_change;
    dw2_change.writerGUID = {{}, 2};
    dw2_change.reader_info.writer_ownership_strength = 2;
    eprosima::fastdds::rtps::CacheChange_t dw3_change;
    dw3_change.writerGUID = {{}, 3};
    dw3_change.reader_info.writer_ownership_strength = 3;

    // Receives a sample with seq 1 from DW1 and update instance with strength 1.
    ASSERT_TRUE(add_test_change(history, dw1_change, changes));

    // Receives a sample with seq 1 from DW2 and update instance with strength 2.
    ASSERT_TRUE(add_test_change(history, dw2_change, changes));

    // Receives a sample with seq 2 from DW1 and update instance with strength 1.
    ASSERT_FALSE(add_test_change(history, dw1_change, changes));

    // Receives a sample with seq 2 from DW2 and update instance with strength 2.
    ASSERT_TRUE(add_test_change(history, dw2_change, changes));

    // Receives a sample with seq 1 from DW3 and update instance with strength 3.
    ASSERT_TRUE(add_test_change(history, dw3_change, changes));

    // Receives a sample with seq 3 from DW1 and update instance with strength 1.
    ASSERT_FALSE(add_test_change(history, dw1_change, changes));

    // Receives a sample with seq 3 from DW2 and update instance with strength 2.
    ASSERT_FALSE(add_test_change(history, dw2_change, changes));

    // Receives a sample with seq 2 from DW3 and update instance with strength 1.
    dw3_change.reader_info.writer_ownership_strength = 1;
    ASSERT_TRUE(add_test_change(history, dw3_change, changes));

    // Receives a sample with seq 4 from DW2 and update instance with strength 2.
    ASSERT_TRUE(add_test_change(history, dw2_change, changes));

    ASSERT_EQ(9u, history.getHistorySize());
}

/*!
 * \test DDS-OWN-HIST-02 Tests `DataReaderInstance` handles successfully the reception of Keyed samples with
 * different Ownership's strength.
 */
TEST(DataReaderHistory, exclusive_ownership_keyed_sample_reception)
{
    TestType* type_ = new TestType();

    const TypeSupport type(type_);
    type->is_compute_key_provided = true;
    const Topic topic("test", "test");
    DataReaderQos qos;
    qos.ownership().kind = eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS;
    qos.history().kind = KEEP_ALL_HISTORY_QOS;
    DataReaderHistory history(type, topic, qos);
    eprosima::fastdds::RecursiveTimedMutex mutex;
    eprosima::fastdds::rtps::StatelessReader reader(&history, &mutex);
    std::vector<std::unique_ptr<eprosima::fastdds::rtps::CacheChange_t>> changes;

    const InstanceHandle_t instance_1 = eprosima::fastdds::rtps::GUID_t{{}, 1};
    const InstanceHandle_t instance_2 = eprosima::fastdds::rtps::GUID_t{{}, 2};
    const InstanceHandle_t instance_3 = eprosima::fastdds::rtps::GUID_t{{}, 3};
    eprosima::fastdds::rtps::CacheChange_t dw1_change;
    dw1_change.writerGUID = {{}, 1};
    dw1_change.reader_info.writer_ownership_strength = 1;
    eprosima::fastdds::rtps::CacheChange_t dw2_change;
    dw2_change.writerGUID = {{}, 2};
    dw2_change.reader_info.writer_ownership_strength = 2;
    eprosima::fastdds::rtps::CacheChange_t dw3_change;
    dw3_change.writerGUID = {{}, 3};
    dw3_change.reader_info.writer_ownership_strength = 3;

    // Receives instance 1 with seq 1 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_1;
    ASSERT_TRUE(add_test_change(history, dw1_change, changes));

    // Receives instance 2 with seq 2 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_2;
    ASSERT_TRUE(add_test_change(history, dw1_change, changes));

    // Receives instance 1 with seq 1 from DW2 and update instance with strength 2.
    dw2_change.instanceHandle = instance_1;
    ASSERT_TRUE(add_test_change(history, dw2_change, changes));

    // Receives instance 1 with seq 3 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_1;
    ASSERT_FALSE(add_test_change(history, dw1_change, changes));

    // Receives instance 2 with seq 4 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_2;
    ASSERT_TRUE(add_test_change(history, dw1_change, changes));

    // Receives instance 2 with seq 1 from DW3 and update instance with strength 3.
    dw3_change.instanceHandle = instance_2;
    ASSERT_TRUE(add_test_change(history, dw3_change, changes));

    // Receives instance 1 with seq 5 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_1;
    ASSERT_FALSE(add_test_change(history, dw1_change, changes));

    // Receives instance 2 with seq 6 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_2;
    ASSERT_FALSE(add_test_change(history, dw1_change, changes));

    // Receives instance 3 with seq 2 from DW3 and update instance with strength 3.
    dw3_change.instanceHandle = instance_3;
    ASSERT_TRUE(add_test_change(history, dw3_change, changes));

    // Receives instance 3 with seq 2 from DW2 and update instance with strength 2.
    dw2_change.instanceHandle = instance_3;
    ASSERT_FALSE(add_test_change(history, dw2_change, changes));

    // Receives instance 3 with seq 3 from DW3 and update instance with strength 1.
    dw3_change.instanceHandle = instance_3;
    dw3_change.reader_info.writer_ownership_strength = 1;
    ASSERT_TRUE(add_test_change(history, dw3_change, changes));

    // Receives instance 3 with seq 3 from DW2 and update instance with strength 2.
    dw2_change.instanceHandle = instance_3;
    ASSERT_TRUE(add_test_change(history, dw2_change, changes));

    // Receives instance 1 with seq 7 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_1;
    ASSERT_FALSE(add_test_change(history, dw1_change, changes));

    // Receives instance 2 with seq 8 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_2;
    ASSERT_FALSE(add_test_change(history, dw1_change, changes));

    // Receives instance 3 with seq 9 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_3;
    ASSERT_FALSE(add_test_change(history, dw1_change, changes));

    // Receives instance 1 with seq 4 from DW2 and update instance with strength 2.
    dw2_change.instanceHandle = instance_1;
    ASSERT_TRUE(add_test_change(history, dw2_change, changes));

    // Receives instance 2 with seq 5 from DW2 and update instance with strength 2.
    dw2_change.instanceHandle = instance_2;
    ASSERT_FALSE(add_test_change(history, dw2_change, changes));

    // Receives instance 3 with seq 6 from DW2 and update instance with strength 2.
    dw2_change.instanceHandle = instance_3;
    ASSERT_TRUE(add_test_change(history, dw2_change, changes));

    ASSERT_EQ(18u, history.getHistorySize());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
