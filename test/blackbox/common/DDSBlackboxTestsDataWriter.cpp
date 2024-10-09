// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <chrono>

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>

#include "BlackboxTests.hpp"
#include "PubSubParticipant.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

using namespace eprosima::fastdds;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class DDSDataWriter : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = false;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

/**
 * Test that checks DataWriter::wait_for_acknowledgments for a specific instance
 */
TEST_P(DDSDataWriter, WaitForAcknowledgmentInstance)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();

    writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport).init();
    ASSERT_TRUE(writer.isInitialized());

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();
    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    // Disable communication to prevent reception of ACKs
    test_transport->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = true;

    auto data = default_keyedhelloworld_data_generator(2);

    // Register instances
    auto instance_handle_1 = writer.register_instance(data.front());
    EXPECT_NE(instance_handle_1, rtps::c_InstanceHandle_Unknown);
    auto instance_handle_2 = writer.register_instance(data.back());
    EXPECT_NE(instance_handle_2, rtps::c_InstanceHandle_Unknown);

    reader.startReception(data);

    KeyedHelloWorld sample_1 = data.front();
    KeyedHelloWorld sample_2 = data.back();

    writer.send(data);
    EXPECT_TRUE(data.empty());

    // Intraprocess does not use transport layer. The ACKs cannot be disabled.
    if (INTRAPROCESS != GetParam())
    {
        EXPECT_FALSE(writer.waitForInstanceAcked(&sample_1, rtps::c_InstanceHandle_Unknown, std::chrono::seconds(1)));
        EXPECT_FALSE(writer.waitForInstanceAcked(&sample_2, instance_handle_2, std::chrono::seconds(1)));
    }
    else
    {
        std::cout << "INTRAPROCESS does not use transport layer. Therefore ACKs cannot be disabled" << std::endl;
    }

    // Enable communication and wait for acknowledgment
    test_transport->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = false;

    EXPECT_TRUE(writer.waitForInstanceAcked(&sample_1, instance_handle_1, std::chrono::seconds(1)));
    EXPECT_TRUE(writer.waitForInstanceAcked(&sample_2, rtps::c_InstanceHandle_Unknown, std::chrono::seconds(1)));

}

/**
 * Test that checks DataWriter::get_key_value
 */
TEST_P(DDSDataWriter, GetKeyValue)
{
    using namespace eprosima::fastdds::dds;

    // Test variables
    KeyedHelloWorld data;
    eprosima::fastdds::rtps::InstanceHandle_t wrong_handle;
    wrong_handle.value[0] = 0xee;
    eprosima::fastdds::rtps::InstanceHandle_t valid_handle;
    KeyedHelloWorld valid_data;
    valid_data.key(27);
    valid_data.index(1);
    valid_data.message("HelloWorld");

    // Create and initialize writers
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME + "_KEY");
    PubSubWriter<KeyedHelloWorldPubSubType> keyed_writer(TEST_TOPIC_NAME + "_KEY");
    writer.init();
    keyed_writer.init();

    DataWriter* datawriter = &writer.get_native_writer();
    DataWriter* instance_datawriter = &keyed_writer.get_native_writer();

    // 1. Check nullptr on key_holder
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, datawriter->get_key_value(nullptr, wrong_handle));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(nullptr, wrong_handle));

    // 2. Check HANDLE_NIL
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, datawriter->get_key_value(&data, HANDLE_NIL));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(&data, HANDLE_NIL));

    // 3. Check type should have keys
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_ILLEGAL_OPERATION, datawriter->get_key_value(&data, wrong_handle));

    // 4. Calling get_key_value with a key not yet registered returns eprosima::fastdds::dds::RETCODE_BAD_PARAMETER
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(&data, wrong_handle));

    // 5. Calling get_key_value on a registered instance should work.
    valid_handle = instance_datawriter->register_instance(&valid_data);
    EXPECT_NE(HANDLE_NIL, valid_handle);
    data.key(0);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, instance_datawriter->get_key_value(&data, valid_handle));
    EXPECT_EQ(valid_data.key(), data.key());

    // 6. Calling get_key_value on an unregistered instance should return eprosima::fastdds::dds::RETCODE_BAD_PARAMETER.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, instance_datawriter->unregister_instance(&valid_data, valid_handle));
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(&data, valid_handle));

    // 7. Calling get_key_value with a valid instance should work
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, instance_datawriter->write(&valid_data, HANDLE_NIL));
    data.key(0);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, instance_datawriter->get_key_value(&data, valid_handle));
    EXPECT_EQ(valid_data.key(), data.key());

    // 8. Calling get_key_value on a disposed instance should work.
    ASSERT_EQ(eprosima::fastdds::dds::RETCODE_OK, instance_datawriter->dispose(&valid_data, valid_handle));
    data.key(0);
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, instance_datawriter->get_key_value(&data, valid_handle));
    EXPECT_EQ(valid_data.key(), data.key());
}

