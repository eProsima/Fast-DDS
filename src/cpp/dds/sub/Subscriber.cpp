/*
 * Copyright 2020, Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
        const dds::domain::DomainParticipant& dp)
    : ::dds::core::Reference<detail::Subscriber>(
        new detail::Subscriber(
            dp.delegate().get(),
            dp.default_subscriber_qos(),
            nullptr,
            dds::core::status::StatusMask::all()))
    , participant_(nullptr)
{
}

Subscriber::Subscriber(
        const dds::domain::DomainParticipant& dp,
        const qos::SubscriberQos& qos,
        SubscriberListener* listener,
        const dds::core::status::StatusMask& mask)
    : ::dds::core::Reference<detail::Subscriber>(
        new detail::Subscriber(
            dp.delegate().get(), qos, listener, mask))
    , participant_(&dp)
{
}

Subscriber::~Subscriber()
{
}

const qos::SubscriberQos& Subscriber::qos() const
{
    return delegate()->get_qos();
}

void Subscriber::qos(
        const qos::SubscriberQos& pqos)
{
    ReturnCode_t result = delegate()->set_qos(pqos);
    if (result == ReturnCode_t::RETCODE_IMMUTABLE_POLICY)
    {
        throw dds::core::ImmutablePolicyError("Immutable Qos");
    }
    else if (result == ReturnCode_t::RETCODE_UNSUPPORTED)
    {
        throw dds::core::UnsupportedError("Unsupported Qos");
    }
    else if (result == ReturnCode_t::RETCODE_INCONSISTENT_POLICY)
    {
        throw dds::core::InconsistentPolicyError("Inconsistent Qos");
    }
}

Subscriber& Subscriber::operator <<(
        const qos::SubscriberQos& qos)
{
    this->qos(qos);
    return *this;
}

Subscriber& Subscriber::operator >>(
        qos::SubscriberQos& qos)
{
    qos = this->qos();
    return *this;
}

Subscriber& Subscriber::default_datareader_qos(
        const qos::DataReaderQos& drqos)
{
    ReturnCode_t result = delegate()->set_default_datareader_qos(drqos);
    if ( result == ReturnCode_t::RETCODE_INCONSISTENT_POLICY)
    {
        throw dds::core::InconsistentPolicyError("Inconsistent Qos");
    }
    if ( result == ReturnCode_t::RETCODE_UNSUPPORTED)
    {
        throw dds::core::UnsupportedError("Unsupported Qos");
    }
    return *this;
}

qos::DataReaderQos Subscriber::default_datareader_qos() const
{
    return delegate()->get_default_datareader_qos();
}

//void Subscriber::listener(
//        Listener* plistener,
//        const dds::core::status::StatusMask& /*event_mask*/)
//{
//    delegate()->set_listener(plistener /*, event_mask*/);
//}

//typename Subscriber::Listener* Subscriber::listener() const
//{
//    return dynamic_cast<Listener*>(delegate()->get_listener());
//}

const dds::domain::DomainParticipant& Subscriber::participant() const
{
    return *participant_;
}

} //namespace sub
} //namespace dds

