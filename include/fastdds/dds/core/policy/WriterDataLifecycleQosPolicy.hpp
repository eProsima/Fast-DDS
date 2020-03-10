// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file WriterDataLifecycleQosPolicy.hpp
 */

#ifndef _FASTDDS_WRITERDATALIFECYCLEQOSPOLICY_HPP_
#define _FASTDDS_WRITERDATALIFECYCLEQOSPOLICY_HPP_

#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

//! @brief A struct storing the base status
class WriterDataLifecycleQosPolicy : public QosPolicy
{
public:

    WriterDataLifecycleQosPolicy()
        : QosPolicy(false, (QosPolicyId_t)16)
        , autodispose_unregistered_instances(true)
    {}

    WriterDataLifecycleQosPolicy(
            bool autodispose)
        : QosPolicy(false, (QosPolicyId_t)16)
        ,  autodispose_unregistered_instances(autodispose)
    {}

    virtual RTPS_DllAPI ~WriterDataLifecycleQosPolicy() {}

    bool operator ==(
            const WriterDataLifecycleQosPolicy& b) const
    {
        return (this->autodispose_unregistered_instances == b.autodispose_unregistered_instances);
    }

    inline void clear() override
    {
        WriterDataLifecycleQosPolicy reset = WriterDataLifecycleQosPolicy();
        std::swap(*this, reset);
    }

public:

    bool autodispose_unregistered_instances;

};

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // _FASTDDS_WRITERDATALIFECYCLEQOSPOLICY_HPP_
