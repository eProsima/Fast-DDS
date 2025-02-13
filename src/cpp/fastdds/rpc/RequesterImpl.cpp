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

#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/WriteParams.hpp>

#include "RequesterImpl.hpp"
#include "ServiceImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

RequesterImpl::RequesterImpl(
        ServiceImpl* service,
        const RequesterQos& qos)
        : requester_reader_(nullptr),
        requester_writer_(nullptr),
        requester_publisher_(nullptr),
        requester_subscriber_(nullptr),
        qos_(qos),
        service_(service),
        valid_(false)
{
    // Create required DDS entities. If any of them fails, the requester is not valid
    ReturnCode_t retcode = create_dds_entities(qos);
    valid_ = (retcode == RETCODE_OK);
}

RequesterImpl::~RequesterImpl()
{
    if (requester_subscriber_)
    {
        requester_subscriber_->delete_contained_entities();
        service_->participant_->delete_subscriber(requester_subscriber_);
    }

    if (requester_publisher_)
    {
        requester_publisher_->delete_contained_entities();
        service_->participant_->delete_publisher(requester_publisher_);
    }

    service_ = nullptr;
}

const std::string& RequesterImpl::get_service_name() const
{
    return service_->service_name_;
}

ReturnCode_t RequesterImpl::send_request(
        void* data,
        RequestInfo& info)
{
    // TODO: Implement here the endpoint matching algorithm
    rtps::WriteParams wparams;
    ReturnCode_t retcode;

    retcode = requester_writer_->write(data, wparams);
    // Fill RequestInfo's related sample identity with the information expected for the corresponding reply
    info.related_sample_identity = wparams.sample_identity();

    return retcode;
}

ReturnCode_t RequesterImpl::take_reply(
        void* data,
        RequestInfo& info)
{
    // TODO: Implement here the endpoint matching algorithm
    return requester_reader_->take_next_sample(data, &info);
}

ReturnCode_t RequesterImpl::take_reply(
        LoanableCollection& data,
        LoanableSequence<RequestInfo>& info)
{
    // TODO: Implement here the endpoint matching algorithm
    return requester_reader_->take(data, info);
}

ReturnCode_t RequesterImpl::create_dds_entities(const RequesterQos& qos)
{
    if (!service_)
    {
        // Service is not configured properly, so the requester does not have access to a valid DomainParticipant
        EPROSIMA_LOG_ERROR(REQUESTER, "Service is nullptr");
        return RETCODE_ERROR;
    }
    
    // Create publisher and writer for the Request topic
    requester_publisher_ = service_->participant_->create_publisher(PUBLISHER_QOS_DEFAULT);

    if (!requester_publisher_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Error creating request publisher");
        return RETCODE_ERROR;
    }

    requester_writer_ = requester_publisher_->create_datawriter(service_->request_topic_, qos.writer_qos);

    if (!requester_writer_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Error creating request writer");
        return RETCODE_ERROR;
    }

    // Create subscriber and reader for the Reply topic
    requester_subscriber_ = service_->participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    if (!requester_subscriber_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Error creating reply subscriber");
        return RETCODE_ERROR;
    }

    requester_reader_ = requester_subscriber_->create_datareader(service_->reply_filtered_topic_, qos.reader_qos);

    if (!requester_reader_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Error creating reply reader");
        return RETCODE_ERROR;
    }

    return RETCODE_OK;
}

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima
