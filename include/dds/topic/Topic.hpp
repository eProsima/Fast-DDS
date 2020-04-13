// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Topic.hpp
 */

#ifndef OMG_DDS_TOPIC_HPP_
#define OMG_DDS_TOPIC_HPP_


#include <dds/topic/detail/Topic.hpp>
#include <dds/core/Entity.hpp>
#include <dds/topic/qos/TopicQos.hpp>
#include <dds/domain/DomainParticipant.hpp>

namespace dds {
namespace topic {

class TopicListener;


class Topic : public dds::core::TEntity<detail::Topic>
{
    friend class TopicImpl;
    friend class DomainParticipantImpl;

public:

    OMG_DDS_REF_TYPE_PROTECTED_DC(
            Topic,
        dds::core::TEntity,
        detail::Topic)

    OMG_DDS_IMPLICIT_REF_BASE(
            Topic)

    /**
     * Create a new Topic.
     *
     * The Topic will be created with the QoS values specified on the last
     * successful call to @link dds::domain::DomainParticipant::default_topic_qos(const ::dds::pub::qos::TopicQos& qos)
     * dp.default_topic_qos(qos) @endlink or, if the call was never made, the
     * @ref anchor_dds_pub_topic_qos_defaults "default" values.
     *
     * @param dp the domain participant
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     */
    OMG_DDS_API Topic(
            const dds::domain::DomainParticipant& dp,
            const std::string& topic_name,
            const std::string& type_name);

    /**
     * Create a new Topic.
     *
     * The Topic will be created with the given QosPolicy settings and if
     * applicable, attaches the optionally specified TopicListener to it.
     *
     * See @ref DCPS_Modules_Infrastructure_Listener "listener" for more information
     * about listeners and possible status propagation to other entities.
     *
     * @param dp the domain participant to create the Topic with.
     * @param qos a collection of QosPolicy settings for the new Topic. In case
     *            these settings are not self consistent, no Topic is created.
     * @param listener the Topic listener
     * @param mask the mask of events notified to the listener
     * @throws dds::core::Error
     *                  An internal error has occurred.
     * @throws dds::core::OutOfResourcesError
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     * @throws dds::core::InconsistentPolicyError
     *                  The parameter qos contains conflicting QosPolicy settings.
     */
    OMG_DDS_API Topic(
            const dds::domain::DomainParticipant& dp,
            const std::string& topic_name,
            const std::string& type_name,
            const qos::TopicQos& qos,
            TopicListener* listener = nullptr,
            const dds::core::status::StatusMask& mask = dds::core::status::StatusMask::none());

    /** @cond */
    virtual OMG_DDS_API ~Topic();
    /** @endcond */

private:

    dds::domain::DomainParticipant* participant_;

};

} /* namespace topic */
} /* namespace dds */

#endif /* OMG_DDS_TOPIC_HPP_ */