TEST_P(DDSDataWriter, WithTimestampOperations)
{
    using namespace eprosima::fastdds::dds;

    // Test variables
    eprosima::fastdds::dds::Time_t ts;

    KeyedHelloWorld valid_data;
    valid_data.key(27);
    valid_data.index(1);
    valid_data.message("HelloWorld");

    // Create and initialize reader
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .history_depth(10)
            .init();
    ASSERT_TRUE(reader.isInitialized());
    DataReader& datareader = reader.get_native_reader();

    // Create and initialize writer
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .history_depth(10)
            .init();
    ASSERT_TRUE(writer.isInitialized());
    DataWriter& datawriter = writer.get_native_writer();
    DataWriterQos qos = datawriter.get_qos();
    qos.writer_data_lifecycle().autodispose_unregistered_instances = false;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, datawriter.set_qos(qos));

    // Wait discovery, since we are going to unregister an instance
    reader.wait_discovery();
    writer.wait_discovery();

    ts.seconds = 0;
    ts.nanosec = 1;
    // Register with custom timestamp
    EXPECT_NE(HANDLE_NIL, datawriter.register_instance_w_timestamp(&valid_data, ts));
    // TODO(MiguelCompay): Remove the following line when register_instance operation gets propagated to the reader.
    // See redmine issue #14494
    ts.nanosec--;
    // Write with custom timestamp
    ts.nanosec++;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, datawriter.write_w_timestamp(&valid_data, HANDLE_NIL, ts));
    // Dispose with custom timestamp
    ts.nanosec++;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, datawriter.dispose_w_timestamp(&valid_data, HANDLE_NIL, ts));
    // Write with custom timestamp
    ts.nanosec++;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, datawriter.write_w_timestamp(&valid_data, HANDLE_NIL, ts));
    // Unregister with custom timestamp
    ts.nanosec++;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, datawriter.unregister_instance_w_timestamp(&valid_data, HANDLE_NIL,
            ts));

    // Wait and take all data
    auto num_samples = ts.nanosec;
    while (num_samples != datareader.get_unread_count())
    {
        EXPECT_TRUE(datareader.wait_for_unread_message({ 10, 0 }));
    }

    FASTDDS_CONST_SEQUENCE(DataSeq, KeyedHelloWorld);
    SampleInfoSeq infos;
    DataSeq datas;
    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, datareader.take(datas, infos));

    // Check received timestamps
    ts.seconds = 0;
    ts.nanosec = 1;
    EXPECT_EQ(static_cast<decltype(num_samples)>(infos.length()), num_samples);
    for (SampleInfoSeq::size_type n = 0; n < infos.length(); ++n)
    {
        EXPECT_EQ(ts, infos[n].source_timestamp);
        ts.nanosec++;
    }

    EXPECT_EQ(eprosima::fastdds::dds::RETCODE_OK, datareader.return_loan(datas, infos));
}

/**
 * Regression test for EasyRedmine issue https://eprosima.easyredmine.com/issues/17961
 *
 * The test:
 *     1. Creates a DomainParticipant with a listener which captures the offered_deadline_missed
 *        events.
 *     2. Creates a DataWriter with a 1 ms deadline period, without any listener and that never
 *        publishes data.
 *     3. Wait for the deadline callback to be triggered at least once within a second.
 *     4. Checks that the callback was indeed triggered.
 */

