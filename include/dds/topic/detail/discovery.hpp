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

namespace dds
{
namespace topic
{

template <typename TOPIC>
TOPIC
discover(
    const dds::domain::DomainParticipant& dp,
    const std::string& topic_name,
    const dds::core::Duration& timeout)
{
    //TODO: Fix when discovery is implmented
//    ISOCPP_REPORT_STACK_DDS_BEGIN(dp);
//    TOPIC t = org::opensplice::topic::lookup_topic<TOPIC, typename TOPIC::DELEGATE_T>::discover(dp, topic_name, timeout);

//    return t;
}


template <typename ANYTOPIC, typename FwdIterator>
uint32_t
discover(
    const dds::domain::DomainParticipant& dp,
    FwdIterator begin,
    uint32_t max_size)
{
    //TODO: Fix when discovery is implmented
//	ISOCPP_REPORT_STACK_DDS_BEGIN(dp);
//    std::vector<ANYTOPIC> list;

//    org::opensplice::topic::lookup_topic<ANYTOPIC, typename ANYTOPIC::DELEGATE_T>::discover(dp, list, max_size);

//    FwdIterator fit = begin;
//    for (typename std::vector<ANYTOPIC>::const_iterator it = list.begin(); it != list.end(); ++it) {
//       *fit++ = *it;
//    }

//    return list.size();
}

template <typename ANYTOPIC, typename BinIterator>
uint32_t
discover_all(
    const dds::domain::DomainParticipant& dp,
    BinIterator begin)
{
    //TODO: Fix when discovery is implmented
//	ISOCPP_REPORT_STACK_DDS_BEGIN(dp);
//    std::vector<ANYTOPIC> list;

//    org::opensplice::topic::lookup_topic<ANYTOPIC, typename ANYTOPIC::DELEGATE_T>::discover(dp, list, (uint32_t)dds::core::LENGTH_UNLIMITED);

//    BinIterator bit = begin;
//    for (typename std::vector<ANYTOPIC>::const_iterator it = list.begin(); it != list.end(); ++it) {
//       *bit++ = *it;
//    }

//    return list.size();
}


template <typename FwdIterator>
void
ignore(
    const dds::domain::DomainParticipant& dp,
    FwdIterator begin, FwdIterator end)
{
    //TODO: Fix when discovery is implmented
//    ISOCPP_THROW_EXCEPTION(ISOCPP_UNSUPPORTED_ERROR, "Function not currently supported");
}

}
}

/** @endcond */

#endif //EPROSIMA_DDS_TOPIC_DETAIL_DISCOVER_HPP_
