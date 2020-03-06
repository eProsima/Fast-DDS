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

#ifndef EPROSIMA_DDS_TOPIC_TTOPICDESCRIPTION_HPP_
#define EPROSIMA_DDS_TOPIC_TTOPICDESCRIPTION_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/topic/TopicDescription.hpp>

namespace dds {
namespace topic {

template<typename DELEGATE>
TTopicDescription<DELEGATE>::~TTopicDescription()
{
}

template<typename DELEGATE>
const std::string& TTopicDescription<DELEGATE>::name() const
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->name();
}

template<typename DELEGATE>
const std::string& TTopicDescription<DELEGATE>::type_name() const
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->type_name();
}

template<typename DELEGATE>
const dds::domain::DomainParticipant& TTopicDescription<DELEGATE>::domain_participant() const
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->domain_participant();
}

} //namespace topic
} //namespace dds

#endif //EPROSIMA_DDS_TOPIC_TTOPICDESCRIPTION_HPP_
