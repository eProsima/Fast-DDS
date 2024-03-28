#include <fastdds/subscriber/history/DataReaderHistory.hpp>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastrtps/utils/TimedMutex.hpp>
#include <fastdds/rtps/reader/StatelessReader.h>


#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::detail;

// Auxiliary class
class TestType : public TopicDataType
{
public:

    MOCK_METHOD2(serialize, bool(
                void* data,
                eprosima::fastrtps::rtps::SerializedPayload_t* payload));

    MOCK_METHOD3(serialize, bool(
                void* data,
                eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                DataRepresentationId_t data_representation));

    MOCK_METHOD2(deserialize, bool(
                eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                void* data));

    MOCK_METHOD2(getSerializedSizeProvider, std::function<uint32_t()> (
                void* data, DataRepresentationId_t data_representation));

    MOCK_METHOD1(getSerializedSizeProvider, std::function<uint32_t()> (
                void* data));

    MOCK_METHOD0(createData, void* ());

    MOCK_METHOD1(deleteData, void(
                void* data));

    MOCK_METHOD3(getKey, bool(
                void* data,
                eprosima::fastdds::dds::InstanceHandle_t* ihandle,
                bool));
};

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
    eprosima::fastrtps::RecursiveTimedMutex mutex;
    eprosima::fastrtps::rtps::StatelessReader reader(&history, &mutex);

    eprosima::fastrtps::rtps::CacheChange_t dw1_change;
    dw1_change.writerGUID = {{}, 1};
    dw1_change.reader_info.writer_ownership_strength = 1;
    eprosima::fastrtps::rtps::CacheChange_t dw2_change;
    dw2_change.writerGUID = {{}, 2};
    dw2_change.reader_info.writer_ownership_strength = 2;
    eprosima::fastrtps::rtps::CacheChange_t dw3_change;
    dw3_change.writerGUID = {{}, 3};
    dw3_change.reader_info.writer_ownership_strength = 3;

    // Receives a sample with seq 1 from DW1 and update instance with strength 1.
    ++dw1_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw1_change));

    // Receives a sample with seq 1 from DW2 and update instance with strength 2.
    ++dw2_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw2_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw2_change));

    // Receives a sample with seq 2 from DW1 and update instance with strength 1.
    ++dw1_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_FALSE(history.update_instance_nts(&dw1_change));

    // Receives a sample with seq 2 from DW2 and update instance with strength 2.
    ++dw2_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw2_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw2_change));

    // Receives a sample with seq 1 from DW3 and update instance with strength 3.
    ++dw3_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw3_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw3_change));

    // Receives a sample with seq 3 from DW1 and update instance with strength 1.
    ++dw1_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_FALSE(history.update_instance_nts(&dw1_change));

    // Receives a sample with seq 3 from DW2 and update instance with strength 2.
    ++dw2_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw2_change, 0));
    ASSERT_FALSE(history.update_instance_nts(&dw2_change));

    // Receives a sample with seq 2 from DW3 and update instance with strength 1.
    ++dw3_change.sequenceNumber;
    dw3_change.reader_info.writer_ownership_strength = 1;
    ASSERT_TRUE(history.received_change(&dw3_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw3_change));

    // Receives a sample with seq 4 from DW2 and update instance with strength 2.
    ++dw2_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw2_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw2_change));

    ASSERT_EQ(9u, history.getHistorySize());
}

/*!
 * \test DDS-OWN-HIST-02 Tests `DataReaderInstance` handles successfully the reception of Keyed samples with
 * different Ownership's strength.
 */
