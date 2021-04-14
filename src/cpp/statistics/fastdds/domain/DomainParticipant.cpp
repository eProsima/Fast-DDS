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
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.h>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/statistics/topic_names.hpp>
#include <fastrtps/types/TypesBase.h>

#include <fastdds/domain/DomainParticipantImpl.hpp>
#include <fastdds/publisher/DataWriterImpl.hpp>
#include <statistics/types/typesPubSubTypes.h>
#include <utils/SystemInfo.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

constexpr const char* HISTORY_LATENCY_TOPIC_ALIAS = "HISTORY_LATENCY_TOPIC";
constexpr const char* NETWORK_LATENCY_TOPIC_ALIAS = "NETWORK_LATENCY_TOPIC";
constexpr const char* PUBLICATION_THROUGHPUT_TOPIC_ALIAS = "PUBLICATION_THROUGHPUT_TOPIC";
constexpr const char* SUBSCRIPTION_THROUGHPUT_TOPIC_ALIAS = "SUBSCRIPTION_THROUGHPUT_TOPIC";
constexpr const char* RTPS_SENT_TOPIC_ALIAS = "RTPS_SENT_TOPIC";
constexpr const char* RTPS_LOST_TOPIC_ALIAS = "RTPS_LOST_TOPIC";
constexpr const char* RESENT_DATAS_TOPIC_ALIAS = "RESENT_DATAS_TOPIC";
constexpr const char* HEARTBEAT_COUNT_TOPIC_ALIAS = "HEARTBEAT_COUNT_TOPIC";
constexpr const char* ACKNACK_COUNT_TOPIC_ALIAS = "ACKNACK_COUNT_TOPIC";
constexpr const char* NACKFRAG_COUNT_TOPIC_ALIAS = "NACKFRAG_COUNT_TOPIC";
constexpr const char* GAP_COUNT_TOPIC_ALIAS = "GAP_COUNT_TOPIC";
constexpr const char* DATA_COUNT_TOPIC_ALIAS = "DATA_COUNT_TOPIC";
constexpr const char* PDP_PACKETS_TOPIC_ALIAS = "PDP_PACKETS_TOPIC";
constexpr const char* EDP_PACKETS_TOPIC_ALIAS = "EDP_PACKETS_TOPIC";
constexpr const char* DISCOVERY_TOPIC_ALIAS = "DISCOVERY_TOPIC";
constexpr const char* SAMPLE_DATAS_TOPIC_ALIAS = "SAMPLE_DATAS_TOPIC";
constexpr const char* PHYSICAL_DATA_TOPIC_ALIAS = "PHYSICAL_DATA_TOPIC";

