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

#ifndef EPROSIMA_DDS_TOPIC_TANYTOPIC_IMPL_HPP_
#define EPROSIMA_DDS_TOPIC_TANYTOPIC_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/topic/AnyTopic.hpp>

namespace dds {
namespace topic {

template<typename DELEGATE>
TAnyTopic<DELEGATE>::~TAnyTopic()
{
}

template<typename DELEGATE>
dds::topic::qos::TopicQos
TAnyTopic<DELEGATE>::qos() const
{
    return this->delegate()->get_qos();
}

template<typename DELEGATE>
void TAnyTopic<DELEGATE>::qos(
        const dds::topic::qos::TopicQos& qos)
{
    this->delegate()->set_qos(qos);
}

template<typename DELEGATE>
TAnyTopic<DELEGATE>& TAnyTopic<DELEGATE>::operator <<(
        const dds::topic::qos::TopicQos& qos)
{
    this->qos(qos);
    return *this;
}

template<typename DELEGATE>
const TAnyTopic<DELEGATE>& TAnyTopic<DELEGATE>::operator >>(
        dds::topic::qos::TopicQos& qos) const
{
    qos = this->qos();
    return *this;
}

template<typename DELEGATE>
dds::core::status::InconsistentTopicStatus TAnyTopic<DELEGATE>::inconsistent_topic_status() const
{
    dds::core::status::InconsistentTopicStatus status;
    this->delegate()->get_inconsistent_topic_status(status);
    return status;
}

template<typename DELEGATE>
dds::core::cond::StatusCondition* TAnyTopic<DELEGATE>::status_condition()
{
    return this->delegate()->get_statuscondition();
}

} //namespace topic
} //namespace dds

#endif //EPROSIMA_DDS_TOPIC_TANYTOPIC_IMPL_HPP_
