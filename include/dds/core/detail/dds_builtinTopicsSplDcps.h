#ifndef DDS_BUILTINTOPICSSPLTYPES_H
#define DDS_BUILTINTOPICSSPLTYPES_H

#include <c_base.h>
#include <c_misc.h>
#include <c_sync.h>
#include <c_collection.h>
#include <c_field.h>
#include <dds/core/ddscore.hpp>
#include <dds/topic/BuiltinTopic.hpp>
#include <org/opensplice/topic/TypeHash.hpp>
#include "v_copyIn.h"

typedef os_uchar *_DDS_octSeq;
v_copyin_result
__DDS_octSeq__copyIn(
    c_type dbType,
    const dds::core::ByteSeq *from,
    _DDS_octSeq *to);


typedef c_long _DDS_BuiltinTopicKey_t[3];
v_copyin_result
__DDS_BuiltinTopicKey_t__copyIn(
    c_type dbType,
    const dds::topic::BuiltinTopicKey *from,
    _DDS_BuiltinTopicKey_t *to);

typedef c_sequence _DDS_StringSeq;
v_copyin_result
__DDS_StringSeq__copyIn(
    c_type dbType,
    const dds::core::StringSeq *from,
    _DDS_StringSeq *to);

typedef c_short _DDS_DataRepresentationId_t;

struct _DDS_Duration_t;
OMG_DDS_API v_copyin_result __DDS_Duration_t__copyIn(c_type dbType, const struct dds::core::Duration *from, struct _DDS_Duration_t *to);
OMG_DDS_API void __DDS_Duration_t__copyOut(const void *_from, void *_to);
struct _DDS_Duration_t {
    c_long sec;
    c_ulong nanosec;
};


struct _DDS_UserDataQosPolicy;
OMG_DDS_API v_copyin_result __DDS_UserDataQosPolicy__copyIn(c_type dbType, const dds::core::policy::UserData *from, struct _DDS_UserDataQosPolicy *to);
OMG_DDS_API void __DDS_UserDataQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_UserDataQosPolicy {
    _DDS_octSeq value;
};

struct _DDS_TopicDataQosPolicy;
OMG_DDS_API v_copyin_result __DDS_TopicDataQosPolicy__copyIn(c_type dbType, const dds::core::policy::TopicData *from, struct _DDS_TopicDataQosPolicy *to);
OMG_DDS_API void __DDS_TopicDataQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_TopicDataQosPolicy {
    _DDS_octSeq value;
};

struct _DDS_GroupDataQosPolicy;
OMG_DDS_API v_copyin_result __DDS_GroupDataQosPolicy__copyIn(c_type dbType, const dds::core::policy::GroupData *from, struct _DDS_GroupDataQosPolicy *to);
OMG_DDS_API void __DDS_GroupDataQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_GroupDataQosPolicy {
    _DDS_octSeq value;
};

struct _DDS_TransportPriorityQosPolicy;
OMG_DDS_API v_copyin_result __DDS_TransportPriorityQosPolicy__copyIn(c_type dbType, const dds::core::policy::TransportPriority *from, struct _DDS_TransportPriorityQosPolicy *to);
OMG_DDS_API void __DDS_TransportPriorityQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_TransportPriorityQosPolicy {
    c_long value;
};

struct _DDS_LifespanQosPolicy;
OMG_DDS_API v_copyin_result __DDS_LifespanQosPolicy__copyIn(c_type dbType, const dds::core::policy::Lifespan *from, struct _DDS_LifespanQosPolicy *to);
OMG_DDS_API void __DDS_LifespanQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_LifespanQosPolicy {
    struct _DDS_Duration_t duration;
};

enum _DDS_DurabilityQosPolicyKind {
    _DDS_VOLATILE_DURABILITY_QOS,
    _DDS_TRANSIENT_LOCAL_DURABILITY_QOS,
    _DDS_TRANSIENT_DURABILITY_QOS,
    _DDS_PERSISTENT_DURABILITY_QOS
};
struct _DDS_DurabilityQosPolicy;
OMG_DDS_API v_copyin_result __DDS_DurabilityQosPolicy__copyIn(c_type dbType, const dds::core::policy::Durability *from, struct _DDS_DurabilityQosPolicy *to);
OMG_DDS_API void __DDS_DurabilityQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_DurabilityQosPolicy {
    enum _DDS_DurabilityQosPolicyKind kind;
};

