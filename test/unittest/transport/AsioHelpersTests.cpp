// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <limits>
#include <memory>
#include <thread>

#include <asio.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/utils/IPFinder.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <utils/Semaphore.hpp>

#include <MockReceiverResource.h>
#include <rtps/transport/asio_helpers.hpp>
#include <rtps/transport/UDPv4Transport.h>

using namespace eprosima::fastdds::rtps;


// Regression tests for redmine issue #22210

template <typename BufferOption, typename SocketType, typename Protocol>
void test_buffer_setting(
        int initial_buffer_value,
        int minimum_buffer_value)
{
    asio::io_service io_service;
    auto socket = std::make_unique<SocketType>(io_service);

    // Open the socket with the provided protocol
    socket->open(Protocol::v4());

    uint32_t final_buffer_value = 0;

    // Replace this with your actual implementation of try_setting_buffer_size
    ASSERT_TRUE(asio_helpers::try_setting_buffer_size<BufferOption>(
                *socket, initial_buffer_value, minimum_buffer_value, final_buffer_value));



    BufferOption option;
    asio::error_code ec;
    socket->get_option(option, ec);
    if (!ec)
    {
        ASSERT_EQ(static_cast<uint32_t>(option.value()), final_buffer_value);
    }
    else
    {
        throw std::runtime_error("Failed to get buffer option");
    }
}

// Test that the UDP buffer size is set actually to the value stored as the final value
TEST(AsioHelpersTests, udp_buffer_size)
{
    uint32_t minimum_buffer_value = 0;
    for (uint32_t initial_buffer_value = std::numeric_limits<uint32_t>::max(); initial_buffer_value > 0;
            initial_buffer_value /= 4)
    {
        test_buffer_setting<asio::socket_base::send_buffer_size, asio::ip::udp::socket, asio::ip::udp>(
            initial_buffer_value, minimum_buffer_value);
        test_buffer_setting<asio::socket_base::receive_buffer_size, asio::ip::udp::socket, asio::ip::udp>(
            initial_buffer_value, minimum_buffer_value);
    }
}

// Test that the TCP buffer size is set actually to the value stored as the final value
TEST(AsioHelpersTests, tcp_buffer_size)
{
    uint32_t minimum_buffer_value = 0;
    for (uint32_t initial_buffer_value = std::numeric_limits<uint32_t>::max(); initial_buffer_value > 0;
            initial_buffer_value /= 4)
    {
        test_buffer_setting<asio::socket_base::send_buffer_size, asio::ip::tcp::socket, asio::ip::tcp>(
            initial_buffer_value, minimum_buffer_value);
        test_buffer_setting<asio::socket_base::receive_buffer_size, asio::ip::tcp::socket, asio::ip::tcp>(
            initial_buffer_value, minimum_buffer_value);
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
