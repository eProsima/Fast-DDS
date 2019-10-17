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

/**
 * @file DomainParticipantImpl.cpp
 */

#include <dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

namespace dds {
namespace domain {

DomainParticipant::DomainParticipant(
        uint32_t did)
    : ::dds::core::Reference(
            new detail::DomainParticipant(
                    did,
                    eprosima::fastdds::dds::PARTICIPANT_DEFAULT_QOS,
                    NULL,
                    dds::core::status::StatusMask::none()))
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    this->delegate()->init(this->impl_);
}

DomainParticipant::DomainParticipant(
        uint32_t id,
        const dds::domain::qos::DomainParticipantQos& qos,
        dds::domain::DomainParticipantListener* listener,
        const dds::core::status::StatusMask& mask) :
    ::dds::core::Reference(
            new detail::DomainParticipant(
                    id,
                    qos,
                    listener,
                    mask))
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    this->delegate()->init(this->impl_);
}

DomainParticipant::~DomainParticipant()
{
}

void DomainParticipant::listener(
        Listener* listener,
        const ::dds::core::status::StatusMask& event_mask)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->listener(listener, event_mask);
}

typename DomainParticipant::Listener* DomainParticipant::listener() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->listener();
}

const dds::domain::qos::DomainParticipantQos& DomainParticipant::qos() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->qos();
}

void DomainParticipant::qos(
        const dds::domain::qos::DomainParticipantQos& qos)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->qos(qos);
}

uint32_t DomainParticipant::domain_id() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->domain_id();
}

void DomainParticipant::assert_liveliness()
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->assert_liveliness();
}

bool DomainParticipant::contains_entity(
        const ::dds::core::InstanceHandle& handle)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->contains_entity(handle);
}

dds::core::Time DomainParticipant::current_time() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->current_time();
}

dds::domain::qos::DomainParticipantQos DomainParticipant::default_participant_qos()
{
    //To implement
//    ISOCPP_REPORT_STACK_NC_BEGIN();
    return detail::DomainParticipant::default_participant_qos();
}

void DomainParticipant::default_participant_qos(
        const ::dds::domain::qos::DomainParticipantQos& qos)
{
    //To implement
//    ISOCPP_REPORT_STACK_NC_BEGIN();
    detail::DomainParticipant::default_participant_qos(qos);
}

dds::pub::qos::PublisherQos DomainParticipant::default_publisher_qos() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->default_publisher_qos();
}

DomainParticipant& DomainParticipant::default_publisher_qos(
        const ::dds::pub::qos::PublisherQos& qos)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    this->delegate()->default_publisher_qos(qos);
    return *this;
}

dds::sub::qos::SubscriberQos DomainParticipant::default_subscriber_qos() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->default_subscriber_qos();
}

DomainParticipant& DomainParticipant::default_subscriber_qos(
        const ::dds::sub::qos::SubscriberQos& qos)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->default_subscriber_qos(qos);
    return *this;
}

dds::topic::qos::TopicQos DomainParticipant::default_topic_qos() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->default_topic_qos();
}

DomainParticipant& DomainParticipant::default_topic_qos(
        const dds::topic::qos::TopicQos& qos)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->default_topic_qos(qos);
    return *this;
}

DomainParticipant& DomainParticipant::operator <<(
        const dds::domain::qos::DomainParticipantQos& qos)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->qos(qos);
    return *this;
}

const DomainParticipant& DomainParticipant::operator >>(
        dds::domain::qos::DomainParticipantQos& qos) const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    qos = this->qos();
    return *this;
}

} //namespace domain
} //namespace dds
