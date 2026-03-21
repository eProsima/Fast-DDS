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

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

#include <statistics/fastdds/domain/DomainParticipantImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

fastdds::dds::ReturnCode_t DomainParticipant::enable_statistics_datawriter(
        const std::string& topic_name,
        const eprosima::fastdds::dds::DataWriterQos& dwqos)
{
#ifndef FASTDDS_STATISTICS
    (void) topic_name;
    (void) dwqos;

    return fastdds::dds::RETCODE_UNSUPPORTED;
#else
    return static_cast<DomainParticipantImpl*>(impl_)->enable_statistics_datawriter(topic_name, dwqos);
#endif // FASTDDS_STATISTICS
}

fastdds::dds::ReturnCode_t DomainParticipant::enable_statistics_datawriter_with_profile(
        const std::string& profile_name,
        const std::string& topic_name)
{
#ifndef FASTDDS_STATISTICS
    (void) profile_name;
    (void) topic_name;

    return fastdds::dds::RETCODE_UNSUPPORTED;
#else
    return static_cast<DomainParticipantImpl*>(impl_)->enable_statistics_datawriter_with_profile(profile_name,
                   topic_name);
#endif // FASTDDS_STATISTICS
}

fastdds::dds::ReturnCode_t DomainParticipant::disable_statistics_datawriter(
        const std::string& topic_name)
{
#ifndef FASTDDS_STATISTICS
    (void) topic_name;

    return fastdds::dds::RETCODE_UNSUPPORTED;
#else
    return static_cast<DomainParticipantImpl*>(impl_)->disable_statistics_datawriter(topic_name);
#endif // FASTDDS_STATISTICS
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
    (void)domain_participant;
    return nullptr;
#endif // FASTDDS_STATISTICS
}

fastdds::dds::ReturnCode_t DomainParticipant::enable_monitor_service()
{
#ifdef FASTDDS_STATISTICS
    return static_cast<DomainParticipantImpl*>(impl_)->enable_monitor_service();
#else
    return fastdds::dds::RETCODE_UNSUPPORTED;
#endif // FASTDDS_STATISTICS
}

fastdds::dds::ReturnCode_t DomainParticipant::disable_monitor_service()
{
#ifdef FASTDDS_STATISTICS
    return static_cast<DomainParticipantImpl*>(impl_)->disable_monitor_service();
#else
    return fastdds::dds::RETCODE_UNSUPPORTED;
#endif // FASTDDS_STATISTICS
}

fastdds::dds::ReturnCode_t DomainParticipant::fill_discovery_data_from_cdr_message(
        fastdds::rtps::ParticipantBuiltinTopicData& data,
        const fastdds::statistics::MonitorServiceStatusData& msg)
{
#ifdef FASTDDS_STATISTICS
    return static_cast<DomainParticipantImpl*>(impl_)->fill_discovery_data_from_cdr_message(data, msg);
#else
    (void)data;
    (void)msg;
    return fastdds::dds::RETCODE_UNSUPPORTED;
#endif // FASTDDS_STATISTICS
}

fastdds::dds::ReturnCode_t DomainParticipant::fill_discovery_data_from_cdr_message(
        fastdds::dds::PublicationBuiltinTopicData& data,
        const fastdds::statistics::MonitorServiceStatusData& msg)
{
#ifdef FASTDDS_STATISTICS
    return static_cast<DomainParticipantImpl*>(impl_)->fill_discovery_data_from_cdr_message(data, msg);
#else
    (void)data;
    (void)msg;
    return fastdds::dds::RETCODE_UNSUPPORTED;
#endif // FASTDDS_STATISTICS
}

fastdds::dds::ReturnCode_t DomainParticipant::fill_discovery_data_from_cdr_message(
        fastdds::dds::SubscriptionBuiltinTopicData& data,
        const fastdds::statistics::MonitorServiceStatusData& msg)
{
#ifdef FASTDDS_STATISTICS
    return static_cast<DomainParticipantImpl*>(impl_)->fill_discovery_data_from_cdr_message(data, msg);
#else
    (void)data;
    (void)msg;
    return fastdds::dds::RETCODE_UNSUPPORTED;
#endif // FASTDDS_STATISTICS
}

} // dds
} // statistics
} // fastdds
} // eprosima
