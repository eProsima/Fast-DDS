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

#ifndef TYPES_DYNAMIC_TYPE_MEMBER_H
#define TYPES_DYNAMIC_TYPE_MEMBER_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/MemberDescriptor.h>

namespace eprosima {
namespace fastrtps {
namespace types {

class AnnotationDescriptor;
class DynamicType;

class DynamicTypeMember
{
public:

    DynamicTypeMember();
    DynamicTypeMember(const DynamicTypeMember* other);
    DynamicTypeMember(const MemberDescriptor* descriptor, MemberId id);

    ~DynamicTypeMember();

    ResponseCode ApplyAnnotation(AnnotationDescriptor& descriptor);
    ResponseCode ApplyAnnotation(const std::string& key, const std::string& value);
    bool Equals(const DynamicTypeMember*) const;

    ResponseCode GetAnnotation(AnnotationDescriptor& descriptor, uint32_t idx);
    uint32_t GetAnnotationCount();
    bool GetKeyAnnotation() const;

    std::vector<uint64_t> GetUnionLabels() const;
    ResponseCode GetDescriptor(MemberDescriptor* descriptor) const;
    MemberId GetId() const;
    std::string GetName() const;
    bool IsDefaultUnionValue() const;

    const MemberDescriptor* GetDescriptor() const
    {
        return &mDescriptor;
    }

protected:

    uint32_t GetIndex() const;
    void SetIndex(uint32_t index);
    void SetParent(DynamicType* pType);

    friend class DynamicTypeBuilder;
    friend class DynamicType;
    friend class DynamicData;

    DynamicType* mParent;
    MemberDescriptor mDescriptor;
    std::vector<AnnotationDescriptor*> mAnnotation;
    MemberId mId;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_MEMBER_H
