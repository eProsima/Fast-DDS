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

/**
 * @file ClientApp.cpp
 *
 */

#include "ClientApp.hpp"

#include <chrono>
#include <memory>
#include <thread>
#include <string>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantExtendedQos.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>
#include <fastdds/dds/rpc/exceptions.hpp>
#include <fastdds/dds/rpc/interfaces/RpcFuture.hpp>
#include <fastdds/dds/rpc/exceptions.hpp>

#include "app_utils.hpp"
#include "CLIParser.hpp"
#include "InputFeedProcessor.hpp"
#include "types/calculatorClient.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rpc {

std::unique_ptr<std::stringstream> InputFeedProcessor::provided_input_feed(nullptr);

using calculator_example::create_CalculatorClient;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::rpc;

Ping::Ping(
        std::shared_ptr<calculator_example::Calculator> client)
    : client_(client)
{
}

OperationStatus Ping::execute()
{
    // Send a test request to the server and wait for a reply
    if (auto client = client_.lock())
    {
        RpcFuture<int32_t> future = client->addition(0, 0);

        if (future.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready)
        {
            client_server_info("ClientApp", "Server not reachable");
            return OperationStatus::TIMEOUT;
        }

        try
        {
            future.get();

            client_server_info("ClientApp", "Server reachable");
            return OperationStatus::SUCCESS;
        }
        catch (const RpcBrokenPipeException&)
        {
            client_server_info("ClientApp", "Server not reachable");
            return OperationStatus::ERROR;
        }
        catch (const RpcException& e)
        {
            client_server_error("ClientApp", "RPC exception occurred: " << e.what());
            return OperationStatus::ERROR;
        }
    }
    else
    {
        throw std::runtime_error("Client reference expired");
    }
}

RepresentationLimits::RepresentationLimits(
        std::shared_ptr<calculator_example::Calculator> client)
    : client_(client)
{
}

OperationStatus RepresentationLimits::execute()
{
    // Send the request to the server and wait for the reply
    if (auto client = client_.lock())
    {
        auto future = client->representation_limits();

        if (future.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready)
        {
            client_server_error("ClientApp", "Operation timed out");

            return OperationStatus::TIMEOUT;
        }

        try
        {
            representation_limits_ = future.get();

            // Print the results
            client_server_info("ClientApp",
                    "Representation limits received: min_value = " << representation_limits_.min_value <<
                    ", max_value = " <<
                    representation_limits_.max_value);

            return OperationStatus::SUCCESS;
        }
        catch (const RpcException& e)
        {
            client_server_error("ClientApp", "RPC exception occurred: " << e.what());

            return OperationStatus::ERROR;
        }
    }
    else
    {
        throw std::runtime_error("Client reference expired");
    }
}

Addition::Addition(
        std::shared_ptr<calculator_example::Calculator> client,
        std::int32_t x,
        std::int32_t y)
    : x_(x)
    , y_(y)
    , client_(client)
{
}

OperationStatus Addition::execute()
{
    // Send the request to the server and wait for the reply
    if (auto client = client_.lock())
    {
        RpcFuture<int32_t> future = client->addition(x_, y_);

        if (future.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready)
        {
            client_server_error("ClientApp", "Operation timed out");

            return OperationStatus::TIMEOUT;
        }

        try
        {
            result_ = future.get();
            // Print the result
            client_server_info("ClientApp", "Addition result: " << x_ << " + " << y_ << " = " << result_);

            return OperationStatus::SUCCESS;
        }
        catch (const RpcException& e)
        {
            client_server_error("ClientApp", "RPC exception occurred: " << e.what());

            return OperationStatus::ERROR;
        }
    }
    else
    {
        throw std::runtime_error("Client reference expired");
    }
}

Substraction::Substraction(
        std::shared_ptr<calculator_example::Calculator> client,
        std::int32_t x,
        std::int32_t y)
    : x_(x)
    , y_(y)
    , client_(client)
{
}

OperationStatus Substraction::execute()
{
    // Send the request to the server and wait for the reply
    if (auto client = client_.lock())
    {
        RpcFuture<int32_t> future = client->subtraction(x_, y_);

        if (future.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready)
        {
            client_server_error("ClientApp", "Operation timed out");

            return OperationStatus::TIMEOUT;
        }

        try
        {
            result_ = future.get();

            // Print the result
            client_server_info("ClientApp", "Substraction result: " << x_ << " - " << y_ << " = " << result_);

            return OperationStatus::SUCCESS;
        }
        catch (const RpcException& e)
        {
            client_server_error("ClientApp", "RPC exception occurred: " << e.what());

            return OperationStatus::ERROR;
        }
    }
    else
    {
        throw std::runtime_error("Client reference expired");
    }
}