enum _DDS_PresentationQosPolicyAccessScopeKind {
    _DDS_INSTANCE_PRESENTATION_QOS,
    _DDS_TOPIC_PRESENTATION_QOS,
    _DDS_GROUP_PRESENTATION_QOS
};
struct _DDS_PresentationQosPolicy;
OMG_DDS_API v_copyin_result __DDS_PresentationQosPolicy__copyIn(c_type dbType, const dds::core::policy::Presentation *from, struct _DDS_PresentationQosPolicy *to);
OMG_DDS_API void __DDS_PresentationQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_PresentationQosPolicy {
    enum _DDS_PresentationQosPolicyAccessScopeKind access_scope;
    c_bool coherent_access;
    c_bool ordered_access;
};

struct _DDS_DeadlineQosPolicy;
OMG_DDS_API v_copyin_result __DDS_DeadlineQosPolicy__copyIn(c_type dbType, const dds::core::policy::Deadline *from, struct _DDS_DeadlineQosPolicy *to);
OMG_DDS_API void __DDS_DeadlineQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_DeadlineQosPolicy {
    struct _DDS_Duration_t period;
};

struct _DDS_LatencyBudgetQosPolicy;
OMG_DDS_API v_copyin_result __DDS_LatencyBudgetQosPolicy__copyIn(c_type dbType, const dds::core::policy::LatencyBudget *from, struct _DDS_LatencyBudgetQosPolicy *to);
OMG_DDS_API void __DDS_LatencyBudgetQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_LatencyBudgetQosPolicy {
    struct _DDS_Duration_t duration;
};

enum _DDS_OwnershipQosPolicyKind {
    _DDS_SHARED_OWNERSHIP_QOS,
    _DDS_EXCLUSIVE_OWNERSHIP_QOS
};
struct _DDS_OwnershipQosPolicy;
OMG_DDS_API v_copyin_result __DDS_OwnershipQosPolicy__copyIn(c_type dbType, const dds::core::policy::Ownership *from, struct _DDS_OwnershipQosPolicy *to);
OMG_DDS_API void __DDS_OwnershipQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_OwnershipQosPolicy {
    enum _DDS_OwnershipQosPolicyKind kind;
};

#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
struct _DDS_OwnershipStrengthQosPolicy;
OMG_DDS_API v_copyin_result __DDS_OwnershipStrengthQosPolicy__copyIn(c_type dbType, const dds::core::policy::OwnershipStrength *from, struct _DDS_OwnershipStrengthQosPolicy *to);
OMG_DDS_API void __DDS_OwnershipStrengthQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_OwnershipStrengthQosPolicy {
    c_long value;
};
#endif  // OMG_DDS_OWNERSHIP_SUPPORT

enum _DDS_LivelinessQosPolicyKind {
    _DDS_AUTOMATIC_LIVELINESS_QOS,
    _DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
    _DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS
};
struct _DDS_LivelinessQosPolicy;
OMG_DDS_API v_copyin_result __DDS_LivelinessQosPolicy__copyIn(c_type dbType, const dds::core::policy::Liveliness *from, struct _DDS_LivelinessQosPolicy *to);
OMG_DDS_API void __DDS_LivelinessQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_LivelinessQosPolicy {
    enum _DDS_LivelinessQosPolicyKind kind;
    struct _DDS_Duration_t lease_duration;
};

struct _DDS_TimeBasedFilterQosPolicy;
OMG_DDS_API v_copyin_result __DDS_TimeBasedFilterQosPolicy__copyIn(c_type dbType, const dds::core::policy::TimeBasedFilter *from, struct _DDS_TimeBasedFilterQosPolicy *to);
OMG_DDS_API void __DDS_TimeBasedFilterQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_TimeBasedFilterQosPolicy {
    struct _DDS_Duration_t minimum_separation;
};

struct _DDS_PartitionQosPolicy;
OMG_DDS_API v_copyin_result __DDS_PartitionQosPolicy__copyIn(c_type dbType, const dds::core::policy::Partition *from, struct _DDS_PartitionQosPolicy *to);
OMG_DDS_API void __DDS_PartitionQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_PartitionQosPolicy {
    _DDS_StringSeq name;
};

enum _DDS_ReliabilityQosPolicyKind {
    _DDS_BEST_EFFORT_RELIABILITY_QOS,
    _DDS_RELIABLE_RELIABILITY_QOS
};
struct _DDS_ReliabilityQosPolicy;
OMG_DDS_API v_copyin_result __DDS_ReliabilityQosPolicy__copyIn(c_type dbType, const dds::core::policy::Reliability *from, struct _DDS_ReliabilityQosPolicy *to);
OMG_DDS_API void __DDS_ReliabilityQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_ReliabilityQosPolicy {
    enum _DDS_ReliabilityQosPolicyKind kind;
    struct _DDS_Duration_t max_blocking_time;
    c_bool synchronous;
};

