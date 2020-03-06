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

#ifndef EPROSIMA_DDS_SUB_DETAIL_TRANK_IMPL_HPP_
#define EPROSIMA_DDS_SUB_DETAIL_TRANK_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/sub/Rank.hpp>

namespace dds {
namespace sub {

template<typename DELEGATE>
TRank<DELEGATE>::TRank()
{
}

template<typename DELEGATE>
TRank<DELEGATE>::TRank(
        int32_t s,
        int32_t a,
        int32_t ag)
    : dds::core::Value<DELEGATE>(s, a, ag)
{
}

template<typename DELEGATE>
int32_t TRank<DELEGATE>::absolute_generation() const
{
    //To implement
    //    return this->delegate().absolute_generation();
}

template<typename DELEGATE>
inline int32_t TRank<DELEGATE>::generation() const
{
    //To implement
    //    return this->delegate().generation();
}

template<typename DELEGATE>
inline int32_t TRank<DELEGATE>::sample() const
{
    //To implement
    //    return this->delegate().sample();
}

} //namespace sub
} //namespace dds

#endif //EPROSIMA_DDS_SUB_DETAIL_TRANK_IMPL_HPP_
