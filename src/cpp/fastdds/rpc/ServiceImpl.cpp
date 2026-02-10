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

#include <fastdds/dds/rpc/ServiceTypeSupport.hpp>

#include "ReplierImpl.hpp"
#include "RequesterImpl.hpp"
#include "RequestReplyContentFilterFactory.hpp"
#include "ServiceImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

ServiceImpl::ServiceImpl(
        const std::string& /*service_name*/,
        const std::string& /*service_type_name*/,
        DomainParticipantImpl* /*participant*/,
        PublisherImpl* /*service_publisher*/,
        SubscriberImpl* /*service_subscriber*/)
{
}

ServiceImpl::~ServiceImpl()
{
}

ReturnCode_t ServiceImpl::remove_requester(
        RequesterImpl* /*requester*/)
{
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t ServiceImpl::remove_replier(
        ReplierImpl* /*replier*/)
{
    return RETCODE_UNSUPPORTED;
}

RequesterImpl* ServiceImpl::create_requester(
        const RequesterQos& /*qos*/)
{
    return nullptr;
}

ReplierImpl* ServiceImpl::create_replier(
        const ReplierQos& /*qos*/)
{
    return nullptr;
}

ReturnCode_t ServiceImpl::enable()
{
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t ServiceImpl::close()
{
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t ServiceImpl::create_request_reply_topics()
{
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t ServiceImpl::delete_contained_entities()
{
    return RETCODE_UNSUPPORTED;
}

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima
