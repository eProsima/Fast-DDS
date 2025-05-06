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
namespace rpc_client_server {

using namespace calculator_example;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::rpc;

ServerApp::ServerApp(
        const CLIParser::config& config,
        const std::string& service_name)
    : server_(nullptr)
    , participant_(nullptr)
    , thread_pool_size_(config.thread_pool_size)
    , stop_(false)
{
    create_participant();
    create_server(service_name);

    client_server_info("ServerApp", "Server initialized with ID: " << participant_->guid().guidPrefix);
}

ServerApp::~ServerApp()
{
    // TODO (Carlosespicur): deleting the server manually here is necessary because participant_->delete_contained_entities()
    // does not disable the service. Delete the following line when the bug is fixed in RPC internal API
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

    DomainParticipantExtendedQos participant_qos;
    factory->get_participant_extended_qos_from_default_profile(participant_qos);

    participant_qos.user_data().data_vec().push_back(static_cast<uint8_t>(CLIParser::EntityKind::SERVER));

    participant_ = factory->create_participant(participant_qos.domainId(), participant_qos);

    if (!participant_)
    {
        throw std::runtime_error("Participant initialization failed");
    }
}

void ServerApp::create_server(
        const std::string& service_name)
{
    // Create the server with default QoS
    std::shared_ptr<CalculatorServer_IServerImplementation> server_impl =
            std::make_shared<CalculatorServerImplementation>();

    server_ = create_CalculatorServer(
                    *participant_,
                    service_name.c_str(),
                    ReplierQos(),
                    thread_pool_size_,
                    server_impl);

    if (!server_)
    {
        throw std::runtime_error("Server initialization failed");
    }
}

} // namespace rpc_client_server
} // namespace examples
} // namespace fastdds
} // namespace eprosima