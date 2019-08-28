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

#ifndef EPROSIMA_DDS_PUB_TPUBLISHER_IMPL_HPP_
#define EPROSIMA_DDS_PUB_TPUBLISHER_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/pub/Publisher.hpp>
//#include <org/opensplice/core/ReportUtils.hpp>

namespace dds {
namespace pub {

template<typename DELEGATE>
TPublisher<DELEGATE>::TPublisher(
        const dds::domain::DomainParticipant& dp)
    : ::dds::core::Reference<DELEGATE>(
            new DELEGATE(dp,
                         dp.default_publisher_qos(),
                         NULL,
                         dds::core::status::StatusMask::none()))
{
    //To implement
}

template<typename DELEGATE>
TPublisher<DELEGATE>::TPublisher(
        const dds::domain::DomainParticipant& dp,
        const qos::PublisherQos& qos,
        PublisherListener* listener,
        const dds::core::status::StatusMask& mask)
    : ::dds::core::Reference<DELEGATE>(new DELEGATE(dp, qos, listener, mask))
{
    //To implement
}

template<typename DELEGATE>
TPublisher<DELEGATE>::~TPublisher()
{
}

template<typename DELEGATE>
const qos::PublisherQos& TPublisher<DELEGATE>::qos() const
{
    //To implement
}

template<typename DELEGATE>
void TPublisher<DELEGATE>::qos(
        const qos::PublisherQos& pqos)
{
    //To implement
}

template<typename DELEGATE>
TPublisher<DELEGATE>& TPublisher<DELEGATE>::operator <<(
        const qos::PublisherQos& qos)
{
    //To implement
}

template<typename DELEGATE>
TPublisher<DELEGATE>& TPublisher<DELEGATE>::operator >>(
        qos::PublisherQos& qos)
{
    //To implement
}

template<typename DELEGATE>
TPublisher<DELEGATE>& TPublisher<DELEGATE>::default_datawriter_qos(
        const qos::DataWriterQos& dwqos)
{
    //To implement
}

template<typename DELEGATE>
qos::DataWriterQos TPublisher<DELEGATE>::default_datawriter_qos() const
{
    //To implement
}

template<typename DELEGATE>
void TPublisher<DELEGATE>::listener(
        Listener* plistener,
        const dds::core::status::StatusMask& event_mask)
{
    //To implement
}

template<typename DELEGATE>
typename TPublisher<DELEGATE>::Listener* TPublisher<DELEGATE>::listener() const
{
    //To implement
}

template<typename DELEGATE>
void TPublisher<DELEGATE>::wait_for_acknowledgments(
        const dds::core::Duration& timeout)
{
    //To implement
}

template<typename DELEGATE>
const dds::domain::DomainParticipant& TPublisher<DELEGATE>::participant() const
{
    //To implement
}

} //namespace pub
} //namespace dds

#endif //EPROSIMA_DDS_PUB_TPUBLISHER_IMPL_HPP_
