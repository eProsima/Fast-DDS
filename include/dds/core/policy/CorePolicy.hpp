/* Copyright 2010, Object Management Group, Inc.
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
#include <dds/core/policy/detail/QosPolicyCount.hpp>

#include <dds/core/detail/inttypes.hpp>
#include <dds/core/types.hpp>
#include <dds/core/LengthUnlimited.hpp>
#include <dds/core/detail/Value.hpp>
#include <dds/core/Duration.hpp>

//==============================================================================
// MACROS
//
#define OMG_DDS_POLICY_TRAITS(POLICY, ID) \
    template <> \
    class policy_id<POLICY> { \
    public: \
        static const dds::core::policy::QosPolicyId value = ID; \
    }; \
    template <> \
    class policy_name<POLICY> { \
    public:\
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
template<typename D>
class TUserData : public dds::core::Value<D>
{
public:
    /**
     * Creates a UserData QoS instance with an empty UserData
     */
    TUserData();

    /**
     * Creates a UserData QoS instance
     *
     * @param sequence the sequence of octets
     */
    explicit TUserData(
            const dds::core::ByteSeq& sequence);

    /**
     * Creates a UserData QoS instance
     *
     * @param value_begin a pointer to the beginning of a sequence
     * of octets
     * @param value_end a pointer to the end of a sequence
     * of octets
     */
    TUserData(
            const uint8_t* value_begin,
            const uint8_t* value_end);

    /**
     * Copies a UserData QoS instance
     *
     * @param other the UserData QoS instance to copy
     */
    TUserData(
            const TUserData& other);

    /**
     * Sets the sequence
     *
     * @param sequence a sequence of octets
     */
    TUserData& value(
            const dds::core::ByteSeq& sequence);

    /**
     * Sets the sequence
     *
     * @param begin an iterator pointing to the beginning of a sequence
     * of octets
     * @param end an iterator pointing to the end of a sequence of octets
     */
    template<typename OCTET_ITER>
    TUserData& value(
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
template<typename D>
class TGroupData : public dds::core::Value<D>
{
public:
    /**
     * Creates a GroupData QoS instance
     */
    TGroupData();

    /**
     * Creates a GroupData QoS instance
     *
     * @param sequence the sequence of octets representing the GroupData
     */
    explicit TGroupData(
            const dds::core::ByteSeq& sequence);

    /**
     * Copies a GroupData QoS instance
     *
     * @param other the GroupData QoS instance to copy
     */
    TGroupData(const TGroupData& other);

    /**
     * Creates a GroupData QoS instance
     *
     * @param value_begin a pointer to the beginning of a sequence
     * of octets
     * @param value_end a pointer to the end of a sequence
     * of octets
     */
    TGroupData(
            const uint8_t* value_begin,
            const uint8_t* value_end);

    /**
     * Set the sequence
     *
     * @param sequence a sequence of octets
     */
    TGroupData& value(
            const dds::core::ByteSeq& sequence);

    /**
     * Set the sequence
     *
     * @param begin an iterator pointing to the beginning of a sequence
     * of octets
     * @param end an iterator pointing to the end of a sequence of octets
     */
    template<typename OCTET_ITER>
    TGroupData& value(
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
template<typename D>
class TTopicData : public dds::core::Value<D>
{
public:
    /**
     * Creates a TopicData QoS instance
     */
    TTopicData();

    /**
     * Creates a TopicData QoS instance
     *
     * @param sequence the sequence of octets representing the TopicData
     */
    explicit TTopicData(
            const dds::core::ByteSeq& sequence);

    /**
     * Copies a TopicData QoS instance
     *
     * @param other the TopicData QoS instance to copy
     */
    TTopicData(
            const TTopicData& other);

    /**
     * Creates a TopicData QoS instance
     *
     * @param value_begin a pointer to the beginning of a sequence
     * of octets
     * @param value_end a pointer to the end of a sequence
     * of octets
     */
    TTopicData(
            const uint8_t* value_begin,
            const uint8_t* value_end);

    /**
     * Set the sequence
     *
     * @param sequence a sequence of octets
     */
    TTopicData& value(
            const dds::core::ByteSeq& sequence);

    /**
     * Set the sequence
     *
     * @param begin an iterator pointing to the beginning of a sequence
     * of octets
     * @param end an iterator pointing to the end of a sequence of octets
     */
    template<typename OCTET_ITER>
    TTopicData& value(
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
template<typename D>
class TEntityFactory : public dds::core::Value<D>
{
public:
    /**
     * Creates an EntityFactory QoS instance
     *
     * @param autoenable_created_entities boolean indicating whether
     * created Entities should be automatically enabled
     */
    explicit TEntityFactory(
            bool autoenable_created_entities = true);

    /**
     * Copies an EntityFactory QoS instance
     *
     * @param other the EntityFactory QoS instance to copy
     */
    TEntityFactory(
            const TEntityFactory& other);

    /**
     * Sets a boolean indicating whether created Entities should be
     * automatically enabled
     *
     * @param autoenable_created_entities boolean indicating whether
     * created Entities should be automatically enabled
     */
    TEntityFactory& autoenable_created_entities(
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
    static TEntityFactory AutoEnable();

    /**
     * @return an EntityFactory QoS instance with autoenable_created_entities
     * set to false
     */
    static TEntityFactory ManuallyEnable();
};

//==============================================================================

/**
 * \copydoc DCPS_QoS_TransportPriority
 */
template<typename D>
class TTransportPriority : public dds::core::Value<D>
{
public:
    /**
     * Creates a TransportPriority QoS instance
     *
     * @param priority the priority value
     */
    explicit TTransportPriority(
            int32_t priority = 0);

    /**
     * Copies a TransportPriority QoS instance
     *
     * @param other the TransportPriority QoS instance to copy
     */
    TTransportPriority(
            const TTransportPriority& other);

    /**
     * Sets the priority value
     *
     * @param priority the priority value
     */
    TTransportPriority& value(
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
template<typename D>
class TLifespan : public dds::core::Value<D>
{
public:
    /**
     * Creates a Lifespan QoS instance
     *
     * @param duration Lifespan expiration duration
     */
    explicit TLifespan(
            const dds::core::Duration& duration = dds::core::Duration::infinite());

    /**
     * Copies a Lifespan QoS instance
     *
     * @param other the Lifespan QoS instance to copy
     */
    TLifespan(
            const TLifespan& other);

    /**
     * Sets the expiration duration
     *
     * @param duration expiration duration
     */
    TLifespan& duration(
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
template<typename D>
class TDeadline : public dds::core::Value<D>
{
public:
    /**
     * Creates a Deadline QoS instance
     *
     * @param period deadline period
     */
    explicit TDeadline(
            const dds::core::Duration& period = dds::core::Duration::infinite());

    /**
     * Copies a Deadline QoS instance
     *
     * @param other the Deadline QoS instance to copy
     */
    TDeadline(
            const TDeadline& other);

    /**
     * Sets the deadline period
     *
     * @param period deadline period
     */
    TDeadline& period(
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
template<typename D>
class TLatencyBudget : public dds::core::Value<D>
{
public:
    /**
     * Creates a LatencyBudget QoS instance
     *
     * @param duration duration
     */
    explicit TLatencyBudget(
            const dds::core::Duration& duration = dds::core::Duration::zero());

    /**
     * Copies a LatencyBudget QoS instance
     *
     * @param other the LatencyBudget QoS instance to copy
     */
    TLatencyBudget(
            const TLatencyBudget& other);

    /**
     * Sets the duration
     *
     * @param duration duration
     */
    TLatencyBudget& duration(
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
template<typename D>
class TTimeBasedFilter : public dds::core::Value<D>
{
public:
    /**
     * Creates a TimeBasedFilter QoS instance
     *
     * @param period minimum separation period
     */
    explicit TTimeBasedFilter(
        const dds::core::Duration& period = dds::core::Duration::zero());

    /**
     * Copies a TimeBasedFilter QoS instance
     *
     * @param other the TimeBasedFilter QoS instance to copy
     */
    TTimeBasedFilter(
            const TTimeBasedFilter& other);

    /**
     * Sets the minimum separation period
     *
     * @param period minimum separation period
     */
    TTimeBasedFilter& minimum_separation(
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
template<typename D>
class TPartition : public dds::core::Value<D>
{
public:
    /**
     * Creates a Partition QoS instance
     *
     * @param name partition name
     */
    explicit TPartition(
            const std::string& name = "");

    /**
     * Creates a Partition QoS instance
     *
     * @param names a sequence containing multiple partition names
     */
    explicit TPartition(
            const dds::core::StringSeq& names);

    /**
     * Copies a Partition QoS instance
     *
     * @param other the Partition QoS instance to copy
     */
    TPartition(
            const TPartition& other);

    /**
     * Sets the partition name
     *
     * @param name the partition name
     */
    TPartition& name(
            const std::string& name);

    /**
     * Sets multiple partition names
     *
     * @param names a sequence containing multiple partition names
     */
    TPartition& name(
            const dds::core::StringSeq& names);

    /**
     * Gets the partition names
     *
     * @return a sequence containing the partition names
     */
    const dds::core::StringSeq name() const;
};

//==============================================================================
#ifdef OMG_DDS_OWNERSHIP_SUPPORT

/**
 * \copydoc DCPS_QoS_Ownership
 */
template<typename D>
class TOwnership : public dds::core::Value<D>
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
    explicit TOwnership(
        dds::core::policy::OwnershipKind::Type kind = dds::core::policy::OwnershipKind::SHARED);

    /**
     * Copies an Ownership QoS instance
     *
     * @param other the Ownership QoS instance to copy
     */
    TOwnership(
            const TOwnership& other);

    /**
     * Set the kind
     *
     * @param kind the kind
     */
    TOwnership& kind(
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
    static TOwnership Exclusive();

    /**
     * @return an Ownership QoS instance with the kind set to SHARED
     */
    static TOwnership Shared();
};


//==============================================================================

/**
 * \copydoc DCPS_QoS_OwnershipStrength
 */
template<typename D>
class TOwnershipStrength : public dds::core::Value<D>
{
public:
    /**
     * Creates an OwnershipStrength QoS instance
     *
     * @param strength ownership strength
     */
    explicit TOwnershipStrength(
            int32_t strength = 0);

    /**
     * Copies an OwnershipStrength QoS instance
     *
     * @param other the OwnershipStrength QoS instance to copy
     */
    TOwnershipStrength(
            const TOwnershipStrength& other);

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
    TOwnershipStrength& value(
            int32_t strength);
};

#endif  // OMG_DDS_OWNERSHIP_SUPPORT
//==============================================================================

/**
 * \copydoc DCPS_QoS_WriterDataLifecycle
 */
template<typename D>
class TWriterDataLifecycle : public dds::core::Value<D>
{
public:
    /**
     * Creates a WriterDataLifecycle QoS instance
     *
     * @param autodispose_unregistered_instances Specifies the behavior of the DataWriter
     * with regards to the lifecycle of the data-instances it manages.
     */
    explicit TWriterDataLifecycle(
            bool autodispose_unregistered_instances = true);

    /**
     * Copies a WriterDataLifecycle QoS instance
     *
     * @param other the WriterDataLifecycle QoS instance to copy
     */
    TWriterDataLifecycle(
            const TWriterDataLifecycle& other);

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
    TWriterDataLifecycle& autodispose_unregistered_instances(
        bool autodispose_unregistered_instances);

    /**
     * @return a WriterDataLifecycle QoS instance with autodispose_unregistered_instances
     * set to true
     */
    static TWriterDataLifecycle AutoDisposeUnregisteredInstances();

    /**
     * @return a WriterDataLifecycle QoS instance with autodispose_unregistered_instances
     * set to false
     */
    static TWriterDataLifecycle ManuallyDisposeUnregisteredInstances();

};

//==============================================================================

/**
 * \copydoc DCPS_QoS_ReaderDataLifecycle
 */
template<typename D>
class TReaderDataLifecycle : public dds::core::Value<D>
{
public:
    /**
     * Creates a ReaderDataLifecycle QoS instance
     *
     * @param autopurge_nowriter_samples_delay the autopurge nowriter samples delay
     * @param autopurge_disposed_samples_delay the autopurge disposed samples delay
     */
    TReaderDataLifecycle(
        const dds::core::Duration& autopurge_nowriter_samples_delay = dds::core::Duration::infinite(),
        const dds::core::Duration& autopurge_disposed_samples_delay = dds::core::Duration::infinite());

    /**
     * Copies a ReaderDataLifecycle QoS instance
     *
     * @param other the ReaderDataLifecycle QoS instance to copy
     */
    TReaderDataLifecycle(
            const TReaderDataLifecycle& other);
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
    TReaderDataLifecycle& autopurge_nowriter_samples_delay(
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
    TReaderDataLifecycle& autopurge_disposed_samples_delay(
        const dds::core::Duration& autopurge_disposed_samples_delay);

    /**
     * @return a ReaderDataLifecycle QoS instance which will not autopurge disposed
     * samples
     */
    static TReaderDataLifecycle NoAutoPurgeDisposedSamples();

    /**
     * @param autopurge_disposed_samples_delay the autopurge disposed samples delay
     * @return a ReaderDataLifecycle QoS instance with autopurge_disposed_samples_delay
     * set to a specified value
     */
    static TReaderDataLifecycle AutoPurgeDisposedSamples(
        const dds::core::Duration& autopurge_disposed_samples_delay);

};

//==============================================================================

/**
 * \copydoc DCPS_QoS_Durability
 */
template<typename D>
class TDurability : public dds::core::Value<D>
{
public:
    /**
     * Creates a Durability QoS instance
     *
     * @param kind the kind
     */
    explicit TDurability(
        dds::core::policy::DurabilityKind::Type kind = dds::core::policy::DurabilityKind::VOLATILE);

    /**
     * Copies a Durability QoS instance
     *
     * @param other the Durability QoS instance to copy
     */
    TDurability(
            const TDurability& other);

    /**
    * Set the kind
    *
    * @param kind the kind
    */
    TDurability& kind(
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
    static TDurability Volatile();

    /**
     * @return a Durability QoS instance with the kind set to TRANSIENT_LOCAL
     */
    static TDurability TransientLocal();

    /**
     * @return a Durability QoS instance with the kind set to TRANSIENT
     */
    static TDurability Transient();

    /**
     * @return a Durability QoS instance with the kind set to PERSISTENT
     */
    static TDurability Persistent();
};

//==============================================================================

/**
 * \copydoc DCPS_QoS_Presentation
 */
template<typename D>
class TPresentation : public dds::core::Value<D>
{
public:
    /**
     * Creates a Presentation QoS instance
     *
     * @param access_scope the access_scope kind
     * @param coherent_access the coherent_access setting
     * @param ordered_access the ordered_access setting
     */
    TPresentation(
        dds::core::policy::PresentationAccessScopeKind::Type access_scope
        = dds::core::policy::PresentationAccessScopeKind::INSTANCE,
        bool coherent_access = false,
        bool ordered_access = false);

    /**
     * Copies a Presentation QoS instance
     *
     * @param other the Presentation QoS instance to copy
     */
    TPresentation(
            const TPresentation& other);

    /**
     * Sets the access_scope kind
     *
     * @param access_scope the access_scope kind
     */
    TPresentation& access_scope(
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
    TPresentation& coherent_access(
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
    TPresentation& ordered_access(
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
    static TPresentation GroupAccessScope(
            bool coherent_access = false,
            bool ordered_access = false);

    /**
     * @param coherent_access the coherent_access setting
     * @param ordered_access the ordered_access setting
     *
     * @return a Presentation QoS instance with a INSTANCE access_score and coherent_access
     * and ordered_access set to the specified values
     */
    static TPresentation InstanceAccessScope(
            bool coherent_access = false,
            bool ordered_access = false);

    /**
     * @param coherent_access the coherent_access setting
     * @param ordered_access the ordered_access setting
     *
     * @return a Presentation QoS instance with a TOPIC access_score and coherent_access
     * and ordered_access set to the specified values
     */
    static TPresentation TopicAccessScope(
            bool coherent_access = false,
            bool ordered_access = false);
};

//==============================================================================

/**
 * \copydoc DCPS_QoS_Reliability
 */
template<typename D>
class TReliability : public dds::core::Value<D>
{
public:
    /**
     * Creates a Reliability QoS instance
     *
     * @param kind the kind
     * @param max_blocking_time the max_blocking_time
     */
    TReliability(
        dds::core::policy::ReliabilityKind::Type kind = dds::core::policy::ReliabilityKind::BEST_EFFORT,
        const dds::core::Duration& max_blocking_time = dds::core::Duration::from_millisecs(100));

    /**
     * Copies a Reliability QoS instance
     *
     * @param other the Reliability QoS instance to copy
     */
    TReliability(
            const TReliability& other);

    /**
     * Sets the kind
     *
     * @param kind the kind
     */
    TReliability& kind(
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
    TReliability& max_blocking_time(
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
    static TReliability Reliable(
            const dds::core::Duration& max_blocking_time = dds::core::Duration::from_millisecs(100));

    /**
     * @return a Reliability QoS instance with the kind set to BEST_EFFORT
     */
    static TReliability BestEffort(
            const dds::core::Duration& max_blocking_time = dds::core::Duration::from_millisecs(100));

};

//==============================================================================

/**
 * \copydoc DCPS_QoS_DestinationOrder
 */
template<typename D>
class TDestinationOrder : public dds::core::Value<D>
{
public:
    /**
     * Creates a DestinationOrder QoS instance
     *
     * @param kind the kind
     */
    explicit TDestinationOrder(
        dds::core::policy::DestinationOrderKind::Type kind
        = dds::core::policy::DestinationOrderKind::BY_RECEPTION_TIMESTAMP);

    /**
     * Copies a DestinationOrder QoS instance
     *
     * @param other the Reliability QoS instance to copy
     */
    TDestinationOrder(
            const TDestinationOrder& other);

public:
    /**
     * Sets the kind
     *
     * @param kind the kind
     */
    TDestinationOrder& kind(
            dds::core::policy::DestinationOrderKind::Type kind);

    /**
     * Gets the kind
     *
     * @return the kind
     */
    dds::core::policy::DestinationOrderKind::Type kind() const;

public:
    /**
     * @return a DestinationOrder QoS instance with the kind set to BY_SOURCE_TIMESTAMP
     */
    static TDestinationOrder SourceTimestamp();

    /**
     * @return a DestinationOrder QoS instance with the kind set to BY_RECEPTION_TIMESTAMP
     */
    static TDestinationOrder ReceptionTimestamp();
};

//==============================================================================

/**
 * \copydoc DCPS_QoS_History
 */
template<typename D>
class THistory : public dds::core::Value<D>
{
public:
    /**
     * Creates a History QoS instance
     *
     * @param kind the kind
     * @param depth the history depth
     */
    THistory(
            dds::core::policy::HistoryKind::Type kind = dds::core::policy::HistoryKind::KEEP_LAST,
             int32_t depth = 1);

    /**
     * Copies a History QoS instance
     *
     * @param other the History QoS instance to copy
     */
    THistory(
            const THistory& other);

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
    THistory& kind(
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
    THistory& depth(
            int32_t depth);

    /**
     * @return a History QoS instance with the kind set to KEEP_ALL
     */
    static THistory KeepAll();

    /**
     * @param depth the history depth
     * @return a History QoS instance with the kind set to KEEP_LAST and the
     * depth set to the supplied value
     */
    static THistory KeepLast(
            uint32_t depth);
};

//==============================================================================

/**
 * \copydoc DCPS_QoS_ResourceLimits
 */
template<typename D>
class TResourceLimits : public dds::core::Value<D>
{
public:
    /**
     * Creates a ResourceLimits QoS instance
     *
     * @param max_samples the max_samples value
     * @param max_instances the max_instances value
     * @param max_samples_per_instance the max_samples_per_instance value
     */
    TResourceLimits(
            int32_t max_samples = dds::core::LENGTH_UNLIMITED,
            int32_t max_instances = dds::core::LENGTH_UNLIMITED,
            int32_t max_samples_per_instance = dds::core::LENGTH_UNLIMITED);

    /**
     * Copies a ResourceLimits QoS instance
     *
     * @param other the ResourceLimits QoS instance to copy
     */
    TResourceLimits(
            const TResourceLimits& other);

public:
    /**
     * Sets the max_samples value
     *
     * @param max_samples the max_samples value
     */
    TResourceLimits& max_samples(
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
    TResourceLimits& max_instances(
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
    TResourceLimits& max_samples_per_instance(
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
template<typename D>
class TLiveliness : public dds::core::Value<D>
{
public:
    /**
     * Creates a Liveliness QoS instance
     *
     * @param kind the kind
     * @param lease_duration the lease_duration
     */
    TLiveliness(
        dds::core::policy::LivelinessKind::Type kind = dds::core::policy::LivelinessKind::AUTOMATIC,
        const dds::core::Duration& lease_duration = dds::core::Duration::infinite());

    /**
     * Copies a Liveliness QoS instance
     *
     * @param other the Liveliness QoS instance to copy
     */
    TLiveliness(
            const TLiveliness& other);

    /**
     * Sets the kind
     *
     * @param kind the kind
     */
    TLiveliness& kind(
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
    TLiveliness& lease_duration(
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
    static TLiveliness Automatic();

    /**
     * @return a Liveliness QoS instance with the kind set to MANUAL_BY_PARTICIPANT
     * and the lease_duration set to the supplied value
     */
    static TLiveliness ManualByParticipant(
            const dds::core::Duration& lease_duration = dds::core::Duration::infinite());

    /**
     * @return a Liveliness QoS instance with the kind set to MANUAL_BY_TOPIC
     * and the lease_duration set to the supplied value
     */
    static TLiveliness ManualByTopic(
            const dds::core::Duration& lease_duration = dds::core::Duration::infinite());
};


//==============================================================================
#ifdef OMG_DDS_PERSISTENCE_SUPPORT

/**
 * \copydoc DCPS_QoS_DurabilityService
 */
template <typename D>
class TDurabilityService : public dds::core::Value<D>
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
    TDurabilityService(
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
    TDurabilityService(
            const TDurabilityService& other);

    /**
     * Sets the service_cleanup_delay value
     *
     * @param service_cleanup_delay the service_cleanup_delay value
     */
    TDurabilityService& service_cleanup_delay(
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
    TDurabilityService& history_kind(
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
    TDurabilityService& history_depth(
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
    TDurabilityService& max_samples(
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
    TDurabilityService& max_instances(
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
    TDurabilityService& max_samples_per_instance(
            int32_t max_samples_per_instance);

    /**
     * Gets the max_samples_per_instance value
     *
     * @return the max_samples_per_instance value
     */
    int32_t max_samples_per_instance() const;
};

#endif  //OMG_DDS_PERSISTENCE_SUPPORT

//==============================================================================

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

template<typename D>
class TDataRepresentation : public dds::core::Value<D>
{

public:
    explicit TDataRepresentation(
        const dds::core::policy::DataRepresentationIdSeq& value);

    TDataRepresentation(const TDataRepresentation& other)
        : dds::core::Value<D>(other.value())
    { }
public:
    TDataRepresentation& value(
            const dds::core::policy::DataRepresentationIdSeq& value);

    const dds::core::policy::DataRepresentationIdSeq value() const;

    dds::core::policy::DataRepresentationIdSeq&
};

#endif  //OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT


//============================================================================

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

template <typename D>
class TTypeConsistencyEnforcement : public dds::core::Value<D>
{
public:
    explicit TTypeConsistencyEnforcement(
            dds::core::policy::TypeConsistencyEnforcementKind::Type kind);

    TTypeConsistencyEnforcement& kind(
            dds::core::policy::TypeConsistencyEnforcementKind::Type kind);

    dds::core::policy::TypeConsistencyEnforcementKind::Type  kind() const;
};

#endif  //OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

//==============================================================================
// Policy Trait Classes

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
typedef uint16_t DataRepresentationId;

typedef std::vector<DataRepresentationId> DataRepresentationIdSeq;
#endif

typedef dds::core::policy::detail::Deadline Deadline;

typedef dds::core::policy::detail::DestinationOrder DestinationOrder;

typedef dds::core::policy::detail::Durability Durability;

typedef dds::core::policy::detail::EntityFactory EntityFactory;

typedef dds::core::policy::detail::GroupData GroupData;

typedef dds::core::policy::detail::History History;

typedef dds::core::policy::detail::LatencyBudget LatencyBudget;

typedef dds::core::policy::detail::Lifespan Lifespan;

typedef dds::core::policy::detail::Liveliness Liveliness;

typedef dds::core::policy::detail::Ownership Ownership;

#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
typedef dds::core::policy::detail::OwnershipStrength OwnershipStrength;
#endif  //OMG_DDS_OWNERSHIP_SUPPORT

typedef dds::core::policy::detail::Partition Partition;

template<typename Policy>
class policy_id;

template<typename Policy>
class policy_name;

typedef dds::core::policy::detail::Presentation Presentation;

typedef ::dds::core::policy::detail::QosPolicyCount QosPolicyCount;

typedef std::vector<QosPolicyCount> QosPolicyCountSeq;

typedef uint32_t QosPolicyId;

typedef dds::core::policy::detail::ReaderDataLifecycle ReaderDataLifecycle;

typedef dds::core::policy::detail::Reliability Reliability;

typedef dds::core::policy::detail::ResourceLimits ResourceLimits;

typedef dds::core::policy::detail::TimeBasedFilter TimeBasedFilter;

typedef dds::core::policy::detail::TopicData TopicData;

typedef dds::core::policy::detail::TransportPriority TransportPriority;

typedef dds::core::policy::detail::UserData UserData;

typedef dds::core::policy::detail::WriterDataLifecycle WriterDataLifecycle;

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
typedef ::dds::core::policy::detail::DurabilityService DurabilityService;
#endif  //OMG_DDS_PERSISTENCE_SUPPORT

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
/** @todo - Known issue. */
#endif



OMG_DDS_POLICY_TRAITS(UserData,             1)
OMG_DDS_POLICY_TRAITS(Durability,           2)
OMG_DDS_POLICY_TRAITS(Presentation,         3)
OMG_DDS_POLICY_TRAITS(Deadline,             4)
OMG_DDS_POLICY_TRAITS(LatencyBudget,        5)
OMG_DDS_POLICY_TRAITS(Ownership,            6)

#ifdef OMG_DDS_OWNERSHIP_SUPPORT
OMG_DDS_POLICY_TRAITS(OwnershipStrength,    7)
#endif  //OMG_DDS_OWNERSHIP_SUPPORT

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

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
OMG_DDS_POLICY_TRAITS(DurabilityService,    22)
#endif  //OMG_DDS_PERSISTENCE_SUPPORT


} //namespace policy
} //namespace core
} //namespace dds


#endif //OMG_DDS_CORE_POLICY_CORE_POLICY_HPP_
