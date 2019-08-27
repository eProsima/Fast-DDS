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

#ifndef EPROSIMA_DDS_CORE_POLICY_TQOSPOLICYCOUNT_IMPL_HPP_
#define EPROSIMA_DDS_CORE_POLICY_TQOSPOLICYCOUNT_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/policy/QosPolicyCount.hpp>

// Implementation

namespace dds {
namespace core {
namespace policy {

template<typename D>
TQosPolicyCount<D>::TQosPolicyCount(
        QosPolicyId policy_id,
        int32_t count)
    : dds::core::Value<D>(policy_id, count) { }

template<typename D>
TQosPolicyCount<D>::TQosPolicyCount(
        const TQosPolicyCount& other)
    : dds::core::Value<D>(other.policy_id(), other.count()) { }

template<typename D>
QosPolicyId TQosPolicyCount<D>::policy_id() const
{
    //To implement
}

template<typename D>
int32_t TQosPolicyCount<D>::count() const
{
    //To implement
}

} //namespace policy
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_POLICY_TQOSPOLICYCOUNT_IMPL_HPP_
