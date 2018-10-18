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

#ifndef TYPES_DYNAMIC_TYPE_BUILDER_H
#define TYPES_DYNAMIC_TYPE_BUILDER_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/DynamicTypePtr.h>

namespace eprosima {
namespace fastrtps {
namespace types {

class AnnotationDescriptor;
class TypeDescriptor;
class MemberDescriptor;
class DynamicType;
class DynamicTypeMember;

class DynamicTypeBuilder
{
public:

    RTPS_DllAPI ResponseCode AddEmptyMember(uint32_t index, const std::string& name);
    RTPS_DllAPI ResponseCode AddMember(const MemberDescriptor* descriptor);
    RTPS_DllAPI ResponseCode AddMember(MemberId id, const std::string& name, DynamicTypeBuilder* mType = nullptr);
    RTPS_DllAPI ResponseCode AddMember(MemberId id, const std::string& name, DynamicTypeBuilder* mType,
        const std::string& defaultValue);
    RTPS_DllAPI ResponseCode AddMember(MemberId id, const std::string& name, DynamicTypeBuilder* mType,
        const std::string& defaultValue, const std::vector<uint64_t>& unionLabels, bool isDefaultLabel);
    RTPS_DllAPI ResponseCode AddMember(MemberId id, const std::string& name, DynamicType_ptr mType = nullptr);
    RTPS_DllAPI ResponseCode AddMember(MemberId id, const std::string& name, DynamicType_ptr mType,
        const std::string& defaultValue);
    RTPS_DllAPI ResponseCode AddMember(MemberId id, const std::string& name, DynamicType_ptr mType,
        const std::string& defaultValue, const std::vector<uint64_t>& unionLabels, bool isDefaultLabel);

    RTPS_DllAPI ResponseCode ApplyAnnotation(AnnotationDescriptor& descriptor);
    RTPS_DllAPI ResponseCode ApplyAnnotation(std::string key, std::string value);
    RTPS_DllAPI ResponseCode ApplyAnnotationToMember(MemberId id, AnnotationDescriptor& descriptor);
    RTPS_DllAPI ResponseCode ApplyAnnotationToMember(MemberId id, std::string key, std::string value);


    RTPS_DllAPI DynamicType_ptr Build();

    RTPS_DllAPI ResponseCode CopyFrom(const DynamicTypeBuilder* other);

    ResponseCode GetAllMembers(std::map<MemberId, DynamicTypeMember*>& members);
    RTPS_DllAPI inline TypeKind GetKind() const
    {
        return mKind;
    }
    RTPS_DllAPI std::string GetName() const;
    const TypeDescriptor* getTypeDescriptor() const
    {
        return mDescriptor;
    }

    bool IsConsistent() const;
    bool IsDiscriminatorType() const;

    RTPS_DllAPI ResponseCode SetName(const std::string& name);
protected:

    DynamicTypeBuilder();
    DynamicTypeBuilder(const DynamicTypeBuilder* builder);
    DynamicTypeBuilder(const TypeDescriptor* descriptor);

    virtual ~DynamicTypeBuilder();

    friend class DynamicType;
    friend class DynamicTypeBuilderFactory;

    TypeDescriptor* mDescriptor;
    std::vector<AnnotationDescriptor*> mAnnotation;
    std::map<MemberId, DynamicTypeMember*> mMemberById;         // Aggregated members
    std::map<std::string, DynamicTypeMember*> mMemberByName;    // Uses the pointers from "mMemberById".
    std::string mName;
    TypeKind mKind;
    MemberId mCurrentMemberId;
    uint32_t mMaxIndex;

    ResponseCode _ApplyAnnotation(AnnotationDescriptor& descriptor);
    ResponseCode _ApplyAnnotation(const std::string& key, const std::string& value);
    ResponseCode _ApplyAnnotationToMember(MemberId id, AnnotationDescriptor& descriptor);
    ResponseCode _ApplyAnnotationToMember(MemberId id, const std::string& key, const std::string& value);

    bool CheckUnionConfiguration(const MemberDescriptor* descriptor);

    // Checks if there is a member with the given name.
    bool ExistsMemberByName(const std::string& name) const;

    void RefreshMemberIds();

    void Clear();

    ResponseCode CopyFromBuilder(const DynamicTypeBuilder* other);
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_BUILDER_H
