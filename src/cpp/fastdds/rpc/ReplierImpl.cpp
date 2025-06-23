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

#include "ReplierImpl.hpp"

#include <string>

#include <fastdds/dds/builtin/topic/PublicationBuiltinTopicData.hpp>
#include <fastdds/dds/builtin/topic/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/dds/core/condition/Condition.hpp>
#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/qos/ReplierQos.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/rpc/RequestInfo.hpp>
#include <fastdds/rtps/common/SampleIdentity.hpp>
#include <fastdds/rtps/common/WriteParams.hpp>

#include "ServiceImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * @brief Fills the related sample identity of the request.
 *
 * This will fill the related sample identity of the request with values taken from the sample identity.
 * Values different from unknown are preserved.
 *
 * @param info [in,out] The request information to update.
 */
static void fill_related_sample_identity(
        RequestInfo& info)
{
    // When sending a reply, the code here expects that related_sample_identity
    // has the sample_identity of the corresponding request.

    static const rtps::SampleIdentity unknown_identity = rtps::SampleIdentity::unknown();

    // If the related guid is unknown, we consider that the request is not related to a previous one,
    // so we set the related sample identity to the received sample identity
    if (unknown_identity.writer_guid() == info.related_sample_identity.writer_guid())
    {
        info.related_sample_identity = info.sample_identity;
        return;
    }

    // There is a special case where only the related guid is set.
    // This is used in ROS 2 to convey the GUID of the reply reader.
    // In this case we just set the sequence number of the related sample identity
    if (unknown_identity.sequence_number() == info.related_sample_identity.sequence_number())
    {
        info.related_sample_identity.sequence_number() = info.sample_identity.sequence_number();
    }
}

ReplierImpl::ReplierImpl(
        ServiceImpl* service,
        const ReplierQos& qos)
    : replier_reader_(nullptr)
    , replier_writer_(nullptr)
    , qos_(qos)
    , service_(service)
    , enabled_(false)
    , matched_status_changed_(false)
{
}

ReplierImpl::~ReplierImpl()
{
    close();
    service_ = nullptr;
}

const std::string& ReplierImpl::get_service_name() const
{
    return service_->get_service_name();
}

