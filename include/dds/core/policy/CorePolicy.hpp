/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef OMG_DDS_CORE_POLICY_CORE_POLICY_HPP_
#define OMG_DDS_CORE_POLICY_CORE_POLICY_HPP_

#include <dds/core/policy/PolicyKind.hpp>
#include <dds/core/policy/detail/CorePolicy.hpp>
#include <dds/core/policy/QosPolicyCount.hpp>

#include <dds/core/detail/inttypes.hpp>
#include <dds/core/types.hpp>
#include <dds/core/LengthUnlimited.hpp>
#include <dds/core/detail/Value.hpp>
#include <dds/core/Duration.hpp>

//==============================================================================
// MACROS
//
#define OMG_DDS_POLICY_TRAITS(POLICY, ID) \
    template<> \
    class policy_id<POLICY> { \
public: \
        static const dds::core::policy::QosPolicyId value = ID; \
    }; \
    template<> \
    class policy_name<POLICY> { \
public: \
        static const std::string& name(); \
    };

#define OMG_DDS_DEFINE_POLICY_TRAITS(POLICY, NAME) \
    const std::string& dds::core::policy::policy_name<POLICY>::name() { \
        static std::string name = #NAME; \
        return name; \
    }


namespace dds {
namespace core {
namespace policy {

//==============================================================================
/**
 * \copydoc DCPS_QoS_UserData
 */
class UserData : public dds::core::Value<detail::UserData>
{
public:

    /**
     * Creates a UserData QoS instance with an empty UserData
     */
    UserData();

    /**
     * Creates a UserData QoS instance
     *
     * @param sequence the sequence of octets
     */
    explicit UserData(
            const dds::core::ByteSeq& sequence);

    /**
     * Creates a UserData QoS instance
     *
     * @param value_begin a pointer to the beginning of a sequence
     * of octets
     * @param value_end a pointer to the end of a sequence
     * of octets
     */
    UserData(
            const uint8_t* value_begin,
            const uint8_t* value_end);

    /**
     * Copies a UserData QoS instance
     *
     * @param other the UserData QoS instance to copy
     */
    UserData(
            const UserData& other);

    /**
     * Sets the sequence
     *
     * @param sequence a sequence of octets
     */
    UserData& value(
            const dds::core::ByteSeq& sequence);

    /**
     * Sets the sequence
     *
     * @param begin an iterator pointing to the beginning of a sequence
     * of octets
     * @param end an iterator pointing to the end of a sequence of octets
     */
    template<typename OCTET_ITER>
    UserData& value(
            OCTET_ITER begin,
            OCTET_ITER end);

    /**
     * Gets the sequence
     *
     * @return a sequence of octets
     */
    const dds::core::ByteSeq value() const;

    /**
     * Gets a pointer to the first octet in the sequence
     *
     * @return a pointer to the first octet in the sequence
     */
    const uint8_t* begin() const;

    /**
     * Gets a pointer to the last octet in the sequence
     *
     * @return a pointer to the first octet in the sequence
     */
    const uint8_t* end() const;
};

//==============================================================================

/**
 * \copydoc DCPS_QoS_GroupData
 */
class GroupData : public dds::core::Value<detail::GroupData>
{
public:

    /**
     * Creates a GroupData QoS instance
     */
    GroupData();

    /**
     * Creates a GroupData QoS instance
     *
     * @param sequence the sequence of octets representing the GroupData
     */
    explicit GroupData(
            const dds::core::ByteSeq& sequence);

    /**
     * Copies a GroupData QoS instance
     *
     * @param other the GroupData QoS instance to copy
     */
    GroupData(
            const GroupData& other);

    /**
     * Creates a GroupData QoS instance
     *
     * @param value_begin a pointer to the beginning of a sequence
     * of octets
     * @param value_end a pointer to the end of a sequence
     * of octets
     */
    GroupData(
            const uint8_t* value_begin,
            const uint8_t* value_end);

    /**
     * Set the sequence
     *
     * @param sequence a sequence of octets
     */
    GroupData& value(
            const dds::core::ByteSeq& sequence);

    /**
     * Set the sequence
     *
     * @param begin an iterator pointing to the beginning of a sequence
     * of octets
     * @param end an iterator pointing to the end of a sequence of octets
     */
    template<typename OCTET_ITER>
    GroupData& value(
            OCTET_ITER begin,
            OCTET_ITER end);

    /**
     * Get the sequence
     *
     * @return a sequence of octets
     */
    const dds::core::ByteSeq value() const;

    /**
     * Gets a pointer to the first octet in the sequence
     *
     * @return a pointer to the first octet in the sequence
     */
    const uint8_t* begin() const;

    /**
     * Gets a pointer to the last octet in the sequence
     *
     * @return a pointer to the last octet in the sequence
     */
    const uint8_t* end() const;
};

//==============================================================================

/**
 * \copydoc DCPS_QoS_TopicData
 */
class TopicData : public dds::core::Value<detail::TopicData>
{
public:

    /**
     * Creates a TopicData QoS instance
     */
    TopicData();