TEST(DDSDataWriter, OfferedDeadlineMissedListener)
{
    using namespace eprosima::fastdds::dds;

    class WriterWrapper : public DomainParticipantListener
    {
    public:

        WriterWrapper(
                std::condition_variable& cv,
                std::atomic_bool& deadline_called)
            : cv_(cv)
            , deadline_called_(deadline_called)
        {
            StatusMask status_mask = StatusMask::none();
            status_mask << StatusMask::offered_deadline_missed();
            participant_ = DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT,
                            this, status_mask);

            type_support_.reset(new HelloWorldPubSubType());
            type_support_.register_type(participant_, "DeadlineListenerTest");

            topic_ = participant_->create_topic("deadline_listener_test", "DeadlineListenerTest", TOPIC_QOS_DEFAULT);

            publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);

            DataWriterQos dw_qos = DATAWRITER_QOS_DEFAULT;
            dw_qos.deadline().period = Time_t(0, 1000000);
            datawriter_ = publisher_->create_datawriter(
                topic_,
                dw_qos);

            // Apparently the time is not started until the first data is published
            HelloWorld data;
            datawriter_->write(&data);
        }

        virtual ~WriterWrapper()
        {
            participant_->delete_contained_entities();
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }

        void on_offered_deadline_missed(
                DataWriter* /* writer */,
                const OfferedDeadlineMissedStatus& /* status */) override
        {
            deadline_called_.store(true);
            cv_.notify_one();
        }

    protected:

        std::condition_variable& cv_;
        std::atomic_bool& deadline_called_;
        DomainParticipant* participant_;
        TypeSupport type_support_;
        Topic* topic_;
        Publisher* publisher_;
        DataWriter* datawriter_;
    };

    std::mutex mtx;
    std::condition_variable cv;
    std::atomic_bool deadline_called{false};
    std::unique_lock<std::mutex> lck(mtx);

    WriterWrapper writer_w(cv, deadline_called);

    auto ret = cv.wait_for(lck, std::chrono::seconds(1), [&]()
                    {
                        return deadline_called.load();
                    });
    ASSERT_TRUE(ret);
}

/**
 * Regression test for EasyRedmine issue https://eprosima.easyredmine.com/issues/20059
 *
 * The test creates a writer and reader that communicate with transient_local reliable QoS.
 * The issue corresponds to a race condition involving writer's history destruction and heartbeat delivery, so in order
 * to increment the probability of occurrence a high history depth and heartbeat frequency are used.
 *
 * Note:
 *   - Only affects TRANSPORT case (UDP or SHM communication, data_sharing and intraprocess disabled)
 *   - Destruction order matters: writer must be destroyed before reader (otherwise heartbeats would no be sent while
 *     destroying the writer)
 * Edit: this test has been updated to ensure that HistoryQoS and ResourceLimitQoS constraints are met (#20401).
 */
TEST(DDSDataWriter, HeartbeatWhileDestruction)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    // Force writer to be destroyed before reader, so they are still matched, and heartbeats are sent while writer is destroyed
    {
        PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

        // A high number of samples increases the probability of the data race to occur
        size_t n_samples = 1000;

        reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
                .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
                .init();
        ASSERT_TRUE(reader.isInitialized());

        writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
                .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
                .history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS)
                .history_depth(static_cast<int32_t>(n_samples))
                .resource_limits_max_samples(static_cast<int32_t>(n_samples))
                .resource_limits_max_instances(static_cast<int32_t>(1))
                .resource_limits_max_samples_per_instance(static_cast<int32_t>(n_samples))
                .heartbeat_period_seconds(0)
                .heartbeat_period_nanosec(20 * 1000)
                .init();
        ASSERT_TRUE(writer.isInitialized());

        reader.wait_discovery();
        writer.wait_discovery();

        auto data = default_helloworld_data_generator(n_samples);
        reader.startReception(data);
        writer.send(data);

        EXPECT_TRUE(data.empty());
    }
}

/**
 * This is a regression test for issue https://eprosima.easyredmine.com/issues/20504.
 * It checks that a DataWriter be created with default Qos and a large history depth.
 */
TEST(DDSDataWriter, default_qos_large_history_depth)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.history_depth(1000).init();
    ASSERT_TRUE(writer.isInitialized());
}

/**
 * Utility class to set some values other than default to those Qos common to Topic and DataWriter.
 *
 * This is a class instead of a free function to avoid linking with its TestsDataReader counterpart.
 */
class TestsDataWriterQosCommonUtils
{
public:

    template<typename T>
    static void set_common_qos(
            T& qos)
    {
        qos.durability_service().history_kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
        qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
        qos.durability().kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
        qos.deadline().period = {0, 500000000};
        qos.latency_budget().duration = 0;
        qos.liveliness().kind = eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
        qos.resource_limits().max_samples = 1000;
        qos.transport_priority().value = 1;
        qos.ownership().kind = eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS;
        qos.representation().m_value.push_back(eprosima::fastdds::dds::DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);
        qos.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
        qos.lifespan().duration = {5, 0};
    }

};