FibonacciSeq::FibonacciSeq(
        std::shared_ptr<calculator_example::Calculator> client,
        std::uint32_t n_results)
    : n_results_(n_results)
    , client_(client)
    , reader_(nullptr)
{
}

OperationStatus FibonacciSeq::execute()
{
    // If no requests have been sent, send a new request to the server
    // If a request has been sent and the feed is still open, wait for the next value
    if (auto client = client_.lock())
    {
        // Send a new request to the server if no request has been sent yet
        if (!reader_)
        {
            reader_ = client->fibonacci_seq(n_results_);

            if (!reader_)
            {
                client_server_error("ClientApp", "Failed to create Client Reader");

                return OperationStatus::ERROR;
            }
        }

        // Read the next value from the feed
        int32_t value;
        Duration_t timeout{1, 0}; // 1s

        try
        {
            if (reader_->read(value, timeout))
            {
                client_server_info("ClientApp", "Fibonacci sequence value: " << value);

                // Output feed not closed yet
                return OperationStatus::PENDING;
            }
            else
            {
                client_server_info("ClientApp", "Fibonacci sequence feed finished");

                // Request finished, unset the reader before the next request
                reader_.reset();

                return OperationStatus::SUCCESS;
            }
        }
        catch (const RpcTimeoutException& e)
        {
            client_server_error("ClientApp", "Operation timed out: " << e.what());

            return OperationStatus::TIMEOUT;
        }
        catch (const RpcException& e)
        {
            client_server_error("ClientApp", "RPC exception occurred: " << e.what());

            return OperationStatus::ERROR;
        }
    }
    else
    {
        throw std::runtime_error("Client reference expired");
    }
}

SumAll::SumAll(
        std::shared_ptr<calculator_example::Calculator> client)
    : client_(client)
    , writer_(nullptr)
    , result_(0)
    , input_feed_closed_(false)
{
}

OperationStatus SumAll::execute()
{
    if (auto client = client_.lock())
    {
        RpcFuture<int32_t> future;
        // Parse the input data and send it to the server
        // until the input feed is closed
        try
        {
            while (!input_feed_closed_)
            {
                if (!writer_)
                {
                    future = client->sum_all(writer_);
                    if (!writer_)
                    {
                        client_server_error("ClientApp", "Failed to create Client Writer");

                        return OperationStatus::ERROR;
                    }

                    if (!InputFeedProcessor::provided_input_feed)
                    {
                        // If no input feed is provided, print the help message
                        InputFeedProcessor::print_help();
                    }
                }

                // Get the input from the user
                auto input = InputFeedProcessor::get_input();

                // Check the input status
                switch (input.first)
                {
                    // Valid number received
                    case InputFeedProcessor::Status::VALID_INPUT:
                        // Send the number to the server
                        writer_->write(input.second);
                        client_server_info("ClientApp", "Input sent: " << input.second);
                        break;

                    // Invalid input received
                    case InputFeedProcessor::Status::INVALID_INPUT:
                        client_server_error("ClientApp", "Invalid input. Please enter a valid number.");
                        break;

                    // Input feed closed
                    case InputFeedProcessor::Status::FEED_CLOSED:
                        client_server_info("ClientApp", "Input feed closed.");
                        input_feed_closed_ = true;
                        writer_->finish();
                        break;

                    default:
                        client_server_error("ClientApp", "Unknown input status.");
                        break;
                }
            }

            if (future.wait_for(std::chrono::milliseconds(1000)) != std::future_status::ready)
            {
                client_server_error("ClientApp", "Operation timed out");

                return OperationStatus::TIMEOUT;
            }

            result_ = future.get();
            client_server_info("ClientApp", "Sum result: " << result_);
            writer_.reset();

            return OperationStatus::SUCCESS;
        }
        catch (const RpcException& e)
        {
            client_server_error("ClientApp", "Exception ocurred: " << e.what());

            return OperationStatus::ERROR;
        }
    }
    else
    {
        throw std::runtime_error("Client reference expired");
    }
}

Accumulator::Accumulator(
        std::shared_ptr<calculator_example::Calculator> client)
    : client_(client)
    , writer_(nullptr)
    , reader_(nullptr)
    , valid_user_input_(false)
{
}

