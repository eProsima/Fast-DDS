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
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.hpp>
#include <fastdds/utils/IPLocator.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

ParticipantModule::ParticipantModule(
        const std::string& discovery_protocol,
        const std::string& guid_prefix,
        const std::string& unicast_metatraffic_port)
{
    if (discovery_protocol.compare(ParticipantType::SERVER) == 0)
    {
        discovery_protocol_ = DiscoveryProtocol::SERVER;
        std::istringstream server_guid_prefix_str(guid_prefix);
        server_guid_prefix_str >> server_guid_prefix_;
        unicast_metatraffic_port_ = atoi(unicast_metatraffic_port.c_str());
    }
    else if (discovery_protocol.compare(ParticipantType::CLIENT) == 0)
    {
        discovery_protocol_ = DiscoveryProtocol::CLIENT;
    }
    else
    {
        discovery_protocol_ = DiscoveryProtocol::SIMPLE;
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
    if (DiscoveryProtocol::SERVER == discovery_protocol_)
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
        ParticipantDiscoveryStatus status,
        const ParticipantBuiltinTopicData& info,
        bool& should_be_ignored)
{
    static_cast<void>(should_be_ignored);
    if (status == ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Participant " << participant->guid() << " discovered participant " << info.guid << ": "
                  << ++matched_ << std::endl;
    }
    else if (status == ParticipantDiscoveryStatus::CHANGED_QOS_PARTICIPANT)
    {
        std::cout << "Participant " << participant->guid() << " detected changes on participant " << info.guid
                  << std::endl;
    }
    else if (status == ParticipantDiscoveryStatus::REMOVED_PARTICIPANT ||
            status == ParticipantDiscoveryStatus::DROPPED_PARTICIPANT)
    {
        std::cout << "Participant " << participant->guid() << " undiscovered participant " << info.guid << ": "
                  << --matched_ << std::endl;
    }
}
