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

#ifndef EPROSIMA_DDS_CORE_TINSTANCEHANDLE_IMPL_HPP_
#define EPROSIMA_DDS_CORE_TINSTANCEHANDLE_IMPL_HPP_

/**
 * @file
 */
#include <dds/core/detail/Value.hpp>

/*
 * OMG PSM class declaration
 */
#include <dds/core/InstanceHandle.hpp>

namespace dds {
namespace core {

InstanceHandle::InstanceHandle()
    : Value()
{
}

InstanceHandle::InstanceHandle(
        const detail::InstanceHandle& arg0)
    : Value(arg0)
{
}

/*
InstanceHandle::InstanceHandle(
        const dds::core::null_type& nullHandle)
    : Value(nullHandle)
{
}
*/

InstanceHandle::InstanceHandle(
        const InstanceHandle& other)
    : Value(other.delegate())
{
}

InstanceHandle::~InstanceHandle()
{
}

InstanceHandle& InstanceHandle::operator=(
        const InstanceHandle& that)
{
    //To implement
    if(this != &that)
    {
        this->delegate() = that.delegate();
    }
    return *this;
}

bool InstanceHandle::operator ==(
        const InstanceHandle& that) const
{
    //To implement
    return this->delegate() == that.delegate();
}

bool InstanceHandle::operator <(
        const InstanceHandle& that) const
{
    //To implement
    return this->delegate() < that.delegate();
}

bool InstanceHandle::operator >(
        const InstanceHandle& that) const
{
    //To implement
    for (int i = 0; i < 16; ++i)
    {
        if (delegate().value[i] > that.delegate().value[i])
        {
            return true;
        }
    }
    return false;
    //return this->delegate() > that.delegate();
}

const InstanceHandle InstanceHandle::nil()
{
    //To implement
    //dds::core::null_type nt;
    //static InstanceHandle nil_handle(nt);
    //return nil_handle;
    return InstanceHandle();
}

bool InstanceHandle::is_nil() const
{
    //To implement
    return !this->delegate().isDefined();
}

} //namespace core
} //namespace dds

/*
inline std::ostream& operator <<(
        std::ostream& os,
        const dds::core::InstanceHandle& h)
{
    os << h.delegate();
    return os;
}
*/

#endif //EPROSIMA_DDS_CORE_TINSTANCEHANDLE_IMPL_HPP_
