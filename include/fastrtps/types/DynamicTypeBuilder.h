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
protected:
    DynamicTypeBuilder();

    DynamicTypeBuilder(const DynamicTypeBuilder* builder);

    DynamicTypeBuilder(const TypeDescriptor* descriptor);

    virtual ~DynamicTypeBuilder();

    friend class DynamicType;
    friend class DynamicTypeBuilderFactory;

    TypeDescriptor* descriptor_;
    std::map<MemberId, DynamicTypeMember*> member_by_id_;         // Aggregated members
    std::map<std::string, DynamicTypeMember*> member_by_name_;    // Uses the pointers from "member_by_id_".
    std::string name_;
    TypeKind kind_;
    MemberId current_member_id_;
    uint32_t max_index_;

    ResponseCode _apply_annotation_to_member(
            MemberId id,
            AnnotationDescriptor& descriptor);

    ResponseCode _apply_annotation_to_member(
            MemberId id,
            const std::string& key,
            const std::string& value);

    bool check_union_configuration(const MemberDescriptor* descriptor);

    // Checks if there is a member with the given name.
    bool exists_member_by_name(const std::string& name) const;

    void refresh_member_ids();

    void clear();

    ResponseCode copy_from_builder(const DynamicTypeBuilder* other);

public:
    RTPS_DllAPI ResponseCode add_empty_member(
            uint32_t index,
            const std::string& name);

    RTPS_DllAPI ResponseCode add_member(const MemberDescriptor* descriptor);

    RTPS_DllAPI ResponseCode add_member(
            MemberId id,
            const std::string& name,
            DynamicTypeBuilder* type_ = nullptr);

    RTPS_DllAPI ResponseCode add_member(
            MemberId id,
            const std::string& name,
            DynamicTypeBuilder* type_,
            const std::string& defaultValue);

    RTPS_DllAPI ResponseCode add_member(
            MemberId id,
            const std::string& name,
            DynamicTypeBuilder* type_,
            const std::string& defaultValue,
            const std::vector<uint64_t>& unionLabels,
            bool isDefaultLabel);

    RTPS_DllAPI ResponseCode add_member(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type_ = nullptr);

    RTPS_DllAPI ResponseCode add_member(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type_,
            const std::string& defaultValue);

    RTPS_DllAPI ResponseCode add_member(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type_,
            const std::string& defaultValue,
            const std::vector<uint64_t>& unionLabels,
            bool isDefaultLabel);

    RTPS_DllAPI ResponseCode apply_annotation(AnnotationDescriptor& descriptor);

    RTPS_DllAPI ResponseCode apply_annotation(
            const std::string& key,
            const std::string& value);

    RTPS_DllAPI ResponseCode apply_annotation_to_member(
            MemberId id,
            AnnotationDescriptor& descriptor);

    RTPS_DllAPI ResponseCode apply_annotation_to_member(
            MemberId id,
            const std::string& key,
            const std::string& value);

    RTPS_DllAPI DynamicType_ptr build();

    RTPS_DllAPI ResponseCode copy_from(const DynamicTypeBuilder* other);

    ResponseCode get_all_members(std::map<MemberId, DynamicTypeMember*>& members);

    RTPS_DllAPI inline TypeKind get_kind() const
    {
        return kind_;
    }

    RTPS_DllAPI std::string get_name() const;

    const TypeDescriptor* get_type_descriptor() const
    {
        return descriptor_;
    }

    bool is_consistent() const;

    bool is_discriminator_type() const;

    RTPS_DllAPI ResponseCode set_name(const std::string& name);
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_BUILDER_H
