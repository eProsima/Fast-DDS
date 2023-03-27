// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <condition_variable>
#include <mutex>
#include <thread>

#include <gtest/gtest.h>

#include <fastdds/core/policy/ParameterSerializer.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
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
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastrtps/transport/test_UDPv4TransportDescriptor.h>
#include <fastrtps/types/TypesBase.h>

#include "BlackboxTests.hpp"
#include "../api/dds-pim/PubSubWriter.hpp"
#include "../api/dds-pim/PubSubReader.hpp"
#include "../types/FixedSized.h"
#include "../types/FixedSizedPubSubTypes.h"
#include "../types/HelloWorldPubSubTypes.h"

namespace eprosima {
namespace fastdds {
namespace dds {

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

/**
 * This test checks whether it is safe to delete not enabled DDS entities *
 */
TEST(DDSBasic, DeleteDisabledEntities)
{
    // Set DomainParticipantFactory to create disabled entities
    DomainParticipantFactoryQos factory_qos;
    factory_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(nullptr, factory);
    factory->set_qos(factory_qos);
    DomainParticipantFactoryQos factory_qos_check;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, factory->get_qos(factory_qos_check));
    ASSERT_EQ(false, factory_qos_check.entity_factory().autoenable_created_entities);

    // Create a disabled DomainParticipant, setting it to in turn create disable entities
    DomainParticipantQos participant_qos;
    participant_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipant* participant = factory->create_participant((uint32_t)GET_PID() % 230, participant_qos);
    ASSERT_NE(nullptr, participant);
    DomainParticipantQos participant_qos_check;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, participant->get_qos(participant_qos_check));
    ASSERT_EQ(false, participant_qos_check.entity_factory().autoenable_created_entities);

    // Create a disabled Publisher, setting it to in turn create disable entities
    PublisherQos publisher_qos;
    publisher_qos.entity_factory().autoenable_created_entities = false;
    Publisher* publisher = participant->create_publisher(publisher_qos);
    ASSERT_NE(nullptr, publisher);
    PublisherQos publisher_qos_check;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, publisher->get_qos(publisher_qos_check));
    ASSERT_EQ(false, publisher_qos_check.entity_factory().autoenable_created_entities);

    // Create a disabled Subscriber, setting it to in turn create disable entities
    SubscriberQos subscriber_qos;
    subscriber_qos.entity_factory().autoenable_created_entities = false;
    Subscriber* subscriber = participant->create_subscriber(subscriber_qos);
    ASSERT_NE(nullptr, subscriber);
    SubscriberQos subscriber_qos_check;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, subscriber->get_qos(subscriber_qos_check));
    ASSERT_EQ(false, subscriber_qos_check.entity_factory().autoenable_created_entities);

    // Register type
    TypeSupport type_support;
    type_support.reset(new HelloWorldPubSubType());
    type_support.register_type(participant);
    ASSERT_NE(nullptr, type_support);

    // Create Topic
    Topic* topic = participant->create_topic("HelloWorld", type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(nullptr, topic);

    // Create a disabled DataWriter
    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(nullptr, datawriter);

    // Create a disabled DataReader
    DataReader* datareader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(nullptr, datareader);

    // Delete all entities
    publisher->delete_datawriter(datawriter);
    subscriber->delete_datareader(datareader);
    participant->delete_publisher(publisher);
    participant->delete_subscriber(subscriber);
    participant->delete_topic(topic);
    factory->delete_participant(participant);
}

/**
 * This test checks a race condition when calling DomainParticipantImpl::create_instance_handle()
 * from different threads simultaneously. This was resulting in a `free(): invalid pointer` crash
 * when deleting publishers created this way, as there was a clash in their respective instance
 * handles. Not only did the crash occur, but it was also reported by TSan.
 *
 * The test spawns 200 threads, each creating a publisher and then waiting on a command from the
 * main thread to delete them (so all of them at deleted at the same time).
 */
TEST(DDSBasic, MultithreadedPublisherCreation)
{
    /* Get factory */
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(nullptr, factory);

    /* Create participant */
    DomainParticipant* participant = factory->create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant);

    /* Test synchronization variables */
    std::mutex finish_mtx;
    std::condition_variable finish_cv;
    bool should_finish = false;

    /* Function to create publishers, deleting them on command */
    auto thread_run =
            [participant, &finish_mtx, &finish_cv, &should_finish]()
            {
                /* Create publisher */
                Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
                ASSERT_NE(nullptr, publisher);

                {
                    /* Wait for test completion request */
                    std::unique_lock<std::mutex> lock(finish_mtx);
                    finish_cv.wait(lock, [&should_finish]()
                            {
                                return should_finish;
                            });
                }

                /* Delete publisher */
                ASSERT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_publisher(publisher));
            };

    {
        /* Create threads */
        std::vector<std::thread> threads;
        for (size_t i = 0; i < 200; i++)
        {
            threads.push_back(std::thread(thread_run));
        }

        /* Command threads to delete their publishers */
        {
            std::lock_guard<std::mutex> guard(finish_mtx);
            should_finish = true;
            finish_cv.notify_all();
        }

        /* Wait for all threads to join */
        for (std::thread& thr : threads)
        {
            thr.join();
        }
    }

    /* Clean up */
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, factory->delete_participant(participant));
}

