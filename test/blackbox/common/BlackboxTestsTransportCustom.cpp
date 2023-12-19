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

#include "BlackboxTests.hpp"

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <fastdds/rtps/transport/ChainingTransportDescriptor.h>
#include <fastdds/rtps/transport/ChainingTransport.h>
#include <fastdds/rtps/attributes/PropertyPolicy.h>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>

#include <gtest/gtest.h>

using BuiltinTransports = eprosima::fastdds::rtps::BuiltinTransports;

class TestChainingTransportDescriptor : public eprosima::fastdds::rtps::ChainingTransportDescriptor
{
public:

    TestChainingTransportDescriptor(
            std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> low_level)
        : ChainingTransportDescriptor(low_level)
    {
    }

    std::function<void()> init_function_called;

    std::function<void()> receive_function_called;

    std::function<void()> send_function_called;

    eprosima::fastdds::rtps::TransportInterface* create_transport() const override;
};

const std::string test_property_name = "test_property";
const std::string test_property_value = "test_value";

class TestChainingTransport : public eprosima::fastdds::rtps::ChainingTransport
{
public:

    TestChainingTransport(
            const TestChainingTransportDescriptor& descriptor)
        : ChainingTransport(descriptor)
        , descriptor_(descriptor)
    {
    }

    eprosima::fastdds::rtps::TransportDescriptorInterface* get_configuration() override
    {
        return &descriptor_;
    }

    bool init(
            const eprosima::fastrtps::rtps::PropertyPolicy* properties = nullptr) override
    {
        const std::string* value =
                eprosima::fastrtps::rtps::PropertyPolicyHelper::find_property(*properties, test_property_name);
        if (value && 0 == value->compare(test_property_value))
        {
            descriptor_.init_function_called();
        }
        return low_level_transport_->init(properties);
    }

    bool send(
            eprosima::fastrtps::rtps::SenderResource* low_sender_resource,
            const eprosima::fastrtps::rtps::octet* send_buffer,
            uint32_t send_buffer_size,
            eprosima::fastrtps::rtps::LocatorsIterator* destination_locators_begin,
            eprosima::fastrtps::rtps::LocatorsIterator* destination_locators_end,
            const std::chrono::steady_clock::time_point& timeout) override
    {
        descriptor_.send_function_called();

        // Call low level transport
        return low_sender_resource->send(send_buffer, send_buffer_size, destination_locators_begin,
                       destination_locators_end, timeout);
    }

    void receive(
            eprosima::fastdds::rtps::TransportReceiverInterface* next_receiver,
            const eprosima::fastrtps::rtps::octet* receive_buffer,
            uint32_t receive_buffer_size,
            const eprosima::fastrtps::rtps::Locator_t& local_locator,
            const eprosima::fastrtps::rtps::Locator_t& remote_locator) override
    {
        descriptor_.receive_function_called();

        // Call upper level
        next_receiver->OnDataReceived(receive_buffer, receive_buffer_size, local_locator, remote_locator);
    }

private:

    TestChainingTransportDescriptor descriptor_;
};

eprosima::fastdds::rtps::TransportInterface* TestChainingTransportDescriptor::create_transport() const
{
    return new TestChainingTransport(*this);
}

TEST(ChainingTransportTests, basic_test)
{
    bool writer_init_function_called = false;
    bool writer_receive_function_called = false;
    bool writer_send_function_called = false;
    bool reader_init_function_called = false;
    bool reader_receive_function_called = false;
    bool reader_send_function_called = false;
    eprosima::fastrtps::rtps::PropertyPolicy test_property_policy;
    test_property_policy.properties().push_back({test_property_name, test_property_value});
    std::shared_ptr<UDPv4TransportDescriptor> udp_transport = std::make_shared<UDPv4TransportDescriptor>();
    std::shared_ptr<TestChainingTransportDescriptor> writer_transport =
            std::make_shared<TestChainingTransportDescriptor>(udp_transport);
    writer_transport->init_function_called = [&writer_init_function_called]()
            {
                writer_init_function_called = true;
            };
    writer_transport->receive_function_called = [&writer_receive_function_called]()
            {
                writer_receive_function_called = true;
            };
    writer_transport->send_function_called = [&writer_send_function_called]()
            {
                writer_send_function_called = true;
            };
    std::shared_ptr<TestChainingTransportDescriptor> reader_transport =
            std::make_shared<TestChainingTransportDescriptor>(udp_transport);
    reader_transport->init_function_called = [&reader_init_function_called]()
            {
                reader_init_function_called = true;
            };
    reader_transport->receive_function_called = [&reader_receive_function_called]()
            {
                reader_receive_function_called = true;
            };
    reader_transport->send_function_called = [&reader_send_function_called]()
            {
                reader_send_function_called = true;
            };

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.disable_builtin_transport()
            .add_user_transport_to_pparams(writer_transport)
            .history_depth(10)
            .property_policy(test_property_policy)
            .init();

    ASSERT_TRUE(writer.isInitialized());

    reader.disable_builtin_transport()
            .add_user_transport_to_pparams(reader_transport)
            .property_policy(test_property_policy)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();
    reader.startReception(data);
    writer.send(data);
    ASSERT_TRUE(data.empty());
    reader.block_for_all();

    ASSERT_TRUE(writer_init_function_called);
    ASSERT_TRUE(writer_receive_function_called);
    ASSERT_TRUE(writer_send_function_called);
    ASSERT_TRUE(reader_init_function_called);
    ASSERT_TRUE(reader_receive_function_called);
    ASSERT_TRUE(reader_send_function_called);
}

