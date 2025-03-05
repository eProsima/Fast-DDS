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
        const std::string& service_name,
        const std::string& service_type_name,
        DomainParticipantImpl* participant,
        PublisherImpl* service_publisher,
        SubscriberImpl* service_subscriber)
    : service_name_(service_name)
    , service_type_name_(service_type_name)
    , request_topic_name_(service_name + "_Request")
    , request_type_name_(service_type_name + "_Request")
    , reply_topic_name_(service_name + "_Reply")
    , reply_type_name_(service_type_name + "_Reply")
    , reply_filtered_topic_name_(service_name + "_ReplyFiltered")
    , participant_(participant)
    , service_publisher_(service_publisher)
    , service_subscriber_(service_subscriber)
    , enabled_(false)
{
}

ServiceImpl::~ServiceImpl()
{
    delete_contained_entities();

    // Unset participant, publisher and subscriber (shared with other services)
    participant_ = nullptr;
    service_publisher_ = nullptr;
    service_subscriber_ = nullptr;
}

ReturnCode_t ServiceImpl::remove_requester(
        RequesterImpl* requester)
{
    ReturnCode_t ret = RETCODE_OK;
    std::lock_guard<std::mutex> lock(mtx_requesters_);
    auto it = std::find(requesters_.begin(), requesters_.end(), requester);

    if (it == requesters_.end())
    {
        // Requester not found
        EPROSIMA_LOG_ERROR(SERVICE, "Trying to remove a non-registered requester.");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    // In case the requester is enabled, disable it first
    ret = (*it)->close();

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Error closing Requester for Service '" << service_name_ << "'");
        return ret;
    }

    delete *it;
    requesters_.erase(it);
    return RETCODE_OK;
}

ReturnCode_t ServiceImpl::remove_replier(
        ReplierImpl* replier)
{
    ReturnCode_t ret = RETCODE_OK;
    std::lock_guard<std::mutex> lock(mtx_repliers_);
    auto it = std::find(repliers_.begin(), repliers_.end(), replier);
    if (it == repliers_.end())
    {
        // Replier not found
        EPROSIMA_LOG_ERROR(SERVICE, "Trying to remove a non-registered replier.");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    // In case the replier is enabled, disable it first
    ret = (*it)->close();

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Error closing Replier for Service '" << service_name_ << "'");
        return ret;
    }

    delete *it;
    repliers_.erase(it);
    return RETCODE_OK;
}

RequesterImpl* ServiceImpl::create_requester(
        const RequesterQos& qos)
{
    // Check if parameters are valid
    if (!validate_qos(qos))
    {
        return nullptr;
    }

    RequesterImpl* requester(nullptr);

    try
    {
        // create a new Requester (disabled by default)
        requester = new RequesterImpl(this, qos);
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Error creating Requester instance: " << e.what());
        return nullptr;
    }

    // If the service is active, try to enable the requester
    if (enabled_ && (RETCODE_OK != requester->enable()))
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
        const ReplierQos& qos)
{
    // Check if parameters are valid
    if (!validate_qos(qos))
    {
        return nullptr;
    }

    ReplierImpl* replier(nullptr);

    try
    {
        // create a new Replier (disabled by default)
        replier = new ReplierImpl(this, qos);
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Error creating Replier instance: " << e.what());
        return nullptr;
    }

    // If the service is active, try to enable the replier
    if (enabled_ && (RETCODE_OK != replier->enable()))
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

ReturnCode_t ServiceImpl::enable()
{
    ReturnCode_t ret = RETCODE_OK;

    if (!enabled_)
    {
        // Create request/reply topics
        ret = create_request_reply_topics();

        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(SERVICE, "Error creating request/reply topics for Service '" << service_name_ << "'");
            return ret;
        }

        enabled_ = true;

        {
            // Try to enable internal Requesters.
            // It will also create the DDS entities in the Service Request/Reply topics.
            // If something goes wrong, Requester remains disabled and show an error message.
            std::lock_guard<std::mutex> lock(mtx_requesters_);
            for (auto requester : requesters_)
            {
                ret = requester->enable();

                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(SERVICE, "Error enabling Requester for Service '" << service_name_ << "'");
                }
                else
                {
                    EPROSIMA_LOG_INFO(SERVICE, "Requester for Service '" << service_name_ << "' enabled.");
                }
            }
        }

        {
            // Enable internal Repliers.
            // It will also create the DDS entities in the Service Request/Reply topics
            std::lock_guard<std::mutex> lock(mtx_repliers_);
            for (auto replier : repliers_)
            {
                ret = replier->enable();

                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(SERVICE, "Error enabling Replier for Service '" << service_name_ << "'");
                }
                else
                {
                    EPROSIMA_LOG_INFO(SERVICE, "Replier for Service '" << service_name_ << "' enabled.");
                }
            }
        }
    }

    return ret;
}

ReturnCode_t ServiceImpl::close()
{
    ReturnCode_t ret = RETCODE_OK;

    if (enabled_)
    {
        // If Service contains requesters or repliers, disable them first
        {
            std::lock_guard<std::mutex> lock(mtx_requesters_);
            for (auto requester : requesters_)
            {
                ret = requester->close();

                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(SERVICE, "Error closing Requester for Service '" << service_name_ << "'");
                    return ret;
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(mtx_repliers_);
            for (auto replier : repliers_)
            {
                ret = replier->close();

                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(SERVICE, "Error closing Replier for Service '" << service_name_ << "'");
                    return ret;
                }
            }
        }

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

        enabled_ = false;
    }

    return ret;
}

ReturnCode_t ServiceImpl::create_request_reply_topics()
{
    ServiceTypeSupport service_type = participant_->find_service_type(service_type_name_);
    if (service_type.empty_types())
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Service type '" << service_type_name_ << "' not registered.");
        return RETCODE_ERROR;
    }

    request_topic_ = participant_->create_topic(request_topic_name_, request_type_name_);

    if (!request_topic_)
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Error creating Request topic for Service '" << service_name_ << "'");
        return RETCODE_ERROR;
    }

    reply_topic_ = participant_->create_topic(reply_topic_name_, reply_type_name_);

    if (!reply_topic_)
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Error creating Reply topic for Service '" << service_name_ << "'");
        return RETCODE_ERROR;
    }

    reply_filtered_topic_ = participant_->create_contentfilteredtopic(
        reply_filtered_topic_name_,
        reply_topic_,
        " ",
        std::vector<std::string>(),
        rpc::RequestReplyContentFilterFactory::FILTER_NAME);

    if (!reply_filtered_topic_)
    {
        EPROSIMA_LOG_ERROR(SERVICE,
                "Error creating Content filtered Reply topic for Service '" << service_name_ << "'");
        return RETCODE_ERROR;
    }

    return RETCODE_OK;
}

ReturnCode_t ServiceImpl::delete_contained_entities()
{
    ReturnCode_t ret = RETCODE_OK;

    // Close the Service
    ret = close();

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(SERVICE, "Error closing Service '" << service_name_ << "'");
        return ret;
    }

    // Remove all requesters
    {
        std::lock_guard<std::mutex> lock(mtx_requesters_);
        for (auto requester : requesters_)
        {
            delete requester;
        }
        requesters_.clear();
    }

    // Remove all repliers
    {
        std::lock_guard<std::mutex> lock(mtx_repliers_);
        for (auto replier : repliers_)
        {
            delete replier;
        }
        repliers_.clear();
    }

    return ret;
}

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima