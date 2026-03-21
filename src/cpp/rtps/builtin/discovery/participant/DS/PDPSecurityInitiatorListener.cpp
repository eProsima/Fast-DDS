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
 * @file PDPSecurityInitiatorListener.cpp
 *
 */

#include <rtps/builtin/discovery/participant/DS/PDPSecurityInitiatorListener.hpp>

#include <mutex>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipantListener.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/discovery/endpoint/EDP.h>
#include <rtps/builtin/discovery/participant/PDP.h>
#include <rtps/builtin/discovery/participant/PDPEndpoints.hpp>
#include <rtps/builtin/discovery/participant/PDPListener.h>
#include <rtps/network/utils/external_locators.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/resources/TimedEvent.h>

using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastdds {
namespace rtps {

PDPSecurityInitiatorListener::PDPSecurityInitiatorListener(
        PDP* parent,
        SecurityInitiatedCallback response_cb)
    : PDPListener(parent)
    , response_cb_(response_cb)
{
}

void PDPSecurityInitiatorListener::process_alive_data(
        ParticipantProxyData* old_data,
        ParticipantProxyData& new_data,
        GUID_t& writer_guid,
        RTPSReader* reader,
        std::unique_lock<std::recursive_mutex>& lock)
{
    if (reader->matched_writer_is_matched(writer_guid))
    {
        // Act as the standard PDPListener when the writer is matched.
        // This will be the case for unauthenticated participants when
        // allowed_unathenticated_participants is true
        PDPListener::process_alive_data(old_data, new_data, writer_guid, reader, lock);
        return;
    }

    if (old_data == nullptr)
    {
        auto callback_data = new_data;
        reader->getMutex().unlock();
        lock.unlock();

        //! notify security manager in order to start handshake
        bool ret = parent_pdp_->getRTPSParticipant()->security_manager().discovered_participant(callback_data);
        //! Reply to the remote participant
        if (ret)
        {
            response_cb_(callback_data);
        }

        // Take again the reader lock
        reader->getMutex().lock();
    }

}

bool PDPSecurityInitiatorListener::check_discovery_conditions(
        ParticipantProxyData& /* participant_data */)
{
    /* Do not check PID_VENDOR_ID */
    // In Discovery Server we don't impose
    // domain ids to be the same
    /* Do not check PID_DOMAIN_ID */
    /* Do not check PARTICIPANT_TYPE */
    return true;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
