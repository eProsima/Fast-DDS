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
 * @file RTPSParticipantAllocationAttributes.hpp
 */

#ifndef _FASTDDS_RTPS_RTPSPARTICIPANTALLOCATIONATTRIBUTES_HPP_
#define _FASTDDS_RTPS_RTPSPARTICIPANTALLOCATIONATTRIBUTES_HPP_

#include <fastrtps/utils/collections/ResourceLimitedContainerConfig.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * @brief Holds limits for collections of remote locators.
 */
struct RemoteLocatorsAllocationAttributes
{
    bool operator ==(
            const RemoteLocatorsAllocationAttributes& b) const
    {
        return (this->max_unicast_locators == b.max_unicast_locators) &&
               (this->max_multicast_locators == b.max_multicast_locators);
    }

    /** Maximum number of unicast locators per remote entity.
     *
     * This attribute controls the maximum number of unicast locators to keep for
     * each discovered remote entity (be it a participant, reader of writer). It is
     * recommended to use the highest number of local addresses found on all the systems
     * belonging to the same domain as this participant.
     */
    size_t max_unicast_locators = 4u;

    /** Maximum number of multicast locators per remote entity.
     *
     * This attribute controls the maximum number of multicast locators to keep for
     * each discovered remote entity (be it a participant, reader of writer). The
     * default value of 1 is usually enough, as it doesn't make sense to add more
     * than one multicast locator per entity.
     */
    size_t max_multicast_locators = 1u;
};

/**
 * @brief Holds limits for send buffers allocations.
 */
struct SendBuffersAllocationAttributes
{
    bool operator ==(
            const SendBuffersAllocationAttributes& b) const
    {
        return (this->preallocated_number == b.preallocated_number) &&
               (this->dynamic == b.dynamic);
    }

    /** Initial number of send buffers to allocate.
     *
     * This attribute controls the initial number of send buffers to be allocated.
     * The default value of 0 will perform an initial guess of the number of buffers
     * required, based on the number of threads from which a send operation could be
     * started.
     */
    size_t preallocated_number = 0u;

    /** Whether the number of send buffers is allowed to grow.
     *
     * This attribute controls how the buffer manager behaves when a send buffer is not
     * available. When true, a new buffer will be created. When false, it will wait for a
     * buffer to be returned. This is a trade-off between latency and dynamic allocations.
     */
    bool dynamic = false;
};

/**
 * @brief Holds limits for variable-length data.
 */
struct VariableLengthDataLimits
{
    bool operator ==(
            const VariableLengthDataLimits& b) const
    {
        return (this->max_properties == b.max_properties) &&
               (this->max_user_data == b.max_user_data) &&
               (this->max_partitions == b.max_partitions) &&
               (this->max_datasharing_domains == b.max_datasharing_domains);
    }

    //! Defines the maximum size (in octets) of properties data in the local or remote participant
    size_t max_properties = 0;
    //! Defines the maximum size (in octets) of user data in the local or remote participant
    size_t max_user_data = 0;
    //! Defines the maximum size (in octets) of partitions data
    size_t max_partitions = 0;
    //! Defines the maximum size (in elements) of the list of data sharing domain IDs
    size_t max_datasharing_domains = 0;
};

/**
 * @brief Holds allocation limits affecting collections managed by a participant.
 */
struct RTPSParticipantAllocationAttributes
{
    //! Holds limits for collections of remote locators.
    RemoteLocatorsAllocationAttributes locators;
    //! Defines the allocation behaviour for collections dependent on the total number of participants.
    ResourceLimitedContainerConfig participants;
    //! Defines the allocation behaviour for collections dependent on the total number of readers per participant.
    ResourceLimitedContainerConfig readers;
    //! Defines the allocation behaviour for collections dependent on the total number of writers per participant.
    ResourceLimitedContainerConfig writers;
    //! Defines the allocation behaviour for the send buffer manager.
    SendBuffersAllocationAttributes send_buffers;
    //! Holds limits for variable-length data
    VariableLengthDataLimits data_limits;

    //! @return the allocation config for the total of readers in the system (participants * readers)
    ResourceLimitedContainerConfig total_readers() const
    {
        return total_endpoints(readers);
    }

    //! @return the allocation config for the total of writers in the system (participants * writers)
    ResourceLimitedContainerConfig total_writers() const
    {
        return total_endpoints(writers);
    }

    bool operator ==(
            const RTPSParticipantAllocationAttributes& b) const
    {
        return (this->locators == b.locators) &&
               (this->participants == b.participants) &&
               (this->readers == b.readers) &&
               (this->writers == b.writers) &&
               (this->send_buffers == b.send_buffers) &&
               (this->data_limits == b.data_limits);
    }

private:

    ResourceLimitedContainerConfig total_endpoints(
            const ResourceLimitedContainerConfig& endpoints) const
    {
        constexpr size_t max = (std::numeric_limits<size_t>::max)();
        size_t initial;
        size_t maximum;
        size_t increment;

        initial = participants.initial * endpoints.initial;
        maximum = (participants.maximum == max || endpoints.maximum == max)
                ? max : participants.maximum * endpoints.maximum;
        increment = (std::max)(participants.increment, endpoints.increment);

        return { initial, maximum, increment };
    }

};

const RTPSParticipantAllocationAttributes c_default_RTPSParticipantAllocationAttributes
    = RTPSParticipantAllocationAttributes();

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_RTPSPARTICIPANTALLOCATIONATTRIBUTES_HPP_ */
