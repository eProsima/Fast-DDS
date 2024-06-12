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

#ifndef _FASTDDS_XTYPES_DYNAMIC_TYPES_TYPE_DESCRIPTOR_IMPL_HPP_
#define _FASTDDS_XTYPES_DYNAMIC_TYPES_TYPE_DESCRIPTOR_IMPL_HPP_

#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class TypeDescriptorImpl : public virtual TypeDescriptor
{
    //! Type kind.
    TypeKind kind_ {TK_NONE};

    //! Type name.
    ObjectName name_;

    //! SuperType of an structure or base type of an alias type.
    traits<DynamicType>::ref_type base_type_;

    //! Discriminator type for a union.
    traits<DynamicType>::ref_type discriminator_type_;

    //! Length for strings, arrays, sequences, maps and bitmasks.
    BoundSeq bound_;

    //! Value Type for strings, arrays, sequences, maps and bitmasks.
    traits<DynamicType>::ref_type element_type_;

    //! Key Type for maps.
    traits<DynamicType>::ref_type key_element_type_;

    ExtensibilityKind extensibility_kind_ {ExtensibilityKind::APPENDABLE};

    bool is_extensibility_set_ {false};

    bool is_nested_ {false};

public:

    TypeDescriptorImpl() noexcept = default;

    TypeDescriptorImpl(
            TypeKind kind,
            const ObjectName& name);

    TypeDescriptorImpl(
            const TypeDescriptorImpl& type) noexcept = default;

    TypeDescriptorImpl(
            TypeDescriptorImpl&& type) noexcept = default;

    virtual ~TypeDescriptorImpl() noexcept = default;

    TypeKind kind() const noexcept override
    {
        return kind_;
    }

    TypeKind& kind() noexcept override
    {
        return kind_;
    }

    void kind(
            TypeKind kind) noexcept override
    {
        kind_ = kind;
    }

    ObjectName& name() noexcept override
    {
        return name_;
    }

    const ObjectName& name() const noexcept override
    {
        return name_;
    }

    void name(
            const ObjectName& name) noexcept override
    {
        name_ = name;
    }

    virtual void name(
            ObjectName&& name) noexcept override
    {
        name_ = std::move(name);
    }

    traits<DynamicType>::ref_type base_type() const noexcept override
    {
        return base_type_;
    }

    traits<DynamicType>::ref_type& base_type() noexcept override
    {
        return base_type_;
    }

    void base_type(
            traits<DynamicType>::ref_type type) noexcept override
    {
        base_type_ = type;
    }

    traits<DynamicType>::ref_type discriminator_type() const noexcept override
    {
        return discriminator_type_;
    }

    traits<DynamicType>::ref_type& discriminator_type() noexcept override
    {
        return discriminator_type_;
    }

    void discriminator_type(
            traits<DynamicType>::ref_type type) noexcept override
    {
        discriminator_type_ = type;
    }

    const BoundSeq& bound() const noexcept override
    {
        return bound_;
    }

    BoundSeq& bound() noexcept override
    {
        return bound_;
    }

    void bound(
            const BoundSeq& bound) noexcept override
    {
        bound_ = bound;
    }

    void bound(
            BoundSeq&& bound) noexcept override
    {
        bound_ = std::move(bound);
    }

    traits<DynamicType>::ref_type element_type() const noexcept override
    {
        return element_type_;
    }

    traits<DynamicType>::ref_type& element_type() noexcept override
    {
        return element_type_;
    }

    void element_type(
            traits<DynamicType>::ref_type type) noexcept override
    {
        element_type_ = type;
    }

    traits<DynamicType>::ref_type key_element_type() const noexcept override
    {
        return key_element_type_;
    }

    traits<DynamicType>::ref_type& key_element_type() noexcept override
    {
        return key_element_type_;
    }

    void key_element_type(
            traits<DynamicType>::ref_type type) noexcept override
    {
        key_element_type_ = type;
    }

    ExtensibilityKind extensibility_kind() const noexcept override
    {
        return extensibility_kind_;
    }

    ExtensibilityKind& extensibility_kind() noexcept override
    {
        return extensibility_kind_;
    }

    void extensibility_kind(
            ExtensibilityKind extensibility_kind) noexcept override
    {
        extensibility_kind_ = extensibility_kind;
        is_extensibility_set_ = true;
    }

    bool is_nested() const noexcept override
    {
        return is_nested_;
    }

    bool& is_nested() noexcept override
    {
        return is_nested_;
    }

    void is_nested(
            bool is_nested) noexcept override
    {
        is_nested_ = is_nested;
    }

    ReturnCode_t copy_from(
            traits<TypeDescriptor>::ref_type descriptor) noexcept override;

    ReturnCode_t copy_from(
            const TypeDescriptorImpl& descriptor) noexcept;

    bool equals(
            traits<TypeDescriptor>::ref_type descriptor) noexcept override;

    bool equals(
            TypeDescriptorImpl& descriptor) noexcept;

    bool is_consistent() noexcept override;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_XTYPES_DYNAMIC_TYPES_TYPE_DESCRIPTOR_IMPL_HPP_

