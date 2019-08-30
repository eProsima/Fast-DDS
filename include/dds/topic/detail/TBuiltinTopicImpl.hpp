/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef EPROSIMA_DDS_TOPIC_TBUILTINTOPIC_IMPL_HPP_
#define EPROSIMA_DDS_TOPIC_TBUILTINTOPIC_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/topic/BuiltinTopic.hpp>

namespace dds {
namespace topic {

//TParticipantBuiltinTopicData
template<typename D>
const dds::topic::BuiltinTopicKey& TParticipantBuiltinTopicData<D>::key() const
{
    //To implement
//    return this->delegate().key();
}

template<typename D>
const dds::core::policy::UserData& TParticipantBuiltinTopicData<D>::user_data() const
{
    //To implement
//    return this->delegate().user_data();
}

//TTopicBuiltinTopicData
template<typename D>
const dds::topic::BuiltinTopicKey& TTopicBuiltinTopicData<D>::key() const
{
    //To implement
//    return this->delegate().key();
}

template<typename D>
const std::string& TTopicBuiltinTopicData<D>::name() const
{
    //To implement
//    return this->delegate().name();
}

template<typename D>
const std::string& TTopicBuiltinTopicData<D>::type_name() const
{
    //To implement
//    return this->delegate().type_name();
}

template<typename D>
const dds::core::policy::Durability& TTopicBuiltinTopicData<D>::durability() const
{
    //To implement
//    return this->delegate().durability();
}

#ifdef OMG_DDS_PERSISTENCE_SUPPORT
template<typename D>
const dds::core::policy::DurabilityService& TTopicBuiltinTopicData<D>::durability_service() const
{
    //To implement
//    return this->delegate().durability_service();
}

#endif  // OMG_DDS_PERSISTENCE_SUPPORT

template<typename D>
const dds::core::policy::Deadline& TTopicBuiltinTopicData<D>::deadline() const
{
    //To implement
//    return this->delegate().deadline();
}

template<typename D>
const dds::core::policy::LatencyBudget& TTopicBuiltinTopicData<D>::latency_budget() const
{
    //To implement
//    return this->delegate().latency_budget();
}

template<typename D>
const dds::core::policy::Liveliness& TTopicBuiltinTopicData<D>::liveliness() const
{
    //To implement
//    return this->delegate().liveliness();
}

template<typename D>
const dds::core::policy::Reliability& TTopicBuiltinTopicData<D>::reliability() const
{
    //To implement
//    return this->delegate().reliability();
}

template<typename D>
const dds::core::policy::TransportPriority& TTopicBuiltinTopicData<D>::transport_priority() const
{
    //To implement
//    return this->delegate().transport_priority();
}

template<typename D>
const dds::core::policy::Lifespan& TTopicBuiltinTopicData<D>::lifespan() const
{
    //To implement
//    return this->delegate().lifespan();
}

template<typename D>
const dds::core::policy::DestinationOrder& TTopicBuiltinTopicData<D>::destination_order() const
{
    //To implement
//    return this->delegate().destination_order();
}

template<typename D>
const dds::core::policy::History& TTopicBuiltinTopicData<D>::history() const
{
    //To implement
//    return this->delegate().history();
}

template<typename D>
const dds::core::policy::ResourceLimits& TTopicBuiltinTopicData<D>::resource_limits() const
{
    //To implement
//    return this->delegate().resource_limits();
}

template<typename D>
const dds::core::policy::Ownership& TTopicBuiltinTopicData<D>::ownership() const
{
    //To implement
//    return this->delegate().ownership();
}

template<typename D>
const dds::core::policy::TopicData& TTopicBuiltinTopicData<D>::topic_data() const
{
    //To implement
//    return this->delegate().topic_data();
}

//TPublicationBuiltinTopicData
template<typename D>
const dds::topic::BuiltinTopicKey&  TPublicationBuiltinTopicData<D>::key() const
{
    //To implement
//    return this->delegate().key();
}

template<typename D>
const dds::topic::BuiltinTopicKey& TPublicationBuiltinTopicData<D>::participant_key() const
{
    //To implement
//    return this->delegate().participant_key();
}

template<typename D>
const std::string& TPublicationBuiltinTopicData<D>::topic_name() const
{
    //To implement
//    return this->delegate().topic_name();
}

template<typename D>
const std::string& TPublicationBuiltinTopicData<D>::type_name() const
{
    //To implement
//    return this->delegate().type_name();
}

template<typename D>
const dds::core::policy::Durability& TPublicationBuiltinTopicData<D>::durability() const
{
    //To implement
//    return this->delegate().durability();
}

#ifdef OMG_DDS_PERSISTENCE_SUPPORT

template<typename D>
const dds::core::policy::DurabilityService& TPublicationBuiltinTopicData<D>::durability_service() const
{
    //To implement
//    return this->delegate().durability_service();
}
#endif  //OMG_DDS_PERSISTENCE_SUPPORT

template<typename D>
const dds::core::policy::Deadline& TPublicationBuiltinTopicData<D>::deadline() const
{
    //To implement
//    return this->delegate().deadline();
}

template<typename D>
const dds::core::policy::LatencyBudget& TPublicationBuiltinTopicData<D>::latency_budget() const
{
    //To implement
//    return this->delegate().latency_budget();
}

template<typename D>
const dds::core::policy::Liveliness& TPublicationBuiltinTopicData<D>::liveliness() const
{
    //To implement
//    return this->delegate().liveliness();
}

template<typename D>
const dds::core::policy::Reliability& TPublicationBuiltinTopicData<D>::reliability() const
{
    //To implement
//    return this->delegate().reliability();
}


template<typename D>
const dds::core::policy::Lifespan& TPublicationBuiltinTopicData<D>::lifespan() const
{
    //To implement
//    return this->delegate().lifespan();
}

template<typename D>
const dds::core::policy::UserData& TPublicationBuiltinTopicData<D>::user_data() const
{
    //To implement
//    return this->delegate().user_data();
}

template<typename D>
const dds::core::policy::Ownership& TPublicationBuiltinTopicData<D>::ownership() const
{
//To implement
//    return this->delegate().ownership();
}

#ifdef OMG_DDS_OWNERSHIP_SUPPORT

template<typename D>
const dds::core::policy::OwnershipStrength& TPublicationBuiltinTopicData<D>::ownership_strength() const
{
    //To implement
//    return this->delegate().ownership_strength();
}
#endif  // OMG_DDS_OWNERSHIP_SUPPORT


template<typename D>
const dds::core::policy::DestinationOrder& TPublicationBuiltinTopicData<D>::destination_order() const
{
    //To implement
//    return this->delegate().destination_order();
}

template<typename D>
const dds::core::policy::Presentation& TPublicationBuiltinTopicData<D>::presentation() const
{
    //To implement
//    return this->delegate().presentation();
}

template<typename D>
const dds::core::policy::Partition& TPublicationBuiltinTopicData<D>::partition() const
{
    //To implement
//    return this->delegate().partition();
}

template<typename D>
const dds::core::policy::TopicData& TPublicationBuiltinTopicData<D>::topic_data() const
{
    //To implement
//    return this->delegate().topic_data();
}

template<typename D>
const dds::core::policy::GroupData& TPublicationBuiltinTopicData<D>::group_data() const
{
    //To implement
//    return this->delegate().group_data();
}

template<typename D>
const dds::topic::BuiltinTopicKey& TSubscriptionBuiltinTopicData<D>::key() const
{
    //To implement
//    return this->delegate().key();
}

template<typename D>
const dds::topic::BuiltinTopicKey& TSubscriptionBuiltinTopicData<D>::participant_key() const
{
    //To implement
//    return this->delegate().participant_key();
}

template<typename D>
const std::string& TSubscriptionBuiltinTopicData<D>::topic_name() const
{
    //To implement
//    return this->delegate().topic_name();
}

template<typename D>
const std::string& TSubscriptionBuiltinTopicData<D>::type_name() const
{
    //To implement
//    return this->delegate().type_name();
}

template<typename D>
const dds::core::policy::Durability& TSubscriptionBuiltinTopicData<D>::durability() const
{
    //To implement
//    return this->delegate().durability();
}

template<typename D>
const dds::core::policy::Deadline& TSubscriptionBuiltinTopicData<D>::deadline() const
{
    //To implement
//    return this->delegate().deadline();
}

template<typename D>
const dds::core::policy::LatencyBudget& TSubscriptionBuiltinTopicData<D>::latency_budget() const
{
    //To implement
//    return this->delegate().latency_budget();
}

template<typename D>
const dds::core::policy::Liveliness& TSubscriptionBuiltinTopicData<D>::liveliness() const
{
    //To implement
//    return this->delegate().liveliness();
}

template<typename D>
const dds::core::policy::Reliability& TSubscriptionBuiltinTopicData<D>::reliability() const
{
    //To implement
//    return this->delegate().reliability();
}

template<typename D>
const dds::core::policy::Ownership& TSubscriptionBuiltinTopicData<D>::ownership() const
{
    //To implement
//    return this->delegate().ownership();
}

template<typename D>
const dds::core::policy::DestinationOrder& TSubscriptionBuiltinTopicData<D>::destination_order() const
{
    //To implement
//    return this->delegate().destination_order();
}

template<typename D>
const dds::core::policy::UserData& TSubscriptionBuiltinTopicData<D>::user_data() const
{
    //To implement
//    return this->delegate().user_data();
}

template<typename D>
const dds::core::policy::TimeBasedFilter& TSubscriptionBuiltinTopicData<D>::time_based_filter() const
{
    //To implement
//    return this->delegate().time_based_filter();
}

template<typename D>
const dds::core::policy::Presentation& TSubscriptionBuiltinTopicData<D>::presentation() const
{
    //To implement
//    return this->delegate().presentation();
}

template<typename D>
const dds::core::policy::Partition& TSubscriptionBuiltinTopicData<D>::partition() const
{
    //To implement
//    return this->delegate().partition();
}

template<typename D>
const dds::core::policy::TopicData& TSubscriptionBuiltinTopicData<D>::topic_data() const
{
    //To implement
//    return this->delegate().topic_data();
}

template<typename D>
const dds::core::policy::GroupData& TSubscriptionBuiltinTopicData<D>::group_data() const
{
    //To implement
//    return this->delegate().group_data();
}

} //namespace topic
} //namespace dds

#endif //EPROSIMA_DDS_TOPIC_TBUILTINTOPIC_IMPL_HPP_
