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
 * @file DiscoveryServerPDPEndpointsSecure.hpp
 */

#ifndef FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_DS__DISCOVERYSERVERPDPENDPOINTSSECURE_HPP_
#define FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_DS__DISCOVERYSERVERPDPENDPOINTSSECURE_HPP_

#include <memory>

#include <fastdds/rtps/builtin/data/BuiltinEndpoints.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <rtps/builtin/BuiltinReader.hpp>
#include <rtps/builtin/discovery/participant/DS/DiscoveryServerPDPEndpoints.hpp>
#include <rtps/reader/StatelessReader.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Container for the builtin endpoints of secure PDPClient and PDPServer
 */
struct DiscoveryServerPDPEndpointsSecure : public DiscoveryServerPDPEndpoints
{
    ~DiscoveryServerPDPEndpointsSecure() override = default;

    BuiltinEndpointSet_t builtin_endpoints() const override
    {
        return DiscoveryServerPDPEndpoints::builtin_endpoints() |
               DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_ANNOUNCER | DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_DETECTOR;
    }

    bool enable_pdp_readers(
            RTPSParticipantImpl* participant) override
    {
        return DiscoveryServerPDPEndpoints::enable_pdp_readers(participant) &&
               participant->enableReader(stateless_reader.reader_);
    }

    void disable_pdp_readers(
            RTPSParticipantImpl* participant) override
    {
        participant->disableReader(stateless_reader.reader_);
        DiscoveryServerPDPEndpoints::disable_pdp_readers(participant);
    }

    void delete_pdp_endpoints(
            RTPSParticipantImpl* participant) override
    {
        participant->deleteUserEndpoint(stateless_reader.reader_->getGuid());
        DiscoveryServerPDPEndpoints::delete_pdp_endpoints(participant);
    }

    void remove_from_pdp_reader_history(
            const InstanceHandle_t& remote_participant) override
    {
        stateless_reader.remove_from_history(remote_participant);
        DiscoveryServerPDPEndpoints::remove_from_pdp_reader_history(remote_participant);
    }

    void remove_from_pdp_reader_history(
            CacheChange_t* change) override
    {
        assert(nullptr != change);
        if (change->writerGUID.entityId == c_EntityId_SPDPWriter)
        {
            stateless_reader.history_->remove_change(change);
        }
        else
        {
            DiscoveryServerPDPEndpoints::remove_from_pdp_reader_history(change);
        }
    }

    //! Builtin Simple PDP reader
    BuiltinReader<StatelessReader> stateless_reader;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_DS__DISCOVERYSERVERPDPENDPOINTSSECURE_HPP_
