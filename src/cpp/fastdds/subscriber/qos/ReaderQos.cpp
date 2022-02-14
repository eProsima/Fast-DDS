// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ReaderQos.cpp
 *
 */

#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>
#include <fastdds/dds/log/Log.hpp>

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastdds {
namespace dds {

void ReaderQos::setQos(
        const ReaderQos& qos,
        bool first_time)
{
    if (first_time)
    {
        m_durability = qos.m_durability;
        m_durability.hasChanged = true;
    }
    if (first_time || m_deadline.period != qos.m_deadline.period)
    {
        m_deadline = qos.m_deadline;
        m_deadline.hasChanged = true;
    }
    if (m_latencyBudget.duration != qos.m_latencyBudget.duration)
    {
        m_latencyBudget = qos.m_latencyBudget;
        m_latencyBudget.hasChanged = true;
    }
    if (first_time)
    {
        m_liveliness = qos.m_liveliness;
        m_liveliness.hasChanged = true;
    }
    if (first_time)
    {
        m_reliability = qos.m_reliability;
        m_reliability.hasChanged = true;
    }
    if (first_time)
    {
        m_ownership = qos.m_ownership;
        m_ownership.hasChanged = true;
    }
    if (m_destinationOrder.kind != qos.m_destinationOrder.kind)
    {
        m_destinationOrder = qos.m_destinationOrder;
        m_destinationOrder.hasChanged = true;
    }
    if (m_userData.data_vec() != qos.m_userData.data_vec())
    {
        m_userData = qos.m_userData;
        m_userData.hasChanged = true;
    }
    if (m_timeBasedFilter.minimum_separation != qos.m_timeBasedFilter.minimum_separation )
    {
        m_timeBasedFilter = qos.m_timeBasedFilter;
        m_timeBasedFilter.hasChanged = true;
    }
    if (first_time || m_presentation.access_scope != qos.m_presentation.access_scope ||
            m_presentation.coherent_access != qos.m_presentation.coherent_access ||
            m_presentation.ordered_access != qos.m_presentation.ordered_access)
    {
        m_presentation = qos.m_presentation;
        m_presentation.hasChanged = true;
    }
    if (qos.m_partition.names() != m_partition.names())
    {
        m_partition = qos.m_partition;
        m_partition.hasChanged = true;
    }
    if (m_topicData.getValue() != qos.m_topicData.getValue())
    {
        m_topicData = qos.m_topicData;
        m_topicData.hasChanged = true;
    }
    if (m_groupData.getValue() != qos.m_groupData.getValue())
    {
        m_groupData = qos.m_groupData;
        m_groupData.hasChanged = true;
    }
    if (first_time || m_durabilityService.history_kind != qos.m_durabilityService.history_kind ||
            m_durabilityService.history_depth != qos.m_durabilityService.history_depth ||
            m_durabilityService.max_instances != qos.m_durabilityService.max_instances ||
            m_durabilityService.max_samples != qos.m_durabilityService.max_samples ||
            m_durabilityService.max_samples_per_instance != qos.m_durabilityService.max_samples_per_instance ||
            m_durabilityService.service_cleanup_delay != qos.m_durabilityService.service_cleanup_delay)
    {
        m_durabilityService = qos.m_durabilityService;
        m_durabilityService.hasChanged = true;
    }
    if (m_lifespan.duration != qos.m_lifespan.duration )
    {
        m_lifespan = qos.m_lifespan;
        m_lifespan.hasChanged = true;
    }
    if (first_time)
    {
        m_disablePositiveACKs = qos.m_disablePositiveACKs;
        m_disablePositiveACKs.hasChanged = true;
    }

    if (representation.m_value != qos.representation.m_value)
    {
        representation = qos.representation;
        representation.hasChanged = true;
    }

    if (first_time ||
            type_consistency.m_kind != qos.type_consistency.m_kind ||
            type_consistency.m_ignore_member_names != qos.type_consistency.m_ignore_member_names ||
            type_consistency.m_ignore_string_bounds != qos.type_consistency.m_ignore_string_bounds ||
            type_consistency.m_ignore_sequence_bounds != qos.type_consistency.m_ignore_sequence_bounds ||
            type_consistency.m_force_type_validation != qos.type_consistency.m_force_type_validation ||
            type_consistency.m_prevent_type_widening != qos.type_consistency.m_prevent_type_widening)
    {
        type_consistency = qos.type_consistency;
        type_consistency.hasChanged = true;
    }
}

bool ReaderQos::checkQos() const
{
    if (m_durability.kind == PERSISTENT_DURABILITY_QOS)
    {
        logError(RTPS_QOS_CHECK, "PERSISTENT Durability not supported");
        return false;
    }
    if (m_destinationOrder.kind == BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS)
    {
        logError(RTPS_QOS_CHECK, "BY SOURCE TIMESTAMP DestinationOrder not supported");
        return false;
    }
    if (m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS && m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
    {
        logError(RTPS_QOS_CHECK, "BEST_EFFORT incompatible with EXCLUSIVE ownership");
        return false;
    }
    return true;
}

bool ReaderQos::canQosBeUpdated(
        const ReaderQos& qos) const
{
    bool updatable = true;
    if ( m_durability.kind != qos.m_durability.kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Durability kind cannot be changed after the creation of a subscriber.");
    }

    if (m_liveliness.kind != qos.m_liveliness.kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Liveliness Kind cannot be changed after the creation of a subscriber.");
    }

    if (m_liveliness.lease_duration != qos.m_liveliness.lease_duration)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Liveliness lease duration cannot be changed after the creation of a subscriber.");
    }

    if (m_liveliness.announcement_period != qos.m_liveliness.announcement_period)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Liveliness announcement cannot be changed after the creation of a subscriber.");
    }

    if (m_reliability.kind != qos.m_reliability.kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Reliability Kind cannot be changed after the creation of a subscriber.");
    }
    if (m_ownership.kind != qos.m_ownership.kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Ownership Kind cannot be changed after the creation of a subscriber.");
    }
    if (m_destinationOrder.kind != qos.m_destinationOrder.kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Destination order Kind cannot be changed after the creation of a subscriber.");
    }
    return updatable;
}

void ReaderQos::clear()
{
    m_durability.clear();
    m_deadline.clear();
    m_latencyBudget.clear();
    m_liveliness.clear();
    m_reliability.clear();
    m_ownership.clear();
    m_destinationOrder.clear();
    m_userData.clear();
    m_timeBasedFilter.clear();
    m_presentation.clear();
    m_partition.clear();
    m_topicData.clear();
    m_groupData.clear();
    m_durabilityService.clear();
    m_lifespan.clear();
    m_disablePositiveACKs.clear();
    representation.clear();
    type_consistency.clear();
}

} //namespace dds
} //namespace fastdds
} //namespace eprosima
