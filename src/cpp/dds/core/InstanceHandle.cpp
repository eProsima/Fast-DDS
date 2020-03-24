/*
 * Copyright 2020, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 * All rights reserved.
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
 */

#include <dds/core/InstanceHandle.hpp>

namespace dds {
namespace core {


InstanceHandle::InstanceHandle()
    : Value<detail::InstanceHandle>()
{
}

InstanceHandle::InstanceHandle(
            const null_type& /*nullHandle*/)
    : Value<detail::InstanceHandle>()
{
}

InstanceHandle::InstanceHandle(
            const InstanceHandle& other)
    : Value<detail::InstanceHandle>(other)
{
}

InstanceHandle::~InstanceHandle()
{
}

InstanceHandle::InstanceHandle(
            const detail::InstanceHandle& arg0)
    : Value<detail::InstanceHandle>(arg0)
{
}

InstanceHandle& InstanceHandle::operator =(
            const InstanceHandle& that)
{
    delegate() = that.delegate();
    return *this;
}

bool InstanceHandle::operator ==(
            const InstanceHandle& that) const
{
    return delegate() == that.delegate();
}

const InstanceHandle InstanceHandle::nil()
{
    return nil_handle_;
}

bool InstanceHandle::is_nil() const
{
    return *this == nil_handle_;
}

const InstanceHandle InstanceHandle::nil_handle_;

} //namespace core
} //namespace dds

inline std::ostream& operator <<(
        std::ostream& os,
        const dds::core::InstanceHandle& h)
{
    os << h.delegate();
    return os;
}


