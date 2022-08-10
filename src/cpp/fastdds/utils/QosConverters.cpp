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

/*
 * QosConverters.cpp
 *
 */

#include <fastdds/utils/QosConverters.hpp>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastdds/rtps/common/Property.h>

namespace eprosima {
namespace fastdds {
namespace dds {

using fastrtps::PublisherAttributes;
using fastrtps::rtps::Property;

void set_qos_from_attributes(
        DataWriterQos& qos,
        const PublisherAttributes& attr)
{
    qos.writer_resource_limits().matched_subscriber_allocation = attr.matched_subscriber_allocation;
    qos.properties() = attr.properties;
    qos.throughput_controller() = attr.throughputController;
    qos.endpoint().unicast_locator_list = attr.unicastLocatorList;
    qos.endpoint().multicast_locator_list = attr.multicastLocatorList;
    qos.endpoint().remote_locator_list = attr.remoteLocatorList;
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
        std::string partitions;
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

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
