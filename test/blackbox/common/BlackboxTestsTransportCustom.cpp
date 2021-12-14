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

#include <gtest/gtest.h>

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

    eprosima::fastdds::rtps::TransportDescriptorInterface* get_configuration()
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

    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);

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
