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

#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/TypesBase.h>

#include <list>
#include <map>
#include <memory>
#include <set>

namespace eprosima {
namespace fastrtps {
namespace types {

class MemberDescriptor;
class DynamicTypeMember;
class DynamicType;

class TypeDescriptor
{
protected:

    TypeKind kind_ = TK_NONE;               // Type Kind.
    std::string name_;                      // Type Name.
    DynamicType_ptr base_type_;             // SuperType of an structure or base type of an alias type.
    DynamicType_ptr discriminator_type_;    // Discrimination type for a union.
    std::vector<uint32_t> bound_;           // Length for strings, arrays, sequences, maps and bitmasks.
    DynamicType_ptr element_type_;          // Value Type for arrays, sequences, maps, bitmasks.
    DynamicType_ptr key_element_type_;      // Key Type for maps.
    std::set<AnnotationDescriptor> annotation_; // Annotations to apply

    bool is_key_defined_ = false;

    std::list<DynamicTypeMember> members_;
    std::map<MemberId, const DynamicTypeMember*> member_by_id_;      // members references indexed by id
    std::set<std::string, const DynamicTypeMember*> member_by_name_;    // members references indexed by name

    using annotation_iterator = std::set<AnnotationDescriptor>::iterator;
    using member_iterator = std::list<DynamicTypeMember>::iterator;

    RTPS_DllAPI void clean();

    RTPS_DllAPI MemberId get_members_count() const;

    static bool is_type_name_consistent(
            const std::string& sName);

    friend class DynamicTypeBuilderFactory;
    friend class TypeObjectFactory;
    friend class DynamicType;
    friend class MemberDescriptor;
    friend class DynamicDataHelper;

    // Annotations application
    bool annotation_is_extensibility() const;

    bool annotation_is_mutable() const;

    bool annotation_is_final() const;

    bool annotation_is_appendable() const;

    bool annotation_is_nested() const;

    bool annotation_is_bit_bound() const;

    bool annotation_is_key() const;

    bool annotation_is_non_serialized() const;

    bool key_annotation() const;

    // Annotation getters
    std::string annotation_get_extensibility() const;

    bool annotation_get_nested() const;

    uint16_t annotation_get_bit_bound() const;

    bool annotation_get_key() const;

    //! Returns a Type annotation based on DynamicType name
    std::pair<annotation_iterator, bool> get_annotation(
            const std::string& name) const;

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t get_annotation(
            AnnotationDescriptor& descriptor,
            MemberId idx) const;

    // TODO: doxygen
    RTPS_DllAPI MemberId get_annotation_count() const;

public:

    RTPS_DllAPI TypeDescriptor() = default;

    RTPS_DllAPI TypeDescriptor(
            const TypeDescriptor& other) = default;

    RTPS_DllAPI TypeDescriptor& operator=(
            const TypeDescriptor& descriptor) = default;

    RTPS_DllAPI TypeDescriptor(
            const std::string& name,
            TypeKind kind);

    RTPS_DllAPI ~TypeDescriptor();

    RTPS_DllAPI ReturnCode_t copy_from(
            const TypeDescriptor& descriptor);

    RTPS_DllAPI bool operator==(const TypeDescriptor& descriptor) const;

    RTPS_DllAPI bool equals(
            const TypeDescriptor& descriptor) const;

    RTPS_DllAPI bool is_consistent() const;

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

    RTPS_DllAPI void set_kind(
            TypeKind kind);

    RTPS_DllAPI void set_name(
            std::string name);

    RTPS_DllAPI ReturnCode_t get_all_members(
            std::map<MemberId, DynamicTypeMember*>& members) const;

    RTPS_DllAPI ReturnCode_t get_all_members_by_name(
            std::map<std::string, DynamicTypeMember*>& members) const;

    RTPS_DllAPI ReturnCode_t get_member(
            DynamicTypeMember& member,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t get_member_by_name(
            DynamicTypeMember& member,
            const std::string& name) const;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_TYPE_DESCRIPTOR_H
