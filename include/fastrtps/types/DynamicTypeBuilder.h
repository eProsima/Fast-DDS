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

    MemberId current_member_id_ = 0;

    bool check_union_configuration(
            const MemberDescriptor& descriptor);

    // Checks if there is a member with the given name.
    bool exists_member_by_name(
            const std::string& name) const;

    // Checks if there is a member with the given id.
    bool exists_member_by_id(
            MemberId id) const;

    //! This method only adds an empty element to the members collection with the right index
    member_iterator add_empty_member(
            uint32_t index,
            const std::string& name);

    void refresh_member_ids();

    void clear();

    DynamicTypeBuilder(const DynamicTypeBuilder&) = default;
    DynamicTypeBuilder(DynamicTypeBuilder&&) = delete;
    DynamicTypeBuilder& operator=(const DynamicTypeBuilder&) = default;
    DynamicTypeBuilder& operator=(DynamicTypeBuilder&&) = delete;

    const TypeDescriptor& get_type_descriptor() const
    {
        return static_cast<const TypeDescriptor&>(*this);
    }

    // Annotation setters

    //! auxiliary method for all bellow
    template<typename C, typename M>
    void annotation_set(const std::string& id, C& c, M& m);

    //! auxiliary method for all bellow
    void annotation_set(const std::string& id, std::string& new_val);

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

    ~DynamicTypeBuilder() = default;

    friend class DynamicTypeBuilderFactory;

    RTPS_DllAPI bool equals(
            const DynamicType& other) const;

    using TypeDescriptor::get_annotation;

    using TypeDescriptor::get_annotation_count;

    RTPS_DllAPI ReturnCode_t get_descriptor(TypeDescriptor& descriptor)
    {
        descriptor = get_type_descriptor();
        return ReturnCode_t::RETCODE_OK;
    }

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t add_member(
            const MemberDescriptor& descriptor) noexcept;

    template<typename... Ts>
    ReturnCode_t add_member(Ts&&... Args) noexcept
    {
        return add_member(MemberDescriptor(std::forward<Ts>(Args)...));
    }

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t apply_annotation(
            AnnotationDescriptor& descriptor);

    // TODO: doxygen
    RTPS_DllAPI ReturnCode_t apply_annotation(
            const std::string& annotation_name,
            const std::string& key,
            const std::string& value);

    // TODO: doxygen
    template<typename... Ts>
    ReturnCode_t apply_annotation_to_member(
            MemberId id,
            Ts&&... Args)
    {
        auto it = member_by_id_.find(id);
        if (it != member_by_id_.end())
        {
            it->second->apply_annotation(std::forward<Ts>(Args)...);
            return ReturnCode_t::RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error applying annotation to member. MemberId not found.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
    }

    RTPS_DllAPI DynamicType_ptr build() const;

    RTPS_DllAPI ReturnCode_t copy_from(
            const DynamicTypeBuilder* other);

    using TypeDescriptor::get_all_members;

    using TypeDescriptor::get_all_members_by_name;

    using TypeDescriptor::get_name;

    using TypeDescriptor::get_member_id_by_name;

    using TypeDescriptor::is_consistent;

    bool is_discriminator_type() const;

    using TypeDescriptor::set_name;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_BUILDER_H
