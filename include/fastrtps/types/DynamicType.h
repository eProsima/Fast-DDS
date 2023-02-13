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

#include <fastrtps/types/TypeDescriptor.h>

namespace eprosima {

namespace fastdds {
namespace dds {
class DomainParticipantImpl;
} // namespace dds
} // namespace fastdds

namespace fastrtps {
namespace types {

class AnnotationDescriptor;
class TypeDescriptor;
class DynamicTypeMember;
class DynamicTypeBuilder;
class DynamicTypeBuilderFactory;

class DynamicType
    : protected TypeDescriptor
    , public std::enable_shared_from_this<DynamicType>
{
    // Only create objects from the associated factory
    struct use_the_create_method
    {
        explicit use_the_create_method() = default;
    };

public:

    DynamicType(
            use_the_create_method);

    DynamicType(
            use_the_create_method,
            const TypeDescriptor* descriptor);

    RTPS_DllAPI virtual ~DynamicType();

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
    friend class DynamicDataHelper;
    friend class fastdds::dds::DomainParticipantImpl;

    RTPS_DllAPI virtual void clear();

    // Checks if there is a member with the given name.
    bool exists_member_by_name(
            const std::string& name) const;

//    ReturnCode_t apply_annotation(
//            AnnotationDescriptor& descriptor);
//
//    ReturnCode_t apply_annotation(
//            const std::string& annotation_name,
//            const std::string& key,
//            const std::string& value);
//
//    ReturnCode_t apply_annotation_to_member(
//            MemberId id,
//            AnnotationDescriptor& descriptor);
//
//    ReturnCode_t apply_annotation_to_member(
//            MemberId id,
//            const std::string& annotation_name,
//            const std::string& key,
//            const std::string& value);

    ReturnCode_t get_annotation(
            AnnotationDescriptor& descriptor,
            uint32_t idx) const;

    uint32_t get_annotation_count() const;

    using TypeDescriptor::get_base_type;

    using TypeDescriptor::get_discriminator_type;

    using TypeDescriptor::get_element_type;

    using TypeDescriptor::get_key_element_type;

    ReturnCode_t get_member(
            DynamicTypeMember& member,
            MemberId id) const;

    ReturnCode_t get_member_by_name(
            DynamicTypeMember& member,
            const std::string& name) const;

    std::map<MemberId, DynamicTypeMember*> member_by_id_;         // Aggregated members
    std::map<std::string, DynamicTypeMember*> member_by_name_;    // Uses the pointers from "member_by_id_".
    bool is_key_defined_ = false;

    // Serialization ancillary
    void serialize_empty_data(
            const DynamicType_ptr pType,
            eprosima::fastcdr::Cdr& cdr) const;

    bool deserialize_discriminator(
            uint64_t& discriminator_value,
            eprosima::fastcdr::Cdr& cdr);

    void serialize_discriminator(
            DynamicData& data,
            eprosima::fastcdr::Cdr& cdr) const;

    void serializeKey(
            DynamicData& data,
            eprosima::fastcdr::Cdr& cdr) const;

public:

    // Serializes and deserializes the Dynamic Data.
    void serialize(
            DynamicData& data,
            eprosima::fastcdr::Cdr& cdr) const;

    bool deserialize(
            DynamicData& data,
            eprosima::fastcdr::Cdr& cdr);

    size_t getCdrSerializedSize(
            const DynamicData& data,
            size_t current_alignment = 0);

    size_t getEmptyCdrSerializedSize(
            size_t current_alignment = 0);

    size_t getKeyMaxCdrSerializedSize(
            size_t current_alignment = 0);

    size_t getMaxCdrSerializedSize(
            size_t current_alignment = 0);

public:

    RTPS_DllAPI bool equals(
            const DynamicType* other) const;

    RTPS_DllAPI ReturnCode_t get_all_members(
            std::map<MemberId, DynamicTypeMember*>& members) const;

    RTPS_DllAPI ReturnCode_t get_all_members_by_name(
            std::map<std::string, DynamicTypeMember*>& members) const;

    using TypeDescriptor::get_bounds;

    using TypeDescriptor::get_bounds_size;

    RTPS_DllAPI ReturnCode_t get_descriptor(
            TypeDescriptor& descriptor) const;

    RTPS_DllAPI const TypeDescriptor& get_descriptor() const;

    RTPS_DllAPI bool key_annotation() const;

    RTPS_DllAPI inline TypeKind get_kind() const
    {
        return kind_;
    }

    RTPS_DllAPI std::string get_name() const;

    RTPS_DllAPI MemberId get_members_count() const;

    using TypeDescriptor::get_total_bounds;

    RTPS_DllAPI const TypeDescriptor& get_type_descriptor() const
    {
        return static_cast<const TypeDescriptor&>(*this);
    }

    RTPS_DllAPI bool has_children() const;

    RTPS_DllAPI bool is_consistent() const;

    RTPS_DllAPI bool is_complex_kind() const;

    RTPS_DllAPI bool is_discriminator_type() const;

    RTPS_DllAPI size_t get_size() const;

};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_H
