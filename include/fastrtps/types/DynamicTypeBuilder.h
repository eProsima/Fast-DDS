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

#include <fastrtps/types/TypeDescriptor.h>

namespace eprosima {
namespace fastrtps {
namespace types {

class AnnotationDescriptor;
class TypeDescriptor;
class MemberDescriptor;
class DynamicType;
class DynamicTypeMember;

class DynamicTypeBuilder
    : protected TypeDescriptor
    , public std::enable_shared_from_this<DynamicTypeBuilder>
{
    // Only create objects from the associated factory
    struct use_the_create_method
    {
        explicit use_the_create_method() = default;
    };

    std::map<MemberId, DynamicTypeMember*> member_by_id_;         // Aggregated members
    std::map<std::string, DynamicTypeMember*> member_by_name_;    // Uses the pointers from "member_by_id_".
    MemberId current_member_id_ = 0;
    uint32_t max_index_ = 0;

    bool check_union_configuration(
            const MemberDescriptor* descriptor);

    // Checks if there is a member with the given name.
    bool exists_member_by_name(
            const std::string& name) const;

    void refresh_member_ids();

    void clear();

    DynamicTypeBuilder(const DynamicTypeBuilder&) = default;
    DynamicTypeBuilder(DynamicTypeBuilder&&) = delete;
    DynamicTypeBuilder& operator=(const DynamicTypeBuilder&) = default;
    DynamicTypeBuilder& operator=(DynamicTypeBuilder&&) = delete;

    // Annotation setters
    void annotation_set_extensibility(
            const std::string& extensibility);

    void annotation_set_mutable();

    void annotation_set_final();

    void annotation_set_appendable();

    void annotation_set_nested(
            bool nested);

    void annotation_set_bit_bound(
            uint16_t bit_bound);

    void annotation_set_key(
            bool key);

    void annotation_set_non_serialized(
            bool non_serialized);

public:

    DynamicTypeBuilder(
            use_the_create_method);

    DynamicTypeBuilder(
            use_the_create_method,
            const DynamicTypeBuilder* builder);

    DynamicTypeBuilder(
            use_the_create_method,
            const TypeDescriptor* descriptor);

    virtual ~DynamicTypeBuilder();

    friend class DynamicTypeBuilderFactory;

    RTPS_DllAPI ReturnCode_t add_empty_member(
            uint32_t index,
            const std::string& name);

    RTPS_DllAPI ReturnCode_t add_member(
            const MemberDescriptor* descriptor);

    RTPS_DllAPI ReturnCode_t add_member(
            MemberId id,
            const std::string& name,
            DynamicTypeBuilder* type_ = nullptr);

    RTPS_DllAPI ReturnCode_t add_member(
            MemberId id,
            const std::string& name,
            DynamicTypeBuilder* type_,
            const std::string& defaultValue);

    RTPS_DllAPI ReturnCode_t add_member(
            MemberId id,
            const std::string& name,
            DynamicTypeBuilder* type_,
            const std::string& defaultValue,
            const std::vector<uint64_t>& unionLabels,
            bool isDefaultLabel);

    RTPS_DllAPI ReturnCode_t add_member(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type_ = nullptr);

    RTPS_DllAPI ReturnCode_t add_member(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type_,
            const std::string& defaultValue);

    RTPS_DllAPI ReturnCode_t add_member(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type_,
            const std::string& defaultValue,
            const std::vector<uint64_t>& unionLabels,
            bool isDefaultLabel);

    RTPS_DllAPI ReturnCode_t apply_annotation(
            AnnotationDescriptor& descriptor);

    RTPS_DllAPI ReturnCode_t apply_annotation(
            const std::string& annotation_name,
            const std::string& key,
            const std::string& value);

    RTPS_DllAPI ReturnCode_t apply_annotation_to_member(
            MemberId id,
            AnnotationDescriptor& descriptor);

    RTPS_DllAPI ReturnCode_t apply_annotation_to_member(
            MemberId id,
            const std::string& annotation_name,
            const std::string& key,
            const std::string& value);

    RTPS_DllAPI DynamicType_ptr build() const;

    RTPS_DllAPI ReturnCode_t copy_from(
            const DynamicTypeBuilder* other);

    ReturnCode_t get_all_members(
            std::map<MemberId, DynamicTypeMember*>& members);

    RTPS_DllAPI inline TypeKind get_kind() const
    {
        return kind_;
    }

    using TypeDescriptor::get_name;

    RTPS_DllAPI MemberId get_member_id_by_name(
            const std::string& name) const;

    const TypeDescriptor& get_type_descriptor() const
    {
        return static_cast<const TypeDescriptor&>(*this);
    }

    using TypeDescriptor::is_consistent;

    bool is_discriminator_type() const;

    using TypeDescriptor::set_name;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_BUILDER_H
