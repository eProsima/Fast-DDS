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
 * @file ClientApp.hpp
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_RPC_CLIENT_SERVER_FEED__CLIENTAPP_HPP
#define FASTDDS_EXAMPLES_CPP_RPC_CLIENT_SERVER_FEED__CLIENTAPP_HPP

#include <atomic>
#include <memory>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/rpc/interfaces/RpcClientReader.hpp>
#include <fastdds/dds/rpc/interfaces/RpcClientWriter.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "types/calculator.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rpc_client_server {

enum class OperationStatus
{
    PENDING,
    SUCCESS,
    TIMEOUT,
    ERROR
};

// TODO (Carlosespicur): Move operations to a different file?

class Operation
{

public:

    virtual OperationStatus execute() = 0;

};

class Ping : public Operation
{
public:

    Ping(
            std::shared_ptr<calculator_example::Calculator> client);

    OperationStatus execute() override;

private:

    std::weak_ptr<calculator_example::Calculator> client_;

};

class RepresentationLimits : public Operation
{

public:

    RepresentationLimits(
            std::shared_ptr<calculator_example::Calculator> client);

    OperationStatus execute() override;

private:

    std::int32_t min_value_;
    std::int32_t max_value_;
    std::weak_ptr<calculator_example::Calculator> client_;

};

class Addition : public Operation
{

public:

    Addition(
            std::shared_ptr<calculator_example::Calculator> client,
            std::int32_t x,
            std::int32_t y);

    OperationStatus execute() override;

private:

    std::int32_t x_;
    std::int32_t y_;
    std::int32_t result_;
    std::weak_ptr<calculator_example::Calculator> client_;

};

class Substraction : public Operation
{

public:

    Substraction(
            std::shared_ptr<calculator_example::Calculator> client,
            std::int32_t x,
            std::int32_t y);

    OperationStatus execute() override;

private:

    std::int32_t x_;
    std::int32_t y_;
    std::int32_t result_;
    std::weak_ptr<calculator_example::Calculator> client_;

};

class FibonacciSeq : public Operation
{

public:

    FibonacciSeq(
            std::shared_ptr<calculator_example::Calculator> client,
            std::uint32_t n_results);

    OperationStatus execute() override;

private:

    std::uint32_t n_results_;
    std::weak_ptr<calculator_example::Calculator> client_;
    std::shared_ptr<eprosima::fastdds::dds::rpc::RpcClientReader<int32_t>> reader_;

};

class SumAll : public Operation
{

public:

    SumAll(
            std::shared_ptr<calculator_example::Calculator> client);

    OperationStatus execute() override;

private:

    std::weak_ptr<calculator_example::Calculator> client_;
    std::shared_ptr<eprosima::fastdds::dds::rpc::RpcClientWriter<int32_t>> writer_;
    std::int32_t result_;
    bool input_feed_closed_;

};

class Accumulator : public Operation
{

public:

    Accumulator(
            std::shared_ptr<calculator_example::Calculator> client);

    OperationStatus execute() override;

private:

    std::weak_ptr<calculator_example::Calculator> client_;
    std::shared_ptr<eprosima::fastdds::dds::rpc::RpcClientWriter<int32_t>> writer_;
    std::shared_ptr<eprosima::fastdds::dds::rpc::RpcClientReader<int32_t>> reader_;
    bool valid_user_input_;

};

class Filter : public Operation
{

public:

    Filter(
            std::shared_ptr<calculator_example::Calculator> client,
            std::uint8_t filter_kind);

    OperationStatus execute() override;

private:

    std::weak_ptr<calculator_example::Calculator> client_;
    std::shared_ptr<eprosima::fastdds::dds::rpc::RpcClientWriter<int32_t>> writer_;
    std::shared_ptr<eprosima::fastdds::dds::rpc::RpcClientReader<int32_t>> reader_;
    std::uint8_t filter_kind_;
    bool input_feed_closed_;

};

class ClientApp : public Application
{
public:

    ClientApp(
            const CLIParser::config& config,
            const std::string& service_name);

    ~ClientApp() override;

    void run() override;

    void stop() override;

private:

    //! Create a participant for internal RPCDDS entities
    void create_participant();

    //! Create a client
    void create_client(const std::string& service_name);

    //! Test connection to the server. Returns true if the server is reachable and false otherwise
    bool ping_server(
            std::chrono::milliseconds wait_time = std::chrono::milliseconds(1000));

    //! Set the operation to be executed. If ping is true, a ping operation is set
    void set_operation(
            bool ping = false);

    bool is_stopped()
    {
        return stop_.load();
    }

    std::shared_ptr<calculator_example::Calculator> client_;
    dds::DomainParticipant* participant_;
    CLIParser::config config_;
    std::unique_ptr<Operation> operation_;
    std::atomic<bool> stop_;

};

} // namespace rpc_client_server
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_RPC_CLIENT_SERVER_FEED__CLIENTAPP_HPP