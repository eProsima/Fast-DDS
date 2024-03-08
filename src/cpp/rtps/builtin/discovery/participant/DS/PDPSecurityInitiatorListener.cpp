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

#include <fastdds/dds/log/Log.hpp>

#include <fastdds/rtps/builtin/discovery/endpoint/EDP.h>
#include <fastdds/rtps/builtin/discovery/participant/PDP.h>
#include <fastdds/rtps/builtin/discovery/participant/PDPListener.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastdds/rtps/participant/RTPSParticipantListener.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/resources/TimedEvent.h>

#include <fastrtps/utils/TimeConversion.h>

#include <fastdds/core/policy/ParameterList.hpp>
#include <rtps/builtin/discovery/participant/PDPEndpoints.hpp>
#include <rtps/network/utils/external_locators.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>

#include <mutex>

using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastrtps {
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

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
