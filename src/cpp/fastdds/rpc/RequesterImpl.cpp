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

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/rpc/RequestInfo.hpp>
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
    FASTDDS_TODO_BEFORE(3, 3, "Implement matching algorithm");

    if (!enabled_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Trying to send a request with a disabled requester");
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
    FASTDDS_TODO_BEFORE(3, 3, "Implement matching algorithm");

    if (!enabled_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Trying to take a reply with a disabled requester");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    return requester_reader_->take_next_sample(data, &info);
}

ReturnCode_t RequesterImpl::take_reply(
        LoanableCollection& data,
        LoanableSequence<RequestInfo>& info)
{
    FASTDDS_TODO_BEFORE(3, 3, "Implement matching algorithm");

    if (!enabled_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Trying to take a reply with a disabled requester");
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

ReturnCode_t RequesterImpl::create_dds_entities(
        const RequesterQos& qos)
{
    // Create writer for the Request topic
    requester_writer_ =
            service_->get_publisher()->create_datawriter(service_->get_request_topic(), qos.writer_qos, nullptr);

    if (!requester_writer_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Error creating request writer");
        return RETCODE_ERROR;
    }

    requester_reader_ =
            service_->get_subscriber()->create_datareader(
        service_->get_reply_filtered_topic(), qos.reader_qos, nullptr);

    if (!requester_reader_)
    {
        EPROSIMA_LOG_ERROR(REQUESTER, "Error creating reply reader");
        return RETCODE_ERROR;
    }

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

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima
