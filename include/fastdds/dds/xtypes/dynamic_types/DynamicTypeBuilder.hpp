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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICTYPEBUILDER_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICTYPEBUILDER_HPP

#include <memory>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/AnnotationDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicTypeBuilder : public std::enable_shared_from_this<DynamicTypeBuilder>
{
public:

    using _ref_type = typename traits<DynamicTypeBuilder>::ref_type;

    /*!
     * Provides a summary of the state of this type overwriting a provided object.
     * @param [inout] descriptor @ref TypeDescriptor
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the copy was successful.
     * @retval RETCODE_BAD_PARAMETER when descriptor reference is nil.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_descriptor(
            traits<TypeDescriptor>::ref_type& descriptor) = 0;

    /*!
     * Returns the fully qualified name of this type
     * @return Type name.
     */
    FASTDDS_EXPORTED_API virtual ObjectName get_name() = 0;

    /*!
     * Returns the @ref TypeKind associated
     * @return @ref TypeKind
     */
    FASTDDS_EXPORTED_API virtual TypeKind get_kind() = 0;

    /*!
     * Returns a member looked for by name.
     * @param [inout] member @ref DynamicTypeMember reference used to return the member.
     * @param [in] name Member name.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when member was found.
     * @retval RETCODE_BAD_PARAMETER when member wasn't found.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_member_by_name(
            traits<DynamicTypeMember>::ref_type& member,
            const ObjectName& name) = 0;

    /*!
     * Returns all members sorted by name.
     * @param [inout] member DynamicTypeMemberByName reference used to return all members.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK always.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_all_members_by_name(
            DynamicTypeMembersByName& member) = 0;
    /*!
     * Returns a member looked for by @ref MemberId.
     * @param [inout] member @ref DynamicTypeMember reference used to return the member.
     * @param [in] id Member identifier.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when member was found.
     * @retval RETCODE_BAD_PARAMETER when member wasn't found.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_member(
            traits<DynamicTypeMember>::ref_type& member,
            MemberId id) = 0;

    /*!
     * Returns all members sorted by MemberId.
     * @param [inout] member DynamicTypeMemberById reference used to return all members.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK always.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_all_members(
            DynamicTypeMembersById& member) = 0;

    /*!
     * This operation returns the current number of members.
     * @return Current number of members
     */
    FASTDDS_EXPORTED_API virtual uint32_t get_member_count() = 0;

    /*!
     * This operation returns the member that corresponds to the specified index.
     * @param [inout] member @ref DynamicTypeMember reference used to return the member.
     * @param [in] index Index
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when member was found.
     * @retval RETCODE_BAD_PARAMETER when index is out-of-range.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_member_by_index(
            traits<DynamicTypeMember>::ref_type& member,
            uint32_t index) = 0;

    /*!
     * This operation returns the current number of annotations to the type.
     * @return Current number of annotations
     */
    FASTDDS_EXPORTED_API virtual uint32_t get_annotation_count() = 0;

    /*!
     * This operation returns the annotation that corresponds to the specified index.
     * @param [inout] descriptor @ref AnnotationDescriptor reference where information is copied.
     * @param [in] idx Index
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when member was found.
     * @retval RETCODE_BAD_PARAMETER when reference is nil or index is out-of-range.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_annotation(
            traits<AnnotationDescriptor>::ref_type& descriptor,
            uint32_t idx) = 0;

    /*!
     * Compares current state against a @ref DynamicType reference.
     * @param [in] other @ref DynamicType reference to compare to.
     * @return \b bool `true` on equality
     */
    FASTDDS_EXPORTED_API virtual bool equals(
            traits<DynamicType>::ref_type other) = 0;

    /*!
     * Adds a 'member' to this type, where the new 'member' has the meaning defined in the specification of
     * the DynamicTypeMember class.
     * @param [in] descriptor @ref MemberDescriptor reference used for the new member.
     * @return ReturnCode_t
     * @retval RETCODE_OK when the member was created successfully.
     * @retval RETCODE_BAD_PARAMETER when there is an inconsistency.
     * @retval RETCODE_PRECONDITION_NOT_MET when the type does not have members.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t add_member(
            traits<MemberDescriptor>::ref_type descriptor) = 0;

    /*!
     * Apply the given annotation to this type.
     * @param [in] descriptor @ref AnnotationDescriptor reference to be applied.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the annotation was applied successful.
     * @retval RETCODE_BAD_PARAMETER when there is an inconsistency.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t apply_annotation(
            traits<AnnotationDescriptor>::ref_type descriptor) = 0;

    /*!
     * Apply the given annotation to a member of this type.
     * @param [in] member_id Member identifier.
     * @param [in] descriptor @ref AnnotationDescriptor reference to be applied.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the annotation was applied successful.
     * @retval RETCODE_BAD_PARAMETER when there is an inconsistency.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t apply_annotation_to_member(
            MemberId member_id,
            traits<AnnotationDescriptor>::ref_type descriptor) = 0;

    /*!
     * Create an immutable DynamicType object containing a snapshot of this builder's current state.
     * @return @ref DynamicType reference.
     */
    FASTDDS_EXPORTED_API virtual traits<DynamicType>::ref_type build() = 0;

protected:

    DynamicTypeBuilder() = default;

    virtual ~DynamicTypeBuilder() = default;

    traits<DynamicTypeBuilder>::ref_type _this();

private:

    DynamicTypeBuilder(
            const DynamicTypeBuilder&) = delete;

    DynamicTypeBuilder(
            DynamicTypeBuilder&&) = delete;

    DynamicTypeBuilder& operator =(
            const DynamicTypeBuilder&) = delete;

    DynamicTypeBuilder& operator =(
            DynamicTypeBuilder&&) = delete;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICTYPEBUILDER_HPP
