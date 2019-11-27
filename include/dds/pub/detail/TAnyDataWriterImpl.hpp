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

//#ifndef EPROSIMA_DDS_PUB_DETAIL_TANYDATAWRITER_HPP_
//#define EPROSIMA_DDS_PUB_DETAIL_TANYDATAWRITER_HPP_

/**
 * @file
 */


// TODO This class currently isn't being compiled. When needed, most of its methods will require conversions.

/*
 * OMG PSM class declaration
 */
#include <dds/pub/AnyDataWriter.hpp>

namespace dds {
namespace pub {

template<typename DELEGATE>
TAnyDataWriter<DELEGATE>::~TAnyDataWriter()
{
    publisher_ = nullptr;
}

template<typename DELEGATE>
const dds::pub::Publisher&
TAnyDataWriter<DELEGATE>::publisher() const
{
    return *publisher_;
}

template<typename DELEGATE>
const dds::topic::TopicDescription& TAnyDataWriter<DELEGATE>::topic_description() const
{
    return this->delegate()->topic_description();
}

template<typename DELEGATE>
qos::DataWriterQos TAnyDataWriter<DELEGATE>::qos() const
{
    return this->delegate()->get_qos();
}

template<typename DELEGATE>
void TAnyDataWriter<DELEGATE>::qos(
        const qos::DataWriterQos& qos)
{
    this->delegate()->set_qos(qos);
}

template<typename DELEGATE>
TAnyDataWriter<DELEGATE>& TAnyDataWriter<DELEGATE>::operator <<(
        const dds::pub::qos::DataWriterQos& qos)
{
    this->qos(qos);
    return *this;
}

template<typename DELEGATE>
const TAnyDataWriter<DELEGATE>& TAnyDataWriter<DELEGATE>::operator >>(
        qos::DataWriterQos& qos) const
{
    qos = this->qos();
    return *this;
}

template<typename DELEGATE>
void TAnyDataWriter<DELEGATE>::wait_for_acknowledgments(
        const dds::core::Duration& timeout)
{
    this->delegate()->wait_for_acknowledgments(timeout);
}

template<typename DELEGATE>
const dds::core::status::LivelinessLostStatus TAnyDataWriter<DELEGATE>::liveliness_lost_status()
{
    return this->delegate()->get_liveliness_lost_status();
}

template<typename DELEGATE>
const dds::core::status::OfferedDeadlineMissedStatus TAnyDataWriter<DELEGATE>::offered_deadline_missed_status()
{
    return this->delegate()->get_offered_deadline_missed_status();
}

template<typename DELEGATE>
const dds::core::status::OfferedIncompatibleQosStatus TAnyDataWriter<DELEGATE>::offered_incompatible_qos_status()
{
    return this->delegate()->get_offered_incompatible_qos_status();
}

template<typename DELEGATE>
const dds::core::status::PublicationMatchedStatus TAnyDataWriter<DELEGATE>::publication_matched_status()
{
    return this->delegate()->get_publication_matched_status();
}

template<typename DELEGATE>
void TAnyDataWriter<DELEGATE>::assert_liveliness()
{
    this->delegate()->assert_liveliness();
}

} //namespace pub
} //namespace dds


//#endif //EPROSIMA_DDS_PUB_DETAIL_TANYDATAWRITER_HPP_
