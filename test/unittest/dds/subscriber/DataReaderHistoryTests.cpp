#include <fastdds/subscriber/history/DataReaderHistory.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/topic/Topic.hpp>

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

TEST(DataReaderHistory, test)
{
    TestType test_type;
    const TypeSupport type(&test_type);
    const Topic topic("test", "test");
    const DataReaderQos qos;
    DataReaderHistory history(type, topic, qos);

    history.received_change(nullptr, 0);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
