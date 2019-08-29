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

#ifndef EPROSIMA_DDS_SUB_DETAIL_FIND_HPP_
#define EPROSIMA_DDS_SUB_DETAIL_FIND_HPP_

/**
 * @file
 */

#include <string>
#include <vector>

#include <dds/sub/DataReader.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/sub/status/DataState.hpp>
#include <dds/topic/TopicDescription.hpp>

//#include <org/opensplice/sub/SubscriberDelegate.hpp>
//#include <org/opensplice/sub/BuiltinSubscriberDelegate.hpp>
//#include <org/opensplice/sub/AnyDataReaderDelegate.hpp>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace sub {
namespace detail {


/*********************************************************************
 * To be able to properly copy found readers into the given iterators
 * (which are related to typed DataReaders or AnyDataReaders), we have
 * to specialize the find() template functions.
 * But template functions specialization is not supported by C++. So,
 * we have to use these helper classes to get the specialization.
 *********************************************************************/

//TODO: Fix when AnyDataReaderDelegate is implemented
//typedef std::vector<org::opensplice::sub::AnyDataReaderDelegate::ref_type> base_readers_vector;
//typedef std::vector<org::opensplice::sub::AnyDataReaderDelegate::ref_type>::iterator base_readers_iterator;

/*
 * Copy helper class for typed readers.
 */
template<
        typename READER,
        typename ITERATOR>
class ReadersCopySpecialization
{
public:
    static
    bool copy(/*base_readers_iterator base_iter,*/
              ITERATOR typed_iter) {
        //To implement
    }
};

/*
 * Copy helper class for any readers.
 */
template<typename ITERATOR>
class ReadersCopySpecialization<::dds::sub::AnyDataReader, ITERATOR>
{
public:
    static
    bool copy(/*base_readers_iterator base_iter,*/
              ITERATOR any_iter) {
        //To implement
    }
};


/*
 * Copy helper class for list of readers.
 */
template<
        typename READER,
        typename ITERATOR>
class ReadersCopy
{
public:
    static
    uint32_t copy(/*base_readers_vector base_readers,*/
                  ITERATOR begin,
                  uint32_t max_size)
    {
        //To implement
    }

    static
    uint32_t copy(/*base_readers_vector base_readers,*/
                  ITERATOR begin)
    {
        //To implement
    }
};

} //namespace detail



template<
        typename READER,
        typename FwdIterator>
uint32_t find(
        const Subscriber& sub,
         const std::string &topic_name,
         FwdIterator begin,
        uint32_t max_size)
{
    //To implement
}

template<
        typename READER,
        typename BinIterator>
uint32_t find(
        const Subscriber& sub,
        const std::string &topic_name,
        BinIterator begin)
{
    //To implement
}

template<
        typename READER,
        typename T,
        typename FwdIterator>
uint32_t find(
        const Subscriber& sub,
        const dds::topic::TopicDescription& topic_description,
        FwdIterator begin,
        uint32_t max_size)
{
    //To implement
}

template<
        typename READER,
        typename T,
        typename BinIterator>
uint32_t find(
        const Subscriber& sub,
        const dds::topic::TopicDescription& topic_description,
        BinIterator begin)
{
    //To implement
}

template<
        typename READER,
        typename FwdIterator>
uint32_t find(
        const Subscriber& sub,
        const status::DataState& rs,
        FwdIterator begin,
        uint32_t max_size)
{
    //To implement
}

template<
        typename READER,
        typename BinIterator>
uint32_t find(
        const Subscriber& sub,
        const status::DataState& rs,
        BinIterator begin)
{
    //To implement
}

} //namespace sub
} //namespace dds

/** @endcond */

#endif //EPROSIMA_DDS_SUB_DETAIL_FIND_HPP_

