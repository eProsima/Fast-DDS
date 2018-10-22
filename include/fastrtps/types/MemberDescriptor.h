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
#include <fastrtps/types/DynamicTypePtr.h>

namespace eprosima{
namespace fastrtps{
namespace types{

class DynamicType;

class MemberDescriptor
{
public:
    RTPS_DllAPI MemberDescriptor();
    RTPS_DllAPI MemberDescriptor(uint32_t index, const std::string& name);
    RTPS_DllAPI MemberDescriptor(MemberId id, const std::string& name, DynamicType_ptr mType);
    RTPS_DllAPI MemberDescriptor(MemberId id, const std::string& name, DynamicType_ptr mType,
        const std::string& defaultValue);
    RTPS_DllAPI MemberDescriptor(MemberId id, const std::string& name, DynamicType_ptr mType,
        const std::string& defaultValue, const std::vector<uint64_t>& unionLabels, bool isDefaultLabel);
    RTPS_DllAPI MemberDescriptor(const MemberDescriptor* descriptor);
    RTPS_DllAPI ~MemberDescriptor();

    bool CheckUnionLabels(const std::vector<uint64_t>& labels) const;
    RTPS_DllAPI ResponseCode CopyFrom(const MemberDescriptor* other);
    RTPS_DllAPI bool Equals(const MemberDescriptor* other) const;
    RTPS_DllAPI TypeKind GetKind() const;
    RTPS_DllAPI MemberId GetId() const;
    RTPS_DllAPI  uint32_t GetIndex() const;
    RTPS_DllAPI std::string GetName() const;
    RTPS_DllAPI std::vector<uint64_t> GetUnionLabels() const;
    RTPS_DllAPI bool IsDefaultUnionValue() const;
    RTPS_DllAPI bool IsConsistent(TypeKind parentKind) const;

    RTPS_DllAPI void AddUnionCaseIndex(uint64_t value);
    RTPS_DllAPI void SetId(MemberId id);
    RTPS_DllAPI void SetIndex(uint32_t index);
    RTPS_DllAPI void SetName(const std::string& name);
    RTPS_DllAPI void SetType(DynamicType_ptr type);
    RTPS_DllAPI void SetDefaultUnionValue(bool bDefault);

protected:

    friend class DynamicTypeBuilderFactory;
    friend class DynamicData;
    friend class DynamicTypeMember;
    friend class TypeObjectFactory;

    bool IsDefaultValueConsistent(const std::string& sDefaultValue) const;

    bool IsTypeNameConsistent(const std::string& sName) const;

    std::string mName;                  // Name of the member
    MemberId mId;                       // MemberId, it should be filled automatically when the member is added.
    DynamicType_ptr mType;              // Member's Type.
    std::string mDefaultValue;          // Default value of the member in string.
    uint32_t mIndex;                    // Definition order of the member inside it's parent.
    std::vector<uint64_t> mLabels;      // Case Labels for unions.
    bool mDefaultLabel;                 // TRUE if it's the default option of a union.
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_MEMBER_DESCRIPTOR_H
