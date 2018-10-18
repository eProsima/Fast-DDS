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
#include <fastrtps/types/DynamicTypePtr.h>

namespace eprosima {
namespace fastrtps {
namespace types {

class AnnotationDescriptor;
class TypeDescriptor;
class DynamicTypeMember;
class DynamicTypeBuilder;

class DynamicType
{
public:

    bool Equals(const DynamicType* other) const;

    ResponseCode GetAllMembers(std::map<MemberId, DynamicTypeMember*>& members);
    ResponseCode GetAllMembersByName(std::map<std::string, DynamicTypeMember*>& members);

    uint32_t GetBounds(uint32_t index = 0) const;
    uint32_t GetBoundsSize() const;

    ResponseCode GetDescriptor(TypeDescriptor* descriptor) const;

    bool GetKeyAnnotation() const;
    inline TypeKind GetKind() const
    {
        return mKind;
    }
    std::string GetName() const;
    MemberId GetMembersCount() const;
    uint32_t GetTotalBounds() const;

    const TypeDescriptor* getTypeDescriptor() const
    {
        return mDescriptor;
    }

    bool HasChildren() const;
    bool IsConsistent() const;
    bool IsComplexKind() const;
    bool IsDiscriminatorType() const;

protected:

    friend class DynamicTypeBuilder;
    friend class DynamicTypeBuilderFactory;
    friend class MemberDescriptor;
    friend class TypeDescriptor;
    friend class DynamicData;
    friend class DynamicDataFactory;
    friend class AnnotationDescriptor;
    friend class TypeObjectFactory;
    friend class DynamicTypeMember;

    DynamicType();
    DynamicType(const TypeDescriptor* descriptor);
    DynamicType(const DynamicTypeBuilder* other);

    virtual ~DynamicType();

    virtual void Clear();

    ResponseCode CopyFromBuilder(const DynamicTypeBuilder* other);

    // Checks if there is a member with the given name.
    bool ExistsMemberByName(const std::string& name) const;

    // This method is used by Dynamic Data to override the name of the types based on ALIAS.
    void SetName(const std::string& name);

    ResponseCode ApplyAnnotation(AnnotationDescriptor& descriptor);
    ResponseCode ApplyAnnotation(const std::string& key, const std::string& value);
    ResponseCode ApplyAnnotationToMember(MemberId id, AnnotationDescriptor& descriptor);
    ResponseCode ApplyAnnotationToMember(MemberId id, const std::string& key, const std::string& value);

    ResponseCode GetAnnotation(AnnotationDescriptor& descriptor, uint32_t idx);
    uint32_t GetAnnotationCount();
    DynamicType_ptr GetBaseType() const;
    DynamicType_ptr GetDiscriminatorType() const;
    DynamicType_ptr GetElementType() const;
    DynamicType_ptr GetKeyElementType() const;
    ResponseCode GetMember(DynamicTypeMember& member, MemberId id);
    ResponseCode GetMemberByName(DynamicTypeMember& member, const std::string& name);

    TypeDescriptor* mDescriptor;
    std::vector<AnnotationDescriptor*> mAnnotation;
    std::map<MemberId, DynamicTypeMember*> mMemberById;         // Aggregated members
    std::map<std::string, DynamicTypeMember*> mMemberByName;    // Uses the pointers from "mMemberById".
    std::string mName;
    TypeKind mKind;
    bool mIsKeyDefined;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_H
