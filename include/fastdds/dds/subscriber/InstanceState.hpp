// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file InstanceState.hpp
 */

#ifndef _FASTDDS_DDS_SUBSCRIBER_INSTANCESTATE_HPP_
#define _FASTDDS_DDS_SUBSCRIBER_INSTANCESTATE_HPP_

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Indicates if the samples are from an alive @ref DataWriter or not.
 *
 * For each instance, the middleware internally maintains an instance state. The instance state
 * can be:
 *
 * @li @ref ALIVE_INSTANCE_STATE indicates that (a) samples have been received for the instance,
 *     (b) there are alive @ref DataWriter entities writing the instance, and (c) the instance has
 *     not been explicitly disposed (or else more samples have been received after it was disposed).
 *
 * @li @ref NOT_ALIVE_DISPOSED_INSTANCE_STATE indicates the instance was explicitly disposed by a
 *     @ref DataWriter by means of the dispose operation.
 *
 * @li @ref NOT_ALIVE_NO_WRITERS_INSTANCE_STATE indicates the instance has been declared as
 *     not-alive by the @ref DataReader because it detected that there are no alive @ref DataWriter
 *     entities writing that instance.
 *
 * The precise behavior events that cause the instance state to change depends on the setting of the
 * OWNERSHIP QoS:
 *
 * @li If OWNERSHIP is set to EXCLUSIVE_OWNERSHIP_QOS, then the instance state becomes
 *     @ref NOT_ALIVE_DISPOSED_INSTANCE_STATE only if the @ref DataWriter that "owns" the instance
 *     explicitly disposes it. The instance state becomes @ref ALIVE_INSTANCE_STATE again only
 *     if the @ref DataWriter that owns the instance writes it.
 *
 * @li If OWNERSHIP is set to SHARED_OWNERSHIP_QOS, then the instance state becomes
 *     @ref NOT_ALIVE_DISPOSED_INSTANCE_STATE if any @ref DataWriter explicitly disposes the
 *     instance. The instance state becomes @ref ALIVE_INSTANCE_STATE as soon as any
 *     @ref DataWriter writes the instance again.
 *
 * The instance state available in the @ref SampleInfo is a snapshot of the instance state of the
 * instance at the time the collection was obtained (i.e. at the time read or take was called). The
 * instance state is therefore the same for all samples in the returned collection that refer to the
 * same instance.
 */
enum InstanceStateKind : uint16_t
{
    /// Instance is currently in existence.
    ALIVE_INSTANCE_STATE = 0x0001 << 0,
    /// Not alive disposed instance. The instance has been disposed by a DataWriter.
    NOT_ALIVE_DISPOSED_INSTANCE_STATE = 0x0001 << 1,
    /// Not alive no writers for instance. None of the @ref DataWriter objects that are
    /// currently alive (according to the LIVELINESS QoS) are writing the instance.
    NOT_ALIVE_NO_WRITERS_INSTANCE_STATE = 0x0001 << 2
};

/// A bit-mask (list) of instance states, i.e. @ref InstanceStateKind
using InstanceStateMask = uint16_t;

/// Not alive instance state
constexpr InstanceStateMask NOT_ALIVE_INSTANCE_STATE =
        NOT_ALIVE_DISPOSED_INSTANCE_STATE | NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;
/// Any instance state
constexpr InstanceStateMask ANY_INSTANCE_STATE = ALIVE_INSTANCE_STATE | NOT_ALIVE_INSTANCE_STATE;

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_DDS_SUBSCRIBER_INSTANCESTATE_HPP_
