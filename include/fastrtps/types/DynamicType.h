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

class DynamicType
{
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

    DynamicType();

    DynamicType(const TypeDescriptor* descriptor);

    DynamicType(const DynamicTypeBuilder* other);

    virtual ~DynamicType();

    virtual void clear();

    ReturnCode_t copy_from_builder(const DynamicTypeBuilder* other);

    // Checks if there is a member with the given name.
    bool exists_member_by_name(const std::string& name) const;

    // This method is used by Dynamic Data to override the name of the types based on ALIAS.
    void set_name(const std::string& name);

    ReturnCode_t apply_annotation(AnnotationDescriptor& descriptor);

    ReturnCode_t apply_annotation(
            const std::string& annotation_name,
            const std::string& key,
            const std::string& value);

    ReturnCode_t apply_annotation_to_member(
            MemberId id,
            AnnotationDescriptor& descriptor);

    ReturnCode_t apply_annotation_to_member(
            MemberId id,
            const std::string& annotation_name,
            const std::string& key,
            const std::string& value);

    ReturnCode_t get_annotation(
            AnnotationDescriptor& descriptor,
            uint32_t idx);

    uint32_t get_annotation_count();

    DynamicType_ptr get_base_type() const;

    DynamicType_ptr get_discriminator_type() const;

    DynamicType_ptr get_element_type() const;

    DynamicType_ptr get_key_element_type() const;

    ReturnCode_t get_member(
            DynamicTypeMember& member,
            MemberId id);

    ReturnCode_t get_member_by_name(
            DynamicTypeMember& member,
            const std::string& name);

    TypeDescriptor* descriptor_;
    std::map<MemberId, DynamicTypeMember*> member_by_id_;         // Aggregated members
    std::map<std::string, DynamicTypeMember*> member_by_name_;    // Uses the pointers from "member_by_id_".
    std::string name_;
    TypeKind kind_;
    bool is_key_defined_;

public:
    RTPS_DllAPI bool equals(const DynamicType* other) const;

    RTPS_DllAPI ReturnCode_t get_all_members(std::map<MemberId, DynamicTypeMember*>& members);

    RTPS_DllAPI ReturnCode_t get_all_members_by_name(std::map<std::string, DynamicTypeMember*>& members);

    RTPS_DllAPI uint32_t get_bounds(uint32_t index = 0) const;

    RTPS_DllAPI uint32_t get_bounds_size() const;

    RTPS_DllAPI ReturnCode_t get_descriptor(TypeDescriptor* descriptor) const;

    RTPS_DllAPI const TypeDescriptor* get_descriptor() const;

    RTPS_DllAPI TypeDescriptor* get_descriptor();

    RTPS_DllAPI bool key_annotation() const;

    RTPS_DllAPI inline TypeKind get_kind() const
    {
        return kind_;
    }

    RTPS_DllAPI std::string get_name() const;

    RTPS_DllAPI MemberId get_members_count() const;

    RTPS_DllAPI uint32_t get_total_bounds() const;

    RTPS_DllAPI const TypeDescriptor* get_type_descriptor() const
    {
        return descriptor_;
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
