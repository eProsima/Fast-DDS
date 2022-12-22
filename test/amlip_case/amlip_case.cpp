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
#include <random>
#include <thread>

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/log/Log.hpp>
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
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/types/TypesBase.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "types/requestPubSubTypes.h"
#include "types/replyPubSubTypes.h"

// #define LOG EPROSIMA_LOG_ERROR
// #define LOG(x, y) std::cout << x << " : " << y << std::endl
#define LOG(x, y)

using namespace eprosima::fastdds::dds;

constexpr const int TEST_DOMAIN = 42;
constexpr const bool USE_INTERPROCESS = true;

constexpr const char* REQUEST_TOPIC_NAME = "request";
constexpr const char* REPLY_TOPIC_NAME = "reply";
constexpr const char* TARGET_TOPIC_NAME = "target";

constexpr const int N_CLIENTS = 2;
constexpr const int N_SERVERS = 2;
constexpr const int N_TASKS = 4;
constexpr const int CLIENT_TIME_AFTER_TASK = 20;
constexpr const int SERVER_TIME_AFTER_TASK = 10;

void sleep_for(int t_ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(t_ms));
}

void sleep_for_random(int min, int max = 0)
{
    if (!max)
    {
        max = min * 2;
    }
    auto sleep_time = rand()%(max-min + 1) + min;
    sleep_for(sleep_time);
}

eprosima::fastrtps::Duration_t end_of_times()
{
    return eprosima::fastrtps::Duration_t(
        eprosima::fastrtps::Duration_t::INFINITE_SECONDS,
        eprosima::fastrtps::Duration_t::INFINITE_NANOSECONDS);
}

struct CustomListener : public DataReaderListener
{
    virtual void on_data_available(
            DataReader* reader) override
    {
        eprosima::fastdds::dds::SampleInfo info;
        reader->get_first_untaken_info(&info);

        LOG(_LISTENER_,
            "Message received in topic " << reader->get_topicdescription()->get_name()
            << " from: " << info.sample_identity.writer_guid()
            << " and has: " << reader->get_unread_count()
            << " to read.");
    }
};

struct ParticipantClient
{
    ParticipantClient(int id)
        : id_(id)
    {
        LOG(CLIENT_INI, "Creating participant " << id_);

        /////
        // CREATE PARTICIPANT
        DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
        // Create QoS
        DomainParticipantQos pqos;
        // Set name
        pqos.name(std::string("ParticipantClient_") + std::to_string(id));
        // Deactivate using typelookup service
        pqos.wire_protocol().builtin.typelookup_config.use_client = false;
        pqos.wire_protocol().builtin.typelookup_config.use_server = false;
        // Only UDP
        pqos.transport().use_builtin_transports = false;
        pqos.transport().user_transports.push_back(std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>());
        // Create participant
        participant = factory->create_participant(TEST_DOMAIN, pqos);

        /////
        // REGISTER TYPES
        // Request
        type_support_request.reset(new RequestPubSubType());
        // Deactivate using dynamic types (seg fault)
        type_support_request->auto_fill_type_information(false);
        type_support_request->auto_fill_type_object(false);
        // Register
        type_support_request.register_type(participant);

        // Reply
        type_support_reply.reset(new ReplyPubSubType());
        // Deactivate using dynamic types (seg fault)
        type_support_reply->auto_fill_type_information(false);
        type_support_reply->auto_fill_type_object(false);
        // Register
        type_support_reply.register_type(participant);

        /////
        // CREATE TOPICS
        topic_request = participant->create_topic(REQUEST_TOPIC_NAME, type_support_request.get_type_name(), TOPIC_QOS_DEFAULT);
        topic_reply = participant->create_topic(REPLY_TOPIC_NAME, type_support_reply.get_type_name(), TOPIC_QOS_DEFAULT);
        topic_target = participant->create_topic(TARGET_TOPIC_NAME, type_support_reply.get_type_name(), TOPIC_QOS_DEFAULT);

        /////
        // CREATE ENTITIES
        // Create publisher
        publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
        // Create subscriber
        subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

        // QoS Writer
        DataWriterQos dw_qos;
        dw_qos.data_sharing().off();
        dw_qos.publish_mode().kind = eprosima::fastdds::dds::PublishModeQosPolicyKind::SYNCHRONOUS_PUBLISH_MODE;
        dw_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
        dw_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
        dw_qos.history().kind = eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;

        // QoS Reader
        DataReaderQos dr_qos;
        dr_qos.data_sharing().off();
        dr_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
        dr_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
        dr_qos.history().kind = eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;

        // Writers and Readers
        writer_request = publisher->create_datawriter(topic_request, dw_qos);
        reader_reply = subscriber->create_datareader(topic_reply, dr_qos, &listener_reply);
        writer_target = publisher->create_datawriter(topic_target, dw_qos);

        LOG(CLIENT_INI, "Participant " << id_ << " created.");
    };

