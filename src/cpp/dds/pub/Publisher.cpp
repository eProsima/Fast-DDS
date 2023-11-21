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
#include <dds/pub/Publisher.hpp>
#include <dds/pub/PublisherListener.hpp>

namespace dds {
namespace pub {

Publisher::Publisher(
        const dds::domain::DomainParticipant& dp)
    : ::dds::core::Reference<detail::Publisher>(
        new detail::Publisher(
            dp.delegate().get(),
            dp.default_publisher_qos(),
            nullptr,
            dds::core::status::StatusMask::all()))
    , participant_(&dp)
{
}

Publisher::Publisher(
        const dds::domain::DomainParticipant& dp,
        const qos::PublisherQos& qos,
        PublisherListener* listener,
        const dds::core::status::StatusMask& mask)
    : ::dds::core::Reference<detail::Publisher>(
        new detail::Publisher(
            dp.delegate().get(), qos, listener, mask))
    , participant_(&dp)
{
}

Publisher::~Publisher()
{
}

const qos::PublisherQos& Publisher::qos() const
{
    return this->delegate()->get_qos();
}

void Publisher::qos(
        const qos::PublisherQos& pqos)
{
    ReturnCode_t code = this->delegate()->set_qos(pqos);
    if (code == eprosima::fastdds::dds::RETCODE_IMMUTABLE_POLICY)
    {
        throw dds::core::ImmutablePolicyError("Immutable Qos");
    }
    else if (code == eprosima::fastdds::dds::RETCODE_INCONSISTENT_POLICY)
    {
        throw dds::core::InconsistentPolicyError("Inconsistent Qos");
    }
    else if (code == eprosima::fastdds::dds::RETCODE_UNSUPPORTED)
    {
        throw dds::core::UnsupportedError("Unsupported values on DomainParticipantQos");
    }
}

Publisher& Publisher::operator <<(
        const qos::PublisherQos& qos)
{
    this->qos(qos);
    return *this;
}

Publisher& Publisher::operator >>(
        qos::PublisherQos& qos)
{
    qos = this->qos();
    return *this;
}

Publisher& Publisher::default_datawriter_qos(
        const qos::DataWriterQos& dwqos)
{
    ReturnCode_t code = this->delegate()->set_default_datawriter_qos(dwqos);
    if (code == eprosima::fastdds::dds::RETCODE_INCONSISTENT_POLICY)
    {
        throw dds::core::InconsistentPolicyError("Inconsistent Qos");
    }
    if (code == eprosima::fastdds::dds::RETCODE_UNSUPPORTED)
    {
        throw dds::core::UnsupportedError("Unsupported values on DataWriterQos");
    }
    return *this;
}

qos::DataWriterQos Publisher::default_datawriter_qos() const
{
    return this->delegate()->get_default_datawriter_qos();
}

//void Publisher::listener(
//        Listener* plistener,
//        const dds::core::status::StatusMask& /*event_mask*/)
//{
//    delegate()->set_listener(plistener /*, event_mask*/);
//}

//typename Publisher::Listener* Publisher::listener() const
//{
//    return dynamic_cast<Listener*>(delegate()->get_listener());
//}

//void Publisher::wait_for_acknowledgments(
//        const dds::core::Duration& timeout)
//{
//    eprosima::fastrtps::Duration_t max_wait(static_cast<int32_t>(timeout.sec()), timeout.nanosec());
//    delegate()->wait_for_acknowledgments(max_wait);
//}

const dds::domain::DomainParticipant& Publisher::participant() const
{
    return *participant_;
}

} //namespace pub
} //namespace dds