    /**
     * Creates a TopicData QoS instance
     *
     * @param sequence the sequence of octets representing the TopicData
     */
    explicit TopicData(
            const dds::core::ByteSeq& sequence);

    /**
     * Copies a TopicData QoS instance
     *
     * @param other the TopicData QoS instance to copy
     */
    TopicData(
            const TopicData& other);

    /**
     * Creates a TopicData QoS instance
     *
     * @param value_begin a pointer to the beginning of a sequence
     * of octets
     * @param value_end a pointer to the end of a sequence
     * of octets
     */
    TopicData(
            const uint8_t* value_begin,
            const uint8_t* value_end);

    /**
     * Set the sequence
     *
     * @param sequence a sequence of octets
     */
    TopicData& value(
            const dds::core::ByteSeq& sequence);

    /**
     * Set the sequence
     *
     * @param begin an iterator pointing to the beginning of a sequence
     * of octets
     * @param end an iterator pointing to the end of a sequence of octets
     */
    template<typename OCTET_ITER>
    TopicData& value(
            OCTET_ITER begin,
            OCTET_ITER end);

    /**
     * Get the sequence
     *
     * @return a sequence of octets
     */
    const dds::core::ByteSeq value() const;

    /**
     * Gets a pointer to the first octet in the sequence
     *
     * @return a pointer to the first octet in the sequence
     */
    const uint8_t* begin() const;

    /**
     * Gets a pointer to the last octet in the sequence
     *
     * @return a pointer to the last octet in the sequence
     */
    const uint8_t* end() const;
};


//==============================================================================

/**
 * \copydoc DCPS_QoS_EntityFactory
 */
class EntityFactory : public dds::core::Value<detail::EntityFactory>
{
public:

    /**
     * Creates an EntityFactory QoS instance
     *
     * @param autoenable_created_entities boolean indicating whether
     * created Entities should be automatically enabled
     */
    explicit EntityFactory(
            bool autoenable_created_entities = true);

    /**
     * Copies an EntityFactory QoS instance
     *
     * @param other the EntityFactory QoS instance to copy
     */
    EntityFactory(
            const EntityFactory& other);

    /**
     * Sets a boolean indicating whether created Entities should be
     * automatically enabled
     *
     * @param autoenable_created_entities boolean indicating whether
     * created Entities should be automatically enabled
     */
    EntityFactory& autoenable_created_entities(
            bool autoenable_created_entities);

    /**
     * Gets a boolean indicating whether Entities should be automatically enabled
     *
     * @return boolean indicating whether created Entities should be automatically
     * enabled
     */
    bool autoenable_created_entities() const;

    /**
     * @return an EntityFactory QoS instance with autoenable_created_entities
     * set to true
     */
    static EntityFactory AutoEnable();

    /**
     * @return an EntityFactory QoS instance with autoenable_created_entities
     * set to false
     */
    static EntityFactory ManuallyEnable();
};

//==============================================================================

/**
 * \copydoc DCPS_QoS_TransportPriority
 */
class TransportPriority : public dds::core::Value<detail::TransportPriority>
{
public:

    /**
     * Creates a TransportPriority QoS instance
     *
     * @param priority the priority value
     */
    explicit TransportPriority(
            int32_t priority = 0);

    /**
     * Copies a TransportPriority QoS instance
     *
     * @param other the TransportPriority QoS instance to copy
     */
    TransportPriority(
            const TransportPriority& other);

    /**
     * Sets the priority value
     *
     * @param priority the priority value
     */
    TransportPriority& value(
            int32_t priority);

    /**
     * Gets the priority value
     *
     * @return the priority value
     */
    int32_t value() const;
};

//==============================================================================

/**
 * \copydoc DCPS_QoS_Lifespan
 */
class Lifespan : public dds::core::Value<detail::Lifespan>
{
public:

    /**
     * Creates a Lifespan QoS instance
     *
     * @param duration Lifespan expiration duration
     */
    explicit Lifespan(
            const dds::core::Duration& duration = dds::core::Duration::infinite());

    /**
     * Copies a Lifespan QoS instance
     *
     * @param other the Lifespan QoS instance to copy
     */
    Lifespan(
            const Lifespan& other);

    /**
     * Sets the expiration duration
     *
     * @param duration expiration duration
     */
    Lifespan& duration(
            const dds::core::Duration& duration);

    /**
     * Gets the expiration duration
     *
     * @return expiration duration
     */
    const dds::core::Duration duration() const;
};

//==============================================================================

/**
 * \copydoc DCPS_QoS_Deadline
 */
class Deadline : public dds::core::Value<detail::Deadline>
{
public:

    /**
     * Creates a Deadline QoS instance
     *
     * @param period deadline period
     */
    explicit Deadline(
            const dds::core::Duration& period = dds::core::Duration::infinite());

    /**
     * Copies a Deadline QoS instance
     *
     * @param other the Deadline QoS instance to copy
     */
    Deadline(
            const Deadline& other);

    /**
     * Sets the deadline period
     *
     * @param period deadline period
     */
    Deadline& period(
            const dds::core::Duration& period);

