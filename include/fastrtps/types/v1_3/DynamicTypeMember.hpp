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

#ifndef TYPES_1_3_DYNAMIC_TYPE_MEMBER_H
#define TYPES_1_3_DYNAMIC_TYPE_MEMBER_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/v1_3/AnnotationDescriptor.hpp>
#include <fastrtps/types/v1_3/MemberDescriptor.hpp>
#include <fastrtps/types/v1_3/MemberId.hpp>

#include <cstdint>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class DynamicTypeMember final
{
    DynamicTypeMember() noexcept = default;

    friend class DynamicTypeMemberImpl;

public:

    bool operator ==(
            const DynamicTypeMember& descriptor) const noexcept;

    bool operator !=(
            const DynamicTypeMember& descriptor) const noexcept;

    /**
     * Provides a summary of the state of this type overwriting a provided object (see [standard] 7.5.2.6.2)
     * @param[inout] descriptor object
     * @return standard @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    RTPS_DllAPI ReturnCode_t get_descriptor(MemberDescriptor & md) const noexcept;

    /**
     * State comparison according with the [standard] sections \b 7.5.2.6.3
     * @remarks using `==` and `!=` operators is more convenient
     * @param[in] descriptor object state to compare to
     * @return \b bool `true` on equality
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    bool equals(
            const DynamicTypeMember& descriptor) const noexcept;

    /**
     * Returns the name of this member
     * @attention The returned value may not persist in time
     * @return const char* member name
     */
    const char* get_name() const noexcept;

    /**
     * Getter for @b id property (see [standard] table 52)
     * @return @ref MemberId
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    MemberId get_id() const noexcept;

    /**
     * Getter for @b annotation property (see [standard] table 52)
     * @return readonly @ref Annotations collection
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    const Annotations* get_annotation() const noexcept;

};

class DynamicTypeMemberImpl;

class DynamicTypeMembersByName final
{
    // const because we don't want to modify the ref map from this class
    using mapping = const std::map<std::string, const DynamicTypeMemberImpl*>;
    mapping* map_;

    // only retrievable from DynamicType and DynamicTypeBuilder
    DynamicTypeMembersByName() = delete;

    DynamicTypeMembersByName(mapping&& map) : map_(new mapping{std::move(map)}) {}

    friend class TypeState;

public:

    //! This class is conceived for stack use only
    RTPS_DllAPI ~DynamicTypeMembersByName()
    {
        delete map_;
    }

    /*
     * Retrieve values from keys:
     * @param key name
     * @return associated member or nullptr if not present
     */
    RTPS_DllAPI const DynamicTypeMember* operator[](const char* key) const noexcept;

    //! get collection size
    RTPS_DllAPI uint64_t size() const noexcept;

    //! check contents
    RTPS_DllAPI bool empty() const noexcept;

    /*
     * Iterate over the key elements.
     * @param key name of the previous key. Use nullptr (default) to get the first key
     * @return next key name or nullptr as end marker
     */
    RTPS_DllAPI const char* next_key(const char* key = nullptr);

};

class DynamicTypeMembersById final
{
    // const because we don't want to modify the ref map from this class
    using mapping = const std::map<MemberId, const DynamicTypeMemberImpl*>;
    mapping* map_ = nullptr;

    // only retrievable from DynamicType and DynamicTypeBuilder
    DynamicTypeMembersById() = delete;

    DynamicTypeMembersById(mapping&& map) : map_(new mapping{std::move(map)}) {}

    friend class TypeState;

public:

    //! This class is conceived for stack use only
    RTPS_DllAPI ~DynamicTypeMembersById()
    {
        delete map_;
    }

    /*
     * Retrieve values from keys:
     * @param key id
     * @return associated member or nullptr if not present
     */
    RTPS_DllAPI const DynamicTypeMember* operator[](MemberId) const noexcept;

    //! get collection size
    RTPS_DllAPI uint64_t size() const noexcept;

    //! check contents
    RTPS_DllAPI bool empty() const noexcept;

    /*
     * Iterate over the key elements.
     * @param key id of the previous key. Use MEMBER_ID_INVALID (default) to get the first key
     * @return next key id or MEMBER_ID_INVALID as end marker
     */
    RTPS_DllAPI MemberId next_key(MemberId = MEMBER_ID_INVALID);
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_TYPE_MEMBER_H
