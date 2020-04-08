// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*
 * TopicImpl.cpp
 *
 */

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/topic/TopicImpl.hpp>
#include <fastdds/domain/DomainParticipantImpl.hpp>

#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/dds/log/Log.hpp>

#include <functional>

namespace eprosima {
namespace fastdds {
namespace dds {

TopicImpl::TopicImpl(
        DomainParticipantImpl* p,
        TypeSupport type_support,
        const TopicQos& qos,
        TopicListener* listen)
    : participant_(p)
    , type_support_(type_support)
    , qos_(&qos == &TOPIC_QOS_DEFAULT ? participant_->get_default_topic_qos() : qos)
    , listener_(listen)
    , user_topic_(nullptr)
{
}

TopicImpl::~TopicImpl()
{
    delete user_topic_;
}

ReturnCode_t TopicImpl::check_qos(
        const TopicQos& qos)
{
    if (PERSISTENT_DURABILITY_QOS == qos.durability().kind)
    {
        logError(DDS_QOS_CHECK, "PERSISTENT Durability not supported");
        return ReturnCode_t::RETCODE_UNSUPPORTED;
    }
    if (BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS == qos.destination_order().kind)
    {
        logError(DDS_QOS_CHECK, "BY SOURCE TIMESTAMP DestinationOrder not supported");
        return ReturnCode_t::RETCODE_UNSUPPORTED;
    }
    if (BEST_EFFORT_RELIABILITY_QOS == qos.reliability().kind &&
            EXCLUSIVE_OWNERSHIP_QOS == qos.ownership().kind)
    {
        logError(DDS_QOS_CHECK, "BEST_EFFORT incompatible with EXCLUSIVE ownership");
        return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
    }
    if (AUTOMATIC_LIVELINESS_QOS == qos.liveliness().kind ||
            MANUAL_BY_PARTICIPANT_LIVELINESS_QOS == qos.liveliness().kind)
    {
        if (qos.liveliness().lease_duration < eprosima::fastrtps::c_TimeInfinite &&
                qos.liveliness().lease_duration <= qos.liveliness().announcement_period)
        {
            logError(DDS_QOS_CHECK, "lease_duration <= announcement period.");
            return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
        }
    }
    return ReturnCode_t::RETCODE_OK;
}

void TopicImpl::set_qos(
        TopicQos& to,
        const TopicQos& from,
        bool first_time)
{
    (void)first_time;
    to = from;

    // Topic Qos is only used to create other Qos, so it can always be updated
}


const TopicQos& TopicImpl::get_qos() const
{
    return qos_;
}

ReturnCode_t TopicImpl::set_qos(
        const TopicQos& qos)
{
    ReturnCode_t ret_val = check_qos(qos);
    if (!ret_val)
    {
        return ret_val;
    }

    // Topic qos can always be updated

    set_qos(qos_, qos, false);
    return ReturnCode_t::RETCODE_OK;
}

const TopicListener* TopicImpl::get_listener() const
{
    return listener_;
}

ReturnCode_t TopicImpl::set_listener(
        TopicListener* listener)
{
    listener_ = listener;
    return ReturnCode_t::RETCODE_OK;
}

DomainParticipant* TopicImpl::get_participant() const
{
    return participant_->get_participant();
}

const Topic* TopicImpl::get_topic() const
{
    return user_topic_;
}

const TypeSupport& TopicImpl::get_type() const
{
    return type_support_;
}

} // dds
} // fastdds
} // eprosima