    /**
     * Gets the deadline period
     *
     * @return deadline period
     */
    const dds::core::Duration period() const;
};

//==============================================================================

/**
 * \copydoc DCPS_QoS_LatencyBudget
 */
class LatencyBudget : public dds::core::Value<detail::LatencyBudget>
{
public:

    /**
     * Creates a LatencyBudget QoS instance
     *
     * @param duration duration
     */
    explicit LatencyBudget(
            const dds::core::Duration& duration = dds::core::Duration::zero());

    /**
     * Copies a LatencyBudget QoS instance
     *
     * @param other the LatencyBudget QoS instance to copy
     */
    LatencyBudget(
            const LatencyBudget& other);

    /**
     * Sets the duration
     *
     * @param duration duration
     */
    LatencyBudget& duration(
            const dds::core::Duration& duration);

    /**
     * Gets the duration
     *
     * @return duration
     */
    const dds::core::Duration duration() const;
};

//==============================================================================

/**
 * \copydoc DCPS_QoS_TimeBasedFilter
 */
class TimeBasedFilter : public dds::core::Value<detail::TimeBasedFilter>
{
public:

    /**
     * Creates a TimeBasedFilter QoS instance
     *
     * @param period minimum separation period
     */
    explicit TimeBasedFilter(
            const dds::core::Duration& period = dds::core::Duration::zero());

    /**
     * Copies a TimeBasedFilter QoS instance
     *
     * @param other the TimeBasedFilter QoS instance to copy
     */
    TimeBasedFilter(
            const TimeBasedFilter& other);

    /**
     * Sets the minimum separation period
     *
     * @param period minimum separation period
     */
    TimeBasedFilter& minimum_separation(
            const dds::core::Duration& period);

    /**
     * Gets the minimum separation period
     *
     * @return minimum separation period
     */
    const dds::core::Duration minimum_separation() const;
};


//==============================================================================

/**
 * \copydoc DCPS_QoS_Partition
 */
class Partition : public dds::core::Value<detail::Partition>
{
public:

    /**
     * Creates a Partition QoS instance
     *
     * @param name partition name
     */
    explicit Partition(
            const std::string& name = "");

    /**
     * Creates a Partition QoS instance
     *
     * @param names a sequence containing multiple partition names
     */
    explicit Partition(
            const dds::core::StringSeq& names);

    /**
     * Copies a Partition QoS instance
     *
     * @param other the Partition QoS instance to copy
     */
    Partition(
            const Partition& other);

    /**
     * Sets the partition name
     *
     * @param name the partition name
     */
    Partition& name(
            const std::string& name);

    /**
     * Sets multiple partition names
     *
     * @param names a sequence containing multiple partition names
     */
    Partition& name(
            const dds::core::StringSeq& names);

    /**
     * Gets the partition names
     *
     * @return a sequence containing the partition names
     */
    const dds::core::StringSeq name() const;
};

//==============================================================================
//#ifdef OMG_DDS_OWNERSHIP_SUPPORT

/**
 * \copydoc DCPS_QoS_Ownership
 */
class Ownership : public dds::core::Value<detail::Ownership>
{
public:

    #   if defined (__SUNPRO_CC) && defined(SHARED)
    #   undef SHARED
    #   endif
    /**
     * Creates an Ownership QoS instance
     *
     * @param kind the kind
     */
    explicit Ownership(
            dds::core::policy::OwnershipKind::Type kind = dds::core::policy::OwnershipKind::SHARED);

    /**
     * Copies an Ownership QoS instance
     *
     * @param other the Ownership QoS instance to copy
     */
    Ownership(
            const Ownership& other);

    /**
     * Set the kind
     *
     * @param kind the kind
     */
    Ownership& kind(
            dds::core::policy::OwnershipKind::Type kind);

    /**
     * Get the kind
     *
     * @param kind the kind
     */
    dds::core::policy::OwnershipKind::Type kind() const;

    /**
     * @return an Ownership QoS instance with the kind set to EXCLUSIVE
     */
    static Ownership Exclusive();

    /**
     * @return an Ownership QoS instance with the kind set to SHARED
     */
    static Ownership Shared();

private:

    static eprosima::fastdds::dds::OwnershipQosPolicyKind to_native(
            OwnershipKind::Type kind);

    static OwnershipKind::Type from_native(
            eprosima::fastdds::dds::OwnershipQosPolicyKind kind);
};


//==============================================================================

/**
 * \copydoc DCPS_QoS_OwnershipStrength
 */
class OwnershipStrength : public dds::core::Value<detail::OwnershipStrength>
{
public:

    /**
     * Creates an OwnershipStrength QoS instance
     *
     * @param strength ownership strength
     */
    explicit OwnershipStrength(
            int32_t strength = 0);

    /**
     * Copies an OwnershipStrength QoS instance
     *
     * @param other the OwnershipStrength QoS instance to copy
     */
    OwnershipStrength(
            const OwnershipStrength& other);

    /**
     * Gets the ownership strength value
     *
     * @return the ownership strength value
     */
    int32_t value() const;

