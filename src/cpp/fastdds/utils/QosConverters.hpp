// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file QosConverters.h
 */



#ifndef _QOS_CONVERTERS_HPP_
#define _QOS_CONVERTERS_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace utils {

using fastrtps::PublisherAttributes;
using fastrtps::SubscriberAttributes;
using fastrtps::TopicAttributes;

void set_qos_from_attributes(
        DataWriterQos& qos,
        const PublisherAttributes& attr);

void set_qos_from_attributes(
        DataReaderQos& qos,
        const SubscriberAttributes& attr);

void set_qos_from_attributes(
        DomainParticipantQos& qos,
        const eprosima::fastrtps::rtps::RTPSParticipantAttributes& attr);

void set_attributes_from_qos(
        fastrtps::rtps::RTPSParticipantAttributes& attr,
        const DomainParticipantQos& qos);

void set_qos_from_attributes(
        TopicQos& qos,
        const TopicAttributes& attr);

void set_qos_from_attributes(
        SubscriberQos& qos,
        const SubscriberAttributes& attr);

void set_qos_from_attributes(
        PublisherQos& qos,
        const PublisherAttributes& attr);

} /* namespace utils */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _QOS_CONVERTERS_HPP_ */
