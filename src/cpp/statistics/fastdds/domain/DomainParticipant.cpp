// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DomainParticipant.cpp
 */

#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>

#include <string>

#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

ReturnCode_t DomainParticipant::enable_statistics_datawriter(
        const std::string& topic_name,
        const eprosima::fastdds::dds::DataWriterQos& dwqos)
{
    (void) topic_name;
    (void) dwqos;

    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

ReturnCode_t DomainParticipant::disable_statistics_datawriter(
        const std::string& topic_name)
{
    (void) topic_name;

    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

DomainParticipant* DomainParticipant::narrow(
        eprosima::fastdds::dds::DomainParticipant* domain_participant)
{
#ifdef FASTDDS_STATISTICS
    return static_cast<DomainParticipant*>(domain_participant);
#else
    (void)domain_participant;
    return nullptr;
#endif // FASTDDS_STATISTICS
}

const DomainParticipant* DomainParticipant::narrow(
        const eprosima::fastdds::dds::DomainParticipant* domain_participant)
{
#ifdef FASTDDS_STATISTICS
    return static_cast<const DomainParticipant*>(domain_participant);
#else
    (void) domain_participant;
    return nullptr;
#endif // FASTDDS_STATISTICS
}

} // dds
} // statistics
} // fastdds
} // eprosima
