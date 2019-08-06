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

#ifndef OMG_DDS_CORE_XTYPES_PRIMITIVE_TYPES_HPP_
#define OMG_DDS_CORE_XTYPES_PRIMITIVE_TYPES_HPP_

#include <dds/core/xtypes/TypeKind.hpp>
#include <dds/core/xtypes/DynamicType.hpp>

namespace dds {
namespace core {
namespace xtypes {

template<typename T>
struct dynamic_type_traits
{
    static constexpr TypeKind TYPE_ID = TypeKind::NO_TYPE;
    static constexpr const char* const NAME = "no_type";
};

template<typename T, typename DELEGATE>
class TPrimitiveType : public TDynamicType<DELEGATE>
{
public:
    TPrimitiveType()
        : DynamicType(
                dynamic_type_traits<T>::TYPE_ID,
                dynamic_type_traits<T>::NAME)
    {
    }
};


/**
 * Primitive type constructor. This function can be used
 * as follows:
 *
 * DynamicType int16Type = primitive_type<int16_t>();
 */
template<typename T>
TPrimitiveType<T, detail::DynamicType> primitive_type()
{
    static TPrimitiveType<T, detail::DynamicType> t;
    return t;
}


} //namespace xtypes
} //namespace core
} //namespace dds

// This include should be at the end since it provides
// template specializations.
#include <dds/core/xtypes/detail/PrimitiveTypes.hpp>

#endif //OMG_DDS_CORE_XTYPES_PRIMITIVE_TYPES_HPP_