    /**
     * Sets the ownership strength value
     *
     * @param strength the ownership strength value
     */
    OwnershipStrength& value(
            int32_t strength);
};

//#endif  // OMG_DDS_OWNERSHIP_SUPPORT
//==============================================================================

/**
 * \copydoc DCPS_QoS_WriterDataLifecycle
 */
class WriterDataLifecycle : public dds::core::Value<detail::WriterDataLifecycle>
{
public:

    /**
     * Creates a WriterDataLifecycle QoS instance
     *
     * @param autodispose_unregistered_instances Specifies the behavior of the DataWriter
     * with regards to the lifecycle of the data-instances it manages.
     */
    explicit WriterDataLifecycle(
            bool autodispose_unregistered_instances = true);

    /**
     * Copies a WriterDataLifecycle QoS instance
     *
     * @param other the WriterDataLifecycle QoS instance to copy
     */
    WriterDataLifecycle(
            const WriterDataLifecycle& other);

    /**
     * Gets a boolean indicating if unregistered instances should be autodisposed
     *
     * @return a boolean indicating if unregistered instances should be autodisposed
     */
    bool autodispose_unregistered_instances() const;

    /**
     * Sets a boolean indicating if unregistered instances should be autodisposed
     *
     * @param autodispose_unregistered_instances a boolean indicating if unregistered
     * instances should be autodisposed
     */
    WriterDataLifecycle& autodispose_unregistered_instances(
            bool autodispose_unregistered_instances);

    /**
     * @return a WriterDataLifecycle QoS instance with autodispose_unregistered_instances
     * set to true
     */
    static WriterDataLifecycle AutoDisposeUnregisteredInstances();

    /**
     * @return a WriterDataLifecycle QoS instance with autodispose_unregistered_instances
     * set to false
     */
    static WriterDataLifecycle ManuallyDisposeUnregisteredInstances();

};

//==============================================================================

/**
 * \copydoc DCPS_QoS_ReaderDataLifecycle
 */
class ReaderDataLifecycle : public dds::core::Value<detail::ReaderDataLifecycle>
{
public:

    /**
     * Creates a ReaderDataLifecycle QoS instance
     *
     * @param autopurge_nowriter_samples_delay the autopurge nowriter samples delay
     * @param autopurge_disposed_samples_delay the autopurge disposed samples delay
     */
    ReaderDataLifecycle(
            const dds::core::Duration& autopurge_nowriter_samples_delay = dds::core::Duration::infinite(),
            const dds::core::Duration& autopurge_disposed_samples_delay = dds::core::Duration::infinite());

    /**
     * Copies a ReaderDataLifecycle QoS instance
     *
     * @param other the ReaderDataLifecycle QoS instance to copy
     */
    ReaderDataLifecycle(
            const ReaderDataLifecycle& other);
    /**
     * Gets the autopurge nowriter samples delay
     *
     * @return the autopurge nowriter samples delay
     */
    const dds::core::Duration autopurge_nowriter_samples_delay() const;

    /**
     * Sets the autopurge nowriter samples delay
     *
     * @param autopurge_nowriter_samples_delay the autopurge nowriter samples delay
     */
    ReaderDataLifecycle& autopurge_nowriter_samples_delay(
            const dds::core::Duration& autopurge_nowriter_samples_delay);

    /**
     * Gets the autopurge_disposed_samples_delay
     *
     * @return the autopurge disposed samples delay
     */
    const dds::core::Duration autopurge_disposed_samples_delay() const;

    /**
     * Sets the autopurge_disposed_samples_delay
     *
     * @return the autopurge disposed samples delay
     */
    ReaderDataLifecycle& autopurge_disposed_samples_delay(
            const dds::core::Duration& autopurge_disposed_samples_delay);

    /**
     * @return a ReaderDataLifecycle QoS instance which will not autopurge disposed
     * samples
     */
    static ReaderDataLifecycle NoAutoPurgeDisposedSamples();

    /**
     * @param autopurge_disposed_samples_delay the autopurge disposed samples delay
     * @return a ReaderDataLifecycle QoS instance with autopurge_disposed_samples_delay
     * set to a specified value
     */
    static ReaderDataLifecycle AutoPurgeDisposedSamples(
            const dds::core::Duration& autopurge_disposed_samples_delay);

};

//==============================================================================

/**
 * \copydoc DCPS_QoS_Durability
 */
class Durability : public dds::core::Value<detail::Durability>
{
public:

    /**
     * Creates a Durability QoS instance
     *
     * @param kind the kind
     */
    explicit Durability(
            dds::core::policy::DurabilityKind::Type kind = dds::core::policy::DurabilityKind::VOLATILE);

    /**
     * Copies a Durability QoS instance
     *
     * @param other the Durability QoS instance to copy
     */
    Durability(
            const Durability& other);

    /**
     * Set the kind
     *
     * @param kind the kind
     */
    Durability& kind(
            dds::core::policy::DurabilityKind::Type kind);

    /**
     * Get the kind
     *
     * @param kind the kind
     */
    dds::core::policy::DurabilityKind::Type  kind() const;

    /**
     * @return a Durability QoS instance with the kind set to VOLATILE
     */
    static Durability Volatile();

