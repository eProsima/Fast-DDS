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
#include <sstream>
#include <vector>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastrtps/types/TypesBase.h>

#include <fastdds/domain/DomainParticipantImpl.hpp>
#include <utils/SystemInfo.hpp>

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
    (void)domain_participant;
    return nullptr;
#endif // FASTDDS_STATISTICS
}

ReturnCode_t DomainParticipant::enable()
{
    ReturnCode_t ret = eprosima::fastdds::dds::DomainParticipant::enable();

    if (enable_)
    {
        create_statistics_builtin_entities();
    }

    return ret;
}

void DomainParticipant::create_statistics_builtin_entities()
{
    // Builtin publisher
    builtin_publisher_ = impl_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);

    // Enable statistics datawriters
    // 1. Find fastdds_statistics PropertyPolicyQos
    const std::string* property_topic_list = eprosima::fastrtps::rtps::PropertyPolicyHelper::find_property(
        impl_->get_qos().properties(), "fastdds.statistics");

    if (nullptr != property_topic_list)
    {
        enable_statistics_builtin_datawriters(*property_topic_list);
    }

    // 2. FASTDDS_STATISTICS environment variable
    std::string env_topic_list;
    const char* data;
    if (ReturnCode_t::RETCODE_OK == SystemInfo::get_env(FASTDDS_STATISTICS_ENVIRONMENT_VARIABLE, &data))
    {
        env_topic_list = data;
    }

    if (!env_topic_list.empty())
    {
        enable_statistics_builtin_datawriters(env_topic_list);
    }
}

void DomainParticipant::enable_statistics_builtin_datawriters(
        const std::string& topic_list)
{
    // Parse list and call enable_statistics_datawriter
    std::stringstream topics(topic_list);
    std::string topic;
    while (std::getline(topics, topic, ';'))
    {
        ReturnCode_t ret = enable_statistics_datawriter(topic, STATISTICS_DATAWRITER_QOS);
        // case RETCODE_ERROR is checked and logged in enable_statistics_datawriter.
        // case RETCODE_INCONSISTENT_POLICY cannot happen. STATISTICS_DATAWRITER_QOS is consitent.
        // case RETCODE_UNSUPPORTED cannot happen because this method is only called if FASTDDS_STATISTICS
        // CMake option is enabled
        assert(ret != RETCODE_INCONSISTENT_POLICY);
        assert(ret != RETCODE_UNSUPPORTED);
        if (ret == ReturnCode_t::RETCODE_BAD_PARAMETER)
        {
            logError(STATISTICS_DOMAIN_PARTICIPANT, "Topic " << topic << "is not a valid statistics topic name/alias");
        }
    }
}

void DomainParticipant::delete_statistics_builtin_entities()
{
    std::vector<eprosima::fastdds::dds::DataWriter*> builtin_writers;
    builtin_publisher_->get_datawriters(builtin_writers);
    for (auto writer : builtin_writers)
    {
        std::string topic_name = writer->get_topic()->get_name();
        disable_statistics_datawriter(topic_name);
    }

    // Delete builtin_publisher
    impl_->delete_publisher(builtin_publisher_);
}

} // dds
} // statistics
} // fastdds
} // eprosima