/*
 * This test:
 *   1. Creates a Topic with custom Qos
 *   2. Updates the default DataWriter Qos that are not in common with Topic Qos with non-default values
 *   3. Creates a DataWriter with DATAWRITER_QOS_USE_TOPIC_QOS
 *   4. Checks that the used Qos are the merge between the default ones and the Topic ones
 */
TEST(DDSDataWriter, datawriter_qos_use_topic_qos)
{
    using namespace eprosima::fastdds::dds;

    /* Create a topic with custom Qos */
    // Set Topic Qos different from default
    TopicQos topic_qos;
    TestsDataWriterQosCommonUtils::set_common_qos(topic_qos);

    // Create DomainParticipant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    /* Create a DataWriter with modified default Qos using the Topic Qos */
    // Create Topic
    TypeSupport type_support;
    type_support.reset(new HelloWorldPubSubType());
    type_support.register_type(participant, "HelloWorld");
    Topic* topic = participant->create_topic("HelloWorldTopic", "HelloWorld", topic_qos);

    // Create the Publisher
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    // Change default DataWriter Qos (only those that are different from Topic Qos)
    DataWriterQos control_qos;
    control_qos.ownership_strength().value = 1;
    control_qos.publish_mode().kind = eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE;
    control_qos.writer_data_lifecycle().autodispose_unregistered_instances = false;
    control_qos.user_data().push_back(0);
    control_qos.endpoint().entity_id = 1;
    control_qos.writer_resource_limits().matched_subscriber_allocation =
            ResourceLimitedContainerConfig::fixed_size_configuration(1u);
    control_qos.data_sharing().off();
    publisher->set_default_datawriter_qos(control_qos);

    // Create DataWriter with DATAREADER_QOS_USE_TOPIC_QOS
    DataWriter* writer = publisher->create_datawriter(topic, DATAWRITER_QOS_USE_TOPIC_QOS);
    ASSERT_NE(writer, nullptr);

    /* Check that used Qos are the merge between the default ones and the Topic ones */
    // Set the topic values on the control DataWriterQos
    TestsDataWriterQosCommonUtils::set_common_qos(control_qos);

    // Get used DataWriter Qos
    DataWriterQos test_qos = writer->get_qos();

    // Check that the Qos that are not in common with Topic Qos are correctly set as the default ones,
    // and that the rest of the Qos are left unmodified
    ASSERT_EQ(control_qos, test_qos);
}

bool validate_subscription_builtin_topic_data(
        const eprosima::fastdds::rtps::SubscriptionBuiltinTopicData& subdata,
        const eprosima::fastdds::dds::DataReader& datareader)
{
    bool ret = true;

    auto dr_qos = datareader.get_qos();
    auto sub_qos = datareader.get_subscriber()->get_qos();

    eprosima::fastdds::rtps::BuiltinTopicKey_t dr_key, part_key;

    entity_id_to_builtin_topic_key(dr_key, datareader.guid().entityId);
    guid_prefix_to_builtin_topic_key(part_key, datareader.get_subscriber()->get_participant()->guid().guidPrefix);

    ret &= (0 == memcmp(subdata.key.value, dr_key.value, sizeof(eprosima::fastdds::rtps::BuiltinTopicKey_t)));
    ret &=
            (0 ==
            memcmp(subdata.participant_key.value, part_key.value,
            sizeof(eprosima::fastdds::rtps::BuiltinTopicKey_t)));
    ret &= (subdata.topic_name == datareader.get_topicdescription()->get_name());
    ret &= (subdata.type_name == datareader.get_topicdescription()->get_type_name());

    // DataReader Qos
    ret &= (subdata.durability == dr_qos.durability());
    ret &= (subdata.deadline == dr_qos.deadline());
    ret &= (subdata.latency_budget == dr_qos.latency_budget());
    ret &= (subdata.liveliness == dr_qos.liveliness());
    ret &= (subdata.reliability == dr_qos.reliability());
    ret &= (subdata.ownership == dr_qos.ownership());
    ret &= (subdata.destination_order == dr_qos.destination_order());
    ret &= (
        (subdata.user_data.size() == dr_qos.user_data().size()) &&
        (0 == memcmp(subdata.user_data.data(), dr_qos.user_data().data(), subdata.user_data.size())));
    // time based filter not implemented

    // Subscriber Qos
    ret &= (subdata.presentation == sub_qos.presentation());
    ret &= (subdata.partition.getNames() == sub_qos.partition().getNames());
    // topic_data not implemented
    // group_data too

    return ret;
}

