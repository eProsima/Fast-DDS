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

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
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
    FASTDDS_TODO_BEFORE(3, 3, "Implement matching algorithm");

    if (!enabled_)
    {
        EPROSIMA_LOG_ERROR(REPLIER, "Trying to send a reply with a disabled replier");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    rtps::WriteParams wparams;
    wparams.related_sample_identity(info.related_sample_identity);

    return replier_writer_->write(data, wparams);
}

ReturnCode_t ReplierImpl::take_request(
        void* data,
        RequestInfo& info)
{
    FASTDDS_TODO_BEFORE(3, 3, "Implement matching algorithm");

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
    FASTDDS_TODO_BEFORE(3, 3, "Implement matching algorithm");

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

ReturnCode_t ReplierImpl::create_dds_entities(
        const ReplierQos& qos)
{
    // Create writer for the Reply topic
    replier_writer_ =
            service_->get_publisher()->create_datawriter(service_->get_reply_topic(), qos.writer_qos, nullptr);

    if (!replier_writer_)
    {
        EPROSIMA_LOG_ERROR(REPLIER, "Error creating replier writer");
        return RETCODE_ERROR;
    }

    replier_reader_ =
            service_->get_subscriber()->create_datareader(service_->get_request_topic(), qos.reader_qos, nullptr);

    if (!replier_reader_)
    {
        EPROSIMA_LOG_ERROR(REPLIER, "Error creating reply reader");
        return RETCODE_ERROR;
    }

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

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima