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
#include <fastdds/dds/domain/DomainParticipantListener.hpp>

#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/dds/log/Log.hpp>

#include <functional>

namespace eprosima {
namespace fastdds {
namespace dds {

TopicImpl::TopicImpl(
        TopicProxyFactory* factory,
        DomainParticipantImpl* p,
        TypeSupport type_support,
        const TopicQos& qos,
        TopicListener* listen)
    : factory_(factory)
    , participant_(p)
    , type_support_(type_support)
    , qos_(&qos == &TOPIC_QOS_DEFAULT ? participant_->get_default_topic_qos() : qos)
    , listener_(listen)
{
}

TopicImpl::~TopicImpl()
{
}

ReturnCode_t TopicImpl::check_qos_including_resource_limits(
        const TopicQos& qos,
        const TypeSupport& type)
{
    ReturnCode_t check_qos_return = check_qos(qos);
    if (RETCODE_OK == check_qos_return &&
            type->is_compute_key_provided)
    {
        check_qos_return = check_allocation_consistency(qos);
    }
    return check_qos_return;
}

ReturnCode_t TopicImpl::check_qos(
        const TopicQos& qos)
{
    if (BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS == qos.destination_order().kind)
    {
        EPROSIMA_LOG_ERROR(DDS_QOS_CHECK, "BY SOURCE TIMESTAMP DestinationOrder not supported");
        return RETCODE_UNSUPPORTED;
    }
    if (AUTOMATIC_LIVELINESS_QOS == qos.liveliness().kind ||
            MANUAL_BY_PARTICIPANT_LIVELINESS_QOS == qos.liveliness().kind)
    {
        if (qos.liveliness().lease_duration < eprosima::fastdds::dds::c_TimeInfinite &&
                qos.liveliness().lease_duration <= qos.liveliness().announcement_period)
        {
            EPROSIMA_LOG_ERROR(DDS_QOS_CHECK, "lease_duration <= announcement period.");
            return RETCODE_INCONSISTENT_POLICY;
        }
    }
    return RETCODE_OK;
}

ReturnCode_t TopicImpl::check_allocation_consistency(
        const TopicQos& qos)
{
    if ((qos.resource_limits().max_samples > 0) &&
            (qos.resource_limits().max_samples <
            (qos.resource_limits().max_instances * qos.resource_limits().max_samples_per_instance)))
    {
        EPROSIMA_LOG_ERROR(DDS_QOS_CHECK,
                "max_samples should be greater than max_instances * max_samples_per_instance");
        return RETCODE_INCONSISTENT_POLICY;
    }
    if ((qos.resource_limits().max_instances <= 0 || qos.resource_limits().max_samples_per_instance <= 0) &&
            (qos.resource_limits().max_samples > 0))
    {
        EPROSIMA_LOG_ERROR(DDS_QOS_CHECK,
                "max_samples should be infinite when max_instances or max_samples_per_instance are infinite");
        return RETCODE_INCONSISTENT_POLICY;
    }
    return RETCODE_OK;
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

bool TopicImpl::can_qos_be_updated(
        const TopicQos& to,
        const TopicQos& from)
{
    (void)to;
    (void)from;

    return true;
}

const TopicQos& TopicImpl::get_qos() const
{
    return qos_;
}

ReturnCode_t TopicImpl::set_qos(
        const TopicQos& qos)
{
    if (&qos == &TOPIC_QOS_DEFAULT)
    {
        const TopicQos& default_qos = participant_->get_default_topic_qos();
        if (!can_qos_be_updated(qos_, default_qos))
        {
            return RETCODE_IMMUTABLE_POLICY;
        }

        set_qos(qos_, default_qos, false);
        return RETCODE_OK;
    }

    ReturnCode_t ret_val = check_qos_including_resource_limits(qos, type_support_);
    if (RETCODE_OK != ret_val)
    {
        return ret_val;
    }

    if (!can_qos_be_updated(qos_, qos))
    {
        return RETCODE_IMMUTABLE_POLICY;
    }

    set_qos(qos_, qos, false);
    return RETCODE_OK;
}

const TopicListener* TopicImpl::get_listener() const
{
    return listener_;
}

void TopicImpl::set_listener(
        TopicListener* listener)
{
    listener_ = listener;
}

void TopicImpl::set_listener(
        TopicListener* listener,
        const StatusMask& mask)
{
    participant_->set_topic_listener(factory_, this, listener, mask);
}

DomainParticipant* TopicImpl::get_participant() const
{
    return participant_->get_participant();
}

const TypeSupport& TopicImpl::get_type() const
{
    return type_support_;
}

TopicListener* TopicImpl::get_listener_for(
        const StatusMask& status,
        const Topic* topic)
{
    if (listener_ != nullptr &&
            topic->get_status_mask().is_active(status))
    {
        return listener_;
    }
    return participant_->get_listener_for(status);
}

} // dds
} // fastdds
} // eprosima
