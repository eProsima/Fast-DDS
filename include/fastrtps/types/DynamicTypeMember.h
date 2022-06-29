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
protected:

    DynamicType* parent_;
    MemberDescriptor descriptor_;
    MemberId id_;

    uint32_t get_index() const;

    void set_index(
            uint32_t index);

    void set_parent(
            DynamicType* pType);

    friend class DynamicTypeBuilder;
    friend class DynamicType;
    friend class DynamicData;

public:

    RTPS_DllAPI DynamicTypeMember();

    RTPS_DllAPI DynamicTypeMember(
            const DynamicTypeMember* other);

    RTPS_DllAPI DynamicTypeMember(
            const MemberDescriptor* descriptor,
            MemberId id);

    ~DynamicTypeMember();

    RTPS_DllAPI ReturnCode_t apply_annotation(
            AnnotationDescriptor& descriptor);

    RTPS_DllAPI ReturnCode_t apply_annotation(
            const std::string& annotation_name,
            const std::string& key,
            const std::string& value);

    RTPS_DllAPI bool equals(
            const DynamicTypeMember*) const;

    RTPS_DllAPI ReturnCode_t get_annotation(
            AnnotationDescriptor& descriptor,
            uint32_t idx);

    RTPS_DllAPI uint32_t get_annotation_count();

    RTPS_DllAPI bool key_annotation() const;

    RTPS_DllAPI std::vector<uint64_t> get_union_labels() const;

    RTPS_DllAPI ReturnCode_t get_descriptor(
            MemberDescriptor* descriptor) const;

    RTPS_DllAPI MemberId get_id() const;

    RTPS_DllAPI std::string get_name() const;

    RTPS_DllAPI bool is_default_union_value() const;

    RTPS_DllAPI const MemberDescriptor* get_descriptor() const
    {
        return &descriptor_;
    }

};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_MEMBER_H
