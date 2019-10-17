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

#ifndef EPROSIMA_DDS_SUB_COND_QUERYCONDITION_IMPL_HPP_
#define EPROSIMA_DDS_SUB_COND_QUERYCONDITION_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/sub/cond/QueryCondition.hpp>

namespace dds {
namespace sub {
namespace cond {

template<typename FUN>
QueryCondition::QueryCondition(
        const dds::sub::Query& query,
        const dds::sub::status::DataState& status, const FUN& functor)
    : dds::sub::cond::TReadCondition<DELEGATE>(
          new DELEGATE(query.delegate().data_reader(), query.delegate(), status, functor))
{
    //To implement
}

template<typename T>
QueryCondition::QueryCondition(
        const dds::sub::DataReader<T>& dr,
        const std::string& expression,
        const std::vector<std::string>& params,
        const dds::sub::status::DataState& status)
    : dds::sub::cond::TReadCondition<DELEGATE>(
          new DELEGATE(dds::sub::AnyDataReader(dr), expression, params, status))
{
    //To implement
}


template<typename T, typename FUN>
QueryCondition::QueryCondition(
        const dds::sub::DataReader<T>& dr,
        const std::string& expression,
        const std::vector<std::string>& params,
        const dds::sub::status::DataState& status,
        const FUN& functor)
    : dds::sub::cond::TReadCondition<DELEGATE>(
          new DELEGATE(dds::sub::AnyDataReader(dr), expression, params, status, functor))
{
    //To implement
}

template<typename FWIterator>
void QueryCondition::parameters(
        const FWIterator& begin,
        const FWIterator end)
{
    //To implement
}

} //namespace cond
} //namespace sub
} //namespace dds

#endif //EPROSIMA_DDS_SUB_COND_QUERYCONDITION_IMPL_HPP_
