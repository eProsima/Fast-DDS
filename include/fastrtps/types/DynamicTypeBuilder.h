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
#include <fastrtps/types/DynamicType.h>


namespace eprosima{
namespace fastrtps{
namespace types{

class AnnotationDescriptor;
class TypeDescriptor;
class MemberDescriptor;
class DynamicType;

class DynamicTypeBuilder : public DynamicType
{
public:
    DynamicTypeBuilder();
    DynamicTypeBuilder(const TypeDescriptor* descriptor);
    DynamicTypeBuilder(const DynamicType* pType);

    virtual ~DynamicTypeBuilder();

    RTPS_DllAPI ResponseCode add_member(const MemberDescriptor* descriptor);

    RTPS_DllAPI ResponseCode apply_annotation(AnnotationDescriptor& descriptor);

    RTPS_DllAPI ResponseCode apply_annotation_to_member(MemberId id, AnnotationDescriptor& descriptor);

    RTPS_DllAPI DynamicType* build();

    RTPS_DllAPI ResponseCode copy_from(const DynamicTypeBuilder* other);
protected:

    MemberId mCurrentMemberId;
    uint32_t mMaxIndex;

    virtual void Clear() override;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_BUILDER_H
