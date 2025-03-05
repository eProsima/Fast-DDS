// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*
 * QosConverters.cpp
 *
 */

#include <string>

#include <fastdds/rtps/common/Property.hpp>
#include <fastdds/utils/QosConverters.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace utils {

using rtps::Property;
using std::string;

void set_qos_from_attributes(
        DataWriterQos& qos,
        const xmlparser::PublisherAttributes& attr)
{
    qos.writer_resource_limits().matched_subscriber_allocation = attr.matched_subscriber_allocation;
    qos.properties() = attr.properties;
    qos.endpoint().unicast_locator_list = attr.unicastLocatorList;
    qos.endpoint().multicast_locator_list = attr.multicastLocatorList;
    qos.endpoint().remote_locator_list = attr.remoteLocatorList;
    qos.endpoint().external_unicast_locators = attr.external_unicast_locators;
    qos.endpoint().ignore_non_matching_locators = attr.ignore_non_matching_locators;
    qos.endpoint().history_memory_policy = attr.historyMemoryPolicy;
    qos.endpoint().user_defined_id = attr.getUserDefinedID();
    qos.endpoint().entity_id = attr.getEntityID();
    qos.reliable_writer_qos().times = attr.times;
    qos.reliable_writer_qos().disable_positive_acks = attr.qos.m_disablePositiveACKs;
    qos.durability() = attr.qos.m_durability;
    qos.durability_service() = attr.qos.m_durabilityService;
    qos.deadline() = attr.qos.m_deadline;
    qos.latency_budget() = attr.qos.m_latencyBudget;
    qos.liveliness() = attr.qos.m_liveliness;
    qos.reliability() = attr.qos.m_reliability;
    qos.lifespan() = attr.qos.m_lifespan;
    qos.user_data().setValue(attr.qos.m_userData);
    qos.ownership() = attr.qos.m_ownership;
    qos.ownership_strength() = attr.qos.m_ownershipStrength;
    qos.destination_order() = attr.qos.m_destinationOrder;
    qos.representation() = attr.qos.representation;
    qos.publish_mode() = attr.qos.m_publishMode;
    qos.history() = attr.topic.historyQos;
    qos.resource_limits() = attr.topic.resourceLimitsQos;
    qos.data_sharing() = attr.qos.data_sharing;
    qos.reliable_writer_qos().disable_heartbeat_piggyback = attr.qos.disable_heartbeat_piggyback;

    if (attr.qos.m_partition.size() > 0 )
    {
        Property property;
        property.name("partitions");
        string partitions;
        bool is_first_partition = true;

        for (auto partition : attr.qos.m_partition.names())
        {
            partitions += (is_first_partition ? "" : ";") + partition;
            is_first_partition = false;
        }

        property.value(std::move(partitions));
        qos.properties().properties().push_back(std::move(property));
    }
}

