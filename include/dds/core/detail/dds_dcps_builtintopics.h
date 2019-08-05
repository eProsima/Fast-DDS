#ifndef _DDS_DCPS_BUILTINTOPICS_H_
#define _DDS_DCPS_BUILTINTOPICS_H_

#include <dds/core/detail/dds_builtinTopics.h>

namespace DDS
{
    typedef dds::core::Time                                                 Time_t;

    typedef org::opensplice::core::policy::SchedulingKind::Type             SchedulingClassQosPolicyKind;
    typedef org::opensplice::core::policy::SchedulingKind::Type             SchedulingClassQosPolicy;
    typedef org::opensplice::core::policy::SchedulingPriorityKind::Type     SchedulingPriorityQosPolicyKind;
    typedef org::opensplice::core::policy::SchedulingPriorityKind::Type     SchedulingPriorityQosPolicy;
    typedef org::opensplice::core::policy::TScheduling<
                    org::opensplice::core::policy::SchedulingDelegate>      SchedulingQosPolicy;

    typedef dds::domain::qos::DomainParticipantQos                          DomainParticipantQos;
    typedef dds::topic::qos::TopicQos                                       TopicQos;
    typedef dds::pub::qos::DataWriterQos                                    DataWriterQos;
    typedef dds::pub::qos::PublisherQos                                     PublisherQos;
    typedef dds::sub::qos::DataReaderQos                                    DataReaderQos;
    typedef dds::sub::qos::SubscriberQos                                    SubscriberQos;
}

#endif /* _DDS_DCPS_BUILTINTOPICS_H_ */
