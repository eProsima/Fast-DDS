// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "ServerApp.hpp"

#include <memory>
#include <string>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantExtendedQos.hpp>
#include <fastdds/dds/domain/qos/ReplierQos.hpp>

#include "app_utils.hpp"
#include "CLIParser.hpp"
#include "types/calculatorServer.hpp"
#include "types/calculatorServerImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rpc {

using namespace calculator_example;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::rpc;

ServerApp::ServerApp(
        const CLIParser::config& config,
        const std::string& service_name)
    : server_impl_(nullptr)
    , participant_(nullptr)
    , thread_pool_size_(config.thread_pool_size)
    , stop_(false)
    , timeout_thread_([&config, this]
            {
                if (config.timeout > 0)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(config.timeout));
                    this->stop();
                }
            })
{
    create_participant();
    create_server(service_name);

    client_server_info("ServerApp", "Server initialized with ID: " << participant_->guid().guidPrefix);
}

ServerApp::~ServerApp()
{
    timeout_thread_.join();

    // As a precautionary measure, delete the server here because participant_->delete_contained_entities()
    // does not automatically disable the service. This line can be removed once the RPC internal API
    // supports service disabling.
    server_.reset();

    if (participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_shared_instance()->delete_participant(participant_);
    }
}

void ServerApp::run()
{
    if (is_stopped())
    {
        return;
    }

    server_->run();

    client_server_info("ServerApp", "Server running");
}

void ServerApp::stop()
{
    stop_.store(true);
    server_->stop();

    client_server_info("ServerApp", "Server execution stopped");
}

void ServerApp::create_participant()
{
    // Create the participant
    auto factory = DomainParticipantFactory::get_shared_instance();

    if (!factory)
    {
        throw std::runtime_error("Failed to get participant factory instance");
    }

    participant_ = factory->create_participant_with_default_profile();

    if (!participant_)
    {
        throw std::runtime_error("Participant initialization failed");
    }
}

void ServerApp::create_server(
        const std::string& service_name)
{
    ReplierQos qos;

    server_impl_ = ServerImpl::create();

    server_ = create_CalculatorServer(
        *participant_,
        service_name.c_str(),
        qos,
        thread_pool_size_,
        server_impl_);

    if (!server_)
    {
        throw std::runtime_error("Server initialization failed");
    }
}

calculator_example::detail::Calculator_representation_limits_Out ServerApp::ServerImpl::representation_limits(
        const eprosima::fastdds::dds::rpc::RpcRequest& info)
{
    static_cast<void>(info);
    calculator_example::detail::Calculator_representation_limits_Out limits;
    limits.min_value = std::numeric_limits<int32_t>::min();
    limits.max_value = std::numeric_limits<int32_t>::max();
    return limits;
}

int32_t ServerApp::ServerImpl::addition(
        const eprosima::fastdds::dds::rpc::RpcRequest& info,
        /*in*/ int32_t value1,
        /*in*/ int32_t value2)
{
    static_cast<void>(info);

    int32_t result = value1 + value2;
    bool negative_1 = value1 < 0;
    bool negative_2 = value2 < 0;
    bool negative_result = result < 0;

    if ((negative_1 == negative_2) && (negative_result != negative_1))
    {
        throw calculator_example::OverflowException();
    }

    return result;
}

int32_t ServerApp::ServerImpl::subtraction(
        const eprosima::fastdds::dds::rpc::RpcRequest& info,
        /*in*/ int32_t value1,
        /*in*/ int32_t value2)
{
    static_cast<void>(info);

    int32_t result = value1 - value2;
    bool negative_1 = value1 < 0;
    bool negative_2 = value2 < 0;
    bool negative_result = result < 0;

    if ((negative_1 != negative_2) && (negative_result != negative_1))
    {
        throw calculator_example::OverflowException();
    }

    return result;
}

void ServerApp::ServerImpl::fibonacci_seq(
        const eprosima::fastdds::dds::rpc::RpcRequest& info,
        /*in*/ uint32_t n_results,
        /*result*/ eprosima::fastdds::dds::rpc::RpcServerWriter<int32_t>& result_writer)
{
    static_cast<void>(info);

    int32_t a = 1;
    int32_t b = 1;
    int32_t c = 0;

    for (uint32_t i = 0; i < n_results; ++i)
    {
        if (a < 0)
        {
            throw calculator_example::OverflowException(
                      "Overflow in Fibonacci sequence. "
                      "The result is too large to be represented.");
        }

        result_writer.write(a);
        c = a + b;
        a = b;
        b = c;
    }
}