    ~ParticipantClient()
    {
        LOG(CLIENT_FIN, "Destroying Participant : " << id_);

        writer_target->wait_for_acknowledgments(end_of_times());
        writer_request->wait_for_acknowledgments(end_of_times());

        publisher->delete_datawriter(writer_target);
        subscriber->delete_datareader(reader_reply);
        publisher->delete_datawriter(writer_request);

        participant->delete_subscriber(subscriber);
        participant->delete_publisher(publisher);

        participant->delete_topic(topic_target);
        participant->delete_topic(topic_reply);
        participant->delete_topic(topic_request);

        DomainParticipantFactory::get_instance()->delete_participant(participant);

        LOG(CLIENT_FIN, "Participant " << id_ << " destroyed");
    }

    void send_task(int task_id)
    {
        /////
        // Send request
        LOG(CLIENT_RUN, "Client " << id_ << " sending request " << task_id);
        // Create request data
        Request request;
        request.task_id(task_id);
        request.client_id(id_);
        // Publish
        writer_request->write(&request);
        LOG(CLIENT_RUN, "Client " << id_ << " sent request " << task_id);

        /////
        // Wait for reply
        int server_id;
        while (true)
        {
            // Wait for data to read
            reader_reply->wait_for_unread_message(end_of_times());
            LOG(CLIENT_RUN, "Client " << id_ << " data to read in Reader Reply.");

            // Read data
            Reply reply;
            SampleInfo info;
            auto result = reader_reply->take_next_sample(&reply, &info);
            ASSERT_EQ(result, ReturnCode_t::RETCODE_OK) << "in topic reply when reader has still data to read: " << reader_reply->get_unread_count();

            if (reply.client_id() == id_
                    && reply.task_id() == task_id)
            {
                server_id = reply.server_id();
                break;
            }
        }
        LOG(CLIENT_RUN, "Client " << id_ << " for request " << task_id << " chooses server " << server_id);

        /////
        // Send target
        // Create reply data
        Reply reply;
        reply.task_id(task_id);
        reply.client_id(id_);
        reply.server_id(server_id);
        // Publish
        writer_target->write(&reply);

        LOG(CLIENT_RES, "Client " << id_ << " for request " << task_id << " chooses server " << server_id);
    }

    int id_;

    DomainParticipant* participant{nullptr};
    TypeSupport type_support_request{};
    TypeSupport type_support_reply{};
    Topic* topic_request{nullptr};
    Topic* topic_reply{nullptr};
    Topic* topic_target{nullptr};
    Publisher* publisher{nullptr};
    Subscriber* subscriber{nullptr};
    DataWriter* writer_request{nullptr};
    DataReader* reader_reply{nullptr};
    DataWriter* writer_target{nullptr};

    CustomListener listener_reply{};
};

