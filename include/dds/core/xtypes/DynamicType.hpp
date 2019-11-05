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
#include <dds/core/xtypes/TypeConsistency.hpp>

#include <string>
#include <cassert>

namespace dds {
namespace core {
namespace xtypes {

/// \brief Abstract base class for all dynamic types.
class DynamicType : public Instanceable
{
public:
    virtual ~DynamicType() = default;

    /// \brief Name of the DynamicType.
    /// \returns DynamicType's name.
    const std::string& name() const { return name_; }

    /// \brief type kind this DynamicType. (see TypeKind)
    /// \returns type kind corresponding to this DynamicType
    const TypeKind& kind() const { return kind_; }

    /// \brief check if this type is primitive
    /// (has the corresponding bit of TypeKind::PRIMITIVE_TYPE in its kind).
    /// \returns true if is primitive
    bool is_primitive_type() const
    {
        return (kind_ & TypeKind::PRIMITIVE_TYPE) != TypeKind::NO_TYPE;
    }

    /// \brief check if this type is a collection
    /// (has the corresponding bit of TypeKind::COLLECTION_TYPE in its kind).
    /// \returns true if is a collection
    bool is_collection_type() const
    {
        return (kind_ & TypeKind::COLLECTION_TYPE) != TypeKind::NO_TYPE;
    }

    /// \brief check if this type is an aggregation
    /// (has the corresponding bit of TypeKind::AGGREGATION_TYPE in its kind).
    /// \returns true if is an aggregation
    bool is_aggregation_type() const
    {
        return (kind_ & TypeKind::AGGREGATION_TYPE) != TypeKind::NO_TYPE;
    }

    /// \brief check if this type is constructed
    /// (has the corresponding bit of TypeKind::CONSTRUCTED_TYPE in its kind).
    /// \returns true if is constructed
    bool is_constructed_type() const
    {
        return (kind_ & TypeKind::CONSTRUCTED_TYPE) != TypeKind::NO_TYPE;
    }

    /// \brief check the compatibility with other DynamicType.
    /// returns The needed consistency required for the types can be compatibles.
    ///   TypeConsistency::EQUALS means that the types are identically.
    ///   TypeConsistency::NONE means that no conversion is known to be compatibles both types.
    ///   Otherwise, a level of consistency was found for convert both types between them.
    ///   See TypeConsistency for more information.
    virtual TypeConsistency is_compatible(const DynamicType& other) const = 0;

protected:
    DynamicType(
            TypeKind kind,
            const std::string& name)
        : kind_(kind)
        , name_(name)
    {}

    DynamicType(const DynamicType& other) = default;
    DynamicType(DynamicType&& other) = default;

    /// \brief Deep clone of the DynamicType.
    /// \returns a new DynamicType without managing.
    virtual DynamicType* clone() const = 0;

private:
    TypeKind kind_;
    std::string name_;


public:
    /// \brief Special managed pointer for DynamicTypes.
    /// It performs some performances to avoid copies for some internal types.
    class Ptr
    {
    public:
        /// \brief Default initialization without pointer any type.
        Ptr()
            : type_(nullptr)
        {}

        /// \brief Creates a copy of a DynamicType that will be managed.
        /// The copy is avoid if DynamnicType is primitive.
        Ptr(const DynamicType& type)
            : type_(type.is_primitive_type() ? &type : type.clone())
        {}

        /// \brief Copy constructor.
        /// Makes an internal copy of the managed DynamicType.
        /// The copy is avoid if DynamnicType is primitive.
        Ptr(const Ptr& ptr)
            : type_((ptr.type_ == nullptr || ptr.type_->is_primitive_type()) ? ptr.type_ : ptr.type_->clone())
        {}

        Ptr(Ptr&& ptr)
            : type_ (ptr.type_)
        {
            ptr.type_ = nullptr;
        }

        virtual ~Ptr()
        {
            reset();
        }

        /// \brief Remove the managed DynamicType and points to nothing.
        void reset()
        {
            if(type_ != nullptr && !type_->is_primitive_type())
            {
                delete type_;
            }
            type_ = nullptr;
        }

        /// \brief Free the internal managed DynamicType and points to nothing.
        const DynamicType* free()
        {
            const DynamicType* freed = type_;
            type_ = nullptr;
            return freed;
        }

        /// \brief Returns a pointer of the internal managed DynamicType.
        /// \returns A pointer of the internal managed DynamicType.
        const DynamicType* get() const { return type_; }

        /// \brief Returns a pointer of the internal managed DynamicType.
        /// \returns A pointer of the internal managed DynamicType.
        const DynamicType* operator ->() const { return type_; }

        /// \brief Returns a reference of the intenral managed DynamicType.
        /// \returns A reference of the internal managed DynamicType.
        const DynamicType& operator *() const { return *type_; }

    private:
        const DynamicType* type_;
    };
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_DYNAMIC_TYPE_HPP_
