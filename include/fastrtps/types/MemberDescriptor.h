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

#include <fastrtps/types/MemberId.h>

namespace eprosima{
namespace fastrtps{
namespace types{

class DynamicType;

class MemberDescriptor
{
public:
    MemberDescriptor();
    MemberDescriptor(const MemberDescriptor* descriptor);

    ResponseCode copy_from(const MemberDescriptor* other);

    bool equals(const MemberDescriptor* other) const;

    bool isConsistent() const;

    std::string get_name() const;
    MemberId get_id() const;

protected:

    std::string mName;
    MemberId mId;
    DynamicType* mType;
    std::string mDefaultValue;
    uint32_t mIndex;
    std::vector<uint64_t> mLabel;
    bool mDefaultLabel;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_MEMBER_DESCRIPTOR_H