struct ParticipantServer
{
    ParticipantServer(int id)
        : id_(id)
    {
        LOG(SERVER_INI, "Creating Participant " << id_);

        /////
        // CREATE PARTICIPANT
        DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
        // Create QoS
        DomainParticipantQos pqos;
        // Set name
        pqos.name(std::string("ParticipantServer_") + std::to_string(id));
        // Deactivate using typelookup service
        pqos.wire_protocol().builtin.typelookup_config.use_client = false;
        pqos.wire_protocol().builtin.typelookup_config.use_server = false;
        // Only UDP
        pqos.transport().use_builtin_transports = false;
        pqos.transport().user_transports.push_back(std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>());
        // Create participant
        participant = factory->create_participant(TEST_DOMAIN, pqos);

        /////
        // REGISTER TYPES
        // Request
        type_support_request.reset(new RequestPubSubType());
        // Deactivate using dynamic types (seg fault)
        type_support_request->auto_fill_type_information(false);
        type_support_request->auto_fill_type_object(false);
        // Register
        type_support_request.register_type(participant);

        // Reply
        type_support_reply.reset(new ReplyPubSubType());
        // Deactivate using dynamic types (seg fault)
        type_support_reply->auto_fill_type_information(false);
        type_support_reply->auto_fill_type_object(false);
        // Register
        type_support_reply.register_type(participant);

        /////
        // CREATE TOPICS
        topic_request = participant->create_topic(REQUEST_TOPIC_NAME, type_support_request.get_type_name(), TOPIC_QOS_DEFAULT);
        topic_reply = participant->create_topic(REPLY_TOPIC_NAME, type_support_reply.get_type_name(), TOPIC_QOS_DEFAULT);
        topic_target = participant->create_topic(TARGET_TOPIC_NAME, type_support_reply.get_type_name(), TOPIC_QOS_DEFAULT);

        /////
        // CREATE ENTITIES
        // Create publisher
        publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
        // Create subscriber
        subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

        // QoS Writer
        DataWriterQos dw_qos;
        dw_qos.data_sharing().off();
        dw_qos.publish_mode().kind = eprosima::fastdds::dds::PublishModeQosPolicyKind::SYNCHRONOUS_PUBLISH_MODE;
        dw_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
        dw_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
        dw_qos.history().kind = eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;

        // QoS Reader
        DataReaderQos dr_qos;
        dr_qos.data_sharing().off();
        dr_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
        dr_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
        dr_qos.history().kind = eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;

        // Writers and Readers
        reader_request = subscriber->create_datareader(topic_request, dr_qos, &listener_request);
        writer_reply = publisher->create_datawriter(topic_reply, dw_qos);
        reader_target = subscriber->create_datareader(topic_target, dr_qos, &listener_target);

        LOG(SERVER_INI, "Participant " << id_ << " created.");
    };

    ~ParticipantServer()
    {
        LOG(SERVER_FIN, "Destroying Participant : " << id_);

        writer_reply->wait_for_acknowledgments(end_of_times());

        subscriber->delete_datareader(reader_target);
        publisher->delete_datawriter(writer_reply);
        subscriber->delete_datareader(reader_request);

        participant->delete_subscriber(subscriber);
        participant->delete_publisher(publisher);

        participant->delete_topic(topic_target);
        participant->delete_topic(topic_reply);
        participant->delete_topic(topic_request);

        DomainParticipantFactory::get_instance()->delete_participant(participant);

        LOG(SERVER_FIN, "Participant " << id_ << " destroyed");
    }

