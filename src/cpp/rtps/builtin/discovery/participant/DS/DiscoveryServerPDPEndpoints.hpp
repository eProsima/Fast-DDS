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
 * @file DiscoveryServerPDPEndpoints.hpp
 */

#ifndef FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_DS__DISCOVERYSERVERPDPENDPOINTS_HPP_
#define FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_DS__DISCOVERYSERVERPDPENDPOINTS_HPP_

#include <memory>

#include <fastdds/rtps/builtin/data/BuiltinEndpoints.hpp>

#include <rtps/builtin/BuiltinReader.hpp>
#include <rtps/builtin/BuiltinWriter.hpp>
#include <rtps/builtin/discovery/participant/PDPEndpoints.hpp>
#include <rtps/history/ITopicPayloadPool.h>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/writer/StatefulWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Container for the builtin endpoints of non-secure PDPClient and PDPServer
 */
struct DiscoveryServerPDPEndpoints : public PDPEndpoints
{
    ~DiscoveryServerPDPEndpoints() override = default;

    BuiltinEndpointSet_t builtin_endpoints() const override
    {
        return DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER | DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    }

    const std::unique_ptr<ReaderListener>& main_listener() const override
    {
        return reader.listener_;
    }

    bool enable_pdp_readers(
            RTPSParticipantImpl* participant) override
    {
        return participant->enableReader(reader.reader_);
    }

    void disable_pdp_readers(
            RTPSParticipantImpl* participant) override
    {
        participant->disableReader(reader.reader_);
    }

    void delete_pdp_endpoints(
            RTPSParticipantImpl* participant) override
    {
        participant->deleteUserEndpoint(writer.writer_->getGuid());
        participant->deleteUserEndpoint(reader.reader_->getGuid());
    }

    void remove_from_pdp_reader_history(
            const InstanceHandle_t& remote_participant) override
    {
        reader.remove_from_history(remote_participant);
    }

    void remove_from_pdp_reader_history(
            CacheChange_t* change) override
    {
        reader.history_->remove_change(change);
    }

    //! Builtin Simple PDP reader
    BuiltinReader<StatefulReader> reader;

    //! Builtin Simple PDP writer
    BuiltinWriter<StatefulWriter> writer;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_DS__DISCOVERYSERVERPDPENDPOINTS_HPP_
