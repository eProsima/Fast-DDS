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
    DataReaderHistory history(type, nullptr, topic, qos);
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
    DataReaderHistory history(type, nullptr, topic, qos);
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

/*!
 * Regression test for https://github.com/eProsima/Fast-DDS/issues/6567
 *
 * A fragmented change of a non-keyed topic is stored in the instance only when it is completed, as is done for keyed
 * topics. Until then it does not replace the oldest sample of a KEEP_LAST instance.
 */
TEST(DataReaderHistory, non_keyed_fragmented_change_added_to_instance_when_completed)
{
    using eprosima::fastdds::rtps::CacheChange_t;
    using eprosima::fastdds::rtps::SerializedPayload_t;

    const TypeSupport type(new TestType());
    const Topic topic("test", "test");
    DataReaderQos qos;
    qos.history().kind = KEEP_LAST_HISTORY_QOS;
    qos.history().depth = 1;
    qos.endpoint().history_memory_policy = eprosima::fastdds::rtps::DYNAMIC_RESERVE_MEMORY_MODE;
    DataReaderHistory history(type, nullptr, topic, qos);
    eprosima::fastdds::RecursiveTimedMutex mutex;
    eprosima::fastdds::rtps::StatelessReader reader(&history, &mutex);

    auto instance_changes = [&history]()
            {
                auto instance = history.lookup_available_instance(InstanceHandle_t(), false);
                EXPECT_TRUE(instance.first);
                std::vector<CacheChange_t*> ret;
                for (CacheChange_t* change : instance.second->second->cache_changes)
                {
                    ret.push_back(change);
                }
                return ret;
            };

    CacheChange_t complete_change;
    complete_change.writerGUID = {{}, 1};
    complete_change.sequenceNumber = {0, 1};
    eprosima::fastdds::rtps::Time_t::now(complete_change.sourceTimestamp);
    ASSERT_TRUE(history.received_change(&complete_change, 0));

    constexpr uint16_t fragment_size = 4;
    CacheChange_t fragmented_change;
    fragmented_change.writerGUID = {{}, 1};
    fragmented_change.sequenceNumber = {0, 2};
    eprosima::fastdds::rtps::Time_t::now(fragmented_change.sourceTimestamp);
    fragmented_change.serializedPayload.reserve(2 * fragment_size);
    fragmented_change.serializedPayload.length = 2 * fragment_size;
    fragmented_change.setFragmentSize(fragment_size, true);
    SerializedPayload_t fragment(fragment_size);
    fragment.length = fragment_size;

    // First fragment: the change is kept in the history only
    ASSERT_FALSE(fragmented_change.add_fragments(fragment, 1, 1));
    ASSERT_TRUE(history.received_change(&fragmented_change, 0));
    EXPECT_EQ(2u, history.getHistorySize());
    EXPECT_EQ(std::vector<CacheChange_t*>{&complete_change}, instance_changes());

    // Last fragment: the change replaces the oldest sample in the instance
    ASSERT_TRUE(fragmented_change.add_fragments(fragment, 2, 1));
    ASSERT_TRUE(history.completed_change(&fragmented_change));
    EXPECT_EQ(1u, history.getHistorySize());
    EXPECT_EQ(std::vector<CacheChange_t*>{&fragmented_change}, instance_changes());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
