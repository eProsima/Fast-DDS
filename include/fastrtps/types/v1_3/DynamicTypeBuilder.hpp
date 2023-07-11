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

#ifndef TYPES_1_3_DYNAMIC_TYPE_BUILDER_H
#define TYPES_1_3_DYNAMIC_TYPE_BUILDER_H

#include <fastrtps/types/v1_3/AnnotationDescriptor.hpp>
#include <fastrtps/types/v1_3/DynamicTypeMember.hpp>
#include <fastrtps/types/v1_3/TypeDescriptor.hpp>

#include <cstdint>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class DynamicTypeBuilder final
{

    DynamicTypeBuilder() noexcept = default;

    friend class DynamicTypeBuilderImpl;

public:

    bool operator ==(
            const DynamicTypeBuilder& other) const noexcept;

    bool operator !=(
            const DynamicTypeBuilder& other) const noexcept;

    /**
     * Provides a summary of the state of this type overwriting a provided object (see [standard] 7.5.2.9.8)
     * @param[inout] descriptor object
     * @return standard @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference.
     */
    ReturnCode_t get_descriptor(TypeDescriptor& td) const noexcept;

    /**
     * Returns the fully qualified name of this type
     * @attention The returned value may not persist in time
     * @return const char* type name
     */
    const char* get_name() const noexcept;

    /**
     * Returns the @ref eprosima::fastrtps::types::TypeKind associated
     * @return standard @ref eprosima::fastrtps::types::TypeKind
     */
    TypeKind get_kind() const noexcept;

    /**
     * Provides a mapping from the name of the member of this type to the member itself (see [standard] 7.5.2.9.15)
     * @param[inout] name member name as null terminated string
     * @param[out] ec @ref ReturnCode_t
     * @return @ref DynamicTypeMember
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Livecycle is linked to the referenced member.
     */
    const DynamicTypeMember* get_member_by_name(const char* name, ReturnCode_t* ec = nullptr) const noexcept;

    /**
     * Provides a mapping from the name of the member of this type to the member itself
     * @param[out] ec @ref ReturnCode_t
     * @return @ref DynamicTypeMembersByName
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Livecycle is linked to the referenced members.
     */
    DynamicTypeMembersByName get_all_members_by_name(ReturnCode_t* ec = nullptr) const noexcept;

    /**
     * Provides a mapping from the member id to the member itself (see [standard] 7.5.2.9.13)
     * @param[inout] id @ref MemberId
     * @param[out] ec @ref ReturnCode_t
     * @return @ref DynamicTypeMember
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Livecycle is linked to the referenced member.
     */
    const DynamicTypeMember* get_member(MemberId id, ReturnCode_t* ec = nullptr) const noexcept;

    /**
     * Adds a 'member' to this type, where the new 'member' has the meaning defined in the specification of
     * the DynamicTypeMember class (see [standard] 7.5.2.9.4)
     * @param[in] md @ref MemberDescriptor
     * @return ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference.
     */
    ReturnCode_t add_member(const MemberDescriptor& md) noexcept;

    /**
     * Provides a mapping from the member id to the member itself
     * @param[out] ec @ref ReturnCode_t
     * @return @ref DynamicTypeMembersById
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Livecycle is linked to the referenced members.
     */
    DynamicTypeMembersById get_all_members(ReturnCode_t* ec = nullptr) const noexcept;

    /**
     * This operation returns the current number of members (see [standard] 7.5.2.9.16)
     * @return current number of members
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    uint32_t get_member_count() const noexcept;

    /**
     * This operation returns the member that corresponds to the specified index (see [standard] 7.5.2.9.14)
     * @param[in] index uint32_t
     * @param[out] ec @ref ReturnCode_t
     * @return @ref DynamicTypeMember
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Livecycle is linked to the referenced members.
     */
    const DynamicTypeMember* get_member_by_index(uint32_t index, ReturnCode_t* ec = nullptr) const noexcept;

    /**
     * This operation returns the current number of annotations to the type (see [standard] 7.5.2.9.11)
     * @return current number of annotations
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    uint32_t get_annotation_count() const noexcept;

    /**
     * This operation returns the annotation that corresponds to the specified index (see [standard] 7.5.2.9.10)
     * @param[in] annotation @ref AnnotationDescriptor
     * @param[in] index uin32_t
     * @return @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Livecycle is linked to the referenced members.
     */
    ReturnCode_t get_annotation(AnnotationDescriptor& annotation, uint32_t index) const noexcept;

    /**
     * Apply the given annotation to this type (see [standard] 7.5.2.9.5)
     * @param[in] annotation @ref AnnotationDescriptor
     * @return @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Livecycle is linked to the referenced members.
     */
    ReturnCode_t apply_annotation(const AnnotationDescriptor& annotation) noexcept;

    /**
     * Apply the given annotation to this type (see [standard] 7.5.2.9.6)
     * @param[in] id @ref MemberId member identifier
     * @param[in] annotation @ref AnnotationDescriptor
     * @return @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Livecycle is linked to the referenced members.
     */
    ReturnCode_t apply_annotation_to_member(MemberId id, const AnnotationDescriptor& annotation) noexcept;

    /**
     * This operation returns all annotations that have previously been applied to this type
     * @param[out] ec @ref ReturnCode_t
     * @return @ref Annotations
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Livecycle is linked to the referenced members.
     */
    const Annotations* get_all_annotations(ReturnCode_t* ec = nullptr) const noexcept;

    /**
     * Create an immutable DynamicType object containing a snapshot of this builder's current state
     * according with [standard] section \b 7.5.2.9.7
     * @return @ref DynamicType
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    const DynamicType* build() const noexcept;

    /**
     * State comparison according with the [standard] section \b 7.5.2.9.9
     * @remarks using `==` and `!=` operators is more convenient
     * @param[in] other object state to compare to
     * @return \b bool `true` on equality
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    bool equals(
            const DynamicType& other) const noexcept;
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_TYPE_BUILDER_H
