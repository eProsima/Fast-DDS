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

#include <dds/core/Reference.hpp>

namespace dds {
namespace core {

template<typename DELEGATE>
Reference<DELEGATE>::Reference(
        dds::core::null_type&)
{
}

template<typename DELEGATE>
Reference<DELEGATE>::Reference(
        const Reference& ref)
{
    (void) ref;
}
template<typename DELEGATE>
template<typename D>
Reference<DELEGATE>::Reference(
        const Reference<D>& ref)
{
    (void) ref;
}

template<typename DELEGATE>
Reference<DELEGATE>::Reference(
        DELEGATE_T* p)
{
    (void) p;
}

template<typename DELEGATE>
Reference<DELEGATE>::Reference(
        const DELEGATE_REF_T& p)
{
    (void) p;
}


template<typename DELEGATE>
Reference<DELEGATE>::~Reference()
{
}

template<typename DELEGATE>
Reference<DELEGATE>::operator DELEGATE_REF_T() const
{
}

template<typename DELEGATE>
template<typename R>
bool Reference<DELEGATE>::operator==(
        const R& ref) const
{
    (void) ref;
    return false;
}
template<typename DELEGATE>
template<typename R>
bool Reference<DELEGATE>::operator!=(
        const R& ref) const
{
    (void) ref;
    return false;
}

template<typename DELEGATE>
template<typename D>
Reference<DELEGATE>& Reference<DELEGATE>::operator=(
        const Reference<D>& that)
{
    (void) that;
}

template<typename DELEGATE>
template<typename R>
Reference<DELEGATE>& Reference<DELEGATE>::operator=(
        const R& rhs)
{
    (void) rhs;
}

template<typename DELEGATE>
Reference<DELEGATE>& Reference<DELEGATE>::operator=(
        const null_type)
{
}

template<typename DELEGATE>
bool Reference<DELEGATE>::is_nil() const
{
    return false;
}


template<typename DELEGATE>
bool Reference<DELEGATE>::operator==(
        const null_type) const
{
    return false;
}

template<typename DELEGATE>
bool Reference<DELEGATE>::operator!=(
        const null_type nil) const
{
    (void) nil;
    return false;
}

template<typename DELEGATE>
void* Reference<DELEGATE>::operator new(
        size_t)
{
}

template<typename DELEGATE>
DELEGATE* Reference<DELEGATE>::operator->()
{
}

template<typename DELEGATE>
const DELEGATE* Reference<DELEGATE>::operator->() const
{
}

template<typename DELEGATE>
Reference<DELEGATE>::operator DELEGATE_REF_T& ()
{

}

template<typename DELEGATE>
Reference<DELEGATE>::operator const DELEGATE_REF_T& () const
{
}

template<typename DELEGATE>
typename Reference<DELEGATE>::DELEGATE_REF_T& Reference<DELEGATE>::delegate()
{
}

template <typename DELEGATE>
const typename Reference<DELEGATE>::DELEGATE_REF_T& Reference<DELEGATE>::delegate() const
{
}

template<typename DELEGATE>
void Reference<DELEGATE>::set_ref(
        DELEGATE_T* p)
{
    (void) p;
}

template<class D>
bool operator ==(
        dds::core::null_type,
        const dds::core::Reference<D>& r)
{
    (void) r;
}

template<class D>
bool operator !=(
        dds::core::null_type,
        const dds::core::Reference<D>& r)
{
    (void) r;
}

} //namespace core
} //namespace dds
