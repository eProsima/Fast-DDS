/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <gtest/gtest.h>

#include <fastdds/dds/core/condition/GuardCondition.hpp>
#include <fastdds/dds/core/condition/WaitSet.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/rtps/transport/low-bandwidth/PayloadCompressionTransportDescriptor.hpp>
#include <fastdds/rtps/transport/low-bandwidth/HeaderReductionTransportDescriptor.hpp>
#include <fastdds/rtps/transport/low-bandwidth/SourceTimestampTransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>

#include <thread>

#include <HelloWorldPubSubTypes.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

#ifdef FASTDDS_CHECK_LICENSE

std::string get_source_directory()
{
    std::string file_path = __FILE__;  // Path to the current .cpp file
#ifdef _WIN32
    const char separator = '\\';
#else
    const char separator = '/';
#endif // ifdef _WIN32

    size_t pos = file_path.find_last_of(separator);
    if (pos == std::string::npos)
    {
        return ".";  // no directory part found, return current dir
    }
    return file_path.substr(0, pos);
}

void publisher_thread(
        DataWriter* writer,
        int sleep_time)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for (uint16_t i = 0; i < 10; ++i)
    {
        HelloWorld hello;
        hello.index(i);
        hello.message("HelloWorld from License test");

        if (writer->write(&hello) != RETCODE_OK)
        {
            std::cout << "License running time reached. Participant closed." << std::endl;
            return;
        }

        std::cout << "Message: '" << hello.message() << "' with index: '" << hello.index() << "' SENT" << std::endl;
        if (sleep_time > 1000)
        {
            std::cout << "Sleeping for " << sleep_time << "ms." << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    }

    return;
}

/*!
 * This test checks that the license verification work as expected.
 * It creates a publisher and two subscribers.
 * It sends 10 samples from the publisher and checks that the subscriber receives them all.
 */
void common_test(
        std::string test_name,
        bool should_fail)
{
    std::string curr_folder = get_source_directory();
    /* Set environment variable */
#ifdef _WIN32
    std::string new_env_path = (curr_folder + "\\tests\\" + test_name);
    _putenv_s("FASTDDSHOME", new_env_path.c_str());
#else
    std::string new_env_path = (curr_folder + "/tests/" + test_name);
    setenv("FASTDDSHOME", new_env_path.c_str(), 1);
#endif // ifdef _WIN32

    //copy_tests_license_files(fastddshome_path, test_name);
    std::cout << test_name << "\n";

    WaitSet wait_set;

    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    DomainParticipant* participant_pub = factory->create_participant_with_default_profile(nullptr, StatusMask::none());
    if (should_fail)
    {
        ASSERT_EQ(participant_pub, nullptr);
        return;
    }
    else
    {
        ASSERT_NE(participant_pub, nullptr);
    }

    DomainParticipant* participant_sub = factory->create_participant_with_default_profile(nullptr, StatusMask::none());
    ASSERT_NE(participant_sub, nullptr) << "Subscriber participant initialization failed";

    TypeSupport type(new HelloWorldPubSubType());
    type.register_type(participant_pub);
    type.register_type(participant_sub);

    PublisherQos pub_qos = PUBLISHER_QOS_DEFAULT;
    participant_pub->get_default_publisher_qos(pub_qos);
    Publisher* publisher = participant_pub->create_publisher(pub_qos, nullptr, StatusMask::none());
    ASSERT_NE (publisher, nullptr) << "Publisher initialization failed";

    SubscriberQos sub_qos = SUBSCRIBER_QOS_DEFAULT;
    participant_sub->get_default_subscriber_qos(sub_qos);
    Subscriber* subscriber = participant_sub->create_subscriber(sub_qos, nullptr, StatusMask::none());
    ASSERT_NE(subscriber, nullptr) << "Subscriber initialization failed";

    // Create the topic
    TopicQos topic_pub_qos = TOPIC_QOS_DEFAULT;
    participant_pub->get_default_topic_qos(topic_pub_qos);
    Topic* topic_pub = participant_pub->create_topic("HelloWorldTopic", type.get_type_name(), topic_pub_qos);
    ASSERT_NE(topic_pub, nullptr) << "Topic publication initialization failed";

    TopicQos topic_sub_qos = TOPIC_QOS_DEFAULT;
    participant_sub->get_default_topic_qos(topic_sub_qos);
    Topic* topic_sub = participant_sub->create_topic("HelloWorldTopic", type.get_type_name(), topic_sub_qos);
    ASSERT_NE(topic_sub, nullptr) << "Topic subscription initialization failed";

    // Create the data writer
    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.history().depth = 5;
    publisher->get_default_datawriter_qos(writer_qos);
    DataWriter* writer = publisher->create_datawriter(topic_pub, writer_qos);
    ASSERT_NE(writer, nullptr) << "DataWriter initialization failed";

    // Create the data writer
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.history().depth = 5;
    subscriber->get_default_datareader_qos(reader_qos);
    DataReader* reader = subscriber->create_datareader(topic_sub, reader_qos);
    ASSERT_NE(reader, nullptr) << "DataWriter initialization failed";

    std::thread thread_pub(publisher_thread, writer, (test_name == "test_license_valid_one_min" ? 10000 : 10));

    int num_received_samples = 0;
    while (10 > num_received_samples)
    {
        SampleInfo info;
        HelloWorld hello;
        if (RETCODE_OK == reader->take_next_sample(&hello, &info))
        {
            if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
            {
                num_received_samples++;
            }
        }
    }

    thread_pub.join();

    if (test_name == "test_license_valid_one_min")
    {
        EXPECT_EQ(num_received_samples, 5);
    }
    else
    {
        EXPECT_EQ(num_received_samples, 10);
    }

    participant_sub->delete_contained_entities();
    ASSERT_EQ(RETCODE_OK, DomainParticipantFactory::get_instance()->delete_participant(participant_sub));

    participant_pub->delete_contained_entities();
    ASSERT_EQ(RETCODE_OK, DomainParticipantFactory::get_instance()->delete_participant(participant_pub));
}

