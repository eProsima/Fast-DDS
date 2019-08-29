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

#ifndef EPROSIMA_DDS_TOPIC_DETAIL_DISCOVER_HPP_
#define EPROSIMA_DDS_TOPIC_DETAIL_DISCOVER_HPP_

#include <dds/topic/AnyTopic.hpp>
#include <dds/topic/Topic.hpp>
//TODO: Fix when discovery is implmented
//#include <org/opensplice/topic/discovery.hpp>

#include <string>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace topic {

template<typename TOPIC>
TOPIC discover(
    const dds::domain::DomainParticipant& dp,
    const std::string& topic_name,
    const dds::core::Duration& timeout)
{
    //To implement
}


template<
        typename ANYTOPIC,
        typename FwdIterator>
uint32_t discover(
    const dds::domain::DomainParticipant& dp,
    FwdIterator begin,
    uint32_t max_size)
{
    //To implement
}

template<
        typename ANYTOPIC,
        typename BinIterator>
uint32_t discover_all(
    const dds::domain::DomainParticipant& dp,
    BinIterator begin)
{
    //To implement
}


template<typename FwdIterator>
void ignore(
    const dds::domain::DomainParticipant& dp,
    FwdIterator begin, FwdIterator end)
{
    //To implement
}

} //namespace topic
} //namespace dds)

/** @endcond */

#endif //EPROSIMA_DDS_TOPIC_DETAIL_DISCOVER_HPP_