/**
 * @test DDS-DW-API-GMSD-01
 *
 * get_matched_subscription_data() must return RETCODE_BAD_PARAMETER
 * if the subscription is not matched.
 */
TEST(DDSDataWriter, datawriter_get_matched_subscription_data_bad_parameter)
{
    using InstanceHandle_t = eprosima::fastdds::rtps::InstanceHandle_t;

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_1(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_2(TEST_TOPIC_NAME);

    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData subdata;

    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .init();

    reader_1.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();
    reader_2.ownership_exclusive()
            .init();

    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader_1.isInitialized());
    ASSERT_TRUE(reader_2.isInitialized());

    // Writer should not be matched with any reader
    writer.wait_discovery(2, std::chrono::seconds(1));

    ASSERT_TRUE(!writer.is_matched());

    auto& native_writer = writer.get_native_writer();

    InstanceHandle_t r1_handle = reader_1.get_native_reader().get_instance_handle();
    ReturnCode_t ret = native_writer.get_matched_subscription_data(subdata, r1_handle);

    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);

    InstanceHandle_t r2_handle = reader_2.get_native_reader().get_instance_handle();
    ret = native_writer.get_matched_subscription_data(subdata, r2_handle);

    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_BAD_PARAMETER);
}

/**
 * @test DDS-DW-API-GMSD-02
 *
 * The operation must succeed when the subscription is matched and correctly
 * retrieve the publication data. Parameterize the test for different transports.
 */
TEST_P(DDSDataWriter, datawriter_get_matched_subscription_data_correctly_behaves)
{
    using InstanceHandle_t = eprosima::fastdds::rtps::InstanceHandle_t;

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_1(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_2(TEST_TOPIC_NAME);

    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData r1_subdata, r2_subdata;

    writer.partition("*")
            .init();

    reader_1.partition("*")
            .init();
    reader_2.user_data({'u', 's', 'e', 'r', 'd', 'a', 't', 'a'})
            .partition("*")
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();

    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader_1.isInitialized());
    ASSERT_TRUE(reader_2.isInitialized());

    // Writer must match with both readers
    writer.wait_discovery(2, std::chrono::seconds::zero());

    ASSERT_EQ(writer.get_matched(), 2u);

    auto& native_writer = writer.get_native_writer();

    InstanceHandle_t r1_handle = reader_1.get_native_reader().get_instance_handle();
    ReturnCode_t ret = native_writer.get_matched_subscription_data(r1_subdata, r1_handle);

    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_TRUE(validate_subscription_builtin_topic_data(r1_subdata, reader_1.get_native_reader()));

    InstanceHandle_t r2_handle = reader_2.get_native_reader().get_instance_handle();
    ret = native_writer.get_matched_subscription_data(r2_subdata, r2_handle);

    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_TRUE(validate_subscription_builtin_topic_data(r2_subdata, reader_2.get_native_reader()));
}

/**
 * @test DDS-DW-API-GMS-01
 *
 * get_matched_subscriptions() must return RETCODE_OK
 * with an empty list if no DataWriters are matched.
 */
TEST(DDSDataWriter, datawriter_get_matched_subscriptions_ok_empty_list)
{
    using InstanceHandle_t = eprosima::fastdds::rtps::InstanceHandle_t;

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_1(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_2(TEST_TOPIC_NAME);

    std::vector<InstanceHandle_t> sub_handles;

    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .init();

    reader_1.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();

    reader_2.ownership_exclusive()
            .init();

    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader_1.isInitialized());
    ASSERT_TRUE(reader_2.isInitialized());

    // Writer should not be matched with any reader
    writer.wait_discovery(2, std::chrono::seconds(2));
    ASSERT_FALSE(writer.is_matched());

    auto& native_writer = writer.get_native_writer();
    ReturnCode_t ret = native_writer.get_matched_subscriptions(sub_handles);

    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(sub_handles.size(), 0u);
}

/**
 * @test DDS-DW-API-GMS-02
 *
 * get_matched_subscriptions() must provide the correct list of matched subscription handles.
 * Parameterize the test for different transports.
 */
