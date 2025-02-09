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

#include <asio.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/rpc/ServiceTypeSupport.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "TCPReqRepHelloWorldService.hpp"
#include "../../types/HelloWorldPubSubTypes.hpp"

#if defined(_WIN32)
#define GET_PID _getpid
#include <process.h>
#else
#define GET_PID getpid
#endif // if defined(_WIN32)

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::rpc;

Service* TCPReqRepHelloWorldService::init(
        DomainParticipant* participant)
{
    std::ostringstream service_name;
    service_name << "TCPReqRepHelloWorldService_" << asio::ip::host_name() << "_" << GET_PID();
    std::string service_type_name = "TCPReqRepHelloWorldServiceType";
    ServiceTypeSupport service_type(TypeSupport(new HelloWorldPubSubType()), TypeSupport(new HelloWorldPubSubType()));

    assert(participant->register_service_type(service_type, service_type_name) == RETCODE_OK);
    return participant->create_service(service_name.str(), service_type_name);
}
