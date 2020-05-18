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

namespace eprosima {
namespace fastdds {
namespace dds {

//! @brief Specifies the behavior of the DataWriter with regards to the lifecycle of the data-instances it manages.
//! @warning This Qos Policy will be implemented in future releases.
//! @note Mutable Qos Policy
class WriterDataLifecycleQosPolicy
{
public:

    /**
     * @brief Constructor
     */
    WriterDataLifecycleQosPolicy()
        : autodispose_unregistered_instances(true)
    {
    }

    /**
     * @brief Destructor
     */
    virtual RTPS_DllAPI ~WriterDataLifecycleQosPolicy()
    {
    }

    bool operator ==(
            const WriterDataLifecycleQosPolicy& b) const
    {
        return (this->autodispose_unregistered_instances == b.autodispose_unregistered_instances);
    }

public:

    /**
     * @brief Controls whether a DataWriter will automatically dispose instances each time they are unregistered.
     * The setting autodispose_unregistered_instances = TRUE indicates that unregistered instances will also be considered
     * disposed. <br>
     * By default, true.
     */
    bool autodispose_unregistered_instances;

};

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // _FASTDDS_WRITERDATALIFECYCLEQOSPOLICY_HPP_