enum _DDS_DestinationOrderQosPolicyKind {
    _DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
    _DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
};
struct _DDS_DestinationOrderQosPolicy;
OMG_DDS_API v_copyin_result __DDS_DestinationOrderQosPolicy__copyIn(c_type dbType, const dds::core::policy::DestinationOrder *from, struct _DDS_DestinationOrderQosPolicy *to);
OMG_DDS_API void __DDS_DestinationOrderQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_DestinationOrderQosPolicy {
    enum _DDS_DestinationOrderQosPolicyKind kind;
};

enum _DDS_HistoryQosPolicyKind {
    _DDS_KEEP_LAST_HISTORY_QOS,
    _DDS_KEEP_ALL_HISTORY_QOS
};
struct _DDS_HistoryQosPolicy;
OMG_DDS_API v_copyin_result __DDS_HistoryQosPolicy__copyIn(c_type dbType, const dds::core::policy::History *from, struct _DDS_HistoryQosPolicy *to);
OMG_DDS_API void __DDS_HistoryQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_HistoryQosPolicy {
    enum _DDS_HistoryQosPolicyKind kind;
    c_long depth;
};

struct _DDS_ResourceLimitsQosPolicy;
OMG_DDS_API v_copyin_result __DDS_ResourceLimitsQosPolicy__copyIn(c_type dbType, const dds::core::policy::ResourceLimits *from, struct _DDS_ResourceLimitsQosPolicy *to);
OMG_DDS_API void __DDS_ResourceLimitsQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_ResourceLimitsQosPolicy {
    c_long max_samples;
    c_long max_instances;
    c_long max_samples_per_instance;
};

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
struct _DDS_DurabilityServiceQosPolicy;
OMG_DDS_API v_copyin_result __DDS_DurabilityServiceQosPolicy__copyIn(c_type dbType, const dds::core::policy::DurabilityService *from, struct _DDS_DurabilityServiceQosPolicy *to);
OMG_DDS_API void __DDS_DurabilityServiceQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_DurabilityServiceQosPolicy {
    struct _DDS_Duration_t service_cleanup_delay;
    enum _DDS_HistoryQosPolicyKind history_kind;
    c_long history_depth;
    c_long max_samples;
    c_long max_instances;
    c_long max_samples_per_instance;
};
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

struct _DDS_ProductDataQosPolicy;
OMG_DDS_API v_copyin_result __DDS_ProductDataQosPolicy__copyIn(c_type dbType, const org::opensplice::core::policy::ProductData *from, struct _DDS_ProductDataQosPolicy *to);
OMG_DDS_API void __DDS_ProductDataQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_ProductDataQosPolicy {
    c_string value;
};

struct _DDS_EntityFactoryQosPolicy;
OMG_DDS_API v_copyin_result __DDS_EntityFactoryQosPolicy__copyIn(c_type dbType, const dds::core::policy::EntityFactory *from, struct _DDS_EntityFactoryQosPolicy *to);
OMG_DDS_API void __DDS_EntityFactoryQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_EntityFactoryQosPolicy {
    c_bool autoenable_created_entities;
};

struct _DDS_ShareQosPolicy;
OMG_DDS_API v_copyin_result __DDS_ShareQosPolicy__copyIn(c_type dbType, const org::opensplice::core::policy::Share *from, struct _DDS_ShareQosPolicy *to);
OMG_DDS_API void __DDS_ShareQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_ShareQosPolicy {
    c_string name;
    c_bool enable;
};

struct _DDS_WriterDataLifecycleQosPolicy;
OMG_DDS_API v_copyin_result __DDS_WriterDataLifecycleQosPolicy__copyIn(c_type dbType, const dds::core::policy::WriterDataLifecycle *from, struct _DDS_WriterDataLifecycleQosPolicy *to);
OMG_DDS_API void __DDS_WriterDataLifecycleQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_WriterDataLifecycleQosPolicy {
    c_bool autodispose_unregistered_instances;
    struct _DDS_Duration_t autopurge_suspended_samples_delay;
    struct _DDS_Duration_t autounregister_instance_delay;
};