TEST(DDSBasic, MultithreadedReaderCreationDoesNotDeadlock)
{
    // Get factory
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(nullptr, factory);

    // Create participant
    DomainParticipant* participant = factory->create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant);

    // Register type
    TypeSupport type_support;
    type_support.reset(new FixedSizedPubSubType());
    type_support.register_type(participant);
    ASSERT_NE(nullptr, type_support);

    // Create subscriber
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber);

    // Create publisher
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, publisher);

    // Create Topic
    Topic* topic = participant->create_topic(TEST_TOPIC_NAME, type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(nullptr, topic);

    // Set QoS
    DataSharingQosPolicy dsp;
    dsp.off();

    DataWriterQos dw_qos;
    DataReaderQos dr_qos;
    dw_qos.data_sharing(dsp);
    dr_qos.data_sharing(dsp);

    // Create DataWriter
    DataWriter* writer = publisher->create_datawriter(topic, dw_qos);
    ASSERT_NE(nullptr, writer);

    std::mutex mtx;
    std::condition_variable cv;
    bool should_finish = false;

    auto thread_run = [subscriber, topic, &mtx, &cv, &should_finish, &dr_qos]()
            {
                // Create reader
                DataReader* reader = subscriber->create_datareader(topic, dr_qos);
                ASSERT_NE(nullptr, reader);

                // Wait for test completion request
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [&should_finish]()
                        {
                            return should_finish;
                        });

                ASSERT_EQ(ReturnCode_t::RETCODE_OK, subscriber->delete_datareader(reader));
            };

    {
        std::vector<std::thread> threads;
        for (size_t i = 0; i < 10; ++i)
        {
            threads.push_back(std::thread(thread_run));
        }

        {
            std::lock_guard<std::mutex> guard(mtx);
            should_finish = true;
            cv.notify_all();
        }

        for (std::thread& thr : threads)
        {
            thr.join();
        }
    }

    ASSERT_EQ(ReturnCode_t::RETCODE_OK, publisher->delete_datawriter(writer));
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_publisher(publisher));
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_subscriber(subscriber));
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_topic(topic));
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, factory->delete_participant(participant));
}

/**
 * Read a parameterList from a CDRMessage.
 * Search for PID_CUSTOM_RELATED_SAMPLE_IDENTITY and PID_RELATED_SAMPLE_IDENTITY.
 * Overwrite PID_CUSTOM_RELATED_SAMPLE_IDENTITY to just leave the new one in msg.
 * @param[in] msg Reference to the message.
 * @param[out] exists_pid_related_sample_identity True if the parameter is inside msg.
 * @param[out] exists_pid_custom_related_sample_identity True if the parameter is inside msg.
 * @return true if parsing was correct, false otherwise.
 */
bool check_related_sample_identity_field(
        fastrtps::rtps::CDRMessage_t& msg,
        bool& exists_pid_related_sample_identity,
        bool& exists_pid_custom_related_sample_identity)
{
    uint32_t qos_size = 0;

    auto parameter_process = [&](
        fastrtps::rtps::CDRMessage_t* msg,
        ParameterId_t& pid,
        uint16_t plength,
        uint32_t& pid_pos)
            {
                switch (pid)
                {
                    case PID_CUSTOM_RELATED_SAMPLE_IDENTITY:
                    {
                        if (plength >= 24)
                        {
                            ParameterSampleIdentity_t p(pid, plength);
                            if (!fastdds::dds::ParameterSerializer<ParameterSampleIdentity_t>::read_from_cdr_message(p,
                                    msg, plength))
                            {
                                return false;
                            }
                            exists_pid_custom_related_sample_identity = true;
                            // Invalid assignment to overwrite parameter, in order to just send the standard PID_RELATED_SAMPLE_IDENTITY
                            msg->buffer[pid_pos] = 0xff;
                            msg->buffer[pid_pos + 1] = 0xff;
                        }
                        break;
                    }
                    case PID_RELATED_SAMPLE_IDENTITY:
                    {
                        if (plength >= 24)
                        {
                            ParameterSampleIdentity_t p(pid, plength);
                            if (!fastdds::dds::ParameterSerializer<ParameterSampleIdentity_t>::read_from_cdr_message(p,
                                    msg, plength))
                            {
                                return false;
                            }
                            exists_pid_related_sample_identity = true;
                        }
                        break;
                    }

                    default:
                        break;
                }
                return true;
            };

    uint32_t original_pos = msg.pos;
    bool is_sentinel = false;
    while (!is_sentinel)
    {
        msg.pos = original_pos + qos_size;

        ParameterId_t pid{PID_SENTINEL};
        uint16_t plength = 0;
        bool valid = true;
        auto msg_pid_pos = msg.pos;
        valid &= fastrtps::rtps::CDRMessage::readUInt16(&msg, (uint16_t*)&pid);
        valid &= fastrtps::rtps::CDRMessage::readUInt16(&msg, &plength);

        if (pid == PID_SENTINEL)
        {
            // PID_SENTINEL is always considered of length 0
            plength = 0;
            is_sentinel = true;
        }

        qos_size += (4 + plength);

        // Align to 4 byte boundary and prepare for next iteration
        qos_size = (qos_size + 3) & ~3;

        if (!valid || ((msg.pos + plength) > msg.length))
        {
            return false;
        }
        else if (!is_sentinel)
        {
            if (!parameter_process(&msg, pid, plength, msg_pid_pos))
            {
                return false;
            }
        }
    }
    return true;
}

