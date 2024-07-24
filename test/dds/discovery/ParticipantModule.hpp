// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributes under the License is distributes on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permisssions and
// limitations under the license.

/**
 * @file ParticipantModule.hpp
 */

#ifndef TEST_DDS_COMMUNICATION_PARTICIPANTMODULE_HPP
#define TEST_DDS_COMMUNICATION_PARTICIPANTMODULE_HPP

#include <string>

#include <fastdds/dds/builtin/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

using namespace eprosima::fastdds::rtps;

class ParticipantModule : public DomainParticipantListener
{
public:

    ParticipantModule(
            const std::string& discovery_protocol,
            const std::string& guid_prefix,
            const std::string& unicast_metatraffic_port);

    ~ParticipantModule();

    void on_participant_discovery(
            DomainParticipant* participant,
            ParticipantDiscoveryStatus status,
            const ParticipantBuiltinTopicData& info,
            bool& should_be_ignored) override;

    bool init();

private:

    using DomainParticipantListener::on_participant_discovery;

    unsigned int matched_ = 0;
    DomainParticipant* participant_ = nullptr;
    DiscoveryProtocol discovery_protocol_;
    GuidPrefix_t server_guid_prefix_;
    uint32_t unicast_metatraffic_port_;
};

} // dds
} // fastdds
} // eprosima

#endif // TEST_DDS_COMMUNICATION_PARTICIPANTMODULE.HPP