enum _DDS_InvalidSampleVisibilityQosPolicyKind {
    _DDS_NO_INVALID_SAMPLES,
    _DDS_MINIMUM_INVALID_SAMPLES,
    _DDS_ALL_INVALID_SAMPLES
};
struct _DDS_InvalidSampleVisibilityQosPolicy;
OMG_DDS_API v_copyin_result __DDS_InvalidSampleVisibilityQosPolicy__copyIn(c_type dbType, const org::opensplice::core::policy::InvalidSampleVisibility *from, struct _DDS_InvalidSampleVisibilityQosPolicy *to);
OMG_DDS_API void __DDS_InvalidSampleVisibilityQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_InvalidSampleVisibilityQosPolicy {
    enum _DDS_InvalidSampleVisibilityQosPolicyKind kind;
};

struct _DDS_SubscriptionKeyQosPolicy;
OMG_DDS_API v_copyin_result __DDS_SubscriptionKeyQosPolicy__copyIn(c_type dbType, const org::opensplice::core::policy::SubscriptionKey *from, struct _DDS_SubscriptionKeyQosPolicy *to);
OMG_DDS_API void __DDS_SubscriptionKeyQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_SubscriptionKeyQosPolicy {
    c_bool use_key_list;
    _DDS_StringSeq key_list;
};

struct _DDS_ReaderDataLifecycleQosPolicy;
OMG_DDS_API v_copyin_result __DDS_ReaderDataLifecycleQosPolicy__copyIn(c_type dbType, const dds::core::policy::ReaderDataLifecycle *from, struct _DDS_ReaderDataLifecycleQosPolicy *to);
OMG_DDS_API void __DDS_ReaderDataLifecycleQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_ReaderDataLifecycleQosPolicy {
    struct _DDS_Duration_t autopurge_nowriter_samples_delay;
    struct _DDS_Duration_t autopurge_disposed_samples_delay;
    c_bool autopurge_dispose_all;
    c_bool enable_invalid_samples;
    struct _DDS_InvalidSampleVisibilityQosPolicy invalid_sample_visibility;
};

struct _DDS_UserKeyQosPolicy;
OMG_DDS_API v_copyin_result __DDS_UserKeyQosPolicy__copyIn(c_type dbType, const org::opensplice::core::policy::SubscriptionKey *from, struct _DDS_UserKeyQosPolicy *to);
OMG_DDS_API void __DDS_UserKeyQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_UserKeyQosPolicy {
    c_bool enable;
    c_string expression;
};

struct _DDS_ReaderLifespanQosPolicy;
OMG_DDS_API v_copyin_result __DDS_ReaderLifespanQosPolicy__copyIn(c_type dbType, const org::opensplice::core::policy::ReaderLifespan *from, struct _DDS_ReaderLifespanQosPolicy *to);
OMG_DDS_API void __DDS_ReaderLifespanQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_ReaderLifespanQosPolicy {
    c_bool use_lifespan;
    struct _DDS_Duration_t duration;
};

struct _DDS_TypeHash;
OMG_DDS_API v_copyin_result __DDS_TypeHash__copyIn(c_type dbType, const class org::opensplice::topic::TypeHash *from, struct _DDS_TypeHash *to);
OMG_DDS_API void __DDS_TypeHash__copyOut(const void *_from, void *_to);
struct _DDS_TypeHash {
    c_ulonglong msb;
    c_ulonglong lsb;
};

struct _DDS_ParticipantBuiltinTopicData;
OMG_DDS_API v_copyin_result __DDS_ParticipantBuiltinTopicData__copyIn(c_type dbType, const dds::topic::ParticipantBuiltinTopicData *from, struct _DDS_ParticipantBuiltinTopicData *to);
OMG_DDS_API void __DDS_ParticipantBuiltinTopicData__copyOut(const void *_from, void *_to);
struct _DDS_ParticipantBuiltinTopicData {
    _DDS_BuiltinTopicKey_t key;
    struct _DDS_UserDataQosPolicy user_data;
};