    /**
     * @return a Durability QoS instance with the kind set to TRANSIENT_LOCAL
     */
    static Durability TransientLocal();

    /**
     * @return a Durability QoS instance with the kind set to TRANSIENT
     */
    static Durability Transient();

    /**
     * @return a Durability QoS instance with the kind set to PERSISTENT
     */
    static Durability Persistent();

private:

    static eprosima::fastdds::dds::DurabilityQosPolicyKind to_native(
            DurabilityKind::Type kind);

    static DurabilityKind::Type from_native(
            eprosima::fastdds::dds::DurabilityQosPolicyKind kind);

};

//==============================================================================

/**
 * \copydoc DCPS_QoS_Presentation
 */
class Presentation : public dds::core::Value<detail::Presentation>
{
public:

    /**
     * Creates a Presentation QoS instance
     *
     * @param access_scope the access_scope kind
     * @param coherent_access the coherent_access setting
     * @param ordered_access the ordered_access setting
     */
    Presentation(
            dds::core::policy::PresentationAccessScopeKind::Type access_scope
                = dds::core::policy::PresentationAccessScopeKind::INSTANCE,
            bool coherent_access = false,
            bool ordered_access = false);

    /**
     * Copies a Presentation QoS instance
     *
     * @param other the Presentation QoS instance to copy
     */
    Presentation(
            const Presentation& other);

    /**
     * Sets the access_scope kind
     *
     * @param access_scope the access_scope kind
     */
    Presentation& access_scope(
            dds::core::policy::PresentationAccessScopeKind::Type access_scope);

    /**
     * Gets the access_scope kind
     *
     * @return the access_scope kind
     */
    dds::core::policy::PresentationAccessScopeKind::Type access_scope() const;

    /**
     * Sets the coherent_access setting
     *
     * @param coherent_access the coherent_access setting
     */
    Presentation& coherent_access(
            bool coherent_access);

    /**
     * Gets the coherent_access setting
     *
     * @return the coherent_access setting
     */
    bool coherent_access() const;

    /**
     * Sets the ordered_access setting
     *
     * @param ordered_access the ordered_access setting
     */
    Presentation& ordered_access(
            bool ordered_access);

    /**
     * Gets the ordered_access setting
     *
     * @return the ordered_access setting
     */
    bool ordered_access() const;

    /**
     * @param coherent_access the coherent_access setting
     * @param ordered_access the ordered_access setting
     *
     * @return a Presentation QoS instance with a GROUP access_score and coherent_access
     * and ordered_access set to the specified values
     */
    static Presentation GroupAccessScope(
            bool coherent_access = false,
            bool ordered_access = false);

    /**
     * @param coherent_access the coherent_access setting
     * @param ordered_access the ordered_access setting
     *
     * @return a Presentation QoS instance with a INSTANCE access_score and coherent_access
     * and ordered_access set to the specified values
     */
    static Presentation InstanceAccessScope(
            bool coherent_access = false,
            bool ordered_access = false);

    /**
     * @param coherent_access the coherent_access setting
     * @param ordered_access the ordered_access setting
     *
     * @return a Presentation QoS instance with a TOPIC access_score and coherent_access
     * and ordered_access set to the specified values
     */
    static Presentation TopicAccessScope(
            bool coherent_access = false,
            bool ordered_access = false);

private:

    static eprosima::fastdds::dds::PresentationQosPolicyAccessScopeKind to_native(
            PresentationAccessScopeKind::Type kind);

    static PresentationAccessScopeKind::Type from_native(
            eprosima::fastdds::dds::PresentationQosPolicyAccessScopeKind kind);
};

//==============================================================================

/**
 * \copydoc DCPS_QoS_Reliability
 */
class Reliability : public dds::core::Value<detail::Reliability>
{
public:

    /**
     * Creates a Reliability QoS instance
     *
     * @param kind the kind
     * @param max_blocking_time the max_blocking_time
     */
    Reliability(
            dds::core::policy::ReliabilityKind::Type kind = dds::core::policy::ReliabilityKind::BEST_EFFORT,
            const dds::core::Duration& max_blocking_time = dds::core::Duration::from_millisecs(100));

    /**
     * Copies a Reliability QoS instance
     *
     * @param other the Reliability QoS instance to copy
     */
    Reliability(
            const Reliability& other);

    /**
     * Sets the kind
     *
     * @param kind the kind
     */
    Reliability& kind(
            dds::core::policy::ReliabilityKind::Type kind);

    /**
     * Gets the kind
     *
     * @return the kind
     */
    dds::core::policy::ReliabilityKind::Type kind() const;

    /**
     * Sets the max_blocking_time
     *
     * @param max_blocking_time the max_blocking_time
     */
    Reliability& max_blocking_time(
            const dds::core::Duration& max_blocking_time);

    /**
     * Gets the max_blocking_time
     *
     * @return the max_blocking_time
     */
    const dds::core::Duration max_blocking_time() const;

