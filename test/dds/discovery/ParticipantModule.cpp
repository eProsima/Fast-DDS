// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima)
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
 * @file ParticipantModule.cpp
 */

# include "ParticipantModule.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastrtps/utils/IPLocator.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastrtps::rtps;

ParticipantModule::ParticipantModule(
        const std::string& discovery_protocol,
        const std::string& guid_prefix,
        const std::string& unicast_metatraffic_port)
{
    if (discovery_protocol.compare(ParticipantType::SERVER) == 0)
    {
        discovery_protocol_ = DiscoveryProtocol_t::SERVER;
        std::istringstream server_guid_prefix_str(guid_prefix);
        server_guid_prefix_str >> server_guid_prefix_;
        unicast_metatraffic_port_ = atoi(unicast_metatraffic_port.c_str());
    }
    else if (discovery_protocol.compare(ParticipantType::CLIENT) == 0)
    {
        discovery_protocol_ = DiscoveryProtocol_t::CLIENT;
    }
    else
    {
        discovery_protocol_ = DiscoveryProtocol_t::SIMPLE;
    }
}

ParticipantModule::~ParticipantModule()
{
    if (nullptr != participant_)
    {
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

bool ParticipantModule::init()
{
    DomainParticipantQos participant_qos;
    participant_qos.wire_protocol().builtin.discovery_config.discoveryProtocol = discovery_protocol_;
    if (DiscoveryProtocol_t::SERVER == discovery_protocol_)
    {
        participant_qos.wire_protocol().prefix = server_guid_prefix_;
        Locator_t locator_server;
        IPLocator::setIPv4(locator_server, "127.0.0.1");
        locator_server.port = unicast_metatraffic_port_;
        participant_qos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator_server);
    }
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participant_qos, this);

    if (nullptr == participant_)
    {
        return false;
    }
    return true;
}

void ParticipantModule::on_participant_discovery(
        DomainParticipant* participant,
        ParticipantDiscoveryInfo&& info)
{
    if (info.status == ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Participant " << participant->guid() << " discovered participant " << info.info.m_guid << ": "
                  << ++matched_ << std::endl;
    }
    else if (info.status == ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT)
    {
        std::cout << "Participant " << participant->guid() << " detected changes on participant " << info.info.m_guid
                  << std::endl;
    }
    else if (info.status == ParticipantDiscoveryInfo::REMOVED_PARTICIPANT ||
            info.status == ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
    {
        std::cout << "Participant " << participant->guid() << " undiscovered participant " << info.info.m_guid << ": "
                  << --matched_ << std::endl;
    }
}
