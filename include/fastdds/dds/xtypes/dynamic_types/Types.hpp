// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file Types.hpp
 */

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__TYPES_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__TYPES_HPP

#include "detail/dynamic_language_binding.hpp"
#include "type_traits.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

constexpr MemberId MEMBER_ID_INVALID {0x0FFFFFFF};

class DynamicData;

template<>
struct traits<DynamicData> : public object_traits<DynamicData>
{
    using base_type = DynamicData;
};

class DynamicDataFactory;

template<>
struct traits<DynamicDataFactory> : public object_traits<DynamicDataFactory>
{
    using base_type = DynamicDataFactory;
};

class DynamicType;

template<>
struct traits<DynamicType> : public object_traits<DynamicType>
{
    using base_type = DynamicType;
};

class DynamicTypeBuilder;

template<>
struct traits<DynamicTypeBuilder> : public object_traits<DynamicTypeBuilder>
{
    using base_type = DynamicTypeBuilder;
};

class DynamicTypeBuilderFactory;

template<>
struct traits<DynamicTypeBuilderFactory> : public object_traits<DynamicTypeBuilderFactory>
{
    using base_type = DynamicTypeBuilderFactory;
};

class DynamicTypeMember;

template<>
struct traits<DynamicTypeMember> : public object_traits<DynamicTypeMember>
{
    using base_type = DynamicTypeMember;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__TYPES_HPP
