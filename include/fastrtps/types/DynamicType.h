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
#include <fastrtps/TopicDataType.h>

namespace eprosima{
namespace fastrtps{
namespace types{

class AnnotationDescriptor;
class TypeDescriptor;
class DynamicTypeMember;

class DynamicType : public eprosima::fastrtps::TopicDataType
{
public:

    virtual ~DynamicType();

    bool Equals(const DynamicType* other) const;

    ResponseCode GetAllMembers(std::map<MemberId, DynamicTypeMember*>& members);
    ResponseCode GetAllMembersByName(std::map<std::string, DynamicTypeMember*>& members);
    ResponseCode GetAnnotation(AnnotationDescriptor& descriptor, uint32_t idx);
    uint32_t GetAnnotationCount();
    DynamicType* GetBaseType() const;
    uint32_t GetBounds(uint32_t index = 0) const;
    uint32_t GetTotalBounds() const;
    uint32_t GetBoundsSize() const;
    ResponseCode GetDescriptor(TypeDescriptor* descriptor) const;
    DynamicType* GetElementType() const;
    DynamicType* GetKeyElementType() const;
    inline TypeKind GetKind() const { return mKind; }
    std::string GetName() const;
    ResponseCode GetMember(DynamicTypeMember& member, MemberId id);
    ResponseCode GetMemberByName(DynamicTypeMember& member, const std::string name);
    MemberId GetMembersCount() const;

    bool HasChildren() const;
    bool IsComplexKind() const;
    bool IsConsistent() const;
    bool IsDiscriminatorType() const;

    void* createData();
    void deleteData(void * data);
    bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data);
    bool getKey(void *data, eprosima::fastrtps::rtps::InstanceHandle_t *ihandle);
    std::function<uint32_t()> getSerializedSizeProvider(void* data);
    bool serialize(void *data, eprosima::fastrtps::rtps::SerializedPayload_t *payload);

protected:

    friend class DynamicTypeBuilder;
    friend class DynamicTypeBuilderFactory;
    friend class MemberDescriptor;
    friend class TypeDescriptor;
    friend class DynamicData;
    friend class AnnotationDescriptor;
    friend class TypeObjectFactory;
    friend class DynamicTypeMember;

    DynamicType();
    DynamicType(const TypeDescriptor* descriptor);
    DynamicType(const DynamicType* other);

    virtual void Clear();

    ResponseCode CopyFromType(const DynamicType* other);

    bool GetKeyAnnotation() const;
    uint32_t GetKeyMaxCdrSerializedSize();
    uint32_t GetMaxSerializedSize();

    // Checks if there is a member with the given name.
    bool ExistsMemberByName(const std::string& name) const;

    void RefreshMaxSerializeSize();

    // This method is used by Dynamic Data to override the name of the types based on ALIAS.
    void SetName(const std::string& name);

    ResponseCode _ApplyAnnotation(AnnotationDescriptor& descriptor);
    ResponseCode _ApplyAnnotation(const std::string& key, const std::string& value);
    ResponseCode _ApplyAnnotationToMember(MemberId id, AnnotationDescriptor& descriptor);
    ResponseCode _ApplyAnnotationToMember(MemberId id, const std::string& key, const std::string& value);

    TypeDescriptor* mDescriptor;
	std::vector<AnnotationDescriptor*> mAnnotation;
	std::map<MemberId, DynamicTypeMember*> mMemberById;         // Aggregated members
    std::map<std::string, DynamicTypeMember*> mMemberByName;    // Uses the pointers from "mMemberById".
    std::string mName;
	TypeKind mKind;

    MD5 m_md5;
    unsigned char* m_keyBuffer;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_H
