/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
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
 */

#ifndef OMG_DDS_CORE_SAFEENUMERATION_HPP_
#define OMG_DDS_CORE_SAFEENUMERATION_HPP_

namespace dds {
namespace core {

/**
 * SafeEnum provides a wrapper for enumerated types in a typesafe
 * manner.
 *
 * SafeEnums allow specification of the underlying type,
 * do not implictly convert to integers, and resolve scoping issues.
 */
template<
    typename Def,
    typename Inner = typename Def::Type>
class SafeEnum : public Def
{
public:

    constexpr SafeEnum(
            Inner v)
        : val(v)
    {
    }

    Inner underlying() const
    {
        return val;
    }

    bool operator ==(
            const SafeEnum& s) const
    {
        return this->val == s.val;
    }

    bool operator !=(
            const SafeEnum& s) const
    {
        return this->val != s.val;
    }

    bool operator <(
            const SafeEnum& s) const
    {
        return this->val <  s.val;
    }

    bool operator <=(
            const SafeEnum& s) const
    {
        return this->val <= s.val;
    }

    bool operator >(
            const SafeEnum& s) const
    {
        return this->val >  s.val;
    }

    bool operator >=(
            const SafeEnum& s) const
    {
        return this->val >= s.val;
    }

private:

    Inner val;

};

} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_SAFEENUMERATION_HPP_