    /**
     * @param the max_blocking_time
     * @return a Reliability QoS instance with the kind set to RELIABLE and the max_blocking_time
     * set to the supplied value
     */
    static OMG_DDS_API Reliability Reliable(
            const dds::core::Duration& max_blocking_time = dds::core::Duration::from_millisecs(100));

    /**
     * @return a Reliability QoS instance with the kind set to BEST_EFFORT
     */
    static Reliability BestEffort(
            const dds::core::Duration& max_blocking_time = dds::core::Duration::from_millisecs(100));

private:

    static ReliabilityKind::Type from_native(
            eprosima::fastdds::dds::ReliabilityQosPolicyKind kind);

    static eprosima::fastdds::dds::ReliabilityQosPolicyKind to_native(
            ReliabilityKind::Type kind);

};

//==============================================================================

/**
 * \copydoc DCPS_QoS_DestinationOrder
 */
class DestinationOrder : public dds::core::Value<detail::DestinationOrder>
{
public:

    /**
     * Creates a DestinationOrder QoS instance
     *
     * @param kind the kind
     */
    explicit DestinationOrder(
            dds::core::policy::DestinationOrderKind::Type kind
                = dds::core::policy::DestinationOrderKind::BY_RECEPTION_TIMESTAMP);

    /**
     * Copies a DestinationOrder QoS instance
     *
     * @param other the Reliability QoS instance to copy
     */
    DestinationOrder(
            const DestinationOrder& other);

    /**
     * Sets the kind
     *
     * @param kind the kind
     */
    DestinationOrder& kind(
            dds::core::policy::DestinationOrderKind::Type kind);

    /**
     * Gets the kind
     *
     * @return the kind
     */
    dds::core::policy::DestinationOrderKind::Type kind() const;

    /**
     * @return a DestinationOrder QoS instance with the kind set to BY_SOURCE_TIMESTAMP
     */
    static DestinationOrder SourceTimestamp();

    /**
     * @return a DestinationOrder QoS instance with the kind set to BY_RECEPTION_TIMESTAMP
     */
    static DestinationOrder ReceptionTimestamp();

private:

    static eprosima::fastdds::dds::DestinationOrderQosPolicyKind to_native(
            DestinationOrderKind::Type kind);

    static DestinationOrderKind::Type from_native(
            eprosima::fastdds::dds::DestinationOrderQosPolicyKind kind);

};

//==============================================================================

/**
 * \copydoc DCPS_QoS_History
 */
class History : public dds::core::Value<detail::History>
{
public:

    /**
     * Creates a History QoS instance
     *
     * @param kind the kind
     * @param depth the history depth
     */
    History(
            dds::core::policy::HistoryKind::Type kind = dds::core::policy::HistoryKind::KEEP_LAST,
            int32_t depth = 1);

    /**
     * Copies a History QoS instance
     *
     * @param other the History QoS instance to copy
     */
    History(
            const History& other);

    /**
     * Gets the kind
     *
     * @return the kind
     */
    dds::core::policy::HistoryKind::Type kind() const;

    /**
     * Sets the kind
     *
     * @param kind the kind
     */
    History& kind(
            dds::core::policy::HistoryKind::Type kind);

    /**
     * Gets the history depth
     *
     * @return the history depth
     */
    int32_t depth() const;

    /**
     * Sets the history depth
     *
     * @param the history depth
     */
    History& depth(
            int32_t depth);

    /**
     * @return a History QoS instance with the kind set to KEEP_ALL
     */
    static History KeepAll();

    /**
     * @param depth the history depth
     * @return a History QoS instance with the kind set to KEEP_LAST and the
     * depth set to the supplied value
     */
    static History KeepLast(
            uint32_t depth);

private:

    friend class DurabilityService;

    static eprosima::fastdds::dds::HistoryQosPolicyKind to_native(
            HistoryKind::Type kind);

    static HistoryKind::Type from_native(
            eprosima::fastdds::dds::HistoryQosPolicyKind kind);

};

//==============================================================================

/**
 * \copydoc DCPS_QoS_ResourceLimits
 */
class ResourceLimits : public dds::core::Value<detail::ResourceLimits>
{
public:

    /**
     * Creates a ResourceLimits QoS instance
     *
     * @param max_samples the max_samples value
     * @param max_instances the max_instances value
     * @param max_samples_per_instance the max_samples_per_instance value
     */
    ResourceLimits(
            int32_t max_samples = dds::core::LENGTH_UNLIMITED,
            int32_t max_instances = dds::core::LENGTH_UNLIMITED,
            int32_t max_samples_per_instance = dds::core::LENGTH_UNLIMITED);

    /**
     * Copies a ResourceLimits QoS instance
     *
     * @param other the ResourceLimits QoS instance to copy
     */
    ResourceLimits(
            const ResourceLimits& other);

public:

    /**
     * Sets the max_samples value
     *
     * @param max_samples the max_samples value
     */
    ResourceLimits& max_samples(
            int32_t max_samples);

    /**
     * Gets the max_samples value
     *
     * @return the max_samples value
     */
    int32_t max_samples() const;

    /**
     * Sets the max_instances value
     *
     * @param max_instances the max_instances value
     */
    ResourceLimits& max_instances(
            int32_t max_instances);

