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

#ifndef EPROSIMA_DDS_CORE_REFERENCE_IMPL_HPP_
#define EPROSIMA_DDS_CORE_REFERENCE_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/Reference.hpp>

namespace dds {
namespace core {

// Implementation
template<typename DELEGATE>
Reference<DELEGATE>::Reference(
        null_type&)
    : impl_()
{
}

template<typename DELEGATE>
Reference<DELEGATE>::Reference(
        const Reference& ref)
    : impl_(ref.impl_)
{
}

template<typename DELEGATE>
template<typename D>
Reference<DELEGATE>::Reference(
        const Reference<D>& ref)
{
    //To Implement
}

template<typename DELEGATE>
Reference<DELEGATE>::Reference(
        DELEGATE_T* p)
    : impl_(p)
{
}

template<typename DELEGATE>
Reference<DELEGATE>::Reference(
        const DELEGATE_REF_T& p)
    : impl_(p)
{
    //OMG_DDS_LOG("MM", "Reference(DELEGATE_REF_T& p)");
}

template<typename DELEGATE>
Reference<DELEGATE>::~Reference()
{
}

template<typename DELEGATE>
Reference<DELEGATE>::operator DELEGATE_REF_T() const
{
    //To implement
}

template<typename DELEGATE>
template<typename R>
bool Reference<DELEGATE>::operator==(
        const R& ref) const
{
    //To implement
}

template<typename DELEGATE>
template<typename R>
bool Reference<DELEGATE>::operator!=(
        const R& ref) const
{
    //To implement
}

template<typename DELEGATE>
template<typename D>
Reference<DELEGATE>& Reference<DELEGATE>::operator=(
        const Reference<D>& that)
{
    //To implement
}

template<typename DELEGATE>
template<typename R>
Reference<DELEGATE>& Reference<DELEGATE>::operator=(
        const R& rhs)
{
   //To implement
}

template<typename DELEGATE>
Reference<DELEGATE>& Reference<DELEGATE>::operator=(
        const null_type)
{
    //To implement
}

template<typename DELEGATE>
bool Reference<DELEGATE>::is_nil() const
{
    //To implement
}

template<typename DELEGATE>
bool Reference<DELEGATE>::operator==(
        const null_type) const
{
    //To implement
}

template<typename DELEGATE>
bool Reference<DELEGATE>::operator!=(
        const null_type) const
{
    //To implement
}

template<typename DELEGATE>
const typename Reference<DELEGATE>::DELEGATE_REF_T& Reference<DELEGATE>::delegate() const
{
    //To implement
}

template<typename DELEGATE>
typename Reference<DELEGATE>::DELEGATE_REF_T& Reference<DELEGATE>::delegate()
{
    //To implement
}

template<typename DELEGATE>
DELEGATE* Reference<DELEGATE>::operator->()
{
    //To implement
}

template<typename DELEGATE>
const DELEGATE* Reference<DELEGATE>::operator->() const
{
    //To implement
}

template<typename DELEGATE>
Reference<DELEGATE>::operator const typename Reference<DELEGATE>::DELEGATE_REF_T& () const
{
    //To implement
}

template<typename DELEGATE>
Reference<DELEGATE>::operator typename Reference<DELEGATE>::DELEGATE_REF_T& ()
{
    //To implement
}

template<typename DELEGATE>
void Reference<DELEGATE>::set_ref(
        DELEGATE_T* p)
{
    //To implement
}


template<class D>
bool operator == (
        null_type, const Reference<D>& r)
{
    //To implement
}

template<class D>
bool operator != (
        null_type, const Reference<D>& r)
{
    //To implement
}

} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_REFERENCE_IMPL_HPP_