void set_qos_from_attributes(
        DataReaderQos& qos,
        const xmlparser::SubscriberAttributes& attr)
{
    qos.reader_resource_limits().matched_publisher_allocation = attr.matched_publisher_allocation;
    qos.properties() = attr.properties;
    qos.expects_inline_qos(attr.expects_inline_qos);
    qos.endpoint().unicast_locator_list = attr.unicastLocatorList;
    qos.endpoint().multicast_locator_list = attr.multicastLocatorList;
    qos.endpoint().remote_locator_list = attr.remoteLocatorList;
    qos.endpoint().external_unicast_locators = attr.external_unicast_locators;
    qos.endpoint().ignore_non_matching_locators = attr.ignore_non_matching_locators;
    qos.endpoint().history_memory_policy = attr.historyMemoryPolicy;
    qos.endpoint().user_defined_id = attr.getUserDefinedID();
    qos.endpoint().entity_id = attr.getEntityID();
    qos.reliable_reader_qos().times = attr.times;
    qos.reliable_reader_qos().disable_positive_acks = attr.qos.m_disablePositiveACKs;
    qos.durability() = attr.qos.m_durability;
    qos.durability_service() = attr.qos.m_durabilityService;
    qos.deadline() = attr.qos.m_deadline;
    qos.latency_budget() = attr.qos.m_latencyBudget;
    qos.liveliness() = attr.qos.m_liveliness;
    qos.reliability() = attr.qos.m_reliability;
    qos.lifespan() = attr.qos.m_lifespan;
    qos.user_data().setValue(attr.qos.m_userData);
    qos.ownership() = attr.qos.m_ownership;
    qos.destination_order() = attr.qos.m_destinationOrder;
    qos.type_consistency() = attr.qos.type_consistency;
    qos.representation() = attr.qos.representation;
    qos.time_based_filter() = attr.qos.m_timeBasedFilter;
    qos.history() = attr.topic.historyQos;
    qos.resource_limits() = attr.topic.resourceLimitsQos;
    qos.data_sharing() = attr.qos.data_sharing;

    if (attr.qos.m_partition.size() > 0 )
    {
        Property property;
        property.name("partitions");
        string partitions;
        bool is_first_partition = true;

        for (auto partition : attr.qos.m_partition.names())
        {
            partitions += (is_first_partition ? "" : ";") + partition;
            is_first_partition = false;
        }

        property.value(std::move(partitions));
        qos.properties().properties().push_back(std::move(property));
    }
}

void set_qos_from_attributes(
        DomainParticipantQos& qos,
        const eprosima::fastdds::rtps::RTPSParticipantAttributes& attr)
{
    qos.user_data().setValue(attr.userData);
    qos.allocation() = attr.allocation;
    qos.wire_protocol().prefix = attr.prefix;
    qos.wire_protocol().participant_id = attr.participantID;
    qos.wire_protocol().builtin = attr.builtin;
    qos.wire_protocol().port = attr.port;
    qos.wire_protocol().default_unicast_locator_list = attr.defaultUnicastLocatorList;
    qos.wire_protocol().default_multicast_locator_list = attr.defaultMulticastLocatorList;
    qos.wire_protocol().default_external_unicast_locators = attr.default_external_unicast_locators;
    qos.wire_protocol().ignore_non_matching_locators = attr.ignore_non_matching_locators;
    qos.wire_protocol().easy_mode(attr.easy_mode_ip);
    qos.transport().user_transports = attr.userTransports;
    qos.transport().use_builtin_transports = attr.useBuiltinTransports;
    qos.transport().send_socket_buffer_size = attr.sendSocketBufferSize;
    qos.transport().listen_socket_buffer_size = attr.listenSocketBufferSize;
    qos.transport().max_msg_size_no_frag = attr.max_msg_size_no_frag;
    qos.transport().netmask_filter = attr.netmaskFilter;
    qos.name() = attr.getName();
    qos.flow_controllers() = attr.flow_controllers;
    qos.builtin_controllers_sender_thread() = attr.builtin_controllers_sender_thread;
    qos.timed_events_thread() = attr.timed_events_thread;
    qos.discovery_server_thread() = attr.discovery_server_thread;
    qos.typelookup_service_thread() = attr.typelookup_service_thread;
#if HAVE_SECURITY
    qos.security_log_thread() = attr.security_log_thread;
#endif // if HAVE_SECURITY

    // Merge attributes and qos properties
    for (auto property : attr.properties.properties())
    {
        string* property_value = fastdds::rtps::PropertyPolicyHelper::find_property(
            qos.properties(), property.name());
        if (nullptr == property_value)
        {
            qos.properties().properties().emplace_back(property);
        }
        else
        {
            *property_value = property.value();
        }
    }
    qos.properties().binary_properties() = attr.properties.binary_properties();
}

void set_extended_qos_from_attributes(
        DomainParticipantExtendedQos& extended_qos,
        const eprosima::fastdds::xmlparser::ParticipantAttributes& attr)
{
    extended_qos.domainId() = attr.domainId;
    set_qos_from_attributes(extended_qos, attr.rtps);
}

