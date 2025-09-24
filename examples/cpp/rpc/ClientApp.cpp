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
#include "types/calculatorClient.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rpc {

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
                    "Representation limits received: min_value = " << representation_limits_.min_value << ", max_value = " <<
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