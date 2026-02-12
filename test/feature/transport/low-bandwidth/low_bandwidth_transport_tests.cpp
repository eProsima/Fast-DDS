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

#include <HelloWorldPubSubTypes.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

/*!
 * This test checks that the low bandwidth transports work as expected.
 * It creates a publisher and two subscribers, one with the low bandwidth transport and another one with a standard UDPv4 transport.
 * It sends 10 samples from the publisher and checks that the subscriber with the low bandwidth transport receives them all.
 * The other subscriber is used to check that the low bandwidth transport does not interfere with normal communication.
 */
void common_test(
        const DomainParticipantQos& participant_qos,
        std::string xml_file)
{
    WaitSet wait_set;

    DomainParticipantFactory::get_instance()->load_XML_profiles_file(xml_file.c_str());

    DomainParticipant* participant_pub {DomainParticipantFactory::get_instance()->create_participant(0,
                                                participant_qos)};
    ASSERT_NE(nullptr, participant_pub);

    DomainParticipant* participant_sub {DomainParticipantFactory::get_instance()->create_participant_with_profile(0,
                                                "ParticipantTest")};
    ASSERT_NE(nullptr, participant_sub);

    DomainParticipant* participant_sub2 {DomainParticipantFactory::get_instance()->create_participant(0,
                                                 PARTICIPANT_QOS_DEFAULT)};
    ASSERT_NE(nullptr, participant_sub2);

    TypeSupport type(new HelloWorldPubSubType());
    type.register_type(participant_pub);
    type.register_type(participant_sub);
    type.register_type(participant_sub2);

    Publisher* publisher {participant_pub->create_publisher(PUBLISHER_QOS_DEFAULT)};
    ASSERT_NE(nullptr, publisher);

    Subscriber* subscriber {participant_sub->create_subscriber(SUBSCRIBER_QOS_DEFAULT)};
    ASSERT_NE(nullptr, subscriber);

    Subscriber* subscriber2 {participant_sub2->create_subscriber(SUBSCRIBER_QOS_DEFAULT)};
    ASSERT_NE(nullptr, subscriber2);

    Topic* topic_pub {participant_pub->create_topic("HelloWorldLowBandwidthTopic",
                              type.get_type_name(), TOPIC_QOS_DEFAULT)};
    ASSERT_NE(nullptr, topic_pub);

    Topic* topic_sub {participant_sub->create_topic("HelloWorldLowBandwidthTopic",
                              type.get_type_name(), TOPIC_QOS_DEFAULT)};
    ASSERT_NE(nullptr, topic_sub);

    Topic* topic_sub2 {participant_sub2->create_topic("HelloWorldLowBandwidthTopic",
                               type.get_type_name(), TOPIC_QOS_DEFAULT)};
    ASSERT_NE(nullptr, topic_sub2);

    DataWriterQos writer_qos;
    writer_qos.history().depth = 10;
    DataWriter* writer {publisher->create_datawriter(topic_pub, writer_qos)};
    ASSERT_NE(nullptr, writer);

    DataReaderQos reader_qos;
    reader_qos.history().depth = 10;
    DataReader* reader {subscriber->create_datareader(topic_sub, reader_qos, nullptr, StatusMask::all())};
    ASSERT_NE(nullptr, reader);

    DataReader* reader2 {subscriber2->create_datareader(topic_sub2, reader_qos, nullptr, StatusMask::all())};
    ASSERT_NE(nullptr, reader2);

    wait_set.attach_condition(reader->get_statuscondition());
    wait_set.attach_condition(reader2->get_statuscondition());

    for (uint16_t i = 0; i < 10; ++i)
    {
        HelloWorld hello;
        hello.index(i);
        hello.message("HelloWorld from Low Bandwidth Transport test");
        EXPECT_EQ(RETCODE_OK, writer->write(&hello));
        std::cout << "Message: '" << hello.message() << "' with index: '" << hello.index() << "' SENT" << std::endl;
    }

    ConditionSeq triggered_conditions;
    uint32_t num_received_samples {0};

    while (10 > num_received_samples)
    {
        ReturnCode_t ret_code = wait_set.wait(triggered_conditions, {10, 0});
        if (RETCODE_OK !=  ret_code)
        {
            break;
        }
        for (Condition* cond : triggered_conditions)
        {
            StatusCondition* status_cond {dynamic_cast<StatusCondition*>(cond)};
            ASSERT_NE(nullptr, status_cond);
            Entity* entity = status_cond->get_entity();
            ASSERT_NE(entity, reader2);
            if (entity == reader2)
            {
                break;
            }
            StatusMask changed_statuses = entity->get_status_changes();
            if (changed_statuses.is_active(StatusMask::subscription_matched()))
            {
                SubscriptionMatchedStatus status;
                reader->get_subscription_matched_status(status);
                std::cout << "Waitset " << status.current_count_change << " subscriber matched." << std::endl;
            }
            if (changed_statuses.is_active(StatusMask::data_available()))
            {
                SampleInfo info;
                HelloWorld hello;
                while (RETCODE_OK == reader->take_next_sample(&hello, &info))
                {
                    if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
                    {
                        num_received_samples++;
                        // Print Hello world message data
                        std::cout << "Message: '" << hello.message() << "' with index: '"
                                  << hello.index() << "' RECEIVED" << std::endl;
                    }
                }
            }
        }
    }

    EXPECT_EQ(10u, num_received_samples);

    participant_sub2->delete_contained_entities();
    ASSERT_EQ(RETCODE_OK, DomainParticipantFactory::get_instance()->delete_participant(participant_sub2));

    participant_sub->delete_contained_entities();
    ASSERT_EQ(RETCODE_OK, DomainParticipantFactory::get_instance()->delete_participant(participant_sub));

    participant_pub->delete_contained_entities();
    ASSERT_EQ(RETCODE_OK, DomainParticipantFactory::get_instance()->delete_participant(participant_pub));
}

