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

ReplierImpl::ReplierImpl(
        ServiceImpl* /*service*/,
        const ReplierQos& /*qos*/)
{
}

ReplierImpl::~ReplierImpl()
{
}

const std::string& ReplierImpl::get_service_name() const
{
    return placeholder_name_;
}

ReturnCode_t ReplierImpl::send_reply(
        void* /*data*/,
        const RequestInfo& /*info*/)
{
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t ReplierImpl::take_request(
        void* /*data*/,
        RequestInfo& /*info*/)
    {
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t ReplierImpl::take_request(
        LoanableCollection& /*data*/,
        LoanableSequence<RequestInfo>& /*info*/)
{
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t ReplierImpl::return_loan(
        LoanableCollection& /*data*/,
        LoanableSequence<RequestInfo>& /*info*/)
    {
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t ReplierImpl::enable()
{
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t ReplierImpl::close()
{
    return RETCODE_UNSUPPORTED;
}

void ReplierImpl::on_publication_matched(
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& /*info*/)
{
}

void ReplierImpl::on_subscription_matched(
        DataReader* /*reader*/,
        const SubscriptionMatchedStatus& /*info*/)
{
}

ReturnCode_t ReplierImpl::create_dds_entities(
        const ReplierQos& /*qos*/)
{
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t ReplierImpl::delete_contained_entities()
{
    return RETCODE_UNSUPPORTED;
}

ReplierImpl::RequesterMatchStatus ReplierImpl::requester_match_status(
        const RequestInfo& /*info*/) const
    {
        return RequesterMatchStatus::UNMATCHED;
}

ReplierImpl::RequesterMatchStatus ReplierImpl::wait_for_matching(
        const fastdds::dds::Duration_t& /*timeout*/,
        const RequestInfo& /*info*/)
{
    return RequesterMatchStatus::UNMATCHED;
}

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima
