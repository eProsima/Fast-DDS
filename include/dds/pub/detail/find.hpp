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

#ifndef EPROSIMA_DDS_PUB_DETAIL_FIND_HPP_
#define EPROSIMA_DDS_PUB_DETAIL_FIND_HPP_

#include <string>

#include <dds/pub/DataWriter.hpp>
#include <dds/pub/Publisher.hpp>
//#include <org/opensplice/pub/PublisherDelegate.hpp>
//#include <org/opensplice/pub/AnyDataWriterDelegate.hpp>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace pub {

template<
        typename WRITER,
        typename FwdIterator>
uint32_t find(
        const Publisher& pub,
        const std::string& topic_name,
        FwdIterator begin,
        int32_t max_size)
{
    //To implement
}


template<
        typename WRITER,
        typename BinIterator>
uint32_t find(
        const Publisher& pub,
        const std::string& topic_name,
        BinIterator begin)
{
    //To implement
}


} //namespace pub
} //namespace dds

/** @endcond */

#endif //EPROSIMA_DDS_PUB_DETAIL_FIND_HPP_