//! This is a regression test for Redmine #19665
//! A Participant with an initial peer (client) creates the correct
//! number of sender resources after discovering a participant with
//! a WAN listening address (TCP server)
TEST(ChainingTransportTests, tcp_client_server_with_wan_correct_sender_resources)
{
    std::atomic<int> times_writer_init_function_called {0};
    std::atomic<int> times_writer_receive_function_called{0};
    std::atomic<int> times_writer_send_function_called{0};
    std::atomic<int> times_reader_init_function_called{0};
    std::atomic<int> times_reader_receive_function_called{0};
    std::atomic<int> times_reader_send_function_called{0};

    eprosima::fastrtps::rtps::PropertyPolicy test_property_policy;
    test_property_policy.properties().push_back({test_property_name, test_property_value});

    uint16_t port = static_cast<uint16_t>(GET_PID());

    if (5000 > port)
    {
        port += 5000;
    }

    std::shared_ptr<eprosima::fastdds::rtps::TCPv4TransportDescriptor> reader_tcp_transport =
            std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();

    reader_tcp_transport->set_WAN_address("127.0.0.1");
    reader_tcp_transport->listening_ports.push_back(port);

    eprosima::fastrtps::rtps::LocatorList_t reader_locators;
    eprosima::fastrtps::rtps::Locator_t reader_loc;
    reader_loc.port = port;
    IPLocator::setIPv4(reader_loc, "127.0.0.1");
    reader_loc.kind = LOCATOR_KIND_TCPv4;
    reader_locators.push_back(reader_loc);

    std::shared_ptr<TestChainingTransportDescriptor> reader_transport =
            std::make_shared<TestChainingTransportDescriptor>(reader_tcp_transport);
    reader_transport->init_function_called = [&times_reader_init_function_called]()
            {
                times_reader_init_function_called.fetch_add(1);
            };
    reader_transport->receive_function_called = [&times_reader_receive_function_called]()
            {
                times_reader_receive_function_called.fetch_add(1);
            };
    reader_transport->send_function_called = [&times_reader_send_function_called]()
            {
                times_reader_send_function_called.fetch_add(1);
            };

    std::shared_ptr<eprosima::fastdds::rtps::TCPv4TransportDescriptor> writer_tcp_transport =
            std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();

    std::shared_ptr<TestChainingTransportDescriptor> writer_transport =
            std::make_shared<TestChainingTransportDescriptor>(writer_tcp_transport);
    writer_transport->init_function_called = [&times_writer_init_function_called]()
            {
                times_writer_init_function_called.fetch_add(1);
            };
    writer_transport->receive_function_called = [&times_writer_receive_function_called]()
            {
                times_writer_receive_function_called.fetch_add(1);
            };
    writer_transport->send_function_called = [&times_writer_send_function_called]()
            {
                times_writer_send_function_called.fetch_add(1);
            };

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    eprosima::fastrtps::rtps::LocatorList_t initial_peers;
    initial_peers.push_back(reader_loc);

    writer.disable_builtin_transport()
            .add_user_transport_to_pparams(writer_transport)
            .initial_peers(initial_peers)
            .history_depth(10)
            .property_policy(test_property_policy)
            .init();

    ASSERT_TRUE(writer.isInitialized());

    reader.disable_builtin_transport()
            .add_user_transport_to_pparams(reader_transport)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .property_policy(test_property_policy)
            .metatraffic_unicast_locator_list(reader_locators)
            .set_default_unicast_locators(reader_locators)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(1);
    reader.startReception(data);
    writer.send(data);
    ASSERT_TRUE(data.empty());
    reader.block_for_all();

    ASSERT_EQ(times_writer_init_function_called.load(), 1);
    ASSERT_EQ(times_reader_init_function_called.load(), 1);
    ASSERT_GE(times_writer_send_function_called.load(), 0);
    ASSERT_GE(times_reader_receive_function_called.load(), 0);

    //! If only 1 sender resource was created
    //! Expect less than 30 calls in send/receive
    //! including discovery phase calls and reception.
    //! Else something is wrong, more than one sender resource
    //! is being created
    ASSERT_LE(times_writer_send_function_called.load(), 30);
    ASSERT_LE(times_reader_receive_function_called.load(), 30);
}

