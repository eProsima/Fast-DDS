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

#ifndef FASTDDS_EXAMPLES_CPP_RPC_CLIENT_SERVER_BASIC__SERVERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_RPC_CLIENT_SERVER_BASIC__SERVERAPP_HPP

#include <atomic>
#include <memory>
#include <string>

#include "fastdds/dds/domain/DomainParticipant.hpp"

#include "Application.hpp"
#include "CLIParser.hpp"
#include "types/calculatorServer.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rpc_client_server {

class ServerApp : public Application
{

public:

    ServerApp(
            const CLIParser::config& config,
            const std::string& service_name);

    ~ServerApp() override;

    void run() override;

    void stop() override;

protected:

    void create_participant();

    void create_server(
            const std::string& server_name);

    bool is_stopped()
    {
        return stop_.load();
    }

    std::shared_ptr<calculator_example::CalculatorServer> server_;
    dds::DomainParticipant* participant_;
    size_t thread_pool_size_;
    std::atomic<bool> stop_;

};

}
}
}
}

#endif // FASTDDS_EXAMPLES_CPP_RPC_CLIENT_SERVER_BASIC__SERVERAPP_HPP