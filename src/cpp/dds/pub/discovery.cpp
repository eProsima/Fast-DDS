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

#include <dds/pub/discovery.hpp>


/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace pub {

void ignore(
        const domain::DomainParticipant& dp,
        const core::InstanceHandle& handle)
{
    //To implement
    (void) dp;
    (void) handle;
}

template<typename FwdIterator>
void ignore(
        const dds::domain::DomainParticipant& dp,
        FwdIterator begin,
        FwdIterator end)
{
    //To implement
    (void) dp;
    (void) begin;
    (void) end;
}

template<typename T>
::dds::core::InstanceHandleSeq matched_subscriptions(
        const DataWriter<T>& dw)
{
    return dw.delegate()->get_matched_subscriptions();
}

template<
    typename T,
    typename FwdIterator>
uint32_t matched_subscriptions(
        const DataWriter<T>& dw,
        FwdIterator begin,
        uint32_t max_size)
{
    ::dds::core::InstanceHandleSeq seq = dw.delegate()->get_matched_subscriptions();
    uint32_t handle_num = seq.size() < max_size ? seq.size() : max_size;
    for (uint32_t h = 0; h < handle_num; h++)
    {
        *begin = seq[h];
    }
    return handle_num;
}

template<typename T>
const dds::topic::SubscriptionBuiltinTopicData matched_subscription_data(
        const DataWriter<T>& dw,
        const ::dds::core::InstanceHandle& h)
{
    return dw.delegate()->get_matched_subscription_data(h);
}

} //namespace pub
} //namespace dds

/** @endcond */
