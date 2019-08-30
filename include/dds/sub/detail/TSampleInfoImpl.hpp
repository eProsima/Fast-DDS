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

#ifndef EPROSIMA_DDS_SUB_DETAIL_TSAMPLEINFO_IMPL_HPP_
#define EPROSIMA_DDS_SUB_DETAIL_TSAMPLEINFO_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/sub/SampleInfo.hpp>

namespace dds {
namespace sub {

template<typename DELEGATE>
TSampleInfo<DELEGATE>::TSampleInfo()
{
}

template<typename DELEGATE>
const dds::core::Time TSampleInfo<DELEGATE>::timestamp() const
{
    //To implement
//    return this->delegate().timestamp();
}

template<typename DELEGATE>
const dds::sub::status::DataState TSampleInfo<DELEGATE>::state() const
{
    //To implement
//    return this->delegate().state();
}

template<typename DELEGATE>
dds::sub::GenerationCount TSampleInfo<DELEGATE>::generation_count() const
{
    //To implement
//    return this->delegate().generation_count();
}

template<typename DELEGATE>
dds::sub::Rank TSampleInfo<DELEGATE>::rank() const
{
    //To implement
//    return this->delegate().rank();
}

template<typename DELEGATE>
bool TSampleInfo<DELEGATE>::valid() const
{
    //To implement
//    return this->delegate().valid();
}

template<typename DELEGATE>
dds::core::InstanceHandle TSampleInfo<DELEGATE>::instance_handle() const
{
    //To implement
//    return this->delegate().instance_handle();
}

template<typename DELEGATE>
dds::core::InstanceHandle TSampleInfo<DELEGATE>::publication_handle() const
{
    //To implement
//    return this->delegate().publication_handle();
}

} //namespace sub
} //namespace dds

#endif //EPROSIMA_DDS_SUB_DETAIL_TSAMPLEINFO_IMPL_HPP_
