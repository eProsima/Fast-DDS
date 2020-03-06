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

#ifndef EPROSIMA_DDS_CORE_TENTITYQOS_IMPL_HPP_
#define EPROSIMA_DDS_CORE_TENTITYQOS_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/TEntityQos.hpp>

namespace dds {
namespace core {

template<typename DELEGATE>
TEntityQos<DELEGATE>::TEntityQos()
    : dds::core::Value<DELEGATE>()
{
}

template<typename DELEGATE>
TEntityQos<DELEGATE>::TEntityQos(
        const TEntityQos& other)
    : dds::core::Value<DELEGATE>(other.delegate())
{
}

/** @cond
 * Somehow, these cause functions duplicates in doxygen documentation.
 */
template<typename DELEGATE>
template<typename T>
TEntityQos<DELEGATE>::TEntityQos(
        const TEntityQos<T>& qos)
    : dds::core::Value<DELEGATE>(qos.delegate())
{
}

/** @endcond */

template<typename DELEGATE>
TEntityQos<DELEGATE>::~TEntityQos()
{
}

/** @cond
 * Somehow, these cause functions duplicates in doxygen documentation.
 */
template<typename DELEGATE>
template<typename POLICY>
TEntityQos<DELEGATE>& TEntityQos<DELEGATE>::policy(
        const POLICY& p)
{
    //To implement
    //    this->dds::core::Value<DELEGATE>::delegate().policy(p);
    //    return *this;
}

template<typename DELEGATE>
template<typename POLICY>
const POLICY& TEntityQos<DELEGATE>::policy() const
{
    //To implement
    //    return this->delegate().template policy<POLICY>();
}

template<typename DELEGATE>
template<typename POLICY>
POLICY& TEntityQos<DELEGATE>::policy()
{
    //To implement
    //    return this->delegate().template policy<POLICY>();
}

template<typename DELEGATE>
template<typename POLICY>
TEntityQos<DELEGATE>& TEntityQos<DELEGATE>::operator <<(
        const POLICY& p)
{
    //To implement
    //    this->policy(p);
    //    return *this;
}

template<typename DELEGATE>
template<typename POLICY>
const TEntityQos<DELEGATE>& TEntityQos<DELEGATE>::operator >>(
        POLICY& p) const
{
    //To implement
    //    p = this->policy<POLICY>();
    //    return *this;
}

template<typename DELEGATE>
template<typename T>
TEntityQos<DELEGATE>& TEntityQos<DELEGATE>::operator =(
        const TEntityQos<T>& other)
{
    //To implement
    //    if(this != (TEntityQos<DELEGATE>*)&other)
    //    {
    //        this->d_ = other.delegate();
    //    }
    //    return *this;
}

/** @endcond */

} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_TENTITYQOS_IMPL_HPP_
