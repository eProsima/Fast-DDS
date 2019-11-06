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

#include <cstring>
#include <cassert>

namespace dds {
namespace core {
namespace xtypes {

/// \brief Internal struct used for enable primitive types
template<typename>
struct PrimitiveTypeKindTrait
{
    static constexpr TypeKind kind = TypeKind::NO_TYPE;
    static constexpr const char* name = "no_type";
};

#define DDS_CORE_XTYPES_PRIMITIVE(TYPE, KIND) \
template<> \
struct PrimitiveTypeKindTrait<TYPE> \
{ \
    static constexpr TypeKind kind = TypeKind::KIND; \
    static constexpr const char* name = #TYPE; \
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
DDS_CORE_XTYPES_PRIMITIVE(wchar_t, CHAR_16_TYPE)

/// \brief DynamicType representing a primitive type.
/// Primitive types can be the following: bool char wchar_t uint8_t int16_t
/// uint16_t int32_t uint32_t int64_t uint64_t float double long double.
/// A PrimitiveType represents a TypeKind::PRIMITIVE_TYPE.
template<typename T>
class PrimitiveType : public DynamicType
{
private:
    template<typename R>
    friend const DynamicType& primitive_type();

    PrimitiveType()
        : DynamicType(PrimitiveTypeKindTrait<T>::kind, PrimitiveTypeKindTrait<T>::name)
    {}

    virtual size_t memory_size() const override
    {
        return sizeof(T);
    };

    virtual void construct_instance(uint8_t* instance) const override
    {
        *reinterpret_cast<T*>(instance) = T(0);
    }

    virtual void destroy_instance(uint8_t* /*instance*/) const override { } //Default does nothing

    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const override
    {
        *reinterpret_cast<T*>(target) = *reinterpret_cast<const T*>(source);
    }

    virtual void copy_instance_from_type(
            uint8_t* target,
            const uint8_t* source,
            const DynamicType& other) const override
    {
        assert(other.is_primitive_type()); (void) other;
        switch(other.kind())
        {
            case TypeKind::BOOLEAN_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const bool*>(source);
                break;
            case TypeKind::UINT_8_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const uint8_t*>(source);
                break;
            case TypeKind::UINT_16_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const uint16_t*>(source);
                break;
            case TypeKind::UINT_32_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const uint32_t*>(source);
                break;
            case TypeKind::UINT_64_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const uint64_t*>(source);
                break;
            case TypeKind::INT_16_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const int16_t*>(source);
                break;
            case TypeKind::INT_32_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const int32_t*>(source);
                break;
            case TypeKind::INT_64_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const int64_t*>(source);
                break;
            case TypeKind::CHAR_8_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const char*>(source);
                break;
            case TypeKind::CHAR_16_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const wchar_t*>(source);
                break;
            case TypeKind::FLOAT_32_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const float*>(source);
                break;
            case TypeKind::FLOAT_64_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const double*>(source);
                break;
            case TypeKind::FLOAT_128_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const long double*>(source);
                break;
            default:
                assert(false); //Must not reached
        }
    }

    virtual void move_instance(
            uint8_t* target,
            uint8_t* source) const override
    {
        copy_instance(target, source);
    }

    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const override
    {
        return *reinterpret_cast<const T*>(instance) == *reinterpret_cast<const T*>(other_instance);
    }

    virtual TypeConsistency is_compatible(
            const DynamicType& other) const override
    {
        if(!other.is_primitive_type())
        {
            return TypeConsistency::NONE;
        }

        if(kind() == other.kind())
        {
            return TypeConsistency::EQUALS;
        }

        TypeConsistency consistency = TypeConsistency::EQUALS;
        if(memory_size() != other.memory_size())
        {
            consistency |= TypeConsistency::IGNORE_TYPE_WIDTH;
        }

        if((kind() & TypeKind::UNSIGNED_TYPE) != (other.kind() & TypeKind::UNSIGNED_TYPE))
        {
            consistency |= TypeConsistency::IGNORE_TYPE_SIGN;
        }
        return consistency;
    }

    virtual void for_each_instance(
            const InstanceNode& node,
            InstanceVisitor visitor) const override
    {
        visitor(node);
    }

protected:
    virtual DynamicType* clone() const override
    {
        return new PrimitiveType<T>();
    }
};

/// \brief Helper function to create PrimitiveTypes.
/// The creation of DynamicTypes representing a PrimitiveType
/// must be always created by this function.
/// \returns A DynamicType representing a PrimitiveType<T>
template<typename T>
const DynamicType& primitive_type()
{
    // The creation of PrimitiveType must be always created
    // by this function in order to not broken the DynamicType::Ptr
    // optimizations for PrimitiveType
    static PrimitiveType<T> p;
    return p;
}

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_PRIMITIVE_TYPES_HPP_
