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

#ifndef EPROSIMA_DDS_CORE_POLICY_DETAIL_CORE_POLICY_HPP_
#define EPROSIMA_DDS_CORE_POLICY_DETAIL_CORE_POLICY_HPP_

//TODO: Fix when PolicyDelegate and Policy are implemented
//#include <org/opensplice/core/policy/PolicyDelegate.hpp>
//#include <dds/core/policy/detail/TCorePolicyImpl.hpp>
//#include <org/opensplice/core/policy/Policy.hpp>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace core {
namespace policy {
namespace detail {

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
//TODO: Fix when PolicyDelegate and Policy are implemented
typedef dds::core::policy::TDataRepresentation<org::opensplice::core::policy::DataRepresentationDelegate>
        DataRepresentation;
class DataRepresentation
{
};
#endif // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TDeadline<org::opensplice::core::policy::DeadlineDelegate> Deadline;
class Deadline
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TDestinationOrder<org::opensplice::core::policy::DestinationOrderDelegate> DestinationOrder;
class DestinationOrder
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TDurability<org::opensplice::core::policy::DurabilityDelegate> Durability;
class Durability
{
};

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TDurabilityService<org::opensplice::core::policy::DurabilityServiceDelegate> DurabilityService;
class DurabilityService
{
};
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TEntityFactory<org::opensplice::core::policy::EntityFactoryDelegate> EntityFactory;
class EntityFactory
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TGroupData<org::opensplice::core::policy::GroupDataDelegate> GroupData;
class GroupData
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::THistory<org::opensplice::core::policy::HistoryDelegate> History;
class History
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TLatencyBudget<org::opensplice::core::policy::LatencyBudgetDelegate> LatencyBudget;
class LatencyBudget
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TLifespan<org::opensplice::core::policy::LifespanDelegate> Lifespan;
class Lifespan
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TLiveliness<org::opensplice::core::policy::LivelinessDelegate> Liveliness;
class Liveliness
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TOwnership<org::opensplice::core::policy::OwnershipDelegate> Ownership;
class Ownership
{
};

#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TOwnershipStrength<org::opensplice::core::policy::OwnershipStrengthDelegate> OwnershipStrength;
class OwnershipStrength
{
};
#endif  //OMG_DDS_OWNERSHIP_SUPPORT

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TPartition<org::opensplice::core::policy::PartitionDelegate> Partition;
class Partition
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TPresentation<org::opensplice::core::policy::PresentationDelegate> Presentation;
class Presentation
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TReaderDataLifecycle<org::opensplice::core::policy::ReaderDataLifecycleDelegate> ReaderDataLifecycle;
class ReaderDataLifecycle
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TReliability<org::opensplice::core::policy::ReliabilityDelegate> Reliability;
class Reliability
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TResourceLimits<org::opensplice::core::policy::ResourceLimitsDelegate> ResourceLimits;
class ResourceLimits
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TTimeBasedFilter<org::opensplice::core::policy::TimeBasedFilterDelegate> TimeBasedFilter;
class TimeBasedFilter
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TTopicData<org::opensplice::core::policy::TopicDataDelegate> TopicData;
class TopicData
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TTransportPriority<org::opensplice::core::policy::TransportPriorityDelegate> TransportPriority;
class TransportPriority
{
};

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TTypeConsistencyEnforcement<org::opensplice::core::policy::TypeConsistencyEnforcementDelegate> TypeConsistencyEnforcement;
class TypeConsistencyEnforcement
{
};
#endif //OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TUserData<org::opensplice::core::policy::UserDataDelegate> UserData;
class UserData
{
};

//TODO: Fix when PolicyDelegate and Policy are implemented
//typedef dds::core::policy::TWriterDataLifecycle<org::opensplice::core::policy::WriterDataLifecycleDelegate> WriterDataLifecycle;
class WriterDataLifecycle
{
};

} //namespace detail
} //namespace policy
} //namespace core
} //namespace dds

/** @endcond */

#endif //EPROSIMA_DDS_CORE_POLICY_DETAIL_CORE_POLICY_HPP_
