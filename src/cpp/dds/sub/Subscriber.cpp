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
    //To implement
//	ISOCPP_REPORT_STACK_DDS_BEGIN(dp);
//    this->delegate()->init(this->impl_);
}

Subscriber::Subscriber(
        const ::dds::domain::DomainParticipant& dp,
        const dds::sub::qos::SubscriberQos& qos,
        dds::sub::SubscriberListener* listener,
        const dds::core::status::StatusMask& mask)
    : ::dds::core::Reference<detail::Subscriber>(new detail::Subscriber(dp, qos, listener, mask))
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(dp);
//    this->delegate()->init(this->impl_);
}

Subscriber::~Subscriber() {}

void Subscriber::notify_datareaders()
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    this->delegate()->notify_datareaders();
}

void Subscriber::listener(
        Listener* listener,
        const dds::core::status::StatusMask& event_mask)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->listener(listener, event_mask);
}

typename Subscriber::Listener* Subscriber::listener() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->listener();
}


const dds::sub::qos::SubscriberQos& Subscriber::qos() const
{
      //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->qos();
}

void Subscriber::qos(
            const dds::sub::qos::SubscriberQos& sqos)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    this->delegate()->qos(sqos);
}

dds::sub::qos::DataReaderQos Subscriber::default_datareader_qos() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->default_datareader_qos();
}

Subscriber& Subscriber::default_datareader_qos(
    const dds::sub::qos::DataReaderQos& qos)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

//    this->delegate()->default_datareader_qos(qos);
//    return *this;
}

const dds::domain::DomainParticipant& Subscriber::participant() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->participant();
}

Subscriber& Subscriber::operator <<(
            const dds::sub::qos::SubscriberQos& qos)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

//    this->qos(qos);
//    return *this;
}

const Subscriber& Subscriber::operator >>(
            dds::sub::qos::SubscriberQos& qos) const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

//    qos = this->qos();
//    return *this;
}

} //namespace sub
} //namespace dds
