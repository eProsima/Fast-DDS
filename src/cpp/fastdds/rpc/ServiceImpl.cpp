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

#include "ServiceImpl.hpp"
#include "RequesterImpl.hpp"
#include "ReplierImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

ServiceImpl::ServiceImpl(
        const std::string& service_name,
        const std::string& service_type_name,
        DomainParticipantImpl* participant)
        : service_name_(service_name),
        service_type_name_(service_type_name),
        participant_(participant)
{
    ReturnCode_t retcode = create_request_reply_topics();
    valid_ = (retcode == RETCODE_OK);
}

ServiceImpl::~ServiceImpl()
{

    // Remove all requesters/repliers
    remove_all_requesters();
    remove_all_repliers();
    
    // Remove filtered Reply topic
    if (reply_filtered_topic_)
    {
        participant_->delete_contentfilteredtopic(reply_filtered_topic_);
    }

    // Remove Request/reply topics
    if (request_topic_)
    {
        participant_->delete_topic(request_topic_);
    }

    if (reply_topic_)
    {
        participant_->delete_topic(reply_topic_);
    }

    // Unset participant
    participant_ = nullptr;
}

ReturnCode_t ServiceImpl::remove_requester(
        RequesterImpl* requester)
{
    std::lock_guard<std::mutex> lock(mtx_requesters_);
    auto it = std::find(requesters_.begin(), requesters_.end(), requester);

    if (it == requesters_.end())
    {
        // Requester not found
        EPROSIMA_LOG_ERROR(SERVICE, "Trying to remove a non-registered requester.");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    delete *it;
    requesters_.erase(it);
    return RETCODE_OK;
}

ReturnCode_t ServiceImpl::remove_replier(
        ReplierImpl* replier)
{
    std::lock_guard<std::mutex> lock(mtx_repliers_);
    auto it = std::find(repliers_.begin(), repliers_.end(), replier);
    if (it == repliers_.end())
    {
        // Replier not found
        EPROSIMA_LOG_ERROR(SERVICE, "Trying to remove a non-registered replier.");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    delete *it;
    repliers_.erase(it);
    return RETCODE_OK;
}

void ServiceImpl::remove_all_requesters()
{
    std::lock_guard<std::mutex> lock(mtx_requesters_);
    for (auto requester : requesters_)
    {
        delete requester;
    }
    requesters_.clear();
}

void ServiceImpl::remove_all_repliers()
{
    std::lock_guard<std::mutex> lock(mtx_repliers_);
    for (auto replier : repliers_)
    {
        delete replier;
    }
    repliers_.clear();
}

RequesterImpl* ServiceImpl::create_requester(
        const RequesterParams& params)
{
    // Check if parameters are valid
    if (!validate_params(params))
    {
        return nullptr;
    }
    
    RequesterImpl* requester(nullptr);

    try
    {
        requester = new RequesterImpl(this, params);
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Error creating Requester instance: " << e.what());
        return nullptr;
    }

    // Check that all requester dds entities are correctly created
    if (!requester->is_valid())
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Error creating requester DDS entities.");
        delete requester;
        return nullptr;
    }

    // Add requester to the service
    {
        std::lock_guard<std::mutex> lock(mtx_requesters_);
        requesters_.push_back(requester);
    }

    return requester;
}

ReplierImpl* ServiceImpl::create_replier(
        const ReplierParams& params)
{
    // Check if parameters are valid
    if (!validate_params(params))
    {
        return nullptr;
    }

    ReplierImpl* replier(nullptr);

    try
    {
        replier = new ReplierImpl(this, params);
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Error creating Replier instance: " << e.what());
        return nullptr;
    }

    // Check that all replier dds entities are correctly created
    if (!replier->is_valid())
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Error creating replier DDS entities.");
        delete replier;
        return nullptr;
    }

    // Add replier to the service
    {
        std::lock_guard<std::mutex> lock(mtx_repliers_);
        repliers_.push_back(replier);
    }

    return replier;
}

ReturnCode_t ServiceImpl::create_request_reply_topics()
{
    ServiceTypeSupport service_type = participant_->find_service_type(service_type_name_);
    if (service_type.empty())
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Service type '" << service_type_name_ << "' not registered.");
        return RETCODE_ERROR;
    }

    request_topic_ = participant_->create_topic(service_name_ + "_Request", service_type.request_type().get_type_name());

    if (!request_topic_)
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Error creating Request topic for Service '" << service_name_ << "'");
        return RETCODE_ERROR;
    }

    reply_topic_ = participant_->create_topic(service_name_ + "_Reply", service_type.reply_type().get_type_name());

    if (!reply_topic_)
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Error creating Reply topic for Service '" << service_name_ << "'");
        return RETCODE_ERROR;
    }

    reply_filtered_topic_ = participant_->create_contentfilteredtopic(
        service_name_ + "_ReplyFiltered",
        reply_topic_,
        " ",
        std::vector<std::string>(),
        "REQUEST_REPLY_CONTENT_FILTER");
    
    if (!reply_filtered_topic_)
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Error creating Content filtered Reply topic for Service '" << service_name_ << "'");
        return RETCODE_ERROR;
    }

    return RETCODE_OK;
}

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima