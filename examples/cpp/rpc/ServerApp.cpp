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

} // namespace rpc
} // namespace examples
} // namespace fastdds
} // namespace eprosima
