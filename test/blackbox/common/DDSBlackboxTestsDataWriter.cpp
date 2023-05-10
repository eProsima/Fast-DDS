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

#include "BlackboxTests.hpp"

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <gtest/gtest.h>

#include <fastrtps/attributes/LibrarySettingsAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <rtps/transport/test_UDPv4Transport.h>

using namespace eprosima::fastrtps;
using test_UDPv4Transport = eprosima::fastdds::rtps::test_UDPv4Transport;
using test_UDPv4TransportDescriptor = eprosima::fastdds::rtps::test_UDPv4TransportDescriptor;

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
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
                xmlparser::XMLProfileManager::library_settings(library_settings);
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
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
                xmlparser::XMLProfileManager::library_settings(library_settings);
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

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();

    writer.disable_builtin_transport().add_user_transport_to_pparams(testTransport).init();
    ASSERT_TRUE(writer.isInitialized());

    reader.reliability(RELIABLE_RELIABILITY_QOS).init();
    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    // Disable communication to prevent reception of ACKs
    test_UDPv4Transport::test_UDPv4Transport_ShutdownAllNetwork = true;

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
    test_UDPv4Transport::test_UDPv4Transport_ShutdownAllNetwork = false;

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
    eprosima::fastrtps::rtps::InstanceHandle_t wrong_handle;
    wrong_handle.value[0] = 0xee;
    eprosima::fastrtps::rtps::InstanceHandle_t valid_handle;
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
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, datawriter->get_key_value(nullptr, wrong_handle));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(nullptr, wrong_handle));

    // 2. Check HANDLE_NIL
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, datawriter->get_key_value(&data, HANDLE_NIL));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(&data, HANDLE_NIL));

    // 3. Check type should have keys
    EXPECT_EQ(ReturnCode_t::RETCODE_ILLEGAL_OPERATION, datawriter->get_key_value(&data, wrong_handle));

    // 4. Calling get_key_value with a key not yet registered returns RETCODE_BAD_PARAMETER
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(&data, wrong_handle));

    // 5. Calling get_key_value on a registered instance should work.
    valid_handle = instance_datawriter->register_instance(&valid_data);
    EXPECT_NE(HANDLE_NIL, valid_handle);
    data.key(0);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->get_key_value(&data, valid_handle));
    EXPECT_EQ(valid_data.key(), data.key());

    // 6. Calling get_key_value on an unregistered instance should return RETCODE_BAD_PARAMETER.
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->unregister_instance(&valid_data, valid_handle));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(&data, valid_handle));

    // 7. Calling get_key_value with a valid instance should work
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->write(&valid_data, HANDLE_NIL));
    data.key(0);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->get_key_value(&data, valid_handle));
    EXPECT_EQ(valid_data.key(), data.key());

    // 8. Calling get_key_value on a disposed instance should work.
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->dispose(&valid_data, valid_handle));
    data.key(0);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->get_key_value(&data, valid_handle));
    EXPECT_EQ(valid_data.key(), data.key());
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
