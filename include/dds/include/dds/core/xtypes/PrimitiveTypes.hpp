/* Copyright 2010, Object Management Group, Inc.
* Copyright 2010, PrismTech, Corp.
* Copyright 2010, Real-Time Innovations, Inc.
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
#ifndef OMG_DDS_CORE_XTYPES_PRIMITIVE_TYPES_HPP_
#define OMG_DDS_CORE_XTYPES_PRIMITIVE_TYPES_HPP_

#include <dds/core/xtypes/DynamicType.hpp>
#include <dds/core/xtypes/TypeKind.hpp>

namespace dds
{
namespace core
{
namespace xtypes
{
template <typename T>
class TPrimitiveType;

template <typename T>
struct dynamic_type_traits
{
    static const TypeKind TYPE_ID = TypeKind::NO_TYPE;
    static const std::string NAME;
};

/**
 * Primitive type constructor. This function can be used
 * as follows:
 *
 * DynamicType int16Type = PrimitiveType<int16_t>();
 */
template <typename T>
TPrimitiveType<T> PrimitiveType();
}
}
}

template <typename T>
class dds::core::xtypes::TPrimitiveType : public dds::core::xtypes::DynamicType
{
public:
    TPrimitiveType() : DynamicType(dynamic_type_traits<T>::TYPE_ID, dynamic_type_traits<T>::NAME) { }
};

template <typename T>
dds::core::xtypes::TPrimitiveType<T>
dds::core::xtypes::PrimitiveType()
{
    static dds::core::xtypes::TPrimitiveType<T> t();
    return t;
}

// This include should be at the end since it provides
// template specializations.
#include <dds/core/xtypes/detail/PrimitiveTypes.hpp>

#endif /* OMG_DDS_CORE_XTYPES_PRIMITIVE_TYPES_HPP_ */
