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
    MOCK_METHOD2(serialize, bool(
                void* data,
                eprosima::fastrtps::rtps::SerializedPayload_t* payload));

    MOCK_METHOD2(deserialize, bool(
                eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                void* data));

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
 * \test DDS-OWN-HIST-01 Tests `DataReaderInstance` handles successfully the reception of Non-Keyed samples and with
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
    dw1_change.sequenceNumber = {0, 1};
    dw1_change.reader_info.writer_ownership_strength = 1;
    eprosima::fastrtps::rtps::CacheChange_t dw2_change;
    dw2_change.writerGUID = {{}, 2};
    dw2_change.sequenceNumber = {0, 1};
    dw2_change.reader_info.writer_ownership_strength = 2;
    eprosima::fastrtps::rtps::CacheChange_t dw3_change;
    dw3_change.writerGUID = {{}, 3};
    dw3_change.sequenceNumber = {0, 1};
    dw3_change.reader_info.writer_ownership_strength = 3;

    // Receives a sample with seq 1 from DW1 and update instance with strength 1.
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw1_change));


    // Receives a sample with seq 1 from DW2 and update instance with strength 2.
    ASSERT_TRUE(history.received_change(&dw2_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw2_change));

    // Receives a sample with seq 2 from DW1 and update instance with strength 1.
    dw1_change.sequenceNumber = {0, 2};
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_FALSE(history.update_instance_nts(&dw1_change));

    // Receives a sample with seq 2 from DW2 and update instance with strength 2.
    dw2_change.sequenceNumber = {0, 2};
    ASSERT_TRUE(history.received_change(&dw2_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw2_change));

    // Receives a sample with seq 1 from DW3 and update instance with strength 3.
    ASSERT_TRUE(history.received_change(&dw3_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw3_change));

    // Receives a sample with seq 3 from DW1 and update instance with strength 1.
    dw1_change.sequenceNumber = {0, 3};
    ASSERT_TRUE(history.received_change(&dw1_change, 0));
    ASSERT_FALSE(history.update_instance_nts(&dw1_change));

    // Receives a sample with seq 3 from DW2 and update instance with strength 2.
    dw2_change.sequenceNumber = {0, 3};
    ASSERT_TRUE(history.received_change(&dw2_change, 0));
    ASSERT_FALSE(history.update_instance_nts(&dw2_change));

    // Receives a sample with seq 2 from DW3 and update instance with strength 1.
    dw3_change.sequenceNumber = {0, 2};
    dw3_change.reader_info.writer_ownership_strength = 1;
    ASSERT_TRUE(history.received_change(&dw3_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw3_change));

    // Receives a sample with seq 4 from DW2 and update instance with strength 2.
    dw2_change.sequenceNumber = {0, 4};
    ASSERT_TRUE(history.received_change(&dw2_change, 0));
    ASSERT_TRUE(history.update_instance_nts(&dw2_change));

    ASSERT_EQ(9u, history.getHistorySize());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