ReturnCode_t DomainParticipant::enable_statistics_datawriter(
        const std::string& topic_name,
        const eprosima::fastdds::dds::DataWriterQos& dwqos)
{
#ifndef FASTDDS_STATISTICS
    (void) topic_name;
    (void) dwqos;

    return ReturnCode_t::RETCODE_UNSUPPORTED;
#else
    const std::string use_topic_name = transform_topic_name_alias(topic_name);
    if(!check_statistics_topic_name(use_topic_name))
    {
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    if (!eprosima::fastdds::dds::DataWriterImpl::check_qos(dwqos))
    {
        return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
    }

    // Register type and topic
    eprosima::fastdds::dds::Topic* topic = nullptr;
    if (register_statistics_type_and_topic(&topic, use_topic_name))
    {
        // Check if the statistics DataWriter already exists and create statistics DataWriter if it does not.
        if (nullptr == builtin_publisher_->lookup_datawriter(use_topic_name))
        {
            if (nullptr == builtin_publisher_->create_datawriter(topic, dwqos))
            {
                logError(STATISTICS_DOMAIN_PARTICIPANT, topic_name << " DataWriter creation has failed");
                return ReturnCode_t::RETCODE_ERROR;
            }
        }
    }
    return ReturnCode_t::RETCODE_OK;
#endif // FASTDDS_STATISTICS
}

ReturnCode_t DomainParticipant::disable_statistics_datawriter(
        const std::string& topic_name)
{
#ifndef FASTDDS_STATISTICS
    (void) topic_name;

    return ReturnCode_t::RETCODE_UNSUPPORTED;
#else
    ReturnCode_t ret = ReturnCode_t::RETCODE_OK;
    const std::string use_topic_name = transform_topic_name_alias(topic_name);
    if(!check_statistics_topic_name(use_topic_name))
    {
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    // Delete statistics DataWriter
    // delete_datawriter does not check that the provided argument is not nullptr (safety check)
    eprosima::fastdds::dds::DataWriter* writer = builtin_publisher_->lookup_datawriter(use_topic_name);
    if (nullptr != writer)
    {
        if (ReturnCode_t::RETCODE_OK != builtin_publisher_->delete_datawriter(writer))
        {
            ret = ReturnCode_t::RETCODE_ERROR;
        }
        // Deregister type and delete topic
        if(!deregister_statistics_type_and_topic(use_topic_name))
        {
            ret = ReturnCode_t::RETCODE_ERROR;
        }
    }
    return ret;
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
        assert(ret != ReturnCode_t::RETCODE_INCONSISTENT_POLICY);
        assert(ret != ReturnCode_t::RETCODE_UNSUPPORTED);
        if (ret == ReturnCode_t::RETCODE_BAD_PARAMETER)
        {
            logError(STATISTICS_DOMAIN_PARTICIPANT, "Topic " << topic << " is not a valid statistics topic name/alias");
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

const std::string DomainParticipant::transform_topic_name_alias(
        const std::string& topic)
{
    std::string topic_name;
    if (HISTORY_LATENCY_TOPIC_ALIAS == topic)
    {
        topic_name = HISTORY_LATENCY_TOPIC;
    }
    else if (NETWORK_LATENCY_TOPIC_ALIAS == topic)
    {
        topic_name = NETWORK_LATENCY_TOPIC;
    }
    else if (PUBLICATION_THROUGHPUT_TOPIC_ALIAS == topic)
    {
        topic_name = PUBLICATION_THROUGHPUT_TOPIC;
    }
    else if (SUBSCRIPTION_THROUGHPUT_TOPIC_ALIAS == topic)
    {
        topic_name = SUBSCRIPTION_THROUGHPUT_TOPIC;
    }
    else if (RTPS_SENT_TOPIC_ALIAS == topic)
    {
        topic_name = RTPS_SENT_TOPIC;
    }
    else if (RTPS_LOST_TOPIC_ALIAS == topic)
    {
        topic_name = RTPS_LOST_TOPIC;
    }
    else if (RESENT_DATAS_TOPIC_ALIAS == topic)
    {
        topic_name = RESENT_DATAS_TOPIC;
    }
    else if (HEARTBEAT_COUNT_TOPIC_ALIAS == topic)
    {
        topic_name = HEARTBEAT_COUNT_TOPIC;
    }
    else if (ACKNACK_COUNT_TOPIC_ALIAS == topic)
    {
        topic_name = ACKNACK_COUNT_TOPIC;
    }
    else if (NACKFRAG_COUNT_TOPIC_ALIAS == topic)
    {
        topic_name = NACKFRAG_COUNT_TOPIC;
    }
    else if (GAP_COUNT_TOPIC_ALIAS == topic)
    {
        topic_name = GAP_COUNT_TOPIC;
    }
    else if (DATA_COUNT_TOPIC_ALIAS == topic)
    {
        topic_name = DATA_COUNT_TOPIC;
    }
    else if (PDP_PACKETS_TOPIC_ALIAS == topic)
    {
        topic_name = PDP_PACKETS_TOPIC;
    }
    else if (EDP_PACKETS_TOPIC_ALIAS == topic)
    {
        topic_name = EDP_PACKETS_TOPIC;
    }
    else if (DISCOVERY_TOPIC_ALIAS == topic)
    {
        topic_name = DISCOVERY_TOPIC;
    }
    else if (SAMPLE_DATAS_TOPIC_ALIAS == topic)
    {
        topic_name = SAMPLE_DATAS_TOPIC;
    }
    else if (PHYSICAL_DATA_TOPIC_ALIAS == topic)
    {
        topic_name = PHYSICAL_DATA_TOPIC;
    }
    else
    {
        topic_name = topic;
    }
    return topic_name;
}

bool DomainParticipant::check_statistics_topic_name(
        const std::string& topic)
{
    if (HISTORY_LATENCY_TOPIC != topic && NETWORK_LATENCY_TOPIC != topic && PUBLICATION_THROUGHPUT_TOPIC != topic &&
            SUBSCRIPTION_THROUGHPUT_TOPIC != topic && RTPS_SENT_TOPIC != topic && RTPS_LOST_TOPIC != topic &&
            RESENT_DATAS_TOPIC != topic && HEARTBEAT_COUNT_TOPIC != topic && ACKNACK_COUNT_TOPIC != topic &&
            NACKFRAG_COUNT_TOPIC != topic && GAP_COUNT_TOPIC != topic && DATA_COUNT_TOPIC != topic &&
            PDP_PACKETS_TOPIC != topic && EDP_PACKETS_TOPIC != topic && DISCOVERY_TOPIC != topic &&
            SAMPLE_DATAS_TOPIC != topic && PHYSICAL_DATA_TOPIC != topic)
    {
        return false;
    }
    return true;
}

bool DomainParticipant::register_statistics_type_and_topic(
        eprosima::fastdds::dds::Topic** topic,
        const std::string& topic_name)
{
    bool return_code = false;
    if (HISTORY_LATENCY_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport history_latency_type(new WriterReaderDataPubSubType);
        return_code = check_register_type(history_latency_type);
        return_code = return_code && find_or_create_topic(topic, topic_name, history_latency_type->getName());
    }
    else if (NETWORK_LATENCY_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport network_latency_type(new Locator2LocatorDataPubSubType);
        return_code = check_register_type(network_latency_type);
        return_code = return_code && find_or_create_topic(topic, topic_name, network_latency_type->getName());
    }
    else if (PUBLICATION_THROUGHPUT_TOPIC == topic_name || SUBSCRIPTION_THROUGHPUT_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport throughput_type(new EntityDataPubSubType);
        return_code = check_register_type(throughput_type);
        return_code = return_code && find_or_create_topic(topic, topic_name, throughput_type->getName());
    }
    else if (RTPS_SENT_TOPIC == topic_name || RTPS_LOST_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport rtps_traffic_type(new Entity2LocatorTrafficPubSubType);
        return_code = check_register_type(rtps_traffic_type);
        return_code = return_code && find_or_create_topic(topic, topic_name, rtps_traffic_type->getName());
    }
    else if (RESENT_DATAS_TOPIC == topic_name || HEARTBEAT_COUNT_TOPIC == topic_name || 
            ACKNACK_COUNT_TOPIC == topic_name || NACKFRAG_COUNT_TOPIC == topic_name || GAP_COUNT_TOPIC == topic_name ||
            DATA_COUNT_TOPIC == topic_name || PDP_PACKETS_TOPIC == topic_name || EDP_PACKETS_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport count_type(new EntityCountPubSubType);
        return_code = check_register_type(count_type);
        return_code = return_code && find_or_create_topic(topic, topic_name, count_type->getName());
    }
    else if (DISCOVERY_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport discovery_type(new DiscoveryTimePubSubType);
        return_code = check_register_type(discovery_type);
        return_code = return_code && find_or_create_topic(topic, topic_name, discovery_type->getName());
    }
    else if (SAMPLE_DATAS_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport sample_identity_count_type(new SampleIdentityCountPubSubType);
        return_code = check_register_type(sample_identity_count_type);
        return_code = return_code && find_or_create_topic(topic, topic_name, sample_identity_count_type->getName());
    }
    else if (PHYSICAL_DATA_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport physical_data_type(new PhysicalDataPubSubType);
        return_code = check_register_type(physical_data_type);
        return_code = return_code && find_or_create_topic(topic, topic_name, physical_data_type->getName());
    }
    return return_code;
}

bool DomainParticipant::find_or_create_topic(
        eprosima::fastdds::dds::Topic** topic,
        const std::string& topic_name,
        const std::string& type_name)
{
        // Find if the topic has been already created and if the associated type is correct
        eprosima::fastdds::dds::TopicDescription* topic_desc = lookup_topicdescription(topic_name);
        if (nullptr != topic_desc)
        {
            if(check_statistics_topic_and_type(topic_desc, topic_name, type_name))
            {
                *topic = dynamic_cast<eprosima::fastdds::dds::Topic*>(topic_desc);
            }
            else
            {
                return false;
            }
        }
        else
        {
            // Create topic. No need to check return pointer. It fails if the topic already exists, if the QoS is
            // inconsistent or if the type is not registered.
            *topic = create_topic(topic_name, type_name, eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
        }
        assert(nullptr != *topic);
        return true;
}

bool DomainParticipant::deregister_statistics_type_and_topic(
        const std::string& topic_name)
{
    bool return_code = false;
    if (HISTORY_LATENCY_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport history_latency_type(new WriterReaderDataPubSubType);
        return_code = delete_topic_and_type(topic_name, history_latency_type->getName());
    }
    else if (NETWORK_LATENCY_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport network_latency_type(new Locator2LocatorDataPubSubType);
        return_code = delete_topic_and_type(topic_name, network_latency_type->getName());
    }
    else if (PUBLICATION_THROUGHPUT_TOPIC == topic_name || SUBSCRIPTION_THROUGHPUT_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport throughput_type(new EntityDataPubSubType);
        return_code =  delete_topic_and_type(topic_name, throughput_type->getName());
    }
    else if (RTPS_SENT_TOPIC == topic_name || RTPS_LOST_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport rtps_traffic_type(new Entity2LocatorTrafficPubSubType);
        return_code =  delete_topic_and_type(topic_name, rtps_traffic_type->getName());
    }
    else if (RESENT_DATAS_TOPIC == topic_name || HEARTBEAT_COUNT_TOPIC == topic_name || 
            ACKNACK_COUNT_TOPIC == topic_name || NACKFRAG_COUNT_TOPIC == topic_name || GAP_COUNT_TOPIC == topic_name ||
            DATA_COUNT_TOPIC == topic_name || PDP_PACKETS_TOPIC == topic_name || EDP_PACKETS_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport count_type(new EntityCountPubSubType);
        return_code =  delete_topic_and_type(topic_name, count_type->getName());
    }
    else if (DISCOVERY_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport discovery_type(new DiscoveryTimePubSubType);
        return_code =  delete_topic_and_type(topic_name, discovery_type->getName());
    }
    else if (SAMPLE_DATAS_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport sample_identity_count_type(new SampleIdentityCountPubSubType);
        return_code =  delete_topic_and_type(topic_name, sample_identity_count_type->getName());
    }
    else if (PHYSICAL_DATA_TOPIC == topic_name)
    {
        eprosima::fastdds::dds::TypeSupport physical_data_type(new PhysicalDataPubSubType);
        return_code =  delete_topic_and_type(topic_name, physical_data_type->getName());
    }
    return return_code;
}

bool DomainParticipant::delete_topic_and_type(
        const std::string& topic_name,
        const std::string& type_name)
{
    eprosima::fastdds::dds::TopicDescription* topic_desc = lookup_topicdescription(topic_name);
    if (nullptr != topic_desc)
    {
        if (check_statistics_topic_and_type(topic_desc, topic_name, type_name))
        {
            eprosima::fastdds::dds::Topic* topic = dynamic_cast<eprosima::fastdds::dds::Topic*>(topic_desc);
            // unregister_type failures are of no concern here. It will fail if the type is still in use (something
            // expected) and if the type_name is empty (which is not going to happen).
            unregister_type(type_name);
            // delete_topic can fail if the topic is referenced by any other entity. This case could happen even if
            // it should not. It also fails if topic is a nullptr (dynamic_cast failure).
            if (ReturnCode_t::RETCODE_OK != delete_topic(topic))
            {
                return false;
            }
            return true;
        }
    }
    return false;
}

bool DomainParticipant::check_register_type(
        eprosima::fastdds::dds::TypeSupport type)
{
    if (ReturnCode_t::RETCODE_PRECONDITION_NOT_MET == register_type(type))
    {
        // No log because it is already logged within register_type
        return false;
    }
    return true;
}

bool DomainParticipant::check_statistics_topic_and_type(
        const eprosima::fastdds::dds::TopicDescription* topic_desc,
        const std::string& topic_name,
        const std::string& type_name)
{
    if (topic_desc->get_type_name() != type_name)
    {
        logError(STATISTICS_DOMAIN_PARTICIPANT, topic_name << "is not using expected type " << type_name <<
            " and is using instead type " << topic_desc->get_type_name());
        return false;
    }
    return true;
}

} // dds
} // statistics
} // fastdds
} // eprosima
