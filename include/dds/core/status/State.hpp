/*
 * Copyright 2010, Object Management Group, Inc.
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

#ifndef OMG_DDS_CORE_STATUS_STATE_HPP_
#define OMG_DDS_CORE_STATUS_STATE_HPP_

#include <bitset>
#include <sstream>
#include <dds/core/macros.hpp>

#include <fastdds/dds/core/status/StatusMask.hpp>


namespace dds {
namespace core {
namespace status {

/**
 * @brief
 * Class to contain the statistics about samples that have been rejected.
 *
 * @see @ref DCPS_Modules_Subscription_SampleInfo "SampleInfo" for more information
 */
class OMG_DDS_API SampleRejectedState : public std::bitset<OMG_DDS_STATE_BIT_COUNT>
{
public:

    /**
     * Convenience typedef for std::bitset<OMG_DDS_STATE_BIT_COUNT>.
     */
    typedef std::bitset<OMG_DDS_STATE_BIT_COUNT> MaskType;

    /**
     * Construct an empty SampleRejectedState.
     */
    SampleRejectedState()
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>()
    {
    }

    /**
     * Copy constructor.
     *
     * Construct an SampleRejectedState with existing SampleRejectedState.
     *
     * @param src the SampleRejectedState to copy from
     */
    SampleRejectedState(
            const SampleRejectedState& src)
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>(src)
    {
    }

    /**
     * Construct a SampleRejectedState with existing MaskType.
     *
     * @param src the MaskType to copy from
     */
    SampleRejectedState(
            const MaskType& src)
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>(src)
    {
    }

    /**
     * Get the NOT_REJECTED.
     *
     *<i>not_rejected</i>
     *      - No sample has been rejected yet.
     *
     * @return the not_rejected SampleRejectedState
     */
    inline static SampleRejectedState not_rejected()
    {
        return SampleRejectedState(0u);
    }

    /**
     * Get the REJECTED_BY_SAMPLES_LIMIT.
     *
     *<i>rejected_by_samples_limit</i>
     *      - The sample was rejected because it would
     *        exceed the maximum number of samples set by the ResourceLimits
     *        QosPolicy.
     *
     * @return the rejected_by_samples_limit SampleRejectedState
     */
    inline static SampleRejectedState rejected_by_samples_limit()
    {
        return SampleRejectedState(0x0001 << 1u);
    }

    /**
     * Get the REJECTED_BY_INSTANCES_LIMIT.
     *
     *<i>rejected_by_instances_limit</i>
     *      - The sample was rejected because it would
     *        exceed the maximum number of instances set by the
     *        ResourceLimits QosPolicy.
     *
     * @return the rejected_by_instances_limit SampleRejectedState
     */
    inline static SampleRejectedState rejected_by_instances_limit()
    {
        return SampleRejectedState(0x0001 << 0u);
    }

    /**
     * Get the REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT.
     *
     *<i>rejected_by_samples_per_instance_limit</i>
     *      - The sample was rejected
     *        because it would exceed the maximum number of samples per
     *        instance set by the ResourceLimits QosPolicy.
     *
     * @return the rejected_by_samples_per_instance_limit SampleRejectedState
     */
    inline static SampleRejectedState rejected_by_samples_per_instance_limit()
    {
        return SampleRejectedState(0x0001 << 2u);
    }

private:

    SampleRejectedState(
            uint32_t s)
        : std::bitset<OMG_DDS_STATE_BIT_COUNT>(s)
    {
    }

};

/**
 * @brief
 * StatusMask is a bitmap or bitset field.
 *
 * This bitset is used to:
 * - determine which listener functions to call
 * - set conditions in dds::core::cond::StatusCondition
 * - indicate status changes when calling dds::core::Entity::status_changes
 *
 * @see @ref DCPS_Modules_Infrastructure_Status  "Status concept"
 */
using StatusMask = eprosima::fastdds::dds::StatusMask;

} //namespace status
} //namespace core
} //namespace dds


#endif //OMG_DDS_CORE_STATUS_STATE_HPP_