TEST(DataReaderHistory, exclusive_ownership_keyed_sample_reception)
{
    TestType* type_ = new TestType();
    // These functions was called due to the type is keyed.
    EXPECT_CALL(*type_, createData()).Times(1);
    EXPECT_CALL(*type_, deleteData(nullptr)).Times(1);

    const TypeSupport type(type_);
    type->m_isGetKeyDefined = true;
    const Topic topic("test", "test");
    DataReaderQos qos;
    qos.ownership().kind = eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS;
    qos.history().kind = KEEP_ALL_HISTORY_QOS;
    DataReaderHistory history(type, topic, qos);
    eprosima::fastrtps::RecursiveTimedMutex mutex;
    eprosima::fastrtps::rtps::StatelessReader reader(&history, &mutex);

    const InstanceHandle_t instance_1 = eprosima::fastrtps::rtps::GUID_t{{}, 1};
    const InstanceHandle_t instance_2 = eprosima::fastrtps::rtps::GUID_t{{}, 2};
    const InstanceHandle_t instance_3 = eprosima::fastrtps::rtps::GUID_t{{}, 3};
    eprosima::fastrtps::rtps::CacheChange_t dw1_change;
    dw1_change.writerGUID = {{}, 1};
    dw1_change.reader_info.writer_ownership_strength = 1;
    eprosima::fastrtps::rtps::CacheChange_t dw2_change;
    dw2_change.writerGUID = {{}, 2};
    dw2_change.reader_info.writer_ownership_strength = 2;
    eprosima::fastrtps::rtps::CacheChange_t dw3_change;
    dw3_change.writerGUID = {{}, 3};
    dw3_change.reader_info.writer_ownership_strength = 3;

    // Receives instance 1 with seq 1 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_1;
    ++dw1_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw1_change));

    // Receives instance 2 with seq 2 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_2;
    ++dw1_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw1_change));

    // Receives instance 1 with seq 1 from DW2 and update instance with strength 2.
    dw2_change.instanceHandle = instance_1;
    ++dw2_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw2_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw2_change));

    // Receives instance 1 with seq 3 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_1;
    ++dw1_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_FALSE(history.update_instance_nts(&dw1_change));

    // Receives instance 2 with seq 4 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_2;
    ++dw1_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw1_change));

    // Receives instance 2 with seq 1 from DW3 and update instance with strength 3.
    dw3_change.instanceHandle = instance_2;
    ++dw3_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw3_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw3_change));

    // Receives instance 1 with seq 5 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_1;
    ++dw1_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_FALSE(history.update_instance_nts(&dw1_change));

    // Receives instance 2 with seq 6 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_2;
    ++dw1_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_FALSE(history.update_instance_nts(&dw1_change));

    // Receives instance 3 with seq 2 from DW3 and update instance with strength 3.
    dw3_change.instanceHandle = instance_3;
    ++dw3_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw3_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw3_change));

    // Receives instance 3 with seq 2 from DW2 and update instance with strength 2.
    dw2_change.instanceHandle = instance_3;
    ++dw2_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw2_change, 0));
    ASSERT_FALSE(history.update_instance_nts(&dw2_change));

    // Receives instance 3 with seq 3 from DW3 and update instance with strength 1.
    dw3_change.instanceHandle = instance_3;
    dw3_change.reader_info.writer_ownership_strength = 1;
    ++dw3_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw3_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw3_change));

    // Receives instance 3 with seq 3 from DW2 and update instance with strength 2.
    dw2_change.instanceHandle = instance_3;
    ++dw2_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw2_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw2_change));

    // Receives instance 1 with seq 7 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_1;
    ++dw1_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_FALSE(history.update_instance_nts(&dw1_change));

    // Receives instance 2 with seq 8 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_2;
    ++dw1_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_FALSE(history.update_instance_nts(&dw1_change));

    // Receives instance 3 with seq 9 from DW1 and update instance with strength 1.
    dw1_change.instanceHandle = instance_3;
    ++dw1_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_FALSE(history.update_instance_nts(&dw1_change));

    // Receives instance 1 with seq 4 from DW2 and update instance with strength 2.
    dw2_change.instanceHandle = instance_1;
    ++dw2_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw2_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw2_change));

    // Receives instance 2 with seq 5 from DW2 and update instance with strength 2.
    dw2_change.instanceHandle = instance_2;
    ++dw2_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw2_change, 0));
    ASSERT_FALSE(history.update_instance_nts(&dw2_change));

    // Receives instance 3 with seq 6 from DW2 and update instance with strength 2.
    dw2_change.instanceHandle = instance_3;
    ++dw2_change.sequenceNumber;
    ASSERT_TRUE(history.received_change(&dw2_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw2_change));

    ASSERT_EQ(18u, history.getHistorySize());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
