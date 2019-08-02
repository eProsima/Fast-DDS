#ifndef DDS_DCPS_BUILTINTOPICSSPLTYPES_H
#define DDS_DCPS_BUILTINTOPICSSPLTYPES_H

#include <dds/core/detail/dds_builtinTopicsSplDcps.h>

struct _DDS_Time_t ;
OMG_DDS_API v_copyin_result __DDS_Time_t__copyIn(c_type dbType, const dds::core::Time *from, struct _DDS_Time_t *to);
OMG_DDS_API void __DDS_Time_t__copyOut(const void *_from, void *_to);
struct _DDS_Time_t {
    c_long sec;
    c_ulong nanosec;
};

enum _DDS_SchedulingClassQosPolicyKind {
    _DDS_SCHEDULE_DEFAULT,
    _DDS_SCHEDULE_TIMESHARING,
    _DDS_SCHEDULE_REALTIME
};

struct _DDS_SchedulingClassQosPolicy ;
OMG_DDS_API v_copyin_result __DDS_SchedulingClassQosPolicy__copyIn(c_type dbType, const org::opensplice::core::policy::SchedulingKind *from, struct _DDS_SchedulingClassQosPolicy *to);
OMG_DDS_API void __DDS_SchedulingClassQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_SchedulingClassQosPolicy {
    enum _DDS_SchedulingClassQosPolicyKind kind;
};

enum _DDS_SchedulingPriorityQosPolicyKind {
    _DDS_PRIORITY_RELATIVE,
    _DDS_PRIORITY_ABSOLUTE
};

struct _DDS_SchedulingPriorityQosPolicy ;
OMG_DDS_API v_copyin_result __DDS_SchedulingPriorityQosPolicy__copyIn(c_type dbType, const org::opensplice::core::policy::SchedulingPriorityKind *from, struct _DDS_SchedulingPriorityQosPolicy *to);
OMG_DDS_API void __DDS_SchedulingPriorityQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_SchedulingPriorityQosPolicy {
    enum _DDS_SchedulingPriorityQosPolicyKind kind;
};

struct _DDS_SchedulingQosPolicy ;
OMG_DDS_API v_copyin_result __DDS_SchedulingQosPolicy__copyIn(c_type dbType, const DDS::SchedulingQosPolicy *from, struct _DDS_SchedulingQosPolicy *to);
OMG_DDS_API void __DDS_SchedulingQosPolicy__copyOut(const void *_from, void *_to);
struct _DDS_SchedulingQosPolicy {
    struct _DDS_SchedulingClassQosPolicy scheduling_class;
    struct _DDS_SchedulingPriorityQosPolicy scheduling_priority_kind;
    c_long scheduling_priority;
};

struct _DDS_DomainParticipantQos ;
OMG_DDS_API v_copyin_result __DDS_DomainParticipantQos__copyIn(c_type dbType, const dds::domain::qos::DomainParticipantQos *from, struct _DDS_DomainParticipantQos *to);
OMG_DDS_API void __DDS_DomainParticipantQos__copyOut(const void *_from, void *_to);
struct _DDS_DomainParticipantQos {
    struct _DDS_UserDataQosPolicy user_data;
    struct _DDS_EntityFactoryQosPolicy entity_factory;
    struct _DDS_SchedulingQosPolicy watchdog_scheduling;
    struct _DDS_SchedulingQosPolicy listener_scheduling;
};

struct _DDS_TopicQos ;
OMG_DDS_API v_copyin_result __DDS_TopicQos__copyIn(c_type dbType, const dds::topic::qos::TopicQos *from, struct _DDS_TopicQos *to);
OMG_DDS_API void __DDS_TopicQos__copyOut(const void *_from, void *_to);
struct _DDS_TopicQos {
    struct _DDS_TopicDataQosPolicy topic_data;
    struct _DDS_DurabilityQosPolicy durability;
    struct _DDS_DurabilityServiceQosPolicy durability_service;
    struct _DDS_DeadlineQosPolicy deadline;
    struct _DDS_LatencyBudgetQosPolicy latency_budget;
    struct _DDS_LivelinessQosPolicy liveliness;
    struct _DDS_ReliabilityQosPolicy reliability;
    struct _DDS_DestinationOrderQosPolicy destination_order;
    struct _DDS_HistoryQosPolicy history;
    struct _DDS_ResourceLimitsQosPolicy resource_limits;
    struct _DDS_TransportPriorityQosPolicy transport_priority;
    struct _DDS_LifespanQosPolicy lifespan;
    struct _DDS_OwnershipQosPolicy ownership;
};

struct _DDS_DataWriterQos ;
OMG_DDS_API v_copyin_result __DDS_DataWriterQos__copyIn(c_type dbType, const dds::pub::qos::DataWriterQos *from, struct _DDS_DataWriterQos *to);
OMG_DDS_API void __DDS_DataWriterQos__copyOut(const void *_from, void *_to);
struct _DDS_DataWriterQos {
    struct _DDS_DurabilityQosPolicy durability;
    struct _DDS_DeadlineQosPolicy deadline;
    struct _DDS_LatencyBudgetQosPolicy latency_budget;
    struct _DDS_LivelinessQosPolicy liveliness;
    struct _DDS_ReliabilityQosPolicy reliability;
    struct _DDS_DestinationOrderQosPolicy destination_order;
    struct _DDS_HistoryQosPolicy history;
    struct _DDS_ResourceLimitsQosPolicy resource_limits;
    struct _DDS_TransportPriorityQosPolicy transport_priority;
    struct _DDS_LifespanQosPolicy lifespan;
    struct _DDS_UserDataQosPolicy user_data;
    struct _DDS_OwnershipQosPolicy ownership;
    struct _DDS_OwnershipStrengthQosPolicy ownership_strength;
    struct _DDS_WriterDataLifecycleQosPolicy writer_data_lifecycle;
};

struct _DDS_PublisherQos ;
OMG_DDS_API v_copyin_result __DDS_PublisherQos__copyIn(c_type dbType, const dds::pub::qos::PublisherQos *from, struct _DDS_PublisherQos *to);
OMG_DDS_API void __DDS_PublisherQos__copyOut(const void *_from, void *_to);
struct _DDS_PublisherQos {
    struct _DDS_PresentationQosPolicy presentation;
    struct _DDS_PartitionQosPolicy partition;
    struct _DDS_GroupDataQosPolicy group_data;
    struct _DDS_EntityFactoryQosPolicy entity_factory;
};

struct _DDS_DataReaderQos ;
OMG_DDS_API v_copyin_result __DDS_DataReaderQos__copyIn(c_type dbType, const dds::sub::qos::DataReaderQos *from, struct _DDS_DataReaderQos *to);
OMG_DDS_API void __DDS_DataReaderQos__copyOut(const void *_from, void *_to);
struct _DDS_DataReaderQos {
    struct _DDS_DurabilityQosPolicy durability;
    struct _DDS_DeadlineQosPolicy deadline;
    struct _DDS_LatencyBudgetQosPolicy latency_budget;
    struct _DDS_LivelinessQosPolicy liveliness;
    struct _DDS_ReliabilityQosPolicy reliability;
    struct _DDS_DestinationOrderQosPolicy destination_order;
    struct _DDS_HistoryQosPolicy history;
    struct _DDS_ResourceLimitsQosPolicy resource_limits;
    struct _DDS_UserDataQosPolicy user_data;
    struct _DDS_OwnershipQosPolicy ownership;
    struct _DDS_TimeBasedFilterQosPolicy time_based_filter;
    struct _DDS_ReaderDataLifecycleQosPolicy reader_data_lifecycle;
    struct _DDS_SubscriptionKeyQosPolicy subscription_keys;
    struct _DDS_ReaderLifespanQosPolicy reader_lifespan;
    struct _DDS_ShareQosPolicy share;
};

struct _DDS_SubscriberQos ;
OMG_DDS_API v_copyin_result __DDS_SubscriberQos__copyIn(c_type dbType, const dds::sub::qos::SubscriberQos *from, struct _DDS_SubscriberQos *to);
OMG_DDS_API void __DDS_SubscriberQos__copyOut(const void *_from, void *_to);
struct _DDS_SubscriberQos {
    struct _DDS_PresentationQosPolicy presentation;
    struct _DDS_PartitionQosPolicy partition;
    struct _DDS_GroupDataQosPolicy group_data;
    struct _DDS_EntityFactoryQosPolicy entity_factory;
    struct _DDS_ShareQosPolicy share;
};

#endif /* DDS_DCPS_BUILTINTOPICSSPLTYPES_H */
