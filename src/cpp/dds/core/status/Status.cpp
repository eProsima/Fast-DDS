/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 *
*/


/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/status/Status.hpp>

namespace dds {
namespace core {
namespace status {

//TInconsistentTopicStatus

InconsistentTopicStatus::InconsistentTopicStatus()
    : dds::core::Value()
{
}

int32_t InconsistentTopicStatus::total_count() const
{
    //To implement
//    return this->delegate().total_count();
}

int32_t InconsistentTopicStatus::total_count_change() const
{
    //To implement
//    return this->delegate().total_count_change();
}

//TSampleLostStatus

SampleLostStatus::SampleLostStatus()
    : dds::core::Value()
{
}

int32_t SampleLostStatus::total_count() const
{
    //To implement
//    return this->delegate().total_count();
}

int32_t SampleLostStatus::total_count_change() const
{
    //To implement
//    return this->delegate().total_count_change();
}

//TSampleRejectedStatus

SampleRejectedStatus::SampleRejectedStatus()
    : dds::core::Value()
{
}

int32_t SampleRejectedStatus::total_count() const
{
    //To implement
//    return this->delegate().total_count();
}

int32_t SampleRejectedStatus::total_count_change() const
{
    //To implement
//    return this->delegate().total_count_change();
}

const dds::core::status::SampleRejectedState SampleRejectedStatus::last_reason() const
{
    //To implement
//    return this->delegate().last_reason();
}

const dds::core::InstanceHandle SampleRejectedStatus::last_instance_handle() const
{
    //To implement
//    return this->delegate().last_instance_handle();
}

//TLivelinessLostStatus
LivelinessLostStatus::LivelinessLostStatus()
    : dds::core::Value()
{
}

int32_t LivelinessLostStatus::total_count() const
{
    //To implement
//    return this->delegate().total_count();
}

int32_t LivelinessLostStatus::total_count_change() const
{
    //To implement
//    return this->delegate().total_count_change();
}

//TLivelinessChangedStatus

LivelinessChangedStatus::LivelinessChangedStatus()
    : dds::core::Value()
{
}

int32_t LivelinessChangedStatus::alive_count() const
{
    //To implement
//    return this->delegate().alive_count();
}

int32_t LivelinessChangedStatus::not_alive_count() const
{
    //To implement
//    return this->delegate().not_alive_count();
}

int32_t LivelinessChangedStatus::alive_count_change() const
{
    //To implement
//    return this->delegate().alive_count();
}

int32_t LivelinessChangedStatus::not_alive_count_change() const
{
    //To implement
//    return this->delegate().not_alive_count();
}

const dds::core::InstanceHandle LivelinessChangedStatus::last_publication_handle() const
{
    //To implement
//    return this->delegate().last_publication_handle();
}

//TOfferedDeadlineMissedStatus

OfferedDeadlineMissedStatus::OfferedDeadlineMissedStatus()
    : dds::core::Value()
{
}

int32_t OfferedDeadlineMissedStatus::total_count() const
{
    //To implement
//    return this->delegate().total_count();
}

int32_t OfferedDeadlineMissedStatus::total_count_change() const
{
    //To implement
//    return this->delegate().total_count_change();
}

const dds::core::InstanceHandle OfferedDeadlineMissedStatus::last_instance_handle() const
{
    //To implement
//    return this->delegate().last_instance_handle();
}

//TRequestedDeadlineMissedStatus

RequestedDeadlineMissedStatus::RequestedDeadlineMissedStatus()
    : dds::core::Value()
{
}

int32_t RequestedDeadlineMissedStatus::total_count() const
{
    //To implement
//    return this->delegate().total_count();
}

int32_t RequestedDeadlineMissedStatus::total_count_change() const
{
    //To implement
//    return this->delegate().total_count_change();
}

const dds::core::InstanceHandle RequestedDeadlineMissedStatus::last_instance_handle() const
{
    //To implement
//    return this->delegate().last_instance_handle();
}

//TOfferedIncompatibleQosStatus

OfferedIncompatibleQosStatus::OfferedIncompatibleQosStatus()
    : dds::core::Value()
{
}

int32_t OfferedIncompatibleQosStatus::total_count() const
{
    //To implement
//    return this->delegate().total_count();
}

int32_t OfferedIncompatibleQosStatus::total_count_change() const
{
    //To implement
//    return this->delegate().total_count_change();
}

dds::core::policy::QosPolicyId OfferedIncompatibleQosStatus::last_policy_id() const
{
    //To implement
//    return this->delegate().last_policy_id();
}

const dds::core::policy::QosPolicyCountSeq OfferedIncompatibleQosStatus::policies() const
{
    //To implement
//    return this->delegate().policies();
}

const dds::core::policy::QosPolicyCountSeq& OfferedIncompatibleQosStatus::policies(
        dds::core::policy::QosPolicyCountSeq& dst) const
{
    //To implement
//    return this->delegate().policies(dst);
}

//TRequestedIncompatibleQosStatus

RequestedIncompatibleQosStatus::RequestedIncompatibleQosStatus()
    : dds::core::Value()
{
}

int32_t RequestedIncompatibleQosStatus::total_count() const
{
    //To implement
//    return this->delegate().total_count();
}

int32_t RequestedIncompatibleQosStatus::total_count_change() const
{
    //To implement
//    return this->delegate().total_count_change();
}

dds::core::policy::QosPolicyId RequestedIncompatibleQosStatus::last_policy_id() const
{
    //To implement
//    return this->delegate().last_policy_id();
}

const dds::core::policy::QosPolicyCountSeq RequestedIncompatibleQosStatus::policies() const
{
    //To implement
//    return this->delegate().policies();
}

const dds::core::policy::QosPolicyCountSeq& RequestedIncompatibleQosStatus::policies(
        dds::core::policy::QosPolicyCountSeq& dst) const
{
    //To implement
//    return this->delegate().policies(dst);
}

//TPublicationMatchedStatus

PublicationMatchedStatus::PublicationMatchedStatus()
    : dds::core::Value()
{
}

int32_t PublicationMatchedStatus::total_count() const
{
    //To implement
//    return this->delegate().total_count();
}

int32_t PublicationMatchedStatus::total_count_change() const
{
    //To implement
//    return this->delegate().total_count_change();
}

int32_t PublicationMatchedStatus::current_count() const
{
    //To implement
//    return this->delegate().current_count();
}

int32_t PublicationMatchedStatus::current_count_change() const
{
    //To implement
//    return this->delegate().current_count_change();
}

const dds::core::InstanceHandle PublicationMatchedStatus::last_subscription_handle() const
{
    //To implement
//    return this->delegate().last_subscription_handle();
}

//TSubscriptionMatchedStatus

SubscriptionMatchedStatus::SubscriptionMatchedStatus()
    : dds::core::Value()
{
}

int32_t SubscriptionMatchedStatus::total_count() const
{
    //To implement
//    return this->delegate().total_count();
}

int32_t SubscriptionMatchedStatus::total_count_change() const
{
    //To implement
//    return this->delegate().total_count_change();
}

int32_t SubscriptionMatchedStatus::current_count() const
{
    //To implement
//    return this->delegate().current_count();
}

int32_t SubscriptionMatchedStatus::current_count_change() const
{
    //To implement
//    return this->delegate().current_count_change();
}

const dds::core::InstanceHandle SubscriptionMatchedStatus::last_publication_handle() const
{
    //To implement
//    return this->delegate().last_publication_handle();
}

} //namespace status
} //namespace core
} //namespace dds
