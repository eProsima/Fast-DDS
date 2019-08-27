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

#ifndef EPROSIMA_DDS_CORE_STATUS_TSTATUS_IMPL_HPP_
#define EPROSIMA_DDS_CORE_STATUS_TSTATUS_IMPL_HPP_

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

template<typename D>
TInconsistentTopicStatus<D>::TInconsistentTopicStatus()
    : dds::core::Value<D>() { }

template<typename D>
int32_t TInconsistentTopicStatus<D>::total_count() const
{
    //To implement
}

template<typename D>
int32_t TInconsistentTopicStatus<D>::total_count_change() const
{
    //To implement
}

//TSampleLostStatus

template<typename D>
TSampleLostStatus<D>::TSampleLostStatus()
    : dds::core::Value<D>() {}

template<typename D>
int32_t TSampleLostStatus<D>::total_count() const
{
    //To implement
}

template<typename D>
int32_t TSampleLostStatus<D>::total_count_change() const
{
    //To implement
}

//TSampleRejectedStatus

template<typename D>
TSampleRejectedStatus<D>::TSampleRejectedStatus()
    : dds::core::Value<D>() { }

template<typename D>
int32_t TSampleRejectedStatus<D>::total_count() const
{
    //To implement
}

template<typename D>
int32_t TSampleRejectedStatus<D>::total_count_change() const
{
    //To implement
}

template<typename D>
const dds::core::status::SampleRejectedState TSampleRejectedStatus<D>::last_reason() const
{
    //To implement
}

template<typename D>
const dds::core::InstanceHandle TSampleRejectedStatus<D>::last_instance_handle() const
{
    //To implement
}
//TLivelinessLostStatus
template<typename D>
TLivelinessLostStatus<D>::TLivelinessLostStatus()
    : dds::core::Value<D>() { }

template<typename D>
int32_t TLivelinessLostStatus<D>::total_count() const
{
    //To implement
}

template<typename D>
int32_t TLivelinessLostStatus<D>::total_count_change() const
{
    //To implement
}

//TLivelinessChangedStatus

template<typename D>
TLivelinessChangedStatus<D>::TLivelinessChangedStatus()
    : dds::core::Value<D>() { }

template<typename D>
int32_t TLivelinessChangedStatus<D>::alive_count() const
{
    //To implement
}

template<typename D>
int32_t TLivelinessChangedStatus<D>::not_alive_count() const
{
    //To implement
}

template<typename D>
int32_t TLivelinessChangedStatus<D>::alive_count_change() const
{
    //To implement
}

template<typename D>
int32_t TLivelinessChangedStatus<D>::not_alive_count_change() const
{
    //To implement
}

template<typename D>
const dds::core::InstanceHandle TLivelinessChangedStatus<D>::last_publication_handle() const
{
    //To implement
}

//TOfferedDeadlineMissedStatus

template<typename D>
TOfferedDeadlineMissedStatus<D>::TOfferedDeadlineMissedStatus()
    : dds::core::Value<D>() { }

template<typename D>
int32_t TOfferedDeadlineMissedStatus<D>::total_count() const
{
    //To implement
}

template<typename D>
int32_t TOfferedDeadlineMissedStatus<D>::total_count_change() const
{
    //To implement
}

template<typename D>
const dds::core::InstanceHandle TOfferedDeadlineMissedStatus<D>::last_instance_handle() const
{
    //To implement
}

//TRequestedDeadlineMissedStatus

template<typename D>
TRequestedDeadlineMissedStatus<D>::TRequestedDeadlineMissedStatus()
    : dds::core::Value<D>() { }

template<typename D>
int32_t TRequestedDeadlineMissedStatus<D>::total_count() const
{
    //To implement
}

template<typename D>
int32_t TRequestedDeadlineMissedStatus<D>::total_count_change() const
{
    //To implement
}

template<typename D>
const dds::core::InstanceHandle TRequestedDeadlineMissedStatus<D>::last_instance_handle() const
{
    //To implement
}

//TOfferedIncompatibleQosStatus

template<typename D>
TOfferedIncompatibleQosStatus<D>::TOfferedIncompatibleQosStatus()
    : dds::core::Value<D>() { }

template<typename D>
int32_t TOfferedIncompatibleQosStatus<D>::total_count() const
{
    //To implement
}

template<typename D>
int32_t TOfferedIncompatibleQosStatus<D>::total_count_change() const
{
    //To implement
}

template<typename D>
dds::core::policy::QosPolicyId TOfferedIncompatibleQosStatus<D>::last_policy_id() const
{
    //To implement
}

template<typename D>
const dds::core::policy::QosPolicyCountSeq TOfferedIncompatibleQosStatus<D>::policies() const
{
    //To implement
}

template<typename D>
const dds::core::policy::QosPolicyCountSeq& TOfferedIncompatibleQosStatus<D>::policies(
        dds::core::policy::QosPolicyCountSeq& dst) const
{
    //To implement
}

//TRequestedIncompatibleQosStatus

template<typename D>
TRequestedIncompatibleQosStatus<D>::TRequestedIncompatibleQosStatus()
    : dds::core::Value<D>() { }

template<typename D>
int32_t TRequestedIncompatibleQosStatus<D>::total_count() const
{
    //To implement
}

template<typename D>
int32_t TRequestedIncompatibleQosStatus<D>::total_count_change() const
{
    //To implement
}

template<typename D>
dds::core::policy::QosPolicyId TRequestedIncompatibleQosStatus<D>::last_policy_id() const
{
    //To implement
}

template<typename D>
const dds::core::policy::QosPolicyCountSeq TRequestedIncompatibleQosStatus<D>::policies() const
{
    //To implement
}

template<typename D>
const dds::core::policy::QosPolicyCountSeq& TRequestedIncompatibleQosStatus<D>::policies(
        dds::core::policy::QosPolicyCountSeq& dst) const
{
    //To implement
}

//TPublicationMatchedStatus

template<typename D>
TPublicationMatchedStatus<D>::TPublicationMatchedStatus()
    : dds::core::Value<D>() { }

template<typename D>
int32_t TPublicationMatchedStatus<D>::total_count() const
{
    //To implement
}

template<typename D>
int32_t TPublicationMatchedStatus<D>::total_count_change() const
{
    //To implement
}

template<typename D>
int32_t TPublicationMatchedStatus<D>::current_count() const
{
    //To implement
}

template<typename D>
int32_t TPublicationMatchedStatus<D>::current_count_change() const
{
    //To implement
}

template<typename D>
const dds::core::InstanceHandle TPublicationMatchedStatus<D>::last_subscription_handle() const
{
    //To implement
}

//TSubscriptionMatchedStatus

template<typename D>
TSubscriptionMatchedStatus<D>::TSubscriptionMatchedStatus()
    : dds::core::Value<D>() { }

template<typename D>
int32_t TSubscriptionMatchedStatus<D>::total_count() const
{
    //To implement
}

template<typename D>
int32_t TSubscriptionMatchedStatus<D>::total_count_change() const
{
    //To implement
}

template <typename D>
int32_t TSubscriptionMatchedStatus<D>::current_count() const
{
    //To implement
}

template<typename D>
int32_t TSubscriptionMatchedStatus<D>::current_count_change() const
{
    //To implement
}

template<typename D>
const dds::core::InstanceHandle TSubscriptionMatchedStatus<D>::last_publication_handle() const
{
    //To implement
}

} //namespace status
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_STATUS_TSTATUS_IMPL_HPP_
