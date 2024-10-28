// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PDPEndpoints.hpp
 */

#ifndef FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDPENDPOINTS_HPP_
#define FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDPENDPOINTS_HPP_

#include <fastdds/rtps/common/Types.hpp>

#include <rtps/participant/RTPSParticipantImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Models that the base PDP class can have a different container for its builtin endpoints.
 * Each subclass of PDP might contain a different subclass of PDPEndpoints.
 */
class PDPEndpoints
{
public:

    // Designed for inheritance, so destructor must be virtual
    virtual ~PDPEndpoints() = default;

    /**
     * Returns a mask with the list of builtin endpoints contained by this class.
     *
     * @return The list of builtin endpoints to announce.
     */
    virtual fastdds::rtps::BuiltinEndpointSet_t builtin_endpoints() const = 0;

    virtual const std::unique_ptr<fastdds::rtps::ReaderListener>& main_listener() const = 0;

    virtual bool enable_pdp_readers(
            fastdds::rtps::RTPSParticipantImpl* participant) = 0;
    virtual void disable_pdp_readers(
            fastdds::rtps::RTPSParticipantImpl* participant) = 0;
    virtual void delete_pdp_endpoints(
            fastdds::rtps::RTPSParticipantImpl* participant) = 0;
    virtual void remove_from_pdp_reader_history(
            const fastdds::rtps::InstanceHandle_t& remote_participant) = 0;
    virtual void remove_from_pdp_reader_history(
            fastdds::rtps::CacheChange_t* change) = 0;

protected:

    // Cannot be directly constructed
    PDPEndpoints() = default;

    // Non-copyable, non-moveable
    PDPEndpoints(
            const PDPEndpoints&) = delete;
    PDPEndpoints(
            PDPEndpoints&&) = delete;
    PDPEndpoints& operator =(
            const PDPEndpoints&) = delete;
    PDPEndpoints& operator =(
            PDPEndpoints&&) = delete;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDPENDPOINTS_HPP_