    /**
     * Gets the max_instances value
     *
     * @return the max_instances value
     */
    int32_t max_instances() const;

    /**
     * Sets the max_samples_per_instance value
     *
     * @param max_samples_per_instance the max_samples_per_instance value
     */
    ResourceLimits& max_samples_per_instance(
            int32_t max_samples_per_instance);

    /**
     * Gets the max_samples_per_instance value
     *
     * @return the max_samples_per_instance value
     */
    int32_t max_samples_per_instance() const;
};

//==============================================================================

/**
 * \copydoc DCPS_QoS_Liveliness
 */
class Liveliness : public dds::core::Value<detail::Liveliness>
{
public:

    /**
     * Creates a Liveliness QoS instance
     *
     * @param kind the kind
     * @param lease_duration the lease_duration
     */
    Liveliness(
            dds::core::policy::LivelinessKind::Type kind = dds::core::policy::LivelinessKind::AUTOMATIC,
            const dds::core::Duration& lease_duration = dds::core::Duration::infinite());

    /**
     * Copies a Liveliness QoS instance
     *
     * @param other the Liveliness QoS instance to copy
     */
    Liveliness(
            const Liveliness& other);

    /**
     * Sets the kind
     *
     * @param kind the kind
     */
    Liveliness& kind(
            dds::core::policy::LivelinessKind::Type kind);

    /**
     * Gets the kind
     *
     * @return the kind
     */
    dds::core::policy::LivelinessKind::Type kind() const;

    /**
     * Sets the lease_duration
     *
     * @return the lease_duration
     */
    Liveliness& lease_duration(
            const dds::core::Duration& lease_duration);

    /**
     * Gets the lease_duration
     *
     * @return the lease_duration
     */
    const dds::core::Duration lease_duration() const;

    /**
     * @return a Liveliness QoS instance with the kind set to AUTOMATIC
     */
    static Liveliness Automatic();

    /**
     * @return a Liveliness QoS instance with the kind set to MANUAL_BY_PARTICIPANT
     * and the lease_duration set to the supplied value
     */
    static Liveliness ManualByParticipant(
            const dds::core::Duration& lease_duration = dds::core::Duration::infinite());

    /**
     * @return a Liveliness QoS instance with the kind set to MANUAL_BY_TOPIC
     * and the lease_duration set to the supplied value
     */
    static Liveliness ManualByTopic(
            const dds::core::Duration& lease_duration = dds::core::Duration::infinite());

private:

    static eprosima::fastdds::dds::LivelinessQosPolicyKind to_native(
            LivelinessKind::Type kind);

    static LivelinessKind::Type from_native(
            eprosima::fastdds::dds::LivelinessQosPolicyKind kind);

};


//==============================================================================
//#ifdef OMG_DDS_PERSISTENCE_SUPPORT

/**
 * \copydoc DCPS_QoS_DurabilityService
 */
class DurabilityService : public dds::core::Value<detail::DurabilityService>
{
public:

    /**
     * Creates a DurabilityService QoS instance
     *
     * @param service_cleanup_delay the service_cleanup_delay
     * @param history_kind the history_kind value
     * @param history_depth the history_depth value
     * @param max_samples the max_samples value
     * @param max_instances the max_instances value
     * @param max_samples_per_instance the max_samples_per_instance value
     */
    DurabilityService(
            const dds::core::Duration& service_cleanup_delay = dds::core::Duration::zero(),
            dds::core::policy::HistoryKind::Type history_kind = dds::core::policy::HistoryKind::KEEP_LAST,
            int32_t history_depth = 1,
            int32_t max_samples = dds::core::LENGTH_UNLIMITED,
            int32_t max_instances = dds::core::LENGTH_UNLIMITED,
            int32_t max_samples_per_instance = dds::core::LENGTH_UNLIMITED);

    /**
     * Copies a DurabilityService QoS instance
     *
     * @param other the DurabilityService QoS instance to copy
     */
    DurabilityService(
            const DurabilityService& other);

    /**
     * Sets the service_cleanup_delay value
     *
     * @param service_cleanup_delay the service_cleanup_delay value
     */
    DurabilityService& service_cleanup_delay(
            const dds::core::Duration& service_cleanup_delay);

    /**
     * Gets the service_cleanup_delay value
     *
     * @return the service_cleanup_delay
     */
    const dds::core::Duration service_cleanup_delay() const;

    /**
     * Sets the history_kind
     *
     * @param the history_kind
     */
    DurabilityService& history_kind(
            dds::core::policy::HistoryKind::Type history_kind);

    /**
     * Gets the history_kind
     *
     * @return history_kind
     */
    dds::core::policy::HistoryKind::Type history_kind() const;

    /**
     * Sets the history_depth value
     *
     * @param history_depth the history_depth value
     */
    DurabilityService& history_depth(
            int32_t history_depth);

    /**
     * Gets the history_depth value
     *
     * @return history_depth
     */
    int32_t history_depth() const;

    /**
     * Sets the max_samples value
     *
     * @param max_samples the max_samples value
     */
    DurabilityService& max_samples(
            int32_t max_samples);

