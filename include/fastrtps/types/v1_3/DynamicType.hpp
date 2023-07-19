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

#ifndef TYPES_1_3_DYNAMIC_TYPE_H
#define TYPES_1_3_DYNAMIC_TYPE_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/v1_3/AnnotationDescriptor.hpp>
#include <fastrtps/types/v1_3/DynamicTypeMember.hpp>
#include <fastrtps/types/v1_3/TypeDescriptor.hpp>

#include <cstdint>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class DynamicType final
{
    DynamicType() noexcept = default;

    friend class DynamicTypeImpl;

public:

    RTPS_DllAPI bool operator ==(
            const DynamicType& other) const noexcept;

    RTPS_DllAPI bool operator !=(
            const DynamicType& other) const noexcept;

    /**
     * Provides a summary of the state of this type overwriting a provided object (see [standard] 7.5.2.8.7)
     * @param[inout] descriptor object
     * @return standard @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference.
     */
    RTPS_DllAPI ReturnCode_t get_descriptor(TypeDescriptor& td) const noexcept;

    /**
     * Returns the fully qualified name of this type
     * @attention The returned value may not persist in time
     * @return const char* type name
     */
    RTPS_DllAPI const char* get_name() const noexcept;

    /**
     * Returns the @ref eprosima::fastrtps::types::TypeKind associated
     * @return standard @ref eprosima::fastrtps::types::TypeKind
     */
    RTPS_DllAPI TypeKind get_kind() const noexcept;

    /**
     * Provides a mapping from the name of the member of this type to the member itself (see [standard] 7.5.2.8.3)
     * @param[inout] name member name as null terminated string
     * @param[out] ec @ref ReturnCode_t
     * @return @ref DynamicTypeMember
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Lifecycle is linked to the referenced member.
     */
    RTPS_DllAPI const DynamicTypeMember* get_member_by_name(const char* name, ReturnCode_t* ec = nullptr) const noexcept;

    /**
     * Provides a mapping from the name of the member of this type to the member itself (see [standard] 7.5.2.8.3)
     * @param[out] ec @ref ReturnCode_t
     * @return @ref DynamicTypeMembersByName
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Lifecycle is linked to the referenced members.
     */
    RTPS_DllAPI DynamicTypeMembersByName get_all_members_by_name(ReturnCode_t* ec = nullptr) const noexcept;

    /**
     * Provides a mapping from the member id to the member itself (see [standard] 7.5.2.8.2)
     * @param[inout] id @ref MemberId
     * @param[out] ec @ref ReturnCode_t
     * @return @ref DynamicTypeMember
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Lifecycle is linked to the referenced member.
     */
    RTPS_DllAPI const DynamicTypeMember* get_member(MemberId id, ReturnCode_t* ec = nullptr) const noexcept;

    /**
     * Provides a mapping from the member id to the member itself (see [standard] 7.5.2.8.2)
     * @param[out] ec @ref ReturnCode_t
     * @return @ref DynamicTypeMembersById
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Lifecycle is linked to the referenced members.
     */
    RTPS_DllAPI DynamicTypeMembersById get_all_members(ReturnCode_t* ec = nullptr) const noexcept;

    /**
     * This operation returns the current number of members (see [standard] 7.5.2.8.12)
     * @return current number of members
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    RTPS_DllAPI uint32_t get_member_count() const noexcept;

    /**
     * This operation returns the member that corresponds to the specified index (see [standard] 7.5.2.8.10)
     * @param[in] index uint32_t
     * @param[out] ec @ref ReturnCode_t
     * @return @ref DynamicTypeMember
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Lifecycle is linked to the referenced members.
     */
    RTPS_DllAPI const DynamicTypeMember* get_member_by_index(uint32_t index, ReturnCode_t* ec = nullptr) const noexcept;

    /**
     * This operation returns the current number of annotations to the type (see [standard] 7.5.2.8.7)
     * @return current number of annotations
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    RTPS_DllAPI uint32_t get_annotation_count() const noexcept;

    /**
     * This operation returns the annotation that corresponds to the specified index (see [standard] 7.5.2.8.10)
     * @param[in] annotation @ref AnnotationDescriptor
     * @param[in] index uin32_t
     * @return @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Lifecycle is linked to the referenced members.
     */
    RTPS_DllAPI ReturnCode_t get_annotation(AnnotationDescriptor& annotation, uint32_t index) const noexcept;

    /**
     * This operation returns all annotations that have previously been applied to this type (see [standard] 7.5.2.8.10)
     * @param[out] ec @ref ReturnCode_t
     * @return @ref Annotations
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Lifecycle is linked to the referenced members.
     */
    RTPS_DllAPI const Annotations* get_all_annotations(ReturnCode_t* ec = nullptr) const noexcept;

    //! checks if type is a valid key
    RTPS_DllAPI bool key_annotation() const;

    /**
     * State comparison according with the [standard] sections \b 7.5.2.7.4 \b 7.5.2.8.4
     * @remarks using `==` and `!=` operators is more convenient
     * @param[in] other object state to compare to
     * @return \b bool `true` on equality
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    RTPS_DllAPI bool equals(
            const DynamicType& other) const noexcept;
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_TYPE_H
