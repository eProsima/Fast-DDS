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

#ifndef EPROSIMA_DDS_DCPS_BUILTINTOPICS_H_
#define EPROSIMA_DDS_DCPS_BUILTINTOPICS_H_

#include <dds/core/detail/dds_builtinTopics.h>

namespace DDS {
typedef dds::core::Time Time_t;

//    typedef org::opensplice::core::policy::SchedulingKind::Type             SchedulingClassQosPolicyKind;
//    typedef org::opensplice::core::policy::SchedulingKind::Type             SchedulingClassQosPolicy;
//    typedef org::opensplice::core::policy::SchedulingPriorityKind::Type     SchedulingPriorityQosPolicyKind;
//    typedef org::opensplice::core::policy::SchedulingPriorityKind::Type     SchedulingPriorityQosPolicy;
//    typedef org::opensplice::core::policy::TScheduling<
//                    org::opensplice::core::policy::SchedulingDelegate>      SchedulingQosPolicy;

typedef dds::domain::qos::DomainParticipantQos DomainParticipantQos;
typedef dds::topic::qos::TopicQos TopicQos;
typedef dds::pub::qos::DataWriterQos DataWriterQos;
typedef dds::pub::qos::PublisherQos PublisherQos;
typedef dds::sub::qos::DataReaderQos DataReaderQos;
typedef dds::sub::qos::SubscriberQos SubscriberQos;
} //namespace DDS

#endif //EPROSIMA_DDS_DCPS_BUILTINTOPICS_H_