void set_attributes_from_qos(
        fastdds::rtps::RTPSParticipantAttributes& attr,
        const DomainParticipantQos& qos)
{
    attr.allocation = qos.allocation();
    attr.properties = qos.properties();
    attr.setName(qos.name());
    attr.prefix = qos.wire_protocol().prefix;
    attr.participantID = qos.wire_protocol().participant_id;
    attr.easy_mode_ip = qos.wire_protocol().easy_mode();
    attr.builtin = qos.wire_protocol().builtin;
    attr.port = qos.wire_protocol().port;
    attr.defaultUnicastLocatorList = qos.wire_protocol().default_unicast_locator_list;
    attr.defaultMulticastLocatorList = qos.wire_protocol().default_multicast_locator_list;
    attr.default_external_unicast_locators = qos.wire_protocol().default_external_unicast_locators;
    attr.ignore_non_matching_locators = qos.wire_protocol().ignore_non_matching_locators;
    attr.userTransports = qos.transport().user_transports;
    attr.useBuiltinTransports = qos.transport().use_builtin_transports;
    attr.sendSocketBufferSize = qos.transport().send_socket_buffer_size;
    attr.listenSocketBufferSize = qos.transport().listen_socket_buffer_size;
    attr.max_msg_size_no_frag = qos.transport().max_msg_size_no_frag;
    attr.netmaskFilter = qos.transport().netmask_filter;
    attr.userData = qos.user_data().data_vec();
    attr.flow_controllers = qos.flow_controllers();
    attr.builtin_controllers_sender_thread = qos.builtin_controllers_sender_thread();
    attr.timed_events_thread = qos.timed_events_thread();
    attr.discovery_server_thread = qos.discovery_server_thread();
    attr.typelookup_service_thread = qos.typelookup_service_thread();
#if HAVE_SECURITY
    attr.security_log_thread = qos.security_log_thread();
#endif // if HAVE_SECURITY
}

void set_attributes_from_extended_qos(
        eprosima::fastdds::xmlparser::ParticipantAttributes& attr,
        const DomainParticipantExtendedQos& extended_qos)
{
    attr.domainId = extended_qos.domainId();
    set_attributes_from_qos(attr.rtps, extended_qos);
}

void set_qos_from_attributes(
        TopicQos& qos,
        const xmlparser::TopicAttributes& attr)
{
    qos.history() = attr.historyQos;
    qos.resource_limits() = attr.resourceLimitsQos;
}

void set_qos_from_attributes(
        SubscriberQos& qos,
        const xmlparser::SubscriberAttributes& attr)
{
    qos.group_data().setValue(attr.qos.m_groupData);
    qos.partition() = attr.qos.m_partition;
    qos.presentation() = attr.qos.m_presentation;
}

void set_qos_from_attributes(
        PublisherQos& qos,
        const xmlparser::PublisherAttributes& attr)
{
    qos.group_data().setValue(attr.qos.m_groupData);
    qos.partition() = attr.qos.m_partition;
    qos.presentation() = attr.qos.m_presentation;
}

void set_qos_from_attributes(
        ReplierQos& qos,
        const xmlparser::ReplierAttributes& attr)
{
    qos.service_name = attr.service_name;
    qos.request_type = attr.request_type;
    qos.reply_type = attr.reply_type;
    qos.request_topic_name = attr.request_topic_name;
    qos.reply_topic_name = attr.reply_topic_name;
    set_qos_from_attributes(qos.writer_qos, attr.publisher);
    set_qos_from_attributes(qos.reader_qos, attr.subscriber);
}

void set_qos_from_attributes(
        RequesterQos& qos,
        const xmlparser::RequesterAttributes& attr)
{
    qos.service_name = attr.service_name;
    qos.request_type = attr.request_type;
    qos.reply_type = attr.reply_type;
    qos.request_topic_name = attr.request_topic_name;
    qos.reply_topic_name = attr.reply_topic_name;
    set_qos_from_attributes(qos.writer_qos, attr.publisher);
    set_qos_from_attributes(qos.reader_qos, attr.subscriber);
}

} /* namespace utils */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
