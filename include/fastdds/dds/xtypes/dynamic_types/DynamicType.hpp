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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICTYPE_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICTYPE_HPP

#include <memory>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/AnnotationDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/dds/xtypes/dynamic_types/VerbatimTextDescriptor.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicType : public std::enable_shared_from_this<DynamicType>
{
public:

    using _ref_type = typename traits<DynamicType>::ref_type;

    /*!
     * Provides a summary of the state of this type overwriting a provided object (see [standard] 7.5.2.8.7)
     * @param [inout] descriptor @ref TypeDescriptor reference where copied the information.
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
     * Returns the member that corresponds to the specified name.
     * @param [inout] member @ref DynamicTypeMember reference used to return the reference to the member.
     * @param [in] name Member name of the member being queried.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the member was found.
     * @retval RETCODE_BAD_PARAMETER when the member doesn't exist.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_member_by_name(
            traits<DynamicTypeMember>::ref_type& member,
            const ObjectName& name) = 0;

    /*!
     * Returns all members by @ref ObjectName.
     * @param [inout] member @ref DynamicTypeMembersByName reference where the information is copied.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_all_members_by_name(
            DynamicTypeMembersByName& member) = 0;

    /*!
     * Returns the member that corresponds to the specified @ref MemberId.
     * @param [inout] member @ref DynamicTypeMember reference used to return the reference to the member.
     * @param [in] id @ref MemberId
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the member was found.
     * @retval RETCODE_BAD_PARAMETER when the member doesn't exist.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_member(
            traits<DynamicTypeMember>::ref_type& member,
            MemberId id) = 0;

    /*!
     * Returns all members by @ref MemberId.
     * @param [inout] member @ref DynamicTypeMembersById reference where the information is copied.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_all_members(
            DynamicTypeMembersById& member) = 0;

    /*!
     * This operation returns the current number of members.
     * @return Current number of members
     */
    FASTDDS_EXPORTED_API virtual uint32_t get_member_count() = 0;

    /**
     * This operation returns the member that corresponds to the specified index.
     * @param [inout] member @ref DynamicTypeMember reference used to return the reference to the member.
     * @param [in] index Index
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the member was found.
     * @retval RETCODE_BAD_PARAMETER when the index is out-of-range.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_member_by_index(
            traits<DynamicTypeMember>::ref_type& member,
            uint32_t index) = 0;

    /*!
     * Returns the number of applied annotations to the type.
     * @return Number of annotations.
     */
    FASTDDS_EXPORTED_API virtual uint32_t get_annotation_count() = 0;

    /*!
     * Returns an applied annotation by index.
     * @param [inout] descriptor @ref AnnotationDescriptor reference where the information is copied.
     * @param [in] idx Index.
     * @return standard @ref ReturnCode_t
     * @retval RETCODE_OK when the copy was successful.
     * @retval RETCODE_BAD_PARAMETER when descriptor reference is nil or index is out-of-range.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_annotation(
            traits<AnnotationDescriptor>::ref_type& descriptor,
            uint32_t idx) = 0;

    /*!
     * Returns the number of applied verbatim text to the type.
     * @return Number of verbatim texts.
     */
    FASTDDS_EXPORTED_API virtual uint32_t get_verbatim_text_count() = 0;

    /*!
     * Returns an applied verbatim text by index.
     * @param [inout] descriptor @ref VerbatimTextDescriptor reference where the information is copied.
     * @param [in] idx Index.
     * @return standard @ref ReturnCode_t
     * @retval RETCODE_OK when the copy was successful.
     * @retval RETCODE_BAD_PARAMETER when descriptor reference is nil or index is out-of-range.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_verbatim_text(
            traits<VerbatimTextDescriptor>::ref_type& descriptor,
            uint32_t idx) = 0;

    /**
     * State comparison according with the [standard] sections \b 7.5.2.8.4
     * @param [in] other @ref DynamicType reference to compare to
     * @return \b bool `true` on equality
     */
    FASTDDS_EXPORTED_API virtual bool equals(
            traits<DynamicType>::ref_type other) = 0;

protected:

    DynamicType() = default;

    virtual ~DynamicType() = default;

    traits<DynamicType>::ref_type _this();

private:

    DynamicType(
            const DynamicType&) = delete;

    DynamicType(
            DynamicType&&) = delete;

    DynamicType& operator =(
            const DynamicType&) = delete;

    DynamicType& operator =(
            DynamicType&&) = delete;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICTYPE_HPP
