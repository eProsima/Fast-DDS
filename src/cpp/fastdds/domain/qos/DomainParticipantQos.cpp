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

#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include <fastdds/rtps/attributes/BuiltinTransports.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/utils/QosConverters.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

const DomainParticipantQos PARTICIPANT_QOS_DEFAULT;

void DomainParticipantQos::setup_transports(
        rtps::BuiltinTransports transports,
        const rtps::BuiltinTransportsOptions& options)
{
    fastrtps::rtps::RTPSParticipantAttributes attr;
    utils::set_attributes_from_qos(attr, *this);

    attr.setup_transports(transports, options);

    utils::set_qos_from_attributes(*this, attr);
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