#if HAVE_ZLIB || HAVE_BZIP2

TEST(LowBandwidthTransportTests, payload_compression)
{
    DomainParticipantQos participant_qos;

    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    auto compress_transport =
            std::make_shared<PayloadCompressionTransportDescriptor>(udp_transport);
    participant_qos.properties().properties().emplace_back(Property(
                "rtps.payload_compression.compression_library",
                "AUTOMATIC"));

    participant_qos.transport().use_builtin_transports = false;
    participant_qos.transport().user_transports.push_back(compress_transport);

    common_test(participant_qos, "payload_compression.xml");
}

#endif // HAVE_ZLIB || HAVE_BZIP2

TEST(LowBandwidthTransportTests, header_reduction)
{
    DomainParticipantQos participant_qos;

    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    auto header_reduction_transport = std::make_shared<HeaderReductionTransportDescriptor>(
        udp_transport);
    participant_qos.properties().properties().emplace_back(Property(
                "rtps.header_reduction.remove_version", "true"));
    participant_qos.properties().properties().emplace_back(Property(
                "rtps.header_reduction.remove_vendor_id", "true"));
    participant_qos.properties().properties().emplace_back(Property(
                "rtps.header_reduction.submessage.combine_id_and_flags",
                "true"));
    participant_qos.properties().properties().emplace_back(Property(
                "rtps.header_reduction.submessage.compress_entitiy_ids",
                "16,16"));

    participant_qos.transport().use_builtin_transports = false;
    participant_qos.transport().user_transports.push_back(header_reduction_transport);

    common_test(participant_qos, "header_reduction.xml");
}


TEST(LowBandwidthTransportTests, sourcetimestamp)
{
    DomainParticipantQos participant_qos;

    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    auto sourcetimestamp_transport = std::make_shared<SourceTimestampTransportDescriptor>(
        udp_transport);

    participant_qos.transport().use_builtin_transports = false;
    participant_qos.transport().user_transports.push_back(sourcetimestamp_transport);

    common_test(participant_qos, "source_timestamp.xml");
}

#if HAVE_ZLIB || HAVE_BZIP2

TEST(LowBandwidthTransportTests, all)
{
    DomainParticipantQos participant_qos;

    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    auto sourcetimestamp_transport = std::make_shared<SourceTimestampTransportDescriptor>(
        udp_transport);

    auto compress_transport =
            std::make_shared<PayloadCompressionTransportDescriptor>(sourcetimestamp_transport);
    participant_qos.properties().properties().emplace_back(Property(
                "rtps.payload_compression.compression_library",
                "AUTOMATIC"));

    auto header_reduction_transport = std::make_shared<HeaderReductionTransportDescriptor>(
        compress_transport);
    participant_qos.properties().properties().emplace_back(Property(
                "rtps.header_reduction.remove_version", "true"));
    participant_qos.properties().properties().emplace_back(Property(
                "rtps.header_reduction.remove_vendor_id", "true"));
    participant_qos.properties().properties().emplace_back(Property(
                "rtps.header_reduction.submessage.combine_id_and_flags",
                "true"));
    participant_qos.properties().properties().emplace_back(Property(
                "rtps.header_reduction.submessage.compress_entitiy_ids",
                "16,16"));

    participant_qos.transport().use_builtin_transports = false;
    participant_qos.transport().user_transports.push_back(header_reduction_transport);

    common_test(participant_qos, "all_low_transports.xml");
}

#endif // HAVE_ZLIB || HAVE_BZIP2

int main(
        int argc,
        char** argv)
{
    //Log::SetVerbosity(Log::Info);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
