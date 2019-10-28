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
 *
*/

#ifndef OMG_DDS_CORE_XTYPES_STRING_TYPE_HPP_
#define OMG_DDS_CORE_XTYPES_STRING_TYPE_HPP_

#include <dds/core/xtypes/MutableCollectionType.hpp>
#include <dds/core/xtypes/PrimitiveTypes.hpp>
#include <dds/core/xtypes/SequenceInstance.hpp>

namespace dds {
namespace core {
namespace xtypes {

template<typename CHAR_T, TypeKind KIND, const char* TYPE_NAME>
class TStringType : public MutableCollectionType
{
public:
    TStringType(
            int bounds = 0)
        : MutableCollectionType(
                KIND,
                TYPE_NAME + ((bounds > 0) ? "_" + std::to_string(bounds) : ""),
                DynamicType::Ptr(primitive_type<CHAR_T>()),
                bounds)
    {}

    virtual size_t memory_size() const override
    {
        return sizeof(std::basic_string<CHAR_T>);
    }

    virtual void construct_instance(
            uint8_t* instance) const override
    {
        new (instance) std::basic_string<CHAR_T>();
        reinterpret_cast<std::basic_string<CHAR_T>*>(instance)->reserve(bounds());
    }

    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const override
    {
        new (target) std::basic_string<CHAR_T>(*reinterpret_cast<const std::basic_string<CHAR_T>*>(source));
    }

    virtual void copy_instance_from_type(
            uint8_t* target,
            const uint8_t* source,
            const DynamicType& other) const override
    {
        assert(other.kind() == KIND);
        new (target) std::basic_string<CHAR_T>(*reinterpret_cast<const std::basic_string<CHAR_T>*>(source));
    }

    virtual void move_instance(
            uint8_t* target,
            uint8_t* source) const override
    {
        new (target) std::basic_string<CHAR_T>(std::move(*reinterpret_cast<const std::basic_string<CHAR_T>*>(source)));
    }

    virtual void destroy_instance(
            uint8_t* instance) const override
    {
        using namespace std;
        reinterpret_cast<std::basic_string<CHAR_T>*>(instance)->std::basic_string<CHAR_T>::~basic_string<CHAR_T>();
    }

    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const override
    {
        return *reinterpret_cast<const std::basic_string<CHAR_T>*>(instance) == *reinterpret_cast<const std::basic_string<CHAR_T>*>(other_instance);
    }

    virtual TypeConsistency is_compatible(
            const DynamicType& other) const override
    {
        if(other.kind() != KIND)
        {
            return TypeConsistency::NONE;
        }

        const TStringType& other_string = static_cast<const TStringType&>(other);

        if(bounds() == other_string.bounds())
        {
            return TypeConsistency::EQUALS;
        }

        return TypeConsistency::IGNORE_STRING_BOUNDS;
    }

    virtual void for_each_instance(
            const InstanceNode& node,
            InstanceVisitor visitor) const override
    {
        visitor(node);
    }

    virtual uint8_t* get_instance_at(
            uint8_t* instance,
            size_t index) const override
    {
        void* char_addr = &reinterpret_cast<std::basic_string<CHAR_T>*>(instance)->operator[](index);
        return static_cast<uint8_t*>(char_addr);
    }

    virtual size_t get_instance_size(
            const uint8_t* instance) const override
    {
        return reinterpret_cast<const std::basic_string<CHAR_T>*>(instance)->size();
    }

protected:
    virtual DynamicType* clone() const override
    {
        return new TStringType(*this);
    }
};

constexpr const char string_type_name[] = "std::string";
using StringType = TStringType<char, TypeKind::STRING_TYPE, string_type_name>;

constexpr const char wstring_type_name[] = "std::wstring";
using WStringType = TStringType<wchar_t, TypeKind::WSTRING_TYPE, wstring_type_name>;

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_STRING_TYPE_HPP_
