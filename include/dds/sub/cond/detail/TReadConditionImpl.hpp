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

#ifndef EPROSIMA_DDS_CORE_COND_TREADCONDITION_IMPL_HPP_
#define EPROSIMA_DDS_CORE_COND_TREADCONDITION_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/sub/cond/ReadCondition.hpp>
//TODO: Fix when ReadConditionDelegate is implemented
//#include <org/opensplice/sub/cond/ReadConditionDelegate.hpp>

namespace dds {
namespace sub {
namespace cond {

template<typename DELEGATE>
TReadCondition<DELEGATE>::TReadCondition(
        const dds::sub::AnyDataReader& dr,
        const dds::sub::status::DataState& status)
{
    //To implement
}

/** @cond
 * Somehow, these cause functions duplicates in doxygen documentation.
 */
template<typename DELEGATE>
template<typename FUN>
TReadCondition<DELEGATE>::TReadCondition(
            const dds::sub::AnyDataReader& dr,
            const dds::sub::status::DataState& status,
            FUN& functor)
{
    //To implement
}

template<typename DELEGATE>
template<typename FUN>
TReadCondition<DELEGATE>::TReadCondition(
            const dds::sub::AnyDataReader& dr,
            const dds::sub::status::DataState& status,
            const FUN& functor)
{
    //To implement
}
/** @endcond */

template<typename DELEGATE>
TReadCondition<DELEGATE>::~TReadCondition() { }

template<typename DELEGATE>
const dds::sub::status::DataState TReadCondition<DELEGATE>::state_filter() const
{
    //To implement
}

template<typename DELEGATE>
const AnyDataReader& TReadCondition<DELEGATE>::data_reader() const
{
    //To implement
}

} //namespace cond
} //namespace sub

namespace core {
namespace cond {

template<typename DELEGATE>
TCondition<DELEGATE>::TCondition(
        /*const dds::sub::cond::TReadCondition<org::opensplice::sub::cond::ReadConditionDelegate>& h*/)
{
    //To implement
}

template<typename DELEGATE>
TCondition<DELEGATE>& TCondition<DELEGATE>::operator=(
        /*const dds::sub::cond::TReadCondition<org::opensplice::sub::cond::ReadConditionDelegate>& rhs*/)
{
    //To implement
}

} //namespace cond
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_COND_TREADCONDITION_IMPL_HPP_
