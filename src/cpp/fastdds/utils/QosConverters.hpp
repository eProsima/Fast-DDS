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
#include <fastdds/dds/domain/qos/DomainParticipantExtendedQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/qos/ReplierQos.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>

#include <xmlparser/attributes/ParticipantAttributes.hpp>
#include <xmlparser/attributes/PublisherAttributes.hpp>
#include <xmlparser/attributes/ReplierAttributes.hpp>
#include <xmlparser/attributes/RequesterAttributes.hpp>
#include <xmlparser/attributes/SubscriberAttributes.hpp>
#include <xmlparser/attributes/TopicAttributes.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace utils {

/**
 * Obtains the DataWriterQos from the PublisherAttributes provided.
 *
 * @param [out] qos Pointer to the QoS to write on
 * @param [in] attr Pointer to the attributes from which to obtain data
 */
void set_qos_from_attributes(
        DataWriterQos& qos,
        const xmlparser::PublisherAttributes& attr);

/**
 * Obtains the DataReaderQos from the SubscriberAttributes provided.
 *
 * @param [out] qos Pointer to the QoS to write on
 * @param [in] attr Pointer to the attributes from which to obtain data
 */
void set_qos_from_attributes(
        DataReaderQos& qos,
        const xmlparser::SubscriberAttributes& attr);

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
 * @param [in, out] qos The DomainParticipantQos to set
 * @param [in] attr The RTPSParticipantAttributes from which the @c qos is set.
 */
void set_qos_from_attributes(
        DomainParticipantQos& qos,
        const eprosima::fastdds::rtps::RTPSParticipantAttributes& attr);

/**
 * @brief Fill DomainParticipantExtendedQos from a given attributes ParticipantAttributes object
 *
 * For the case of the non-binary properties, instead of the ParticipantAttributes overriding the
 * property list in the DomainParticipantExtendedQos, a merge is performed in the following manner:
 *
 * - If any property from the ParticipantAttributes is not in the DomainParticipantExtendedQos, then it is appended
 *   to the DomainParticipantExtendedQos.
 * - If any property from the ParticipantAttributes property is also in the DomainParticipantExtendedQos, then the
 *   value in the DomainParticipantExtendedQos is overridden with that of the ParticipantAttributes.
 *
 * @param [in, out] extended_qos The DomainParticipantExtendedQos to set
 * @param [in] attr The ParticipantAttributes from which the @c extended_qos is set.
 */
void set_extended_qos_from_attributes(
        DomainParticipantExtendedQos& extended_qos,
        const eprosima::fastdds::xmlparser::ParticipantAttributes& attr);

/**
 * Obtains the RTPSParticipantAttributes from the DomainParticipantQos provided.
 *
 * @param [out] attr Pointer to the attributes from which to obtain data
 * @param [in] qos Pointer to the QoS to write on
 */
void set_attributes_from_qos(
        fastdds::rtps::RTPSParticipantAttributes& attr,
        const DomainParticipantQos& qos);

/**
 * Obtains the ParticipantAttributes from the DomainParticipantExtendedQos provided.
 *
 * @param [out] attr Pointer to the attributes from which to obtain data
 * @param [in] extended_qos Pointer to the QoS to write on
 */
void set_attributes_from_extended_qos(
        eprosima::fastdds::xmlparser::ParticipantAttributes& attr,
        const DomainParticipantExtendedQos& extended_qos);

/**
 * Obtains the TopicQos from the TopicAttributes provided.
 *
 * @param [out] qos Pointer to the QoS to write on
 * @param [in] attr Pointer to the attributes from which to obtain data
 */
void set_qos_from_attributes(
        TopicQos& qos,
        const xmlparser::TopicAttributes& attr);

/**
 * Obtains the SubscriberQos from the SubscriberAttributes provided.
 *
 * @param [out] qos Pointer to the QoS to write on
 * @param [in] attr Pointer to the attributes from which to obtain data
 */
void set_qos_from_attributes(
        SubscriberQos& qos,
        const xmlparser::SubscriberAttributes& attr);

/**
 * Obtains the PublisherQos from the PublisherAttributes provided.
 *
 * @param [out] qos Pointer to the QoS to write on
 * @param [in] attr Pointer to the attributes from which to obtain data
 */
void set_qos_from_attributes(
        PublisherQos& qos,
        const xmlparser::PublisherAttributes& attr);

/**
 * Obtains the ReplierQos from the ReplierAttributes provided.
 *
 * @param [out] qos Pointer to the QoS to write on
 * @param [in] attr Pointer to the attributes from which to obtain data
 */
void set_qos_from_attributes(
        ReplierQos& qos,
        const xmlparser::ReplierAttributes& attr);

/**
 * Obtains the RequesterQos from the RequesterAttributes provided.
 *
 * @param [out] qos Pointer to the QoS to write on
 * @param [in] attr Pointer to the attributes from which to obtain data
 */
void set_qos_from_attributes(
        RequesterQos& qos,
        const xmlparser::RequesterAttributes& attr);

} /* namespace utils */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_UTILS_QOS_CONVERTERS_HPP_ */
