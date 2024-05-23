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
 * @file DataReaderQos.cpp
 *
 */

#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/core/policy/QosPolicyUtils.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

const DataReaderQos DATAREADER_QOS_DEFAULT;
const DataReaderQos DATAREADER_QOS_USE_TOPIC_QOS;

ReaderQos DataReaderQos::get_readerqos(
        const SubscriberQos& sqos) const
{
    ReaderQos qos;
    qos.m_durability = durability();
    qos.m_deadline = deadline();
    qos.m_latencyBudget = latency_budget();
    qos.m_liveliness = liveliness();
    qos.m_reliability = reliability();
    qos.m_destinationOrder = destination_order();
    qos.m_presentation = sqos.presentation();
    qos.m_partition = sqos.partition();
    qos.m_groupData = sqos.group_data();
    qos.m_userData = user_data();
    qos.m_ownership = ownership();
    qos.m_timeBasedFilter = time_based_filter();
    qos.m_lifespan = lifespan();
    //qos.m_topicData --> TODO: Fill with TopicQos info
    qos.m_durabilityService = durability_service();
    qos.m_disablePositiveACKs = reliable_reader_qos().disable_positive_acks;
    qos.type_consistency = type_consistency();
    qos.representation = representation();
    qos.data_sharing = data_sharing();

    if (qos.data_sharing.kind() != OFF &&
            qos.data_sharing.domain_ids().empty())
    {
        qos.data_sharing.add_domain_id(utils::default_domain_id());
    }

    return qos;
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
