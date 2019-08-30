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

#ifndef EPROSIMA_DDS_CORE_DETAIL_VALUE_HPP_
#define EPROSIMA_DDS_CORE_DETAIL_VALUE_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/core/Value.hpp>

namespace dds {
namespace core {

/**
 * @internal @todo We can't assume that the compiler supports variadic templates, yet.
 * This code should be refactored to take advantage of compilers that do support variadic
 * templates.
 */

template<typename D>
Value<D>::Value()
{
}

template<typename D>
Value<D>::Value(
        const Value& p)
    : d_(p.d_)
{
}

template<typename D>
template<typename ARG>
Value<D>::Value(
        const ARG& arg)
    : d_(arg)
{
}

template<typename D>
template<
        typename ARG1,
        typename ARG2>
Value<D>::Value(
        const ARG1& arg1,
        const ARG2& arg2)
    : d_(arg1, arg2)
{
}

template<typename D>
template<
        typename ARG1,
        typename ARG2,
        typename ARG3>
Value<D>::Value(
        const ARG1& arg1,
        const ARG2& arg2,
        const ARG3& arg3)
    : d_(arg1, arg2, arg3)
{
}

template<typename D>
template<
        typename ARG1,
        typename ARG2,
        typename ARG3,
        typename ARG4>
Value<D>::Value(
        const ARG1& arg1,
        const ARG2& arg2,
        const ARG3& arg3,
        const ARG4& arg4)
    : d_(arg1, arg2, arg3, arg4)
{
}

template<typename D>
template<
        typename ARG1,
        typename ARG2,
        typename ARG3,
        typename ARG4,
        typename ARG5>
Value<D>::Value(
        const ARG1& arg1,
        const ARG2& arg2,
        const ARG3& arg3,
        const ARG4& arg4,
        const ARG5& arg5)
    : d_(arg1, arg2, arg3, arg4, arg5)
{
}

template<typename D>
template<
        typename ARG1,
        typename ARG2,
        typename ARG3,
        typename ARG4,
        typename ARG5,
        typename ARG6>
Value<D>::Value(
        const ARG1& arg1,
        const ARG2& arg2,
        const ARG3& arg3,
        const ARG4& arg4,
        const ARG5& arg5,
        const ARG6& arg6)
    : d_(arg1, arg2, arg3, arg4, arg5, arg6)
{
}

template<typename D>
Value<D>::~Value()
{
}

template<typename D>
Value<D>& Value<D>::operator =(
        const Value& other)
{
    //To implement
//    if(this != &other)
//    {
//        d_ = other.d_;
//    }
//    return *this;
}

template<typename D>
bool Value<D>::operator ==(
        const Value& other) const
{
    //To implement
//    return (d_ == other.d_);
}

template<typename D>
bool Value<D>::operator !=(
        const Value& other) const
{
    //To implement
//    return !(d_ == other.d_);
}

template<typename D>
const D* Value<D>::operator->() const
{
    //To implement
//    return &d_;
}

template<typename D>
D* Value<D>::operator->()
{
    //To implement
//    return &d_;
}

template<typename D>
const D& Value<D>::delegate() const
{
    //To implement
//    return d_;
}

template<typename D>
D& Value<D>::delegate()
{
    //To implement
//    return d_;
}

template<typename D>
Value<D>::operator D& ()
{
    //To implement
//    return d_;
}

template<typename D>
Value<D>::operator const D& () const
{
    //To implement
//    return d_;
}

} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_DETAIL_VALUE_HPP_