ReturnCode_t ReplierImpl::send_reply(
        void* data,
        const RequestInfo& info)
{
    if (!enabled_)
    {
        EPROSIMA_LOG_ERROR(REPLIER, "Trying to send a reply with a disabled replier");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    Time_t timeout{3, 0}; // Default timeout of 3 seconds
    auto match_status = wait_for_matching(timeout, info);
    if (RequesterMatchStatus::UNMATCHED == match_status)
    {
        // The writer that sent the request has been unmatched.
        EPROSIMA_LOG_WARNING(REPLIER, "Trying to send a reply to a disconnected requester");
        return RETCODE_NO_DATA;
    }
    else if (RequesterMatchStatus::PARTIALLY_MATCHED == match_status)
    {
        // The writer that sent the request is still matched, but the reply topic is not.
        EPROSIMA_LOG_WARNING(REPLIER, "Trying to send a reply to a partially matched requester");
        return RETCODE_TIMEOUT;
    }

    rtps::WriteParams wparams;
    wparams.related_sample_identity(info.related_sample_identity);
    wparams.has_more_replies(info.has_more_replies);

    return replier_writer_->write(data, wparams);
}

ReturnCode_t ReplierImpl::take_request(
        void* data,
        RequestInfo& info)
{
    ReturnCode_t retcode;

    if (!enabled_)
    {
        EPROSIMA_LOG_ERROR(REPLIER, "Trying to take a request with a disabled replier");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    retcode = replier_reader_->take_next_sample(data, &info);
    fill_related_sample_identity(info);

    return retcode;
}

ReturnCode_t ReplierImpl::take_request(
        LoanableCollection& data,
        LoanableSequence<RequestInfo>& info)
{
    ReturnCode_t retcode;

    if (!enabled_)
    {
        EPROSIMA_LOG_ERROR(REPLIER, "Trying to take a request with a disabled replier");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    retcode = replier_reader_->take(data, info);

    // Fill related_sample_identity attribute
    for (LoanableCollection::size_type i = 0; i < info.length(); ++i)
    {
        fill_related_sample_identity(info[i]);
    }

    return retcode;
}

ReturnCode_t ReplierImpl::return_loan(
        LoanableCollection& data,
        LoanableSequence<RequestInfo>& info)
{
    if (!enabled_)
    {
        EPROSIMA_LOG_ERROR(REPLIER, "Trying to return loan with a disabled replier");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    return replier_reader_->return_loan(data, info);
}

ReturnCode_t ReplierImpl::enable()
{
    ReturnCode_t retcode = RETCODE_OK;

    if (!enabled_)
    {
        if (!service_)
        {
            EPROSIMA_LOG_ERROR(REPLIER, "Service is nullptr");
            return RETCODE_ERROR;
        }

        if (!service_->is_enabled())
        {
            EPROSIMA_LOG_ERROR(REPLIER, "Trying to enable Replier on a disabled Service");
            return RETCODE_PRECONDITION_NOT_MET;
        }

        retcode = create_dds_entities(qos_);

        if (RETCODE_OK != retcode)
        {
            EPROSIMA_LOG_ERROR(REPLIER, "Unable to enable replier");
            // If any error occurs, delete the created entities
            // This is necessary to avoid keeping requester in an inconsistent state
            delete_contained_entities();
            return retcode;
        }
        else
        {
            replier_reader_->enable();
            replier_writer_->enable();
        }

        enabled_ = true;
    }

    return retcode;
}

ReturnCode_t ReplierImpl::close()
{
    ReturnCode_t retcode = RETCODE_OK;

    if (enabled_)
    {
        retcode = delete_contained_entities();

        if (RETCODE_OK != retcode)
        {
            EPROSIMA_LOG_ERROR(REPLIER, "Error deleting DDS entities");
            return retcode;
        }

        enabled_ = false;
    }

    return retcode;
}

void ReplierImpl::on_publication_matched(
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& /*info*/)
{

    matched_status_changed_.store(true);
    cv_.notify_one();
}

void ReplierImpl::on_subscription_matched(
        DataReader* /*reader*/,
        const SubscriptionMatchedStatus& /*info*/)
{
    matched_status_changed_.store(true);
    cv_.notify_one();
}

ReturnCode_t ReplierImpl::create_dds_entities(
        const ReplierQos& qos)
{
    // Entities are not autoenabled since the publisher and
    // the subscriber have autoenable_created_entities set to false.

    // Create writer for the Reply topic
    replier_writer_ =
            service_->get_publisher()->create_datawriter(
        service_->get_reply_topic(), qos.writer_qos, this, StatusMask::publication_matched());

    if (!replier_writer_)
    {
        EPROSIMA_LOG_ERROR(REPLIER, "Error creating replier writer");
        return RETCODE_ERROR;
    }

    replier_reader_ =
            service_->get_subscriber()->create_datareader(
        service_->get_request_topic(), qos.reader_qos, this, StatusMask::subscription_matched());

    if (!replier_reader_)
    {
        EPROSIMA_LOG_ERROR(REPLIER, "Error creating reply reader");
        return RETCODE_ERROR;
    }

    // Set the related entity key on both entities
    replier_reader_->set_related_datawriter(replier_writer_);
    replier_writer_->set_related_datareader(replier_reader_);

    return RETCODE_OK;
}

ReturnCode_t ReplierImpl::delete_contained_entities()
{
    // Check if DataWriter and DataReader can be deleted.
    // If not, do nothing and return an error code
    if (replier_writer_)
    {
        if (!service_->get_publisher()->can_be_deleted(replier_writer_))
        {
            EPROSIMA_LOG_ERROR(REPLIER, "Replier DataWriter cannot be deleted");
            return RETCODE_PRECONDITION_NOT_MET;
        }
    }

    if (replier_reader_)
    {
        if (!service_->get_subscriber()->can_be_deleted(replier_reader_))
        {
            EPROSIMA_LOG_ERROR(REPLIER, "Replier DataReader cannot be deleted");
            return RETCODE_PRECONDITION_NOT_MET;
        }
    }

    // Delete DataWriter and DataReader
    service_->get_publisher()->delete_datawriter(replier_writer_);
    replier_writer_ = nullptr;
    service_->get_subscriber()->delete_datareader(replier_reader_);
    replier_reader_ = nullptr;

    return RETCODE_OK;
}

ReplierImpl::RequesterMatchStatus ReplierImpl::requester_match_status(
        const RequestInfo& info) const
{
    // Check if the replier is still matched with the requester in the request topic
    PublicationBuiltinTopicData pub_data;
    if (RETCODE_OK != replier_reader_->get_matched_publication_data(pub_data, info.sample_identity.writer_guid()))
    {
        return RequesterMatchStatus::UNMATCHED;
    }

    auto related_guid = info.related_sample_identity.writer_guid();
    bool reply_topic_matched = false;
    if (related_guid.entityId.is_reader() && info.sample_identity.writer_guid() != related_guid)
    {
        // Custom related GUID (i.e. reply reader GUID) sent with the request.
        // Check if the replier writer is matched with that specific reader.
        SubscriptionBuiltinTopicData sub_data;
        reply_topic_matched = RETCODE_OK == replier_writer_->get_matched_subscription_data(sub_data, related_guid);
    }
    else
    {
        // Take the replier reader GUID from the related_datareader_key within PublicationBuiltinTopicData
        SubscriptionBuiltinTopicData related_reader_sub_data;
        reply_topic_matched = RETCODE_OK == replier_writer_->get_matched_subscription_data(related_reader_sub_data,
                        pub_data.related_datareader_key);
    }

    return reply_topic_matched ?
           RequesterMatchStatus::MATCHED :
           RequesterMatchStatus::PARTIALLY_MATCHED;
}

ReplierImpl::RequesterMatchStatus ReplierImpl::wait_for_matching(
        const fastdds::dds::Duration_t& timeout,
        const RequestInfo& info)
{
    Time_t current_time;
    Time_t::now(current_time);
    Time_t finish_time = current_time + timeout;

    RequesterMatchStatus requester_status = requester_match_status(info);
    while ((RequesterMatchStatus::MATCHED != requester_status) && current_time < finish_time)
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

            // Check if the requester is fully matched
            requester_status = requester_match_status(info);
        }

        // Update the current time
        Time_t::now(current_time);
    }

    return requester_status;
}

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima
