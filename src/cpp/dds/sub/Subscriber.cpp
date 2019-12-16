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
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/sub/Subscriber.hpp>
#include <dds/sub/SubscriberListener.hpp>

namespace dds {
namespace sub {

Subscriber::Subscriber(
        const ::dds::domain::DomainParticipant& dp)
    : ::dds::core::Reference<detail::Subscriber>(
        new detail::Subscriber(dp,
        dp.default_subscriber_qos(),
        nullptr,
        dds::core::status::StatusMask::none()))
{
}

Subscriber::Subscriber(
        const ::dds::domain::DomainParticipant& dp,
        const dds::sub::qos::SubscriberQos& qos,
        dds::sub::SubscriberListener* listener,
        const dds::core::status::StatusMask& mask)
    : ::dds::core::Reference<detail::Subscriber>(new detail::Subscriber(dp, qos, listener, mask))
{
}

Subscriber::~Subscriber() {}

void Subscriber::notify_datareaders()
{
    delegate()->notify_datareaders();
}

void Subscriber::listener(
        Listener* listener,
        const dds::core::status::StatusMask& event_mask)
{
    delegate()->set_listener(listener, event_mask);
}

typename Subscriber::Listener* Subscriber::listener() const
{
    return dynamic_cast<Listener*>(delegate()->get_listener());
}


const dds::sub::qos::SubscriberQos& Subscriber::qos() const
{
    return delegate()->get_qos();
}

void Subscriber::qos(
        const dds::sub::qos::SubscriberQos& sqos)
{
    delegate()->set_qos(sqos);
}

dds::sub::qos::DataReaderQos Subscriber::default_datareader_qos() const
{
    dds::sub::qos::DataReaderQos rqos;
    delegate()->get_default_datareader_qos(rqos);
    return rqos;
}

Subscriber& Subscriber::default_datareader_qos(
        const dds::sub::qos::DataReaderQos& qos)
{
    delegate()->set_default_datareader_qos(qos);
    return *this;
}

const dds::domain::DomainParticipant& Subscriber::participant() const
{
    eprosima::fastdds::dds::DomainParticipant p = delegate()->get_participant();
    std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> ptr(&p);
    participant_->delegate().swap(ptr);

    return *participant_;
}

Subscriber& Subscriber::operator <<(
        const dds::sub::qos::SubscriberQos& qos)
{
    this->qos(qos);
    return *this;
}

const Subscriber& Subscriber::operator >>(
        dds::sub::qos::SubscriberQos& qos) const
{
    qos = this->qos();
    return *this;
}

} //namespace sub
} //namespace dds
