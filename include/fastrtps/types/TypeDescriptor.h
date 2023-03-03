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

#ifndef TYPES_TYPE_DESCRIPTOR_H
#define TYPES_TYPE_DESCRIPTOR_H

#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/TypesBase.h>

#include <limits>
#include <list>
#include <string>
#include <vector>

namespace eprosima {
namespace fastrtps {
namespace types {

class DynamicType;

struct TypeDescriptorData
{
    std::string name_;                      //!< Type Name.
    TypeKind kind_ = TypeKind::TK_NONE;     //!< Type Kind.
    DynamicType_ptr base_type_;             //!< SuperType of an structure or base type of an alias type.
    DynamicType_ptr discriminator_type_;    //!< Discrimination type for a union.
    std::vector<uint32_t> bound_;           //!< Length for strings, arrays, sequences, maps and bitmasks.
    DynamicType_ptr element_type_;          //!< Value Type for arrays, sequences, maps, bitmasks.
    DynamicType_ptr key_element_type_;      //!< Key Type for maps.
    std::list<DynamicTypeMember> members_;  //!< Member descriptors sequence
};

class TypeDescriptor
      : protected TypeDescriptorData
      , protected AnnotationManager
{

    using TypeDescriptorData::TypeDescriptorData;

public:

    RTPS_DllAPI TypeDescriptor(
            const std::string& name,
            TypeKind kind);

    RTPS_DllAPI TypeDescriptor() = default;

    RTPS_DllAPI TypeDescriptor(
            const TypeDescriptor& other);

    RTPS_DllAPI TypeDescriptor(
            TypeDescriptor&& other) = default;

    RTPS_DllAPI TypeDescriptor& operator=(
            const TypeDescriptor& descriptor);

    RTPS_DllAPI TypeDescriptor& operator=(
            TypeDescriptor&& descriptor) = default;

    RTPS_DllAPI ~TypeDescriptor();

    static bool is_type_name_consistent(
            const std::string& sName);

protected:

    bool is_key_defined_ = false;
    std::map<MemberId, DynamicTypeMember*> member_by_id_;       //!< members references indexed by id
    std::map<std::string, DynamicTypeMember*> member_by_name_;  //!< members references indexed by name

    void refresh_indexes();

    // TODO: doxigen
    RTPS_DllAPI ReturnCode_t get_descriptor(
            TypeDescriptor& descriptor) const;

    using member_iterator = std::list<DynamicTypeMember>::iterator;

    RTPS_DllAPI void clean();

    RTPS_DllAPI uint32_t get_members_count() const;

    friend class DynamicTypeBuilderFactory;
    friend class TypeObjectFactory;
    friend class DynamicDataHelper;

    // Checks if there is a member with the given name.
    bool exists_member_by_name(
            const std::string& name) const;

    // Checks if there is a member with the given id.
    bool exists_member_by_id(
            MemberId id) const;

    RTPS_DllAPI void set_name(
            const std::string& name);

    RTPS_DllAPI void set_name(
            std::string&& name);

    RTPS_DllAPI void set_kind(
            TypeKind kind);

    RTPS_DllAPI void set_base_type(
            const DynamicType_ptr& type);

    RTPS_DllAPI void set_base_type(
            DynamicType_ptr&& type);

public:

    using AnnotationManager::annotation_is_bit_bound;
    using AnnotationManager::annotation_is_key;
    using AnnotationManager::annotation_is_non_serialized;

    using AnnotationManager::annotation_is_extensibility;
    using AnnotationManager::annotation_is_mutable;
    using AnnotationManager::annotation_is_final;
    using AnnotationManager::annotation_is_appendable;
    using AnnotationManager::annotation_is_nested;
    using AnnotationManager::key_annotation;

public:
    // ancillary for DynamicData interfaces
    RTPS_DllAPI MemberId get_member_id_by_name(
            const std::string& name) const;

    RTPS_DllAPI MemberId get_member_id_at_index(
            uint32_t index) const;

    RTPS_DllAPI std::pair<const DynamicTypeMember*, bool> get_member(
            MemberId id) const;

public:

    RTPS_DllAPI ReturnCode_t copy_from(
            const TypeDescriptor& descriptor);

    RTPS_DllAPI bool operator==(const TypeDescriptor& descriptor) const;

    RTPS_DllAPI bool equals(
            const TypeDescriptor& descriptor) const;

    RTPS_DllAPI bool is_consistent() const;

    RTPS_DllAPI bool is_primitive() const;

    RTPS_DllAPI DynamicType_ptr get_base_type() const;

    RTPS_DllAPI uint32_t get_bounds(
            uint32_t index = 0) const;

    RTPS_DllAPI uint32_t get_bounds_size() const;

    RTPS_DllAPI DynamicType_ptr get_discriminator_type() const;

    RTPS_DllAPI DynamicType_ptr get_element_type() const;

    RTPS_DllAPI DynamicType_ptr get_key_element_type() const;

    RTPS_DllAPI TypeKind get_kind() const;

    RTPS_DllAPI std::string get_name() const;

    RTPS_DllAPI uint32_t get_total_bounds() const;

    // TODO: doxygen
    RTPS_DllAPI const std::list<DynamicTypeMember>& get_all_members() const;

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t get_all_members(
            std::map<MemberId, const DynamicTypeMember*>& members) const;

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t get_all_members_by_name(
            std::map<std::string, const DynamicTypeMember*>& members) const;

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t get_member(
            MemberDescriptor& member,
            MemberId id) const;

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t get_member_by_name(
            MemberDescriptor& member,
            const std::string& name) const;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_TYPE_DESCRIPTOR_H
