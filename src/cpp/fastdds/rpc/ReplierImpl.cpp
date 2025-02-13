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

#include "ReplierImpl.hpp"
#include "ServiceImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

ReplierImpl::ReplierImpl(
        ServiceImpl* service,
        const ReplierQos& qos)
        : replier_reader_(nullptr),
        replier_writer_(nullptr),
        replier_publisher_(nullptr),
        replier_subscriber_(nullptr),
        qos_(qos),
        service_(service),
        valid_(false)
{
    // Create required DDS entities. If any of them fails, the replier is not valid
    ReturnCode_t retcode = create_dds_entities(qos);
    valid_ = (retcode == RETCODE_OK);
}

ReplierImpl::~ReplierImpl()
{
    if (replier_subscriber_)
    {
        replier_subscriber_->delete_contained_entities();
        // delete replier_subscriber_;
        service_->participant_->delete_subscriber(replier_subscriber_);
    }

    if (replier_publisher_)
    {
        replier_publisher_->delete_contained_entities();
        service_->participant_->delete_publisher(replier_publisher_);
    }

    service_ = nullptr;
}

const std::string& ReplierImpl::get_service_name() const
{
    return service_->service_name_;
}

ReturnCode_t ReplierImpl::send_reply(
        void* data,
        RequestInfo& info)
{
    // TODO Implement matching algorithm
    rtps::WriteParams wparams;
    wparams.related_sample_identity(info.related_sample_identity);

    return replier_writer_->write(data, wparams);
}

ReturnCode_t ReplierImpl::take_request(
        void* data,
        RequestInfo& info)
{
    // TODO Implement matching algorithm
    ReturnCode_t retcode;
    retcode = replier_reader_->take_next_sample(data, &info);
    // Related sample identity is stored in sample_indentity member of info. Change it to related_sample_identity
    info.related_sample_identity = info.sample_identity;

    return retcode;
}

ReturnCode_t ReplierImpl::take_request(
        LoanableCollection& data,
        LoanableSequence<RequestInfo>& info)
{
    // TODO Implement matching algorithm
    ReturnCode_t retcode;
    retcode = replier_reader_->take(data, info);
    
    // Fill related_sample_identity attribute
    for (LoanableCollection::size_type i = 0; i < info.length(); ++i)
    {
        info[i].related_sample_identity = info[i].sample_identity;
    }

    return retcode;
}

ReturnCode_t ReplierImpl::create_dds_entities(const ReplierQos& qos)
{
    if (!service_)
    {
        // Service is not configured properly, so the replier does not have access to a valid DomainParticipant
        EPROSIMA_LOG_ERROR(REPLIER, "Service is nullptr");
        return RETCODE_ERROR;
    }
    
    // Create publisher and writer for the Reply topic
    replier_publisher_ = service_->participant_->create_publisher(PUBLISHER_QOS_DEFAULT);

    if (!replier_publisher_)
    {
        EPROSIMA_LOG_ERROR(REPLIER, "Error creating replier publisher");
        return RETCODE_ERROR;
    }

    replier_writer_ = replier_publisher_->create_datawriter(service_->reply_topic_, qos.writer_qos);

    if (!replier_writer_)
    {
        EPROSIMA_LOG_ERROR(REPLIER, "Error creating replier writer");
        return RETCODE_ERROR;
    }

    // Create subscriber and reader for the Reply topic
    replier_subscriber_ = service_->participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    if (!replier_subscriber_)
    {
        EPROSIMA_LOG_ERROR(REPLIER, "Error creating replier subscriber");
        return RETCODE_ERROR;
    }

    replier_reader_ = replier_subscriber_->create_datareader(service_->request_topic_, qos.reader_qos);

    if (!replier_reader_)
    {
        EPROSIMA_LOG_ERROR(REPLIER, "Error creating reply reader");
        return RETCODE_ERROR;
    }

    return RETCODE_OK;
}

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima