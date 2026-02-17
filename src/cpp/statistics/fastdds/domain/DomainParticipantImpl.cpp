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
 * @file DomainParticipantImpl.cpp
 */

#include <statistics/fastdds/domain/DomainParticipantImpl.hpp>

#include <sstream>
#include <string>
#include <vector>

#include <asio.hpp>

#include <fastdds/core/policy/QosPolicyUtils.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/statistics/topic_names.hpp>

#include <fastdds/publisher/DataWriterImpl.hpp>
#include <fastdds/utils/QosConverters.hpp>
#include <statistics/fastdds/publisher/PublisherImpl.hpp>
#include <statistics/fastdds/subscriber/SubscriberImpl.hpp>
#include <statistics/rtps/GuidUtils.hpp>
#include <statistics/types/types.hpp>
#include <statistics/types/typesPubSubTypes.hpp>
#include <utils/SystemInfo.hpp>
#include <xmlparser/attributes/PublisherAttributes.hpp>
#include <xmlparser/XMLParserCommon.h>
#include <xmlparser/XMLProfileManager.h>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

using xmlparser::XMLProfileManager;
using xmlparser::XMLP_ret;
using xmlparser::DEFAULT_STATISTICS_DATAWRITER_PROFILE;

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
constexpr const char* MONITOR_SERVICE_TOPIC_ALIAS = "MONITOR_SERVICE_TOPIC";

static constexpr uint32_t participant_statistics_mask =
        EventKind::RTPS_SENT | EventKind::RTPS_LOST | EventKind::NETWORK_LATENCY |
        EventKind::EDP_PACKETS | EventKind::PDP_PACKETS |
        EventKind::PHYSICAL_DATA | EventKind::DISCOVERED_ENTITY;

struct ValidEntry
{
    const char* alias;
    const char* name;
    uint32_t event_kind;
};

static const ValidEntry valid_entries[] =
{
    {HISTORY_LATENCY_TOPIC_ALIAS,         HISTORY_LATENCY_TOPIC,         EventKind::HISTORY2HISTORY_LATENCY},
    {NETWORK_LATENCY_TOPIC_ALIAS,         NETWORK_LATENCY_TOPIC,         EventKind::NETWORK_LATENCY},
    {PUBLICATION_THROUGHPUT_TOPIC_ALIAS,  PUBLICATION_THROUGHPUT_TOPIC,  EventKind::PUBLICATION_THROUGHPUT},
    {SUBSCRIPTION_THROUGHPUT_TOPIC_ALIAS, SUBSCRIPTION_THROUGHPUT_TOPIC, EventKind::SUBSCRIPTION_THROUGHPUT},
    {RTPS_SENT_TOPIC_ALIAS,               RTPS_SENT_TOPIC,               EventKind::RTPS_SENT},
    {RTPS_LOST_TOPIC_ALIAS,               RTPS_LOST_TOPIC,               EventKind::RTPS_LOST},
    {RESENT_DATAS_TOPIC_ALIAS,            RESENT_DATAS_TOPIC,            EventKind::RESENT_DATAS},
    {HEARTBEAT_COUNT_TOPIC_ALIAS,         HEARTBEAT_COUNT_TOPIC,         EventKind::HEARTBEAT_COUNT},
    {ACKNACK_COUNT_TOPIC_ALIAS,           ACKNACK_COUNT_TOPIC,           EventKind::ACKNACK_COUNT},
    {NACKFRAG_COUNT_TOPIC_ALIAS,          NACKFRAG_COUNT_TOPIC,          EventKind::NACKFRAG_COUNT},
    {GAP_COUNT_TOPIC_ALIAS,               GAP_COUNT_TOPIC,               EventKind::GAP_COUNT},
    {DATA_COUNT_TOPIC_ALIAS,              DATA_COUNT_TOPIC,              EventKind::DATA_COUNT},
    {PDP_PACKETS_TOPIC_ALIAS,             PDP_PACKETS_TOPIC,             EventKind::PDP_PACKETS},
    {EDP_PACKETS_TOPIC_ALIAS,             EDP_PACKETS_TOPIC,             EventKind::EDP_PACKETS},
    {DISCOVERY_TOPIC_ALIAS,               DISCOVERY_TOPIC,               EventKind::DISCOVERED_ENTITY},
    {SAMPLE_DATAS_TOPIC_ALIAS,            SAMPLE_DATAS_TOPIC,            EventKind::SAMPLE_DATAS},
    {PHYSICAL_DATA_TOPIC_ALIAS,           PHYSICAL_DATA_TOPIC,           EventKind::PHYSICAL_DATA}
};

ReturnCode_t DomainParticipantImpl::enable_statistics_datawriter(
        const std::string& topic_name,
        const efd::DataWriterQos& dwqos)
{
    std::string use_topic_name;
    uint32_t event_kind;
    if (!transform_and_check_topic_name(topic_name, use_topic_name, event_kind))
    {
        return efd::RETCODE_BAD_PARAMETER;
    }

    if (efd::RETCODE_OK != efd::DataWriterImpl::check_qos(dwqos))
    {
        return efd::RETCODE_INCONSISTENT_POLICY;
    }

    // Register type and topic
    efd::Topic* topic = nullptr;
    if (register_statistics_type_and_topic(&topic, use_topic_name))
    {
        // Check if the statistics DataWriter already exists and create statistics DataWriter if it does not.
        if (nullptr == builtin_publisher_->lookup_datawriter(use_topic_name))
        {
            fastdds::rtps::EntityId_t entity_id;
            set_statistics_entity_id(event_kind, entity_id);
            efd::TypeSupport type = participant_->find_type(topic->get_type_name());
            auto writer_impl = builtin_publisher_impl_->create_datawriter_impl(type, topic, dwqos, entity_id);
            auto data_writer = builtin_publisher_impl_->create_datawriter(topic, writer_impl, efd::StatusMask::all());
            if (nullptr == data_writer)
            {
                // Remove already created Impl
                delete writer_impl;
                // Remove topic and type
                delete_topic_and_type(use_topic_name);
                EPROSIMA_LOG_ERROR(STATISTICS_DOMAIN_PARTICIPANT, topic_name << " DataWriter creation has failed");
                return efd::RETCODE_ERROR;
            }

            if (PHYSICAL_DATA_TOPIC == use_topic_name)
            {
                PhysicalData notification;
                notification.participant_guid(*reinterpret_cast<const detail::GUID_s*>(&guid()));
                if (SystemInfo::instance().machine_id().size() > 0)
                {
                    notification.host(SystemInfo::instance().machine_id().to_string());
                }
                else
                {
                    notification.host(asio::ip::host_name() + ":" + std::to_string(efd::utils::default_domain_id()));
                }
                std::string username;
                if (efd::RETCODE_OK == SystemInfo::get_username(username))
                {
                    notification.user(username);
                }
                notification.process(std::to_string(SystemInfo::instance().process_id()));

                const void* data_sample = nullptr;
                data_sample = &notification;

                data_writer->write(const_cast<void*>(data_sample));
            }
            else
            {
                statistics_listener_->set_datawriter(event_kind, data_writer);
                rtps_participant_->set_enabled_statistics_writers_mask(statistics_listener_->enabled_writers_mask());
            }
        }
        return efd::RETCODE_OK;
    }
    return efd::RETCODE_ERROR;
}

ReturnCode_t DomainParticipantImpl::enable_statistics_datawriter_with_profile(
        const std::string& profile_name,
        const std::string& topic_name)
{
    DataWriterQos datawriter_qos;
    xmlparser::PublisherAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillPublisherAttributes(profile_name, attr, false))
    {
        efd::utils::set_qos_from_attributes(datawriter_qos, attr);

        ReturnCode_t ret = enable_statistics_datawriter(topic_name, datawriter_qos);
        // case efd::RETCODE_ERROR is checked and logged in enable_statistics_datawriter.
        // case efd::RETCODE_INCONSISTENT_POLICY could happen if profile defined in XML is inconsistent.
        // case efd::RETCODE_UNSUPPORTED cannot happen because this method is only called if FASTDDS_STATISTICS
        // CMake option is enabled
        if (ret == efd::RETCODE_INCONSISTENT_POLICY)
        {
            EPROSIMA_LOG_ERROR(STATISTICS_DOMAIN_PARTICIPANT,
                    "Statistics DataWriter QoS from profile name " << profile_name << " are not consistent/compatible");
        }
        assert(ret != efd::RETCODE_UNSUPPORTED);
        if (ret == efd::RETCODE_BAD_PARAMETER)
        {
            EPROSIMA_LOG_ERROR(STATISTICS_DOMAIN_PARTICIPANT,
                    "Profile name " << profile_name << " is not a valid statistics topic name/alias");
        }
        return ret;
    }
    EPROSIMA_LOG_ERROR(STATISTICS_DOMAIN_PARTICIPANT,
            "Profile name " << profile_name << " has not been found");
    return efd::RETCODE_ERROR;
}

ReturnCode_t DomainParticipantImpl::disable_statistics_datawriter(
        const std::string& topic_name)
{
    ReturnCode_t ret = efd::RETCODE_OK;
    std::string use_topic_name;
    uint32_t event_kind;
    if (!transform_and_check_topic_name(topic_name, use_topic_name, event_kind))
    {
        return efd::RETCODE_BAD_PARAMETER;
    }

    // Delete statistics DataWriter
    // delete_datawriter does not check that the provided argument is not nullptr (safety check)
    efd::DataWriter* writer = builtin_publisher_->lookup_datawriter(use_topic_name);
    if (nullptr != writer)
    {
        // Avoid calling DataWriter from listener callback
        statistics_listener_->set_datawriter(event_kind, nullptr);
        rtps_participant_->set_enabled_statistics_writers_mask(statistics_listener_->enabled_writers_mask());

        // Delete the DataWriter
        if (efd::RETCODE_OK != builtin_publisher_->delete_datawriter(writer))
        {
            // Restore writer on listener before returning the error
            statistics_listener_->set_datawriter(event_kind, writer);
            rtps_participant_->set_enabled_statistics_writers_mask(statistics_listener_->enabled_writers_mask());
            ret = efd::RETCODE_ERROR;
        }

        // Deregister type and delete topic
        if (!delete_topic_and_type(use_topic_name))
        {
            ret = efd::RETCODE_ERROR;
        }
    }
    return ret;
}

ReturnCode_t DomainParticipantImpl::enable()
{
    ReturnCode_t ret = efd::DomainParticipantImpl::enable();

    if (efd::RETCODE_OK == ret)
    {
        rtps_participant_->add_statistics_listener(statistics_listener_, participant_statistics_mask);
        create_statistics_builtin_entities();

        if (!rtps_participant_->is_monitor_service_created())
        {
            auto enable_ms_property_value = fastdds::rtps::PropertyPolicyHelper::find_property(
                qos_.properties(), fastdds::dds::parameter_enable_monitor_service);

            if (nullptr != enable_ms_property_value && *enable_ms_property_value == "true")
            {
                if (enable_monitor_service() != efd::RETCODE_OK)
                {
                    EPROSIMA_LOG_ERROR(STATISTICS_DOMAIN_PARTICIPANT, "Could not enable the Monitor Service");
                }
            }
        }
    }

    return ret;
}

void DomainParticipantImpl::disable()
{
    if (nullptr != rtps_participant_)
    {
        rtps_participant_->remove_statistics_listener(statistics_listener_, participant_statistics_mask);
    }
    efd::DomainParticipantImpl::disable();
}

ReturnCode_t DomainParticipantImpl::delete_contained_entities()
{
    delete_statistics_builtin_entities();
    return efd::DomainParticipantImpl::delete_contained_entities();
}

ReturnCode_t DomainParticipantImpl::enable_monitor_service()
{
    ReturnCode_t ret = efd::RETCODE_OK;

    if (!rtps_participant_->is_monitor_service_created())
    {
        status_observer_.store(rtps_participant_->create_monitor_service(*this));
    }

    if (!rtps_participant_->enable_monitor_service() ||
            nullptr == status_observer_)
    {
        ret = efd::RETCODE_ERROR;
    }

    return ret;
}

ReturnCode_t DomainParticipantImpl::disable_monitor_service()
{
    ReturnCode_t ret = efd::RETCODE_OK;

    if (!rtps_participant_->is_monitor_service_created() ||
            !rtps_participant_->disable_monitor_service())
    {
        ret = efd::RETCODE_NOT_ENABLED;
    }

    return ret;
}

ReturnCode_t DomainParticipantImpl::fill_discovery_data_from_cdr_message(
        fastdds::rtps::ParticipantBuiltinTopicData& data,
        const fastdds::statistics::MonitorServiceStatusData& msg)
{
    ReturnCode_t ret{efd::RETCODE_OK};

    if (!get_rtps_participant()->fill_discovery_data_from_cdr_message(data, msg))
    {
        ret = efd::RETCODE_ERROR;
    }

    return ret;
}

ReturnCode_t DomainParticipantImpl::fill_discovery_data_from_cdr_message(
        fastdds::dds::PublicationBuiltinTopicData& data,
        const fastdds::statistics::MonitorServiceStatusData& msg)
{
    ReturnCode_t ret{efd::RETCODE_OK};

    if (!get_rtps_participant()->fill_discovery_data_from_cdr_message(data, msg))
    {
        ret = efd::RETCODE_ERROR;
    }

    return ret;
}

ReturnCode_t DomainParticipantImpl::fill_discovery_data_from_cdr_message(
        fastdds::dds::SubscriptionBuiltinTopicData& data,
        const fastdds::statistics::MonitorServiceStatusData& msg)
{
    ReturnCode_t ret{efd::RETCODE_OK};

    if (!get_rtps_participant()->fill_discovery_data_from_cdr_message(data, msg))
    {
        ret = efd::RETCODE_ERROR;
    }

    return ret;
}

efd::PublisherImpl* DomainParticipantImpl::create_publisher_impl(
        const efd::PublisherQos& qos,
        efd::PublisherListener* listener)
{
    return new PublisherImpl(this, qos, listener, statistics_listener_);
}

efd::SubscriberImpl* DomainParticipantImpl::create_subscriber_impl(
        const efd::SubscriberQos& qos,
        efd::SubscriberListener* listener)
{
    return new SubscriberImpl(this, qos, listener, statistics_listener_);
}

void DomainParticipantImpl::create_statistics_builtin_entities()
{
    efd::PublisherImpl* builtin_publisher_impl = nullptr;

    // Builtin publisher
    builtin_publisher_ = create_publisher(efd::PUBLISHER_QOS_DEFAULT, &builtin_publisher_impl);

    builtin_publisher_impl_ = dynamic_cast<PublisherImpl*>(builtin_publisher_impl);
    assert(nullptr != builtin_publisher_impl_);

    // Enable statistics datawriters
    // 1. Find fastdds_statistics PropertyPolicyQos
    const std::string* property_topic_list = eprosima::fastdds::rtps::PropertyPolicyHelper::find_property(
        get_qos().properties(), "fastdds.statistics");

    if (nullptr != property_topic_list)
    {
        enable_statistics_builtin_datawriters(*property_topic_list);
    }

    // 2. FASTDDS_STATISTICS environment variable
    std::string env_topic_list;
    SystemInfo::get_env(FASTDDS_STATISTICS_ENVIRONMENT_VARIABLE, env_topic_list);

    if (!env_topic_list.empty())
    {
        enable_statistics_builtin_datawriters(env_topic_list);
    }
}

void DomainParticipantImpl::enable_statistics_builtin_datawriters(
        const std::string& topic_list)
{
    // Parse list and call enable_statistics_datawriter
    std::stringstream topics(topic_list);
    std::string topic;
    while (std::getline(topics, topic, ';'))
    {
        if (MONITOR_SERVICE_TOPIC_ALIAS == topic)
        {
            if (!rtps_participant_->is_monitor_service_created() &&
                    enable_monitor_service() != efd::RETCODE_OK)
            {
                EPROSIMA_LOG_ERROR(STATISTICS_DOMAIN_PARTICIPANT, "Could not enable the Monitor Service");
            }
            continue;
        }

        DataWriterQos datawriter_qos;
        xmlparser::PublisherAttributes attr;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillPublisherAttributes(topic, attr, false))
        {
            efd::utils::set_qos_from_attributes(datawriter_qos, attr);
        }
        else if (XMLP_ret::XML_OK ==
                XMLProfileManager::fillPublisherAttributes(DEFAULT_STATISTICS_DATAWRITER_PROFILE, attr, false))
        {
            efd::utils::set_qos_from_attributes(datawriter_qos, attr);
        }

        ReturnCode_t ret = enable_statistics_datawriter(topic, datawriter_qos);
        // case efd::RETCODE_ERROR is checked and logged in enable_statistics_datawriter.
        // case efd::RETCODE_INCONSISTENT_POLICY could happen if profile defined in XML is inconsistent.
        // case efd::RETCODE_UNSUPPORTED cannot happen because this method is only called if FASTDDS_STATISTICS
        // CMake option is enabled
        if (ret == efd::RETCODE_INCONSISTENT_POLICY)
        {
            EPROSIMA_LOG_ERROR(STATISTICS_DOMAIN_PARTICIPANT,
                    "Statistics DataWriter QoS from topic " << topic << " are not consistent/compatible");
        }
        assert(ret != efd::RETCODE_UNSUPPORTED);
        if (ret == efd::RETCODE_BAD_PARAMETER)
        {
            EPROSIMA_LOG_ERROR(STATISTICS_DOMAIN_PARTICIPANT,
                    "Topic " << topic << " is not a valid statistics topic name/alias");
        }
    }
}

void DomainParticipantImpl::delete_statistics_builtin_entities()
{
    if (nullptr != builtin_publisher_)
    {
        std::vector<efd::DataWriter*> builtin_writers;
        builtin_publisher_->get_datawriters(builtin_writers);
        for (auto writer : builtin_writers)
        {
            std::string topic_name = writer->get_topic()->get_name();
            disable_statistics_datawriter(topic_name);
        }

        // Delete builtin_publisher
        delete_publisher(builtin_publisher_);
        builtin_publisher_ = nullptr;
        builtin_publisher_impl_ = nullptr;
    }
}

bool DomainParticipantImpl::is_statistics_topic_name(
        const std::string& topic_name) noexcept
{
    for (const ValidEntry& entry : valid_entries)
    {
        if (entry.name == topic_name)
        {
            return true;
        }
    }

    return false;
}

bool DomainParticipantImpl::transform_and_check_topic_name(
        const std::string& topic_name_or_alias,
        std::string& topic_name,
        uint32_t& event_kind) noexcept
{
    for (const ValidEntry& entry : valid_entries)
    {
        if ((entry.alias == topic_name_or_alias) || (entry.name == topic_name_or_alias))
        {
            topic_name = entry.name;
            event_kind = entry.event_kind;
            return true;
        }
    }

    return false;
}

bool DomainParticipantImpl::register_statistics_type_and_topic(
        efd::Topic** topic,
        const std::string& topic_name) noexcept
{
    bool return_code = false;
    if (HISTORY_LATENCY_TOPIC == topic_name)
    {
        efd::TypeSupport history_latency_type(new WriterReaderDataPubSubType);
        history_latency_type->register_type_object_representation();
        return_code = find_or_create_topic_and_type(topic, topic_name, history_latency_type);
    }
    else if (NETWORK_LATENCY_TOPIC == topic_name)
    {
        efd::TypeSupport network_latency_type(new Locator2LocatorDataPubSubType);
        network_latency_type->register_type_object_representation();
        return_code = find_or_create_topic_and_type(topic, topic_name, network_latency_type);
    }
    else if (PUBLICATION_THROUGHPUT_TOPIC == topic_name || SUBSCRIPTION_THROUGHPUT_TOPIC == topic_name)
    {
        efd::TypeSupport throughput_type(new EntityDataPubSubType);
        throughput_type->register_type_object_representation();
        return_code = find_or_create_topic_and_type(topic, topic_name, throughput_type);
    }
    else if (RTPS_SENT_TOPIC == topic_name || RTPS_LOST_TOPIC == topic_name)
    {
        efd::TypeSupport rtps_traffic_type(new Entity2LocatorTrafficPubSubType);
        rtps_traffic_type->register_type_object_representation();
        return_code = find_or_create_topic_and_type(topic, topic_name, rtps_traffic_type);
    }
    else if (RESENT_DATAS_TOPIC == topic_name || HEARTBEAT_COUNT_TOPIC == topic_name ||
            ACKNACK_COUNT_TOPIC == topic_name || NACKFRAG_COUNT_TOPIC == topic_name || GAP_COUNT_TOPIC == topic_name ||
            DATA_COUNT_TOPIC == topic_name || PDP_PACKETS_TOPIC == topic_name || EDP_PACKETS_TOPIC == topic_name)
    {
        efd::TypeSupport count_type(new EntityCountPubSubType);
        count_type->register_type_object_representation();
        return_code = find_or_create_topic_and_type(topic, topic_name, count_type);
    }
    else if (DISCOVERY_TOPIC == topic_name)
    {
        efd::TypeSupport discovery_type(new DiscoveryTimePubSubType);
        discovery_type->register_type_object_representation();
        return_code = find_or_create_topic_and_type(topic, topic_name, discovery_type);
    }
    else if (SAMPLE_DATAS_TOPIC == topic_name)
    {
        efd::TypeSupport sample_identity_count_type(new SampleIdentityCountPubSubType);
        sample_identity_count_type->register_type_object_representation();
        return_code = find_or_create_topic_and_type(topic, topic_name, sample_identity_count_type);
    }
    else if (PHYSICAL_DATA_TOPIC == topic_name)
    {
        efd::TypeSupport physical_data_type(new PhysicalDataPubSubType);
        physical_data_type->register_type_object_representation();
        return_code = find_or_create_topic_and_type(topic, topic_name, physical_data_type);
    }
    return return_code;
}

