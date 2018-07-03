// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPES_MEMBER_DESCRIPTOR_H
#define TYPES_MEMBER_DESCRIPTOR_H

#include <fastrtps/types/TypesBase.h>

namespace eprosima{
namespace fastrtps{
namespace types{

class DynamicType;

class MemberDescriptor
{
public:
    MemberDescriptor();
    MemberDescriptor(const MemberDescriptor* descriptor);

    ~MemberDescriptor();

    ResponseCode copy_from(const MemberDescriptor* other);

    bool equals(const MemberDescriptor* other) const;

    bool isConsistent() const;

    MemberId get_id() const;
    TypeKind get_kind() const;
    std::string get_name() const;
    uint32_t get_index() const;
    void set_index(uint32_t index);

protected:

    friend class DynamicTypeBuilderFactory;
    friend class DynamicData;

    std::string mName;                  // Name of the member
    MemberId mId;                       // MemberId, it should be filled automatically when the member is added.
    DynamicType* mType;                 // Member's Type.
    std::string mDefaultValue;          // Default value of the member in string.
    uint32_t mIndex;                    // Definition order of the member inside it's parent.
    std::vector<uint64_t> mLabel;       // Case Labels for unions.
    bool mDefaultLabel;                 // TRUE if it's the default option of a union.
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_MEMBER_DESCRIPTOR_H
