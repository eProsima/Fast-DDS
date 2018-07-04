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

#ifndef TYPES_DYNAMIC_TYPE_H
#define TYPES_DYNAMIC_TYPE_H

#include <fastrtps/types/TypesBase.h>

namespace eprosima{
namespace fastrtps{
namespace types{

class AnnotationDescriptor;
class TypeDescriptor;
class DynamicTypeMember;

class DynamicType
{
public:
    DynamicType();
    DynamicType(const TypeDescriptor* descriptor);

    virtual ~DynamicType();

    bool Equals(const DynamicType* other) const;

    ResponseCode GetAllMembers(std::map<MemberId, DynamicTypeMember*>& members);
    ResponseCode GetAllMembersByName(std::map<std::string, DynamicTypeMember*>& members);
    ResponseCode GetAnnotation(AnnotationDescriptor& descriptor, uint32_t idx);
    uint32_t GetAnnotationCount();
    DynamicType* GetBaseType() const;
    uint32_t GetBounds(uint32_t index = 0) const;
    uint32_t GetBoundsSize() const;
    ResponseCode GetDescriptor(TypeDescriptor* descriptor) const;
    DynamicType* GetElementType() const;
    DynamicType* GetKeyElementType() const;
    TypeKind GetKind() const;
    std::string GetName() const;
    ResponseCode GetMember(DynamicTypeMember& member, MemberId id);
    ResponseCode GetMemberByName(DynamicTypeMember& member, const std::string name);

    bool IsComplexKind() const;

protected:

    friend class DynamicTypeBuilder;
    friend class DynamicTypeBuilderFactory;
    friend class MemberDescriptor;
    friend class TypeDescriptor;
    friend class DynamicData;
    friend class AnnotationDescriptor;

    DynamicType(const DynamicType* other);

    virtual void Clear();

    ResponseCode CopyFromType(const DynamicType* other);

    // This method is used by Dynamic Data to override the name of the types based on ALIAS.
    void SetName(const std::string& name);

    TypeDescriptor* mDescriptor;
	std::vector<AnnotationDescriptor*> mAnnotation;
	std::map<MemberId, DynamicTypeMember*> mMemberById;         // Aggregated members
    std::map<std::string, DynamicTypeMember*> mMemberByName;    // Uses the pointers from "mMemberById".
    std::string mName;
	TypeKind mKind;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_H