int32_t ServerApp::ServerImpl::sum_all(
        const eprosima::fastdds::dds::rpc::RpcRequest& info,
        /*in*/ eprosima::fastdds::dds::rpc::RpcServerReader<int32_t>& value)
{
    static_cast<void>(info);

    int32_t sum_all_result = 0;

    try
    {
        int32_t value_to_add = 0;
        while (value.read(value_to_add))
        {
            int32_t new_sum = sum_all_result + value_to_add;
            bool current_negative = sum_all_result < 0;
            bool new_negative = value_to_add < 0;
            bool result_negative = new_sum < 0;
            if ((current_negative == new_negative) && (result_negative != current_negative))
            {
                throw calculator_example::OverflowException();
            }
            sum_all_result = new_sum;
        }
    }
    catch (const eprosima::fastdds::dds::rpc::RpcFeedCancelledException& ex)
    {
        static_cast<void>(ex);
        // Feed was cancelled, do nothing and return the current sum
    }

    return sum_all_result;
}

void ServerApp::ServerImpl::accumulator(
        const eprosima::fastdds::dds::rpc::RpcRequest& info,
        /*in*/ eprosima::fastdds::dds::rpc::RpcServerReader<int32_t>& value,
        /*result*/ eprosima::fastdds::dds::rpc::RpcServerWriter<int32_t>& result_writer)
{
    static_cast<void>(info);

    int32_t current_sum = 0;

    try
    {
        int32_t value_to_add = 0;
        while (value.read(value_to_add))
        {
            int32_t new_sum = current_sum + value_to_add;
            bool current_negative = current_sum < 0;
            bool new_negative = value_to_add < 0;
            bool result_negative = new_sum < 0;
            if ((current_negative == new_negative) && (result_negative != current_negative))
            {
                throw calculator_example::OverflowException();
            }
            current_sum = new_sum;
            result_writer.write(current_sum);
        }
    }
    catch (const eprosima::fastdds::dds::rpc::RpcFeedCancelledException& ex)
    {
        static_cast<void>(ex);
        // Feed was cancelled, do nothing
    }
}

void ServerApp::ServerImpl::filter(
        const eprosima::fastdds::dds::rpc::RpcRequest& info,
        /*in*/ eprosima::fastdds::dds::rpc::RpcServerReader<int32_t>& value,
        /*in*/ calculator_example::FilterKind filter_kind,
        /*result*/ eprosima::fastdds::dds::rpc::RpcServerWriter<int32_t>& result_writer)
{
    static_cast<void>(info);

    try
    {
        int32_t value_to_filter = 0;
        while (value.read(value_to_filter))
        {
            switch (filter_kind)
            {
                case calculator_example::FilterKind::EVEN:
                {
                    if (value_to_filter % 2 == 0)
                    {
                        result_writer.write(value_to_filter);
                    }
                    break;
                }
                case calculator_example::FilterKind::ODD:
                {
                    if (value_to_filter % 2 != 0)
                    {
                        result_writer.write(value_to_filter);
                    }
                    break;
                }
                case calculator_example::FilterKind::PRIME:
                {
                    bool prime = true;
                    if (value_to_filter <= 1)
                    {
                        prime = false;
                    }
                    else
                    {
                        for (int i = 2; i < value_to_filter; i++)
                        {
                            if (value_to_filter % i == 0)
                            {
                                prime = false;
                                break;
                            }
                        }
                    }

                    if (prime)
                    {
                        result_writer.write(value_to_filter);
                    }

                    break;
                }

                default:
                    // Invalid filter kind
                    throw eprosima::fastdds::dds::rpc::RemoteInvalidArgumentError();
            }
        }
    }
    catch (const eprosima::fastdds::dds::rpc::RpcFeedCancelledException& ex)
    {
        static_cast<void>(ex);
        // Feed was cancelled, do nothing
    }
}

} // namespace rpc
} // namespace examples
} // namespace fastdds
} // namespace eprosima