    /**
     * Gets the max_samples value
     *
     * @return the max_samples value
     */
    int32_t max_samples() const;

    /**
     * Sets the max_instances value
     *
     * @param max_instances the max_instances value
     */
    DurabilityService& max_instances(
            int32_t max_instances);

    /** Gets the max_instances value
     *
     * @return the max_instances value
     */
    int32_t max_instances() const;

    /**
     * Sets the max_samples_per_instance value
     *
     * @param max_samples_per_instance the max_samples_per_instance value
     */
    DurabilityService& max_samples_per_instance(
            int32_t max_samples_per_instance);

    /**
     * Gets the max_samples_per_instance value
     *
     * @return the max_samples_per_instance value
     */
    int32_t max_samples_per_instance() const;
};

//#endif  //OMG_DDS_PERSISTENCE_SUPPORT

//==============================================================================

//#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

typedef uint16_t DataRepresentationId;

typedef std::vector<DataRepresentationId> DataRepresentationIdSeq;

class DataRepresentation : public dds::core::Value<detail::DataRepresentation>
{

public:

    explicit DataRepresentation(
            const dds::core::policy::DataRepresentationIdSeq& value);

    DataRepresentation(
            const DataRepresentation& other)
        : dds::core::Value<detail::DataRepresentation>(other)
    {
    }

    DataRepresentation& value(
            const dds::core::policy::DataRepresentationIdSeq& value);

    const dds::core::policy::DataRepresentationIdSeq value() const;

    dds::core::policy::DataRepresentationIdSeq& value();

private:

    static std::vector<eprosima::fastdds::dds::DataRepresentationId> to_native(
            const DataRepresentationIdSeq& seq);

};

//#endif  //OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT


//============================================================================

//#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

class TypeConsistencyEnforcement : public dds::core::Value<detail::TypeConsistencyEnforcement>
{
public:

    explicit TypeConsistencyEnforcement(
            dds::core::policy::TypeConsistencyEnforcementKind::Type kind);

    TypeConsistencyEnforcement& kind(
            dds::core::policy::TypeConsistencyEnforcementKind::Type kind);

    dds::core::policy::TypeConsistencyEnforcementKind::Type  kind() const;

    TypeConsistencyEnforcement& ignore_sequence_bounds(
            bool ignore_sequence_bounds);

    TypeConsistencyEnforcement& ignore_string_bounds(
            bool ignore_string_bounds);

    TypeConsistencyEnforcement& ignore_member_names(
            bool ignore_member_names);

    TypeConsistencyEnforcement& prevent_type_widening(
            bool prevent_type_widening);

    TypeConsistencyEnforcement& force_type_validation(
            bool force_type_validation);

    bool ignore_sequence_bounds();

    bool ignore_string_bounds();

    bool ignore_member_names();

    bool prevent_type_widening();

    bool force_type_validation();

};

//#endif  //OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

//==============================================================================
// Policy Trait Classes

template<typename Policy>
class policy_id;

template<typename Policy>
class policy_name;

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
/** @todo - Known issue. */
#endif

OMG_DDS_POLICY_TRAITS(UserData,             1)
OMG_DDS_POLICY_TRAITS(Durability,           2)
OMG_DDS_POLICY_TRAITS(Presentation,         3)
OMG_DDS_POLICY_TRAITS(Deadline,             4)
OMG_DDS_POLICY_TRAITS(LatencyBudget,        5)
OMG_DDS_POLICY_TRAITS(Ownership,            6)

//#ifdef OMG_DDS_OWNERSHIP_SUPPORT
OMG_DDS_POLICY_TRAITS(OwnershipStrength,    7)
//#endif  //OMG_DDS_OWNERSHIP_SUPPORT

OMG_DDS_POLICY_TRAITS(Liveliness,           8)
OMG_DDS_POLICY_TRAITS(TimeBasedFilter,      9)
OMG_DDS_POLICY_TRAITS(Partition,            10)
OMG_DDS_POLICY_TRAITS(Reliability,          11)
OMG_DDS_POLICY_TRAITS(DestinationOrder,     12)
OMG_DDS_POLICY_TRAITS(History,              13)
OMG_DDS_POLICY_TRAITS(ResourceLimits,       14)
OMG_DDS_POLICY_TRAITS(EntityFactory,        15)
OMG_DDS_POLICY_TRAITS(WriterDataLifecycle,  16)
OMG_DDS_POLICY_TRAITS(ReaderDataLifecycle,  17)
OMG_DDS_POLICY_TRAITS(TopicData,            18)
OMG_DDS_POLICY_TRAITS(GroupData,            19)
OMG_DDS_POLICY_TRAITS(TransportPriority,    20)
OMG_DDS_POLICY_TRAITS(Lifespan,             21)

//#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
OMG_DDS_POLICY_TRAITS(DurabilityService,    22)
//#endif  //OMG_DDS_PERSISTENCE_SUPPORT


} //namespace policy
} //namespace core
} //namespace dds


#endif //OMG_DDS_CORE_POLICY_CORE_POLICY_HPP_
