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

#ifndef FASTDDS_EXAMPLES_CPP_RPC__SERVERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_RPC__SERVERAPP_HPP

#include <atomic>
#include <memory>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "types/calculatorServerImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rpc {

class ServerApp : public Application
{

public:

    ServerApp(
            const CLIParser::config& config,
            const std::string& service_name);

    ~ServerApp() override;

    void run() override;

    void stop() override;

    bool is_stopped() const override
    {
        return stop_.load();
    }

protected:

    void create_participant();

    void create_server(
            const std::string& server_name);

private:

    class ServerImpl : public calculator_example::CalculatorServerImplementation,
        public std::enable_shared_from_this<ServerImpl>
    {

    public:

        static std::shared_ptr<ServerImpl> create()
        {
            return std::make_shared<ServerImpl>(ServerImpl());
        }

        std::shared_ptr<ServerImpl> ptr()
        {
            return shared_from_this();
        }

    private:

        explicit ServerImpl() = default;

        calculator_example::detail::Calculator_representation_limits_Out representation_limits(
                const eprosima::fastdds::dds::rpc::RpcRequest& info) override;

        int32_t addition(
                const eprosima::fastdds::dds::rpc::RpcRequest& info,
                /*in*/ int32_t value1,
                /*in*/ int32_t value2) override;

        int32_t subtraction(
                const eprosima::fastdds::dds::rpc::RpcRequest& info,
                /*in*/ int32_t value1,
                /*in*/ int32_t value2) override;

    };

    std::shared_ptr<ServerImpl> server_impl_;
    std::shared_ptr<eprosima::fastdds::dds::rpc::RpcServer> server_;
    dds::DomainParticipant* participant_;
    size_t thread_pool_size_;
    std::atomic<bool> stop_;
    std::thread timeout_thread_;

};

} // namespace rpc
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_RPC__SERVERAPP_HPP
