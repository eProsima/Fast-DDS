// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PDPClientListener.cpp
 *
 */

#include <rtps/builtin/discovery/participant/PDPClientListener.hpp>

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
#include <rtps/network/utils/external_locators.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/resources/TimedEvent.h>

using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastdds {
namespace rtps {

PDPClientListener::PDPClientListener(
        PDP* parent_pdp)
    : PDPListener(parent_pdp)
{
}

bool PDPClientListener::check_discovery_conditions(
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