TEST(LicensesTests, test_license_valid)
{
    common_test("test_license_valid", false);
}

TEST(LicensesTests, test_license_valid_no_time)
{
    common_test("test_license_valid_no_time", false);
}

TEST(LicensesTests, test_license_valid_one_min)
{
    common_test("test_license_valid_one_min", false);
}

TEST(LicensesTests, test_license_missing_id)
{
    common_test("test_license_missing_id", true);
}

TEST(LicensesTests, test_license_missing_version)
{
    common_test("test_license_missing_version", true);
}

TEST(LicensesTests, test_license_missing_holder)
{
    common_test("test_license_missing_holder", true);
}

TEST(LicensesTests, test_license_missing_issued)
{
    common_test("test_license_missing_issued", true);
}

TEST(LicensesTests, test_license_missing_not_before)
{
    common_test("test_license_missing_not_before", true);
}

TEST(LicensesTests, test_license_missing_expires_at)
{
    common_test("test_license_missing_expires_at", true);
}

TEST(LicensesTests, test_license_missing_products)
{
    common_test("test_license_missing_products", true);
}

TEST(LicensesTests, test_license_missing_features)
{
    common_test("test_license_missing_features", true);
}

TEST(LicensesTests, test_license_missing_usage)
{
    common_test("test_license_missing_usage", true);
}

TEST(LicensesTests, test_license_invalid_expired)
{
    common_test("test_license_invalid_expired", true);
}

TEST(LicensesTests, test_license_invalid_other_product)
{
    common_test("test_license_invalid_other_product", true);
}

TEST(LicensesTests, test_license_invalid_future)
{
    common_test("test_license_invalid_future", true);
}

TEST(LicensesTests, test_invalid_sign)
{
    common_test("test_invalid_sign", true);
}

TEST(LicensesTests, test_license_no_json)
{
    common_test("test_license_no_json", true);
}

TEST(LicensesTests, test_license_no_sig)
{
    common_test("test_license_no_sig", true);
}

TEST(LicensesTests, test_no_license_file)
{
    common_test("test_no_license_file", true);
}

TEST(LicensesTests, test_no_license_folder)
{
    common_test("test_no_license_folder", true);
}

TEST(LicensesTests, test_empty_license_file)
{
    common_test("test_empty_license_file", true);
}

#endif // ifdef FASTDDS_CHECK_LICENSE

int main(
        int argc,
        char** argv)
{
    //Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
