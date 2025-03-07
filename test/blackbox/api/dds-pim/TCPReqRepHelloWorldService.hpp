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

#ifndef _TEST_BLACKBOX_TCPREQREPHELLOWORLDSERVICE_HPP_
#define _TEST_BLACKBOX_TCPREQREPHELLOWORLDSERVICE_HPP_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/rpc/Service.hpp>

class TCPReqRepHelloWorldService
{

public:

    TCPReqRepHelloWorldService();

    virtual ~TCPReqRepHelloWorldService() = default;

    eprosima::fastdds::dds::rpc::Service* init(
            eprosima::fastdds::dds::DomainParticipant* participant);

    const std::string& service_name() const
    {
        return service_name_;
    }

    const std::string& service_type_name() const
    {
        return service_type_name_;
    }

    const eprosima::fastdds::dds::rpc::ServiceTypeSupport& service_type() const
    {
        return service_type_;
    }

private:

    std::string service_name_;
    std::string service_type_name_;
    eprosima::fastdds::dds::rpc::ServiceTypeSupport service_type_;

};

#endif // _TEST_BLACKBOX_TCPREQREPHELLOWORLDSERVICE_HPP_