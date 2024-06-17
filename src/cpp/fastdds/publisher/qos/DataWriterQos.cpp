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
 * @file DataWriterQos.cpp
 *
 */

#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/core/policy/QosPolicyUtils.hpp>
namespace eprosima {
namespace fastdds {
namespace dds {

const DataWriterQos DATAWRITER_QOS_DEFAULT;
const DataWriterQos DATAWRITER_QOS_USE_TOPIC_QOS;

DataWriterQos::DataWriterQos()
{
    reliability_.kind = RELIABLE_RELIABILITY_QOS;
    durability_.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
}

WriterQos DataWriterQos::get_writerqos(
        const PublisherQos& pqos,
        const TopicQos& tqos) const
{
    WriterQos qos;
    qos.m_deadline = deadline();
    qos.m_destinationOrder = destination_order();
    qos.m_disablePositiveACKs = reliable_writer_qos().disable_positive_acks;
    qos.m_durability = durability();
    qos.m_durabilityService = durability_service();
    qos.m_groupData = pqos.group_data();
    qos.m_latencyBudget = latency_budget();
    qos.m_lifespan = lifespan();
    qos.m_liveliness = liveliness();
    qos.m_ownership = ownership();
    qos.m_ownershipStrength = ownership_strength();
    qos.m_partition = pqos.partition();
    qos.m_presentation = pqos.presentation();
    qos.m_publishMode = publish_mode();
    qos.m_reliability = reliability();
    qos.m_topicData = tqos.topic_data();
    qos.m_userData = user_data();
    qos.representation = representation();
    qos.data_sharing = data_sharing();

    if (qos.data_sharing.kind() != OFF &&
            qos.data_sharing.domain_ids().empty())
    {
        qos.data_sharing.add_domain_id(utils::default_domain_id());
    }

    return qos;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