TEST_P(DDSDataWriter, datawriter_get_matched_subscriptions_correctly_behaves)
{
    using InstanceHandle_t = eprosima::fastdds::rtps::InstanceHandle_t;

    const size_t num_readers = 5;

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::vector<std::unique_ptr<PubSubReader<HelloWorldPubSubType>>> readers;
    std::vector<InstanceHandle_t> expected_sub_handles;
    std::vector<InstanceHandle_t> sub_handles;

    readers.reserve(num_readers);
    sub_handles.reserve(num_readers);

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    for (size_t i = 0; i < num_readers; ++i)
    {
        readers.emplace_back(new PubSubReader<HelloWorldPubSubType>(TEST_TOPIC_NAME));
        readers.back()->init();
        ASSERT_TRUE(readers.back()->isInitialized());
        expected_sub_handles.emplace_back(readers.back()->get_native_reader().get_instance_handle());
    }

    // Wait for discovery
    writer.wait_discovery(num_readers, std::chrono::seconds::zero());
    ASSERT_EQ(writer.get_matched(), num_readers);

    auto& native_writer = writer.get_native_writer();
    ReturnCode_t ret = native_writer.get_matched_subscriptions(sub_handles);

    // Check that the list of matched publication handles is correct
    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(sub_handles.size(), num_readers);
    ASSERT_TRUE(std::is_permutation(sub_handles.begin(), sub_handles.end(), expected_sub_handles.begin()));

    // Remove two readers and check that the list of matched publication handles is updated
    readers.pop_back();
    readers.pop_back();
    expected_sub_handles.pop_back();
    expected_sub_handles.pop_back();

    // Wait for undiscovery
    writer.wait_reader_undiscovery(static_cast<unsigned int>(num_readers - 2));

    sub_handles.clear();
    ret = native_writer.get_matched_subscriptions(sub_handles);
    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(sub_handles.size(), static_cast<size_t>(num_readers - 2));
    ASSERT_TRUE(std::is_permutation(sub_handles.begin(), sub_handles.end(), expected_sub_handles.begin()));
}

/**
 * @test DDS-DW-API-GMS-03
 *
 * The operation must provide the correct list of matched subscription handles in multiple
 * participants scenario. Parameterize the test for different transports.
 */
TEST_P(DDSDataWriter, datawriter_get_matched_subscriptions_multiple_participants_correctly_behave)
{
    using InstanceHandle_t = eprosima::fastdds::rtps::InstanceHandle_t;

    PubSubParticipant<HelloWorldPubSubType> part_1(1, 1, 1, 1);
    PubSubParticipant<HelloWorldPubSubType> part_2(1, 1, 1, 1);

    part_1.pub_topic_name(TEST_TOPIC_NAME);
    part_1.sub_topic_name(TEST_TOPIC_NAME + "_1");
    part_2.pub_topic_name(TEST_TOPIC_NAME + "_1");
    part_2.sub_topic_name(TEST_TOPIC_NAME);

    ASSERT_TRUE(part_1.init_participant());
    ASSERT_TRUE(part_1.init_publisher(0));
    ASSERT_TRUE(part_1.init_subscriber(0));

    ASSERT_TRUE(part_2.init_participant());
    ASSERT_TRUE(part_2.init_subscriber(0));
    ASSERT_TRUE(part_2.init_publisher(0));

    part_1.pub_wait_discovery();
    part_1.sub_wait_discovery();

    part_2.pub_wait_discovery();
    part_2.sub_wait_discovery();

    auto& writer_p1 = part_1.get_native_writer(0);
    auto& writer_p2 = part_2.get_native_writer(0);

    std::vector<InstanceHandle_t> sub_handles_p1;
    std::vector<InstanceHandle_t> sub_handles_p2;

    ReturnCode_t ret = writer_p1.get_matched_subscriptions(sub_handles_p1);
    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(sub_handles_p1.size(), 1u);
    ASSERT_EQ(sub_handles_p1[0], part_2.get_native_reader(0).get_instance_handle());

    ret = writer_p2.get_matched_subscriptions(sub_handles_p2);
    ASSERT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);
    ASSERT_EQ(sub_handles_p2.size(), 1u);
    ASSERT_EQ(sub_handles_p2[0], part_1.get_native_reader(0).get_instance_handle());
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(DDSDataWriter,
        DDSDataWriter,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<DDSDataWriter::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case DATASHARING:
                    return "Datasharing";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }

        });
