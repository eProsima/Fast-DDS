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

#include "RequesterImpl.hpp"

#include <string>

#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/rpc/RequestInfo.hpp>
#include <fastdds/dds/topic/ContentFilteredTopic.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/WriteParams.hpp>

#include "ServiceImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

RequesterImpl::RequesterImpl(
        ServiceImpl* service,
        const RequesterQos& qos)
    : requester_reader_(nullptr)
    , requester_writer_(nullptr)
    , qos_(qos)
    , service_(service)
    , enabled_(false)
    , matched_status_changed_(false)
{
}

RequesterImpl::~RequesterImpl()
{
    close();
    service_ = nullptr;
}

const std::string& RequesterImpl::get_service_name() const
{
    return service_->get_service_name();
}

ReturnCode_t RequesterImpl::send_request(
        void* data,
        RequestInfo& info)
{
    if (!enabled_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Trying to send a request with a disabled requester");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    Duration_t timeout{3, 0}; // Default timeout of 3 seconds
    if (!wait_for_matching(timeout))
    {
        EPROSIMA_LOG_WARNING(REQUESTER, "Trying to send a request with an unmatched requester");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    rtps::WriteParams wparams;
    wparams.related_sample_identity(info.related_sample_identity);
    ReturnCode_t ret = requester_writer_->write(data, wparams);
    if (RETCODE_OK == ret)
    {
        // Fill RequestInfo's related sample identity with the information expected for the corresponding reply
        info.sample_identity = wparams.sample_identity();

        // Set the full related sample identity when the writer guid is unknown
        if (rtps::GUID_t::unknown() == info.related_sample_identity.writer_guid())
        {
            info.related_sample_identity = wparams.related_sample_identity();
        }

        // Set the sequence number of the related sample identity when it is unknown
        if (rtps::SequenceNumber_t::unknown() == info.related_sample_identity.sequence_number())
        {
            info.related_sample_identity.sequence_number() = wparams.related_sample_identity().sequence_number();
        }
    }

    return ret;
}

ReturnCode_t RequesterImpl::take_reply(
        void* data,
        RequestInfo& info)
{
    if (!enabled_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Trying to take a reply with a disabled requester");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    // For taking the reply is enough to check if the requester is partially matched
    if (replier_match_status() == ReplierMatchStatus::UNMATCHED)
    {
        EPROSIMA_LOG_WARNING(REQUESTER, "Trying to take a reply with an unmatched requester reader");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    return requester_reader_->take_next_sample(data, &info);
}

ReturnCode_t RequesterImpl::take_reply(
        LoanableCollection& data,
        LoanableSequence<RequestInfo>& info)
{
    if (!enabled_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Trying to take a reply with a disabled requester");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    // For taking the reply is enough to check if the requester is partially matched
    if (replier_match_status() == ReplierMatchStatus::UNMATCHED)
    {
        EPROSIMA_LOG_WARNING(REQUESTER, "Trying to take a reply with an unmatched requester");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    return requester_reader_->take(data, info);
}

ReturnCode_t RequesterImpl::return_loan(
        LoanableCollection& data,
        LoanableSequence<RequestInfo>& info)
{
    if (!enabled_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Trying to return a loan with a disabled requester");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    return requester_reader_->return_loan(data, info);
}

ReturnCode_t RequesterImpl::enable()
{
    ReturnCode_t retcode = RETCODE_OK;

    if (!enabled_)
    {
        if (!service_)
        {
            EPROSIMA_LOG_ERROR(REQUESTER, "Service is nullptr");
            return RETCODE_ERROR;
        }

        if (!service_->is_enabled())
        {
            EPROSIMA_LOG_ERROR(REQUESTER, "Trying to enable Requester on a disabled Service");
            return RETCODE_PRECONDITION_NOT_MET;
        }

        retcode = create_dds_entities(qos_);

        if (RETCODE_OK != retcode)
        {
            EPROSIMA_LOG_ERROR(REQUESTER, "Error creating DDS entities");
            // If any error occurs, delete the created entities
            // This is necessary to avoid keeping requester in an inconsistent state
            delete_contained_entities();
            return retcode;
        }
        else
        {
            requester_reader_->enable();
            requester_writer_->enable();
        }

        enabled_ = true;
    }

    return retcode;
}

ReturnCode_t RequesterImpl::close()
{
    ReturnCode_t retcode = RETCODE_OK;

    if (enabled_)
    {
        retcode = delete_contained_entities();

        if (RETCODE_OK != retcode)
        {
            EPROSIMA_LOG_ERROR(REQUESTER, "Error deleting DDS entities");
            return retcode;
        }

        enabled_ = false;
    }

    return retcode;
}

void RequesterImpl::on_publication_matched(
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& /*info*/)
{

    matched_status_changed_.store(true);
    cv_.notify_one();
}

void RequesterImpl::on_subscription_matched(
        DataReader* /*reader*/,
        const SubscriptionMatchedStatus& /*info*/)
{
    matched_status_changed_.store(true);
    cv_.notify_one();
}

ReturnCode_t RequesterImpl::create_dds_entities(
        const RequesterQos& qos)
{
    // Entities are not autoenabled since the publisher and
    // the subscriber have autoenable_created_entities set to false.

    // Create writer for the Request topic
    requester_writer_ =
            service_->get_publisher()->create_datawriter(
        service_->get_request_topic(), qos.writer_qos, this, StatusMask::publication_matched());

    if (!requester_writer_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Error creating request writer");
        return RETCODE_ERROR;
    }

    ContentFilteredTopic* reply_topic = service_->get_reply_filtered_topic();

    requester_reader_ =
            service_->get_subscriber()->create_datareader(
        reply_topic, qos.reader_qos, this, StatusMask::subscription_matched());

    if (!requester_reader_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Error creating reply reader");
        return RETCODE_ERROR;
    }

    // Set the content filter signature to be different from the one used in other requesters
    std::stringstream guid;
    guid << requester_reader_->guid();
    std::vector<std::string> expression_parameters;
    reply_topic->set_filter_expression(guid.str(), expression_parameters);

    // Set the related entity key on both entities
    requester_reader_->set_related_datawriter(requester_writer_);
    requester_writer_->set_related_datareader(requester_reader_);

    return RETCODE_OK;
}

ReturnCode_t RequesterImpl::delete_contained_entities()
{
    // Check if DataWriter and DataReader can be deleted.
    // If not, do nothing and return an error code
    if (requester_writer_)
    {
        if (!service_->get_publisher()->can_be_deleted(requester_writer_))
        {
            EPROSIMA_LOG_ERROR(REQUESTER, "Requester DataWriter cannot be deleted");
            return RETCODE_PRECONDITION_NOT_MET;
        }
    }

    if (requester_reader_)
    {
        if (!service_->get_subscriber()->can_be_deleted(requester_reader_))
        {
            EPROSIMA_LOG_ERROR(REQUESTER, "Requester DataReader cannot be deleted");
            return RETCODE_PRECONDITION_NOT_MET;
        }
    }

    // Delete DataWriter and DataReader
    service_->get_publisher()->delete_datawriter(requester_writer_);
    requester_writer_ = nullptr;
    service_->get_subscriber()->delete_datareader(requester_reader_);
    requester_reader_ = nullptr;

    return RETCODE_OK;
}

RequesterImpl::ReplierMatchStatus RequesterImpl::replier_match_status() const
{
    ReplierMatchStatus replier_match_status = ReplierMatchStatus::UNMATCHED;

    SubscriptionMatchedStatus sub_status;

    if (RETCODE_OK == requester_reader_->get_subscription_matched_status(sub_status))
    {
        if (sub_status.current_count > 0)
        {
            replier_match_status = ReplierMatchStatus::PARTIALLY_MATCHED;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Error getting matched subscriptions status");
    }

    if (replier_match_status == ReplierMatchStatus::PARTIALLY_MATCHED)
    {
        PublicationMatchedStatus pub_status;
        if (RETCODE_OK == requester_writer_->get_publication_matched_status(pub_status))
        {
            if (pub_status.current_count > 0 &&
                    pub_status.current_count == sub_status.current_count)
            {
                replier_match_status = ReplierMatchStatus::MATCHED;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(REQUESTER, "Error getting matched publications status");
        }
    }

    return replier_match_status;
}

bool RequesterImpl::wait_for_matching(
        const fastdds::dds::Duration_t& timeout)
{
    Time_t current_time;
    Time_t::now(current_time);
    Time_t finish_time = current_time + timeout;

    ReplierMatchStatus replier_status = replier_match_status();
    while ((ReplierMatchStatus::MATCHED != replier_status) && current_time < finish_time)
    {
        // Wait for the matched status to change
        // Or every 100 milliseconds.
        std::unique_lock<std::mutex> lock(mtx_);
        bool res = cv_.wait_for(lock,
                        std::chrono::milliseconds(100),
                        [this]()
                        {
                            return matched_status_changed_.load();
                        });

        if (res)
        {
            // Reset the matched status changed flag
            matched_status_changed_.store(false);

            // Check if the replier is fully matched
            replier_status = replier_match_status();
        }

        // Update the current time
        Time_t::now(current_time);
    }

    return (ReplierMatchStatus::MATCHED == replier_status);
}

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima
