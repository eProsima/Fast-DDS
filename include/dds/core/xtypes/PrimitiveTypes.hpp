/*
 * Copyright 2010, Object Management Group, Inc.
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

#include <dds/core/xtypes/DynamicType.hpp>

namespace dds {
namespace core {
namespace xtypes {

template<typename>
struct dynamic_type_traits
{
    static constexpr TypeKind TYPE_ID = TypeKind::NO_TYPE;
    static constexpr const char* NAME = "no_type";
};

#define DDS_CORE_XTYPES_PRIMITIVE(TYPE, KIND) \
template<> \
struct dynamic_type_traits<TYPE> \
{ \
    static constexpr TypeKind TYPE_ID = TypeKind::KIND; \
    static constexpr const char* NAME = #TYPE; \
};\

DDS_CORE_XTYPES_PRIMITIVE(bool, BOOLEAN_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(uint8_t, UINT_8_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(int16_t, INT_16_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(uint16_t, UINT_16_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(int32_t, INT_32_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(uint32_t, UINT_32_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(int64_t, INT_64_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(uint64_t, UINT_64_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(float, FLOAT_32_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(double, FLOAT_64_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(long double, FLOAT_128_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(char, CHAR_8_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(char32_t, CHAR_32_TYPE)

template<typename T>
class PrimitiveType : public DynamicType
{
    template<typename R>
    friend const PrimitiveType<R>& primitive_type();

    virtual size_t memory_size() const
    {
        return sizeof(T);
    };

protected:
    virtual DynamicType* clone() const
    {
        return new PrimitiveType<T>();
    }

private:
    PrimitiveType()
        : DynamicType(dynamic_type_traits<T>::TYPE_ID, dynamic_type_traits<T>::NAME)
    {}
};

template<typename T>
const PrimitiveType<T>& primitive_type()
{
    // The creation of PrimitiveType should be always created
    // by this function in order to not broken the DynamicType::Ptr
    // optimizations for PrimitiveType
    static PrimitiveType<T> p;
    return p;
}

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_PRIMITIVE_TYPES_HPP_
