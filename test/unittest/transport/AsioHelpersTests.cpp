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

#include <chrono>
#include <limits>
#include <memory>
#include <thread>
#include <vector>

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

template<typename BufferOption, typename SocketType, typename Protocol>
void test_buffer_setting(
        int initial_buffer_value,
        int minimum_buffer_value)
{
    asio::io_context io_context;
    auto socket = std::make_unique<SocketType>(io_context);

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

// Test that microsecond durations are converted into a normalized timeval whose tv_usec
// field is always within the [0, 999999] range accepted by the kernel. Leaving the field
// out of range makes setsockopt(SO_SNDTIMEO) fail with EINVAL for every timeout of one
// second or longer, which silently disables the configured send timeout.
TEST(AsioHelpersTests, duration_to_timeval_keeps_tv_usec_in_range)
{
    const std::vector<std::chrono::microseconds> timeouts =
    {
        std::chrono::microseconds(1),
        std::chrono::microseconds(1000),
        std::chrono::microseconds(100000),
        std::chrono::microseconds(999999),
        std::chrono::microseconds(1000000),
        std::chrono::microseconds(1000001),
        std::chrono::microseconds(1500000),
        std::chrono::microseconds(5000000),
        std::chrono::microseconds(30000000),
        std::chrono::microseconds(3600000000LL)
    };

    for (const auto& timeout : timeouts)
    {
        timeval time_struct;
        asio_helpers::duration_to_timeval(timeout, time_struct);

        EXPECT_GE(time_struct.tv_usec, 0) << "timeout of " << timeout.count() << " us";
        EXPECT_LE(time_struct.tv_usec, 999999) << "timeout of " << timeout.count() << " us";

        // The converted value must represent the very same instant.
        const auto total = std::chrono::seconds(time_struct.tv_sec) +
                std::chrono::microseconds(time_struct.tv_usec);
        EXPECT_EQ(std::chrono::duration_cast<std::chrono::microseconds>(total).count(), timeout.count())
            << "timeout of " << timeout.count() << " us";
    }
}

// Test the expected conversion for representative durations.
TEST(AsioHelpersTests, duration_to_timeval_splits_seconds_and_microseconds)
{
    struct TestCase
    {
        std::chrono::microseconds input;
        decltype(timeval::tv_sec) expected_sec;
        decltype(timeval::tv_usec) expected_usec;
    };

    const std::vector<TestCase> cases =
    {
        {std::chrono::microseconds(100000),   0,  100000},
        {std::chrono::microseconds(999999),   0,  999999},
        {std::chrono::microseconds(1000000),  1,       0},
        {std::chrono::microseconds(1500000),  1,  500000},
        {std::chrono::microseconds(5000000),  5,       0},
        {std::chrono::microseconds(30000000), 30,      0}
    };

    for (const auto& test_case : cases)
    {
        timeval time_struct;
        asio_helpers::duration_to_timeval(test_case.input, time_struct);

        EXPECT_EQ(time_struct.tv_sec, test_case.expected_sec)
            << "timeout of " << test_case.input.count() << " us";
        EXPECT_EQ(time_struct.tv_usec, test_case.expected_usec)
            << "timeout of " << test_case.input.count() << " us";
    }
}

// Test that a non-positive timeout is mapped to a zero timeval.
TEST(AsioHelpersTests, duration_to_timeval_maps_non_positive_durations_to_zero)
{
    const std::vector<std::chrono::microseconds> timeouts =
    {
        std::chrono::microseconds(0),
        std::chrono::microseconds(-1),
        std::chrono::microseconds(-1000000)
    };

    for (const auto& timeout : timeouts)
    {
        timeval time_struct;
        time_struct.tv_sec = 123;
        time_struct.tv_usec = 456;
        asio_helpers::duration_to_timeval(timeout, time_struct);

        EXPECT_EQ(time_struct.tv_sec, 0) << "timeout of " << timeout.count() << " us";
        EXPECT_EQ(time_struct.tv_usec, 0) << "timeout of " << timeout.count() << " us";
    }
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
