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
#ifndef OMG_DDS_CORE_XTYPES_DYNAMIC_TYPE_HPP_
#define OMG_DDS_CORE_XTYPES_DYNAMIC_TYPE_HPP_

#include <dds/core/xtypes/Instanceable.hpp>
#include <dds/core/xtypes/TypeKind.hpp>

#include <string>
#include <iostream> //DELETE

namespace dds {
namespace core {
namespace xtypes {

class DynamicType : public Instanceable
{
public:
    virtual ~DynamicType() = default;

    const std::string& name() const { return name_; }
    const TypeKind& kind() const { return kind_; }

    bool is_primitive_type() const
    {
        return (int(kind_) & int(TypeKind::PRIMITIVE_TYPE)) != 0;
    }

    bool is_collection_type() const
    {
        return (int(kind_) & int(TypeKind::COLLECTION_TYPE)) != 0;
    }

    bool is_aggregation_type() const
    {
        return (int(kind_) & int(TypeKind::AGGREGATION_TYPE)) != 0;
    }

    bool is_constructed_type() const
    {
        return (int(kind_) & int(TypeKind::CONSTRUCTED_TYPE)) != 0;
    }

protected:
    DynamicType(
            TypeKind kind,
            const std::string& name)
        : kind_(kind)
        , name_(name)
    {}

    DynamicType(const DynamicType& other) = default;
    DynamicType(DynamicType&& other) = default;

    virtual DynamicType* clone() const = 0;

private:
    TypeKind kind_;
    std::string name_;


public:
    class Ptr
    {
    public:
        Ptr()
            : type_(nullptr)
        {}

        Ptr(const DynamicType& type)
            : type_(type.is_primitive_type() ? &type : type.clone())
        {}

        template<typename DynamicTypeImpl>
        Ptr(const DynamicTypeImpl&& type) //Ok, never comes a PrimitiveType as rvalue
            : type_(new DynamicTypeImpl(std::move(type)))
        {}

        Ptr(const Ptr& ptr)
            : type_(ptr.type_ == nullptr || ptr.type_->is_primitive_type() ? ptr.type_ : ptr.type_->clone())
        {}

        Ptr(Ptr&& ptr)
            : type_ (ptr.type_)
        {
            ptr.type_ = nullptr;
        }

        virtual ~Ptr()
        {
            if(type_ != nullptr && !type_->is_primitive_type())
            {
                delete type_;
            }
        }

        void reset()
        {
            type_ = nullptr;
        }

        const DynamicType* get() const { return type_; }
        const DynamicType& operator *() const { return *type_; }
        const DynamicType* operator ->() const { return type_; }

    private:
        const DynamicType* type_;
    };
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_DYNAMIC_TYPE_HPP_
