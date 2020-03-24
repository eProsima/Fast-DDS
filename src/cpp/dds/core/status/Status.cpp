/*
 * Copyright 2020, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dds/core/status/Status.hpp>

namespace dds {
namespace core {
namespace status {

template<>
StatusMask get_status<InconsistentTopicStatus>()
{
    return StatusMask::inconsistent_topic();
}

InconsistentTopicStatus::InconsistentTopicStatus()
    : dds::core::Value<detail::InconsistentTopicStatus>()
{
}

int32_t InconsistentTopicStatus::total_count() const
{
    return delegate().total_count;
}

void InconsistentTopicStatus::total_count(int32_t count)
{
    delegate().total_count = count;
}

int32_t InconsistentTopicStatus::total_count_change() const
{
    return delegate().total_count_change;
}

void InconsistentTopicStatus::total_count_change(int32_t count_change)
{
    delegate().total_count_change = count_change;
}

template<>
StatusMask get_status<SampleLostStatus>()
{
    return StatusMask::sample_lost();
}


SampleLostStatus::SampleLostStatus()
    : dds::core::Value<detail::SampleLostStatus>()
{
}

int32_t SampleLostStatus::total_count() const
{
    return delegate().total_count;
}

void SampleLostStatus::total_count(int32_t count)
{
    delegate().total_count = count;
}

int32_t SampleLostStatus::total_count_change() const
{
    return delegate().total_count_change;
}

void SampleLostStatus::total_count_change(int32_t count_change)
{
    delegate().total_count_change = count_change;
}


template<>
StatusMask get_status<SampleRejectedStatus>()
{
    return StatusMask::sample_rejected();
}


SampleRejectedStatus::SampleRejectedStatus()
    : dds::core::Value<detail::SampleRejectedStatus>()
{
}

int32_t SampleRejectedStatus::total_count() const
{
    return delegate().total_count;
}

void SampleRejectedStatus::total_count(int32_t count)
{
    delegate().total_count = count;
}


int32_t SampleRejectedStatus::total_count_change() const
{
    return delegate().total_count_change;
}


void SampleRejectedStatus::total_count_change(int32_t count_change)
{
    delegate().total_count_change = count_change;
}


dds::core::status::SampleRejectedKind SampleRejectedStatus::last_reason() const
{
    switch (delegate().last_reason)
    {
        case eprosima::fastdds::dds::REJECTED_BY_INSTANCES_LIMIT:
            return dds::core::status::SampleRejectedKind(dds::core::status::SampleRejectedKind::REJECTED_BY_INSTANCES_LIMIT);
            break;
        case eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT:
            return dds::core::status::SampleRejectedKind(dds::core::status::SampleRejectedKind::REJECTED_BY_SAMPLES_LIMIT);
            break;
        case eprosima::fastdds::dds::REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT:
            return dds::core::status::SampleRejectedKind(dds::core::status::SampleRejectedKind::REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT);
            break;
        default:
            return dds::core::status::SampleRejectedKind(dds::core::status::SampleRejectedKind::NOT_REJECTED);
            break;
    }
}

void SampleRejectedStatus::last_reason(dds::core::status::SampleRejectedKind reason)
{
    switch (reason.underlying())
    {
        case dds::core::status::SampleRejectedKind::REJECTED_BY_INSTANCES_LIMIT:
            delegate().last_reason = eprosima::fastdds::dds::REJECTED_BY_INSTANCES_LIMIT;
            break;
        case dds::core::status::SampleRejectedKind::REJECTED_BY_SAMPLES_LIMIT:
            delegate().last_reason = eprosima::fastdds::dds::REJECTED_BY_SAMPLES_LIMIT;
            break;
        case dds::core::status::SampleRejectedKind::REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT:
            delegate().last_reason = eprosima::fastdds::dds::REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
            break;
        default:
            delegate().last_reason = eprosima::fastdds::dds::NOT_REJECTED;
            break;
    }
}


dds::core::InstanceHandle SampleRejectedStatus::last_instance_handle() const
{
    return delegate().last_instance_handle;
}


void SampleRejectedStatus::last_instance_handle(const dds::core::InstanceHandle& handle)
{
    delegate().last_instance_handle = handle;
}


template<>
StatusMask get_status<LivelinessLostStatus>()
{
    return StatusMask::liveliness_lost();
}


LivelinessLostStatus::LivelinessLostStatus()
    : dds::core::Value<detail::LivelinessLostStatus>()
{
}


int32_t LivelinessLostStatus::total_count() const
{
    return delegate().total_count;
}


void LivelinessLostStatus::total_count(int32_t count)
{
    delegate().total_count = count;
}


int32_t LivelinessLostStatus::total_count_change() const
{
    return delegate().total_count_change;
}


void LivelinessLostStatus::total_count_change(int32_t count_change)
{
    delegate().total_count_change = count_change;
}


template<>
StatusMask get_status<LivelinessChangedStatus>()
{
    return StatusMask::liveliness_changed();
}


LivelinessChangedStatus::LivelinessChangedStatus()
    : dds::core::Value<detail::LivelinessChangedStatus>()
{
}


int32_t LivelinessChangedStatus::alive_count() const
{
    return delegate().alive_count;
}

void LivelinessChangedStatus::alive_count(int32_t count)
{
    delegate().alive_count = count;
}


int32_t LivelinessChangedStatus::not_alive_count() const
{
    return delegate().not_alive_count;
}

void LivelinessChangedStatus::not_alive_count(int32_t count)
{
    delegate().not_alive_count = count;
}


int32_t LivelinessChangedStatus::alive_count_change() const
{
    return delegate().alive_count_change;
}

void LivelinessChangedStatus::alive_count_change(int32_t count_change)
{
    delegate().alive_count_change = count_change;
}


int32_t LivelinessChangedStatus::not_alive_count_change() const
{
    return delegate().not_alive_count_change;
}

void LivelinessChangedStatus::not_alive_count_change(int32_t count_change)
{
    delegate().not_alive_count_change = count_change;
}


dds::core::InstanceHandle LivelinessChangedStatus::last_publication_handle() const
{
    return delegate().last_publication_handle;
}

void LivelinessChangedStatus::last_publication_handle(const dds::core::InstanceHandle& handle)
{
    delegate().last_publication_handle = handle;
}


template<>
StatusMask get_status<OfferedDeadlineMissedStatus>()
{
    return StatusMask::offered_deadline_missed();
}


OfferedDeadlineMissedStatus::OfferedDeadlineMissedStatus()
    : dds::core::Value<detail::OfferedDeadlineMissedStatus>()
{
}

int32_t OfferedDeadlineMissedStatus::total_count() const
{
    return delegate().total_count;
}


void OfferedDeadlineMissedStatus::total_count(int32_t count)
{
    delegate().total_count = count;
}


int32_t OfferedDeadlineMissedStatus::total_count_change() const
{
    return delegate().total_count_change;
}


void OfferedDeadlineMissedStatus::total_count_change(int32_t count_change)
{
    delegate().total_count_change = count_change;
}


dds::core::InstanceHandle OfferedDeadlineMissedStatus::last_instance_handle() const
{
    return delegate().last_instance_handle;
}

void OfferedDeadlineMissedStatus::last_instance_handle(const dds::core::InstanceHandle& handle)
{
    delegate().last_instance_handle = handle;
}


template<>
StatusMask get_status<RequestedDeadlineMissedStatus>()
{
    return StatusMask::requested_deadline_missed();
}


RequestedDeadlineMissedStatus::RequestedDeadlineMissedStatus()
    : dds::core::Value<detail::RequestedDeadlineMissedStatus>()
{
}


int32_t RequestedDeadlineMissedStatus::total_count() const
{
    return delegate().total_count;
}


void RequestedDeadlineMissedStatus::total_count(int32_t count)
{
    delegate().total_count = count;
}


int32_t RequestedDeadlineMissedStatus::total_count_change() const
{
    return delegate().total_count_change;
}


void RequestedDeadlineMissedStatus::total_count_change(int32_t count_change)
{
    delegate().total_count_change = count_change;
}


dds::core::InstanceHandle RequestedDeadlineMissedStatus::last_instance_handle() const
{
    return delegate().last_instance_handle;
}

void RequestedDeadlineMissedStatus::last_instance_handle(const dds::core::InstanceHandle& handle)
{
    delegate().last_instance_handle = handle;
}


template<>
StatusMask get_status<OfferedIncompatibleQosStatus>()
{
    return StatusMask::offered_incompatible_qos();
}


OfferedIncompatibleQosStatus::OfferedIncompatibleQosStatus()
    : dds::core::Value<detail::OfferedIncompatibleQosStatus>()
{
}

int32_t OfferedIncompatibleQosStatus::total_count() const
{
    return delegate().total_count;
}


void OfferedIncompatibleQosStatus::total_count(int32_t count)
{
    delegate().total_count = count;
}


int32_t OfferedIncompatibleQosStatus::total_count_change() const
{
    return delegate().total_count_change;
}


void OfferedIncompatibleQosStatus::total_count_change(int32_t count_change)
{
    delegate().total_count_change = count_change;
}


const dds::core::policy::QosPolicyId& OfferedIncompatibleQosStatus::last_policy_id() const
{
    return delegate().last_policy_id;
}

void OfferedIncompatibleQosStatus::last_policy_id(const dds::core::policy::QosPolicyId& policy_id)
{
    delegate().last_policy_id = policy_id;
}


const dds::core::policy::QosPolicyCountSeq& OfferedIncompatibleQosStatus::policies() const
{
    return delegate().policies;
}

void OfferedIncompatibleQosStatus::policies(const dds::core::policy::QosPolicyCountSeq& policies)
{
    delegate().policies = policies;
}


template<>
StatusMask get_status<RequestedIncompatibleQosStatus>()
{
    return StatusMask::requested_incompatible_qos();
}


RequestedIncompatibleQosStatus::RequestedIncompatibleQosStatus()
    : dds::core::Value<detail::RequestedIncompatibleQosStatus>()
{
}

int32_t RequestedIncompatibleQosStatus::total_count() const
{
    return delegate().total_count;
}


void RequestedIncompatibleQosStatus::total_count(int32_t count)
{
    delegate().total_count = count;
}


int32_t RequestedIncompatibleQosStatus::total_count_change() const
{
    return delegate().total_count_change;
}


void RequestedIncompatibleQosStatus::total_count_change(int32_t count_change)
{
    delegate().total_count_change = count_change;
}


const dds::core::policy::QosPolicyId& RequestedIncompatibleQosStatus::last_policy_id() const
{
    return delegate().last_policy_id;
}

void RequestedIncompatibleQosStatus::last_policy_id(const dds::core::policy::QosPolicyId& policy_id)
{
    delegate().last_policy_id = policy_id;
}


const dds::core::policy::QosPolicyCountSeq& RequestedIncompatibleQosStatus::policies() const
{
    return delegate().policies;
}

void RequestedIncompatibleQosStatus::policies(const dds::core::policy::QosPolicyCountSeq& policies)
{
    delegate().policies = policies;
}


template<>
StatusMask get_status<PublicationMatchedStatus>()
{
    return StatusMask::publication_matched();
}


PublicationMatchedStatus::PublicationMatchedStatus()
    : dds::core::Value<detail::PublicationMatchedStatus>()
{
}

int32_t PublicationMatchedStatus::total_count() const
{
    return delegate().total_count;
}


void PublicationMatchedStatus::total_count(int32_t count)
{
    delegate().total_count = count;
}


int32_t PublicationMatchedStatus::total_count_change() const
{
    return delegate().total_count_change;
}


void PublicationMatchedStatus::total_count_change(int32_t count_change)
{
    delegate().total_count_change = count_change;
}


int32_t PublicationMatchedStatus::current_count() const
{
    return delegate().current_count;
}

void PublicationMatchedStatus::current_count(int32_t count)
{
    delegate().current_count = count;
}


int32_t PublicationMatchedStatus::current_count_change() const
{
    return delegate().current_count_change;
}

void PublicationMatchedStatus::current_count_change(int32_t count_change)
{
    delegate().current_count_change = count_change;
}


dds::core::InstanceHandle PublicationMatchedStatus::last_subscription_handle() const
{
    return delegate().last_subscription_handle;
}

void PublicationMatchedStatus::last_subscription_handle(const dds::core::InstanceHandle& handle)
{
    delegate().last_subscription_handle = handle;
}


template<>
StatusMask get_status<SubscriptionMatchedStatus>()
{
    return StatusMask::subscription_matched();
}


SubscriptionMatchedStatus::SubscriptionMatchedStatus()
    : dds::core::Value<detail::SubscriptionMatchedStatus>()
{
}

int32_t SubscriptionMatchedStatus::total_count() const
{
    return delegate().total_count;
}


void SubscriptionMatchedStatus::total_count(int32_t count)
{
    delegate().total_count = count;
}


int32_t SubscriptionMatchedStatus::total_count_change() const
{
    return delegate().total_count_change;
}


void SubscriptionMatchedStatus::total_count_change(int32_t count_change)
{
    delegate().total_count_change = count_change;
}


int32_t SubscriptionMatchedStatus::current_count() const
{
    return delegate().current_count;
}

void SubscriptionMatchedStatus::current_count(int32_t count)
{
    delegate().current_count = count;
}


int32_t SubscriptionMatchedStatus::current_count_change() const
{
    return delegate().current_count_change;
}

void SubscriptionMatchedStatus::current_count_change(int32_t count_change)
{
    delegate().current_count_change = count_change;
}


dds::core::InstanceHandle SubscriptionMatchedStatus::last_publication_handle() const
{
    return delegate().last_publication_handle;
}

void SubscriptionMatchedStatus::last_publication_handle(const dds::core::InstanceHandle& handle)
{
    delegate().last_publication_handle = handle;
}

template<typename STATUS>
StatusMask get_status()
{
    return StatusMask::none();
}

} //namespace status
} //namespace core
} //namespace dds

