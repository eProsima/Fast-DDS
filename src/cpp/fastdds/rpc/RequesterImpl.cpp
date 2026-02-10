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
        ServiceImpl* /*service*/,
        const RequesterQos& /*qos*/)
{
}

RequesterImpl::~RequesterImpl()
{
}

const std::string& RequesterImpl::get_service_name() const
{
    return placeholder_name_;
}

ReturnCode_t RequesterImpl::send_request(
        void* /*data*/,
        RequestInfo& /*info*/)
{
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t RequesterImpl::take_reply(
        void* /*data*/,
        RequestInfo& /*info*/)
{
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t RequesterImpl::take_reply(
        LoanableCollection& /*data*/,
        LoanableSequence<RequestInfo>& /*info*/)
{
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t RequesterImpl::return_loan(
        LoanableCollection& /*data*/,
        LoanableSequence<RequestInfo>& /*info*/)
{
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t RequesterImpl::enable()
{
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t RequesterImpl::close()
{
    return RETCODE_UNSUPPORTED;
}

void RequesterImpl::on_publication_matched(
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& /*info*/)
{
    return;
}

void RequesterImpl::on_subscription_matched(
        DataReader* /*reader*/,
        const SubscriptionMatchedStatus& /*info*/)
{
    return;
}

ReturnCode_t RequesterImpl::create_dds_entities(
        const RequesterQos& /*qos*/)
{
    return RETCODE_UNSUPPORTED;
}

ReturnCode_t RequesterImpl::delete_contained_entities()
{
    return RETCODE_UNSUPPORTED;
}

RequesterImpl::ReplierMatchStatus RequesterImpl::replier_match_status() const
{
    return ReplierMatchStatus::UNMATCHED;
}

bool RequesterImpl::wait_for_matching(
        const fastdds::dds::Duration_t& /*timeout*/)
{
    return false;
}

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima
