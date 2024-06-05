// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_UTILS_QOS_CONVERTERS_HPP_
#define _FASTDDS_UTILS_QOS_CONVERTERS_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/qos/ReplierQos.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/attributes/TopicAttributes.h>

#include <xmlparser/attributes/PublisherAttributes.hpp>
#include <xmlparser/attributes/ReplierAttributes.hpp>
#include <xmlparser/attributes/RequesterAttributes.hpp>
#include <xmlparser/attributes/SubscriberAttributes.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace utils {

using fastdds::TopicAttributes;

/**
 * Obtains the DataWriterQos from the PublisherAttributes provided.
 *
 * @param[out] qos Pointer to the QoS to write on
 * @param[in] attr Pointer to the attributes from which to obtain data
 */
void set_qos_from_attributes(
        DataWriterQos& qos,
        const PublisherAttributes& attr);

/**
 * Obtains the DataReaderQos from the SubscriberAttributes provided.
 *
 * @param[out] qos Pointer to the QoS to write on
 * @param[in] attr Pointer to the attributes from which to obtain data
 */
void set_qos_from_attributes(
        DataReaderQos& qos,
        const SubscriberAttributes& attr);

/**
 * @brief Fill DomainParticipantQos from a given attributes RTPSParticipantAttributes object
 *
 * For the case of the non-binary properties, instead of the RTPSParticipantAttributes overriding the
 * property list in the DomainParticipantQos, a merge is performed in the following manner:
 *
 * - If any property from the RTPSParticipantAttributes is not in the DomainParticipantQos, then it is appended
 *   to the DomainParticipantQos.
 * - If any property from the RTPSParticipantAttributes property is also in the DomainParticipantQos, then the
 *   value in the DomainParticipantQos is overridden with that of the RTPSParticipantAttributes.
 *
 * @param[in, out] qos The DomainParticipantQos to set
 * @param[in] attr The RTPSParticipantAttributes from which the @c qos is set.
 */
void set_qos_from_attributes(
        DomainParticipantQos& qos,
        const eprosima::fastdds::rtps::RTPSParticipantAttributes& attr);

/**
 * Obtains the RTPSParticipantAttributes from the DomainParticipantQos provided.
 *
 * @param[out] attr Pointer to the attributes from which to obtain data
 * @param[in] qos Pointer to the QoS to write on
 */
void set_attributes_from_qos(
        fastdds::rtps::RTPSParticipantAttributes& attr,
        const DomainParticipantQos& qos);

/**
 * Obtains the TopicQos from the TopicAttributes provided.
 *
 * @param[out] qos Pointer to the QoS to write on
 * @param[in] attr Pointer to the attributes from which to obtain data
 */
void set_qos_from_attributes(
        TopicQos& qos,
        const TopicAttributes& attr);

/**
 * Obtains the SubscriberQos from the SubscriberAttributes provided.
 *
 * @param[out] qos Pointer to the QoS to write on
 * @param[in] attr Pointer to the attributes from which to obtain data
 */
void set_qos_from_attributes(
        SubscriberQos& qos,
        const SubscriberAttributes& attr);

/**
 * Obtains the PublisherQos from the PublisherAttributes provided.
 *
 * @param[out] qos Pointer to the QoS to write on
 * @param[in] attr Pointer to the attributes from which to obtain data
 */
void set_qos_from_attributes(
        PublisherQos& qos,
        const PublisherAttributes& attr);

/**
 * Obtains the ReplierQos from the ReplierAttributes provided.
 *
 * @param[out] qos Pointer to the QoS to write on
 * @param[in] attr Pointer to the attributes from which to obtain data
 */
void set_qos_from_attributes(
        ReplierQos& qos,
        const ReplierAttributes& attr);

/**
 * Obtains the RequesterQos from the RequesterAttributes provided.
 *
 * @param[out] qos Pointer to the QoS to write on
 * @param[in] attr Pointer to the attributes from which to obtain data
 */
void set_qos_from_attributes(
        RequesterQos& qos,
        const RequesterAttributes& attr);

} /* namespace utils */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_UTILS_QOS_CONVERTERS_HPP_ */
