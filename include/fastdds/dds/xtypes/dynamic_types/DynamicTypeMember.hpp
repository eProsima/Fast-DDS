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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICTYPEMEMBER_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICTYPEMEMBER_HPP

#include <map>
#include <memory>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/AnnotationDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/dds/xtypes/dynamic_types/VerbatimTextDescriptor.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/*!
 * Represents a "member" of a type. A "member" in this sense may be a member of an aggregated type, a constant within an
 * enumeration, or some other type substructure.
 */
class DynamicTypeMember : public std::enable_shared_from_this<DynamicTypeMember>
{
public:

    using _ref_type = typename traits<DynamicTypeMember>::ref_type;

    /*!
     * Provides a summary of the state of this type overwriting a provided object (see [standard] 7.5.2.6.2)
     * @param [inout] descriptor @ref MemberDescriptor reference where  the information is copied.
     * @return standard @ref ReturnCode_t
     * @retval RETCODE_OK when the copy was successful.
     * @retval RETCODE_BAD_PARAMETER when descriptor reference is nil.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_descriptor(
            traits<MemberDescriptor>::ref_type& descriptor) = 0;

    /*!
     * Returns the number of applied annotations to the member.
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
     * Returns the number of applied verbatim text to the member.
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
     * State comparison according with the [standard] sections \b 7.5.2.6.3
     * @param [in] other @ref DynamicTypeMember reference to compare to
     * @return \b bool `true` on equality
     */
    FASTDDS_EXPORTED_API virtual bool equals(
            traits<DynamicTypeMember>::ref_type other) = 0;

    /**
     * Getter for @b id property according with the [standard] section \b 7.5.2.6.4
     * @return @ref MemberId
     */
    FASTDDS_EXPORTED_API virtual MemberId get_id()  = 0;

    /**
     * Returns the name of this member.
     * @return Member name.
     */
    FASTDDS_EXPORTED_API virtual ObjectName get_name() = 0;

protected:

    DynamicTypeMember() = default;

    virtual ~DynamicTypeMember() = default;

    traits<DynamicTypeMember>::ref_type _this();

private:

    DynamicTypeMember(
            const DynamicTypeMember&) = delete;

    DynamicTypeMember(
            DynamicTypeMember&&) = delete;

    DynamicTypeMember& operator =(
            const DynamicTypeMember&) = delete;

    DynamicTypeMember& operator =(
            DynamicTypeMember&&) = delete;
};

typedef std::map<ObjectName, traits<DynamicTypeMember>::ref_type> DynamicTypeMembersByName;

typedef std::map<MemberId, traits<DynamicTypeMember>::ref_type> DynamicTypeMembersById;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICTYPEMEMBER_HPP