OperationStatus Accumulator::execute()
{
    if (auto client = client_.lock())
    {
        if (!reader_)
        {
            assert(writer_ == nullptr);
            reader_ = client->accumulator(writer_);

            if (!reader_ || !writer_)
            {
                client_server_error("ClientApp", "Failed to create Client Reader/Writer");

                return OperationStatus::ERROR;
            }

            if (!InputFeedProcessor::provided_input_feed)
            {
                // If no input feed is provided, print the help message
                InputFeedProcessor::print_help();
            }
        }

        // Send a new value or close the input feed
        try
        {
            while (!valid_user_input_)
            {
                auto input = InputFeedProcessor::get_input();

                // Check the input status
                switch (input.first)
                {
                    // Valid number received
                    case InputFeedProcessor::Status::VALID_INPUT:
                        // Send the number to the server
                        writer_->write(input.second);
                        client_server_info("ClientApp", "Input sent: " << input.second);
                        valid_user_input_ = true;
                        break;

                    // Invalid input received
                    case InputFeedProcessor::Status::INVALID_INPUT:
                        client_server_error("ClientApp", "Invalid input. Please enter a valid number.");
                        break;

                    // Input feed closed
                    case InputFeedProcessor::Status::FEED_CLOSED:
                        client_server_info("ClientApp", "Input feed closed.");
                        writer_->finish();
                        valid_user_input_ = true;
                        break;

                    default:
                        client_server_error("ClientApp", "Unknown input status.");
                        break;
                }
            }

            valid_user_input_ = false;

            // Read the next value from the output feed
            int32_t value;

            Duration_t timeout{1, 0}; // 1s

            if (reader_->read(value, timeout))
            {
                client_server_info("ClientApp", "Accumulated sum: " << value);

                // Output feed not closed yet
                return OperationStatus::PENDING;
            }
            else
            {
                client_server_info("ClientApp", "Accumulator feed finished");

                reader_.reset();
                writer_.reset();

                return OperationStatus::SUCCESS;
            }
        }
        catch (const RpcTimeoutException& e)
        {
            client_server_error("ClientApp", "Operation timed out: " << e.what());

            return OperationStatus::TIMEOUT;
        }
        catch (const RpcException& e)
        {
            client_server_error("ClientApp", "RPC exception occurred: " << e.what());

            return OperationStatus::ERROR;
        }
    }
    else
    {
        throw std::runtime_error("Client reference expired");
    }
}

Filter::Filter(
        std::shared_ptr<calculator_example::Calculator> client,
        std::uint8_t filter_kind)
    : client_(client)
    , writer_(nullptr)
    , reader_(nullptr)
    , filter_kind_(filter_kind)
    , input_feed_closed_(false)
{
}

OperationStatus Filter::execute()
{
    if (auto client = client_.lock())
    {
        // Parse the input data and send it to the server
        // until the input feed is closed
        try
        {
            while (!input_feed_closed_)
            {
                if (!writer_)
                {
                    assert(reader_ == nullptr);
                    reader_ = client->filter(writer_, get_filter_kind(filter_kind_));

                    if (!reader_ || !writer_)
                    {
                        client_server_error("ClientApp", "Failed to create Client Reader/Writer");

                        return OperationStatus::ERROR;
                    }

                    if (!InputFeedProcessor::provided_input_feed)
                    {
                        // If no input feed is provided, print the help message
                        InputFeedProcessor::print_help();
                    }
                }

                // Get the input from the user
                auto input = InputFeedProcessor::get_input();

                // Check the input status
                switch (input.first)
                {
                    // Valid number received
                    case InputFeedProcessor::Status::VALID_INPUT:
                        // Send the number to the server
                        writer_->write(input.second);
                        client_server_info("ClientApp", "Input sent: " << input.second);
                        break;

                    // Invalid input received
                    case InputFeedProcessor::Status::INVALID_INPUT:
                        client_server_error("ClientApp", "Invalid input. Please enter a valid number.");
                        break;

                    // Input feed closed
                    case InputFeedProcessor::Status::FEED_CLOSED:
                        client_server_info("ClientApp", "Input feed closed.");
                        input_feed_closed_ = true;
                        writer_->finish();
                        break;

                    default:
                        client_server_error("ClientApp", "Unknown input status.");
                        break;
                }
            }

            // Get the next value from the output feed
            int32_t value;

            Duration_t timeout{1, 0}; // 1s

            if (reader_->read(value, timeout))
            {
                client_server_info("ClientApp", "Filtered sequence value: " << value);

                // Output feed not closed yet
                return OperationStatus::PENDING;
            }
            else
            {
                client_server_info("ClientApp", "Filtered sequence feed finished");

                reader_.reset();
                writer_.reset();
                input_feed_closed_ = false;

                return OperationStatus::SUCCESS;
            }
        }
        catch (const RpcTimeoutException& e)
        {
            client_server_error("ClientApp", "Operation timed out: " << e.what());

            return OperationStatus::TIMEOUT;
        }
        catch (const RpcException& e)
        {
            client_server_error("ClientApp", "Exception ocurred: " << e.what());

            return OperationStatus::ERROR;
        }
    }
    else
    {
        throw std::runtime_error("Client reference expired");
    }
}

