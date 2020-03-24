/*
 * Copyright 2020, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License") {}
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

#include <dds/core/policy/CorePolicy.hpp>

namespace dds {
namespace core {
namespace policy {

OMG_DDS_DEFINE_POLICY_TRAITS(UserData)
OMG_DDS_DEFINE_POLICY_TRAITS(Durability)
OMG_DDS_DEFINE_POLICY_TRAITS(Presentation)
OMG_DDS_DEFINE_POLICY_TRAITS(Deadline)
OMG_DDS_DEFINE_POLICY_TRAITS(LatencyBudget)
OMG_DDS_DEFINE_POLICY_TRAITS(Ownership)

//#ifdef OMG_DDS_OWNERSHIP_SUPPORT
OMG_DDS_DEFINE_POLICY_TRAITS(OwnershipStrength)
//#endif  //OMG_DDS_OWNERSHIP_SUPPORT

OMG_DDS_DEFINE_POLICY_TRAITS(Liveliness)
OMG_DDS_DEFINE_POLICY_TRAITS(TimeBasedFilter)
OMG_DDS_DEFINE_POLICY_TRAITS(Partition)
OMG_DDS_DEFINE_POLICY_TRAITS(Reliability)
OMG_DDS_DEFINE_POLICY_TRAITS(DestinationOrder)
OMG_DDS_DEFINE_POLICY_TRAITS(History)
OMG_DDS_DEFINE_POLICY_TRAITS(ResourceLimits)
OMG_DDS_DEFINE_POLICY_TRAITS(EntityFactory)
OMG_DDS_DEFINE_POLICY_TRAITS(WriterDataLifecycle)
OMG_DDS_DEFINE_POLICY_TRAITS(ReaderDataLifecycle)
OMG_DDS_DEFINE_POLICY_TRAITS(TopicData)
OMG_DDS_DEFINE_POLICY_TRAITS(GroupData)
OMG_DDS_DEFINE_POLICY_TRAITS(TransportPriority)
OMG_DDS_DEFINE_POLICY_TRAITS(Lifespan)

//#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
OMG_DDS_DEFINE_POLICY_TRAITS(DurabilityService)
//#endif  //OMG_DDS_PERSISTENCE_SUPPORT


UserData::UserData()
: dds::core::Value<detail::UserData>()
{
}

UserData::UserData(
        const dds::core::ByteSeq& sequence)
: dds::core::Value<detail::UserData>(sequence)
{
}

UserData::UserData(
        const UserData& other)
: dds::core::Value<detail::UserData>(other)
{
}

void UserData::value(
        const dds::core::ByteSeq& sequence)
{
    delegate().setValue(sequence);
}

const dds::core::ByteSeq& UserData::value() const
{
    return delegate().getValue();
}

const uint8_t* UserData::begin() const
{
    return &delegate().front();
}

const uint8_t* UserData::end() const
{
    return &delegate().back();
}


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
: dds::core::Value<detail::GroupData>(other)
{
}

void GroupData::value(
        const dds::core::ByteSeq& sequence)
{
    delegate().setValue(sequence);
}

const dds::core::ByteSeq& GroupData::value() const
{
    return delegate().getValue();
}

const uint8_t* GroupData::begin() const
{
    return &delegate().front();
}

const uint8_t* GroupData::end() const
{
    return &delegate().back();
}


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
: dds::core::Value<detail::TopicData>(other)
{
}

void TopicData::value(
        const dds::core::ByteSeq& sequence)
{
    delegate().setValue(sequence);
}

const dds::core::ByteSeq& TopicData::value() const
{
    return delegate().getValue();
}

const uint8_t* TopicData::begin() const
{
    return &delegate().front();
}

const uint8_t* TopicData::end() const
{
    return &delegate().back();
}


EntityFactory::EntityFactory(
        bool autoenable_created_entities)
: dds::core::Value<detail::EntityFactory>(autoenable_created_entities)
{
}

EntityFactory::EntityFactory(
        const EntityFactory& other)
: dds::core::Value<detail::EntityFactory>(other)
{
}

void EntityFactory::autoenable_created_entities(
        bool autoenable_created_entities)
{
    delegate().autoenable_created_entities = autoenable_created_entities;
}

bool EntityFactory::autoenable_created_entities() const
{
    return delegate().autoenable_created_entities;
}

EntityFactory EntityFactory::AutoEnable()
{
    static EntityFactory autoenable(true);
    return autoenable;
}

EntityFactory EntityFactory::ManuallyEnable()
{
    static EntityFactory manualenable(false);
    return manualenable;
}


TransportPriority::TransportPriority(
        int32_t priority)
: dds::core::Value<detail::TransportPriority>(priority)
{
}

TransportPriority::TransportPriority(
        const TransportPriority& other)
: dds::core::Value<detail::TransportPriority>(other)
{
}

void TransportPriority::value(
        int32_t priority)
{
    delegate().value = priority;
}

int32_t TransportPriority::value() const
{
    return delegate().value;
}


Lifespan::Lifespan(
        const dds::core::Duration& duration)
: dds::core::Value<detail::Lifespan>(duration.delegate())
{
}

Lifespan::Lifespan(
        const Lifespan& other)
: dds::core::Value<detail::Lifespan>(other)
{
}

void Lifespan::duration(
        const dds::core::Duration& duration)
{
    delegate().duration = duration.delegate();
}

const dds::core::Duration Lifespan::duration() const
{
    return dds::core::Duration(delegate().duration.seconds, delegate().duration.nanosec);
}


Deadline::Deadline(
        const dds::core::Duration& period)
: dds::core::Value<detail::Deadline>(period.delegate())
{
}

Deadline::Deadline(
        const Deadline& other)
: dds::core::Value<detail::Deadline>(other)
{
}

void Deadline::period(
        const dds::core::Duration& period)
{
    delegate().period = period.delegate();
}

const dds::core::Duration Deadline::period() const
{
    return dds::core::Duration(delegate().period.seconds, delegate().period.nanosec);
}


LatencyBudget::LatencyBudget(
        const dds::core::Duration& duration)
: dds::core::Value<detail::LatencyBudget>(duration.delegate())
{
}

LatencyBudget::LatencyBudget(
        const LatencyBudget& other)
: dds::core::Value<detail::LatencyBudget>(other)
{
}

void LatencyBudget::duration(
        const dds::core::Duration& duration)
{
    delegate().duration = duration.delegate();
}

const dds::core::Duration LatencyBudget::duration() const
{
    return dds::core::Duration(delegate().duration.seconds, delegate().duration.nanosec);
}


TimeBasedFilter::TimeBasedFilter(
        const dds::core::Duration& period)
: dds::core::Value<detail::TimeBasedFilter>(period.delegate())
{
}

TimeBasedFilter::TimeBasedFilter(
        const TimeBasedFilter& other)
: dds::core::Value<detail::TimeBasedFilter>(other)
{
}

void TimeBasedFilter::minimum_separation(
        const dds::core::Duration& period)
{
    delegate().minimum_separation = period.delegate();
}
const dds::core::Duration TimeBasedFilter::minimum_separation() const
{
    return dds::core::Duration(delegate().minimum_separation.seconds, delegate().minimum_separation.nanosec);
}


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
: dds::core::Value<detail::Partition>(other)
{
}

Partition::Partition(
        const dds::core::StringSeq& names)
: dds::core::Value<detail::Partition>()
{
    delegate().setNames(names);
}

void Partition::names(
        dds::core::StringSeq& names)
{
    delegate().names(names);
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


Ownership::Ownership(
        dds::core::policy::OwnershipKind::Type kind)
: dds::core::Value<detail::Ownership>(Ownership::to_native(kind))
{
}

Ownership::Ownership(
        const Ownership& other)
: dds::core::Value<detail::Ownership>(other)
{
}

void Ownership::kind(
        dds::core::policy::OwnershipKind::Type kind)
{
    delegate().kind = Ownership::to_native(kind);
}

dds::core::policy::OwnershipKind::Type Ownership::kind() const
{
    return Ownership::from_native(delegate().kind);
}

Ownership Ownership::Exclusive()
{
    static Ownership exclusive(dds::core::policy::OwnershipKind::EXCLUSIVE);
    return exclusive;
}

Ownership Ownership::Shared()
{
    static Ownership shared(dds::core::policy::OwnershipKind::SHARED);
    return shared;
}

eprosima::fastdds::dds::OwnershipQosPolicyKind Ownership::to_native(
        OwnershipKind::Type kind)
{
    eprosima::fastdds::dds::OwnershipQosPolicyKind result;
    switch (kind)
    {
        case OwnershipKind::EXCLUSIVE:
            result = eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS;
            break;
        case OwnershipKind::SHARED:
            result = eprosima::fastdds::dds::SHARED_OWNERSHIP_QOS;
            break;
    }
    return result;
}

OwnershipKind::Type Ownership::from_native(
        eprosima::fastdds::dds::OwnershipQosPolicyKind kind)
{
    OwnershipKind::Type result;
    switch (kind)
    {
        case eprosima::fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS:
            result = OwnershipKind::EXCLUSIVE;
            break;
        case eprosima::fastdds::dds::SHARED_OWNERSHIP_QOS:
            result = OwnershipKind::SHARED;
            break;
    }
    return result;
}


OwnershipStrength::OwnershipStrength(
        int32_t strength)
: dds::core::Value<detail::OwnershipStrength>(strength)
{
}

OwnershipStrength::OwnershipStrength(
        const OwnershipStrength& other)
: dds::core::Value<detail::OwnershipStrength>(other)
{
}

int32_t OwnershipStrength::value() const
{
    return delegate().value;
}

void OwnershipStrength::value(
        int32_t strength)
{
    delegate().value = strength;
}


WriterDataLifecycle::WriterDataLifecycle(
        bool autodispose_unregistered_instances)
: dds::core::Value<detail::WriterDataLifecycle>(autodispose_unregistered_instances)
{
}

WriterDataLifecycle::WriterDataLifecycle(
        const WriterDataLifecycle& other)
: dds::core::Value<detail::WriterDataLifecycle>(other)
{
}

bool WriterDataLifecycle::autodispose_unregistered_instances() const
{
    return delegate().autodispose_unregistered_instances;
}

void WriterDataLifecycle::autodispose_unregistered_instances(
        bool autodispose_unregistered_instances)
{
    delegate().autodispose_unregistered_instances = autodispose_unregistered_instances;
}

WriterDataLifecycle WriterDataLifecycle::AutoDisposeUnregisteredInstances()
{
    static WriterDataLifecycle autodispose(true);
    return autodispose;
}

WriterDataLifecycle WriterDataLifecycle::ManuallyDisposeUnregisteredInstances()
{
    static WriterDataLifecycle manualdispose(false);
    return manualdispose;
}


ReaderDataLifecycle::ReaderDataLifecycle(
        const dds::core::Duration& autopurge_nowriter_samples_delay,
        const dds::core::Duration& autopurge_disposed_samples_delay)
: dds::core::Value<detail::ReaderDataLifecycle>(autopurge_nowriter_samples_delay, autopurge_disposed_samples_delay)
{
}

ReaderDataLifecycle::ReaderDataLifecycle(
        const ReaderDataLifecycle& other)
: dds::core::Value<detail::ReaderDataLifecycle>(other)
{
}

const dds::core::Duration ReaderDataLifecycle::autopurge_nowriter_samples_delay() const
{
    return dds::core::Duration(delegate().autopurge_no_writer_samples_delay.seconds,
            delegate().autopurge_no_writer_samples_delay.nanosec);
}

void ReaderDataLifecycle::autopurge_nowriter_samples_delay(
        const dds::core::Duration& autopurge_nowriter_samples_delay)
{
    delegate().autopurge_no_writer_samples_delay = autopurge_nowriter_samples_delay.delegate();
}

const dds::core::Duration ReaderDataLifecycle::autopurge_disposed_samples_delay() const
{
    return dds::core::Duration(delegate().autopurge_disposed_samples_delay.seconds,
            delegate().autopurge_disposed_samples_delay.nanosec);
}


void ReaderDataLifecycle::autopurge_disposed_samples_delay(
        const dds::core::Duration& autopurge_disposed_samples_delay)
{
    delegate().autopurge_disposed_samples_delay = autopurge_disposed_samples_delay.delegate();
}


ReaderDataLifecycle ReaderDataLifecycle::NoAutoPurgeDisposedSamples()
{
    static ReaderDataLifecycle no_auto_purge(dds::core::Duration::infinite(), dds::core::Duration::infinite());
    return no_auto_purge;
}

ReaderDataLifecycle ReaderDataLifecycle::AutoPurgeDisposedSamples(
        const dds::core::Duration& autopurge_disposed_samples_delay)
{
    static ReaderDataLifecycle auto_purge(dds::core::Duration::infinite(), autopurge_disposed_samples_delay);
    return auto_purge;
}

Durability::Durability(
        dds::core::policy::DurabilityKind::Type kind)
: dds::core::Value<detail::Durability>(Durability::to_native(kind))
{
}

Durability::Durability(
        const Durability& other)
: dds::core::Value<detail::Durability>(other)
{
}

void Durability::kind(
        dds::core::policy::DurabilityKind::Type kind)
{
    delegate().kind = Durability::to_native(kind);
}

dds::core::policy::DurabilityKind::Type  Durability::kind() const
{
    return Durability::from_native(delegate().kind);
}

Durability Durability::Volatile()
{
    static Durability volat(DurabilityKind::VOLATILE);
    return volat;
}

Durability Durability::TransientLocal()
{
    static Durability translocal(DurabilityKind::TRANSIENT_LOCAL);
    return translocal;
}

Durability Durability::Transient()
{
    static Durability transient(DurabilityKind::TRANSIENT);
    return transient;
}

Durability Durability::Persistent()
{
    static Durability persistent(DurabilityKind::PERSISTENT);
    return persistent;
}

eprosima::fastdds::dds::DurabilityQosPolicyKind Durability::to_native(
        DurabilityKind::Type kind)
{
    eprosima::fastdds::dds::DurabilityQosPolicyKind result;
    switch (kind)
    {
        case DurabilityKind::PERSISTENT:
            result = eprosima::fastdds::dds::PERSISTENT_DURABILITY_QOS;
            break;
        case DurabilityKind::TRANSIENT:
            result = eprosima::fastdds::dds::TRANSIENT_DURABILITY_QOS;
            break;
        case DurabilityKind::TRANSIENT_LOCAL:
            result = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
            break;
        case DurabilityKind::VOLATILE:
            result = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
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
        case eprosima::fastdds::dds::PERSISTENT_DURABILITY_QOS:
            result = DurabilityKind::PERSISTENT;
            break;
        case eprosima::fastdds::dds::TRANSIENT_DURABILITY_QOS:
            result = DurabilityKind::TRANSIENT;
            break;
        case eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS:
            result = DurabilityKind::TRANSIENT_LOCAL;
            break;
        case eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS:
            result = DurabilityKind::VOLATILE;
            break;
    }
    return result;
}


Presentation::Presentation(
        dds::core::policy::PresentationAccessScopeKind::Type access_scope,
        bool coherent_access,
        bool ordered_access)
: dds::core::Value<detail::Presentation>(
        Presentation::to_native(access_scope),
        coherent_access,
        ordered_access)
{
}

Presentation::Presentation(
        const Presentation& other)
: dds::core::Value<detail::Presentation>(other)
{
}

void Presentation::access_scope(
        dds::core::policy::PresentationAccessScopeKind::Type access_scope)
{
    delegate().access_scope = Presentation::to_native(access_scope);
}

dds::core::policy::PresentationAccessScopeKind::Type Presentation::access_scope() const
{
    return Presentation::from_native(delegate().access_scope);
}

void Presentation::coherent_access(
        bool coherent_access)
{
    delegate().coherent_access = coherent_access;
}

bool Presentation::coherent_access() const
{
    return delegate().coherent_access;
}

void Presentation::ordered_access(
        bool ordered_access)
{
    delegate().ordered_access = ordered_access;
}
bool Presentation::ordered_access() const
{
    return delegate().ordered_access;
}

Presentation Presentation::GroupAccessScope(
        bool coherent_access,
        bool ordered_access)
{
    static Presentation groupaccess(PresentationAccessScopeKind::GROUP,
            coherent_access,
            ordered_access);
    return groupaccess;
}

Presentation Presentation::InstanceAccessScope(
        bool coherent_access,
        bool ordered_access)
{
    static Presentation instanceaccess(PresentationAccessScopeKind::INSTANCE,
            coherent_access,
            ordered_access);
    return instanceaccess;
}

Presentation Presentation::TopicAccessScope(
        bool coherent_access,
        bool ordered_access)
{
    static Presentation topicaccess(PresentationAccessScopeKind::TOPIC,
            coherent_access,
            ordered_access);
    return topicaccess;
}

eprosima::fastdds::dds::PresentationQosPolicyAccessScopeKind Presentation::to_native(
        PresentationAccessScopeKind::Type kind)
{
    eprosima::fastdds::dds::PresentationQosPolicyAccessScopeKind result;
    switch (kind)
    {
        case PresentationAccessScopeKind::GROUP:
            result = eprosima::fastdds::dds::GROUP_PRESENTATION_QOS;
            break;
        case PresentationAccessScopeKind::INSTANCE:
            result = eprosima::fastdds::dds::INSTANCE_PRESENTATION_QOS;
            break;
        case PresentationAccessScopeKind::TOPIC:
            result = eprosima::fastdds::dds::TOPIC_PRESENTATION_QOS;
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
        case eprosima::fastdds::dds::GROUP_PRESENTATION_QOS:
            result = PresentationAccessScopeKind::GROUP;
            break;
        case eprosima::fastdds::dds::INSTANCE_PRESENTATION_QOS:
            result = PresentationAccessScopeKind::INSTANCE;
            break;
        case eprosima::fastdds::dds::TOPIC_PRESENTATION_QOS:
            result = PresentationAccessScopeKind::TOPIC;
            break;
    }
    return result;
}

Reliability::Reliability(
        dds::core::policy::ReliabilityKind::Type kind,
        const dds::core::Duration& max_blocking_time)
: dds::core::Value<detail::Reliability>(Reliability::to_native(kind), max_blocking_time.delegate())
{
}

Reliability::Reliability(
        const Reliability& other)
: dds::core::Value<detail::Reliability>(other)
{
}

void Reliability::kind(
        dds::core::policy::ReliabilityKind::Type kind)
{
    delegate().kind = Reliability::to_native(kind);
}

dds::core::policy::ReliabilityKind::Type Reliability::kind() const
{
    return Reliability::from_native(delegate().kind);
}

void Reliability::max_blocking_time(
        const dds::core::Duration& max_blocking_time)
{
    delegate().max_blocking_time = max_blocking_time.delegate();
}

const dds::core::Duration Reliability::max_blocking_time() const
{
    return dds::core::Duration(delegate().max_blocking_time.seconds,
            delegate().max_blocking_time.nanosec);
}

Reliability Reliability::Reliable(
        const dds::core::Duration& max_blocking_time)
{
    static Reliability reliable (ReliabilityKind::RELIABLE, max_blocking_time);
    return reliable;
}

Reliability Reliability::BestEffort(
        const dds::core::Duration& max_blocking_time)
{
    static Reliability best_effort (ReliabilityKind::BEST_EFFORT, max_blocking_time);
    return best_effort;
}


ReliabilityKind::Type Reliability::from_native(
        eprosima::fastdds::dds::ReliabilityQosPolicyKind kind)
{
    ReliabilityKind::Type result;
    switch (kind)
    {
        case eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS:
            result = ReliabilityKind::BEST_EFFORT;
            break;
        case eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS:
            result = ReliabilityKind::RELIABLE;
            break;
    }
    return result;
}

eprosima::fastdds::dds::ReliabilityQosPolicyKind Reliability::to_native(
        ReliabilityKind::Type kind)
{
    eprosima::fastdds::dds::ReliabilityQosPolicyKind result;
    switch (kind)
    {
        case ReliabilityKind::BEST_EFFORT:
            result = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
            break;
        case ReliabilityKind::RELIABLE:
            result = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
            break;
    }
    return result;
}

DestinationOrder::DestinationOrder(
        dds::core::policy::DestinationOrderKind::Type kind)
: dds::core::Value<detail::DestinationOrder>(DestinationOrder::to_native(kind))
{
}

DestinationOrder::DestinationOrder(
        const DestinationOrder& other)
: dds::core::Value<detail::DestinationOrder>(other)
{
}

void DestinationOrder::kind(
        dds::core::policy::DestinationOrderKind::Type kind)
{
    delegate().kind = DestinationOrder::to_native(kind);
}

dds::core::policy::DestinationOrderKind::Type DestinationOrder::kind() const
{
    return DestinationOrder::from_native(delegate().kind);
}

DestinationOrder DestinationOrder::SourceTimestamp()
{
    static DestinationOrder by_reception(DestinationOrderKind::BY_RECEPTION_TIMESTAMP);
    return by_reception;
}

DestinationOrder DestinationOrder::ReceptionTimestamp()
{
    static DestinationOrder by_source(DestinationOrderKind::BY_SOURCE_TIMESTAMP);
    return by_source;
}

eprosima::fastdds::dds::DestinationOrderQosPolicyKind DestinationOrder::to_native(
        DestinationOrderKind::Type kind)
{
    eprosima::fastdds::dds::DestinationOrderQosPolicyKind result;
    switch (kind)
    {
        case DestinationOrderKind::BY_RECEPTION_TIMESTAMP:
            result = eprosima::fastdds::dds::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
            break;
        case DestinationOrderKind::BY_SOURCE_TIMESTAMP:
            result = eprosima::fastdds::dds::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
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
        case eprosima::fastdds::dds::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS:
            result = DestinationOrderKind::BY_RECEPTION_TIMESTAMP;
            break;
        case eprosima::fastdds::dds::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS:
            result = DestinationOrderKind::BY_SOURCE_TIMESTAMP;
            break;
    }
    return result;
}


History::History(
        dds::core::policy::HistoryKind::Type kind,
        int32_t depth)
: dds::core::Value<detail::History>(History::to_native(kind), depth)
{
}

History::History(
        const History& other)
: dds::core::Value<detail::History>(other)
{
}

dds::core::policy::HistoryKind::Type History::kind() const
{
    return History::from_native(delegate().kind);
}

void History::kind(
        dds::core::policy::HistoryKind::Type kind)
{
    delegate().kind = History::to_native(kind);
}

int32_t History::depth() const
{
    return delegate().depth;
}

void History::depth(
        int32_t depth)
{
    delegate().depth = depth;
}

History History::KeepAll()
{
    static History keep_all(HistoryKind::KEEP_ALL);
    return keep_all;
}

History History::KeepLast(
        uint32_t depth)
{
    static History keep_last(HistoryKind::KEEP_LAST, depth);
    return keep_last;
}

eprosima::fastdds::dds::HistoryQosPolicyKind History::to_native(
        HistoryKind::Type kind)
{
    eprosima::fastdds::dds::HistoryQosPolicyKind result;
    switch (kind)
    {
        case HistoryKind::KEEP_ALL:
            result = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
            break;
        case HistoryKind::KEEP_LAST:
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
            result = HistoryKind::KEEP_LAST;
            break;
    }
    return result;
}

ResourceLimits::ResourceLimits(
        uint32_t max_samples,
        uint32_t max_instances,
        uint32_t max_samples_per_instance)
: dds::core::Value<detail::ResourceLimits>(max_samples, max_instances, max_samples_per_instance)
{
}

ResourceLimits::ResourceLimits(
        const ResourceLimits& other)
: dds::core::Value<detail::ResourceLimits>(other)
{
}

void ResourceLimits::max_samples(
        int32_t max_samples)
{
    delegate().max_samples = max_samples;
}

int32_t ResourceLimits::max_samples() const
{
    return delegate().max_samples;
}

void ResourceLimits::max_instances(
        int32_t max_instances)
{
    delegate().max_instances = max_instances;
}

int32_t ResourceLimits::max_instances() const
{
    return delegate().max_instances;
}

void ResourceLimits::max_samples_per_instance(
        int32_t max_samples_per_instance)
{
    delegate().max_samples_per_instance = max_samples_per_instance;
}

int32_t ResourceLimits::max_samples_per_instance() const
{
    return delegate().max_samples_per_instance;
}


Liveliness::Liveliness(
        dds::core::policy::LivelinessKind::Type kind,
        const dds::core::Duration& lease_duration)
: dds::core::Value<detail::Liveliness>(Liveliness::to_native(kind), lease_duration.delegate())
{
}

Liveliness::Liveliness(
        const Liveliness& other)
: dds::core::Value<detail::Liveliness>(other)
{
}

void Liveliness::kind(
        dds::core::policy::LivelinessKind::Type kind)
{
    delegate().kind = Liveliness::to_native(kind);
}

dds::core::policy::LivelinessKind::Type Liveliness::kind() const
{
    return Liveliness::from_native(delegate().kind);
}

void Liveliness::lease_duration(
        const dds::core::Duration& lease_duration)
{
    delegate().lease_duration = lease_duration.delegate();
}

const dds::core::Duration Liveliness::lease_duration() const
{
    return dds::core::Duration(delegate().lease_duration.seconds,
            delegate().lease_duration.nanosec);
}

Liveliness Liveliness::Automatic()
{
    static Liveliness automatic (LivelinessKind::AUTOMATIC);
    return automatic;
}

Liveliness Liveliness::ManualByParticipant(
        const dds::core::Duration& lease_duration)
{
    static Liveliness manual_by_participant (LivelinessKind::MANUAL_BY_PARTICIPANT, lease_duration);
    return manual_by_participant;
}

Liveliness Liveliness::ManualByTopic(
        const dds::core::Duration& lease_duration)
{
    static Liveliness manual_by_topic (LivelinessKind::MANUAL_BY_TOPIC, lease_duration);
    return manual_by_topic;
}

eprosima::fastdds::dds::LivelinessQosPolicyKind Liveliness::to_native(
        LivelinessKind::Type kind)
{
    eprosima::fastdds::dds::LivelinessQosPolicyKind result;
    switch (kind)
    {
        case LivelinessKind::AUTOMATIC:
            result = eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS;
            break;
        case LivelinessKind::MANUAL_BY_PARTICIPANT:
            result = eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
            break;
        case LivelinessKind::MANUAL_BY_TOPIC:
            result = eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS;
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
        case eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS:
            result = LivelinessKind::AUTOMATIC;
            break;
        case eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
            result = LivelinessKind::MANUAL_BY_PARTICIPANT;
            break;
        case eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS:
            result = LivelinessKind::MANUAL_BY_TOPIC;
            break;
    }
    return result;
}






DurabilityService::DurabilityService(
        const dds::core::Duration& service_cleanup_delay,
        dds::core::policy::HistoryKind::Type history_kind,
        int32_t history_depth,
        int32_t max_samples,
        int32_t max_instances,
        int32_t max_samples_per_instance)
: dds::core::Value<detail::DurabilityService>(
        service_cleanup_delay.delegate(),
        History::to_native(history_kind),
        history_depth,
        max_samples,
        max_instances,
        max_samples_per_instance)
{
}

DurabilityService::DurabilityService(
        const DurabilityService& other)
: dds::core::Value<detail::DurabilityService>(other)
{
}

void DurabilityService::service_cleanup_delay(
        const dds::core::Duration& service_cleanup_delay)
{
    delegate().service_cleanup_delay = service_cleanup_delay.delegate();
}

const dds::core::Duration DurabilityService::service_cleanup_delay() const
{
    return dds::core::Duration(delegate().service_cleanup_delay.seconds,
            delegate().service_cleanup_delay.nanosec);
}

void DurabilityService::history_kind(
        dds::core::policy::HistoryKind::Type history_kind)
{
    delegate().history_kind = History::to_native(history_kind);
}

dds::core::policy::HistoryKind::Type DurabilityService::history_kind() const
{
    return History::from_native(delegate().history_kind);
}

void DurabilityService::history_depth(
        int32_t history_depth)
{
    delegate().history_depth = history_depth;
}

int32_t DurabilityService::history_depth() const
{
    return delegate().history_depth;
}

void DurabilityService::max_samples(
        int32_t max_samples)
{
    delegate().max_samples = max_samples;
}

int32_t DurabilityService::max_samples() const
{
    return delegate().max_samples;
}

void DurabilityService::max_instances(
        int32_t max_instances)
{
    delegate().max_instances = max_instances;
}

int32_t DurabilityService::max_instances() const
{
    return delegate().max_instances;
}

void DurabilityService::max_samples_per_instance(
        int32_t max_samples_per_instance)
{
    delegate().max_samples_per_instance = max_samples_per_instance;
}

int32_t DurabilityService::max_samples_per_instance() const
{
    return delegate().max_samples_per_instance;
}


} //namespace policy
} //namespace core
} //namespace dds