    void process_task()
    {
        /////
        // Answer at least 1 request
        Request request;
        while(true)
        {
            /////
            // Wait for request
            while (true)
            {
                // Wait for data to read
                reader_request->wait_for_unread_message(end_of_times());
                LOG(SERVER_RUN, "Server " << id_ << " data to read in Reader Request.");

                // Read data
                SampleInfo info;
                auto result = reader_request->take_next_sample(&request, &info);
                ASSERT_EQ(result, ReturnCode_t::RETCODE_OK) << "in topic reque when reader has still data to read:st " << reader_request->get_unread_count();

                // If data has already been answered with another target, keep reading
                // Else stop reading requests and answer reply
                if (std::find(
                        already_targeted_.begin(),
                        already_targeted_.end(),
                        request)
                    == already_targeted_.end())
                {
                    break;
                }
            }
            LOG(SERVER_RUN, "Server " << id_ << " are replying to client " << request.client_id() << " task " << request.task_id());

            /////
            // Send reply
            // Create reply data
            Reply reply;
            reply.task_id(request.task_id());
            reply.client_id(request.client_id());
            reply.server_id(id_);
            // Publish
            writer_reply->write(&reply);

            /////
            // Wait for target
            Reply target;
            while (true)
            {
                // Wait for data to read
                reader_target->wait_for_unread_message(end_of_times());

                // Read data
                SampleInfo info;
                auto result = reader_target->take_next_sample(&target, &info);
                ASSERT_EQ(result, ReturnCode_t::RETCODE_OK) << "in topic targe when reader has still data to read:t " << reader_target->get_unread_count();

                Request request_associated;
                request_associated.client_id(target.client_id());
                request_associated.task_id(target.task_id());
                already_targeted_.push_back(request_associated);

                if (request.client_id() == target.client_id() &&
                    request.task_id() == target.task_id())
                {
                    // Answer that target has been found
                    break;
                }
            }

            // If target is this server, finish
            // Otherwise start the process from the beginning
            if (target.server_id() == id_)
            {
                break;
                LOG(SERVER_RUN, "Server " << id_ << " chosen by " << request.client_id() << " task " << request.task_id());
            }
            else
            {
                LOG(SERVER_RUN, "Server " << id_ << " rejected by " << request.client_id() << " task " << request.task_id());
            }
        }

        LOG(SERVER_RES, "Server " << id_ << " chosen by " << request.client_id() << " task " << request.task_id());
    }

    int id_;
    std::vector<Request> already_targeted_;

    DomainParticipant* participant{nullptr};
    TypeSupport type_support_request{};
    TypeSupport type_support_reply{};
    Topic* topic_request{nullptr};
    Topic* topic_reply{nullptr};
    Topic* topic_target{nullptr};
    Publisher* publisher{nullptr};
    Subscriber* subscriber{nullptr};
    DataReader* reader_request{nullptr};
    DataWriter* writer_reply{nullptr};
    DataReader* reader_target{nullptr};

    CustomListener listener_request{};
    CustomListener listener_target{};
};

TEST(amlip_case_test, test_amlip_multiservice_case)
{
    // Disable intraprocess if chosen
    if (!USE_INTERPROCESS)
    {
        eprosima::fastrtps::LibrarySettingsAttributes att;
        att.intraprocess_delivery = eprosima::fastrtps::IntraprocessDeliveryType::INTRAPROCESS_OFF;
        eprosima::fastrtps::xmlparser::XMLProfileManager::library_settings(att);
    }

    // Each Client Routine
    auto client_lambda = [](int id, int tasks_to_send){

        // Create Participant
        ParticipantClient participant(id);

        // Wait for N messages
        for (int i=0; i<tasks_to_send; i++)
        {
            // Send new task
            participant.send_task(i);
            sleep_for_random(CLIENT_TIME_AFTER_TASK);  // Simulate send and reception of task
        }
    };

    // Each Server Routine
    auto server_lambda = [](int id, int tasks_to_process){

        // Create entities
        ParticipantServer participant(id);

        // Wait for N messages
        for (int i=0; i<tasks_to_process; i++)
        {
            // Process task and convert it to lowercase
            participant.process_task();
            sleep_for_random(SERVER_TIME_AFTER_TASK);  // Simulate send and reception of task
        }
    };

    // Generate all clients
    std::array<std::thread, N_CLIENTS> clients_threads;
    for (unsigned int i=0; i< N_CLIENTS; i++)
    {
        clients_threads[i] = std::thread(client_lambda, i, N_TASKS);
    }

    // Generate all servers
    auto messages_per_server = N_TASKS * N_CLIENTS / N_SERVERS;
    std::array<std::thread, N_SERVERS> servers_threads;
    for (unsigned int i=0; i< N_SERVERS; i++)
    {
        servers_threads[i] = std::thread(server_lambda, i + N_CLIENTS, messages_per_server);
    }

    // Wait for all of them to finish
    for (unsigned int i=0; i< N_CLIENTS; i++)
    {
        clients_threads[i].join();
    }
    for (unsigned int i=0; i< N_SERVERS; i++)
    {
        servers_threads[i].join();
    }
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
