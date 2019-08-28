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

#ifndef EPROSIMA_DDS_TOPIC_DETAIL_FIND_HPP_
#define EPROSIMA_DDS_TOPIC_DETAIL_FIND_HPP_


//TODO: Fix when find is implemented
//#include <org/opensplice/topic/find.hpp>


#include <string>


namespace dds {
namespace topic {

template <typename TOPIC>
TOPIC find(const dds::domain::DomainParticipant& dp, const std::string& topic_name)
{
    //TODO: Fix when find is implemented
//    ISOCPP_REPORT_STACK_DDS_BEGIN(dp);
//    TOPIC t = org::opensplice::topic::finder<TOPIC, typename TOPIC::DELEGATE_T>::find(dp, topic_name);

//    return t;
}


} //namespace topic
} //namespace dds

#endif //EPROSIMA_DDS_TOPIC_DETAIL_FIND_HPP_