bool DomainParticipantImpl::find_or_create_topic_and_type(
        efd::Topic** topic,
        const std::string& topic_name,
        const efd::TypeSupport& type) noexcept
{
    // Find if the topic has been already created and if the associated type is correct
    efd::TopicDescription* topic_desc = lookup_topicdescription(topic_name);
    if (nullptr != topic_desc)
    {
        if (topic_desc->get_type_name() != type->get_name())
        {
            EPROSIMA_LOG_ERROR(STATISTICS_DOMAIN_PARTICIPANT,
                    topic_name << " is not using expected type " << type->get_name() <<
                    " and is using instead type " << topic_desc->get_type_name());
            return false;
        }
        else
        {
            // TODO(jlbueno) This casting should be checked after other TopicDescription implementations are
            // included: ContentFilteredTopic, MultiTopic.
            *topic = dynamic_cast<efd::Topic*>(topic_desc);
        }
    }
    else
    {
        if (efd::RETCODE_PRECONDITION_NOT_MET == register_type(type, type->get_name()))
        {
            // No log because it is already logged within register_type
            return false;
        }
        // Create topic. No need to check return pointer. It fails if the topic already exists, if the QoS is
        // inconsistent or if the type is not registered.
        *topic = create_topic(topic_name, type->get_name(), efd::TOPIC_QOS_DEFAULT);
    }
    assert(nullptr != *topic);
    return true;
}

bool DomainParticipantImpl::delete_topic_and_type(
        const std::string& topic_name) noexcept
{
    efd::TopicDescription* topic_desc = lookup_topicdescription(topic_name);
    assert(nullptr != topic_desc);
    efd::Topic* topic = dynamic_cast<efd::Topic*>(topic_desc);
    std::string type_name = topic->get_type_name();
    // delete_topic can fail if the topic is referenced by any other entity. This case could happen even if
    // it should not. It also fails if topic is a nullptr (dynamic_cast failure).
    if (efd::RETCODE_OK != delete_topic(topic))
    {
        return false;
    }
    // unregister_type failures are of no concern here. It will fail if the type is still in use (something
    // expected) and if the type_name is empty (which is not going to happen).
    unregister_type(type_name);
    return true;
}

bool DomainParticipantImpl::get_monitoring_status(
        const fastdds::rtps::GUID_t& entity_guid,
        eprosima::fastdds::statistics::MonitorServiceData& status)
{
    ReturnCode_t ret = efd::RETCODE_ERROR;

    if (entity_guid.entityId.is_reader())
    {
        std::lock_guard<std::mutex> lock(mtx_subs_);
        for (auto& sub : subscribers_)
        {
            if (sub.second->get_monitoring_status(status, entity_guid))
            {
                ret = efd::RETCODE_OK;
                break;
            }
        }
    }
    else if (entity_guid.entityId.is_writer())
    {
        std::lock_guard<std::mutex> lock(mtx_pubs_);
        for (auto& pub : publishers_)
        {
            if (pub.second->get_monitoring_status(status, entity_guid))
            {
                ret = efd::RETCODE_OK;
                break;
            }
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(STATISTICS_DOMAIN_PARTICIPANT,
                "Unknown entity type to get the status from " << entity_guid.entityId);
    }

    return (ret == efd::RETCODE_OK);
}

} // dds
} // statistics
} // fastdds
} // eprosima
