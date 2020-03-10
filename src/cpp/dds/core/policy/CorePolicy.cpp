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

#ifndef EPROSIMA_DDS_CORE_POLICY_TCOREPOLICY_IMPL_HPP_
#define EPROSIMA_DDS_CORE_POLICY_TCOREPOLICY_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/policy/CorePolicy.hpp>
#include <dds/core/policy/PolicyKind.hpp>

namespace dds {
namespace core {
namespace policy {

//UserData

UserData::UserData()
    : dds::core::Value<detail::UserData>()
{
}

UserData::UserData(
        const dds::core::ByteSeq& sequence)
    : dds::core::Value<detail::UserData>(sequence)
{
}

/** @internal @bug OSPL-1746 Implementation required */
UserData::UserData(
        const uint8_t* value_begin,
        const uint8_t* value_end)
{
    uint8_t* it = const_cast<uint8_t*>(value_begin);
    while (it != value_end)
    {
        delegate().data_vec().push_back(*it);
        ++it;
    }
}

UserData::UserData(
        const UserData& other)
    : dds::core::Value<detail::UserData>(other.delegate())
{
}

UserData& UserData::value(
        const dds::core::ByteSeq& sequence)
{
    delegate().data_vec(sequence);
    return *this;
}

/** @internal @bug OSPL-1746 Implementation required */
template<typename OCTET_ITER>
UserData& UserData::value(
        OCTET_ITER begin,
        OCTET_ITER end)
{
    OCTET_ITER it = const_cast<OCTET_ITER>(begin);
    while (it != end)
    {
        delegate().data_vec().push_back(*it);
        ++it;
    }
    return *this;
}

const dds::core::ByteSeq UserData::value() const
{
    return delegate().data_vec();
}

/** @internal @bug OSPL-1746 Implementation required */
const uint8_t* UserData::begin() const
{
    return delegate().data_vec().data();
}

/** @internal @bug OSPL-1746 Implementation required */
const uint8_t* UserData::end() const
{
    return begin() + delegate().data_vec().size();
}

//GroupData

GroupData::GroupData()
    : dds::core::Value<detail::GroupData>()
{
}

GroupData::GroupData(
        const dds::core::ByteSeq& sequence)
    : dds::core::Value<detail::GroupData>(sequence)
{
}

GroupData::GroupData(
        const GroupData& other)
    : dds::core::Value<detail::GroupData>(other.delegate())
{
}

GroupData::GroupData(
        const uint8_t* value_begin,
        const uint8_t* value_end)
{
    uint8_t* it = const_cast<uint8_t*>(value_begin);
    while (it != value_end)
    {
        delegate().push_back(*it);
        ++it;
    }
}

GroupData& GroupData::value(
        const dds::core::ByteSeq& sequence)
{
    delegate().setValue(sequence);
    return *this;
}

/** @internal @bug OSPL-1746 Implementation required */
template<typename OCTET_ITER>
GroupData& GroupData::value(
        OCTET_ITER begin,
        OCTET_ITER end)
{
    OCTET_ITER it = const_cast<OCTET_ITER>(begin);
    while (it != end)
    {
        delegate().push_back(*it);
        ++it;
    }
    return *this;
}

const dds::core::ByteSeq GroupData::value() const
{
    return delegate().getValue();
}

/** @internal @bug OSPL-1746 Implementation required */
const uint8_t* GroupData::begin() const
{
    return delegate().getValue().data();
}

/** @internal @bug OSPL-1746 Implementation required */
const uint8_t* GroupData::end() const
{
    return begin() + delegate().getValue().size();
}

//TopicData

TopicData::TopicData()
    : dds::core::Value<detail::TopicData>()
{
}

TopicData::TopicData(
        const dds::core::ByteSeq& sequence)
    : dds::core::Value<detail::TopicData>(sequence)
{
}

TopicData::TopicData(
        const TopicData& other)
    : dds::core::Value<detail::TopicData>(other.delegate())
{
}

/** @internal @bug OSPL-1746 Implementation required */
TopicData::TopicData(
        const uint8_t* value_begin,
        const uint8_t* value_end)
{
    uint8_t* it = const_cast<uint8_t*>(value_begin);
    while (it != value_end)
    {
        delegate().push_back(*it);
        ++it;
    }
}

TopicData& TopicData::value(
        const dds::core::ByteSeq& sequence)
{
    delegate().setValue(sequence);
    return *this;
}

/** @internal @bug OSPL-1746 Implementation required */
template<typename OCTET_ITER>
TopicData& TopicData::value(
        OCTET_ITER begin,
        OCTET_ITER end)
{
    OCTET_ITER it = const_cast<OCTET_ITER>(begin);
    while (it != end)
    {
        delegate().push_back(*it);
        ++it;
    }
    return *this;
}

const dds::core::ByteSeq TopicData::value() const
{
    return this->delegate().getValue();
}

/** @internal @bug OSPL-1746 Implementation required */
const uint8_t* TopicData::begin() const
{
    return delegate().getValue().data();
}

/** @internal @bug OSPL-1746 Implementation required */
const uint8_t* TopicData::end() const
{
    return begin() + delegate().getValue().size();
}

//EntityFactory

EntityFactory::EntityFactory(
        bool autoenable_created_entities)
    : dds::core::Value<detail::EntityFactory>(autoenable_created_entities)
{
}

EntityFactory::EntityFactory(
        const EntityFactory& other)
    : dds::core::Value<detail::EntityFactory>(other.delegate())
{
}

EntityFactory& EntityFactory::autoenable_created_entities(
        bool autoenable_created_entities)
{
    delegate().autoenable_created_entities = autoenable_created_entities;
    return *this;
}

bool EntityFactory::autoenable_created_entities() const
{
    return delegate().autoenable_created_entities;
}

EntityFactory EntityFactory::AutoEnable()
{
    return EntityFactory(true);
}

EntityFactory EntityFactory::ManuallyEnable()
{
    return EntityFactory(false);
}

//TransportPriority

TransportPriority::TransportPriority(
        int32_t priority)
    : dds::core::Value<detail::TransportPriority>(priority)
{
}

TransportPriority::TransportPriority(
        const TransportPriority& other)
    : dds::core::Value<detail::TransportPriority>(other.delegate())
{
}

TransportPriority& TransportPriority::value(
        int32_t priority)
{
    // TODO Review... signed vs unsigned :-/
    delegate().value = static_cast<uint32_t>(priority);
    return *this;
}

int32_t TransportPriority::value() const
{
    // TODO Review... signed vs unsigned :-/
    return static_cast<int32_t>(delegate().value);
}

//TLifeSpan

Lifespan::Lifespan(
        const dds::core::Duration& duration)
    : dds::core::Value<detail::Lifespan>(
        eprosima::fastrtps::Duration_t(static_cast<int32_t>(duration.sec()), duration.nanosec()))
{
}

Lifespan::Lifespan(
        const Lifespan& other)
    : dds::core::Value<detail::Lifespan>(other.delegate())
{
}

Lifespan& Lifespan::duration(
        const dds::core::Duration& duration)
{
    delegate().duration.seconds = static_cast<int32_t>(duration.sec());
    delegate().duration.nanosec = duration.nanosec();
    return *this;
}

const dds::core::Duration Lifespan::duration() const
{
    return Duration(delegate().duration.seconds, delegate().duration.nanosec);
}

//Deadline

Deadline::Deadline(
        const dds::core::Duration& period)
    : dds::core::Value<detail::Deadline>(
        eprosima::fastrtps::Duration_t(static_cast<int32_t>(period.sec()), period.nanosec()))
{
}

Deadline::Deadline(
        const Deadline& other)
    : dds::core::Value<detail::Deadline>(other.delegate())
{
}

Deadline& Deadline::period(
        const dds::core::Duration& period)
{
    delegate().period.seconds = static_cast<int32_t>(period.sec());
    delegate().period.nanosec = period.nanosec();
    return *this;
}

const dds::core::Duration Deadline::period() const
{
    return Duration(delegate().period.seconds, delegate().period.nanosec);
}

//LatencyBudget

LatencyBudget::LatencyBudget(
        const dds::core::Duration& duration)
    : dds::core::Value<detail::LatencyBudget>(
        eprosima::fastrtps::Duration_t(static_cast<int32_t>(duration.sec()), duration.nanosec()))
{
}

LatencyBudget::LatencyBudget(
        const LatencyBudget& other)
    : dds::core::Value<detail::LatencyBudget>(other.delegate())
{
}

LatencyBudget& LatencyBudget::duration(
        const dds::core::Duration& duration)
{
    delegate().duration.seconds = static_cast<int32_t>(duration.sec());
    delegate().duration.nanosec = duration.nanosec();
    return *this;
}

const dds::core::Duration LatencyBudget::duration() const
{
    return Duration(delegate().duration.seconds, delegate().duration.nanosec);
}

//TimeBasedFilter
TimeBasedFilter::TimeBasedFilter(
        const dds::core::Duration& minimum_separation)
    : dds::core::Value<detail::TimeBasedFilter>(
        eprosima::fastrtps::Duration_t(static_cast<int32_t>(minimum_separation.sec()), minimum_separation.nanosec()))
{
}

TimeBasedFilter::TimeBasedFilter(
        const TimeBasedFilter& other)
    : dds::core::Value<detail::TimeBasedFilter>(other.delegate())
{
}

TimeBasedFilter& TimeBasedFilter::minimum_separation(
        const dds::core::Duration& minimum_separation)
{
    delegate().minimum_separation.seconds = static_cast<int32_t>(minimum_separation.sec());
    delegate().minimum_separation.nanosec = minimum_separation.nanosec();
    return *this;

}

const dds::core::Duration TimeBasedFilter::minimum_separation() const
{
    return Duration(delegate().minimum_separation.seconds, delegate().minimum_separation.nanosec);
}

//Partition

Partition::Partition()
    : dds::core::Value<detail::Partition>()
{
}

Partition::Partition(
        uint16_t in_length)
    : dds::core::Value<detail::Partition>(in_length)
{
}

Partition::Partition(
        const Partition& other)
    : dds::core::Value<detail::Partition>(other.delegate())
{
}

Partition& Partition::name(
        const std::string& name)
{
    delegate().push_back(name.c_str());
    return *this;
}

Partition& Partition::names(
        StringSeq& names)
{
    delegate().names(names);
    return *this;
}

const dds::core::StringSeq Partition::names() const
{
    return delegate().names();
}

void Partition::push_back(
        const char* name)
{
    delegate().push_back(name);
}

void Partition::clear()
{
    delegate().clear();
}

//#ifdef OMG_DDS_OWNERSHIP_SUPPORT

//Ownership

Ownership::Ownership(
        dds::core::policy::OwnershipKind::Type kind)
    : dds::core::Value<detail::Ownership>(Ownership::to_native(kind))
{
}

Ownership::Ownership(
        const Ownership& other)
    : dds::core::Value<detail::Ownership>(other.delegate())
{
}

Ownership& Ownership::kind(
        dds::core::policy::OwnershipKind::Type kind)
{
    delegate().kind = Ownership::to_native(kind);
    return *this;
}

dds::core::policy::OwnershipKind::Type Ownership::kind() const
{
    return Ownership::from_native(delegate().kind);
}

Ownership Ownership::Exclusive()
{
    return Ownership(dds::core::policy::OwnershipKind::EXCLUSIVE);
}

Ownership Ownership::Shared()
{
    return Ownership(dds::core::policy::OwnershipKind::SHARED);
}

eprosima::fastdds::dds::OwnershipQosPolicyKind Ownership::to_native(
        OwnershipKind::Type kind)
{
    return kind == OwnershipKind::SHARED
           ? eprosima::fastdds::dds::OwnershipQosPolicyKind::SHARED_OWNERSHIP_QOS
           : eprosima::fastdds::dds::OwnershipQosPolicyKind::EXCLUSIVE_OWNERSHIP_QOS;
}

OwnershipKind::Type Ownership::from_native(
        eprosima::fastdds::dds::OwnershipQosPolicyKind kind)
{
    return kind == eprosima::fastdds::dds::OwnershipQosPolicyKind::SHARED_OWNERSHIP_QOS
           ? OwnershipKind::SHARED
           : OwnershipKind::EXCLUSIVE;
}

//OwnershipStrength
OwnershipStrength::OwnershipStrength(
        int32_t strength)
    : dds::core::Value<detail::OwnershipStrength>(strength)
{
}

OwnershipStrength::OwnershipStrength(
        const OwnershipStrength& other)
    : dds::core::Value<detail::OwnershipStrength>(other.delegate())
{
}

int32_t OwnershipStrength::value() const
{
    // TODO Review signed vs unsigned
    return static_cast<int32_t>(delegate().value);
}

OwnershipStrength& OwnershipStrength::value(
        int32_t strength)
{
    delegate().value = static_cast<uint32_t>(strength);
    return *this;
}

//#endif  //OMG_DDS_OWNERSHIP_SUPPORT

//TWriterDataLifeCycle
WriterDataLifecycle::WriterDataLifecycle(
        bool autodispose_unregistered_instances)
    : dds::core::Value<detail::WriterDataLifecycle>(autodispose_unregistered_instances)
{
}

WriterDataLifecycle::WriterDataLifecycle(
        const WriterDataLifecycle& other)
    : dds::core::Value<detail::WriterDataLifecycle>(other.delegate())
{
}

bool WriterDataLifecycle::autodispose_unregistered_instances() const
{
    return delegate().autodispose_unregistered_instances;
}

WriterDataLifecycle& WriterDataLifecycle::autodispose_unregistered_instances(
        bool autodispose_unregistered_instances)
{
    delegate().autodispose_unregistered_instances = autodispose_unregistered_instances;
    return *this;
}

WriterDataLifecycle WriterDataLifecycle::AutoDisposeUnregisteredInstances()
{
    return WriterDataLifecycle(true);
}

WriterDataLifecycle WriterDataLifecycle::ManuallyDisposeUnregisteredInstances()
{
    return WriterDataLifecycle(false);
}

//ReaderDataLifecycle
ReaderDataLifecycle::ReaderDataLifecycle(
        const dds::core::Duration& autopurge_nowriter_samples_delay,
        const dds::core::Duration& autopurge_disposed_samples_delay)
    : dds::core::Value<detail::ReaderDataLifecycle>(
        eprosima::fastrtps::Duration_t(static_cast<int32_t>(autopurge_nowriter_samples_delay.sec()),
        autopurge_nowriter_samples_delay.nanosec()),
        eprosima::fastrtps::Duration_t(static_cast<int32_t>(autopurge_disposed_samples_delay.sec()),
        autopurge_disposed_samples_delay.nanosec()))
{
}

ReaderDataLifecycle::ReaderDataLifecycle(
        const ReaderDataLifecycle& other)
    : dds::core::Value<detail::ReaderDataLifecycle>(other.delegate())
{
}

const dds::core::Duration ReaderDataLifecycle::autopurge_nowriter_samples_delay() const
{
    return Duration(this->delegate().autopurge_no_writer_samples_delay.seconds,
                   this->delegate().autopurge_no_writer_samples_delay.nanosec);
}

ReaderDataLifecycle& ReaderDataLifecycle::autopurge_nowriter_samples_delay(
        const dds::core::Duration& autopurge_nowriter_samples_delay)
{
    delegate().autopurge_no_writer_samples_delay.seconds = static_cast<int32_t>(
        autopurge_nowriter_samples_delay.sec());
    delegate().autopurge_no_writer_samples_delay.nanosec = autopurge_nowriter_samples_delay.nanosec();
    return *this;
}

const dds::core::Duration ReaderDataLifecycle::autopurge_disposed_samples_delay() const
{
    return Duration(this->delegate().autopurge_disposed_samples_delay.seconds,
                   this->delegate().autopurge_disposed_samples_delay.nanosec);
}

ReaderDataLifecycle& ReaderDataLifecycle::autopurge_disposed_samples_delay(
        const dds::core::Duration& autopurge_disposed_samples_delay)
{
    delegate().autopurge_disposed_samples_delay.seconds = static_cast<int32_t>(
        autopurge_disposed_samples_delay.sec());
    delegate().autopurge_disposed_samples_delay.nanosec = autopurge_disposed_samples_delay.nanosec();
    return *this;
}

ReaderDataLifecycle ReaderDataLifecycle::NoAutoPurgeDisposedSamples()
{
    return ReaderDataLifecycle();
}

ReaderDataLifecycle ReaderDataLifecycle::AutoPurgeDisposedSamples(
        const dds::core::Duration& autopurge_disposed_samples_delay)
{
    return ReaderDataLifecycle().autopurge_disposed_samples_delay(autopurge_disposed_samples_delay);
}

//Durability

Durability::Durability(
        dds::core::policy::DurabilityKind::Type kind)
    : dds::core::Value<detail::Durability>(Durability::to_native(kind))
{
}

Durability::Durability(
        const Durability& other)
    : dds::core::Value<detail::Durability>(other.delegate())
{
}

Durability& Durability::kind(
        dds::core::policy::DurabilityKind::Type kind)
{
    delegate().kind = Durability::to_native(kind);
    return *this;
}

dds::core::policy::DurabilityKind::Type Durability::kind() const
{
    return Durability::from_native(delegate().kind);
}

Durability Durability::Volatile()
{
    return Durability(dds::core::policy::DurabilityKind::VOLATILE);
}

Durability Durability::TransientLocal()
{
    return Durability(dds::core::policy::DurabilityKind::TRANSIENT_LOCAL);
}

Durability Durability::Transient()
{
    return Durability(dds::core::policy::DurabilityKind::TRANSIENT);
}

Durability Durability::Persistent()
{
    return Durability(dds::core::policy::DurabilityKind::PERSISTENT);
}

eprosima::fastdds::dds::DurabilityQosPolicyKind Durability::to_native(
        DurabilityKind::Type kind)
{
    eprosima::fastdds::dds::DurabilityQosPolicyKind result;
    switch (kind)
    {
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
        case policy::DurabilityKind::TRANSIENT:
            result = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_DURABILITY_QOS;
            break;
        case policy::DurabilityKind::PERSISTENT:
            result = eprosima::fastdds::dds::DurabilityQosPolicyKind::PERSISTENT_DURABILITY_QOS;
            break;
#endif
        case policy::DurabilityKind::VOLATILE:
            result = eprosima::fastdds::dds::DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS;
            break;
        case policy::DurabilityKind::TRANSIENT_LOCAL:
        default:
            result = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
            break;
    }
    return result;
}

DurabilityKind::Type Durability::from_native(
        eprosima::fastdds::dds::DurabilityQosPolicyKind kind)
{
    DurabilityKind::Type result;
    switch (kind)
    {
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
        case eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_DURABILITY_QOS:
            result = DurabilityKind::TRANSIENT;
            break;
        case eprosima::fastdds::dds::DurabilityQosPolicyKind::PERSISTENT_DURABILITY_QOS:
            result = DurabilityKind::PERSISTENT;
            break;
#endif
        case eprosima::fastdds::dds::DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS:
            result = DurabilityKind::VOLATILE;
            break;
        case eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS:
        default:
            result = DurabilityKind::TRANSIENT_LOCAL;
            break;
    }
    return result;
}

//Presentation
Presentation::Presentation(
        dds::core::policy::PresentationAccessScopeKind::Type access_scope,
        bool coherent_access,
        bool ordered_access)
    : dds::core::Value<detail::Presentation>(Presentation::to_native(access_scope), coherent_access, ordered_access)
{
}

Presentation::Presentation(
        const Presentation& other)
    : dds::core::Value<detail::Presentation>(other.delegate())
{
}

Presentation& Presentation::access_scope(
        dds::core::policy::PresentationAccessScopeKind::Type access_scope)
{
    delegate().access_scope = Presentation::to_native(access_scope);
    return *this;
}

dds::core::policy::PresentationAccessScopeKind::Type Presentation::access_scope() const
{
    return Presentation::from_native(delegate().access_scope);
}

eprosima::fastdds::dds::PresentationQosPolicyAccessScopeKind Presentation::to_native(
        PresentationAccessScopeKind::Type kind)
{
    eprosima::fastdds::dds::PresentationQosPolicyAccessScopeKind result;
    switch (kind)
    {
        case policy::PresentationAccessScopeKind::TOPIC:
            result = eprosima::fastdds::dds::TOPIC_PRESENTATION_QOS;
            break;
#ifdef  OMG_DDS_OBJECT_MODEL_SUPPORT
        case policy::PresentationAccessScopeKind::GROUP:
            result = eprosima::fastdds::dds::GROUP_PRESENTATION_QOS;
            break;
#endif  // OMG_DDS_OBJECT_MODEL_SUPPORT
        case policy::PresentationAccessScopeKind::INSTANCE:
        default:
            result = eprosima::fastdds::dds::INSTANCE_PRESENTATION_QOS;
            break;
    }
    return result;
}

PresentationAccessScopeKind::Type Presentation::from_native(
        eprosima::fastdds::dds::PresentationQosPolicyAccessScopeKind kind)
{
    PresentationAccessScopeKind::Type result;
    switch (kind)
    {
        case eprosima::fastdds::dds::TOPIC_PRESENTATION_QOS:
            result = PresentationAccessScopeKind::TOPIC;
            break;
#ifdef  OMG_DDS_OBJECT_MODEL_SUPPORT
        case eprosima::fastdds::dds::GROUP_PRESENTATION_QOS:
            result = PresentationAccessScopeKind::GROUP;
            break;
#endif  // OMG_DDS_OBJECT_MODEL_SUPPORT
        case eprosima::fastdds::dds::INSTANCE_PRESENTATION_QOS:
        default:
            result = PresentationAccessScopeKind::INSTANCE;
            break;
    }
    return result;
}

Presentation& Presentation::coherent_access(
        bool coherent_access)
{
    this->delegate().coherent_access = coherent_access;
    return *this;
}

bool Presentation::coherent_access() const
{
    return this->delegate().coherent_access;
}

Presentation& Presentation::ordered_access(
        bool ordered_access)
{
    this->delegate().ordered_access = ordered_access;
    return *this;
}

bool Presentation::ordered_access() const
{
    return this->delegate().ordered_access;
}

Presentation Presentation::GroupAccessScope(
        bool coherent_access,
        bool ordered_access)
{
    return Presentation(dds::core::policy::PresentationAccessScopeKind::GROUP, coherent_access, ordered_access);
}

Presentation Presentation::InstanceAccessScope(
        bool coherent_access,
        bool ordered_access)
{
    return Presentation(dds::core::policy::PresentationAccessScopeKind::INSTANCE, coherent_access, ordered_access);
}

Presentation Presentation::TopicAccessScope(
        bool coherent_access,
        bool ordered_access)
{
    return Presentation(dds::core::policy::PresentationAccessScopeKind::TOPIC, coherent_access, ordered_access);
}

//Reliability
Reliability::Reliability(
        dds::core::policy::ReliabilityKind::Type kind,
        const dds::core::Duration& max_blocking_time)
    :  dds::core::Value<detail::Reliability>(
        Reliability::to_native(kind),
        eprosima::fastrtps::Duration_t(static_cast<int32_t>(max_blocking_time.sec()), max_blocking_time.nanosec()))
{
}

Reliability::Reliability(
        const Reliability& other)
    : dds::core::Value<detail::Reliability>(other.delegate())
{
}

Reliability& Reliability::kind(
        dds::core::policy::ReliabilityKind::Type kind)
{
    delegate().kind = Reliability::to_native(kind);
    return *this;
}

dds::core::policy::ReliabilityKind::Type Reliability::kind() const
{
    return Reliability::from_native(delegate().kind);
}

Reliability& Reliability::max_blocking_time(
        const dds::core::Duration& max_blocking_time)
{
    delegate().max_blocking_time.seconds = static_cast<int32_t>(max_blocking_time.sec());
    delegate().max_blocking_time.nanosec = max_blocking_time.nanosec();
    return *this;
}

const dds::core::Duration Reliability::max_blocking_time() const
{
    return Duration(delegate().max_blocking_time.seconds, delegate().max_blocking_time.nanosec);
}

ReliabilityKind::Type Reliability::from_native(
        eprosima::fastdds::dds::ReliabilityQosPolicyKind kind)
{
    return kind == eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS
           ? ReliabilityKind::RELIABLE
           : ReliabilityKind::BEST_EFFORT;
}

eprosima::fastdds::dds::ReliabilityQosPolicyKind Reliability::to_native(
        ReliabilityKind::Type kind)
{
    return kind == ReliabilityKind::RELIABLE
           ? eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS
           : eprosima::fastdds::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
}

Reliability Reliability::Reliable(
        const dds::core::Duration& max_blocking_time)
{
    return Reliability(dds::core::policy::ReliabilityKind::RELIABLE, max_blocking_time);
}

Reliability Reliability::BestEffort(
        const dds::core::Duration& max_blocking_time)
{
    return Reliability(dds::core::policy::ReliabilityKind::BEST_EFFORT, max_blocking_time);
}

//DestinationOrder

DestinationOrder::DestinationOrder(
        dds::core::policy::DestinationOrderKind::Type kind)
    : dds::core::Value<detail::DestinationOrder>(DestinationOrder::to_native(kind))
{
}

DestinationOrder::DestinationOrder(
        const DestinationOrder& other)
    : dds::core::Value<detail::DestinationOrder>(other.delegate())
{
}

DestinationOrder& DestinationOrder::kind(
        dds::core::policy::DestinationOrderKind::Type kind)
{
    delegate().kind = to_native(kind);
    return *this;
}

dds::core::policy::DestinationOrderKind::Type DestinationOrder::kind() const
{
    return DestinationOrder::from_native(delegate().kind);
}

eprosima::fastdds::dds::DestinationOrderQosPolicyKind DestinationOrder::to_native(
        DestinationOrderKind::Type kind)
{
    eprosima::fastdds::dds::DestinationOrderQosPolicyKind result;
    switch (kind)
    {
        case policy::DestinationOrderKind::BY_SOURCE_TIMESTAMP:
            result = eprosima::fastdds::dds::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
            break;
        case policy::DestinationOrderKind::BY_RECEPTION_TIMESTAMP:
        default:
            result = eprosima::fastdds::dds::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
            break;
    }
    return result;
}

DestinationOrderKind::Type DestinationOrder::from_native(
        eprosima::fastdds::dds::DestinationOrderQosPolicyKind kind)
{
    DestinationOrderKind::Type result;
    switch (kind)
    {
        case eprosima::fastdds::dds::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS:
            result = DestinationOrderKind::BY_SOURCE_TIMESTAMP;
            break;
        case eprosima::fastdds::dds::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS:
        default:
            result = DestinationOrderKind::BY_RECEPTION_TIMESTAMP;
            break;
    }
    return result;
}

DestinationOrder DestinationOrder::SourceTimestamp()
{
    return DestinationOrder(dds::core::policy::DestinationOrderKind::BY_SOURCE_TIMESTAMP);
}

DestinationOrder DestinationOrder::ReceptionTimestamp()
{
    return DestinationOrder(dds::core::policy::DestinationOrderKind::BY_RECEPTION_TIMESTAMP);
}

//History
History::History(
        dds::core::policy::HistoryKind::Type kind,
        int32_t depth)
    : dds::core::Value<detail::History>(History::to_native(kind), depth)
{
}

History::History(
        const History& other)
    : dds::core::Value<detail::History>(other.delegate())
{
}

dds::core::policy::HistoryKind::Type History::kind() const
{
    return History::from_native(delegate().kind);
}

History& History::kind(
        dds::core::policy::HistoryKind::Type kind)
{
    delegate().kind = History::to_native(kind);
    return *this;
}

int32_t History::depth() const
{
    return delegate().depth;
}

History& History::depth(
        int32_t depth)
{
    delegate().depth = depth;
    return *this;
}

History History::KeepAll()
{
    return History(dds::core::policy::HistoryKind::KEEP_ALL, 1);
}

History History::KeepLast(
        uint32_t depth)
{
    return History(dds::core::policy::HistoryKind::KEEP_LAST, static_cast<int32_t>(depth));
}

eprosima::fastdds::dds::HistoryQosPolicyKind History::to_native(
        HistoryKind::Type kind)
{
    eprosima::fastdds::dds::HistoryQosPolicyKind result;
    switch (kind)
    {
        case policy::HistoryKind::KEEP_ALL:
            result = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
            break;
        case policy::HistoryKind::KEEP_LAST:
        default:
            result = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
            break;
    }
    return result;
}

HistoryKind::Type History::from_native(
        eprosima::fastdds::dds::HistoryQosPolicyKind kind)
{
    HistoryKind::Type result;
    switch (kind)
    {
        case eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS:
            result = HistoryKind::KEEP_ALL;
            break;
        case eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS:
        default:
            result = HistoryKind::KEEP_LAST;
            break;
    }
    return result;
}

//ResourceLimits
ResourceLimits::ResourceLimits(
        uint32_t max_samples,
        uint32_t max_instances,
        uint32_t max_samples_per_instance)
    :  dds::core::Value<detail::ResourceLimits>(max_samples, max_instances, max_samples_per_instance)
{
}

ResourceLimits::ResourceLimits(
        const ResourceLimits& other)
    : dds::core::Value<detail::ResourceLimits>(other.delegate())
{
}

ResourceLimits& ResourceLimits::max_samples(
        int32_t max_samples)
{
    delegate().max_samples = max_samples;
    return *this;
}

int32_t ResourceLimits::max_samples() const
{
    return delegate().max_samples;
}

ResourceLimits& ResourceLimits::max_instances(
        int32_t max_instances)
{
    delegate().max_instances = max_instances;
    return *this;
}

int32_t ResourceLimits::max_instances() const
{
    return delegate().max_instances;
}

ResourceLimits& ResourceLimits::max_samples_per_instance(
        int32_t max_samples_per_instance)
{
    delegate().max_samples_per_instance = max_samples_per_instance;
    return *this;
}

int32_t ResourceLimits::max_samples_per_instance() const
{
    return delegate().max_samples_per_instance;
}

//Liveliness
Liveliness::Liveliness(
        dds::core::policy::LivelinessKind::Type kind,
        const dds::core::Duration& lease_duration)
    : dds::core::Value<detail::Liveliness>(
        Liveliness::to_native(kind),
        eprosima::fastrtps::Duration_t(static_cast<int32_t>(lease_duration.sec()), lease_duration.nanosec()))
{
}

Liveliness::Liveliness(
        const Liveliness& other)
    : dds::core::Value<detail::Liveliness>(other.delegate())
{
}

Liveliness& Liveliness::kind(
        dds::core::policy::LivelinessKind::Type kind)
{
    delegate().kind = Liveliness::to_native(kind);
    return *this;
}

dds::core::policy::LivelinessKind::Type Liveliness::kind() const
{
    return Liveliness::from_native(delegate().kind);
}

Liveliness& Liveliness::lease_duration(
        const dds::core::Duration& lease_duration)
{
    delegate().lease_duration.seconds = static_cast<int32_t>(lease_duration.sec());
    delegate().lease_duration.nanosec = lease_duration.nanosec();
    return *this;
}

const dds::core::Duration Liveliness::lease_duration() const
{
    return Duration(delegate().lease_duration.seconds, delegate().lease_duration.nanosec);
}

Liveliness Liveliness::Automatic()
{
    return Liveliness(dds::core::policy::LivelinessKind::AUTOMATIC, dds::core::Duration::infinite());
}

Liveliness Liveliness::ManualByParticipant(
        const dds::core::Duration& lease_duration)
{
    return Liveliness(dds::core::policy::LivelinessKind::MANUAL_BY_PARTICIPANT, lease_duration);
}

Liveliness Liveliness::ManualByTopic(
        const dds::core::Duration& lease_duration)
{
    return Liveliness(dds::core::policy::LivelinessKind::MANUAL_BY_TOPIC, lease_duration);
}

eprosima::fastdds::dds::LivelinessQosPolicyKind Liveliness::to_native(
        LivelinessKind::Type kind)
{
    eprosima::fastdds::dds::LivelinessQosPolicyKind result;
    switch (kind)
    {
        case policy::LivelinessKind::MANUAL_BY_TOPIC:
            result = eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS;
            break;
        case policy::LivelinessKind::MANUAL_BY_PARTICIPANT:
            result = eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
            break;
        case policy::LivelinessKind::AUTOMATIC:
        default:
            result = eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS;
            break;
    }
    return result;
}

LivelinessKind::Type Liveliness::from_native(
        eprosima::fastdds::dds::LivelinessQosPolicyKind kind)
{
    LivelinessKind::Type result;
    switch (kind)
    {
        case eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS:
            result = LivelinessKind::MANUAL_BY_TOPIC;
            break;
        case eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
            result = LivelinessKind::MANUAL_BY_PARTICIPANT;
            break;
        case eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS:
        default:
            result = LivelinessKind::AUTOMATIC;
            break;
    }
    return result;
}

// #ifdef OMG_DDS_PERSISTENCE_SUPPORT

//DurabilityService

DurabilityService::DurabilityService(
        const dds::core::Duration& service_cleanup_delay,
        dds::core::policy::HistoryKind::Type history_kind,
        int32_t history_depth,
        int32_t max_samples,
        int32_t max_instances,
        int32_t max_samples_per_instance)
    : dds::core::Value<detail::DurabilityService>(
        eprosima::fastrtps::Duration_t(
            static_cast<int32_t>(service_cleanup_delay.sec()),
            service_cleanup_delay.nanosec()),
        History::to_native(history_kind),
        history_depth,
        max_samples,
        max_instances,
        max_samples_per_instance)
{
}

DurabilityService::DurabilityService(
        const DurabilityService& other)
    : dds::core::Value<detail::DurabilityService>(other.delegate())
{
}

DurabilityService& DurabilityService::service_cleanup_delay(
        const dds::core::Duration& service_cleanup_delay)
{
    delegate().service_cleanup_delay.seconds = static_cast<int32_t>(service_cleanup_delay.sec());
    delegate().service_cleanup_delay.nanosec = service_cleanup_delay.nanosec();
    return *this;
}

const dds::core::Duration DurabilityService::service_cleanup_delay() const
{
    return Duration(delegate().service_cleanup_delay.seconds, delegate().service_cleanup_delay.nanosec);
}

DurabilityService& DurabilityService::history_kind(
        dds::core::policy::HistoryKind::Type kind)
{
    switch (kind)
    {
        case policy::HistoryKind::KEEP_ALL:
            delegate().history_kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
            break;
        case policy::HistoryKind::KEEP_LAST:
        default:
            delegate().history_kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
            break;
    }
    return *this;
}

dds::core::policy::HistoryKind::Type DurabilityService::history_kind() const
{
    dds::core::policy::HistoryKind::Type result;
    switch (delegate().history_kind)
    {
        case eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS:
            result = HistoryKind::KEEP_ALL;
            break;
        case eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS:
        default:
            result = HistoryKind::KEEP_LAST;
            break;
    }
    return result;
}

DurabilityService& DurabilityService::history_depth(
        int32_t history_depth)
{
    delegate().history_depth = history_depth;
    return *this;
}

int32_t DurabilityService::history_depth() const
{
    return delegate().history_depth;
}

DurabilityService& DurabilityService::max_samples(
        int32_t max_samples)
{
    delegate().max_samples = max_samples;
    return *this;
}

int32_t DurabilityService::max_samples() const
{
    return this->delegate().max_samples;
}

DurabilityService& DurabilityService::max_instances(
        int32_t max_instances)
{
    delegate().max_instances = max_instances;
    return *this;
}

int32_t DurabilityService::max_instances() const
{
    return this->delegate().max_instances;
}

DurabilityService& DurabilityService::max_samples_per_instance(
        int32_t max_samples_per_instance)
{
    delegate().max_samples_per_instance = max_samples_per_instance;
    return *this;
}

int32_t DurabilityService::max_samples_per_instance() const
{
    return this->delegate().max_samples_per_instance;
}

// #endif  // OMG_DDS_PERSISTENCE_SUPPORT


// #ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

//DataRepresentation

/** @internal @bug OSPL-1746 Implementation required */
DataRepresentation::DataRepresentation(
        const dds::core::policy::DataRepresentationIdSeq& value)
    : dds::core::Value<detail::DataRepresentation> (DataRepresentation::to_native(value))
{
}

std::vector<eprosima::fastdds::dds::DataRepresentationId> DataRepresentation::to_native(
        const DataRepresentationIdSeq& seq)
{
    std::vector<eprosima::fastdds::dds::DataRepresentationId> result;
    for (DataRepresentationId id : seq)
    {
        result.push_back(static_cast<eprosima::fastdds::dds::DataRepresentationId>(id));
    }
    return result;
}

/** @internal @bug OSPL-1746 Implementation required */
DataRepresentation& DataRepresentation::value(
        const dds::core::policy::DataRepresentationIdSeq& value)
{
    for (DataRepresentationId id : value)
    {
        delegate().m_value.push_back(static_cast<eprosima::fastdds::dds::DataRepresentationId>(id));
    }
    return *this;
}

/** @internal @bug OSPL-1746 Implementation required */
const dds::core::policy::DataRepresentationIdSeq DataRepresentation::value() const
{
    DataRepresentationIdSeq seq;
    for (eprosima::fastdds::dds::DataRepresentationId id : delegate().m_value)
    {
        seq.push_back(static_cast<uint16_t>(id));
    }
    return seq;
}

// #endif  //OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

// #ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

//TypeConsistencyEnforcement

/** @internal @bug OSPL-1746 Implementation required */
TypeConsistencyEnforcement::TypeConsistencyEnforcement(
        dds::core::policy::TypeConsistencyEnforcementKind::Type kind)
    : dds::core::Value<detail::TypeConsistencyEnforcement>(kind)
{
}

/** @internal @bug OSPL-1746 Implementation required */
TypeConsistencyEnforcement& TypeConsistencyEnforcement::kind(
        dds::core::policy::TypeConsistencyEnforcementKind::Type kind)
{
    delegate().m_kind = kind;
    return *this;
}

/** @internal @bug OSPL-1746 Implementation required */
dds::core::policy::TypeConsistencyEnforcementKind::Type TypeConsistencyEnforcement::kind() const
{
    return delegate().m_kind;
}

TypeConsistencyEnforcement& TypeConsistencyEnforcement::ignore_sequence_bounds(
        bool ignore_sequence_bounds)
{
    delegate().m_ignore_sequence_bounds = ignore_sequence_bounds;
    return *this;
}

TypeConsistencyEnforcement& TypeConsistencyEnforcement::ignore_string_bounds(
        bool ignore_string_bounds)
{
    delegate().m_ignore_string_bounds = ignore_string_bounds;
    return *this;
}

TypeConsistencyEnforcement& TypeConsistencyEnforcement::ignore_member_names(
        bool ignore_member_names)
{
    delegate().m_ignore_member_names = ignore_member_names;
    return *this;
}

TypeConsistencyEnforcement& TypeConsistencyEnforcement::prevent_type_widening(
        bool prevent_type_widening)
{
    delegate().m_prevent_type_widening = prevent_type_widening;
    return *this;
}

TypeConsistencyEnforcement& TypeConsistencyEnforcement::force_type_validation(
        bool force_type_validation)
{
    delegate().m_force_type_validation = force_type_validation;
    return *this;
}

bool TypeConsistencyEnforcement::ignore_sequence_bounds()
{
    return delegate().m_ignore_sequence_bounds;
}

bool TypeConsistencyEnforcement::ignore_string_bounds()
{
    return delegate().m_ignore_string_bounds;
}

bool TypeConsistencyEnforcement::ignore_member_names()
{
    return delegate().m_ignore_member_names;
}

bool TypeConsistencyEnforcement::prevent_type_widening()
{
    return delegate().m_prevent_type_widening;
}

bool TypeConsistencyEnforcement::force_type_validation()
{
    return delegate().m_force_type_validation;
}

// #endif  //OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

} //namespace policy
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_POLICY_TCOREPOLICY_IMPL_HPP_
