// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unful required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file SimplePDPEndpointsSecure.hpp
 */

#ifndef FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_SIMPLE__SIMPLEPDPENDPOINTSSECURE_HPP_
#define FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_SIMPLE__SIMPLEPDPENDPOINTSSECURE_HPP_

#include <memory>

#include <fastdds/rtps/builtin/data/BuiltinEndpoints.hpp>

#include <rtps/builtin/BuiltinReader.hpp>
#include <rtps/builtin/BuiltinWriter.hpp>
#include <rtps/builtin/discovery/participant/simple/SimplePDPEndpoints.hpp>
#include <rtps/history/ITopicPayloadPool.h>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/writer/StatefulWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Container for the builtin endpoints of SPDPSimple
 */
struct SimplePDPEndpointsSecure : public SimplePDPEndpoints
{
    ~SimplePDPEndpointsSecure() override = default;

    BuiltinEndpointSet_t builtin_endpoints() const override
    {
        return SimplePDPEndpoints::builtin_endpoints() |
               DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_ANNOUNCER | DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_DETECTOR;
    }

    const std::unique_ptr<ReaderListener>& main_listener() const override
    {
        return secure_reader.listener_;
    }

    bool enable_pdp_readers(
            RTPSParticipantImpl* participant) override
    {
        return SimplePDPEndpoints::enable_pdp_readers(participant) &&
               participant->enableReader(secure_reader.reader_);
    }

    void disable_pdp_readers(
            RTPSParticipantImpl* participant) override
    {
        participant->disableReader(secure_reader.reader_);
        SimplePDPEndpoints::disable_pdp_readers(participant);
    }

    void delete_pdp_endpoints(
            RTPSParticipantImpl* participant) override
    {
        participant->deleteUserEndpoint(secure_reader.reader_->getGuid());
        participant->deleteUserEndpoint(secure_writer.writer_->getGuid());
        SimplePDPEndpoints::delete_pdp_endpoints(participant);
    }

    void remove_from_pdp_reader_history(
            const InstanceHandle_t& remote_participant) override
    {
        secure_reader.remove_from_history(remote_participant);
        SimplePDPEndpoints::remove_from_pdp_reader_history(remote_participant);
    }

    void remove_from_pdp_reader_history(
            CacheChange_t* change) override
    {
        assert(nullptr != change);
        if (change->writerGUID.entityId == c_EntityId_SPDPWriter)
        {
            SimplePDPEndpoints::remove_from_pdp_reader_history(change);
        }
        else
        {
            secure_reader.history_->remove_change(change);
        }
    }

    //! Builtin Simple PDP secure reader
    BuiltinReader<StatefulReader> secure_reader;

    //! Builtin Simple PDP secure writer
    BuiltinWriter<StatefulWriter> secure_writer;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_SIMPLE__SIMPLEPDPENDPOINTSSECURE_HPP_