struct _DDS_TopicBuiltinTopicData;
OMG_DDS_API v_copyin_result __DDS_TopicBuiltinTopicData__copyIn(c_type dbType, const dds::topic::TopicBuiltinTopicData *from, struct _DDS_TopicBuiltinTopicData *to);
OMG_DDS_API void __DDS_TopicBuiltinTopicData__copyOut(const void *_from, void *_to);
struct _DDS_TopicBuiltinTopicData {
    _DDS_BuiltinTopicKey_t key;
    c_string name;
    c_string type_name;
    struct _DDS_DurabilityQosPolicy durability;
    struct _DDS_DurabilityServiceQosPolicy durability_service;
    struct _DDS_DeadlineQosPolicy deadline;
    struct _DDS_LatencyBudgetQosPolicy latency_budget;
    struct _DDS_LivelinessQosPolicy liveliness;
    struct _DDS_ReliabilityQosPolicy reliability;
    struct _DDS_TransportPriorityQosPolicy transport_priority;
    struct _DDS_LifespanQosPolicy lifespan;
    struct _DDS_DestinationOrderQosPolicy destination_order;
    struct _DDS_HistoryQosPolicy history;
    struct _DDS_ResourceLimitsQosPolicy resource_limits;
    struct _DDS_OwnershipQosPolicy ownership;
    struct _DDS_TopicDataQosPolicy topic_data;
};

struct _DDS_TypeBuiltinTopicData;
OMG_DDS_API v_copyin_result __DDS_TypeBuiltinTopicData__copyIn(c_type dbType, const org::opensplice::topic::TypeBuiltinTopicData *from, struct _DDS_TypeBuiltinTopicData *to);
OMG_DDS_API void __DDS_TypeBuiltinTopicData__copyOut(const void *_from, void *_to);
struct _DDS_TypeBuiltinTopicData {
    c_string name;
    _DDS_DataRepresentationId_t data_representation_id;
    struct _DDS_TypeHash type_hash;
    _DDS_octSeq meta_data;
    _DDS_octSeq extentions;
};

struct _DDS_PublicationBuiltinTopicData;
OMG_DDS_API v_copyin_result __DDS_PublicationBuiltinTopicData__copyIn(c_type dbType, const dds::topic::PublicationBuiltinTopicData *from, struct _DDS_PublicationBuiltinTopicData *to);
OMG_DDS_API void __DDS_PublicationBuiltinTopicData__copyOut(const void *_from, void *_to);
struct _DDS_PublicationBuiltinTopicData {
    _DDS_BuiltinTopicKey_t key;
    _DDS_BuiltinTopicKey_t participant_key;
    c_string topic_name;
    c_string type_name;
    struct _DDS_DurabilityQosPolicy durability;
    struct _DDS_DeadlineQosPolicy deadline;
    struct _DDS_LatencyBudgetQosPolicy latency_budget;
    struct _DDS_LivelinessQosPolicy liveliness;
    struct _DDS_ReliabilityQosPolicy reliability;
    struct _DDS_LifespanQosPolicy lifespan;
    struct _DDS_DestinationOrderQosPolicy destination_order;
    struct _DDS_UserDataQosPolicy user_data;
    struct _DDS_OwnershipQosPolicy ownership;
    struct _DDS_OwnershipStrengthQosPolicy ownership_strength;
    struct _DDS_PresentationQosPolicy presentation;
    struct _DDS_PartitionQosPolicy partition;
    struct _DDS_TopicDataQosPolicy topic_data;
    struct _DDS_GroupDataQosPolicy group_data;
};

struct _DDS_SubscriptionBuiltinTopicData;
OMG_DDS_API v_copyin_result __DDS_SubscriptionBuiltinTopicData__copyIn(c_type dbType, const dds::topic::SubscriptionBuiltinTopicData *from, struct _DDS_SubscriptionBuiltinTopicData *to);
OMG_DDS_API void __DDS_SubscriptionBuiltinTopicData__copyOut(const void *_from, void *_to);
struct _DDS_SubscriptionBuiltinTopicData {
    _DDS_BuiltinTopicKey_t key;
    _DDS_BuiltinTopicKey_t participant_key;
    c_string topic_name;
    c_string type_name;
    struct _DDS_DurabilityQosPolicy durability;
    struct _DDS_DeadlineQosPolicy deadline;
    struct _DDS_LatencyBudgetQosPolicy latency_budget;
    struct _DDS_LivelinessQosPolicy liveliness;
    struct _DDS_ReliabilityQosPolicy reliability;
    struct _DDS_OwnershipQosPolicy ownership;
    struct _DDS_DestinationOrderQosPolicy destination_order;
    struct _DDS_UserDataQosPolicy user_data;
    struct _DDS_TimeBasedFilterQosPolicy time_based_filter;
    struct _DDS_PresentationQosPolicy presentation;
    struct _DDS_PartitionQosPolicy partition;
    struct _DDS_TopicDataQosPolicy topic_data;
    struct _DDS_GroupDataQosPolicy group_data;
};

