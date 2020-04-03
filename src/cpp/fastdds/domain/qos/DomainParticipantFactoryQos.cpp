// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DomainParticipantQos.cpp
 *
 */

#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

void DomainParticipantFactoryQos::set_qos(
        const DomainParticipantFactoryQos& qos)
{
    if (entity_factory.autoenable_created_entities != qos.entity_factory.autoenable_created_entities)
    {
        entity_factory = qos.entity_factory;
        entity_factory.hasChanged = true;
    }
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
