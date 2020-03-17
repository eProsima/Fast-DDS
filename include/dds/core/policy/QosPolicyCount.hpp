/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef OMG_DDS_CORE_POLICY_QOS_POLICY_COUNT_HPP_
#define OMG_DDS_CORE_POLICY_QOS_POLICY_COUNT_HPP_

#include <dds/core/policy/detail/QosPolicyCount.hpp>
#include <dds/core/Value.hpp>

namespace dds {
namespace core {
namespace policy {

using QosPolicyId = detail::QosPolicyId;
using QosPolicyCountSeq = detail::QosPolicyCountSeq;

/**
 * The QosPolicyCount object shows, for a QosPolicy, the total number of
 * times that the concerned DataWriter discovered a DataReader for the
 * same Topic and a requested DataReaderQos that is incompatible with
 * the one offered by the DataWriter.
 */
class QosPolicyCount : public dds::core::Value<detail::QosPolicyCount>
{
public:

    /**
     * Creates a QosPolicyCount instance
     *
     * @param policy_id the policy_id
     * @param count the count
     */
    QosPolicyCount(
            QosPolicyId policy_id,
            int32_t count)
        : dds::core::Value<detail::QosPolicyCount>(policy_id, count)
    {
    }

    /**
     * Copies a QosPolicyCount instance
     *
     * @param other the QosPolicyCount instance to copy
     */
    QosPolicyCount(
            const QosPolicyCount& other)
        : dds::core::Value<detail::QosPolicyCount>(other.policy_id(), other.count())
    {
    }

    /**
     * Gets the policy_id
     *
     * @return the policy_id
     */
    QosPolicyId policy_id() const
    {
        return this->delegate().policy_id;
    }

    /**
     * Gets the count
     *
     * @return the count
     */
    int32_t count() const
    {
        return this->delegate().count;
    }

};

} //namespace policy
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_POLICY_QOS_POLICY_COUNT_HPP_