struct _DDS_CMParticipantBuiltinTopicData;
OMG_DDS_API v_copyin_result __DDS_CMParticipantBuiltinTopicData__copyIn(c_type dbType, const org::opensplice::topic::CMParticipantBuiltinTopicData *from, struct _DDS_CMParticipantBuiltinTopicData *to);
OMG_DDS_API void __DDS_CMParticipantBuiltinTopicData__copyOut(const void *_from, void *_to);
struct _DDS_CMParticipantBuiltinTopicData {
    _DDS_BuiltinTopicKey_t key;
    struct _DDS_ProductDataQosPolicy product;
};


struct _DDS_CMPublisherBuiltinTopicData;
OMG_DDS_API v_copyin_result __DDS_CMPublisherBuiltinTopicData__copyIn(c_type dbType, const org::opensplice::topic::CMPublisherBuiltinTopicData *from, struct _DDS_CMPublisherBuiltinTopicData *to);
OMG_DDS_API void __DDS_CMPublisherBuiltinTopicData__copyOut(const void *_from, void *_to);
struct _DDS_CMPublisherBuiltinTopicData {
    _DDS_BuiltinTopicKey_t key;
    struct _DDS_ProductDataQosPolicy product;
    _DDS_BuiltinTopicKey_t participant_key;
    c_string name;
    struct _DDS_EntityFactoryQosPolicy entity_factory;
    struct _DDS_PartitionQosPolicy partition;
};

struct _DDS_CMSubscriberBuiltinTopicData;
OMG_DDS_API v_copyin_result __DDS_CMSubscriberBuiltinTopicData__copyIn(c_type dbType, const org::opensplice::topic::CMSubscriberBuiltinTopicData *from, struct _DDS_CMSubscriberBuiltinTopicData *to);
OMG_DDS_API void __DDS_CMSubscriberBuiltinTopicData__copyOut(const void *_from, void *_to);
struct _DDS_CMSubscriberBuiltinTopicData {
    _DDS_BuiltinTopicKey_t key;
    struct _DDS_ProductDataQosPolicy product;
    _DDS_BuiltinTopicKey_t participant_key;
    c_string name;
    struct _DDS_EntityFactoryQosPolicy entity_factory;
    struct _DDS_ShareQosPolicy share;
    struct _DDS_PartitionQosPolicy partition;
};

struct _DDS_CMDataWriterBuiltinTopicData;
OMG_DDS_API v_copyin_result __DDS_CMDataWriterBuiltinTopicData__copyIn(c_type dbType, const org::opensplice::topic::CMDataWriterBuiltinTopicData *from, struct _DDS_CMDataWriterBuiltinTopicData *to);
OMG_DDS_API void __DDS_CMDataWriterBuiltinTopicData__copyOut(const void *_from, void *_to);
struct _DDS_CMDataWriterBuiltinTopicData {
    _DDS_BuiltinTopicKey_t key;
    struct _DDS_ProductDataQosPolicy product;
    _DDS_BuiltinTopicKey_t publisher_key;
    c_string name;
    struct _DDS_HistoryQosPolicy history;
    struct _DDS_ResourceLimitsQosPolicy resource_limits;
    struct _DDS_WriterDataLifecycleQosPolicy writer_data_lifecycle;
};

struct _DDS_CMDataReaderBuiltinTopicData;
OMG_DDS_API v_copyin_result __DDS_CMDataReaderBuiltinTopicData__copyIn(c_type dbType, const org::opensplice::topic::CMDataReaderBuiltinTopicData *from, struct _DDS_CMDataReaderBuiltinTopicData *to);
OMG_DDS_API void __DDS_CMDataReaderBuiltinTopicData__copyOut(const void *_from, void *_to);
struct _DDS_CMDataReaderBuiltinTopicData {
    _DDS_BuiltinTopicKey_t key;
    struct _DDS_ProductDataQosPolicy product;
    _DDS_BuiltinTopicKey_t subscriber_key;
    c_string name;
    struct _DDS_HistoryQosPolicy history;
    struct _DDS_ResourceLimitsQosPolicy resource_limits;
    struct _DDS_ReaderDataLifecycleQosPolicy reader_data_lifecycle;
    struct _DDS_UserKeyQosPolicy subscription_keys;
    struct _DDS_ReaderLifespanQosPolicy reader_lifespan;
    struct _DDS_ShareQosPolicy share;
};



#endif /* DDS_BUILTINTOPICSSPLTYPES_H */
