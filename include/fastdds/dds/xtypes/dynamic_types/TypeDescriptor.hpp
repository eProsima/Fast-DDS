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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__TYPEDESCRIPTOR_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__TYPEDESCRIPTOR_HPP

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicType;

/*!
 * TypeDescriptor definition according to [standard] section \b 7.5.2.4.
 */
class FASTDDS_EXPORTED_API TypeDescriptor
{
public:

    using _ref_type = typename traits<TypeDescriptor>::ref_type;

    /*!
     * Returns the @ref TypeKind associated.
     * @return standard @ref TypeKind.
     */
    virtual TypeKind kind() const = 0;

    /*!
     * Returns the @ref TypeKind associated.
     * @return standard @ref TypeKind.
     */
    virtual TypeKind& kind() = 0;

    /*!
     * Modifies the underlying @ref TypeKind.
     * @param [in] kind @ref TypeKind to be set.
     */
    virtual void kind(
            TypeKind kind) = 0;

    /*!
     * Returns the fully qualified name of this type.
     * @return Fully qualified name.
     */
    virtual ObjectName& name() = 0;

    /*!
     * Returns the fully qualified name of this type.
     * @return Fully qualified name.
     */
    virtual const ObjectName& name() const = 0;

    /*!
     * Modifies the underlying type name by copy.
     * @param [in] name Fully qualified name.
     */
    virtual void name(
            const ObjectName& name) = 0;

    /*!
     * Modifies the underlying type name by move.
     * @param [in] name Fully qualified name.
     */
    virtual void name(
            ObjectName&& name) = 0;

    /*!
     * Returns a reference to the base type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual traits<DynamicType>::ref_type base_type() const = 0;

    /*!
     * Returns a reference to the base type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual traits<DynamicType>::ref_type& base_type() = 0;

    /*!
     * Modifies the underlying base type reference.
     * @param [in] type @ref DynamicType reference.
     */
    virtual void base_type(
            traits<DynamicType>::ref_type type) = 0;

    /*!
     * Returns a reference discriminator type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual traits<DynamicType>::ref_type discriminator_type() const = 0;

    /*!
     * Returns a reference discriminator type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual traits<DynamicType>::ref_type& discriminator_type() = 0;

    /*!
     * Modifies the underlying discriminator type reference.
     * @param [in] type @ref DynamicType reference.
     */
    virtual void discriminator_type(
            traits<DynamicType>::ref_type type) = 0;

    /*!
     * Returns the bound.
     * @return @ref BoundSeq.
     */
    virtual const BoundSeq& bound() const = 0;

    /*!
     * Returns the bound.
     * @return @ref BoundSeq.
     */
    virtual BoundSeq& bound() = 0;

    /*!
     * Modifies the underlying bound by copy.
     * @param [in] bound @ref BoundSeq
     */
    virtual void bound(
            const BoundSeq& bound) = 0;

    /*!
     * Modifies the underlying bound by move.
     * @param [in] bound @ref BoundSeq
     */
    virtual void bound(
            BoundSeq&& bound) = 0;

    /*!
     * Returns a reference element type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual traits<DynamicType>::ref_type element_type() const = 0;

    /*!
     * Returns a reference element type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual traits<DynamicType>::ref_type& element_type() = 0;

    /*!
     * Modifies the underlying element type reference.
     * @param [in] type @ref DynamicType reference.
     */
    virtual void element_type(
            traits<DynamicType>::ref_type type) = 0;

    /*!
     * Returns a reference key element type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual traits<DynamicType>::ref_type key_element_type() const = 0;

    /*!
     * Returns a reference key element type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual traits<DynamicType>::ref_type& key_element_type() = 0;

    /*!
     * Modifies the underlying key element type reference.
     * @param [in] type @ref DynamicType reference.
     */
    virtual void key_element_type(
            traits<DynamicType>::ref_type type) = 0;


    /*!
     * Returns the extensibility kind.
     * return @ref ExtensibilityKind
     */
    virtual ExtensibilityKind extensibility_kind() const = 0;

    /*!
     * Returns the extensibility kind.
     * return @ref ExtensibilityKind
     */
    virtual ExtensibilityKind& extensibility_kind() = 0;

    /*!
     * Modifies the extensibility kind.
     * @param [in] extensibility_kind @ref ExtensibilityKind
     */
    virtual void extensibility_kind(
            ExtensibilityKind extensibility_kind) = 0;

    /*!
     * Returns the is_nested property.
     * return Boolean
     */
    virtual bool is_nested() const = 0;

    /*!
     * Returns the is_nested property.
     * return Boolean
     */
    virtual bool& is_nested() = 0;

    /*!
     * Modifies the is_nested property.
     * @param [in] is_nested Boolean value to be set.
     */
    virtual void is_nested(
            bool is_nested) = 0;

    /*!
     * Overwrites the contents of this descriptor with those of another descriptor (see [standard] 7.5.2.4.3).
     * @param [in] descriptor reference.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the copy was successful.
     * @retval RETCODE_BAD_PARAMETER when descriptor reference is nil.
     */
    virtual ReturnCode_t copy_from(
            traits<TypeDescriptor>::ref_type descriptor) = 0;

    /*!
     * Compares according with the [standard] section \b 7.5.2.4.6.
     * @param [in] descriptor reference to compare to.
     * @return \b bool `true` on equality
     */
    virtual bool equals(
            traits<TypeDescriptor>::ref_type descriptor) = 0;

    /*!
     * Indicates whether the states of all of this descriptor's properties are consistent according with the [standard]
     * section \b 7.5.2.4.7.
     * @return \b bool `true` if consistent.
     */
    virtual bool is_consistent() = 0;

protected:

    TypeDescriptor() = default;

    TypeDescriptor(
            const TypeDescriptor& type) = default;

    TypeDescriptor(
            TypeDescriptor&& type) = default;

    virtual ~TypeDescriptor() = default;

private:

    TypeDescriptor& operator =(
            const TypeDescriptor& type) = delete;

    TypeDescriptor& operator =(
            TypeDescriptor&& type) = delete;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__TYPEDESCRIPTOR_HPP