ClientApp::ClientApp(
        const CLIParser::config& config,
        const std::string& service_name)
    : client_(nullptr)
    , participant_(nullptr)
    , config_(config)
    , stop_(false)
{
    create_participant();
    create_client(service_name);

    client_server_info("ClientApp", "Client initialized with ID: " << participant_->guid().guidPrefix);
}

ClientApp::~ClientApp()
{
    // As a precautionary measure, delete the server here because participant_->delete_contained_entities()
    // does not automatically disable the service. This line can be removed once the RPC internal API
    // supports service disabling.
    client_.reset();

    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_shared_instance()->delete_participant(participant_);
    }
}

void ClientApp::run()
{
    if (is_stopped())
    {
        return;
    }

    // Check if a server is available
    if (!ping_server(config_.connection_attempts))
    {
        // Stop the client execution
        if (!is_stopped())
        {
            client_server_info("ClientApp", "Server not reachable. Stopping client execution...");
            stop();
        }
    }

    if (!is_stopped())
    {
        try
        {
            // Server available. Execute the operation.
            set_operation();

            OperationStatus status = operation_->execute();

            while (OperationStatus::PENDING == status && !is_stopped())
            {
                // Wait before checking the next value
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                // Get the next value of the feed
                status = operation_->execute();
            }

            if (OperationStatus::SUCCESS != status)
            {
                throw std::runtime_error("Operation failed or interrupted");
            }

        }
        catch (std::runtime_error& e)
        {
            // Stop the client execution
            client_server_error("ClientApp", std::string(e.what()) +  ". Stopping client execution...");
            ClientApp::stop();
        }
    }

    if (!is_stopped())
    {
        // Stop the client execution
        client_server_info("ClientApp", "Operation finished. Stopping client execution...");
        ClientApp::stop();
    }
}

void ClientApp::stop()
{
    stop_.store(true);
    client_server_info("ClientApp", "Client execution stopped");
}

void ClientApp::create_participant()
{
    // Create the participant
    auto factory = DomainParticipantFactory::get_shared_instance();

    if (nullptr == factory)
    {
        throw std::runtime_error("Failed to get participant factory instance");
    }

    participant_ = factory->create_participant_with_default_profile();

    if (!participant_)
    {
        throw std::runtime_error("Participant initialization failed");
    }
}

void ClientApp::create_client(
        const std::string& service_name)
{
    RequesterQos qos;
    client_ = create_CalculatorClient(*participant_, service_name.c_str(), qos);

    if (!client_)
    {
        throw std::runtime_error("Failed to create client");
    }
}

void ClientApp::set_operation()
{
    // Set the operation based on the user input
    switch (config_.operation)
    {
        case CLIParser::OperationKind::ADDITION:
            operation_ = std::unique_ptr<Operation>(new Addition(client_, config_.x, config_.y));
            break;
        case CLIParser::OperationKind::SUBSTRACTION:
            operation_ = std::unique_ptr<Operation>(new Substraction(client_, config_.x, config_.y));
            break;
        case CLIParser::OperationKind::REPRESENTATION_LIMITS:
            operation_ = std::unique_ptr<Operation>(new RepresentationLimits(client_));
            break;
        case CLIParser::OperationKind::FIBONACCI:
            operation_ = std::unique_ptr<Operation>(new FibonacciSeq(client_, config_.n_results));
            break;
        case CLIParser::OperationKind::SUM_ALL:
            operation_ = std::unique_ptr<Operation>(new SumAll(client_));
            break;
        case CLIParser::OperationKind::ACCUMULATOR:
            operation_ = std::unique_ptr<Operation>(new Accumulator(client_));
            break;
        case CLIParser::OperationKind::FILTER:
            operation_ = std::unique_ptr<Operation>(new Filter(client_, config_.filter_kind));
            break;
        default:
            throw std::runtime_error("Invalid operation");
    }
}

bool ClientApp::ping_server(
        std::size_t attempts)
{
    operation_ = std::unique_ptr<Operation>(new Ping(client_));

    for (size_t i = 0; i < attempts; ++i)
    {
        if (!is_stopped())
        {
            client_server_debug("ClientApp",
                    "Trying to reach server, attempt " << (i + 1) << "/" << attempts);

            std::this_thread::sleep_for(std::chrono::seconds(1));

            if (OperationStatus::SUCCESS == operation_->execute())
            {
                return true;
            }

            // Server not reachable
            client_server_debug("ClientApp",
                    "Server not reachable, attempt " << (i + 1) << "/" << attempts << " failed.");

            if (i == attempts - 1)
            {
                client_server_error("ClientApp", "Failed to connect to server");
            }
        }
    }

    return false;
}

} // namespace rpc
} // namespace examples
} // namespace fastdds
} // namespace eprosima