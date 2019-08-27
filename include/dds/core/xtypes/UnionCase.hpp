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

#ifndef OMG_DDS_CORE_XTYPES_UNION_CASE_HPP_
#define OMG_DDS_CORE_XTYPES_UNION_CASE_HPP_

#include <dds/core/xtypes/detail/UnionCase.hpp>
#include <dds/core/xtypes/MemberType.hpp>

#include <dds/core/Reference.hpp>

namespace dds{
namespace core{
namespace xtypes{

template<
        typename T,
        template <typename Q> class DELEGATE>
class TUnionCase : public Reference< DELEGATE<T> >
{
public:
    TUnionCase();

    TUnionCase(
            T discriminator,
            const MemberType& member);

    T discriminator();

    const MemberType& member();
};

template<typename T>
struct UnionCase : public TUnionCase <T, detail::UnionCase> {} ;

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_T_UNION_CASE_HPP_
