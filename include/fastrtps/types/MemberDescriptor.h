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

namespace eprosima {
namespace fastrtps {
namespace types {

class DynamicType;
class AnnotationDescriptor;

class MemberDescriptor
{
protected:

    std::string name_;                  // Name of the member
    MemberId id_;                       // MemberId, it should be filled automatically when the member is added.
    DynamicType_ptr type_;              // Member's Type.
    std::string default_value_;         // Default value of the member in string.
    uint32_t index_;                    // Definition order of the member inside it's parent.
    std::vector<uint64_t> labels_;      // Case Labels for unions.
    bool default_label_;                // TRUE if it's the default option of a union.

    std::vector<AnnotationDescriptor*> annotation_; // Annotations to apply

    friend class DynamicTypeBuilderFactory;
    friend class DynamicData;
    friend class DynamicTypeMember;
    friend class TypeObjectFactory;

    bool is_default_value_consistent(
            const std::string& sDefaultValue) const;

    bool is_type_name_consistent(
            const std::string& sName) const;

public:

    FASTDDS_EXPORTED_API MemberDescriptor();

    FASTDDS_EXPORTED_API MemberDescriptor(
            uint32_t index,
            const std::string& name);

    FASTDDS_EXPORTED_API MemberDescriptor(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type_);

    FASTDDS_EXPORTED_API MemberDescriptor(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type_,
            const std::string& defaultValue);

    FASTDDS_EXPORTED_API MemberDescriptor(
            MemberId id,
            const std::string& name,
            DynamicType_ptr type_,
            const std::string& defaultValue,
            const std::vector<uint64_t>& unionLabels,
            bool isDefaultLabel);

    FASTDDS_EXPORTED_API MemberDescriptor(
            const MemberDescriptor* descriptor);

    FASTDDS_EXPORTED_API ~MemberDescriptor();

    bool check_union_labels(
            const std::vector<uint64_t>& labels) const;

    FASTDDS_EXPORTED_API ReturnCode_t copy_from(
            const MemberDescriptor* other);

    FASTDDS_EXPORTED_API bool equals(
            const MemberDescriptor* other) const;

    FASTDDS_EXPORTED_API TypeKind get_kind() const;

    FASTDDS_EXPORTED_API MemberId get_id() const;

    FASTDDS_EXPORTED_API uint32_t get_index() const;

    FASTDDS_EXPORTED_API std::string get_name() const;

    FASTDDS_EXPORTED_API std::vector<uint64_t> get_union_labels() const;

    FASTDDS_EXPORTED_API std::string get_default_value() const
    {
        if (!default_value_.empty())
        {
            return default_value_;
        }
        // Try annotation
        return annotation_get_default();
    }

    FASTDDS_EXPORTED_API bool is_default_union_value() const;

    FASTDDS_EXPORTED_API bool is_consistent(
            TypeKind parentKind) const;

    FASTDDS_EXPORTED_API void add_union_case_index(
            uint64_t value);

    FASTDDS_EXPORTED_API void set_id(
            MemberId id);

    FASTDDS_EXPORTED_API void set_index(
            uint32_t index);

    FASTDDS_EXPORTED_API void set_name(
            const std::string& name);

    FASTDDS_EXPORTED_API void set_type(
            DynamicType_ptr type);

    FASTDDS_EXPORTED_API DynamicType_ptr get_type() const
    {
        return type_;
    }

    FASTDDS_EXPORTED_API void set_default_union_value(
            bool bDefault);

    FASTDDS_EXPORTED_API void set_default_value(
            const std::string& value)
    {
        default_value_ = value;
    }

    // Annotations
    ReturnCode_t apply_annotation(
            AnnotationDescriptor& descriptor);

    ReturnCode_t apply_annotation(
            const std::string& annotation_name,
            const std::string& key,
            const std::string& value);

    AnnotationDescriptor* get_annotation(
            const std::string& name) const;

    // Annotations application
    FASTDDS_EXPORTED_API bool annotation_is_optional() const;

    FASTDDS_EXPORTED_API bool annotation_is_key() const;

    FASTDDS_EXPORTED_API bool annotation_is_must_understand() const;

    FASTDDS_EXPORTED_API bool annotation_is_non_serialized() const;

    FASTDDS_EXPORTED_API bool annotation_is_value() const;

    FASTDDS_EXPORTED_API bool annotation_is_default_literal() const;

    FASTDDS_EXPORTED_API bool annotation_is_position() const;

    FASTDDS_EXPORTED_API bool annotation_is_bit_bound() const;

    // Annotations getters
    FASTDDS_EXPORTED_API bool annotation_get_key() const;

    FASTDDS_EXPORTED_API std::string annotation_get_value() const;

    FASTDDS_EXPORTED_API std::string annotation_get_default() const;

    FASTDDS_EXPORTED_API uint16_t annotation_get_position() const;

    FASTDDS_EXPORTED_API uint16_t annotation_get_bit_bound() const;

    // Annotations setters
    FASTDDS_EXPORTED_API void annotation_set_optional(
            bool optional);

    FASTDDS_EXPORTED_API void annotation_set_key(
            bool key);

    FASTDDS_EXPORTED_API void annotation_set_must_understand(
            bool must_understand);

    FASTDDS_EXPORTED_API void annotation_set_non_serialized(
            bool non_serialized);

    FASTDDS_EXPORTED_API void annotation_set_value(
            const std::string& value);

    FASTDDS_EXPORTED_API void annotation_set_default(
            const std::string& default_value);

    FASTDDS_EXPORTED_API void annotation_set_default_literal();

    FASTDDS_EXPORTED_API void annotation_set_position(
            uint16_t position);

    FASTDDS_EXPORTED_API void annotation_set_bit_bound(
            uint16_t bit_bound);
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_MEMBER_DESCRIPTOR_H