/**
 * This test checks that PID_RELATED_SAMPLE_IDENTITY and
 * PID_CUSTOM_RELATED_SAMPLE_IDENTITY are being sent as parameter,
 * and that the new PID_RELATED_SAMPLE_IDENTITY is being properly
 * interpreted.
 * Inside the transport filter, both PIDs are indentified, and the old PID is overwritten.
 * Reader only receives the new PID, and identifies the related sample identity.
 */
TEST(DDSBasic, PidRelatedSampleIdentity)
{
    PubSubWriter<HelloWorldPubSubType> reliable_writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reliable_reader(TEST_TOPIC_NAME);

    // Test transport will be used in order to filter inlineQoS
    auto test_transport = std::make_shared<eprosima::fastrtps::rtps::test_UDPv4TransportDescriptor>();
    bool exists_pid_related_sample_identity = false;
    bool exists_pid_custom_related_sample_identity = false;

    test_transport->drop_data_messages_filter_ =
            [&exists_pid_related_sample_identity, &exists_pid_custom_related_sample_identity]
                (eprosima::fastrtps::rtps::CDRMessage_t& msg)-> bool
            {
                // Inside this filter, the two flags passed in register whether both PIDs are included in the msg to be sent.
                // The legacy value is overwritten in order to send a msg with only the standard PID_RELATED_SAMPLE_IDENTITY as valid parameter,
                // so that the reader will only parse that one.
                bool ret = check_related_sample_identity_field(msg, exists_pid_related_sample_identity,
                                exists_pid_custom_related_sample_identity);
                EXPECT_TRUE(ret);
                return false;
            };

    reliable_writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(test_transport)
            .init();
    ASSERT_TRUE(reliable_writer.isInitialized());

    reliable_reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(test_transport)
            .init();
    ASSERT_TRUE(reliable_reader.isInitialized());

    reliable_writer.wait_discovery();
    reliable_reader.wait_discovery();

    DataWriter& native_writer = reliable_writer.get_native_writer();

    HelloWorld data;
    // Send reply associating it with the client request.
    eprosima::fastrtps::rtps::WriteParams write_params;
    eprosima::fastrtps::rtps::SampleIdentity related_sample_identity_;
    eprosima::fastrtps::rtps::GUID_t unknown_guid;
    related_sample_identity_.writer_guid(unknown_guid);
    eprosima::fastrtps::rtps::SequenceNumber_t seq(51, 24);
    related_sample_identity_.sequence_number(seq);
    write_params.related_sample_identity() = related_sample_identity_;

    // Publish the new value, deduce the instance handle
    bool write_ret = native_writer.write((void*)&data, write_params);
    ASSERT_EQ(true, write_ret);

    DataReader& native_reader = reliable_reader.get_native_reader();

    HelloWorld read_data;
    eprosima::fastdds::dds::SampleInfo info;
    eprosima::fastrtps::Duration_t timeout;
    timeout.seconds = 2;
    while (!native_reader.wait_for_unread_message(timeout))
    {
    }

    ASSERT_EQ(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK,
            native_reader.take_next_sample((void*)&read_data, &info));

    ASSERT_TRUE(exists_pid_related_sample_identity);
    ASSERT_TRUE(exists_pid_custom_related_sample_identity);

    ASSERT_EQ(related_sample_identity_, info.related_sample_identity);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