TEST(ChainingTransportTests, builtin_transports_basic_test)
{
    std::vector<BuiltinTransports> bt_list;
    bt_list.push_back(BuiltinTransports::DEFAULT);
    bt_list.push_back(BuiltinTransports::DEFAULTv6);
    bt_list.push_back(BuiltinTransports::SHM);
    bt_list.push_back(BuiltinTransports::UDPv4);
    bt_list.push_back(BuiltinTransports::UDPv6);
    bt_list.push_back(BuiltinTransports::LARGE_DATA);
#ifndef __APPLE__
    bt_list.push_back(BuiltinTransports::LARGE_DATAv6);
#endif // __APPLE__

    for (auto test_transport : bt_list)
    {
        {
            PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
            PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

            writer.setup_transports(test_transport)
                    .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
                    .init();

            ASSERT_TRUE(writer.isInitialized());

            reader.setup_transports(test_transport)
                    .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
                    .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
                    .init();

            ASSERT_TRUE(reader.isInitialized());

            // Wait for discovery.
            writer.wait_discovery();
            reader.wait_discovery();

            auto data = default_helloworld_data_generator();
            // size_t num_messages = data.size();
            reader.startReception(data);
            writer.send(data);
            ASSERT_TRUE(data.empty());
            reader.block_for_all();

            // Check reception
            // reader.wait_for_all_received(std::chrono::seconds(3), num_messages);
            EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(3)));
        }
    }

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    BuiltinTransports test_transport = BuiltinTransports::NONE;

    writer.setup_transports(test_transport)
            .init();

    ASSERT_FALSE(writer.isInitialized());

    reader.setup_transports(test_transport)
            .init();

    ASSERT_FALSE(reader.isInitialized());
}

TEST(ChainingTransportTests, builtin_transports_env_var_test)
{
    const std::string env_var_name("FASTDDS_BUILTIN_TRANSPORTS");

    std::vector<std::string> bt_list;
    bt_list.push_back("DEFAULT");
    bt_list.push_back("DEFAULTv6");
    bt_list.push_back("SHM");
    bt_list.push_back("UDPv4");
    bt_list.push_back("UDPv6");
    bt_list.push_back("LARGE_DATA");
#ifndef __APPLE__
    bt_list.push_back("LARGE_DATAv6");
#endif // __APPLE__

    for (auto test_transport : bt_list)
    {
        {
#ifdef _WIN32
            _putenv_s(env_var_name.c_str(), test_transport.c_str());
#else
            setenv(env_var_name.c_str(), test_transport.c_str(), 1);
#endif // _WIN32

            PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
            PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

            writer.history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
                    .init();

            ASSERT_TRUE(writer.isInitialized());

            reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
                    .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
                    .init();

            ASSERT_TRUE(reader.isInitialized());

            // Wait for discovery.
            writer.wait_discovery();
            reader.wait_discovery();

            auto data = default_helloworld_data_generator();
            // size_t num_messages = data.size();
            reader.startReception(data);
            writer.send(data);
            ASSERT_TRUE(data.empty());
            reader.block_for_all();

            // Check reception
            EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(3)));
        }
    }

    std::string value("NONE");
#ifdef _WIN32
    _putenv_s(env_var_name.c_str(), value.c_str());
#else
    setenv(env_var_name.c_str(), value.c_str(), 1);
#endif // _WIN32

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.init();

    ASSERT_FALSE(writer.isInitialized());

    reader.init();

    ASSERT_FALSE(reader.isInitialized());
}

TEST(ChainingTransportTests, builtin_transports_xml_test)
{
    std::vector<std::string> bt_list;
    bt_list.push_back("participant_none");
    bt_list.push_back("participant_default");
    bt_list.push_back("participant_defaultv6");
    bt_list.push_back("participant_shm");
    bt_list.push_back("participant_udp");
    bt_list.push_back("participant_udpv6");
    bt_list.push_back("participant_largedata");
#ifndef __APPLE__
    bt_list.push_back("participant_largedatav6");
#endif // __APPLE__

    for (auto test_transport : bt_list)
    {
        {
            PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
            PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

            writer.set_xml_filename("builtin_transports_profile.xml");
            writer.set_participant_profile(test_transport);
            writer.init();

            ASSERT_TRUE(writer.isInitialized());

            reader.set_xml_filename("builtin_transports_profile.xml");
            reader.set_participant_profile(test_transport);
            reader.init();

            ASSERT_TRUE(reader.isInitialized());

            // Wait for discovery.
            writer.wait_discovery();
            reader.wait_discovery();

            auto data = default_helloworld_data_generator();
            // size_t num_messages = data.size();
            reader.startReception(data);
            writer.send(data);
            ASSERT_TRUE(data.empty());
            reader.block_for_all();

            // Check reception
            EXPECT_TRUE(writer.waitForAllAcked(std::chrono::seconds(3)));
        }
    }
}
