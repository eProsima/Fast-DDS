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
#include <dds/domain/DomainParticipantListener.hpp>
#include <dds/core/Exception.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/rtps/common/Time_t.h>

namespace dds {
namespace domain {

DomainParticipant::DomainParticipant(
        uint32_t did)
    : dds::core::Reference<detail::DomainParticipant>(
        new detail::DomainParticipant(
            did,
            eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT,
            nullptr,
            dds::core::status::StatusMask::all()))
{
}

DomainParticipant::DomainParticipant(
        uint32_t id,
        const dds::domain::qos::DomainParticipantQos& qos,
        dds::domain::DomainParticipantListener* listener,
        const dds::core::status::StatusMask& mask)
    : dds::core::Reference<detail::DomainParticipant>(
        new detail::DomainParticipant(
            id,
            qos,
            listener,
            mask))
{
}

DomainParticipant::~DomainParticipant()
{
}

//void DomainParticipant::listener(
//        Listener* /*listener*/,
//        const ::dds::core::status::StatusMask& /*event_mask*/)
//{
//    this->delegate()->set_listener(listener /*, event_mask*/);
//}

//typename DomainParticipant::Listener* DomainParticipant::listener() const
//{
//    return dynamic_cast<Listener*>(this->delegate()->get_listener());
//}

//const dds::domain::qos::DomainParticipantQos& DomainParticipant::qos() const
//{
//        return this->delegate()->get_qos();
//}

//void DomainParticipant::qos(
//        const dds::domain::qos::DomainParticipantQos& /*qos*/)
//{
//        this->delegate()->set_qos(qos);
//}

//uint32_t DomainParticipant::domain_id() const
//{
//        return this->delegate()->get_domain_id();
//}

//void DomainParticipant::assert_liveliness()
//{
//        this->delegate()->assert_liveliness();
//}

//bool DomainParticipant::contains_entity(
//        const ::dds::core::InstanceHandle& /*handle*/)
//{
//        return this->delegate()->contains_entity(handle);
//}

//dds::core::Time DomainParticipant::current_time() const
//{
//        eprosima::fastrtps::Time_t now;
//        this->delegate()->get_current_time(now);
//        return core::Time(now.seconds, now.nanosec);
//}

dds::domain::qos::DomainParticipantQos DomainParticipant::default_participant_qos()
{
    qos::DomainParticipantQos qos;
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->get_default_participant_qos(qos);
    return qos;
}

void DomainParticipant::default_participant_qos(
        const ::dds::domain::qos::DomainParticipantQos& qos)
{
    if (eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_default_participant_qos(qos) ==
            eprosima::fastrtps::types::ReturnCode_t::RETCODE_INCONSISTENT_POLICY)
    {
        throw dds::core::InconsistentPolicyError("Inconsistent Qos");
    }
}

dds::pub::qos::PublisherQos DomainParticipant::default_publisher_qos() const
{
    return this->delegate()->get_default_publisher_qos();
}

DomainParticipant& DomainParticipant::default_publisher_qos(
        const ::dds::pub::qos::PublisherQos& qos)
{
    this->delegate()->set_default_publisher_qos(qos);
    return *this;
}

dds::sub::qos::SubscriberQos DomainParticipant::default_subscriber_qos() const
{
    return this->delegate()->get_default_subscriber_qos();
}

DomainParticipant& DomainParticipant::default_subscriber_qos(
        const ::dds::sub::qos::SubscriberQos& qos)
{
    this->delegate()->set_default_subscriber_qos(qos);
    return *this;
}

//dds::topic::qos::TopicQos DomainParticipant::default_topic_qos() const
//{
//    return this->delegate()->get_default_topic_qos();
//}

//DomainParticipant& DomainParticipant::default_topic_qos(
//        const dds::topic::qos::TopicQos& qos)
//{
//    this->delegate()->set_default_topic_qos(qos);
//    return *this;
//}

//DomainParticipant& DomainParticipant::operator <<(
//        const dds::domain::qos::DomainParticipantQos& qos)
//{
//    this->qos(qos);
//    return *this;
//}

//const DomainParticipant& DomainParticipant::operator >>(
//        dds::domain::qos::DomainParticipantQos& qos) const
//{
//    qos = this->qos();
//    return *this;
//}

} //namespace domain
} //namespace dds
