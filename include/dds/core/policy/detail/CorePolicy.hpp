#ifndef OMG_DDS_CORE_POLICY_DETAIL_CORE_POLICY_HPP_
#define OMG_DDS_CORE_POLICY_DETAIL_CORE_POLICY_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
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
 */


#include <org/opensplice/core/policy/PolicyDelegate.hpp>
#include <dds/core/policy/detail/TCorePolicyImpl.hpp>
#include <org/opensplice/core/policy/Policy.hpp>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds { namespace core { namespace policy { namespace detail {
#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
    typedef dds::core::policy::TDataRepresentation<org::opensplice::core::policy::DataRepresentationDelegate>
    DataRepresentation;
#endif // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

    typedef dds::core::policy::TDeadline<org::opensplice::core::policy::DeadlineDelegate>
    Deadline;

    typedef dds::core::policy::TDestinationOrder<org::opensplice::core::policy::DestinationOrderDelegate>
    DestinationOrder;

    typedef dds::core::policy::TDurability<org::opensplice::core::policy::DurabilityDelegate>
    Durability;

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    typedef dds::core::policy::TDurabilityService<org::opensplice::core::policy::DurabilityServiceDelegate>
    DurabilityService;
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

    typedef dds::core::policy::TEntityFactory<org::opensplice::core::policy::EntityFactoryDelegate>
    EntityFactory;

    typedef dds::core::policy::TGroupData<org::opensplice::core::policy::GroupDataDelegate>
    GroupData;

    typedef dds::core::policy::THistory<org::opensplice::core::policy::HistoryDelegate>
    History;

    typedef dds::core::policy::TLatencyBudget<org::opensplice::core::policy::LatencyBudgetDelegate>
    LatencyBudget;

    typedef dds::core::policy::TLifespan<org::opensplice::core::policy::LifespanDelegate>
    Lifespan;

    typedef dds::core::policy::TLiveliness<org::opensplice::core::policy::LivelinessDelegate>
    Liveliness;

    typedef dds::core::policy::TOwnership<org::opensplice::core::policy::OwnershipDelegate>
    Ownership;

#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
    typedef dds::core::policy::TOwnershipStrength<org::opensplice::core::policy::OwnershipStrengthDelegate>
    OwnershipStrength;
#endif  // OMG_DDS_OWNERSHIP_SUPPORT

    typedef dds::core::policy::TPartition<org::opensplice::core::policy::PartitionDelegate>
    Partition;

    typedef dds::core::policy::TPresentation<org::opensplice::core::policy::PresentationDelegate>
    Presentation;

    typedef dds::core::policy::TReaderDataLifecycle<org::opensplice::core::policy::ReaderDataLifecycleDelegate>
    ReaderDataLifecycle;

    typedef dds::core::policy::TReliability<org::opensplice::core::policy::ReliabilityDelegate>
    Reliability;

    typedef dds::core::policy::TResourceLimits<org::opensplice::core::policy::ResourceLimitsDelegate>
    ResourceLimits;

    typedef dds::core::policy::TTimeBasedFilter<org::opensplice::core::policy::TimeBasedFilterDelegate>
    TimeBasedFilter;

    typedef dds::core::policy::TTopicData<org::opensplice::core::policy::TopicDataDelegate>
    TopicData;

    typedef dds::core::policy::TTransportPriority<org::opensplice::core::policy::TransportPriorityDelegate>
    TransportPriority;

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
    typedef dds::core::policy::TTypeConsistencyEnforcement<org::opensplice::core::policy::TypeConsistencyEnforcementDelegate>
    TypeConsistencyEnforcement;
#endif // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

    typedef dds::core::policy::TUserData<org::opensplice::core::policy::UserDataDelegate>
    UserData;

    typedef dds::core::policy::TWriterDataLifecycle<org::opensplice::core::policy::WriterDataLifecycleDelegate>
    WriterDataLifecycle;
} } } } // namespace dds::core::policy::detail

/** @endcond */

#endif /* OMG_DDS_CORE_POLICY_DETAIL_CORE_POLICY_HPP_ */
