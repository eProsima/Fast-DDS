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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_TYPE_DESCRIPTOR_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_TYPE_DESCRIPTOR_HPP

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicType;

/*!
 * TypeDescriptor definition according to [standard] section \b 7.5.2.4.
 */
class RTPS_DllAPI TypeDescriptor
{
public:

    /*!
     * Returns the @ref TypeKind associated.
     * @return standard @ref TypeKind.
     */
    virtual TypeKind kind() const noexcept = 0;

    /*!
     * Returns the @ref TypeKind associated.
     * @return standard @ref TypeKind.
     */
    virtual TypeKind& kind() noexcept = 0;

    /*!
     * Modifies the underlying @ref TypeKind.
     * @param[in] @ref TypeKind to be set.
     */
    virtual void kind(
            TypeKind kind) noexcept = 0;

    /*!
     * Returns the fully qualified name of this type.
     * @return Fully qualified name.
     */
    virtual ObjectName& name() noexcept = 0;

    /*!
     * Returns the fully qualified name of this type.
     * @return Fully qualified name.
     */
    virtual const ObjectName& name() const noexcept = 0;

    /*!
     * Modifies the underlying type name by copy.
     * @param[in] name Fully qualified name.
     */
    virtual void name(
            const ObjectName& name) noexcept = 0;

    /*!
     * Modifies the underlying type name by move.
     * @param[in] name Fully qualified name.
     */
    virtual void name(
            ObjectName&& name) noexcept = 0;

    /*!
     * Returns a reference to the base type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual type_traits<DynamicType>::ref_type base_type() const noexcept = 0;

    /*!
     * Returns a reference to the base type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual type_traits<DynamicType>::ref_type& base_type() noexcept = 0;

    /*!
     * Modifies the underlying base type reference.
     * @param[in] @ref DynamicType reference.
     */
    virtual void base_type(
            type_traits<DynamicType>::ref_type type) noexcept = 0;

    /*!
     * Returns a reference discriminator type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual type_traits<DynamicType>::ref_type discriminator_type() const noexcept = 0;

    /*!
     * Returns a reference discriminator type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual type_traits<DynamicType>::ref_type& discriminator_type() noexcept = 0;

    /*!
     * Modifies the underlying discriminator type reference.
     * @param[in] @ref DynamicType reference.
     */
    virtual void discriminator_type(
            type_traits<DynamicType>::ref_type type) noexcept = 0;

    /*!
     * Returns the bound.
     * @return @ref BoundSeq.
     */
    virtual const BoundSeq& bound() const noexcept = 0;

    /*!
     * Returns the bound.
     * @return @ref BoundSeq.
     */
    virtual BoundSeq& bound() noexcept = 0;

    /*!
     * Modifies the underlying bound by copy.
     * @param[in] @ref BoundSeq
     */
    virtual void bound(
            const BoundSeq& bound) noexcept = 0;

    /*!
     * Modifies the underlying bound by move.
     * @param[in] @ref BoundSeq
     */
    virtual void bound(
            BoundSeq&& bound) noexcept = 0;

    /*!
     * Returns a reference element type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual type_traits<DynamicType>::ref_type element_type() const noexcept = 0;

    /*!
     * Returns a reference element type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual type_traits<DynamicType>::ref_type& element_type() noexcept = 0;

    /*!
     * Modifies the underlying element type reference.
     * @param[in] @ref DynamicType reference.
     */
    virtual void element_type(
            type_traits<DynamicType>::ref_type type) noexcept = 0;

    /*!
     * Returns a reference key element type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual type_traits<DynamicType>::ref_type key_element_type() const noexcept = 0;

    /*!
     * Returns a reference key element type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual type_traits<DynamicType>::ref_type& key_element_type() noexcept = 0;

    /*!
     * Modifies the underlying key element type reference.
     * @param[in] @ref DynamicType reference.
     */
    virtual void key_element_type(
            type_traits<DynamicType>::ref_type type) noexcept = 0;


    /*!
     * Returns the extensibility kind.
     * return @ref ExtensibilityKind
     */
    virtual ExtensibilityKind extensibility_kind() const noexcept = 0;

    /*!
     * Returns the extensibility kind.
     * return @ref ExtensibilityKind
     */
    virtual ExtensibilityKind& extensibility_kind() noexcept = 0;

    /*!
     * Modifies the extensibility kind.
     * @param[in] @ref ExtensibilityKind
     */
    virtual void extensibility_kind(
            ExtensibilityKind extensibility_kind) noexcept = 0;

    /*!
     * Returns the is_nested property.
     * return Boolean
     */
    virtual bool is_nested() const noexcept = 0;

    /*!
     * Returns the is_nested property.
     * return Boolean
     */
    virtual bool& is_nested() noexcept = 0;

    /*!
     * Mofifies the is_nested property.
     * @param[in] Boolean
     */
    virtual void is_nested(
            bool is_nested) noexcept = 0;

    /*!
     * Overwrites the contents of this descriptor with those of another descriptor (see [standard] 7.5.2.3.1).
     * @param[in] descriptor reference.
     */
    virtual ReturnCode_t copy_from(
            type_traits<TypeDescriptor>::ref_type descriptor) noexcept = 0;

    /*!
     * Compares according with the [standard] section \b 7.5.2.4.7
     * @param[in] descriptor reference to compare to.
     * @return \b bool `true` on equality
     */
    virtual bool equals(
            type_traits<TypeDescriptor>::ref_type descriptor) const noexcept = 0;

    /*!
     * Indicates whether the states of all of this descriptor's properties are consistent.
     * @return \b bool `true` if consistent.
     */
    virtual bool is_consistent() const noexcept = 0;

protected:

    TypeDescriptor() noexcept = default;

    TypeDescriptor(
            const TypeDescriptor& type) noexcept = default;

    TypeDescriptor(
            TypeDescriptor&& type) noexcept = default;

    virtual ~TypeDescriptor() noexcept = default;

private:

    TypeDescriptor& operator =(
            const TypeDescriptor& type) noexcept = delete;

    TypeDescriptor& operator =(
            TypeDescriptor&& type) noexcept = delete;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_TYPE_DESCRIPTOR_HPP
